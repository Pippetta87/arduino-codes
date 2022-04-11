/*
  NTP TZ DST - bare minimum
  NetWork Time Protocol - Time Zone - Daylight Saving Time

  Our target for this MINI sketch is:
  - get the SNTP request running
  - set the timezone
  - (implicit) respect daylight saving time
  - how to "read" time to be printed to Serial.Monitor
  
  This example is a stripped down version of the NTP-TZ-DST (v2)
  And works for ESP8266 core 2.7.4 and 3.0.2

  by noiasca
  2020-09-22
*/

#ifndef STASSID
#define STASSID "Vodafoneebeb"                            // set your SSID
#define STAPSK  "1lUB4jV1pdCCczvNdMyOvQQK"                        // set your wifi password
#endif

/* Configuration of NTP */
#define MY_NTP_SERVER "europe.pool.ntp.org"           
#define MY_TZ "CET-1CEST,M3.5.0,M10.5.0/3"   

/* Necessary Includes */
#include <time.h>                   // time() ctime()
#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>  // we need wifi to get internet access
#endif
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h> 
#endif
#if !defined(__time_t_defined) // avoid conflict with newlib or other posix libc
typedef unsigned long time_t;
#endif

/* Globals */
time_t now;                         // this is the epoch
tm tm,tmonce;                              // the structure tm holds time information in a more convient way
struct tm * timeinfo;
unsigned long onceepoch;
void showTime() {
  time(&now);                       // read the current time
  localtime_r(&now, &tm);           // update the structure tm with the current time
  Serial.print("year:");
  Serial.print(tm.tm_year + 1900);  // years since 1900
  Serial.print("\tmonth:");
  Serial.print(tm.tm_mon + 1);      // January = 0 (!)
  Serial.print("\tday:");
  Serial.print(tm.tm_mday);         // day of month
  Serial.print("\thour:");
  Serial.print(tm.tm_hour);         // hours since midnight  0-23
  Serial.print("\tmin:");
  Serial.print(tm.tm_min);          // minutes after the hour  0-59
  Serial.print("\tsec:");
  Serial.print(tm.tm_sec);          // seconds after the minute  0-61*
  Serial.print("\twday");
  Serial.print(tm.tm_wday);         // days since Sunday 0-6
  if (tm.tm_isdst == 1)             // Daylight Saving Time flag
    Serial.print("\tDST");
  else
    Serial.print("\tstandard");
  Serial.println();
  Serial.println("Epoch:");
    Serial.println(getTime());
      timeinfo = localtime ( &now );
  Serial.println("Timeinfo:");
    Serial.println(mktime(timeinfo));
    Serial.println("Timeinfo+100000:");
    Serial.println(mktime(timeinfo)+100000);
    onceepoch=(mktime(timeinfo)+100000);
    time_t onceepoch_tm=(time_t)onceepoch;
    localtime_r(&onceepoch_tm, &tmonce);           // update the structure tm with the current time
Serial.print("year:");
  Serial.print(tmonce.tm_year + 1900);  // years since 1900
  Serial.print("\tmonth:");
  Serial.print(tmonce.tm_mon + 1);      // January = 0 (!)
  Serial.print("\tday:");
  Serial.print(tmonce.tm_mday);         // day of month
    Serial.println("asctime");         // asctime
      localtime(&now);
      Serial.print(asctime(&tmonce));         // day of month


}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nNTP TZ DST - bare minimum");

 configTime(0, 0, MY_NTP_SERVER); // 0, 0 because we will use TZ in the next line
 setenv("TZ", MY_TZ, 1); // Set environment variable with your time zone
 tzset();
  // start network
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print ( "." );
  }
  Serial.println("\nWiFi connected");
  // by default, the NTP will be started after 60 secs
}

void loop() {
  showTime();
  delay(1000); // dirty delay
}
