/*
 * ESP32 Chip model = ESP32-D0WDQ6 Rev 3
00:08:19.394 -> This chip has 2 cores
00:08:19.394 -> Chip ID: 2292260
*/
#include <stdio.h>
//#define getName(var)  #var
# define getName(var, str)  sprintf(str, "%s", #var)

#include <Preferences.h>
Preferences preferences;
//typedef unsigned char byte;
//Configuration of NTP
#define MY_NTP_SERVER "europe.pool.ntp.org"          
#define MY_TZ "CET-1CEST,M3.5.0,M10.5.0/3"
/*
+----------+-------------+-----------------+
|          | attenuation | suggested range |
|    SoC   |     (dB)    |      (mV)       |
+==========+=============+=================+
|          |       0     |    100 ~  950   |
|          +-------------+-----------------+
|          |       2.5   |    100 ~ 1250   |
|   ESP32  +-------------+-----------------+
|          |       6     |    150 ~ 1750   |
|          +-------------+-----------------+
|          |      11     |    150 ~ 2450   |
+----------+-------------+-----------------+
|          |       0     |      0 ~  750   |
|          +-------------+-----------------+
|          |       2.5   |      0 ~ 1050   |
| ESP32-S2 +-------------+-----------------+
|          |       6     |      0 ~ 1300   |
|          +-------------+-----------------+
|          |      11     |      0 ~ 2500   |
+----------+-------------+-----------------+
attenuazione di 0dB che permette di avere una tensione di fondo scala pari a 1.1V
attenuazione di 2.5dB che permette di avere una tensione di fondo scala pari a 1.5V
attenuazione di  6dB che permette di avere una tensione di fondo scala pari a 2.2V
attenuazione di  11dB che permette di avere una tensione di fondo scala pari a 3.9V
*/
//18650 4.2-2.7
//lead acid 12.7-11.6
#define lead70Ah_ANALOG_IN A7//D35
#define battery18650_ANALOG_IN A6//D34
//resistor lead battery voltage sensor
// Floats for ADC voltage & Input voltage
float adc_voltage_lead = 0.0;
float adc_voltage_18650 = 0.0;
float in_voltage_lead = 0.0;
float in_voltage_18650 = 0.0;
// Float for Reference Voltage
volatile float ref_voltage_18650 = 1.042;
volatile float ref_voltage_lead = 3.388;
// Floats for resistor values in divider (in ohms)
float R1_lead = 29970.0;
float R2_lead = 7510.0;
float R1_18650 = 29970.0;
float R2_18650 = 7510.0;  
// Integer for ADC value
int adc_value_lead = 0;
int adc_value_18650 = 0;
//tuned ref Voltage
float Vref_18650_tune;
float Vref_lead_tune;

//#include <math.h>
//#include <stdio.h>
#include <driver/adc.h>
#define __STDC_WANT_LIB_EXT1__ 1
//#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#define MQTT_KEEP_ALIVE 5 // int, in seconds
#define MQTT_CLEAN_SESSION false // bool, resuse existing session
#define MQTT_TIMEOUT 15000 // int, in miliseconds
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval
#include <OneWire.h>
#include <DallasTemperature.h>
// Data wire is plugged into pin 7 on the Arduino
#define ONE_WIRE_BUS 4

DeviceAddress INT_TEMP = { 0x28, 0x8E, 0x28, 0x95, 0xF0, 0x1, 0x3C, 0x15};
DeviceAddress SERRA_TEMP = { 0x28, 0xC5, 0xFA, 0xC, 0x5F, 0x20, 0x1, 0xD8};
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

int humpin = 39; //read humidity sensor
unsigned short RelayWaterControll = 25;
unsigned short RelayValveControll = 26;

int32_t conchannel;
const char* ssid = "Vodafoneebeb";
const char* password = "1lUB4jV1pdCCczvNdMyOvQQK";
const char* mqtt_server = "mqtt.flespi.io";
const int mqttPort = 1883;
const char* mqttUser = "iCF53tOTdMwPvvCQFdjFI0YaagAoIVjQlOJe5nV7XhMgAdX5g6AOVmOTsc7YVgF6";
const char* mqttPassword = "";
const char* clientId = "212.serra.abete.ruf";
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
time_t now_tt,epocheau_tt;
struct tm epochstart_tm;               // this is the epoch
struct tm tm;                              // the structure tm holds time information in a more convient way
//struct tm timeinfo;//,tmepocheeau;
struct tm tmepocheeau;
unsigned long epocheau;//internal_tick;
const char* ntpServer = "europe.pool.ntp.org";
const long  gmtOffset_sec = +3600;   //Replace with your GMT offset (seconds)
const int   daylightOffset_sec = 0;  //Replace with your daylight offset (seconds)
//char timeinfo[24];
//char datetimeaux[30];//var to pubblish date
// Global variables for Time
// time_t rawtime;// global holding current datetime as Epoch

char const *stringformat;
//String  hum_str,temp_str,watertick_str, forcestart_str, pumptime_str, humidityth_str,epochstart_str,epocheau_str, measured_18650_str, measured_lead_str;
//char temp1char[6];
//char hum1char[6];
//char watertickchar[10];
char *watertickchar;
//char forcestartchar[6];
//char pumptimechar[20],humiditythchar[6],measured_18650char[5],measured_leadchar[6];
char *pumptimechar,*humiditythchar,*measured_18650char,*measured_leadchar,*temp1char,*hum1char, *epocheauchar;
//char epochstartchar[30];
char *epochstartchar;
//char epocheauchar[30];

unsigned short WATERING_HOUR=14;
bool EMERGENCY_STOP=false;
bool wateringon;// = ((digitalRead(RelayWaterControl1) == LOW) || (digitalRead(RelayWaterControl1) == 0));
unsigned short WATERING_HOUR_range=6;
//control variables for nano
//bool forcestart=false;
//int pumptime=270000;
//short humidityth=400;
struct remotedata_STRUCT
{
//control var from wemos
  volatile bool forcestart = false;
  volatile bool external_pump = false;
  volatile unsigned long pumptime = 300*1000;
  volatile unsigned short humidityth = 1700;
  volatile float measured_18650 = 4.2;
  volatile float measured_lead = 12.7;
} remotedata;
struct sensorsdata_STRUCT
{
//sensor variables
unsigned short hum1=3000;  // variable to store the value read
short temp1 = 59;  // variable to store the value read
short temp1_min = 2000;
short temp1_max = 500;
unsigned long watertick=0;
char lastactive[25];
} sensorsdata;
const unsigned long MAX_PUMP_TIME=30*60*1000;
bool force_switchoff,force_switchon;
unsigned short hour_temp1_max=14;
unsigned short hour_temp1_min=5;
unsigned short Hour;
unsigned short humM,humm;
unsigned short upper_temp1th=2500;//centigradi *100
unsigned short lower_temp1th=1000;//centigradi *100
unsigned short tempint = 1500;
unsigned short startcounter=0;

//Variable for hum1, temp1, time history
time_t *time_hist;
short *temp1_hist;
unsigned short *hum1_hist;

//string def for topic identification
const char* topic1="forcestart";
const char* topic2="pumptime";
const char* topic3="humidityth";
const char* topic4="externalpump";
const char* topic5="Vref18650tune";
const char* topic6="Vrefleadtune";

//variables for timing controll counting
unsigned short hum1_mcount=0;
unsigned long lastMsg = 0;
unsigned long LastVolt=0,LastSensors=0;
unsigned long prev_read=0;
unsigned long prev_send=0;
unsigned short bootCount = 0;

WiFiClient espClient;
PubSubClient client(espClient);

//sleep controll
unsigned long sleep_timer=0;
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  30*60       /* Time ESP32 will go to sleep (in seconds) */

