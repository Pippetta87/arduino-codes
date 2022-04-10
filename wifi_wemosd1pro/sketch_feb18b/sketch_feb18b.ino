/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP82ESP8266Client66 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

/*pubsubclient - state
 * 
    -4 : MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time
    -3 : MQTT_CONNECTION_LOST - the network connection was broken
    -2 : MQTT_CONNECT_FAILED - the network connection failed
    -1 : MQTT_DISCONNECTED - the client is disconnected cleanly
    0 : MQTT_CONNECTED - the client is connected
    1 : MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT
    2 : MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier
    3 : MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection
    4 : MQTT_CONNECT_BAD_CREDENTIALS - the username/password were rejected
    5 : MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect


 */
//#include <WiFiUdp.h>
//#include <NTPClient.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <time.h>
//#include <sys/time.h>                   // struct timeval

// Update these with values suitable for your network.

int32_t conchannel;
const char* ssid = "Vodafoneebeb";
const char* password = "1lUB4jV1pdCCczvNdMyOvQQK";
const char* mqtt_server = "mqtt.flespi.io";
const int mqttPort = 1883;
const char* mqttUser = "iCF53tOTdMwPvvCQFdjFI0YaagAoIVjQlOJe5nV7XhMgAdX5g6AOVmOTsc7YVgF6";
const char* mqttPassword = "";
const char* clientId = "serra.abete.ruf";
String bssidstr;
uint8_t encryptionType;
int32_t RSSI;
uint8_t* bssid;
int32_t channel;
bool isHidden; 
uint8_t curBss;
uint8_t prevRssi;

unsigned long epochTime;
  int monthDay;
  int currentMonth;
  int currentYear;
  int currentHour;
  int currentMinute;
  int currentSecond; 
  int weekDay;
  int day;
  int month;
  
const char* ntpServer = "europe.pool.ntp.org";
const long  gmtOffset_sec = 3600;   //Replace with your GMT offset (seconds)
const int   daylightOffset_sec = 0;  //Replace with your daylight offset (seconds)
char timeinfo[24];
char* datetimeaux;//var to pubblish date

//serra variables
int humidity1 = 900;
int temperature1 = 22;
   
//control variables for nano
bool forcestart = false;
int pumptime = 270000;
int humidityth = 400;



WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
long tlastwater;

//i2c varconst
const byte SlaveDeviceId = 2;

void send_to_nano(){
  Wire.beginTransmission(SlaveDeviceId);
   Wire.write(forcestart);//write one byte of forcestart
  Wire.write(pumptime >> 8);//write two bytes of pumptime
  Wire.write(pumptime & 255);
  Wire.write(humidityth >> 8);//write two bytes of humidityth
  Wire.write(humidityth & 255);
  Wire.endTransmission();
}

bool check_i2c_slave() {
      Wire.beginTransmission(SlaveDeviceId);
   int error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      Serial.println(SlaveDeviceId, HEX);
      return true;
      }
}

void get_i2c_nano() {
 // Request data from slave.
  Wire.beginTransmission(SlaveDeviceId);
  int available = Wire.requestFrom(SlaveDeviceId, (uint8_t)6);
  
  if(available == 6)
  {
     humidity1 = Wire.read() << 8 | Wire.read();//legge umidita da nano
     temperature1 = Wire.read() << 8 | Wire.read();//legge temperatura da nano
     tlastwater = Wire.read() << 8 | Wire.read();//legge quanto tempo da ultima innaffiata

    Serial.println(humidity1);
    Serial.println(temperature1);
    Serial.println(tlastwater);

  }
  else
  {
    Serial.print("Unexpected number of bytes received: ");
    Serial.println(available);
  }

  int result = Wire.endTransmission();
  if(result)
  {
    Serial.print("Unexpected endTransmission result: ");
    Serial.println(result);
  }
}

