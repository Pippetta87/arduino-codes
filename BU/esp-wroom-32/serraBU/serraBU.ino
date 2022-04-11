//#include <WiFiUdp.h>
//#include <NTPClient.h>
//#include <SPI.h> //Library for using SPI Communication
//#include <mcp2515.h> //Library for using CAN Communication
//struct can_frame canMsg;
//MCP2515 mcp2515(15);//D8=15
/*
 * ESP32 Chip model = ESP32-D0WDQ6 Rev 3
00:08:19.394 -> This chip has 2 cores
00:08:19.394 -> Chip ID: 2292260

#if !defined(__time_t_defined) // avoid conflict with newlib or other posix libc
typedef unsigned long time_t;
#endif
*/
//Configuration of NTP
#define MY_NTP_SERVER "europe.pool.ntp.org"           
#define MY_TZ "CET-1CEST,M3.5.0,M10.5.0/3"   
/*
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

void initialize() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
}
*/
#include <Wire.h>
#include </home/salvicio/Arduino/libraries/I2C_Anything/I2C_Anything.h> 
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval
#define SDA 21
#define SCL 22

//const int MY_INT_PIN = 16;
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

//String formattedTime = timeClient.getFormattedTime();
//  Serial.print("Formatted Time: ");
//  Serial.println(formattedTime);  
//  int currentHour = timeClient.getHours();
 
  //Get a time structure
 // struct tm *timeinfo;// pointer to a tm struct;
 // struct tm *ptm = gmtime ((time_t *)&epochTime); 
  time_t now,epocheau_tm;                     // this is the epoch
  tm tm,epochstart_tm,tmepocheeau;                              // the structure tm holds time information in a more convient way
  struct tm * timeinfo;
  unsigned long epocheau,internal_tick;

unsigned long prev_read=0;
unsigned long prev_send=0;
  
const char* ntpServer = "europe.pool.ntp.org";
const long  gmtOffset_sec = +3600;   //Replace with your GMT offset (seconds)
const int   daylightOffset_sec = 0;  //Replace with your daylight offset (seconds)
//char timeinfo[24];
//char datetimeaux[30];//var to pubblish date
// Global variables for Time
// time_t rawtime;// global holding current datetime as Epoch 

String  temp_str;
char temp1char[6];
String  hum_str;
char hum1char[6];
char humiditythchar[6],pumptimechar[20],forcestartchar[6];
char epochstartchar[30];
String forcestart_str, pumptime_str, humidityth_str,epochstart_str,epocheau_str;
char epocheauchar[30];
//control variables for nano
//bool forcestart=false;
//int pumptime=270000;
//short humidityth=400;
struct remotedata_STRUCT
{
//control var from wemos
bool forcestart = false;
int pumptime = 270000;
short humidityth = 400;
int Hour;
} remotedata;

struct sensorsdata_STRUCT
{
//sensor variables
unsigned short hum1;  // variable to store the value read
unsigned short temp1 = 59;  // variable to store the value read
unsigned long watertick;
} sensorsdata;

const short i2c_size_esp=sizeof(remotedata.forcestart)+sizeof(remotedata.pumptime)+sizeof(remotedata.humidityth)+sizeof(remotedata.Hour);
const short i2c_size_nano=sizeof(sensorsdata.hum1)+sizeof(sensorsdata.temp1)+sizeof(sensorsdata.watertick);


const char* topic1="forcestart";
  const char* topic2="pumptime";
  const char* topic3="humidityth";
  
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
//#define MSG_BUFFER_SIZE  (50)
//char msg[MSG_BUFFER_SIZE];
//int value = 0;
//long tlastwater;

void i2c_scan(){
    byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
}
}