template <typename T> void load_head(T *arr,T val){
  size_t size = sizeof(arr)/sizeof(arr[0]);
  for (int i=0;i<size;i++){
  arr[size-i]=arr[size-i-1];
  }
  arr[0]=val;
}
const unsigned short hist_index_max=10;
void hist_var(time_t *arr_time,time_t val_t,unsigned short *arr_hum1,unsigned short val_hum1,short *arr_temp1,short val_temp1){
  size_t size = sizeof(arr_time)/sizeof(arr_time[0]);
  if(size<hist_index_max){
  byte *p =(byte*) malloc(sizeof(time_t)*hist_index_max);
  memcpy(p,arr_time,size);
  free(p);
  size = sizeof(arr_hum1)/sizeof(arr_hum1[0]);
  // if(size<hist_index_max){
  p =(byte*) malloc(sizeof(unsigned short)*hist_index_max);
  memcpy(p,arr_hum1,size);
  free(p);
  // size = sizeof(arr_temp1)/sizeof(arr_temp1[0]);
  // if(size<hist_index_max){
  p =(byte*) malloc(sizeof(short)*hist_index_max);
  memcpy(p,arr_temp1,size);
  free(p);
  }
  load_head(arr_time,val_t);
  load_head(arr_hum1,val_hum1);
  load_head(arr_temp1,val_temp1);
}

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason){
  case ESP_SLEEP_WAKEUP_EXT0 : Serial.println(F("Wakeup caused by external signal using RTC_IO")); break;
  case ESP_SLEEP_WAKEUP_EXT1 : Serial.println(F("Wakeup caused by external signal using RTC_CNTL")); break;
  case ESP_SLEEP_WAKEUP_TIMER : Serial.println(F("Wakeup caused by timer")); break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println(F("Wakeup caused by touchpad")); break;
  case ESP_SLEEP_WAKEUP_ULP : Serial.println(F("Wakeup caused by ULP program")); break;
  default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void setup_wifi() {
  WiFi.mode(WIFI_STA);
  //delay(500);
  for (unsigned long tmp=millis();millis()-tmp<=5*100;){}
  //search for best bssid
  byte available_networks = WiFi.scanNetworks();
  Serial.println(F("Available network"));
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
  Serial.println(F("BSSID"));
  Serial.println(bssidstr);
  Serial.println(F("channel"));
  Serial.println(conchannel);
  for (int network = 0; network < available_networks; network++) {
  if (WiFi.SSID(network) == ssid) {
  Serial.print(F("Found one: "));
  Serial.println (WiFi.RSSI(network));
  if ((uint8_t)WiFi.RSSI(network) > prevRssi) {
  netnum = network;
  prevRssi = (uint8_t)WiFi.RSSI(network);
  bssid = WiFi.BSSID(network);
  conchannel=WiFi.channel(network);
  bssidstr=WiFi.BSSIDstr(network);
  Serial.print(F("Connecting to bssid"));
  Serial.print(bssidstr);
  Serial.print(prevRssi);
  }
  }
  }
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print(F("Connecting to "));
  Serial.print(ssid);
  Serial.println();
  Serial.println(F("Connecting to bssid"));
  Serial.println(bssidstr);
  WiFi.begin(ssid,password,conchannel,bssid);
  Serial.println(WiFi.status());
  WiFi.printDiag(Serial);
  while (WiFi.status() != WL_CONNECTED) {
  for (unsigned long tmp=millis();millis()-tmp<=1*1000;){}
  //delay(1000);
  Serial.print(F("."));
  }
  //randomSeed(micros());
  Serial.println(F(""));
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());
}

