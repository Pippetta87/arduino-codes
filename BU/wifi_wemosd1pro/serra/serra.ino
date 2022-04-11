#include <WiFiUdp.h>
#include <ArduinoMqttClient.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <Wire.h>

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
const char ssid[] = "Vodafoneebeb";        // your network SSID (name)
const char pass[] = "1lUB4jV1pdCCczvNdMyOvQQK";    // your network password (use for WPA, or use as key for WEP)

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org");

//serra variables
int humidity1 = 900;
int temperature1 = 22;
//int datetimerelayon;

//control variables for nano
int forcestart = 0;
int pumptime = 270000;
int humidityth = 400;

const char broker[] = "mqtt.flespi.io";
int        port     = 1883;
const char topic[]  = "hum1";//real unique topic
const char topic2[]  = "temp1";
const char topic3[]  = "lastactive";

//set interval for sending messages (milliseconds)
const long interval = 6000;
unsigned long previousMillis = 0;


int count = 0;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  WiFi.mode(WIFI_STA);
  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  Serial.println();

// Initialize a NTPClient to get time
  timeClient.begin();
 //Loading Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(19800);  //IST is UTC+5:30 Hrs

   // You can provide a unique client ID, if not set the library uses Arduino-millis()
  // Each client must have a unique client ID
  mqttClient.setId("serra");

  // You can provide a username and password for authentication
  mqttClient.setUsernamePassword("iCF53tOTdMwPvvCQFdjFI0YaagAoIVjQlOJe5nV7XhMgAdX5g6AOVmOTsc7YVgF6", "");

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

mqttClient.connect(broker, port);
  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
}

void loop() {

    timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  Serial.print("Epoch Time: ");
  Serial.println(epochTime);
  
  String formattedTime = timeClient.getFormattedTime();
  Serial.print("Formatted Time: ");
  Serial.println(formattedTime);  
  int currentHour = timeClient.getHours();
  Serial.print("Hour: ");
  Serial.println(currentHour);  
  int currentMinute = timeClient.getMinutes();
  Serial.print("Minutes: ");
  Serial.println(currentMinute); 
   
  int currentSecond = timeClient.getSeconds();
  Serial.print("Seconds: ");
  Serial.println(currentSecond);  
  int weekDay = timeClient.getDay();
  Serial.print("Week Day: ");
  Serial.println(weekDay);    
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
    Serial.println(millis());    

  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  mqttClient.poll();

 unsigned long currentMillis = millis();
 if (currentMillis - previousMillis >= interval) {

    // save the last time a message was sent

    previousMillis = currentMillis;
  //  topic[]  = "hum1";//real unique topic
  // topic2[]  = "temp1";
  //  topic3[]  = "lastactive";
    Serial.print("Sending message to topic: hum1");
    Serial.println(topic);
    Serial.println(humidity1);

    Serial.print("Sending message to topic: temp1");
    Serial.println(topic2);
    Serial.println(temperature1);

    Serial.print("Sending message to topic: ");
    Serial.println(topic3);
    //Serial.println(datetimerelayon);

    // send message, the Print interface can be used to set the message contents
    mqttClient.beginMessage(topic);
    mqttClient.print(humidity1);
    mqttClient.endMessage();

    mqttClient.beginMessage(topic2);
    mqttClient.print(temperature1);
    mqttClient.endMessage();

//    mqttClient.beginMessage(topic3);
//    mqttClient.print(datetimerelayon);
//    mqttClient.endMessage();
 }
    Serial.println();
    timeClient.update();

  Serial.println(timeClient.getFormattedTime());
  
  }
