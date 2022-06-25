/*
  modified on Sep 8, 2020
  Modified by MohammedDamirchi from Arduino Examples
  Home
*/


// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:
  unsigned short sensorValue = analogRead(32);
  // print out the value you read:
  Serial.println(sensorValue);
  }