void array_to_string(byte array[], unsigned int len, char buffer[]){
  for (unsigned int i = 0; i < len; i++){
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
    buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}

void debugging(char *what){
 char str[20];
   getName(*what, str);
            Serial.println(F(str));
    Serial.println(what);
      client.publish("telemetry",str);
      client.publish("telemetry",what);
}
void debugging(int what){
 char str[20];
const char *format="%d";
 char *mychar;
 mychar=i2chara(format,what);
   getName(what,str);
            Serial.println(F(str));
    Serial.println(what);
      client.publish("telemetry",str);
      client.publish("telemetry",mychar);
}
void debugging(unsigned short what){
 char str[20];
const char *format="%hu";
 char *mychar;
 mychar=us2chara(format,what);
   getName(what,str);
            Serial.println(F(str));
    Serial.println(what);
      client.publish("telemetry",str);
      client.publish("telemetry",mychar);
}
void debugging(float what){
 char str[20];
const char *format="%6.3f";
 char *mychar;
 mychar=f2chara(format,what);
   getName(what,str);
            Serial.println(F(str));
    Serial.println(what);
      client.publish("telemetry",str);
      client.publish("telemetry",mychar);
}
void debugging(unsigned long what){
 char str[20];
const char *format="%lu";
 char *mychar;
 mychar=us2chara(format,what);
   getName(what,str);
            Serial.println(F(str));
    Serial.println(what);
      client.publish("telemetry",str);
      client.publish("telemetry",mychar);
}
void debugging(short what){
 char str[20];
const char *format="%hi";
 char *mychar;
 mychar=s2chara(format,what);
   getName(what,str);
            Serial.println(F(str));
    Serial.println(what);
      client.publish("telemetry",str);
      client.publish("telemetry",mychar);
}
void debugging(bool what){
 char str[20];
const char *format="%s";
 char *mychar;
const char *vOut = what ? "true" : "false";
 mychar=str2chara(format,vOut);
   getName(what,str);
            Serial.println(F(str));
    Serial.println(what);
      client.publish("telemetry",str);
      client.publish("telemetry",mychar);
}
void debugging(size_t what){
const char *format="%hu";
char str[20];
 char *mychar;
 mychar=ul2chara(format,(unsigned long)what);
   getName(what,str);
    Serial.println(str);
        Serial.println(what);
      client.publish("telemetry",str);
      client.publish("telemetry",mychar);
}
void debugging(const char dbgmsg[50]){
            Serial.println(F(dbgmsg));
      client.publish("telemetry",dbgmsg);
}
void callback(char* topic,byte* payload,unsigned int length) {
  Serial.println(F("received mqtt payload"));
  unsigned long tmp=0;
  // volatile double intpayload=0;
  size_t sz;
  //byte * payload_str;
  //char *payloadchar;
  Serial.print(F("Message arrived ["));
  Serial.print(topic);
  Serial.print(F("] "));
  Serial.println(F("strncmp(topic, topic1, strlen(topic1))"));
  int aux=(strncmp(topic, topic1, strlen(topic1))==0);
 //debugging(aux);
  //Serial.println("line 185");
  // payload_str = payload; // Make payload a string by NULL terminating it.
  //           Serial.println("line 187");
  //          Serial.print(F("payload_str"));
  //for (byte i = 0; i < payload.uid.size; i++) {
  // Serial.print(payload.uid.uidByte[i] < 0x10 ? " 0" : " ");
  // Serial.print(payload.uid.uidByte[i], HEX);
  // }
  //payload_str[length]='\0';
  //payload_str=(char *)payload_str;
  // payload_str.toCharArray(payloadchar, payload_str.length()); //packaging up the data to publish to mqtt whoa...
  //     Serial.println("line 190");
  // if (payload_str[0]=='t'||payloadchar[0]=='f'){
  //intpayload = atoi((char *)payload);
  //intpayload = atof((char *)payload);
  // Serial.println("atoi((char *)payload)"); 
  // Allocate the correct amount of memory for the payload copy
  //byte* p = (byte*)malloc(length);
  // Copy the payload to the new buffer
//  memcpy(p,payload,length);
  //client.publish("outTopic", p, length);
  // Free the memory
  // char pchar[20];
  //array_to_string(p,length,pchar);
  //  strcpy(pchar,(char*)p);
  char pchar[length + 1];
  // copy contents of payload to message
  memcpy(pchar, payload, length);
  // add NULL terminator to message, making it a correct c string
  pchar[length + 1] = '\0';
  // use string functions with message
  // e.g
  int messageLength = strlen(pchar);
        free(payload);
debugging(pchar);
  //String myString = String((char *)byteArray);
  if (aux==1){
  debugging("Arduino think Received message on topic forcestart");
  //     client.publish("telemetry","Arduino think Received message on topic forcestart");
  //strncpy(arr, (char*)p, sizeof(arr));
  //arr[sizeof(arr)-1]=NULL;
  //Serial.println("RAW payload");
  //     Serial.println(arr);
  //   Serial.println("-- RAW payload --");
    //strncpy(pchar, char(*p), sizeof(pchar));
  //pchar[sizeof(pchar)-1]=NULL;
  if (atoi(pchar)==1){
  Serial.println(F("payload"));
  Serial.println(atoi(pchar));
  debugging(atoi(pchar));
  if (remotedata.forcestart==false){
    force_switchon=true; debugging("force_switchon=true");//Serial.println(F("force_switchon=true"));client.publish("telemetry","force_switchon=true");
    }
  else{
    force_switchon=false; debugging("force_switchon=false");//Serial.println(F("force_switchon=false"));client.publish("telemetry","force_switchon=false");
    }
    remotedata.forcestart=true;
  }
 else{
  if (atoi(pchar)==0){
      if (remotedata.forcestart==true){force_switchoff=true;debugging("force_switchoff=true");//Serial.println(F("force_switchoff=true"));client.publish("telemetry","force_switchoff=true");
      }
      else{
  force_switchoff=false;debugging("force_switchoff=false");//Serial.println(F("force_switchoff=false"));client.publish("telemetry","force_switchoff=false");
  }
  remotedata.forcestart=false;
  debugging("setting remotedata.forcestart=false");
  }
  else{
debugging("wrong string from forcestart payload");
  }
  }
  }
  Serial.println(F("strncmp(topic, topic2, strlen(topic2))"));
  aux=(strncmp(topic, topic2, strlen(topic2))==0);
  tmp=tmp+aux;
  //Serial.println(aux);
  if (aux==1){
    debugging("Arduino think Received message on topic pumptime");
  sz = snprintf(NULL, 0, pchar);
  debugging(sz);
  remotedata.pumptime=strtoul(pchar,NULL,10);
  debugging(remotedata.pumptime);
  //pumptime_str[pumptime_str.length()] = '\0'; // Make payload a string by NULL terminating it.
  //pumptime_str.toCharArray(pumptimechar, pumptime_str.length() + 1); //packaging up the data to publish to mqtt whoa...
  stringformat="%lu";
  pumptimechar=ul2chara(stringformat,remotedata.pumptime);
  //client.publish("telemetry","message on topic pumptime=");
  //client.publish("telemetry",pumptimechar);
  debugging(pumptimechar);
  }
  Serial.println(F("strncmp(topic, topic3, strlen(topic3))"));
  aux=(strncmp(topic, topic3, strlen(topic3))==0);
  tmp=tmp+aux;
  //Serial.println(aux);
  if (aux==1){
    debugging("Arduino think Received message on topic humidityth");
  //client.publish("telemetry","Arduino think Received message on topic humidityth");
  sz = snprintf(NULL, 0, pchar);
  debugging(sz);
  //Serial.println(F("sz = snprintf(NULL, 0, (char*)p)"));
  //Serial.println(sz);
  remotedata.humidityth=(unsigned short)strtoul(pchar,NULL,10);
  //debugging(remotedata.humidityth);
  //Serial.println(F("remotedata.humidityth changed to"));
  //Serial.println(remotedata.humidityth);
  // humidityth_str=(String) intpayload;
  // humidityth_str[humidityth_str.length()] = '\0'; // Make payload a string by NULL terminating it.
  // humidityth_str.toCharArray(humiditythchar, humidityth_str.length() + 1); //packaging up the data to publish to mqtt whoa...
  stringformat="%hu";
  humiditythchar=us2chara(stringformat,remotedata.humidityth);
  debugging(humiditythchar);
  /*client.publish("telemetry","Message on topic humidityth=");
  client.publish("telemetry",humiditythchar);
  Serial.println("humiditythchar");
  Serial.println(humiditythchar);*/
  }
  //}
  Serial.println(F("strncmp(topic, topic4, strlen(topic4))"));
  aux=(strncmp(topic, topic4, strlen(topic4))==0);
  tmp=tmp+aux;
  //Serial.println(F("aux4"));
  //Serial.println(aux);
  if (aux==1){
    debugging("Arduino think Received message on topic external_pump");
 // client.publish("telemetry","Arduino think Received message on topic external_pump");
  //                                 client.publish("telemetry","intpayload");
  //                                      client.publish("telemetry",intpayload);
  //                                     Serial.println("payload_str");
  // Serial.println((char *)payload_str);
  Serial.println(F("topic external pump :waiting understand payload"));//
  debugging("topic external pump :waiting understand payload");
  if (atoi(pchar)==1){
  //Serial.println("intpayload");
  // Serial.println(intpayload);
  remotedata.external_pump=true;
  //Serial.println(F("external_pump set"));          //
  //client.publish("telemetry","external_pump set");
  debugging("external_pump set");
  digitalWrite(RelayValveControll, LOW);
  }
  else{
  // Serial.println("intpayload");
  //Serial.println(intpayload);
  remotedata.external_pump=false;
  debugging("external_pump unset");
  //Serial.println(F("external_pump unset"));          //
  //client.publish("telemetry","external_pump unset");
  digitalWrite(RelayValveControll, HIGH);
  }
  }  
  Serial.println(F("strncmp(topic, topic5, strlen(topic5))"));
  aux=(strncmp(topic, topic5, strlen(topic5))==0);
  tmp=tmp+aux;
  //Serial.println(aux);
  if (aux==1){
  //client.publish("telemetry","Arduino think Received message on topic measured_18650");
  debugging("Arduino think Received message on topic measured_18650");
  remotedata.measured_18650 = strtof(pchar, NULL);
  Serial.println(F("remotedata.measured_18650"));
  Serial.println(remotedata.measured_18650);
  stringformat="%.3f";
  measured_18650char=f2chara("%.3f",remotedata.measured_18650);
  debugging(measured_18650char);
  //measured_18650_str=(String) intpayload;
  //measured_18650_str[measured_18650_str.length()] = '\0'; // Make payload a string by NULL terminating it.
  //measured_18650_str.toCharArray(measured_18650char, measured_18650_str.length() + 1); //packaging up the data to publish to mqtt whoa... 
  //client.publish("telemetry","Arduino think Received message on topic measured_18650=");
  //client.publish("telemetry",measured_18650char);
  }  
  Serial.println(F("strncmp(topic, topic6, strlen(topic6))"));
  aux=(strncmp(topic, topic6, strlen(topic6))==0);
  tmp=tmp+aux;
  //Serial.println(aux);
  if (aux==1){
  //client.publish("telemetry","Arduino think Received message on topic measured_lead");
  debugging("Arduino think Received message on topic measured_lead");
  remotedata.measured_lead =  strtof(pchar,NULL);
  Serial.println(F("remotedata.measured_lead"));
  Serial.println(remotedata.measured_lead);
  stringformat="%.3f";
  measured_leadchar=f2chara(stringformat,remotedata.measured_lead);
  //measured_lead_str=(String) intpayload;
  //measured_lead_str[measured_18650_str.length()] = '\0'; // Make payload a string by NULL terminating it.
  //measured_lead_str.toCharArray(measured_leadchar, measured_lead_str.length() + 1); //packaging up the data to publish to mqtt whoa...
  //client.publish("telemetry","Arduino think Received message on topic measured_leadchar=");
  //client.publish("telemetry",measured_leadchar);
  debugging(measured_leadchar);
  } 
  //char *topiclist = { "forcestart", "pumptime", "humidityth"};
  if (tmp==1)
  {
    debugging("topic recognised");
  //Serial.println(F("topic recognised"));
  }
  else
  {    
    debugging("topic not recognised or more than one");
    //Serial.println(F("topic not recognised or more than one"));
  }
  //free(p);
}

/* Possible values for client.state()
 MQTT_CONNECTION_TIMEOUT     -4
 MQTT_CONNECTION_LOST        -3
 MQTT_CONNECT_FAILED         -2
 MQTT_DISCONNECTED           -1
 MQTT_CONNECTED               0
 MQTT_CONNECT_BAD_PROTOCOL    1
 MQTT_CONNECT_BAD_CLIENT_ID   2
 MQTT_CONNECT_UNAVAILABLE     3
 MQTT_CONNECT_BAD_CREDENTIALS 4
 MQTT_CONNECT_UNAUTHORIZED    5
*/
//MQTT broker reconnection
void reconnect() {
  // Loop until we're reconnected
  short mqtt_rc=0;
 // String mqtt_rcstr;
  char *mqtt_rcchr;
  while (!client.connected()) {
  mqtt_rc=client.state();
  Serial.println(client.state());
  Serial.print(F("Attempting MQTT connection..."));
  // Create a random client ID
  //clientId += String(random(0xffff), HEX);
  // Attempt to connect
  if (client.connect(clientId,mqttUser,"")) {
  Serial.println(F("connected"));
  // Once connected, publish an announcement...
  client.publish("telemetry", "hello world");
  client.subscribe("forcestart");
  client.subscribe("pumptime");
  client.subscribe("humidityth");
  client.subscribe("externalpump");
  client.subscribe("Vref18650tune");
  client.subscribe("Vrefleadtune");
  }
  }
  //mqtt_rcstr=(String) mqtt_rc;
  //mqtt_rcstr[mqtt_rcstr.length()] = '\0';// Make payload a string by NULL terminating it.
  //mqtt_rcstr.toCharArray(mqtt_rcchr, mqtt_rcstr.length() + 1);//packaging up
  stringformat="%hi";
  mqtt_rcchr=s2chara(stringformat,mqtt_rc);
  //client.publish("telemetry", "mqtt disconnected due to rc=");
  //client.publish("telemetry", mqtt_rcchr);
debugging(mqtt_rcchr);
}

void setup_time () {
  //init and get the time
  //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println(F("\nNTP TZ DST - bare minimum"));
  configTime(0, 0, MY_NTP_SERVER); // 0, 0 because we will use TZ in the next line
  setenv("TZ", MY_TZ, 1); // Set environment variable with your time zone
  tzset();
  Serial.println(F("\nWaiting for time"));
  while (!time(nullptr)) {
  Serial.print(F("."));
  //delay(1000);
  for (unsigned long tmp=millis();millis()-tmp<=1*1000;){}
  }
  printLocalTime();
}

void printLocalTime(){
  char *time_chr;
  time(&now_tt);
  localtime_r(&now_tt,&tm);
  Serial.printf("The current date/time is: %s", asctime(&tm));
  debugging("The current date/time is:");
  stringformat="%s";
  time_chr=str2chara(stringformat,asctime(&tm));
  debugging(time_chr);
  Hour=tm.tm_hour;
  //  Serial.println(tm.tm_min);
  }
   
void setup() {
  Serial.begin(115200);
  while (!Serial){}
  Serial.println(F("serial init"));
  // Open Preferences with my-app namespace. Each application module, library, etc
  // has to use a namespace name to prevent key name collisions. We will open storage in
  // RW-mode (second parameter has to be false).
  // Note: Namespace name is limited to 15 chars.
  
  bootCount++;
  Serial.println("Boot number: " + String(bootCount));
  //Print the wakeup reason for ESP32
  print_wakeup_reason();
  /*
  First we configure the wake up source
  We set our ESP32 to wake up every 5 seconds
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");
  //humM=analogRead(humpin);
  //humm=humM;
  const unsigned long recent_epoch=1648248186;//epoch unix 25/03/2022 23.43
  setup_wifi();
  Serial.println(F("wifi init"));
  setup_time();
  Serial.println(F("time setup"));
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Serial.println(F("MQTT init"));
  while(mktime(&epochstart_tm)<recent_epoch){
  time(&now_tt);
  localtime_r(&now_tt,&epochstart_tm);
  Serial.println(F("setting epochstart"));
  //delay(1000);
  for (unsigned long tmp=millis();millis()-tmp<=1*1000;){}
  }
  client.publish("telemetry","epochstart");
  epochstartchar=str2chara("%s",asctime(&epochstart_tm));
  //epochstart_str = (String) asctime(&epochstart_tm); //converting ftemp (the float variable above) to a string
  //epochstart_str[epochstart_str.length()] = '\0';
  //epochstart_str.toCharArray(epochstartchar, epochstart_str.length()); //packaging up the data to publish to mqtt whoa...
  //    char* aux=asctime(epochstart_tm);
  debugging(epochstartchar);
  //client.publish("telemetry", epochstartchar);
  //Serial.println(F("Dallas Temperature IC Control Library Demo"));
  debugging("Dallas Temperature IC Control Library Demo");
  // Start up the library
  sensors.begin();
  Serial.println(F("Settin RelayWaterControl1 to output"));
  pinMode(RelayWaterControll, OUTPUT);
  digitalWrite(RelayWaterControll, HIGH);
  Serial.println(F("Settin RelayValveControl1 to output"));
  pinMode(RelayValveControll, OUTPUT);
  digitalWrite(RelayValveControll, HIGH);
  //           digitalWrite(RelayValveControll, LOW);
  preferences.begin("remotedata", false);
  // Remove all preferences under the opened namespace
  //preferences.clear();
  // Or remove the counter key only
  //preferences.remove("counter");
  //testing valve connection
  // Note: Key name is limited to 15 chars.
  /*
  typedef struct {
  time_t time_hist_disk;
  unsigned short hum1_hist_disk;
  short temp1_hist_disk;
  } hist_var_my;
  */
  remotedata.forcestart = preferences.getBool("forcestart", false);
  EMERGENCY_STOP = preferences.getBool("EMERGENCY_STOP", false);
  ref_voltage_18650 =preferences.getFloat("ref_voltage_18650", 1.042);
  ref_voltage_lead =preferences.getFloat("ref_voltage_lead", 3.388);
  remotedata.external_pump=preferences.getBool("external_pump", false);
  remotedata.pumptime = preferences.getULong("pumptime", 270000);
  remotedata.humidityth = preferences.getUShort("humidityth", 1600);
  //sensorsdata.watertick = preferences.getULong("watertick", 0);
  String tmp=preferences.getString("lastactive", "never");
  //int tmp_l = tmp.length();
  strcpy(sensorsdata.lastactive, tmp.c_str());
  preferences.getBytes("time_hist", &time_hist, sizeof(time_hist));
  preferences.getBytes("hum1_hist", &hum1_hist, sizeof(hum1_hist));
  preferences.getBytes("temp1_hist", &temp1_hist, sizeof(temp1_hist));
  preferences.end();
  size_t sizeof_time_hist=sizeof(time_hist);
  debugging(sizeof_time_hist);
    size_t sizeof_hum1_hist=sizeof(hum1_hist);
  debugging(sizeof_hum1_hist);
      size_t sizeof_temp1_hist=sizeof(temp1_hist);
  debugging(sizeof_temp1_hist);
  adcAttachPin(humpin);//Attach a pin to ADC (also clears any other analog mode that could be on). Returns TRUE or FALSE result.
  sensors_reading();
}

void water_on(){
  strncpy(sensorsdata.lastactive, asctime(&tm), 25);
  //Serial.println(F("sensorsdata.lastactive"));
  //Serial.println(sensorsdata.lastactive);
  debugging(sensorsdata.lastactive);
  sensorsdata.lastactive[sizeof(sensorsdata.lastactive) - 1] = 0;
  digitalWrite(RelayValveControll, LOW);
  //delay(15*1000);
  for (unsigned long tmp=millis();millis()-tmp<=15*1000;){}
  digitalWrite(RelayWaterControll, LOW);
  startcounter+=1;
  //if (!wateringon){
  sensorsdata.watertick=millis();
  //}
}

void water_off(){
  digitalWrite(RelayWaterControll, HIGH);
  for (unsigned long tmp=millis();millis()-tmp<=5*1000;){}
  //  delay(5*1000);
  digitalWrite(RelayValveControll, HIGH);
  //client.publish("telemetry", "watering off");
  //Serial.println(F("watering off!!!"));
  debugging("watering off");
}

void tune_Vref_18650(){
  adc1_config_width(ADC_WIDTH_BIT_12);
  unsigned long somma=0;
  float val;
  adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_0);
  //Serial.println(F("reading 100 samples from adc_6"));
  debugging("reading 100 samples from adc_6");
  for (int i=0;i<100;i++){
  //Serial.println(" adc1_get_raw(ADC1_CHANNEL_6");
  //Serial.println(adc1_get_raw(ADC1_CHANNEL_6));
  somma += adc1_get_raw(ADC1_CHANNEL_6);
  // Serial.println(" somma");
  //Serial.println(somma);
  for (unsigned long tmp=millis();millis()-tmp<=1*100;){}
  //delay(100);
  }
  //Serial.println(" adc1_get_raw(ADC1_CHANNEL_6");
  //Serial.println(adc1_get_raw(ADC1_CHANNEL_6));
  val=somma/(float)100;
  Vref_18650_tune=remotedata.measured_18650/val*4096*(R2_18650/(R1_18650+R2_18650));
  ref_voltage_18650=(float)Vref_18650_tune;
  //client.publish("telemetry","Setting Ref Voltage 18650");
  debugging("Setting Ref Voltage 18650");
}

