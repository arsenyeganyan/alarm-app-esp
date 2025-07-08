import { sendMp3ToESP } from "./utils/sendMp3ToESP.js";
import { 
  // updateTime,
  renderAlarmList,
  stopAlarm,
  toggleAlarm,
  removeAlarm,
  setAlarm,
  timezoneConfig,
} from "./utils/timeFunctions.js";

export const socket = new WebSocket('ws://' + location.hostname + ':81/');

document.addEventListener('DOMContentLoaded' , (e) => {

  function openPopup() {
    const popup = document.querySelector(".container");
    popup.style.display = "block";
  }

  function closePopup() {
    const popup = document.querySelector(".container");
    popup.style.display = "none";
  }

  const alarmMessage = document.getElementById('alarm-message');
  
  window.toggleAlarm = toggleAlarm;
  window.removeAlarm = removeAlarm;
  window.setAlarm = setAlarm;
  window.stopAlarm = stopAlarm;
  window.timezoneConfig = timezoneConfig;
  window.openPopup = openPopup;
  window.closePopup = closePopup;
  
  let currentEpoch = null;
  let timerInterval = null;
  
  let alarmAudio = new Audio();
  const audioInput = document.getElementById('audio-file');
  
  socket.addEventListener('open', () => {
    console.log('WebSocket connected to ESP32');
    socket.send(JSON.stringify({ type: 'get_alarms' }));
  });
  
  socket.addEventListener('message', (event) => {
    try {
      const data = event.data;
  
      if (data === "send_sound") {
        console.log("Alarm triggered by ESP32");
        alarmMessage.style.display = 'block';
  
        const file = audioInput.files[0];
        if (!file || file.type !== 'audio/mpeg') {
          console.warn("No MP3 selected to send");
          socket.send(JSON.stringify({ type: "no_sound_available" }));
          return;
        }
  
        sendMp3ToESP();
      }
  
      else if (data.startsWith("{")) {
        const json = JSON.parse(data);
  
        if (json.type === "configured_time") {
          const epoch = parseInt(json.epoch);
  
          if (!isNaN(epoch)) {
            currentEpoch = epoch;
            if (timerInterval) clearInterval(timerInterval);
            startLocalClock();
          } else {
            console.error("⚠️ Invalid epoch value received:", data.epoch);
          }
        }
        if (json.alarms) {
          const alarmList = document.getElementById('alarm-list');
          alarmList.innerHTML = '';
  
          console.log("Alarms received from ESP32:", json.alarms);
  
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
        if (json.time) {
          
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
  
  function startLocalClock() {
    timerInterval = setInterval(() => {
      currentEpoch += 1;
  
      const date = new Date(currentEpoch * 1000);
      const timeStr = date.toLocaleTimeString();
      const dateStr = date.toLocaleDateString();
  
      document.getElementById("current-time").textContent = `${dateStr} ${timeStr}`;
  
      socket.send(JSON.stringify({ type: "check_alarm" }));
    }, 1000);
  }
  
  const errorMessage = document.getElementById('error-message');
  errorMessage.style.display = 'none';
  
  audioInput.addEventListener('change', () => {
    const file = audioInput.files[0];
  
    if (file && file.type === 'audio/mpeg') {
      const url = URL.createObjectURL(file);
      alarmAudio.src = url;
    }
  });
  
  // setInterval(updateTime, 1000);
  renderAlarmList();
  // updateTime();
})