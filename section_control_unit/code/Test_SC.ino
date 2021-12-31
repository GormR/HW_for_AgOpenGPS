/*
  Test code for SC board for AgOpenGPS

  Reads an analog input pin, maps the result to a range from 0 to 255 and uses
  the result to set the pulse width modulation (PWM) of an output pin.
  Also prints the results to the Serial Monitor.

  The circuit:
  - potentiometer connected to analog pin 7.
    Center pin of the potentiometer goes to the analog pin.
    side pins of the potentiometer go to +5V and ground
  - LED connected from digital pin 9 to ground through 220 ohm resistor

  created 29 Dec. 2021
  by GR

  This example code is in the public domain.
*/

#include <EEPROM.h>
#define EEP_Ident 0x5005

//Variables for config - 0 is false
struct Config 
{
  uint8_t raiseTime = 2;
  uint8_t lowerTime = 4;
  uint8_t enableToolLift = 1;
  uint8_t isRelayActiveHigh = 1; // must always be 1

};  Config aogConfig;   //4 bytes

// These constants won't change. They're used to give names to the pins used:
const int pressureInPin = A7;  // Analog input pin that the pressure sensor or potentiometer is attached to
const int flowInPin = A1;      // Digital input pin that the flow sensor or a pushbutton is attached to (can be used for another analog sensor as well)
const int speedInPin = 2;      // Digital input pin that the speed sensor or a pushbutton is attached to (option; 7 sections only in that case)
const int mainInPin = A6;      // Analog input pin for main switch (finally a digital input only)

const int sec0OutPin = 2;      // Digital output for section 0 (instead of speed sensor)
const int sec1OutPin = 3;      // Digital output for section 1
const int sec2OutPin = 4;      // Digital output for section 2
const int sec3OutPin = 5;      // Digital output for section 3
const int sec4OutPin = 6;      // Digital output for section 4
const int sec5OutPin = 7;      // Digital output for section 5
const int sec6OutPin = 8;      // Digital output for section 6
const int sec7OutPin = 9;      // Digital output for section 7

const int pressureOutPin = A2; // Digital output for pressure modification (tristated if no mod)
const int levelOutPin = A3;    // Digital output for hydraulic rise/lower (tristated if no mod)

const int speedOutPin = 10;    // Digital output for speed PWM (from AgOpenGPS)

//The variables used for storage
uint8_t relayHi=0, relayLo = 0x55, tramline = 0, uTurn = 0, hydLift = 0; 
float gpsSpeed;
uint8_t raiseTimer = 0, lowerTimer = 0, lastTrigger = 0; 
bool isRaise = false;
bool isLower = false;

int pressureValue = 0;         // value read from the pot or sensor
uint8_t flowValue = 0;         // value read from the flow sensor (simple counter for test)
uint8_t speedValue = 0;        // value read from the speed sensor (simple counter for test)

void setup()
{
  //set the baud rate
  Serial.begin(38400);

/*  EEPROM.get(0, EEread);              // read identifier

  if (EEread != EEP_Ident)   // check on first start and write EEPROM
  {
    EEPROM.put(0, EEP_Ident);
    EEPROM.put(6, aogConfig);
  }
  else
  {
    EEPROM.get(6, aogConfig);
  }
*/
  //set the pins to be outputs (pin numbers)
  pinMode(speedOutPin, OUTPUT);  
}


uint8_t GetSetRelays(void)
{
  // pin, rate, duration  130 pp meter, 3.6 kmh = 1 m/sec or gpsSpeed * 130/3.6 or gpsSpeed * 36.1111
  // gpsSpeed is 10x actual speed so 3.61111
  gpsSpeed *= 3.61111;
  tone(speedOutPin, gpsSpeed);

  //set the pins to be outputs (pin numbers)
  pinMode(sec0OutPin, INPUT);  
  pinMode(sec1OutPin, INPUT);  
  pinMode(sec2OutPin, INPUT);  
  pinMode(sec3OutPin, INPUT);  
  pinMode(sec4OutPin, INPUT);  
  pinMode(sec5OutPin, INPUT);  
  pinMode(sec6OutPin, INPUT);  
  pinMode(sec7OutPin, INPUT);  

  uint8_t help = 0;
  if (digitalRead(sec0OutPin)) help |= 0x01; //section 1 thru 8
  if (digitalRead(sec1OutPin)) help |= 0x02; 
  if (digitalRead(sec2OutPin)) help |= 0x04; 
  if (digitalRead(sec3OutPin)) help |= 0x08; 
  if (digitalRead(sec4OutPin)) help |= 0x10; 
  if (digitalRead(sec5OutPin)) help |= 0x20; 
  if (digitalRead(sec6OutPin)) help |= 0x40; 
  if (digitalRead(sec7OutPin)) help |= 0x80; 
 
  //set the pins to be outputs (pin numbers)
  //pinMode(sec0OutPin, OUTPUT);  // remarked for speed pulse input here
  pinMode(sec1OutPin, OUTPUT);  
  pinMode(sec2OutPin, OUTPUT);  
  pinMode(sec3OutPin, OUTPUT);  
  pinMode(sec4OutPin, OUTPUT);  
  pinMode(sec5OutPin, OUTPUT);  
  pinMode(sec6OutPin, OUTPUT);  
  pinMode(sec7OutPin, OUTPUT);  
  pinMode(speedOutPin, OUTPUT);  

  //change the pin number as required (pinD#, bitRead....)
  digitalWrite(sec0OutPin, bitRead(relayLo, 0)); //section 1 thru 8
  digitalWrite(sec1OutPin, bitRead(relayLo, 1));
  digitalWrite(sec2OutPin, bitRead(relayLo, 2));
  digitalWrite(sec3OutPin, bitRead(relayLo, 3));
  digitalWrite(sec4OutPin, bitRead(relayLo, 4));
  digitalWrite(sec5OutPin, bitRead(relayLo, 5));
  digitalWrite(sec6OutPin, bitRead(relayLo, 6));
  digitalWrite(sec7OutPin, bitRead(relayLo, 7));

  if ((isLower) || (isRaise))
  {
    pinMode(levelOutPin, OUTPUT);  
    if (isLower) digitalWrite(levelOutPin, false);  
    else         digitalWrite(levelOutPin, true);  
  }
  else
  {
    pinMode(levelOutPin, INPUT);  
  }

  //digitalWrite (12, bitRead(tramline,0)); //left and right tram
  //digitalWrite (13, bitRead(tramline,2)); //D12, D13

  return(help);
}


void loop()  //Loop triggers every sec 
{
  // read the analog in value:
  pressureValue = analogRead(pressureInPin);
  int mainSwitch = analogRead(mainInPin);

  relayLo ^= 0xff;  // toggle all relays

  // print the results to the Serial Monitor:
  Serial.print("Pressure = ");
  Serial.print(pressureValue);
  Serial.print("\t Main switch ");
  if (mainSwitch > 100)  Serial.print("on");
  else                   Serial.print("off");
  Serial.print("\t Relays: ");
  Serial.println(GetSetRelays());

  delay(1000);
}
