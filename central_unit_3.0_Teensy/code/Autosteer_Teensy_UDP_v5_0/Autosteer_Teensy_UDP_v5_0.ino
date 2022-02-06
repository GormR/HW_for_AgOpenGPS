

/*
   UDP Autosteer code for Teensy 4.1
   For AgOpenGPS
   24 Nov 2021
   Like all Arduino code - copied from somewhere else :)
   So don't claim it as your own
*/

////////////////// User Settings /////////////////////////

//How many degrees before decreasing Max PWM
#define LOW_HIGH_DEGREES 5.0

/*  PWM Frequency ->
     490hz (default) = 0
     122hz = 1
     3921hz = 2
*/
#define PWM_Frequency 0

/////////////////////////////////////////////

// if USB power delivery is not available, comment out
#define USB_PD

// if not in eeprom, overwrite
#define EEP_Ident 5100

// Address of CMPS14 shifted right one bit for arduino wire library
#define CMPS14_ADDRESS 0x60

// BNO08x definitions
#define REPORT_INTERVAL 90 //Report interval in ms (same as the delay at the bottom)

//   ***********  Motor drive connections  **************888
//Connect ground only for cytron, Connect Ground and +5v for IBT2

//Dir1 for Cytron Dir, Both L and R enable for IBT2
#define DIR1_RL_ENABLE  4

//PWM1 for Cytron PWM, Left PWM for IBT2
#define PWM1_LPWM  2

//Not Connected for Cytron, Right PWM for IBT2
#define PWM2_RPWM  3

//--------------------------- Switch Input Pins ------------------------
#define STEERSW_PIN 32
#define WORKSW_PIN 34
#define REMOTE_PIN 37


//Define sensor pin for current or pressure sensor
#define ANALOG_SENSOR_PIN A0

#define CONST_180_DIVIDED_BY_PI 57.2957795130823

#include <Wire.h>
extern TwoWire Wire1;

#include "tcpm_driver.h"  // **USB-PD**
#include "usb_pd.h"

const int usb_pd_int_pin = 35;
const int debug_led_pin  = 13;
const int debug0 = 11;
const int debug1 = 12;
const int enableDCDC = 36;

// USB-C Specific - TCPM start 1
const struct tcpc_config_t tcpc_config[CONFIG_USB_PD_PORT_COUNT] = {
  {0, fusb302_I2C_SLAVE_ADDR, &fusb302_tcpm_drv},
};
// USB-C Specific - TCPM end 1

#include <EEPROM.h>
#include "zADS1115.h"
ADS1115_lite adc(ADS1115_DEFAULT_ADDRESS);     // Use this for the 16-bit version ADS1115

#include <IPAddress.h>
#include "BNO08x_AOG.h"

// ethernet
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>

// ethernet interface ip address
// static uint8_t myip[] = { 192, 168, 1, 73 };
// TODO: Change the ipaddress? Or is it a replacement
IPAddress myip(192, 168, 0, 123);

// gateway ip address
static uint8_t gwip[] = { 192, 168, 0, 1 };

//DNS- you just need one anyway
static uint8_t myDNS[] = { 8, 8, 8, 8 };

//mask
static uint8_t mask[] = { 255, 255, 255, 0 };

//this is port of this autosteer module
uint16_t NTRIPUDPport = 2233;
uint16_t GNSSUDPport  = 5544;
uint16_t steerUDPport = 8888;
uint16_t AOGUDPport   = 9999;  //AOG port that listens

EthernetUDP udpNTRIP;
EthernetUDP udpSteer;

//sending back to where and which port
//static uint8_t ipDestination[] = {192, 168, 1, 255};
IPAddress ipDestination(192, 168, 0, 255);

// ethernet mac address - must be unique on your network
static uint8_t mymac[] = { 0x70, 0x69, 0x69, 0x2D, 0x30, 0x31 };

//uint8_t Ethernet::buffer[200]; // udp send and receive buffer
uint8_t udpData[UDP_TX_PACKET_MAX_SIZE];  // Buffer For Receiving UDP Data

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

//loop time variables in microseconds
const uint16_t LOOP_TIME = 25;  //40Hz
uint32_t lastTime = LOOP_TIME;
uint32_t currentTime = LOOP_TIME;
uint32_t lastNMEATime = 0;

const uint16_t WATCHDOG_THRESHOLD = 100;
const uint16_t WATCHDOG_FORCE_VALUE = WATCHDOG_THRESHOLD + 2; // Should be greater than WATCHDOG_THRESHOLD
uint8_t watchdogTimer = WATCHDOG_FORCE_VALUE;

