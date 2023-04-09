//ESP32 Chip model = ESP32-D0WDQ6 Rev 1
//This chip has 2 cores
//Chip ID: 13084016

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:
  unsigned short sensorValue = analogRead(4);
  // print out the value you read:
  Serial.println(sensorValue);
  }
