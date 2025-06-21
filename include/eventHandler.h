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

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  if (type == WStype_TEXT) {
    StaticJsonDocument<200> doc;
    DeserializationError err = deserializeJson(doc, payload);

    if (err) {
      Serial.println("JSON parse failed");
      return;
    }

    String messageType = doc["type"];

    if (messageType == "set_alarm") {
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
      RtcData currentTime = rtcReading();
      for (Alarm &alarm : alarmList) {
        if (alarm.enabled && alarm.time == String(currentTime)) {
          Serial.println("Triggering alarm for time: " + String(currentTime));
          alarm.enabled = false;
          webSocket.sendTXT(num, "ring_alarm");
          break;
        }
      }
    }
  }
}