//show life in AgIO
uint8_t helloAgIO[] = {0x80, 0x81, 0x7f, 0xC7, 1, 0, 0x47 };
uint8_t helloCounter = 0;

//fromAutoSteerData FD 253 - ActualSteerAngle*100 -5,6, SwitchByte-7, pwmDisplay-8
uint8_t AOG[] = {0x80, 0x81, 0x7f, 0xFD, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0xCC };
int16_t AOGSize = sizeof(AOG);

// F9P input data
const uint8_t NO_OF_NMEA_SENTENCES = 2;  // 2 sentences per time
const uint16_t NMEA_MAX_BYTES = 500;
uint8_t noOfF9Ps = 0;
uint8_t F9P_1[2*NMEA_MAX_BYTES];
uint8_t F9P_1state = 0;
uint16_t F9P_1cnt = 0;
uint8_t F9P_2[NMEA_MAX_BYTES];
uint16_t F9P_2cnt = 0;
uint8_t F9P_2state = 0;

// booleans to see if we are using CMPS or BNO08x
bool useCMPS = false;
bool useBNO08x = false;

// BNO08x address variables to check where it is
const uint8_t bno08xAddresses[] = {0x4A, 0x4B};
const int16_t nrBNO08xAdresses = sizeof(bno08xAddresses) / sizeof(bno08xAddresses[0]);
uint8_t bno08xAddress;
BNO080 bno08x;

float bno08xHeading = 0;
double bno08xRoll = 0;
double bno08xPitch = 0;

int16_t bno08xHeading10x = 0;
int16_t bno08xRoll10x = 0;

//EEPROM
int16_t EEread = 0;

//Relays
bool isRelayActiveHigh = true;
uint8_t relay = 0, relayHi = 0, uTurn = 0;
uint8_t tram = 0;

//Switches
uint8_t remoteSwitch = 0, workSwitch = 0, steerSwitch = 1, switchByte = 0;

//On Off
uint8_t guidanceStatus = 0;

//speed sent as *10
float gpsSpeed = 0;

//steering variables
float steerAngleActual = 0;
float steerAngleSetPoint = 0; //the desired angle from AgOpen
int16_t steeringPosition = 0; //from steering sensor
float steerAngleError = 0; //setpoint - actual

// Where is this for?
float CurrentSpeed = 0.0;

//pwm variables
int16_t pwmDrive = 0, pwmDisplay = 0;
float pValue = 0;
float errorAbs = 0;
float highLowPerDeg = 0;

//Steer switch button  ***********************************************************************************************************
uint8_t currentState = 1, reading, previous = 0;
uint8_t pulseCount = 0; // Steering Wheel Encoder
bool encEnable = false; //debounce flag
uint8_t thisEnc = 0, lastEnc = 0;

//Variables for settings
struct Storage {
  uint8_t Kp = 40;  //proportional gain
  uint8_t lowPWM = 10;  //band of no action
  int16_t wasOffset = 0;
  uint8_t minPWM = 9;
  uint8_t highPWM = 60;//max PWM value
  float steerSensorCounts = 30;
  float AckermanFix = 1;     //sent as percent
};  Storage steerSettings;  //11 bytes

//Variables for settings - 0 is false
struct Setup {
  uint8_t InvertWAS = 0;
  uint8_t IsRelayActiveHigh = 1; //if zero, active low (default)
  uint8_t MotorDriveDirection = 0;
  uint8_t SingleInputWAS = 1;
  uint8_t CytronDriver = 0;
  uint8_t SteerSwitch = 0;  //1 if switch selected
  uint8_t SteerButton = 0;  //1 if button selected
  uint8_t ShaftEncoder = 0;
  uint8_t PressureSensor = 0;
  uint8_t CurrentSensor = 0;
  uint8_t PulseCountMax = 5;
  uint8_t IsDanfoss = 0;
};  Setup steerConfig;          //9 bytes

//reset function
void(* resetFunc) (void) = 0;

int currentPos = 0;