void setup_wifi() {
  
  WiFi.mode(WIFI_STA);
  delay(500);
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
    delay(1000);
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
  Serial.println("strncmp(topic, topic1, strlen(topic1))");
      bool aux=(strncmp(topic, topic1, strlen(topic1))==0);
      Serial.println(aux);
      tmp=tmp+aux;
    if (aux==1){
      client.publish("telemetry","Arduino think Received message on topic forcestart");
 //   payload[length] = '\0'; // Make payload a string by NULL terminating it.
// forcestart = atoi((char *)payload);
   //forcestart=intpayload;
   //forcestart=(bool) payload
   remotedata.forcestart=true;
          Serial.println("changed to");
          Serial.println(remotedata.forcestart);
                forcestart_str = String(remotedata.forcestart); //converting ftemp (the float variable above) to a string 
    forcestart_str.toCharArray(forcestartchar, forcestart_str.length() + 1); //packaging up the data to publish to mqtt whoa...
                client.publish("telemetry","Arduino think Received message on topic force start: forcestart=");
                                client.publish("telemetry",forcestartchar);
    }
  Serial.println("strncmp(topic, topic2, strlen(topic2))");
aux=(strncmp(topic, topic2, strlen(topic2))==0);
      tmp=tmp+aux;
      Serial.println(aux);
      if (aux==1){
      client.publish("telemetry","Arduino think Received message on topic pumptime");
             remotedata.pumptime=intpayload;
          Serial.println("changed to");
          Serial.println(remotedata.pumptime);
              pumptime_str[pumptime_str.length()] = '\0'; // Make payload a string by NULL terminating it.
         pumptime_str.toCharArray(pumptimechar, pumptime_str.length() + 1); //packaging up the data to publish to mqtt whoa...
                client.publish("telemetry","Arduino think Received message on topic force start: pumptime=");
                                client.publish("telemetry",pumptimechar);
    }
    Serial.println("strncmp(topic, topic3, strlen(topic3))");
aux=(strncmp(topic, topic3, strlen(topic3))==0);
      tmp=tmp+aux;
      Serial.println(aux);
      if (aux==1){
      client.publish("telemetry","Arduino think Received message on topic humidityth");
   remotedata.humidityth = intpayload;
    Serial.println(remotedata.humidityth);
    humidityth_str=(String) intpayload;
    humidityth_str[humidityth_str.length()] = '\0'; // Make payload a string by NULL terminating it.
    humidityth_str.toCharArray(humiditythchar, humidityth_str.length() + 1); //packaging up the data to publish to mqtt whoa...
                client.publish("telemetry","Arduino think Received message on topic force start: humidityth=");
                                client.publish("telemetry",humiditythchar);
    }
    if (tmp==1)
    {
            Serial.println("topic recognised");
      }
      else
      {     
        Serial.println("topic not recognised or more than one");
      }
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
/*
void can_send (short address)
{
canMsg.can_id = address; //CAN id as address
  canMsg.can_dlc = 3; //CAN data length as num
  canMsg.data[0] = forcestart;
  canMsg.data[1] =pumptime;
  canMsg.data[2] =humidityth;
  mcp2515.sendMessage(&canMsg); //Sends the CAN message
 Serial.println("Sent forcestart, pumptime, humidityth via CAN.");
}

void can_read(short address){
   if ((mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) && (canMsg.can_id == address)){
     humidity1 = canMsg.data[0];
    temperature1 = canMsg.data[1];
    watertick = canMsg.data[2]; 
     Serial.println("received forcestart via can:");
     Serial.println(forcestart);
 Serial.println("received pumptime via can:");
     Serial.println(pumptime);
      Serial.println("received humidityth via can:");
     Serial.println(humidityth);   
   }
   else{
        Serial.println("mcp2515.readMessage(&canMsg):");
    Serial.println(mcp2515.readMessage(&canMsg));
   }
}
*/

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
  //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.println("\nNTP TZ DST - bare minimum");
 configTime(0, 0, MY_NTP_SERVER); // 0, 0 because we will use TZ in the next line
 setenv("TZ", MY_TZ, 1); // Set environment variable with your time zone
 tzset();
    Serial.println("\nWaiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  printLocalTime();
}

void printLocalTime()
{
  time(&now);
   localtime_r(&now,&tm);
 //Serial.printf("The current date/time is: %s", asctime(&tm));
    remotedata.Hour=tm.tm_hour;
//  Serial.println(tm.tm_min);
}

/*
ICACHE_RAM_ATTR void my_isr(){
  Serial.println("Interrupt Detected");
}
*/

void setup() {
    Serial.begin(115200);
    while (!Serial){}
        Serial.println("serial init");
  //pinMode(interruptPin, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, CHANGE);
//  pinMode(MY_INT_PIN, INPUT_PULLUP);
//  attachInterrupt(MY_INT_PIN, my_isr, CHANGE);
 // Wire.begin((SDA, SCL));
    Wire.begin();

      Serial.println("Wire init");
setup_wifi();
      Serial.println("wifi init");
  setup_time();
        Serial.println("time setup");
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
        Serial.println("MQTT init");
time(&now);
localtime_r(&now,&epochstart_tm);
       Serial.println("setting epochstart");
 Serial.println((String) asctime(&epochstart_tm));
         client.publish("telemetry","epochstart");
    epochstart_str = (String) asctime(&epochstart_tm); //converting ftemp (the float variable above) to a string 
    epochstart_str[epochstart_str.length()] = '\0';
     epochstart_str.toCharArray(epochstartchar, epochstart_str.length()); //packaging up the data to publish to mqtt whoa...
//    char* aux=asctime(epochstart_tm);
         client.publish("telemetry", epochstartchar);

  //SPI.begin(); //Begins SPI communication
//  mcp2515.reset();
//  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ); //Sets CAN at speed 500KBPS and Clock 8MHz
//  mcp2515.setNormalMode();

} 

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

  internal_tick = millis();
  if (internal_tick - lastMsg > 5000) {
  
    lastMsg = internal_tick;
    //++value;
    //the String.toCharArray() 
    //snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", String.toCharArray(asctime(timeinfo)));
  //asctime(timeinfo).toCharArray(msg, 24);
       // Serial.print(aux);
     temp_str = (String) sensorsdata.temp1; //converting ftemp (the float variable above) to a string 
     temp_str[temp_str.length()] = '\0';
    temp_str.toCharArray(temp1char, temp_str.length()); //packaging up the data to publish to mqtt whoa...
 hum_str = (String) sensorsdata.hum1; //converting ftemp (the float variable above) to a string 
     hum_str[hum_str.length()] = '\0';
    hum_str.toCharArray(hum1char, hum_str.length()); //packaging up the data to publish to mqtt whoa...
        Serial.println("Publish message to hum1");
 client.publish("hum1", hum1char);
         Serial.println("Publish message to temp1");
 client.publish("temp1", temp1char);
 
/*
String  temp_str;
char temp1char[6];
String  hum_str;
char hum1char[6];
char humiditythchar[6],pumptimechar[20],forcestartchar[6];
String forcestart_str, pumptime_str, humidityth_str;
serra variables
int humidity1 = 900;
int temperature1 = 22; 
control variables for nano
int forcestart = 0;
int pumptime = 270000;
int humidityth = 400;
*/
}


if (millis()-prev_send>=5*1000){
  //can_read(0x002);
Serial.println("prev_send");
         Serial.println(prev_send);
         Serial.println("millis()-prev_send");
         Serial.println(millis()-prev_send);
        Wire.beginTransmission(2); // transmit to device #2
 I2C_writeAnything(remotedata);              // sends one byte  
  int aux=Wire.endTransmission();    // stop transmitting
        client.publish("telemetry","trasmission aver I2C to nano !!???");
        //client.publish("telemetry",aux);
                 Serial.println("end wire transmission");
                  Serial.println(aux);
         Serial.println("remote data size");
         Serial.println(sizeof(remotedata));

/*
 * byte, which indicates the status of the transmission:

    0:success
    1:data too long to fit in transmit buffer
    2:received NACK on transmit of address
    3:received NACK on transmit of data
    4:other error 
 */

  prev_send=millis();
}
if (millis()-prev_read>=10*1000){
//  can_send(0x02);
 Wire.requestFrom(2, sizeof(sensorsdata));// read from slave 2
         Serial.print("sizeof(sensorsdata)");
        Serial.print(sizeof(sensorsdata));
        Serial.print("Wire.available()");
        Serial.print(Wire.available());
    if (Wire.available()){
    I2C_writeAnything(sensorsdata);
    }
    client.publish("telemetry","received sensors from nano over I2C !!???");
//onceepoch=(mktime(&tm)+sensorsdata.watertick/1000);
//   time_t onceepoch_tm=(time_t)onceepoch;
//    localtime_r(&onceepoch_tm, &tmonce);
if (sensorsdata.watertick!=0){
  epocheau= (mktime(&epochstart_tm)+sensorsdata.watertick/1000);
  epocheau_tm=(time_t)epocheau;
      localtime_r(&epocheau_tm, &tmepocheeau);           // update the structure tm with the current time
      Serial.print("Convertin epocheau to date time");
    Serial.print(tmepocheeau.tm_year+1900);
    Serial.print(" ");
    Serial.print(tmepocheeau.tm_mon+1);
    Serial.print(" ");
    Serial.print(tmepocheeau.tm_mday);
    Serial.print(" ");
    Serial.print(tmepocheeau.tm_hour);
    Serial.print(F(":"));
    Serial.print(tmepocheeau.tm_min);
    Serial.print(F(":"));
    Serial.print(tmepocheeau.tm_sec);

      client.publish("telemetry","received sensors from nano over I2C !!???");
    
    epocheau_str=(String) asctime(&tmepocheeau);
    epocheau_str[epocheau_str.length()] = '\0';// Make payload a string by NULL terminating it.
    epocheau_str.toCharArray(epocheauchar, epocheau_str.length() + 1);//packaging up the data to publish to mqtt whoa...
    client.publish("telemetry","epocheauchar date/time");
        client.publish("lastactive",epocheauchar);
}
else{
        Serial.println("Last watering: never (sensorsdata.watertick==0)");
    client.publish("telemetry","epocheauchar date/time: never (sensorsdata.watertick==0)");

}

    prev_read=millis();

}
//  Serial.println("i2c_size_esp");
//       Serial.println(i2c_size_esp);
//  Serial.println("i2c_size_nano");
//    Serial.println(i2c_size_nano);
//i2c_scan();
}
