#include "rtc.h"
#include "RTClib.h"

RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

struct RtcData {
  String time;
  float temperature;
};

void setRTCFromInput(const String& input) {
  if (input.length() < 19) {
    Serial.println("Error: Time string too short.");
    return;
  }

  int year   = input.substring(0, 4).toInt();
  int month  = input.substring(5, 7).toInt();
  int day    = input.substring(8, 10).toInt();
  int hour   = input.substring(11, 13).toInt();
  int minute = input.substring(14, 16).toInt();
  int second = input.substring(17, 19).toInt();

  if (year > 2000 && month > 0 && day > 0 && hour >= 0 && minute >= 0 && second >= 0) {
    rtc.adjust(DateTime(year, month, day, hour, minute, second));
    Serial.println("✅ RTC time set successfully!");
  } else {
    Serial.println("❌ Invalid format or value.");
  }
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
  } else {
    Serial.println("✅ RTC is running. Current time:");
    RtcData now = rtcReading();
    Serial.println(now.time);
  }
}