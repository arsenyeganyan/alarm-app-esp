const alarmMessage = document.getElementById('alarm-message');
const currentTimeDisplay = document.getElementById('current-time');
const timezoneInput = document.getElementById('timezone');

import { socket } from "../client.js";

export function renderAlarmList() {
  if (socket.readyState === WebSocket.OPEN) {
    socket.send(JSON.stringify({ type: "get_alarms" }));
  }
}

// export function updateTime() {
//   // const now = new Date();
//   // const timeString = now.toLocaleTimeString();
//   socket.addEventListener("message", (event) => {
//     try {
//         const data = event.data;
    
//         if (data.startsWith("{")) {
//           const json = JSON.parse(data);
//           if (json.time) {
            
//           }
//         }
//       } catch (e) {
//         console.error("WebSocket message parse error (update time):", e);
//       }
//   })

//   currentTimeDisplay.textContent = timeString;

//   if (socket.readyState === WebSocket.OPEN) {
//     socket.send(JSON.stringify({ type: "check_alarm" }));
//   }
// }

export function stopAlarm() {
  alarmMessage.style.display = "none";

  socket.send(JSON.stringify({ type: "stop_alarm" }));

  renderAlarmList();
}

export function toggleAlarm(index, isChecked) {
  const time = document.querySelectorAll('.alarm-time-label')[index].textContent;
  socket.send(JSON.stringify({ type: 'toggle_alarm', time, enabled: isChecked }));
  renderAlarmList();
}

export function setAlarm() {
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

export function removeAlarm(button, index) {
  if (socket.readyState === WebSocket.OPEN) {
    const alarmTime = button.parentElement.querySelector('.alarm-time-label').textContent;
    socket.send(JSON.stringify({ type: 'remove_alarm', time: alarmTime }));
  }
  button.parentElement.remove();

  renderAlarmList();
}

export function timezoneConfig() {
  const input = timezoneInput.value;
  socket.send(JSON.stringify({ type: "config_timezone", offset: input }));
  renderAlarmList();
}