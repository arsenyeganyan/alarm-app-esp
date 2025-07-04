import { socket } from "../client.js";

const audioInput = document.getElementById('audio-file');

export function sendMp3ToESP() {
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
  socket.send(JSON.stringify({ type: 'START_MP3' }));

  const reader = new FileReader();

  reader.onload = function (event) {
    if (offset < file.size) {
      const chunk = event.target.result;
      socket.send(chunk);
      offset += chunk.byteLength;
      readNextChunk();
    } else {
      socket.send(JSON.stringify({ type: 'END_MP3' }));
      console.log('Finished sending file.');
      socket.send(JSON.stringify({ type: 'ring_sound' }));
    }
  };

  function readNextChunk() {
    const slice = file.slice(offset, offset + CHUNK_SIZE);
    reader.readAsArrayBuffer(slice);
  }

  readNextChunk();
}