void tune_Vref_lead(){
  adc1_config_width(ADC_WIDTH_BIT_12);
  unsigned long somma=0;
  float val;
  adc1_config_channel_atten(ADC1_CHANNEL_7,ADC_ATTEN_DB_11);
  Serial.println(F("reading 100 samples from adc_6"));
  for (int i=0;i<100;i++){
  //Serial.println(" adc1_get_raw(ADC1_CHANNEL_7");
  //Serial.println(adc1_get_raw(ADC1_CHANNEL_7));
  somma += adc1_get_raw(ADC1_CHANNEL_7);
  // Serial.println(" somma");
  //Serial.println(somma);
  for (unsigned long tmp=millis();millis()-tmp<=1*100;){}
  // delay(100);
  }
  //Serial.println(" adc1_get_raw(ADC1_CHANNEL_6");
  //Serial.println(adc1_get_raw(ADC1_CHANNEL_6));
  val=somma/(float)100;
  Vref_lead_tune=remotedata.measured_lead/val*4096*(R2_lead/(R1_lead+R2_lead));
  ref_voltage_lead=(float)Vref_lead_tune;
 // client.publish("telemetry","setting Ref Voltage Lead");
  debugging("setting Ref Voltage Lead");
}

void measure_voltages(){
  //Serial.println(F("Measuring Voltage"));
 // client.publish("telemetry","Measure Voltage");
 debugging("Measure Voltage");
  LastVolt=millis();
  adc1_config_width(ADC_WIDTH_BIT_12);
  unsigned long somma=0;
  float val;
  adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_0);
  //Serial.println(F("reading 100 samples from adc_6"));
  debugging("reading 100 samples from adc_6");
  for (int i=0;i<100;i++){
  //Serial.println(" adc1_get_raw(ADC1_CHANNEL_6");
  //Serial.println(adc1_get_raw(ADC1_CHANNEL_6));
  somma += adc1_get_raw(ADC1_CHANNEL_6);
  // Serial.println(" somma");
  //Serial.println(somma);
  for (unsigned long tmp=millis();millis()-tmp<=1*100;){}
  // delay(100);
  }
  //Serial.println(" adc1_get_raw(ADC1_CHANNEL_6");
  //Serial.println(adc1_get_raw(ADC1_CHANNEL_6));
  val=somma/(float)100;
  //Serial.println(F(" val"));
  //Serial.println(val);
  debugging(val);
  float adc_voltage_val = (val * ref_voltage_18650)/4096.0;
  float in_voltage_18650 = adc_voltage_val/(R2_18650/(R1_18650+R2_18650)) ;
  //Serial.println(F(" in_voltage_val_18650"));
  //Serial.println(in_voltage_18650);
  debugging(in_voltage_18650);
  // Read the Analog Input
  //   adc_value_lead = analogRead(lead70Ah_ANALOG_IN);
  //   adc_value_18650 = analogRead(battery18650_ANALOG_IN);
  //Serial.println(" adc_value_18650");
  //Serial.println(adc_value_18650);
  // Determine voltage at ADC input
  //  adc_voltage_lead  = (adc_value_lead * ref_voltage_lead)/4096.0;
  //    adc_voltage_18650 = (adc_value_18650 * ref_voltage_18650)/4096.0;
  //     float adc_voltage_val = (val * ref_voltage)/4096.0;
  // Calculate voltage at divider input
  //   in_voltage_lead = adc_voltage_lead / (R2_lead/(R1_lead+R2_lead)) ;
  //     in_voltage_18650 = adc_voltage_18650/(R2_18650/(R1_18650+R2_18650)) ;
  //float in_voltage_val = adc_voltage_val/(R2_18650/(R1_18650+R2_18650)) ;
  somma=0;
  val=0;
  adc1_config_channel_atten(ADC1_CHANNEL_7,ADC_ATTEN_DB_11);
  //Serial.println(F("reading 100 samples from adc_7"));
  debugging("reading 100 samples from adc_7");
  for (int i=0;i<100;i++){
  //  Serial.println(" adc1_get_raw(ADC1_CHANNEL_6 at 13.4");
  //Serial.println(adc1_get_raw(ADC1_CHANNEL_6));
  somma += adc1_get_raw(ADC1_CHANNEL_7);
  for (unsigned long tmp=millis();millis()-tmp<=1*100;){}
  //delay(100);
  }
  val=somma/(float)100;
  // Determine voltage at ADC input
  adc_voltage_val = (val * ref_voltage_lead)/4096.0;
  // Calculate voltage at divider input
  float in_voltage_lead = adc_voltage_val/(R2_lead/(R1_lead+R2_lead)) ;
  //Serial.println(F(" in_voltage_val_lead"));
  //Serial.println(in_voltage_lead);
  debugging(in_voltage_lead);
  //ftoa(float n, char* res, int afterpoint)
  /*char Volt_str[8];
  ftoa(in_voltage_18650, Volt_str, 2); //converting ftemp (the float variable above) to a string
  client.publish("telemetry","Voltage 18650");
  client.publish("telemetry",Volt_str);
  ftoa(in_voltage_lead, Volt_str, 2); //converting ftemp (the float variable above) to a string
  client.publish("telemetry","Voltage Lead");
  client.publish("telemetry",Volt_str);*/
  //Lead 12.7-11.7
  //18650 4.2-2.7
  if (in_voltage_lead<=11.75){
  //client.publish("telemetry","LEAD BATTERY DISCHARGED!!!");
  debugging("LEAD BATTERY DISCHARGED!!!");
  }
  if (in_voltage_18650<=2.9){
  //client.publish("telemetry","18650 BATTERY DISCHARGED!!!");
  debugging("18650 BATTERY DISCHARGED!!!");
  }
  //temp_str[temp_str.length()] = '\0';
  //temp_str.toCharArray(temp1char, temp_str.length()+1); //packaging up the data to publish to mqtt whoa...
}

