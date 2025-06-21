#ifndef SPEAKER_H
#define SPEAKER_H

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <AudioGeneratorMP3.h>
#include <AudioFileSourceSPIFFS.h>
#include <AudioOutputI2S.h>

class AudioGeneratorMP3;
class AudioFileSourceSPIFFS;
class AudioOutputI2S;

extern AudioGeneratorMP3* mp3;
extern AudioFileSourceSPIFFS* file;
extern AudioOutputI2S* out;

using namespace fs;

extern bool isPlaying;
extern bool shouldLoopAudio;

void playAudio(const String& path);
void stopAudio();

#endif  