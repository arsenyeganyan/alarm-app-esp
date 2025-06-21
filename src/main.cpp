#include "socketSetup.h"

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");

  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount SPIFFS");
    return;
  }

  socketBegin();
  // rtcSetup();
}

void loop() {
  webSocket.loop();
  server.handleClient();

  if (mp3 && mp3->isRunning()) {
    if (!mp3->loop()) {
      Serial.println("MP3 playback done");
      mp3->stop();
      isPlaying = false;

      if (shouldLoopAudio) {
        Serial.println("Restarting MP3 playback...");
        playAudio("/received.mp3");
      }
    }
  }
}