void sensors_reading(){
  unsigned short hum1_measures=5;
  hum1_mcount+=1;
  LastSensors=millis();
  //Serial.print(F(" Requesting temperatures..."));
  debugging(" Requesting temperatures...");
  /*
  ROM = 28 8E 28 95 F0 1 3C 15
  ROM = 28 C5 FA C 5F 20 1 D8
  */
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println(F("DONE"));
  /********************************************************************/
  Serial.print(F("Temperatura interna(*C): "));
  Serial.print(sensors.getTempC(INT_TEMP));
  Serial.print(F("Temperatura esterna(*C): "));
  Serial.print(sensors.getTempC(SERRA_TEMP)); // Serial.print(sensors.getTempCByIndex(1)); // Why "byIndex"?  
  //Serial.print("Temperature is: ");
  //Serial.print(sensors.getTempCByIndex(0)); // Why "byIndex"?  
  sensorsdata.temp1=sensors.getTempC(SERRA_TEMP)*100;
  tempint=sensors.getTempC(INT_TEMP)*100;
  // You can have more than one DS18B20 on the same bus.  
  // 0 refers to the first IC on the wire
  //int length = snprintf( NULL, 0, "%d", tempint );
  //char* tempint_str =(char*) malloc( length + 1 );
  //snprintf( tempint_str, length + 1, "%d", tempint );
  //client.publish("telemetry","temperatura interna");
  //client.publish("telemetry",tempint_str);
  debugging(tempint);
  while (analogRead(humpin)==0){
    debugging("waiting hum1 different from 0");
  //Serial.println(F("waiting hum1 different from 0"));          // debug value
  //client.publish("telemetry","waiting hum1 different from 0");
  } 
  for (unsigned short hum1_mcount=0;hum1_mcount<=hum1_measures;hum1_mcount++){ 
  sensorsdata.hum1 += analogRead(humpin);  // read the input pin
  }
  sensorsdata.hum1=sensorsdata.hum1/hum1_measures;
  // temp1 = analogRead(temppin);  // read the input pin
  //Serial.println(F("humidity"));          // debug value
  //Serial.println(sensorsdata.hum1);          // debug value
  debugging(sensorsdata.hum1);
  //     Serial.println("temperature");          // debug value
  // Serial.println(temp1);          // debug value
  //Serial.println(remotedata.forcestart);          // debug value
  //Serial.println(F("sensorsdata.temp1/100"));          // debug value
  //Serial.println(sensorsdata.temp1/100);          // debug value
  debugging("Real T1 multiplied by 100");
  debugging(sensorsdata.temp1);
  if (sensorsdata.temp1<-50*100){
    debugging("Day after tomorrow or SENSOR PROBLEM!!!");
  //Serial.println(F("Day after tomorrow or SENSOR PROBLEM!!!"));       // debug value
  //client.publish("telemetry","Day after tomorrow or SENSOR PROBLEM!!!");
  }
}

// Reverses a string 'str' of length 'len'
void reverse(char* str, int len){
  int i = 0, j = len - 1, temp;
  while (i < j) {
  temp = str[i];
  str[i] = str[j];
  str[j] = temp;
  i++;
  j--;
  }
}

// Converts a given integer x to string str[].
// d is the number of digits required in the output.
// If d is more than the number of digits in x,
// then 0s are added at the beginning.
int intToStr(int x, char str[], int d){
  int i = 0;
  while (x) {
  str[i++] = (x % 10) + '0';
  x = x / 10;
  }
  // If number of digits required is more, then
  // add 0s at the beginning
  while (i < d)str[i++] = '0';
  reverse(str, i);
  str[i] = '\0';
  return i;
}

// Converts a floating-point/double number to a string.
void ftoa(float n, char* res, int afterpoint){
  // Extract integer part
  int ipart = (int)n;
  // Extract floating part
  float fpart = n - (float)ipart;
  // convert integer part to string
  int i = intToStr(ipart, res, 0);
  // check for display option after point
  if (afterpoint != 0) {
  res[i] = '.'; // add dot
  // Get the value of fraction part upto given no.
  // of points after dot. The third parameter
  // is needed to handle cases like 233.007
  fpart = fpart * pow(10, afterpoint);
  intToStr((int)fpart, res + i + 1, afterpoint);
  }
 }