void setup_wifi() {
  
  WiFi.mode(WIFI_STA);
  delay(5000);
  //search for best bssid
    byte available_networks = WiFi.scanNetworks();
              Serial.println("Available network");
          Serial.println(available_networks);

int netnum = 0;
prevRssi = 0;
// first lets find the SSID of the network you are looking for
// by iterating through all of the avaialble networks
// since in an enterprise there may be more than one BSSID for the SSID
// Lets find the stringest one
  bssid = WiFi.BSSID(0);
      conchannel=WiFi.channel(0);
      bssidstr=WiFi.BSSIDstr(0);
      Serial.println("BSSID");
      Serial.println(bssidstr);
      Serial.println("channel");
      Serial.println(conchannel);

  for (int network = 0; network < available_networks; network++) {
    if (WiFi.SSID(network) == ssid) {
      Serial.print("Found one ");
      Serial.println (WiFi.RSSI(network));
      if ((uint8_t)WiFi.RSSI(network) > prevRssi) {
      netnum = network;
      prevRssi = (uint8_t)WiFi.RSSI(network);
      bssid = WiFi.BSSID(network);
      conchannel=WiFi.channel(network);
      bssidstr=WiFi.BSSIDstr(network);
         Serial.print("Connecting to bssid");
  Serial.print(bssidstr);
    Serial.print(prevRssi);

    }
    }
}

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println();
    Serial.println("Connecting to bssid");
  Serial.println(bssidstr);
  
          WiFi.begin(ssid,password,conchannel,bssid);
Serial.println(WiFi.status());
WiFi.printDiag(Serial);
  while (WiFi.status() != WL_CONNECTED) {
    delay(5000);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
 payload[length] = '\0'; // Make payload a string by NULL terminating it.
   int intpayload = atoi((char *)payload);      
  //char *topiclist = { "forcestart", "pumptime", "humidityth"};
  int tmp=0;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  const char* topic1="forcestart";
    const char* topic2="pumptime";
  const char* topic3="humidityth";
  Serial.println("strncmp(topic, topic1, strlen(topic1))");
      bool aux=(strncmp(topic, topic1, strlen(topic1))==0);
      Serial.println(aux);
      tmp=tmp+aux;
    if (aux==1){
      client.publish("telemetry","Arduino think Received message on topic forcestart");
    //payload[length] = '\0'; // Make payload a string by NULL terminating it.
   //forcestart = atoi((char *)payload);
   forcestart=intpayload;
          Serial.println("changed to");
          Serial.println(forcestart);
    }
  Serial.println("strncmp(topic, topic2, strlen(topic2))");
aux=(strncmp(topic, topic2, strlen(topic2))==0);
      tmp=tmp+aux;
      Serial.println(aux);
      if (aux==1){
      client.publish("telemetry","Arduino think Received message on topic pumptime");
             pumptime=intpayload;
          Serial.println("changed to");
          Serial.println(pumptime);
    }
    Serial.println("strncmp(topic, topic3, strlen(topic3))");
aux=(strncmp(topic, topic3, strlen(topic3))==0);
      tmp=tmp+aux;
      Serial.println(aux);
      if (aux==1){
      client.publish("telemetry","Arduino think Received message on topic humidityth");
   humidityth = intpayload;
    Serial.println(humidityth);

    }
    if (tmp==1)
    {
            Serial.println("topic recognised");
      }
      else
      {     
        Serial.println("topic not recognised or more than one");
      }
       Wire.beginTransmission(2); // transmit to device #2
  Wire.write(humidityth);              // sends humidityth
    Wire.write(pumptime);              // sends pumptime
  Wire.write(forcestart);              // sends forcestart
  Wire.endTransmission();    // stop transmitting
    }
 
  /*for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    if ( (char)payload[i]=='=' ){
      //*(userData->pressure.toInt())
  Serial.println("found equals");
    }
    else
    {
        Serial.println("No equals found");
    }
    */


//client.connect("arduinoClient", "testuser", "testpass")

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    //clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId,mqttUser,"")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("telemetry", "hello world");
      client.subscribe("forcestart");
      client.subscribe("pumptime");
      client.subscribe("humidityth");
}
}
}

void setup_time () {

 //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}

void printLocalTime()
{
  time_t rawtime;
  struct tm * timeinfo;
  time (&rawtime);
  timeinfo = localtime (&rawtime);
 // Serial.println(asctime(timeinfo));
  delay(1000);
  datetimeaux=asctime(timeinfo);
}

void setup() {
  Serial.begin(9600);
  setup_wifi();
  setup_time();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
 // Start the I2C Bus as Master
  Wire.begin(); 
}

//void onReceive( void (*)(int) );

void loop() {
    printLocalTime();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
/*
const char topic[]  = "hum1";//real unique topic
const char topic2[]  = "temp1";
const char topic3[]  = "lastactive";
*/

if (check_i2c_slave()){
  check_i2c_slave();
}
  unsigned long now = millis();
  if (now - lastMsg > 5000) {
  
    lastMsg = now;
    //++value;
    //the String.toCharArray() 
    //snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", String.toCharArray(asctime(timeinfo)));
  //asctime(timeinfo).toCharArray(msg, 24);
       // Serial.print(aux);
     temp_str = String(temperature1); //converting ftemp (the float variable above) to a string 
    temp_str.toCharArray(temp1, temp_str.length() + 1); //packaging up the data to publish to mqtt whoa...
 hum_str = String(humidity1); //converting ftemp (the float variable above) to a string 
    hum_str.toCharArray(hum1, hum_str.length() + 1); //packaging up the data to publish to mqtt whoa...
        Serial.println("Publish message to hum1");
 client.publish("hum1", hum1);
         Serial.println("Publish message to temp1");
 client.publish("temp1", temp1);
         Serial.println("Publish message to lastactive");
    client.publish("lastactive",datetimeaux);

//serra variables
//int humidity1 = 900;
//int temperature1 = 22; 
//control variables for nano
//int forcestart = 0;
//int pumptime = 270000;
//int humidityth = 400;
}



}
