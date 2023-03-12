/*
 *ESP32 Chip model = ESP32-D0WDQ6 Rev 1
This chip has 2 cores
Chip ID: 13084016
 */
/*
R=21.2 Omh
sun ADC>100/4095
 */
/*
 * ESP32 Chip model = ESP32-D0WDQ6 Rev 3
00:08:19.394 -> This chip has 2 cores
00:08:19.394 -> Chip ID: 2292260
*/
#include <stdio.h>
//#define getName(var)  #var
# define getName(var, str)  sprintf(str, "%s", #var)

//#include <Preferences.h>
//Preferences preferences;
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

//#include <math.h>
//#include <stdio.h>
#include <driver/adc.h>
#define __STDC_WANT_LIB_EXT1__ 1
/*#include <Arduino.h>
#include "esp_wpa2.h"
#define EAP_ANONYMOUS_IDENTITY "anonymous@example.com" //anonymous identity
#define EAP_IDENTITY "f.rossi11@studenti.unipi.it"                  //user identity
#define EAP_PASSWORD "0577222714" //eduroam user password
const char* ssid = "UniPisa";*/
#include <WiFi.h>
//OTA
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
//end OTA
#include <PubSubClient.h>
#define MQTT_KEEP_ALIVE 60 // int, in seconds
#define MQTT_CLEAN_SESSION false // bool, resuse existing session
#define MQTT_TIMEOUT 15000 // int, in miliseconds
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval


//vodafone ruffolo credentials 
//const char* ssid = "Vodafoneebeb";
//const char* password = "1lUB4jV1pdCCczvNdMyOvQQK";

//Tim Carpineto credentials 
 const char* ssid = "ruterino";
const char* password = "un cavallo muto";

//Malaphone
//const char* ssid = "Malaphone";
//const char* password = "g7u1xytgwyz";

const char* mqtt_server = "mqtt.flespi.io";
const int mqttPort = 1883;
const char* mqttUser = "DamGK4gbeV0InPvNfuTUeFThY8a36wxxvJ33B94FUSb5vvHANqjRV0WHYWyShWfA";
const char* mqttPassword = "";
const char* clientId = "solar.colonna.ruffolo";
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

unsigned short Hour;
unsigned short humM,humm;

//Variable for hum1, temp1, time history
time_t *time_hist;
short *temp1_hist;
unsigned short *hum1_hist;

//string def for topic identification
const char* topic_solarvoltage="solarvoltage";

//WiFiClientSecure espClient;
WiFiClient espClient;
PubSubClient client(espClient);

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

//OTA
const char* host = "esp32";

WebServer server(6660);

/*
 * Login page
 */

const char* loginIndex =
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>ESP32 Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
             "<td>Username:</td>"
             "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";

/*
 * Server Index Page
 */

const char* serverIndex =
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')"
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";
 //end OTA
/*
Method to print the reason by which ESP32
has been awaken from sleep
*/


void setup_wifi() {
  int32_t conchannel;
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
  Serial.println();
  Serial.print("Connecting to network: ");
  Serial.println(ssid);
  /*WiFi.disconnect(true);  //disconnect form wifi to set new wifi connection
  WiFi.mode(WIFI_STA); //init wifi mode
 esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_ANONYMOUS_IDENTITY, strlen(EAP_ANONYMOUS_IDENTITY)); //provide identity
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY)); //provide username
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD)); //provide password
esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT();
esp_wifi_sta_wpa2_ent_enable(&config);
WiFi.begin(ssid);
*/
WiFi.begin(ssid,password,conchannel,bssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
 //   Serial.print(".");
     Serial.println(WiFi.status());

  }
  /*
  0  WL_IDLE_STATUS  temporary status assigned when WiFi.begin() is called
1 WL_NO_SSID_AVAIL   when no SSID are available
2 WL_SCAN_COMPLETED scan networks is completed
3 WL_CONNECTED  when connected to a WiFi network
4 WL_CONNECT_FAILED when the connection fails for all the attempts
5 WL_CONNECTION_LOST  when the connection is lost
6 WL_DISCONNECTED when disconnected from a network
*/
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address set: ");
  Serial.println(WiFi.localIP()); //print LAN IP
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
  //client.subscribe("forcestart");
  }
}
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
  stringformat="%s";
  time_chr=str2chara(stringformat,asctime(&tm));
  Hour=tm.tm_hour;
  //  Serial.println(tm.tm_min);
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
  }

void setup() {
    // reads the input on analog pin A0 (value between 0 and 1023)
  int analogValue = analogRead(4);
  Serial.begin(115200);
  while (!Serial){}
  Serial.println(F("serial init"));
  // Open Preferences with my-app namespace. Each application module, library, etc
  // has to use a namespace name to prevent key name collisions. We will open storage in
  // RW-mode (second parameter has to be false).
  // Note: Namespace name is limited to 15 chars.
 
  /*
  First we configure the wake up source
  We set our ESP32 to wake up every 5 seconds
  */

  const unsigned long recent_epoch=1648248186;//epoch unix 25/03/2022 23.43
  setup_wifi();
  Serial.println(F("wifi init"));
  setup_time();
  Serial.println(F("time setup"));
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
        Serial.println("MQTT init");
        while(mktime(&epochstart_tm)<recent_epoch){
          time(&now_tt);
localtime_r(&now_tt,&epochstart_tm);
       Serial.println("setting epochstart");
       delay(1000);
          }

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
  //end OTA
  
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
    analogValue = analogRead(4);

      Serial.println("ESP.getFreeHeap()");
    Serial.println(ESP.getFreeHeap());
    Serial.println("WiFi.macAddress()");
  Serial.println(WiFi.macAddress());
//OTA
  server.handleClient();
//end OTA

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

  /*
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
  */
  //epocheau_str=(String) asctime(&tmepocheeau);
  //      epocheau_str=(String) sensorsdata.lastactive;
  // epocheau_str[epocheau_str.length()] = '\0';// Make payload a string by NULL terminating it.
  //  epocheau_str.toCharArray(epocheauchar, epocheau_str.length() + 1);//packaging up the data to publish to mqtt whoa...  
  //stringformat="%s";
  //epocheauchar=str2chara(stringformat,sensorsdata.lastactive);
  //client.publish("telemetry","epocheauchar date/time");
  //client.publish("telemetry",epocheauchar);
  // watertick_str=(String) sensorsdata.watertick;
  // watertick_str[watertick_str.length()] = '\0';// Make payload a string by NULL terminating it.
  //watertick_str.toCharArray(watertickchar, watertick_str.length() + 1);//packaging up the data to publish to mqtt whoa...  
  //stringformat="%lu";
  //client.publish("telemetry","watertickchar");
  //client.publish("telemetry",watertickchar);
  //client.publish("lastactive",epocheauchar);

/*
  preferences.begin("remotedata", false);
  // Remove all preferences under the opened namespace
  //preferences.clear();
  struct tm wakeTM = *localtime(&now_tt);
  //char wakechar[40];
  //struct tm startTM;
  //    time_t start;
  time_t wake_tt = mktime(&wakeTM);
  //snprintf ( wakechar, 40, "Next wake:\n %s",ctime(&wake_tt));
  // Note: Key name is limited to 15 chars.
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
*/

  Serial.println("Analog reading: ");
  Serial.println(analogValue);   // the raw analog reading
  
  }
