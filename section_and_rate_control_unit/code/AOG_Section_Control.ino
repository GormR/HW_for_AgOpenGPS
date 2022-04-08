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

/////////////////////////////////////////////////////////////////////////////////////////////////
// board options:
#define SPEEDINPUT 1  // define as 1, if section 0 shall be used as speed input (0 for section)
const uint8_t NUMBER_OF_SECTIONS = 7;   // mount the switches and relays RIGHT alligned!
/////////////////////////////////////////////////////////////////////////////////////////////////


#include <EEPROM.h>
#include <AceTMI.h>
#include <AceTMI.h> // SimpleTmiInterface
#include <AceSegment.h> // Tm1637Module

#define EEP_Ident 0x5005

//Program counter reset
void(* resetFunc) (void) = 0;

// display module:
using ace_tmi::SimpleTmi1637Interface;
using ace_segment::LedModule;
using ace_segment::Tm1637Module;
const uint8_t CLK_PIN = A5;
const uint8_t DIO_PIN = A4;
const uint8_t NUM_DIGITS = 4;

// Many TM1637 LED modules contain 10 nF capacitors on their DIO and CLK lines
// which are unreasonably high. This forces a 100 microsecond delay between
// bit transitions. If you remove those capacitors, you can set this as low as
// 1-5 micros.
const uint8_t DELAY_MICROS = 100;

using TmiInterface = SimpleTmi1637Interface;
TmiInterface tmiInterface(DIO_PIN, CLK_PIN, DELAY_MICROS);
Tm1637Module<TmiInterface, NUM_DIGITS> ledModule(tmiInterface);

// LED segment patterns.
const uint8_t NUM_PATTERNS = 10;
const uint8_t PATTERNS[NUM_PATTERNS] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111, // 9
};

//Variables for config - 0 is false
struct Config 
{
  uint8_t raiseTime = 2;
  uint8_t lowerTime = 4;
  uint8_t enableToolLift = 1;
  uint8_t isRelayActiveHigh = 1; // must always be 1 for section control board

};  Config aogConfig;   //4 bytes

const uint8_t LOOP_TIME = 20; //50hz
uint32_t lastTime = LOOP_TIME;
uint32_t currentTime = LOOP_TIME;
uint32_t fifthTime = 0;
uint16_t count = 0;
uint8_t recvState = 0;

//Comm checks
uint8_t watchdogTimer = 0;     //make sure we are talking to AOG
uint8_t serialResetTimer = 0;  //if serial buffer is getting full, empty it
uint8_t statusMainSwitch = 0;  // 0x80 and higher = on; also packet counter

// These constants won't change. They're used to give names to the pins used:
const int pressureInPin = A7;  // Analog input pin that the pressure sensor or potentiometer is attached to
const int flowInPin = A1;      // Digital input pin that the flow sensor or a pushbutton is attached to (can be used for another analog sensor as well)
const int speedInPin = 2;      // Digital input pin that the speed sensor or a pushbutton is attached to (option; 7 sections only in that case)
const int mainInPin = A6;      // Analog input pin for main switch (finally a digital input only)

const int secOutPin[] = {2, 3, 4, 5, 6, 7, 8, 9};      // Digital output for section x (0: instead of speed sensor)

const int pressureOutPin = A2; // Digital output for pressure modification (tristated if no mod)
const int levelOutPin = A3;    // Digital output for hydraulic rise/lower (tristated if no mod)

const int speedOutPin = 10;    // Digital output for speed PWM (from AgOpenGPS)

//The variables used for storage
uint8_t relayHi=0, relayLo = 0, gpsSpeed = 0, tramline = 0, tree = 0, uTurn = 0, hydLift = 0; 
uint8_t forceOffHi=0, forceOffLo = 0, forceOnHi = 0, forceOnLo = 0; 
uint8_t relayBack = 0;         // real relay status read from toggle switches
uint8_t raiseTimer = 0, lowerTimer = 0, lastTrigger = 0; 
bool isRaise = false;
bool isLower = false;

//Communication with AgOpenGPS
int16_t temp, EEread = 0;

//Parsing PGN
bool isPGNFound = false, isHeaderFound = false;
uint8_t pgn = 0, dataLength = 0, idx = 0;
int16_t tempHeader = 0;