//Pubblish messages to MQTT:
//unsigned long to char array
char* i2chara(const char* A, int B){
  char *buf;
  size_t sz;
  sz = snprintf(NULL, 0, A, B);
  buf = (char *)malloc(sz + 1); /* make sure you check for != NULL in real code */
  snprintf(buf, sz+1, A, B);
  return buf;
}
//unsigned long to char array
char* ul2chara(const char* A, unsigned long B){
  char *buf;
  size_t sz;
  sz = snprintf(NULL, 0, A, B);
  buf = (char *)malloc(sz + 1); /* make sure you check for != NULL in real code */
  snprintf(buf, sz+1, A, B);
  return buf;
}
//float to char array
char* f2chara(const char* A, float B){
  char *buf;
  size_t sz;
  sz = snprintf(NULL, 0, A, B);
  buf = (char *)malloc(sz + 1); /* make sure you check for != NULL in real code */
  snprintf(buf, sz+1, A, B);
  return buf;
}
//unsigned short to char array
char* us2chara(const char *A, unsigned short B){
  char *buf;
  size_t sz;
  sz = snprintf(NULL, 0, A, B);
  buf = (char *)malloc(sz + 1); /* make sure you check for != NULL in real code */
  snprintf(buf, sz+1, A, B);
  return buf;
}
// short to char array
char* s2chara(const char *A, short B){
  char *buf;
  size_t sz;
  sz = snprintf(NULL, 0, A, B);
  buf = (char *)malloc(sz + 1); /* make sure you check for != NULL in real code */
  snprintf(buf, sz+1, A, B);
  return buf;
}
//string to char array
char* str2chara(const char* A, char* B){
  char *buf;
  size_t sz;
  sz = snprintf(NULL, 0, A, B);
  buf = (char *)malloc(sz + 1); /* make sure you check for != NULL in real code */
  snprintf(buf, sz+1, A, B);
  return buf;
}
char* str2chara(const char* A,const char* B){
  char *buf;
  size_t sz;
  sz = snprintf(NULL, 0, A, B);
  buf = (char *)malloc(sz + 1); /* make sure you check for != NULL in real code */
  snprintf(buf, sz+1, A, B);
  return buf;
}
/*
d or i  Signed decimal integer  392
u Unsigned decimal integer  7235
o Unsigned octal  610
x Unsigned hexadecimal integer  7fa
X Unsigned hexadecimal integer (uppercase)  7FA
f Decimal floating point, lowercase 392.65
F Decimal floating point, uppercase 392.65
e Scientific notation (mantissa/exponent), lowercase  3.9265e+2
E Scientific notation (mantissa/exponent), uppercase  3.9265E+2
g Use the shortest representation: %e or %f 392.65
G Use the shortest representation: %E or %F 392.65
a Hexadecimal floating point, lowercase -0xc.90fep-2
A Hexadecimal floating point, uppercase -0XC.90FEP-2
c Character a
s String of characters  sample
p Pointer address b8000000
n Nothing printed.
The corresponding argument must be a pointer to a signed int.
The number of characters written so far is stored in the pointed location.  
% A % followed by another % character will write a single % to the stream.  %
*/

