#include <WebSocketsServer.h>
#include <WebServer.h>
#include <vector>
#include <ArduinoJson.h>

#include "./modulesAndUtils/speaker.h"
#include "modulesAndUtils/rtcTime.h"

struct Alarm {
  String time;
  bool enabled;
};

std::vector<Alarm> alarmList;

File mp3File;
bool receivingMp3 = false;

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

String extractHourMinute(const String& timeStr) {
  int timeStart = timeStr.indexOf(' ') + 1;  // Skips "Monday,"
  timeStart = timeStr.indexOf(' ', timeStart) + 1;  // Skips date
  return timeStr.substring(timeStart, timeStart + 5);  // "13:00"
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  if (type == WStype_TEXT) {
    StaticJsonDocument<200> doc;
    DeserializationError err = deserializeJson(doc, payload, length);

    if (err) {
      Serial.println("JSON parse failed");
      return;
    }

    String messageType = doc["type"];

    if (messageType == "set_time") {
      String dt = doc["datetime"];

      int year   = dt.substring(0, 4).toInt();
      int month  = dt.substring(5, 7).toInt();
      int day    = dt.substring(8, 10).toInt();
      int hour   = dt.substring(11, 13).toInt();
      int minute = dt.substring(14, 16).toInt();
      int second = dt.substring(17, 19).toInt();

      if (year > 2000 && month > 0 && day > 0) {
        rtc.adjust(DateTime(year, month, day, hour, minute, second));
        Serial.println("✅ RTC synced from browser.");
      } else {
        Serial.println("❌ Invalid datetime from client.");
      }
    }
    else if (messageType == "set_alarm") {
      String time = doc["time"];
      alarmList.push_back({ time, true });
      Serial.println("Alarm set for " + time);
    }
    else if (messageType == "remove_alarm") {
      String time = doc["time"];
      alarmList.erase(std::remove_if(alarmList.begin(), alarmList.end(),
        [&](Alarm a) { return a.time == time; }), alarmList.end());
      Serial.println("Removed alarm for " + time);
    }
    else if (messageType == "toggle_alarm") {
      String time = doc["time"];
      bool enabled = doc["enabled"];
      for (auto &alarm : alarmList) {
        if (alarm.time == time) {
          alarm.enabled = enabled;
        }
      }
    }
    else if (messageType == "get_alarms") {
      StaticJsonDocument<512> out;
      JsonArray arr = out.createNestedArray("alarms");

      for (const auto& alarm : alarmList) {
        JsonObject obj = arr.createNestedObject();
        obj["time"] = alarm.time;
        obj["enabled"] = alarm.enabled;
      }

      String response;
      serializeJson(out, response);
      webSocket.sendTXT(num, response);
    }
    else if (messageType == "check_alarm") {
      RtcData* currentTime = new RtcData(rtcReading());
      String currentHHMM = extractHourMinute(currentTime->time);

      Serial.println("Current HH:MM: " + currentHHMM);

      for (Alarm &alarm : alarmList) {
        Serial.println("Comparing with alarm time: " + alarm.time);
        if (alarm.enabled && alarm.time == currentHHMM) {
          Serial.println("Triggering alarm for time: " + currentHHMM);
          alarm.enabled = false;
          
          if (SPIFFS.exists("/uploaded.mp3")) {
            Serial.println("Alarm audio already exists on the server.");
            shouldLoopAudio = true;
            playAudio("/uploaded.mp3");
          } else {
            Serial.println("Getting new mp3 file...");
            webSocket.sendTXT(num, "send_sound");
          }

          break;
        }
      }
    }
    else if(messageType == "ring_sound") {
      Serial.println("ring (server)");
      shouldLoopAudio = true;
      playAudio("/uploaded.mp3");
    }
    else if(messageType == "stop_alarm") {
      shouldLoopAudio = false;
      stopAudio();
    }
    else if (messageType == "START_MP3") {
      if (SPIFFS.exists("/uploaded.mp3")) {
        SPIFFS.remove("/uploaded.mp3");
      }

      mp3File = SPIFFS.open("/uploaded.mp3", FILE_WRITE);
      if (!mp3File) {
        Serial.println("Failed to open file for writing");
        return;
      }

      receivingMp3 = true;
      Serial.println("Start receiving MP3...");
    }
    else if (messageType == "END_MP3") {
      if (mp3File) {
        mp3File.close();
      }
      receivingMp3 = false;
      Serial.println("MP3 file received and saved.");
    }
  }
  else if (type == WStype_BIN) {
    if (receivingMp3 && mp3File) {
      mp3File.write(payload, length);
      Serial.printf("Wrote %u bytes\n", length);
    } else {
      Serial.println("Received binary data without START_MP3");
    }
  }
}