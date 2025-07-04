#ifndef RTC_H
#define RTC_H

#include "Arduino.h"
#include "RTClib.h"

struct RtcData {
    String time;
    float temperature;
};

extern RTC_DS3231 rtc;

void setRTCFromTm(struct tm t);
void setRTCFromSystemTime();
void updateTimezoneOffset(int newOffset);
RtcData rtcReading();
void rtcSetup();

#endif