#include "rtc.h"
#include "RTClib.h"
#include <time.h>
#include <sys/time.h>

RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

struct RtcData {
  String time;
  float temperature;
};

void setRTCFromTm(struct tm t) {
  if (t.tm_year < 100 || t.tm_mon < 0 || t.tm_mon > 11 || t.tm_mday < 1 || t.tm_mday > 31) {
    Serial.println("❌ Invalid tm values passed to setRTCFromTm");
    return;
  }

  rtc.adjust(DateTime(
    t.tm_year + 1900,
    t.tm_mon + 1,
    t.tm_mday,
    t.tm_hour,
    t.tm_min,
    t.tm_sec
  ));

  Serial.println("✅ RTC set from system local time!");
}

void setRTCFromSystemTime() {
  struct tm timeinfo;
  memset(&timeinfo, 0, sizeof(struct tm));

  if (!getLocalTime(&timeinfo)) {
    Serial.println("❌ Failed to obtain local time");
    return;
  }

  if (timeinfo.tm_year < 100) {
    Serial.println("❌ Invalid time received");
    return;
  }

  setRTCFromTm(timeinfo);
}

void updateTimezoneOffset(int newOffset) {
  Serial.printf("🕒 Updating timezone offset to %d seconds\n", newOffset);

  configTime(newOffset, 0, "pool.ntp.org");

  struct tm timeinfo;
  Serial.print("⌛ Waiting for time re-sync");
  while (!getLocalTime(&timeinfo)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\n🔁 Time re-synced with new offset");

  setRTCFromTm(timeinfo);
}


RtcData rtcReading () {
  DateTime now = rtc.now();
  
  String yearStr   = String(now.year());
  String monthStr  = (now.month() < 10 ? "0" : "") + String(now.month());
  String dayStr    = (now.day() < 10 ? "0" : "") + String(now.day());
  String hourStr   = (now.hour() < 10 ? "0" : "") + String(now.hour());
  String minuteStr = (now.minute() < 10 ? "0" : "") + String(now.minute());
  String secondStr = (now.second() < 10 ? "0" : "") + String(now.second());

  String dayOfWeek = daysOfTheWeek[now.dayOfTheWeek()];
  String formattedTime = dayOfWeek + ", " + yearStr + "-" + monthStr + "-" + dayStr + " " + hourStr + ":" + minuteStr + ":" + secondStr;

  float temperature = rtc.getTemperature();

  RtcData data = { formattedTime, temperature };
  return data;
}

void rtcSetup () {
  delay(500);

  if (!rtc.begin()) {
    Serial.println("❌ Couldn't find RTC. Check wiring.");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("⚠️ RTC lost power. Please send current time in format:");
    Serial.println("YYYY-MM-DD HH:MM:SS");
    setRTCFromSystemTime();
  } else {
    Serial.println("✅ RTC is running. Current time:");
    RtcData now = rtcReading();
    Serial.println(now.time);
  }
}