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
  stopAudio();

  file = new AudioFileSourceSPIFFS(path.c_str());
  out = new AudioOutputI2S();
  out->SetPinout(26, 25, 27);
  out->begin();
  out->SetGain(0.5);

  mp3 = new AudioGeneratorMP3();
  mp3->begin(file, out);

  isPlaying = true;
  Serial.printf("MP3 begin: %s\n", mp3->isRunning() ? "yes" : "no");
}