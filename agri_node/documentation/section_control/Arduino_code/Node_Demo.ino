#include "AgriNode.h"

AgriNode myNode;

void setup() {
  // Set serial monitor debugging window baud rate to 115200
  Serial.begin(115200);
  Serial.println("Hola!");

  myNode._digitalWrite(128, HIGH);  // power to sensors (relay 1)
  delay(200);
  myNode._analogWrite(130, 3);      // debug: test Modbus timer
}

void loop() 
{
    myNode.digitalWriteExtIO(myNode.digitalReadExtIO());   // debug: simply copy inputs to outputs for testing
    myNode._analogWrite(100, myNode._analogRead(100)>>8);    // debug: output analog input as PWM on IBT-2 (I)
    delay(200);
}
