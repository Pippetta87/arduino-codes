/*
 * Prof. Michele Maffucci
 * 01.05.2017
 * accensione/spegnimento LED da pagina web
 * per maggiori informazioni:
 * https://www.arduino.cc/en/Reference/WiFi
 * https://github.com/esp8266
 */

#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/
#include <Wire.h>
/*
ESP8266 Chip id = 004D9B39
*/

// La connessione di WeMos D1 mini al un network WiFi viene
// realizzata usando un personal encryption WEP e WPA2
// Per poter sapere a quale rete connettersi bisogna
// effettuare un broadcast dell'SSID (nome del network) 

// definizione di due array di caratteri
// in cui memorizzare nome della rete WiFi e password

const char ssid[] = "Vodafoneebeb";       // inserire l'ssid della rete
const char pass[] = "1lUB4jV1pdCCczvNdMyOvQQK";   // password della rete

// Creazione di un server web in ascolto sulla porta 80
// attende contenuti (pagine html, immagini, css, ecc...)
ESP8266WebServer server(80);
// Handle Root
void rootPage() { 
  server.send(200, "text/plain", "Check out https://siytek.com !"); 
}
 
// Handle 404
void notfoundPage(){ 
  server.send(404, "text/plain", "404: Not found"); 
}
//int pinLed = D4;                   // LED connesso tra pin D4 e ground

void setup() {
  Serial.begin(9600);           // inizializzazione Serial Monitor
  delay(10);
  Wire.begin();

 //pinMode(pinLed, OUTPUT);         
 //digitalWrite(pinLed, LOW);      // LED inizialmente spento

  // Connessione alla rete WiFi
  
  Serial.println();
  Serial.println();
  Serial.println("------------- Avvio connessione ------------");
  Serial.print("Tentativo di connessione alla rete: ");
  Serial.println(ssid);

  /* 
   *  Viene impostata l'impostazione station (differente da AP o AP_STA)
   * La modalità STA consente all'ESP8266 di connettersi a una rete Wi-Fi
   * (ad esempio quella creata dal router wireless), mentre la modalità AP 
   * consente di creare una propria rete e di collegarsi
   * ad altri dispositivi (ad esempio il telefono).
   */
  
  WiFi.mode(WIFI_STA);

  /* 
   *  Inizializza le impostazioni di rete della libreria WiFi e fornisce lo stato corrente della rete,
   *  nel caso in esempio ha come parametri ha il nome della rete e la password.
   *  Restituisce come valori:
   *  
   *  WL_CONNECTED quando connesso al network
   *  WL_IDLE_STATUS quando non connesso al network, ma il dispositivo è alimentato
  */
  WiFi.begin(ssid, pass);

  /* 
   *  fino a quando lo non si è connessi alla WiFi
   *  compariranno a distanza di 250 ms dei puntini che
   *  evidenziano lo stato di avanzamento della connessione
  */  
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.print(".");
  }

  // se connesso alla WiFi stampa sulla serial monitor
  // nome della rete e stato di connessione
  Serial.println("");
  Serial.print("Sei connesso ora alla rete: ");
  Serial.println(ssid);
  Serial.println("WiFi connessa");

  // Avvia il server
 // Start Web Server
  server.on("/", rootPage);
  server.onNotFound(notfoundPage);
  server.begin();
  Serial.println("Server avviato");

  // Stampa l'indirizzo IP
  Serial.print("Usa questo URL : ");
  Serial.print("http://");
  Serial.print(WiFi.localIP()); // Restituisce i'IP della scheda
  Serial.println("/");
  Serial.println("\nI2C Scanner");
 Serial.printf(" ESP8266 Chip id = %08X\n", ESP.getChipId());

}
  
// Listen for HTTP requests
void loop(void){ 
  server.handleClient();
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
  else {
    Serial.println("done\n");
  }
  delay(5000);
}

void GetExternalIP()
{
  WiFiClient client;
  if (!client.connect("api.ipify.org", 80)) {
    Serial.println("Failed to connect with 'api.ipify.org' !");
  }
  else {
    int timeout = millis() + 5000;
    client.print("GET /?format=json HTTP/1.1\r\nHost: api.ipify.org\r\n\r\n");
    while (client.available() == 0) {
      if (timeout - millis() < 0) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }
    int size;
    while ((size = client.available()) > 0) {
      uint8_t* msg = (uint8_t*)malloc(size);
      size = client.read(msg, size);
      Serial.write(msg, size);
      free(msg);
    }
  }
}