uint8_t AOG[] = {0x80,0x81, 0x7b, 0xea, 8, 1, 0, 0, 0, 0,0,0,0, 0xCC };
// [5]: Bits: AOG section control AUTO, Section control OFF, Rate L+R ...
// [9]: relays low byte (0 on auto)
//[10]: 1 = forced off low byte
//[11]: relays high byte (0 on auto)
//[12]: 1 = forced off high byte
//
uint8_t Disp[] = {0, 0, 0, 0};   // LED display

int pressureValue = 0;         // value read from the pot or sensor
uint8_t flowValue = 0;         // value read from the flow sensor (simple counter for test)
uint8_t speedValue = 0;        // value read from the speed sensor (simple counter for test)

void setup()
{
  //set the baud rate
  Serial.begin(38400);
  while (!Serial) { ; } // wait for serial port to connect. Needed for native USB

  EEPROM.get(0, EEread);              // read identifier

  if (EEread != EEP_Ident)   // check on first start and write EEPROM
  {
    EEPROM.put(0, EEP_Ident);
    EEPROM.put(6, aogConfig);
  }
  else
  {
    EEPROM.get(6, aogConfig);
  }

  //set the pins for sections to be outputs (pin numbers)
  for (uint8_t i=0; i<9; i++)  pinMode(secOutPin[i], OUTPUT);  

  tmiInterface.begin();
  ledModule.begin();

  ledModule.setPatternAt(0, 0b00110001);
  ledModule.setPatternAt(1, 0b01111001);
  ledModule.setPatternAt(2, PATTERNS[5]);
  ledModule.setPatternAt(3, 0b00110001);

  ledModule.setBrightness(255);

  ledModule.flush();
}

// return value: section keys
uint8_t GetSetRelays(uint8_t mySecRelays)
{
  int mainSwitch = analogRead(mainInPin);
  if ((statusMainSwitch & 0x80) && (mainSwitch < 100))  statusMainSwitch = 0x04;  // send 4 packets with automatic off
  if (!(statusMainSwitch & 0x80) && (mainSwitch > 99))  statusMainSwitch = 0x84;  // send 4 packets with automatic on
  
  // pin, rate, duration  130 pp meter, 3.6 kmh = 1 m/sec or gpsSpeed * 130/3.6 or gpsSpeed * 36.1111
  // gpsSpeed is 10x actual speed so 3.61111
  gpsSpeed *= 3.61111;
  tone(speedOutPin, gpsSpeed);

  // less than 8 relays: "right alligned" on the board
  for (uint8_t i=0; i <(8 - NUMBER_OF_SECTIONS); i++)  mySecRelays <<= 1;  

  //set the pins to be inputs for read of toggle switch (pin numbers)
  uint8_t help = 0;
  for (uint8_t i=0; i<8; i++)
  {
    pinMode(secOutPin[i], INPUT);  
    if (digitalRead(secOutPin[i])) help |= 1 << i;            // read sections 1 thru 8
    pinMode(secOutPin[i], OUTPUT);  
    if (help & (1<<i)) digitalWrite(secOutPin[i], LOW);       // and write the opposite state
    else               digitalWrite(secOutPin[i], HIGH);
  }

  // check if forced: try to change within 10µs: not in force mode, if this is possible
  delayMicroseconds(5);
  uint8_t help1 = 0;
  for (uint8_t i=0; i<8; i++)
  {
    pinMode(secOutPin[i], INPUT);  
    if (digitalRead(secOutPin[i])) help1 |= 1 << i;   //section 1 thru 8
    pinMode(secOutPin[i], OUTPUT);  
    if (mySecRelays & (1<<i) && mainSwitch > 99) 
      digitalWrite(secOutPin[i], HIGH);
    else 
      digitalWrite(secOutPin[i], LOW);
  }
  forceOnLo = help & help1;  // foced on, if read as 1 and seen as one after 10µs
  forceOffLo = (help | help1) ^ 0xff; // ditto, for off

  if (mainSwitch < 100) 
  {
    forceOnLo = 0;
    forceOffLo = 0xff;
  }

  // less than 8 relays: "right alligned" on the board
  for (uint8_t i=0; i <(8 - NUMBER_OF_SECTIONS); i++)  
  {
    help >>= 1;  
    forceOffLo >>= 1;
    forceOnLo >>= 1;
  }

  return(help);
}

