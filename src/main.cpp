#include "socketSetup.h"

// #define AP_SSID "julbas"
// #define AP_PASSWORD "julbasik"

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected.");

  //default offset arm
  configTime(4 * 3600, 0, "pool.ntp.org");
  Serial.print("⌛ Waiting for time sync");

  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\n🌐 Time synchronized via NTP");

  // Serial.printf("📅 Got time: %04d-%02d-%02d %02d:%02d:%02d\n", 
  //   timeinfo.tm_year + 1900,
  //   timeinfo.tm_mon + 1,
  //   timeinfo.tm_mday,
  //   timeinfo.tm_hour,
  //   timeinfo.tm_min,
  //   timeinfo.tm_sec
  // );

  rtcSetup();
  setRTCFromTm(timeinfo);
  socketBegin();
}

void loop() {
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 1000) {
    RtcData now = rtcReading();
    Serial.println(now.time);
    lastPrint = millis();
  }

  webSocket.loop();
  server.handleClient();

  if (mp3 != nullptr) {
    Serial.println("🔄 mp3 exists, entering playback logic");

    if (!mp3->loop()) {
      Serial.println("⏹ MP3 playback finished (via mp3->loop() = false)");

      if (shouldLoopAudio) {
        Serial.println("🔁 Restarting MP3 due to loop flag");
        stopAudio();
        delay(200); // give I2S time to settle
        playAudio("/uploaded.mp3");
      } else {
        stopAudio(); // cleanup if not looping
      }
    }
  }
}