void setup()
{
  Serial2.begin(460800);  // F9P_2
  Serial7.begin(460800);  // F9P_1
  Serial.begin(38400);
  delay(100);
  Serial.println("Setup");
  //PWM rate settings. Set them both the same!!!!
  /*  PWM Frequency ->
       490hz (default) = 0
       122hz = 1
       3921hz = 2
  */
  if (PWM_Frequency == 0)
  {
    analogWriteFrequency(PWM1_LPWM, 490);
    analogWriteFrequency(PWM2_RPWM, 490);
  }
  else if (PWM_Frequency == 1)
  {
    analogWriteFrequency(PWM1_LPWM, 122);
    analogWriteFrequency(PWM2_RPWM, 122);
  }
  else if (PWM_Frequency == 2)
  {
    analogWriteFrequency(PWM1_LPWM, 3921);
    analogWriteFrequency(PWM2_RPWM, 3921);
  }

  //keep pulled high and drag low to activate, noise free safe
  pinMode(WORKSW_PIN, INPUT_PULLUP);
  pinMode(STEERSW_PIN, INPUT_PULLUP);
  pinMode(REMOTE_PIN, INPUT_PULLUP);
  pinMode(DIR1_RL_ENABLE, OUTPUT);

  pinMode(debug0, OUTPUT);
  pinMode(debug1, OUTPUT);
  pinMode(enableDCDC, OUTPUT);
  digitalWrite(enableDCDC, LOW);

#ifdef USB_PD
  pinMode(usb_pd_int_pin, INPUT_PULLUP);   // **USB-PD**
  pinMode(debug_led_pin, OUTPUT);
  digitalWrite(debug_led_pin, LOW);
#endif

  if (steerConfig.CytronDriver) pinMode(PWM2_RPWM, OUTPUT);

  //set up communication
  //Wire.begin();
  Wire1.begin();

   //test if CMPS working
  uint8_t error;
  Wire1.beginTransmission(CMPS14_ADDRESS);
  error = Wire1.endTransmission();

  if (error == 0)
  {
    Serial.println("Error = 0");
    Serial.print("CMPS14 ADDRESs: 0x");
    Serial.println(CMPS14_ADDRESS, HEX);
    Serial.println("CMPS14 Ok.");
    useCMPS = true;
  }
  else
  {
    Serial.println("Error = 4");
    Serial.println("CMPS not Connected or Found");
    useCMPS = false;
  }

  // Check for BNO08x
  if (!useCMPS)
  {
    for (int16_t i = 0; i < nrBNO08xAdresses; i++)
    {
      bno08xAddress = bno08xAddresses[i];

      Serial.print("\r\nChecking for BNO08X on ");
      Serial.println(bno08xAddress, HEX);
      Wire1.beginTransmission(bno08xAddress);
      error = Wire1.endTransmission();

      if (error == 0)
      {
        Serial.println("Error = 0");
        Serial.print("BNO08X ADDRESs: 0x");
        Serial.println(bno08xAddress, HEX);
        Serial.println("BNO08X Ok.");

        // Initialize BNO080 lib
        if (bno08x.begin(bno08xAddress, Wire1))
        {
          Wire1.setClock(400000); //Increase I2C data rate to 400kHz

          // Use gameRotationVector
          bno08x.enableGameRotationVector(REPORT_INTERVAL); //Send data update every REPORT_INTERVAL in ms for BNO085

          // Retrieve the getFeatureResponse report to check if Rotation vector report is corectly enable
          if (bno08x.getFeatureResponseAvailable() == true)
          {
            if (bno08x.checkReportEnable(SENSOR_REPORTID_GAME_ROTATION_VECTOR, REPORT_INTERVAL) == false) bno08x.printGetFeatureResponse();

            // Break out of loop
            useBNO08x = true;
            break;
          }
          else
          {
            Serial.println("BNO08x init fails!!");
          }
        }
        else
        {
          Serial.println("BNO080 not detected at given I2C address.");
        }
      }
      else
      {
        Serial.println("Error = 4");
        Serial.println("BNO08X not Connected or Found");
      }
    }
  }

  //50Khz I2C
  TWBR = 144;

  EEPROM.get(0, EEread);              // read identifier

  if (EEread != EEP_Ident)   // check on first start and write EEPROM
  {
    EEPROM.put(0, EEP_Ident);
    EEPROM.put(10, steerSettings);
    EEPROM.put(40, steerConfig);
  }
  else
  {
    EEPROM.get(10, steerSettings);     // read the Settings
    EEPROM.get(40, steerConfig);
  }

  // for PWM High to Low interpolator
  highLowPerDeg = ((float)(steerSettings.highPWM - steerSettings.lowPWM)) / LOW_HIGH_DEGREES;

  // start the Ethernet connection and the server:
  Ethernet.begin(mymac, myip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start UDP
  udpSteer.begin(steerUDPport);
  udpNTRIP.begin(NTRIPUDPport);

  Serial.println("Setup complete, waiting for AgOpenGPS");

  adc.setSampleRate(ADS1115_REG_CONFIG_DR_128SPS); //128 samples per second
  adc.setGain(ADS1115_REG_CONFIG_PGA_6_144V);


#ifdef USB_PD
  tcpm_init(0);  // **USB-PD**
  delay(50);
  pd_init(0);
  delay(50);
#endif

}// End of Setup

void loop()
{
  int currentRoll = 0;
  int rollLeft = 0;
  int steerLeft = 0;
  
#ifdef USB_PD
  if (LOW == digitalRead(usb_pd_int_pin)) {       // **USB-PD**
      Serial.println("-------------------------int--------------------------------");
      tcpc_alert(0);
      Serial.println("------------------------endint------------------------------");
  }
  pd_run_state_machine(0);
#endif

  ReceiveUdp();

  // Loop triggers every 100 msec and sends back gyro heading, and roll, steer angle etc
  currentTime = millis();

  // NMEA received????
  if (Serial2.available() || Serial7.available())
  {
    if (currentTime > (lastNMEATime + 60))
    {
        //----------------------------------------------------------------------------
        //Serial Send to agopenGPS

        int16_t sa = (int16_t)(steerAngleActual * 100);

 
        AOG[5] = (uint8_t)sa;
        AOG[6] = sa >> 8;

        if (useCMPS)
        {
          Wire1.beginTransmission(CMPS14_ADDRESS);
          Wire1.write(0x02);
          Wire1.endTransmission();

          Wire1.requestFrom(CMPS14_ADDRESS, 2);
          while (Wire1.available() < 2);

          //the heading x10
          AOG[8] = Wire1.read();
          AOG[7] = Wire1.read();

          Wire1.beginTransmission(CMPS14_ADDRESS);
          Wire1.write(0x1C);
          Wire1.endTransmission();

          Wire1.requestFrom(CMPS14_ADDRESS, 2);
          while (Wire1.available() < 2);

          //the roll x10
          AOG[10] = Wire1.read();
          AOG[9] = Wire1.read();
        }
        else if (useBNO08x)
        {
          if (bno08x.dataAvailable() == true)
          {
            bno08xHeading = (bno08x.getYaw()) * CONST_180_DIVIDED_BY_PI; // Convert yaw / heading to degrees
            bno08xHeading = -bno08xHeading; //BNO085 counter clockwise data to clockwise data

            if (bno08xHeading < 0 && bno08xHeading >= -180) //Scale BNO085 yaw from [-180°;180°] to [0;360°]
            {
              bno08xHeading = bno08xHeading + 360;
            }

            bno08xRoll = (bno08x.getRoll()) * CONST_180_DIVIDED_BY_PI; //Convert roll to degrees
            //bno08xPitch = (bno08x.getPitch())* CONST_180_DIVIDED_BY_PI; // Convert pitch to degrees

            bno08xHeading10x = (int16_t)(bno08xHeading * 10);
            bno08xRoll10x = (int16_t)(bno08xRoll * 10);

            //Serial.print(bno08xHeading10x);
            //Serial.print(",");
            //Serial.println(bno08xRoll10x);

            //the heading x10
            AOG[7] = (uint8_t)bno08xHeading10x;
            AOG[8] = bno08xHeading10x >> 8;


            //the roll x10
            AOG[9] = (uint8_t)bno08xRoll10x;
            AOG[10] = bno08xRoll10x >> 8;
          }
        }
        else
        {
          //heading
          AOG[7] = (uint8_t)9999;
          AOG[8] = 9999 >> 8;

          //roll
          AOG[9] = (uint8_t)8888;
          AOG[10] = 8888 >> 8;

          if (rollLeft) 
            currentRoll = currentRoll - 10;
          else 
            currentRoll = currentRoll + 10;

          if (abs(currentRoll) > 20)
          {
            if(rollLeft)
            {
              rollLeft = 0;
            }
            else
            {
              rollLeft = 1;
            }
          }
          
          //roll
          AOG[9] = (uint8_t)currentRoll;
          AOG[10] = currentRoll >> 8;
        }

        AOG[11] = switchByte;
        AOG[12] = (uint8_t)pwmDisplay;

        //checksum
        int16_t CK_A = 0;
        for (uint8_t i = 2; i < AOGSize - 1; i++)
          CK_A = (CK_A + AOG[i]);

        AOG[AOGSize - 1] = CK_A;

        //off to AOG
        SendUdp(AOG, sizeof(AOG), ipDestination, AOGUDPport);

        // Stop sending the helloAgIO message
        helloCounter = 0;

        F9P_1state = 0;
        F9P_2state = 0;
        F9P_1cnt = 0;
        F9P_2cnt = 0;
    }
    
    while (Serial7.available()) 
    {
      if (F9P_1cnt < NMEA_MAX_BYTES)
      {
        F9P_1[F9P_1cnt] = Serial7.read();
        if ((F9P_1cnt & 0xfff0) && F9P_1[F9P_1cnt] == 0x0a && F9P_1[F9P_1cnt - 1] == 0x0d)  F9P_1state++;    // <CR><LF>" detected in NMEA string => got full sentence
        if (F9P_1state == NO_OF_NMEA_SENTENCES)  noOfF9Ps |= 1;  // detect F9P as valid when correct no of sentences is received once
        F9P_1cnt++;
      } 
      else 
      {
        Serial7.read();
        F9P_1cnt = 0;
        F9P_1state = 0;
      }
    }

    while (Serial2.available())
    {
      if (F9P_2cnt < NMEA_MAX_BYTES)
      {
        F9P_2[F9P_2cnt] = Serial2.read();
        if ((F9P_2cnt & 0xfff0) && F9P_2[F9P_2cnt] == 0x0a && F9P_2[F9P_2cnt - 1] == 0x0d)  F9P_2state++;    // <CR><LF>" detected in NMEA string => got full sentence
        if (F9P_2state == NO_OF_NMEA_SENTENCES)  noOfF9Ps |= 2;
        F9P_2cnt++;
      } 
      else 
      {
        Serial2.read();
        F9P_2cnt = 0;
        F9P_2state = 0;
      }
    }

    if ((noOfF9Ps == 3 && F9P_1state == NO_OF_NMEA_SENTENCES && F9P_2state == NO_OF_NMEA_SENTENCES) || 
        (noOfF9Ps == 1 && F9P_1state == NO_OF_NMEA_SENTENCES) || 
        (noOfF9Ps == 2 && F9P_2state == NO_OF_NMEA_SENTENCES))
    {
//      digitalWrite(debug0, HIGH);
      uint16_t i;
      for (i = F9P_1cnt; i < (F9P_1cnt + F9P_2cnt); i++) 
          F9P_1[i] = F9P_2[i-F9P_1cnt];  // concat strings
      SendUdp(F9P_1, i, ipDestination, GNSSUDPport);
  
//      for (uint16_t j=0; j<i; j++) Serial.write(F9P_1[j]);  // debug
//      Serial.print("Units: ");
//      Serial.println(noOfF9Ps);
//      digitalWrite(debug0, LOW);
      F9P_1cnt = 0;
      F9P_1state = 0;
      F9P_2cnt = 0;
      F9P_2state = 0;
    }
   
    lastNMEATime = currentTime;
  }
  

  if (currentTime - lastTime >= LOOP_TIME)
  {
    lastTime = currentTime;

    //reset debounce
    encEnable = true;

    //If connection lost to AgOpenGPS, the watchdog will count up and turn off steering
    if (watchdogTimer++ > 250) watchdogTimer = WATCHDOG_FORCE_VALUE;

    //read all the switches
    workSwitch = digitalRead(WORKSW_PIN);  // read work switch

    if (steerConfig.SteerSwitch == 1)         //steer switch on - off
    {
      steerSwitch = digitalRead(STEERSW_PIN); //read auto steer enable switch open = 0n closed = Off
    }
    else if (steerConfig.SteerButton == 1)    //steer Button momentary
    {
      reading = digitalRead(STEERSW_PIN);
      if (reading == LOW && previous == HIGH)
      {
        if (currentState == 1)
        {
          currentState = 0;
          steerSwitch = 0;
        }
        else
        {
          currentState = 1;
          steerSwitch = 1;
        }
      }
      previous = reading;
    }
    else                                      // No steer switch and no steer button
    {
      // So set the correct value. When guidanceStatus = 1,
      // it should be on because the button is pressed in the GUI
      // But the guidancestatus should have set it off first
      if (guidanceStatus == 1 && steerSwitch == 1 && previous == 0)
      {
        steerSwitch = 0;
        previous = 1;
      }

      // This will set steerswitch off and make the above check wait until the guidanceStatus has gone to 0
      if (guidanceStatus == 0 && steerSwitch == 0 && previous == 1)
      {
        steerSwitch = 1;
        previous = 0;
      }
    }

    if (steerConfig.ShaftEncoder && pulseCount >= steerConfig.PulseCountMax)
    {
      steerSwitch = 1; // reset values like it turned off
      currentState = 1;
      previous = 0;
    }

    // Current sensor?
    if (steerConfig.CurrentSensor)
    {
      int16_t analogValue = analogRead(ANALOG_SENSOR_PIN);

      // When the current sensor is reading current high enough, shut off
      if (abs(((analogValue - 512)) / 10.24) >= steerConfig.PulseCountMax) //amp current limit switch off
      {
        steerSwitch = 1; // reset values like it turned off
        currentState = 1;
        previous = 0;
      }
    }

    // Pressure sensor?
    if (steerConfig.PressureSensor)
    {
      int16_t analogValue = analogRead(ANALOG_SENSOR_PIN);

      // Calculations below do some assumptions, but we should be close?
      // 0-250bar sensor 4-20ma with 150ohm 1V - 5V -> 62,5 bar/V
      // 5v  / 1024 values -> 0,0048828125 V/bit
      // 62,5 * 0,0048828125 = 0,30517578125 bar/count
      // 1v = 0 bar = 204,8 counts
      int16_t steeringWheelPressureReading = (analogValue - 204) * 0.30517578125;

      // When the pressure sensor is reading pressure high enough, shut off
      if (steeringWheelPressureReading >= steerConfig.PulseCountMax)
      {
        steerSwitch = 1; // reset values like it turned off
        currentState = 1;
        previous = 0;
      }
    }

    remoteSwitch = digitalRead(REMOTE_PIN); //read auto steer enable switch open = 0n closed = Off
    switchByte = 0;
    switchByte |= (remoteSwitch << 2); //put remote in bit 2
    switchByte |= (steerSwitch << 1);   //put steerswitch status in bit 1 position
    switchByte |= workSwitch;

    /*
      #if Relay_Type == 1
        SetRelays();       //turn on off section relays
      #elif Relay_Type == 2
        SetuTurnRelays();  //turn on off uTurn relays
      #endif
    */

    //get steering position
    if (steerConfig.SingleInputWAS)   //Single Input ADS
    {
      adc.setMux(ADS1115_REG_CONFIG_MUX_SINGLE_0);
      steeringPosition = adc.getConversion();
      adc.triggerConversion();//ADS1115 Single Mode

      steeringPosition = (steeringPosition >> 1); //bit shift by 2  0 to 13610 is 0 to 5v
    }
    else    //ADS1115 Differential Mode
    {
      adc.setMux(ADS1115_REG_CONFIG_MUX_DIFF_0_1);
      steeringPosition = adc.getConversion();
      adc.triggerConversion();

      steeringPosition = (steeringPosition >> 1); //bit shift by 2  0 to 13610 is 0 to 5v
    }

    //DETERMINE ACTUAL STEERING POSITION

    //convert position to steer angle. 32 counts per degree of steer pot position in my case
    //  ***** make sure that negative steer angle makes a left turn and positive value is a right turn *****
    if (steerConfig.InvertWAS)
    {
      steeringPosition = (steeringPosition - 6805  - steerSettings.wasOffset);   // 1/2 of full scale
      steerAngleActual = (float)(steeringPosition) / -steerSettings.steerSensorCounts;
    }
    else
    {
      steeringPosition = (steeringPosition - 6805  + steerSettings.wasOffset);   // 1/2 of full scale
      steerAngleActual = (float)(steeringPosition) / steerSettings.steerSensorCounts;
    }

    //Ackerman fix
    if (steerAngleActual < 0) steerAngleActual = (steerAngleActual * steerSettings.AckermanFix);

    if (watchdogTimer < WATCHDOG_THRESHOLD)
    {
      //Enable H Bridge for IBT2, hyd aux, etc for cytron
      if (steerConfig.CytronDriver)
      {
        if (steerConfig.IsRelayActiveHigh)
        {
          digitalWrite(PWM2_RPWM, 0);
        }
        else
        {
          digitalWrite(PWM2_RPWM, 1);
        }
      }
      else digitalWrite(DIR1_RL_ENABLE, 1);

      steerAngleError = steerAngleActual - steerAngleSetPoint;   //calculate the steering error
      //if (abs(steerAngleError)< steerSettings.lowPWM) steerAngleError = 0;

      calcSteeringPID();  //do the pid
      motorDrive();       //out to motors the pwm value
    }
    else
    {
      //we've lost the comm to AgOpenGPS, or just stop request
      //Disable H Bridge for IBT2, hyd aux, etc for cytron
      if (steerConfig.CytronDriver)
      {
        if (steerConfig.IsRelayActiveHigh)
        {
          digitalWrite(PWM2_RPWM, 1);
        }
        else
        {
          digitalWrite(PWM2_RPWM, 0);
        }
      }
      else digitalWrite(DIR1_RL_ENABLE, 0); //IBT2

      pwmDrive = 0; //turn off steering motor
      motorDrive(); //out to motors the pwm value
      pulseCount = 0;
    }

    //send empty pgn to AgIO to show activity
    if (++helloCounter > 10)
    {
      SendUdp(helloAgIO, sizeof(helloAgIO), ipDestination, AOGUDPport);
      helloCounter = 0;
    }
  } //end of timed loop

  //This runs continuously, outside of the timed loop, keeps checking for new udpData, turn sense
  delay(1);

  if (encEnable)
  {
    thisEnc = digitalRead(REMOTE_PIN);
    if (thisEnc != lastEnc)
    {
      lastEnc = thisEnc;
      if ( lastEnc) EncoderFunc();
    }
  }

} // end of main loop


// UDP Receive
void ReceiveUdp()
{
  uint16_t len = udpSteer.parsePacket();

  if (len > 0)
  {
    //Serial.print("ReceiveUdp: ");   //////////////////////////////
    //Serial.println(len);            //////////////////////////////
  }

  if (len > 13)
  {
    udpSteer.read(udpData, UDP_TX_PACKET_MAX_SIZE);

    if (udpData[0] == 0x80 && udpData[1] == 0x81 && udpData[2] == 0x7F) //Data
    {
      if (udpData[3] == 0xFE)  //254
      {
        gpsSpeed = ((float)(udpData[5] | udpData[6] << 8)) * 0.1;

        guidanceStatus = udpData[7];

        //Bit 8,9    set point steer angle * 100 is sent
        steerAngleSetPoint = ((float)(udpData[8] | udpData[9] << 8)) * 0.01; //high low bytes

        //Serial.println(gpsSpeed);

        if ((bitRead(guidanceStatus, 0) == 0) || (gpsSpeed < 0.1) || (steerSwitch == 1))
        {
          watchdogTimer = WATCHDOG_FORCE_VALUE; //turn off steering motor
        }
        else          //valid conditions to turn on autosteer
        {
          watchdogTimer = 0;  //reset watchdog
        }

        //Bit 10 Tram
        tram = udpData[10];

        //Bit 11
        relay = udpData[11];

        //Bit 12
        relayHi = udpData[12];

/*
        //----------------------------------------------------------------------------
        //Serial Send to agopenGPS

        int16_t sa = (int16_t)(steerAngleActual * 100);

 
        AOG[5] = (uint8_t)sa;
        AOG[6] = sa >> 8;

        if (useCMPS)
        {
          Wire1.beginTransmission(CMPS14_ADDRESS);
          Wire1.write(0x02);
          Wire1.endTransmission();

          Wire1.requestFrom(CMPS14_ADDRESS, 2);
          while (Wire1.available() < 2);

          //the heading x10
          AOG[8] = Wire1.read();
          AOG[7] = Wire1.read();

          Wire1.beginTransmission(CMPS14_ADDRESS);
          Wire1.write(0x1C);
          Wire1.endTransmission();

          Wire1.requestFrom(CMPS14_ADDRESS, 2);
          while (Wire1.available() < 2);

          //the roll x10
          AOG[10] = Wire1.read();
          AOG[9] = Wire1.read();
        }
        else if (useBNO08x)
        {
          if (bno08x.dataAvailable() == true)
          {
            bno08xHeading = (bno08x.getYaw()) * CONST_180_DIVIDED_BY_PI; // Convert yaw / heading to degrees
            bno08xHeading = -bno08xHeading; //BNO085 counter clockwise data to clockwise data

            if (bno08xHeading < 0 && bno08xHeading >= -180) //Scale BNO085 yaw from [-180°;180°] to [0;360°]
            {
              bno08xHeading = bno08xHeading + 360;
            }

            bno08xRoll = (bno08x.getRoll()) * CONST_180_DIVIDED_BY_PI; //Convert roll to degrees
            //bno08xPitch = (bno08x.getPitch())* CONST_180_DIVIDED_BY_PI; // Convert pitch to degrees

            bno08xHeading10x = (int16_t)(bno08xHeading * 10);
            bno08xRoll10x = (int16_t)(bno08xRoll * 10);

            //Serial.print(bno08xHeading10x);
            //Serial.print(",");
            //Serial.println(bno08xRoll10x);

            //the heading x10
            AOG[7] = (uint8_t)bno08xHeading10x;
            AOG[8] = bno08xHeading10x >> 8;


            //the roll x10
            AOG[9] = (uint8_t)bno08xRoll10x;
            AOG[10] = bno08xRoll10x >> 8;
          }
        }
        else
        {
          //heading
          AOG[7] = (uint8_t)9999;
          AOG[8] = 9999 >> 8;

          //roll
          AOG[9] = (uint8_t)8888;
          AOG[10] = 8888 >> 8;

          if (rollLeft) 
            currentRoll = currentRoll - 10;
          else 
            currentRoll = currentRoll + 10;

          if (abs(currentRoll) > 20)
          {
            if(rollLeft)
            {
              rollLeft = 0;
            }
            else
            {
              rollLeft = 1;
            }
          }
          
          //roll
          AOG[9] = (uint8_t)currentRoll;
          AOG[10] = currentRoll >> 8;
        }

        AOG[11] = switchByte;
        AOG[12] = (uint8_t)pwmDisplay;

        //checksum
        int16_t CK_A = 0;
        for (uint8_t i = 2; i < AOGSize - 1; i++)
          CK_A = (CK_A + AOG[i]);

        AOG[AOGSize - 1] = CK_A;

        //off to AOG
        SendUdp(AOG, sizeof(AOG), ipDestination, AOGUDPport);

        // Stop sending the helloAgIO message
        helloCounter = 0;
*/
        //Serial.println(steerAngleActual);
        //--------------------------------------------------------------------------
      }

      //steer settings
      else if (udpData[3] == 0xFC)  //252
      {
        //PID values
        steerSettings.Kp = ((float)udpData[5]);   // read Kp from AgOpenGPS

        steerSettings.highPWM = udpData[6]; // read high pwm

        steerSettings.lowPWM = (float)udpData[7];   // read lowPWM from AgOpenGPS

        steerSettings.minPWM = udpData[8]; //read the minimum amount of PWM for instant on

        steerSettings.steerSensorCounts = udpData[9]; //sent as setting displayed in AOG

        steerSettings.wasOffset = (udpData[10]);  //read was zero offset Lo

        steerSettings.wasOffset |= (udpData[11] << 8);  //read was zero offset Hi

        steerSettings.AckermanFix = (float)udpData[12] * 0.01;

        //crc
        //udpData[13];

        //store in EEPROM
        EEPROM.put(10, steerSettings);

        // for PWM High to Low interpolator
        highLowPerDeg = ((float)(steerSettings.highPWM - steerSettings.lowPWM)) / LOW_HIGH_DEGREES;
      }

      else if (udpData[3] == 0xFB)  //251 FB - SteerConfig
      {
        uint8_t sett = udpData[5]; //setting0

        if (bitRead(sett, 0)) steerConfig.InvertWAS = 1; else steerConfig.InvertWAS = 0;
        if (bitRead(sett, 1)) steerConfig.IsRelayActiveHigh = 1; else steerConfig.IsRelayActiveHigh = 0;
        if (bitRead(sett, 2)) steerConfig.MotorDriveDirection = 1; else steerConfig.MotorDriveDirection = 0;
        if (bitRead(sett, 3)) steerConfig.SingleInputWAS = 1; else steerConfig.SingleInputWAS = 0;
        if (bitRead(sett, 4)) steerConfig.CytronDriver = 1; else steerConfig.CytronDriver = 0;
        if (bitRead(sett, 5)) steerConfig.SteerSwitch = 1; else steerConfig.SteerSwitch = 0;
        if (bitRead(sett, 6)) steerConfig.SteerButton = 1; else steerConfig.SteerButton = 0;
        if (bitRead(sett, 7)) steerConfig.ShaftEncoder = 1; else steerConfig.ShaftEncoder = 0;

        steerConfig.PulseCountMax = udpData[6];

        //was speed
        //udpData[7];

        sett = udpData[8]; //setting1 - Danfoss valve etc

        if (bitRead(sett, 0)) steerConfig.IsDanfoss = 1; else steerConfig.IsDanfoss = 0;
        if (bitRead(sett, 1)) steerConfig.PressureSensor = 1; else steerConfig.PressureSensor = 0;
        if (bitRead(sett, 2)) steerConfig.CurrentSensor = 1; else steerConfig.CurrentSensor = 0;

        //crc
        //udpData[13];

        EEPROM.put(40, steerConfig);

        //reset the arduino
        resetFunc();

      }//end FB

    } //end if 80 81 7F
  }

  
  len = udpNTRIP.parsePacket();
  if (len > 0)
  {
    udpNTRIP.read(udpData, UDP_TX_PACKET_MAX_SIZE);
    for (uint16_t i = 0; i < len; i++)   // forward NTRIP data to F9Ps
    {
        Serial2.write(udpData[i]);
        Serial7.write(udpData[i]);
    }
  }
}

void SendUdp(uint8_t *data, uint16_t datalen, IPAddress dip, uint16_t dport)
{
  udpSteer.beginPacket(dip, dport);
  udpSteer.write(data, datalen);
  udpSteer.endPacket();
}

//ISR Steering Wheel Encoder
void EncoderFunc()
{
  if (encEnable)
  {
    pulseCount++;
    encEnable = false;
  }
}


void pd_process_source_cap_callback(int port, int cnt, uint32_t *src_caps)   // **USB-PD**
{
  digitalWrite(debug_led_pin, HIGH);
}
