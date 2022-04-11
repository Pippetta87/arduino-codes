#include <Wire.h>
#include <mcp_can.h>
#include <SPI.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <ESP8266WiFi.h>

#define CAN0_INT 16 // Set INT to pin D0
MCP_CAN CAN0(15); // Set CS to pin 15

//long unsigned int rxId;
unsigned char len = 0;
long unsigned int rxId;
uint8_t rxBuf[8];

void setup() {
    Serial.begin(9600); // Used to type in character
    init_mcp();
}

void loop() {

    
    if(!digitalRead(CAN0_INT)) // If CAN0_INT pin is low, read receive buffer
    {
        CAN0.readMsgBuf(&rxId, &len, rxBuf); // Read data: len = data length, buf = data byte(s)


    }
}



void init_mcp() {
    // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
    if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
    {
        Serial.println("MCP2515 Initialized Successfully!");

        delay(1000);
    }
    else
    {
        Serial.println("Error Initializing MCP2515...");
        setup();
    }

    //CAN0.setMode(MCP_NORMAL);
    CAN0.setMode(MCP_LISTENONLY); // Set operation mode to MCP_NORMAL so the MCP2515 sends acks to received data.

    pinMode(CAN0_INT, INPUT); // Configuring pin for /INT input

    Serial.println("MCP2515 Listen-Only...");
}
