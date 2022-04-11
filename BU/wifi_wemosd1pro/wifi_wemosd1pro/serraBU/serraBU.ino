//#include <WiFiUdp.h>
//#include <NTPClient.h>
#include <SPI.h> //Library for using SPI Communication
#include <mcp2515.h> //Library for using CAN Communication
struct can_frame canMsg;
MCP2515 mcp2515(15);//D8=15
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval
#include <coredecls.h>                  // settimeofday_cb()

const int MY_INT_PIN = 16;
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
//String formattedTime = timeClient.getFormattedTime();
//  Serial.print("Formatted Time: ");
//  Serial.println(formattedTime);  
//  int currentHour = timeClient.getHours();
 
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  short monthDay;
  short currentMonth;
  short currentYear;
  short currentHour;
  short currentMinute;
  short currentSecond; 
  short weekDay;
  short curday;
  short curmonth;
  short hourfrommid;

  unsigned long prev_read=0;
unsigned long prev_send=0;
  
const char* ntpServer = "europe.pool.ntp.org";
const long  gmtOffset_sec = 3600;   //Replace with your GMT offset (seconds)
const int   daylightOffset_sec = 0;  //Replace with your daylight offset (seconds)
char timeinfo[24];
char* datetimeaux;//var to pubblish date
// Global variables for Time
  time_t rawtime;// global holding current datetime as Epoch 
//serra variables
int humidity1 = 900;
int temperature1 = 22;
unsigned long watertick=0;

String  temp_str;
char temp1[6];
String  hum_str;
char hum1[6];

//control variables for nano
bool forcestart=false;
int pumptime=270000;
short humidityth=400;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
long tlastwater;

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
    Serial.println("\nWaiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }  
  printLocalTime();
}

void printLocalTime()
{
    struct tm *timeinfo;// pointer to a tm struct;
  time (&rawtime);
  timeinfo = localtime (&rawtime);
 Serial.println(asctime(timeinfo));
  delay(1000);
  datetimeaux=asctime(timeinfo);
  Serial.println("Ora attuale:");
  int Hour=timeinfo->tm_hour;
  //if (timeinfo->tm_min==8){
  //Serial.println("Minuto attuale:");
    Serial.println("trying to send H");
  Serial.println(timeinfo->tm_min);
  Serial.println(Hour);
}

ICACHE_RAM_ATTR void my_isr(){
  Serial.println("Interrupt Detected");
}

void setup() {
    Serial.begin(9600);
delay(100);
  //pinMode(interruptPin, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, CHANGE);
  pinMode(MY_INT_PIN, INPUT_PULLUP);
  attachInterrupt(MY_INT_PIN, my_isr, CHANGE);
delay(100);
setup_wifi();
  setup_time();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  //SPI.begin(); //Begins SPI communication
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ); //Sets CAN at speed 500KBPS and Clock 8MHz
  mcp2515.setNormalMode();
} 

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
/*
const char topic[]  = "hum1";//real unique topic
const char topic2[]  = "temp1";
const char topic3[]  = "lastactive";
*/

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

if (millis()-prev_send>=5*1000){
  can_read(0x002);
Serial.println("prev_send");
         Serial.println(prev_send);
         Serial.println("millis()-prev_send");
         Serial.println(millis()-prev_send);
  prev_send=millis();
}
if (millis()-prev_read>=10*1000){
  can_send(0x02);
    prev_read=millis();

}
}