void doDisplay(void)  // Hack! To be modified!
{
  if (NUMBER_OF_SECTIONS == 5)  // for Vili
  {
    if (relayLo & 0x10) Disp[3] = 0b00000001; else Disp[3] = 0;
    if (relayLo & 0x08) Disp[2] = 0b00000001; else Disp[2] = 0;
    if (relayLo & 0x02) Disp[1] = 0b00000001; else Disp[1] = 0;
    if (relayLo & 0x01) Disp[0] = 0b00000001; else Disp[0] = 0;
    if (relayBack & 0x10) Disp[3] |= 0b00001000;
    if (relayBack & 0x08) Disp[2] |= 0b00001000;
    if (relayBack & 0x02) Disp[1] |= 0b00001000;
    if (relayBack & 0x01) Disp[0] |= 0b00001000;
  }
  if (NUMBER_OF_SECTIONS == 7)  // for Martin
  {
    if (relayLo & 0x40) Disp[3] = 0b00000001; else Disp[3] = 0;
    if (relayLo & 0x10) Disp[2] = 0b00000001; else Disp[2] = 0;
    if (relayLo & 0x04) Disp[1] = 0b00000001; else Disp[1] = 0;
    if (relayLo & 0x01) Disp[0] = 0b00000001; else Disp[0] = 0;
    if (relayBack & 0x40) Disp[3] |= 0b00001000;
    if (relayBack & 0x10) Disp[2] |= 0b00001000;
    if (relayBack & 0x04) Disp[1] |= 0b00001000;
    if (relayBack & 0x01) Disp[0] |= 0b00001000;
  }
  for (uint8_t i=0; i < 4; i++)  ledModule.setPatternAt(i, Disp[i]);
  ledModule.flush();
}

