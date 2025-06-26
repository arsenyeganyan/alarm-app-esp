const socket = new WebSocket('ws://' + location.hostname + ':81/');

socket.addEventListener('open', () => {
  console.log('WebSocket connected to ESP32');
  socket.send(JSON.stringify({ type: 'get_alarms' }));
});

socket.addEventListener('message', (event) => {
  try {
    const data = event.data;

    if (data.type === "request_time") {
      const now = new Date();
      const yyyy = now.getFullYear();
      const MM = String(now.getMonth() + 1).padStart(2, '0');
      const dd = String(now.getDate()).padStart(2, '0');
      const hh = String(now.getHours()).padStart(2, '0');
      const mm = String(now.getMinutes()).padStart(2, '0');
      const ss = String(now.getSeconds()).padStart(2, '0');

      const datetime = `${yyyy}-${MM}-${dd} ${hh}:${mm}:${ss}`;
      console.log("Date: ", datetime);
      
      socket.send(JSON.stringify({
        type: "set_time",
        datetime
      }));

      console.log("🕒 Sent current time to ESP32:", datetime);
    }

    // ESP says to ring the alarm
    else if (data === "send_sound") {
      console.log("Alarm triggered by ESP32");

      alarmMessage.style.display = 'block';
      
      sendMp3ToESP();
    }

    //renders all the available alarms in html
    else if (data.startsWith("{")) {
      const json = JSON.parse(data);
      if (json.alarms) {
        const alarmList = document.getElementById('alarm-list');
        alarmList.innerHTML = '';

        json.alarms.forEach((alarm, index) => {
          const li = document.createElement('li');
          li.innerHTML = `
            <label class="switch">
              <input type="checkbox" ${alarm.enabled ? 'checked' : ''} onchange="toggleAlarm(${index}, this.checked)">
              <span class="slider"></span>
              <span class="alarm-time-label">${alarm.time}</span>
            </label>
            <button class="remove-alarm" onclick="removeAlarm(this, ${index})">Delete</button>
          `;
          alarmList.appendChild(li);
        });
      }
    }

  } catch (e) {
    console.error("WebSocket message parse error:", e);
  }
});

socket.addEventListener('error', (e) => {
  console.error('WebSocket error:', e);
});

socket.addEventListener('close', () => {
  console.log('WebSocket connection closed');
});

const currentTimeDisplay = document.getElementById('current-time');
const alarmMessage = document.getElementById('alarm-message');
const audioInput = document.getElementById('audio-file');
const errorMessage = document.getElementById('error-message');

errorMessage.style.display = 'none';

function sendMp3ToESP() {
  console.log('entered sending mp3');
  
  const file = audioInput.files[0];
  if (!file || file.type !== 'audio/mpeg') {
    console.error('No valid MP3 file selected');
    errorMessage.style.display = 'block';
    return;
  }

  const CHUNK_SIZE = 2048; 
  const socketOpen = socket.readyState === WebSocket.OPEN;

  if (!socketOpen) {
    console.error('WebSocket not open');
    return;
  }

  let offset = 0;
  socket.send(JSON.stringify({
    type: 'START_MP3',
  }));

  const reader = new FileReader();

  reader.onload = function (event) {
    if (offset < file.size) {
      const chunk = event.target.result;
      socket.send(chunk);
      offset += chunk.byteLength;

      readNextChunk();
    } else {
      socket.send(JSON.stringify({
        type: 'END_MP3',
      }));
      console.log('Finished sending file.');

      socket.send(JSON.stringify({
        type: "ring_sound",
      }));
    }
  };

  function readNextChunk() {
    const slice = file.slice(offset, offset + CHUNK_SIZE);
    reader.readAsArrayBuffer(slice);
  }

  readNextChunk();
}

let alarmAudio = new Audio();

// function getCurrentTimeHHMM() {
//   const now = new Date();
//   const hours = String(now.getHours()).padStart(2, '0');
//   const minutes = String(now.getMinutes()).padStart(2, '0');
//   return `${hours}:${minutes}`;
// }

audioInput.addEventListener('change', () => {
  const file = audioInput.files[0];
  
  if (file && file.type === 'audio/mpeg') {
    const url = URL.createObjectURL(file);
    alarmAudio.src = url;
  }
});

function updateTime() {
  const now = new Date();
  const timeString = now.toLocaleTimeString();
  currentTimeDisplay.textContent = timeString;

  // const currentHHMM = getCurrentTimeHHMM();

  if (socket.readyState === WebSocket.OPEN) {
    socket.send(JSON.stringify({
      type: "check_alarm",
    }));
  }
}


function renderAlarmList() {
  if (socket.readyState === WebSocket.OPEN) {
    socket.send(JSON.stringify({ type: "get_alarms" }));
  }
}

function stopAlarm() {
  alarmMessage.style.display = "none";

  let alarmStore = JSON.parse(localStorage.getItem('store'));

  // const currentHHMM = getCurrentTimeHHMM();

  //communicate with the ESP
  socket.send(JSON.stringify({
    type: "stop_alarm",
  }));

  renderAlarmList();
}

function toggleAlarm(index, isChecked) {
  const time = document.querySelectorAll('.alarm-time-label')[index].textContent;
  socket.send(JSON.stringify({ type: 'toggle_alarm', time, enabled: isChecked }));
}

function setAlarm() {
  const input = document.getElementById('alarm-time').value;
  if (input && socket.readyState === WebSocket.OPEN) {
    const newAlarm = {
      type: 'set_alarm',
      time: input
    };
    socket.send(JSON.stringify(newAlarm));
    alert(`Alarm set for ${input}`);
  }

  renderAlarmList();
}

function removeAlarm(button, index) {
  if (socket.readyState === WebSocket.OPEN) {
    const alarmTime = button.parentElement.querySelector('.alarm-time-label').textContent;
    socket.send(JSON.stringify({ type: 'remove_alarm', time: alarmTime }));
  }
  button.parentElement.remove();

  renderAlarmList();
}

setInterval(updateTime, 1000);
renderAlarmList();
updateTime();