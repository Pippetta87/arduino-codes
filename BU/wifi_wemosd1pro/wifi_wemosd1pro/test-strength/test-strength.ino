#include "EspMQTTClient.h"
String sssid;
uint8_t encryptionType;
int32_t RSSI;
uint8_t* BSSID;
int32_t channel;
bool isHidden; 
uint8_t curBss;
const char* SSID = "Vodafoneebeb"; // change this to match your networks SSID
uint8_t prevRssi;

void setup()
{
  Serial.begin(9600);
}

void loop(){
  
  byte available_networks = WiFi.scanNetworks();
int netnum = 0;
prevRssi = 0;
// first lets find the SSID of the network you are looking for
// by iterating through all of the avaialble networks
// since in an enterprise there may be more than one BSSID for the SSID
// Lets find the stringest one
  for (int network = 0; network < available_networks; network++) {
    if (WiFi.SSID(network) == SSID) {
      Serial.print("Found one ");
      Serial.println (WiFi.RSSI(network));
      if ((uint8_t)WiFi.RSSI(network) > prevRssi) {
      netnum = network;
      prevRssi = (uint8_t)WiFi.RSSI(network);
    }
    }
}
WiFi.getNetworkInfo(netnum, sssid, encryptionType, RSSI, BSSID, channel, isHidden);
if (BSSID[5] != curBss){
  Serial.println("New Bss!");
  curBss = BSSID[5];
  Serial.print(BSSID[5],HEX);
  Serial.println(curBss,HEX);
  
}
  Serial.print("Signal strength: ");
  int bars;
//  int bars = map(RSSI,-80,-44,1,6); // this method doesn't refelct the Bars well
  // simple if then to set the number of bars
  
  if (RSSI > -55) { 
    bars = 5;
  } else if (RSSI < -55 & RSSI > -65) {
    bars = 4;
  } else if (RSSI < -65 & RSSI > -70) {
    bars = 3;
  } else if (RSSI < -70 & RSSI > -78) {
    bars = 2;
  } else if (RSSI < -78 & RSSI > -82) {
    bars = 1;
  } else {
    bars = 0;
  }


//print this out on the Serial console for debugging
  Serial.print(RSSI);
  Serial.println("dBm");
  Serial.print("BSS");
  for(int i=0;i<5;i++){
  Serial.print(BSSID[i],HEX);
  Serial.print(":");
  }
Serial.println(BSSID[5],HEX);
  
  Serial.print(bars);
  Serial.println(" bars");
  }
