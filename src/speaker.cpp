#include "modulesAndUtils/speaker.h"

#include <WiFi.h>
#include "driver/i2s.h"

AudioGeneratorMP3 *mp3 = nullptr;
AudioFileSourceSPIFFS *file = nullptr;
AudioOutputI2S *out = nullptr;

bool isPlaying = false;
bool shouldLoopAudio = false;

void stopAudio() {
  if (mp3) {
    mp3->stop();
    delete mp3;
    mp3 = nullptr;
  }
  if (file) {
    delete file;
    file = nullptr;
  }
  if (out) {
    out->stop();
    delete out;
    out = nullptr;
  }
  isPlaying = false;
  Serial.println("Stopped playing audio");
}

void playAudio(const String& path) {
  stopAudio();  // cleanup old instances

  Serial.println("🟡 Attempting to play: " + path);

  if (!SPIFFS.exists(path)) {
    Serial.println("❌ MP3 file not found in SPIFFS.");
    return;
  }

  file = new AudioFileSourceSPIFFS(path.c_str());
  out = new AudioOutputI2S();
  out->SetPinout(26, 25, 27); 
  out->begin();
  out->SetGain(0.1);

  mp3 = new AudioGeneratorMP3();
  if (mp3->begin(file, out)) {
    Serial.println("✅ MP3 begin: success");
    // isPlaying = true;
  } else {
    Serial.println("❌ MP3 begin: failed (during loop?)");
    // isPlaying = false;
    shouldLoopAudio = false;
    stopAudio(); 
  }
}