void loop() 
{
  currentTime = millis();

  // read the analog in value:
  pressureValue = analogRead(pressureInPin);

  if ((currentTime - serialResetTimer) >= 500)
  {
  //  relayLo = 0;
  //  relayHi = 0;
  }

  
  if ((currentTime - lastTime) >= LOOP_TIME)
  {
    lastTime = currentTime;

    //section relays
    //relayBack = GetSetRelays(relayLo);
    //doDisplay();   // Hack! To be modified!

/*    
    //hydraulic lift

    if (hydLift != lastTrigger && (hydLift == 1 || hydLift == 2))
    {
      lastTrigger = hydLift;
      lowerTimer = 0;
      raiseTimer = 0;

      //200 msec per frame so 5 per second
      switch (hydLift)
      {
        //lower
        case 1:
          lowerTimer = aogConfig.lowerTime * 5;
        break;

        //raise
        case 2:
          raiseTimer = aogConfig.raiseTime * 5;     
        break;
      }
    }

    //countdown if not zero, make sure up only
    if (raiseTimer) 
    {
      raiseTimer--;
      lowerTimer = 0;
    }
    if (lowerTimer) lowerTimer--; 

    //if anything wrong, shut off hydraulics, reset last
    if ((hydLift != 1 && hydLift != 2) || watchdogTimer > 10 ) //|| gpsSpeed < 2)
    {
      lowerTimer = 0;
      raiseTimer = 0;
      lastTrigger = 0;
    }

    if (aogConfig.isRelayActiveHigh)
    {
      isLower = isRaise = false;
      if (lowerTimer) isLower = true;
      if (raiseTimer) isRaise = true;
    }
    else
    {
      isLower = isRaise = true;
      if (lowerTimer) isLower = false;
      if (raiseTimer) isRaise = false;        
    }
*/
  } //end of timed loop
  // Serial Receive
  while (Serial.available())
  {
    switch (recvState)
    {
    case 0:
      if (Serial.read() == 0x80) recvState++;
      break;
    case 1:
      {
        if (Serial.read() == 0x81) recvState++;
        else recvState = 0;
      }
      break;
    case 2:
      {
        if (Serial.read() == 0x7f) recvState++;
        else recvState = 0;
      }
      break;
    case 3:
      {
        if (Serial.read() == 0xfe) recvState++;
        else recvState = 0;
      }
      break;
    case 4:
      {
        if (Serial.read() == 0x08) recvState++;
        else recvState = 0;
      }
      break;
    case 5:
      {
        uTurn = Serial.read();  
        recvState++;
      }
      break;
    case 6:
      {
        tree = Serial.read();
        recvState++;
      }
      break;
    case 7:
      {
        hydLift = Serial.read();
        recvState++;
      }
      break;
    case 8:
      {
        tramline = Serial.read();  //actual speed times 4, single uint8_t
        recvState++;
      }
      break;
    case 9:
      {
        AOG[9] = Serial.read();
        recvState++;
      }
      break;
    case 10:
      {
        AOG[10] = Serial.read();
        recvState++;
      }
      break;
    case 11:
      {
        AOG[11] = Serial.read();          // read relay control from AgOpenGPS
        recvState++;
      }
      break;
    case 12:
      {
        AOG[12] = Serial.read();          // read relay control from AgOpenGPS
        recvState++;
      }
      break;
    case 13:
      {
        AOG[13] = Serial.read();          // CRC - may be checked of course, but it's USB and so not needed...
  
        //section relays - we care for 8 relays only here
        uint8_t stillOn  =  AOG[11] & relayLo;
        uint8_t stillOff = (AOG[11] | relayLo) ^ 0xff;
        relayLo = AOG[11];          // read relay control from AgOpenGPS
        relayHi = AOG[12];          // read relay control from AgOpenGPS
        
        relayBack = GetSetRelays(relayLo);
        
        //forceOffLo |= stillOn;                        // a relay is seen as forced off when it's off although AOG wants to set it on
        //forceOffLo &= (relayBack ^ 0x0ff); 
        //forceOnLo |= stillOff;                        // a relay is seen as forced on when it's on although AOG wants to set it off
        //forceOnLo &= relayBack;                       
        //forceOffLo = stillOn & (relayBack ^ 0x0ff);         // a relay is seen as forced off when it's off although AOG wants to set it on
        //forceOnLo = stillOff & relayBack;                   // a relay is seen as forced on when it's on although AOG wants to set it off
        if (!(statusMainSwitch & 0x80)) forceOffLo = 0xff;  // off is off
                
        // send response to AgOpenGPS
        AOG[5] = 0;
        if (statusMainSwitch & 0x7f)
        {
          if (statusMainSwitch & 0x80)  AOG[5] = 0x01;  // tell AOG: auto on
          else                          AOG[5] = 0x02;  // tell AOG: auto off
          statusMainSwitch--;
        }
        AOG[6] = 0;
        AOG[7] = 0;
        AOG[8] = 0;
        AOG[9] = forceOnLo;
        AOG[10] = forceOffLo;
        AOG[11] = 0;
        AOG[12] = 0;

        //add the checksum
        int16_t CK_A = 0;
        for (uint8_t i = 2; i < sizeof(AOG)-1; i++)
        {
          CK_A = (CK_A + AOG[i]);
        }
        AOG[sizeof(AOG)-1] = CK_A;
      
        Serial.write(AOG,sizeof(AOG));        
        Serial.flush();   // flush out buffer
  
        //reset watchdog
        watchdogTimer = 0;
    
        //Reset serial Watchdog  
        serialResetTimer = currentTime;       
        recvState = 0;
  
        doDisplay();   // Hack! To be modified!
        while (Serial.available()) Serial.read();
      }
      break;
    default:
      recvState = 0;
      break;
    }
  }







  

  
  
/* Demo code  
  // read the analog in value:
  pressureValue = analogRead(pressureInPin);
  int mainSwitch = analogRead(mainInPin);

  relayLo ^= 0xff;  // toggle all relays

  uint8_t myRel = GetSetRelays();

  delay(400);
  
  // print the results to the Serial Monitor:
  Serial.print("Pressure = ");
  Serial.print(pressureValue);
  Serial.print("\t Main switch ");
  if (mainSwitch > 100)  Serial.print("on");
  else                   Serial.print("off");
  Serial.print("\t Relays: ");
  Serial.println(myRel);
  ledModule.setPatternAt(3, ((myRel & 0x80) >> 7) * 0b00000100 + ((myRel & 0x40) >> 6) * 0b00010000);
  ledModule.setPatternAt(2, ((myRel & 0x20) >> 5) * 0b00000100 + ((myRel & 0x10) >> 4) * 0b00010000);
  ledModule.setPatternAt(1, ((myRel & 0x08) >> 3) * 0b00000100 + ((myRel & 0x04) >> 2) * 0b00010000);
  ledModule.setPatternAt(0, ((myRel & 0x02) >> 1) * 0b00000100 + ((myRel & 0x01) >> 0) * 0b00010000);
  ledModule.flush();
*/
}











   /*
    * This program only turns the relays for section control
    * On and Off. Connect to the Relay Port in AgOpenGPS
    * 
    * Hydraulic Raise D4
    * Hydraulic Lower D3
    * 
    * Tram Right Side D5
    * Tram left Side D6
    * 
    * Section 0 to 5 -- D7 to D12
    * 
    
    
  //loop time variables in microseconds

  #include <EEPROM.h> 
  #define EEP_Ident 0x5005  

    //Program counter reset
    void(* resetFunc) (void) = 0;

  //Variables for config - 0 is false  
  struct Config {
  uint8_t raiseTime = 2;
  uint8_t lowerTime = 4;
  uint8_t enableToolLift = 0;
  uint8_t isRelayActiveHigh = 0; //if zero, active low (default)
  
  };  Config aogConfig;   //4 bytes
  
  const uint8_t LOOP_TIME = 200; //5hz
  uint32_t lastTime = LOOP_TIME;
  uint32_t currentTime = LOOP_TIME;
  uint32_t fifthTime = 0;
  uint16_t count = 0;

  //Comm checks
  uint8_t watchdogTimer = 0; //make sure we are talking to AOG
  uint8_t serialResetTimer = 0; //if serial buffer is getting full, empty it

  bool isRaise = false;
  bool isLower = false;
  
   //Communication with AgOpenGPS
  int16_t temp, EEread = 0;

   //Parsing PGN
  bool isPGNFound = false, isHeaderFound = false;
  uint8_t pgn = 0, dataLength = 0, idx = 0;
  int16_t tempHeader = 0;
  
  uint8_t AOG[] = {0x80,0x81, 0x7f, 0xED, 8, 0, 0, 0, 0, 0,0,0,0, 0xCC };

  //The variables used for storage
  uint8_t relayHi=0, relayLo = 0, gpsSpeed = 0, tramline = 0, tree = 0, uTurn = 0, hydLift = 0; 
 
  uint8_t raiseTimer = 0, lowerTimer = 0, lastTrigger = 0;  
 
void setup()
  {
    //set the baud rate
     Serial.begin(38400);  
     while (!Serial) { ; } // wait for serial port to connect. Needed for native USB
     
     EEPROM.get(0, EEread);              // read identifier
    
  if (EEread != EEP_Ident)   // check on first start and write EEPROM
  {           
    EEPROM.put(0, EEP_Ident);
    EEPROM.put(6, aogConfig);
  }
  else 
  { 
    EEPROM.get(6, aogConfig);
  }

  //set the pins to be outputs (pin numbers)
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);

  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);  
  pinMode(13, OUTPUT);  
}

void loop()
{
  //Loop triggers every 200 msec and sends back gyro heading, and roll, steer angle etc

  currentTime = millis();
  uint16_t time = currentTime;

  if (currentTime - lastTime >= LOOP_TIME)
  {
    lastTime = currentTime;

    //If connection lost to AgOpenGPS, the watchdog will count up 
    if (watchdogTimer++ > 250) watchdogTimer = 12;

    //clean out serial buffer to prevent buffer overflow
    if (serialResetTimer++ > 20)
    {
      while (Serial.available() > 0) uint8_t t = Serial.read();
      serialResetTimer = 0;
    }

    if (watchdogTimer > 10) 
    {
      relayLo = 0;
      relayHi = 0;
    }
    
    //hydraulic lift

    if (hydLift != lastTrigger && (hydLift == 1 || hydLift == 2))
    {
      lastTrigger = hydLift;
      lowerTimer = 0;
      raiseTimer = 0;

      //200 msec per frame so 5 per second
      switch (hydLift)
      {
        //lower
        case 1:
          lowerTimer = aogConfig.lowerTime * 5;
        break;

        //raise
        case 2:
          raiseTimer = aogConfig.raiseTime * 5;     
        break;
      }
    }

    //countdown if not zero, make sure up only
    if (raiseTimer) 
    {
      raiseTimer--;
      lowerTimer = 0;
    }
    if (lowerTimer) lowerTimer--; 

    //if anything wrong, shut off hydraulics, reset last
    if ((hydLift != 1 && hydLift != 2) || watchdogTimer > 10 ) //|| gpsSpeed < 2)
    {
      lowerTimer = 0;
      raiseTimer = 0;
      lastTrigger = 0;
    }

    if (aogConfig.isRelayActiveHigh)
    {
      isLower = isRaise = false;
      if (lowerTimer) isLower = true;
      if (raiseTimer) isRaise = true;
    }
    else
    {
      isLower = isRaise = true;
      if (lowerTimer) isLower = false;
      if (raiseTimer) isRaise = false;        
    }

    //section relays
    SetRelays();

    AOG[5] = aogConfig.isRelayActiveHigh;
    AOG[6] = (uint8_t)relayLo;
    AOG[7] = (uint8_t)tramline;
    
    
    //add the checksum
    int16_t CK_A = 0;
    for (uint8_t i = 2; i < sizeof(AOG)-1; i++)
    {
      CK_A = (CK_A + AOG[i]);
    }
    AOG[sizeof(AOG)-1] = CK_A;
    
    Serial.write(AOG,sizeof(AOG));        
    Serial.flush();   // flush out buffer
      
  } //end of timed loop

  // Serial Receive
  //Do we have a match with 0x8081?    
  if (Serial.available() > 4 && !isHeaderFound && !isPGNFound) 
  {
    uint8_t temp = Serial.read();
    if (tempHeader == 0x80 && temp == 0x81) 
    {
      isHeaderFound = true;
      tempHeader = 0;        
    }
    else  
    {
      tempHeader = temp;     //save for next time
      return;    
    }
  }

  //Find Source, PGN, and Length
  if (Serial.available() > 2 && isHeaderFound && !isPGNFound)
  {
    Serial.read(); //The 7F or less
    pgn = Serial.read();
    dataLength = Serial.read();
    isPGNFound = true;
    idx=0;
  } 

  //The data package
  if (Serial.available() > dataLength && isHeaderFound && isPGNFound)
  {
    if (pgn == 239) // EF Machine Data
    {
      uTurn = Serial.read();  
      tree = Serial.read();
      
      hydLift = Serial.read();
      tramline = Serial.read();  //actual speed times 4, single uint8_t
      
      //just get the rest of bytes
      Serial.read();   //high,low bytes   
      Serial.read();  
      
      relayLo = Serial.read();          // read relay control from AgOpenGPS
      relayHi = Serial.read();

      if (aogConfig.isRelayActiveHigh)
      {
        tramline = 255 - tramline;
        relayLo = 255 - relayLo;
      }
      
      //Bit 13 CRC
      Serial.read();
      
      //reset watchdog
      watchdogTimer = 0;
  
      //Reset serial Watchdog  
      serialResetTimer = 0;        
      
      //reset for next pgn sentence
      isHeaderFound = isPGNFound = false;
      pgn=dataLength=0;                  
    }
            
    else if (pgn==238) //EE Machine Settings 
    {         
      aogConfig.raiseTime = Serial.read();
      aogConfig.lowerTime = Serial.read();    
      aogConfig.enableToolLift = Serial.read();
      
      //set1 
      uint8_t sett = Serial.read();  //setting0     
      if (bitRead(sett,0)) aogConfig.isRelayActiveHigh = 1; else aogConfig.isRelayActiveHigh = 0;
      
      Serial.read();
      Serial.read();
      Serial.read();
      Serial.read();    

      //crc
      //udpData[13];        //crc
      Serial.read();
  
      //save in EEPROM and restart
      EEPROM.put(6, aogConfig);
      resetFunc();

      //reset for next pgn sentence
      isHeaderFound = isPGNFound = false;
      pgn=dataLength=0;
    }
    else //nothing found, clean up
    {
      isHeaderFound = isPGNFound = false;
      pgn=dataLength=0;   
    }
  }
}

 void SetRelays(void)
 { 
    //change the pin number as required (pinD#, bitRead....)               
    digitalWrite (2, bitRead(relayLo,0)); //section 1 thru 8
    digitalWrite (3, bitRead(relayLo,1));
    digitalWrite (4, bitRead(relayLo,2));
    digitalWrite (5, bitRead(relayLo,3));
    digitalWrite (6, bitRead(relayLo,4));
    digitalWrite (7, bitRead(relayLo,5));
    digitalWrite (8, bitRead(relayLo,6));
    digitalWrite (9, bitRead(relayLo,7));

    //digitalWrite (10, isLower); //hydraulic control D10, D11
    //digitalWrite (11, isRaise);
    
    //digitalWrite (12, bitRead(tramline,0)); //left and right tram
    //digitalWrite (13, bitRead(tramline,2)); //D12, D13
 }

 */
