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
  rtcSetup();
  
  webSocket.broadcastTXT("{\"type\": \"request_time\"}");
}

void loop() {
  static String input = "";

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (input.length() > 0) {
        setRTCFromInput(input);
        input = "";
      }
    } else {
      input += c;
    }
  }

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 1000) {
    RtcData now = rtcReading();
    Serial.println(now.time);
    lastPrint = millis();
  }

  webSocket.loop();
  server.handleClient();

  if (mp3) {
    if (mp3->isRunning()) {
      mp3->loop();
    } else if (shouldLoopAudio && !isPlaying) {
      Serial.println("Restarting MP3 playback...");
      playAudio("/received.mp3");
    } else if (isPlaying) {
      Serial.println("MP3 playback done");
      stopAudio();
    }
  }
}