void loop() {
  if (millis()-LastVolt>2*60*1000){
  measure_voltages();
  }
  if (startcounter>=5&&!remotedata.forcestart){
  EMERGENCY_STOP=true;
  }
  wateringon = ((digitalRead(RelayWaterControll) == LOW) && (digitalRead(RelayWaterControll) == LOW));
  printLocalTime();
  short mqtt_rc=client.state();
  Serial.println(F("MQTT client state:"));
  Serial.println(mqtt_rc);
  if (!client.connected()) {
  Serial.println(F("MQTT NOT connected!!"));
  reconnect();
  }
  client.loop();
  /*
  const char topic[]  = "hum1";//real unique topic
  const char topic2[]  = "temp1";
  const char topic3[]  = "lastactive";
  */
  // internal_tick = millis();
  if (millis()-LastSensors>30*1000){
    sensors_reading();
  }
  if (remotedata.forcestart && !remotedata.external_pump && !wateringon && force_switchon){
    //  printLocalTime();
    //sensorsdata.lastactive=*asctime(&tm);
    water_on();
    EMERGENCY_STOP=false;
    debugging("Avvio innaffiatura per comando da remoto");
    //Serial.println(F("Avvio innaffiatura per comando da remoto"));
    //client.publish("telemetry", "Avvio innaffiatura per forcestart=true");
    }
  else{
    if (remotedata.forcestart){
    //  printLocalTime();
    //sensorsdata.lastactive=*asctime(&tm);
    EMERGENCY_STOP=false;
    debugging("Mi assicuro di mettere emergency_stop a false se remotedata.forcestart is true");
    //Serial.println(F("Mi assicuro di mettere emergency_stop a false se remotedata.forcestart is true"));
    //client.publish("telemetry", "Mi assicuro di mettere emergency_stop a false se remotedata.forcestart is true");
  }
  if (wateringon&&(millis()-sensorsdata.watertick)>=MAX_PUMP_TIME){
  water_off();
  if (remotedata.forcestart==1&&!EMERGENCY_STOP){
  remotedata.forcestart=0;
  debugging("setting forcestart=0 when reached max_pump_time");
  }
  debugging("Fermo pompa per troppo tempo acceso");
  //Serial.println(F("Fermo pompa per troppo tempo acceso"));
  //client.publish("telemetry", "Interompo innaffiatura per pompa accesa troppo tempo");
  }else{
  if (!remotedata.forcestart && sensorsdata.hum1 > remotedata.humidityth && sensorsdata.temp1*100>lower_temp1th && sensorsdata.temp1*100<upper_temp1th && Hour<=WATERING_HOUR+WATERING_HOUR_range&&Hour>=WATERING_HOUR-WATERING_HOUR_range&& !EMERGENCY_STOP && !remotedata.external_pump) {
  printLocalTime();
  //     sensorsdata.lastactive=*asctime(&tm);
  if (!wateringon){
  //Serial.println(F("Avvio watering relais"));
  //client.publish("telemetry","Avvio watering relais");
  debugging("Avvio watering relais");
  water_on();
  }
  else{
    debugging("relais watering gia attivi");
 // Serial.println(F("relais watering gia attivi"));
 // client.publish("telemetry", "relais watering gia attivi");
  }
  //Serial.println(F("sensorsdata.lastactive=asctime(&tm)"));
  //Serial.println(asctime(&tm));
  //Serial.println(F("Avvio innaffiatura per umidita terreno bassa e temperatura sopra soglia"));
  //client.publish("telemetry", "Avvio innaffiatura per umidita terreno bassa e temperatura sopra soglia");
  debugging("Avvio innaffiatura per umidita terreno bassa e temperatura sopra soglia");
  }
  else{
  if (!remotedata.forcestart && sensorsdata.hum1 > remotedata.humidityth && (sensorsdata.temp1*100>lower_temp1th && sensorsdata.temp1*100<upper_temp1th) && (Hour<=WATERING_HOUR+WATERING_HOUR_range&&Hour>=WATERING_HOUR-WATERING_HOUR_range) && EMERGENCY_STOP && !remotedata.external_pump) {
  //Serial.println(F("False al check per soglia umidita determinato da emergency_stop true"));
  //client.publish("telemetry", "False al check per soglia umidita determinato da emergency_stop true");
  debugging("False al check per soglia umidita determinato da emergency_stop true");
  } 
  if (!wateringon && sensorsdata.hum1>remotedata.humidityth && (Hour<=WATERING_HOUR+WATERING_HOUR_range&&Hour>=WATERING_HOUR-WATERING_HOUR_range) && !EMERGENCY_STOP && !(sensorsdata.temp1*100>lower_temp1th && sensorsdata.temp1*100<upper_temp1th) && !remotedata.external_pump){
  if (sensorsdata.temp1*100<=lower_temp1th)
  { //Serial.println(F("Bassa umidita ma temperatura sotto soglia alla watering_hour"));        //
  //client.publish("telemetry","Bassa umidita ma temperatura sotto soglia alla watering_hour");
  debugging("Bassa umidita ma temperatura sotto soglia alla watering_hour");
  }
  if (sensorsdata.temp1*100>upper_temp1th){
  //Serial.println(F("Bassa umidita ma temperatura sopra soglia alla watering_hour"));        //
  //client.publish("telemetry","Bassa umidita ma temperatura sopra soglia alla watering_hour");
  debugging("Bassa umidita ma temperatura sopra soglia alla watering_hour");
  }
  }
  else{
  //&& (sensorsdata.temp1*100>lower_temp1th && sensorsdata.temp1*100<upper_temp1th)
  if (sensorsdata.hum1>remotedata.humidityth && !wateringon && !EMERGENCY_STOP && !remotedata.external_pump && !(Hour<=WATERING_HOUR+WATERING_HOUR_range&&Hour>=WATERING_HOUR-WATERING_HOUR_range)){
  //Serial.println(F("Bassa umidita ma non e' l'ora di innaffiare"));          //
  //client.publish("telemetry","Bassa umidita ma non e' l'ora di innaffiare");
  debugging("Bassa umidita ma non e' l'ora di innaffiare");
  }
  else{
  if (EMERGENCY_STOP && wateringon && !remotedata.forcestart){
  water_off();
  //Serial.println(F("EMERGENCY_STOP")); //
  //client.publish("telemetry","EMERGENCY_STOP!!!");
  debugging("EMERGENCY_STOP!!!");
  }
  else{
  if (!remotedata.forcestart && remotedata.external_pump && !wateringon){
  //Serial.println(F("external_pump setup"));        //
  //client.publish("telemetry","external_pump setup");
  debugging("external_pump setup");
  digitalWrite(RelayValveControll, LOW);
  // delay(15*1000);
  for (unsigned long tmp=millis();millis()-tmp<=15*1000;){}
  }
  else{
  if (wateringon && force_switchoff){
  //Serial.println(F("turning off from forcestart"));          //
  //client.publish("telemetry","turning off from forcestart");
  debugging("turning off from forcestart");
  water_off();
  }
  else{
    if (wateringon && !remotedata.forcestart){
      //Serial.println(F("Should I switch off even if force_switch_off not triggered"));       //
      //client.publish("telemetry","Should I switch off even if force_switch_off not triggered");
      debugging("Should I switch off even if force_switch_off not triggered");
      water_off();
    }
  if (sensorsdata.hum1<=0 && sensorsdata.temp1<=0){
    //Serial.println(F("still no sensors reading"));          //
    //client.publish("telemetry","still no sensors reading");
    debugging("still no sensors reading");
  }
  else{
    if (remotedata.forcestart && wateringon){
    //Serial.println(F("watering due to forcestart"));          //
    //client.publish("telemetry","watering due to forcestart");
    debugging("watering due to forcestart");
    if (EMERGENCY_STOP){
      //Serial.println(F("EMERGENCY_STOP but watering due to forcestart"));          //
      //client.publish("telemetry","EMERGENCY_STOP but watering due to forcestart");
      debugging("EMERGENCY_STOP but watering due to forcestart");
    }
  }
  else{
  if (remotedata.forcestart && remotedata.external_pump){
  //Serial.println(F("conflicting forcestart and external_pump: clean forcestart and setting up for external pump"));        //
  //client.publish("telemetry","conflicting forcestart and external_pump: clean forcestart and setting up for external pump");
  debugging("conflicting forcestart and external_pump: clean forcestart and setting up for external pump");
  water_off();
  remotedata.forcestart=0;
  digitalWrite(RelayValveControll, LOW);
  // delay(15*1000);
  for (unsigned long tmp=millis();millis()-tmp<=15*1000;){}
  }
  else{
  if (!remotedata.forcestart && sensorsdata.hum1 <= remotedata.humidityth && wateringon && !EMERGENCY_STOP && !remotedata.external_pump){
  //Serial.println(F("REached humidity threshold in another hour or temp below th: watering off"));//
  //client.publish("telemetry","REached humidity threshold in another hour or temp below th: watering off");
  debugging("REached humidity threshold in another hour or temp below th: watering off");
  water_off();
  }
  else{
  if (!remotedata.forcestart && sensorsdata.hum1 <= remotedata.humidityth && !wateringon && !EMERGENCY_STOP && !remotedata.external_pump){
  //Serial.println(F("Humidity below threshold e tutto va bene"));//
  //client.publish("telemetry","Humidity below threshold e tutto va bene");
  debugging("Humidity below threshold e tutto va bene");
  }
  else{
  if (!remotedata.forcestart && sensorsdata.hum1 <= remotedata.humidityth && !wateringon && EMERGENCY_STOP && !remotedata.external_pump){
  //Serial.println(F("Humidity below threshold e EMERGENCY_STOP true"));          //
  //client.publish("telemetry","Humidity below threshold e EMERGENCY_STOP true");
  debugging("Humidity below threshold e EMERGENCY_STOP true");
  }
  //Serial.println(F("unknow state??"));        //
  //client.publish("telemetry","unknow state??");
  //Serial.println(F("remotedata.forcestart"));          //
  //Serial.println(remotedata.forcestart);          //
  debugging("unknow state??");
  debugging(remotedata.forcestart);
  if (remotedata.forcestart){
    //client.publish("telemetry","remotedata.forcestart true");
    debugging("remotedata.forcestart true");
  }
  else{
    //client.publish("telemetry","remotedata.forcestart false");
    debugging("remotedata.forcestart false");
  }
  //Serial.println(F("sensorsdata.hum1 <= remotedata.humidityth")); //
  //Serial.println(sensorsdata.hum1 <= remotedata.humidityth);          //
  debugging(sensorsdata.hum1 <= remotedata.humidityth);
  if (sensorsdata.hum1 <= remotedata.humidityth){
    debugging("sensorsdata.hum1 <= remotedata.humidityth true");
//client.publish("telemetry","sensorsdata.hum1 <= remotedata.humidityth true");
    }
  else{
    debugging("sensorsdata.hum1 > remotedata.humidityth false");
   // client.publish("telemetry","sensorsdata.hum1 > remotedata.humidityth false");
  }
  //Serial.println(F("!wateringon"));          //
  //Serial.println(!wateringon);          //
  debugging(!wateringon);
  if (!wateringon){debugging("!wateringon true");//client.publish("telemetry","!wateringon true");
  }else{debugging("!wateringon false");//client.publish("telemetry","!wateringon false");
  }
  //Serial.println(F("EMERGENCY_STOP"));          //
  //Serial.println(EMERGENCY_STOP);          //
    debugging(EMERGENCY_STOP);
  if (EMERGENCY_STOP){
    //client.publish("telemetry","EMERGENCY_STOP true");
      debugging("EMERGENCY_STOP true");
    }
  else{
   // client.publish("telemetry","EMERGENCY_STOP false");
      debugging("EMERGENCY_STOP false");
  }
  //Serial.println(F("remotedata.external_pump"));          //
  //Serial.println(remotedata.external_pump);          //
      debugging(remotedata.external_pump);
  if (remotedata.external_pump){debugging("remotedata.external_pump true");//client.publish("telemetry","remotedata.external_pump true");
  }
  else{
      debugging("remotedata.external_pump false");
 // client.publish("telemetry","remotedata.external_pump false");
  }
    debugging(!(Hour<=WATERING_HOUR+WATERING_HOUR_range&&Hour>=WATERING_HOUR-WATERING_HOUR_range));
//  Serial.println(F("!(Hour<=WATERING_HOUR+WATERING_HOUR_range&&Hour>=WATERING_HOUR-WATERING_HOUR_range)"));          //
 // Serial.println(!(Hour<=WATERING_HOUR+WATERING_HOUR_range&&Hour>=WATERING_HOUR-WATERING_HOUR_range));          //
  if (!(Hour<=WATERING_HOUR+WATERING_HOUR_range&&Hour>=WATERING_HOUR-WATERING_HOUR_range)){
  client.publish("telemetry","!(Hour<=WATERING_HOUR+WATERING_HOUR_range&&Hour>=WATERING_HOUR-WATERING_HOUR_range) true");
    debugging("!(Hour<=WATERING_HOUR+WATERING_HOUR_range&&Hour>=WATERING_HOUR-WATERING_HOUR_range) true");
  }
  else{
  client.publish("telemetry","!(Hour<=WATERING_HOUR+WATERING_HOUR_range&&Hour>=WATERING_HOUR-WATERING_HOUR_range) false");
  debugging("!(Hour<=WATERING_HOUR+WATERING_HOUR_range&&Hour>=WATERING_HOUR-WATERING_HOUR_range) false");
  }
  }
  }
  }
  }
  }
  }
  }
  }
  }
  }
  }
  }
  }
  for (unsigned long tmp=millis();millis()-tmp<=1*1000;){}
  wateringon = ((digitalRead(RelayWaterControll) == LOW) || (digitalRead(RelayWaterControll) == 0));
  //Serial.println("internal_tick");
  //Serial.println(internal_tick);
  Serial.println(F("sensorsdata.watertick"));
  Serial.println(sensorsdata.watertick);
  Serial.println(F("remotedata.pumptime"));
  Serial.println(remotedata.pumptime);
  Serial.println(F("wateringon()"));
  Serial.println(wateringon);
  if (wateringon){
  Serial.println(F("wateringon true"));
  }
  if (((millis()-sensorsdata.watertick)>=remotedata.pumptime)&&(wateringon)&&!remotedata.forcestart){
  water_off();            
  remotedata.forcestart=false;
  client.publish("telemetry","watering off due to (millis()-sensorsdata.watertick)>=remotedata.pumptime");
  }
  wateringon = ((digitalRead(RelayWaterControll) == LOW) || (digitalRead(RelayWaterControll) == 0));
  if ((millis()-sensorsdata.watertick<=remotedata.pumptime)&&(wateringon)){
  Serial.println(F("watering on"));
  Serial.println(F("millis()-sensorsdata.watertick"));
  Serial.println(millis()-sensorsdata.watertick);
  }
  if (millis() - lastMsg > 20*1000) {
  lastMsg = millis();
  //  lastMsg = internal_tick;
  //++value;
  //the String.toCharArray()
  //snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", String.toCharArray(asctime(timeinfo)));
  //asctime(timeinfo).toCharArray(msg, 24);
  // Serial.print(aux);
  temp1char=f2chara("%f5.3",sensorsdata.temp1/(float)100);
  // temp_str = (String) (sensorsdata.temp1/100); //converting ftemp (the float variable above) to a string
  //  temp_str[temp_str.length()] = '\0';
  // temp_str.toCharArray(temp1char, temp_str.length()+1); //packaging up the data to publish to mqtt whoa...
  hum1char=us2chara("%hu",sensorsdata.hum1);
  // hum_str = (String) sensorsdata.hum1; //converting ftemp (the float variable above) to a string
  //    hum_str[hum_str.length()] = '\0';
  //   hum_str.toCharArray(hum1char, hum_str.length()+1); //packaging up the data to publish to mqtt whoa...
  Serial.println(F("Publish message to hum1"));
  client.publish("hum1", hum1char);
  Serial.println(F("Publish message to temp1"));
  client.publish("temp1", temp1char);
  }
    Serial.println(F("line 1204"));
  if (millis()-prev_read>=20*1000){
  if (sensorsdata.watertick!=0){
  //  epocheau= (unsigned long)(mktime(&epochstart_tm)+sensorsdata.watertick/1000);
  //  epocheau_tm=(time_t)epocheau;
  // localtime_r(&epocheau_tm, &tmepocheeau);// update the structure tm with the current time
  //       Serial.printf("The current date/time is: %s", asctime(&tmepocheeau));
  Serial.println(F("sensorsdata.watertick/1000"));
  Serial.println(sensorsdata.watertick/1000);
  Serial.println(F("mktime(&epochstart_tm)"));
  Serial.println(mktime(&epochstart_tm));
  Serial.println(F("mktime(&epochstart_tm)+sensorsdata.watertick/1000"));
  Serial.println(mktime(&epochstart_tm)+sensorsdata.watertick/1000);
  Serial.println(F("Convertin epocheau to date time"));
  Serial.print(tm.tm_year+1900);
  Serial.print(F(" "));
  Serial.print(tm.tm_mon+1);
  Serial.print(F(" "));
  Serial.print(tm.tm_mday);
  Serial.print(F(" "));
  Serial.print(tm.tm_hour);
  Serial.print(F(":"));
  Serial.print(tm.tm_min);
  Serial.print(F(":"));
  Serial.print(tm.tm_sec);
  Serial.println(F(""));
  //epocheau_str=(String) asctime(&tmepocheeau);
  //      epocheau_str=(String) sensorsdata.lastactive;
  // epocheau_str[epocheau_str.length()] = '\0';// Make payload a string by NULL terminating it.
  //  epocheau_str.toCharArray(epocheauchar, epocheau_str.length() + 1);//packaging up the data to publish to mqtt whoa...  
  stringformat="%s";
  epocheauchar=str2chara(stringformat,sensorsdata.lastactive);
  //client.publish("telemetry","epocheauchar date/time");
  //client.publish("telemetry",epocheauchar);
  // watertick_str=(String) sensorsdata.watertick;
  // watertick_str[watertick_str.length()] = '\0';// Make payload a string by NULL terminating it.
  //watertick_str.toCharArray(watertickchar, watertick_str.length() + 1);//packaging up the data to publish to mqtt whoa...  
  stringformat="%lu";
  watertickchar=ul2chara(stringformat,sensorsdata.watertick);
  client.publish("telemetry","watertickchar");
  client.publish("telemetry",watertickchar);
  client.publish("lastactive",epocheauchar);
  }
  else{
  Serial.println(F("Last watering: never (sensorsdata.watertick==0)"));
  client.publish("telemetry","epocheauchar date/time: never (sensorsdata.watertick==0)");
  client.publish("lastactive","never");
  }
  prev_read=millis();
  }
      Serial.println(F("line 1254"));
  if (wateringon){
  if (sensorsdata.hum1>humM&&!(sensorsdata.hum1==0)){
  humM=sensorsdata.hum1;
  }
  if (sensorsdata.hum1<humm&&!(sensorsdata.hum1==0)){
  humm=sensorsdata.hum1;
  }
  Serial.println(F("Minimo massimo umidita"));
  Serial.println(humm);
  Serial.println(humM);
  if (humM-humm<200 && ((millis()-sensorsdata.watertick)>=10*60*1000) && !remotedata.forcestart){
  EMERGENCY_STOP=true;
  }
  }
  else//what I need to reset if !wateringon:humm, humM
  {
  humm=4000;
  humM=300;
  }
      Serial.println(F("line 1274"));
  size_t NumberOfElements = sizeof(time_hist)/sizeof(time_hist[0]);
        Serial.println(F("NumberOfElements"));
      Serial.println(NumberOfElements);
      Serial.println(F("line 1278"));
  if (NumberOfElements>1){
        Serial.println(F("line 1280"));
        double mydiff=difftime(now_tt,*time_hist);
                Serial.println(F("line 1282"));
  if (mydiff>(double)20*60){
            Serial.println(F("line 1282"));
  hist_var(time_hist,now_tt,hum1_hist,sensorsdata.hum1,temp1_hist,sensorsdata.temp1);
  for (unsigned short i=0;i<hist_index_max;i++){
            Serial.println(F("line 1283"));
  client.publish("telemetry","time hist");
  client.publish("telemetry",str2chara("%s",asctime(localtime(&time_hist[i]))));
  client.publish("telemetry","hum1 hist");
  stringformat="%hu";
  client.publish("telemetry",us2chara(stringformat,hum1_hist[i]));
  stringformat="%d";
  client.publish("telemetry","temp1 hist");
  client.publish("telemetry",us2chara(stringformat,temp1_hist[i]));
  Serial.println(F("time hist"));
  Serial.println(asctime(localtime(&time_hist[i])));
  Serial.println(F("hum1_hist"));
  Serial.println(hum1_hist[i]);
  Serial.println(F("temp1_hist"));
  Serial.println(temp1_hist[i]);
  }
  Serial.println(F("line 1299"));
  }
  else{
    Serial.println(F("line 1302"));
  hist_var(time_hist,now_tt,hum1_hist,sensorsdata.hum1,temp1_hist,sensorsdata.temp1);
  }
  }
      Serial.println(F("line 1300"));
  if (sensorsdata.temp1>sensorsdata.temp1_max){
  sensorsdata.temp1_max=sensorsdata.temp1;
  hour_temp1_max=tm.tm_hour;
  }
  if (sensorsdata.temp1<sensorsdata.temp1_min){
  sensorsdata.temp1_min=sensorsdata.temp1;
  hour_temp1_min=tm.tm_hour;
  }
  Serial.println(F("Minimo massimo umidita"));
  Serial.println(humm);
  Serial.println(humM);
  if (humm!=0&&humM-humm<200 && ((millis()-sensorsdata.watertick)>=5*60*1000)){
  client.publish("telemetry","hum1 not decreasing enough: emergency_stop=true");
  Serial.println(F("hum1 not decreasing enough: emergency_stop=true"));
  EMERGENCY_STOP=true;
  }
  // if (difftime(tm, time_sequence_hist[0])>20*60){
  //   }
  if (Hour==23){
  startcounter=0;
  sensorsdata.temp1_min=1000;
  sensorsdata.temp1_max=1000;
  //humM=1600;
  //humm=3000;
  client.publish("telemetry","resetting temp1_min, temp1_max, startcounter");
  Serial.println(F("resetting humm, humM, temp1_min, temp1_max, startcounter"));
  }
  if ((millis()-sleep_timer>=5*60*1000)&&!wateringon&&!remotedata.forcestart){
  preferences.begin("remotedata", false);
  // Remove all preferences under the opened namespace
  //preferences.clear();
  struct tm wakeTM = *localtime(&now_tt);
  //char wakechar[40];
  wakeTM.tm_sec += TIME_TO_SLEEP;
  //struct tm startTM;
  //    time_t start;
  time_t wake_tt = mktime(&wakeTM);
  //snprintf ( wakechar, 40, "Next wake:\n %s",ctime(&wake_tt));
  // Note: Key name is limited to 15 chars.
   Serial.println(F("Writing data to flash before Going to sleep"));
  preferences.putBool("forcestart", remotedata.forcestart);
  preferences.putBool("EMERGENCY_STOP", EMERGENCY_STOP);
  preferences.putULong("pumptime", remotedata.pumptime);
  preferences.putUShort("humidityth", remotedata.humidityth);
  ref_voltage_18650 =preferences.putFloat("ref_voltage_18650", 1.042);
  ref_voltage_lead =preferences.putFloat("ref_voltage_lead", 3.388);
  remotedata.external_pump=preferences.getBool("external_pump", false);
  //preferences.putULong("watertick", sensorsdata.watertick);
  preferences.putString("lastactive", sensorsdata.lastactive);
  preferences.putBytes("time_hist", (byte*)(time_hist), sizeof(time_hist));
  preferences.putBytes("hum1_hist", (byte*)(hum1_hist), sizeof(hum1_hist));
  preferences.putBytes("temp1_hist", (byte*)(temp1_hist), sizeof(temp1_hist));
  // Close the Preferences
  preferences.end();
  Serial.println(F("Going to sleep now"));
  Serial.println(F("boot counter:"));
  Serial.println(bootCount);
  client.publish("telemetry","going to sleep");
  //client.publish("telemetry",wakechar);
  client.publish("telemetry",str2chara("Next wake:\r %s",ctime(&wake_tt)));
  //delay(1000);
  for (unsigned long tmp=millis();millis()-tmp<=1*1000;){}
  Serial.flush();
  esp_deep_sleep_start();
  }
}
