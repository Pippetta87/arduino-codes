/*
//#include <mcp2515.h> //Library for using CAN Communication
//#include <mcp2515.h>
//#include <mcp_can.h>
//#include <SPI.h> //Library for using SPI Communication 

struct can_frame canMsg; 
MCP2515 mcp2515(10); // SPI CS Pin 10 
//MCP_CAN CAN(10);//SPI CS Pin 10 
*/

#include <Wire.h>
#include </home/salvicio/Arduino/libraries/I2C_Anything/I2C_Anything.h> 

#include <OneWire.h> 
#include <DallasTemperature.h>
// Data wire is plugged into pin 7 on the Arduino 
#define ONE_WIRE_BUS 7
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
   
unsigned char Flag_Recv = 0;
int humpin = A1; //read humidity sensor
int RelayWaterControl1 = 3;
unsigned long midref=millis();
unsigned long prev_read=0;
unsigned long prev_send=0;

struct remotedata_STRUCT
{
//control var from wemos
bool forcestart = false;
int pumptime = 270000;
short humidityth = 400;
unsigned short Hour=12;
} remotedata;

struct sensorsdata_STRUCT
{
//sensor variables
unsigned short hum1 = 0;  // variable to store the value read
unsigned short temp1 = 61;  // variable to store the value read
unsigned long watertick;
} sensorsdata;

const short i2c_size=sizeof(remotedata.forcestart)+sizeof(remotedata.pumptime)+sizeof(remotedata.humidityth)+sizeof(remotedata.Hour);

//i2c write bytes
 //    forcestart = Wire.read() << 8 | Wire.read();//legge forcestart

/*
void my_isr(){
  Serial.println("Interrupt Detected");
}

void MCP2515_ISR()
{
     Flag_Recv = 1;
}
*/
void setup() {
     Serial.println("Wire init with addres 2"); 
       Wire.begin(2);        // join i2c bus (address optional for master) slave
  //  Wire.begin((A4,A5));        // join i2c bus (SDA,SCL)
    Serial.begin(115200);
//attachInterrupt(digitalPinToInterrupt(2),MCP2515_ISR,CHANGE);
    // init can bus, baudrate: 100k
//  if(CAN.begin(CAN_100KBPS) ==CAN_OK) Serial.print("can init ok!!\r\n");
//  else Serial.print("Can init fail!!\r\n");


   Serial.println("Dallas Temperature IC Control Library Demo"); 
 // Start up the library 
 sensors.begin();
  //SPI.begin(); //Begins SPI communication
/*mcp2515.reset();
mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ); //Sets CAN at speed 500KBPS and Clock 8MHz
mcp2515.setNormalMode();
*/
     Serial.println("Settin RelayWaterControl1 to output"); 
 pinMode(RelayWaterControl1, OUTPUT);
      Serial.println("Settin onReceive for received from master"); 
   Wire.onReceive (receiveEvent);
         Serial.println("Settin onRequest for requested from master"); 
   Wire.onRequest(requestEvent);
  
/*  while (CAN_OK != CAN.begin(CAN_500KBPS))
    {
        Serial.println("CAN BUS init Failed");
        delay(100);
    }
    Serial.println("CAN BUS Shield Init OK!");
*/
}

/*void can_send (short address)
{
canMsg.can_id = address; //CAN id as address
        Serial.println("canMsg.can_id");
        Serial.println(canMsg.can_id,HEX);
  canMsg.can_dlc = 3; //CAN data length as num
  canMsg.data[0] = hum1;
  canMsg.data[1] = temp1;
  canMsg.data[2] = watertick;
    mcp2515.sendMessage(&canMsg); //Sends the CAN message
        Serial.println("Sent watertick, temp1, hum1 via can.");
}

void can_read(short address)
{
   if ((mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) && (canMsg.can_id == address)){
     forcestart = canMsg.data[0];
    pumptime = canMsg.data[1];
    humidityth = canMsg.data[2]; 
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

// called by interrupt service routine when incoming data arrives
void receiveEvent (int howMany)
 {
    Serial.println("received howMany from master"); 
   Serial.println(howMany); 
 if (howMany >= (sizeof(remotedata.forcestart)+sizeof(remotedata.pumptime)+sizeof(remotedata.humidityth)+sizeof(remotedata.Hour)))
   {
   I2C_readAnything(remotedata);
   //receivedData = true;
   }  // end if have enough data
 }  // end of receiveEvent

 // called by interrupt service routine when request from master
 
void requestEvent (int howMany)
 {
   Serial.println("requested howMany from master"); 
   Serial.println(howMany); 

 if (howMany >= (sizeof(sensorsdata.hum1)+sizeof(sensorsdata.temp1)+sizeof(sensorsdata.watertick)))
   {
   I2C_writeAnything(sensorsdata);
   //sentData = true;
   }  // end if have enough data
 }  // end of receiveEvent
 
void loop() {
    Serial.print(" Requesting temperatures..."); 
 sensors.requestTemperatures(); // Send the command to get temperature readings 
 Serial.println("DONE"); 
/********************************************************************/
 Serial.print("Temperature is: ");
 Serial.print(sensors.getTempCByIndex(0)); // Why "byIndex"?  
 sensorsdata.temp1=sensors.getTempCByIndex(0)*100;
   // You can have more than one DS18B20 on the same bus.  
   // 0 refers to the first IC on the wire 
  sensorsdata.hum1 = analogRead(humpin);  // read the input pin
   // temp1 = analogRead(temppin);  // read the input pin
      Serial.println("humidity");          // debug value
  Serial.println(sensorsdata.hum1);          // debug value
   //     Serial.println("temperature");          // debug value
 // Serial.println(temp1);          // debug value
  
if (sensorsdata.hum1 > remotedata.humidityth && sensorsdata.temp1>1000) {
  digitalWrite(RelayWaterControl1, HIGH);
sensorsdata.watertick=millis();
}
else{
  if (sensorsdata.hum1>400){
          Serial.println("Bassa umidita ma temperatura sotto soglia");          // 
  }
}

/*
if (millis()-prev_read>=5*1000){
 // can_read(0x001);
  Wire.requestFrom(1, i2c_size);    // request 6 bytes from peripheral device #1
   bool forcestart;
unsigned int pumptime;
 unsigned int humidityth;
unsigned long epochTime;
  while (Wire.available()) { // peripheral may send less than requested
byte in[4]={0,0,0,2};//from i2c
    for(int i=3;i>=0;i--){
       val +=in[3-i]<<(8*i);
       }
   Serial.print (val);
    char c = Wire.read(); // receive a byte as character

    Serial.print(c);         // print the character

  }
  prev_read=millis();
}
if (millis()-prev_send>=10*1000){
  //can_send(0x01);
   Wire.beginTransmission(1); // transmit to device #1

  Wire.write("x is ");        // sends five bytes

  Wire.write(x);              // sends one byte  

  Wire.endTransmission();    // stop transmitting


  Serial.println("prev_send");          // debug
    Serial.println(prev_send);          // debug value
Serial.println("prev_send-millis()");          // debug
    Serial.println(millis()-prev_send);          // debug value
    prev_send=millis();

}

delay(5000);
*/
}
