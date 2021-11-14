// mods by GoRoNb:
// - main loop cycle: 8ms instead of 40 => less jitter in WAS/roll/heading reading
// - no delays any more => less jitter in general
// - watchdogtime based on time instead of "main cycles"
// - CMPS14: sensor may be mounted with != 0° (see #define myTwist)
// - CMPS14: request only useful bytes

////////////////// User Settings /////////////////////////
  /* 
   *  Wheel angle sensor zero point...
   *  
   * 3320 minus 127 = 3193. 1288 counts per 1 volt
   * The 127 (half of 255) is so you can take WAS zero of 3320 
   * from 3193 to 3347. Zero from AOG is sent as a number from 0 to 255. 
   * 
   * Leave at 3193 - Only change if using a factory OEM wheel angle Sensor
   * Put your wheels straight forward, adjust WAS physical position
   * So it puts out 2.5v. Do a good installation!
   * 
   * Factory WAS - Wheels pointing forward, measure the voltage. 
   * Example 2.2v - So, 2.5 minus 2.2 = 0.3v times 
   * 1288 counts per volt = 386 counts. 3320 - 386 - 127 = 2706.
   * So your new WAS_ZERO is 2706.
   */
  #define WAS_ZERO 3193    

  //#################################### CMPS14 settings ########################################
  #define BNO80 0      // set to 1 for CMPS14                                                  ##
  #define myTwist 45   // twist of CMPS14 around vertical axis (range: 0..359)                 ##
                       // Example: 45 => CMPS14 heading is twisted 45° CCW vs tractor heading) ##
  //#############################################################################################

  //How many degrees before decreasing Max PWM
  #define LOW_HIGH_DEGREES 5.0

  //value for max step in roll noise 5 is slow, 30 is too fast (noisier)
  #define ROLL_DSP_STEP 5

  //PWM Frequency -> 490hz (default) = 0 and -> 122hz = 1  -> 3921hz = 2
  #define PWM_Frequency 0
  
/////////////////////////////////////////////

  // if not in eeprom, overwrite 
  #define EEP_Ident 4310  

  //version in AOG ex. v4.3.10 -> 4+3+10=17
  #define aogVersion 17

  #define TIMEOUT 500   // GoRoNb: milliseconds of no received data (replacement for 'watchdog')

  //   ***********  Motor drive connections  **************888
  //Connect ground only for cytron, Connect Ground and +5v for IBT2
    
  //Dir1 for Cytron Dir, Both L and R enable for IBT2
  #define DIR1_RL_ENABLE  4  //PD4

  //PWM1 for Cytron PWM, Left PWM for IBT2
  #define PWM1_LPWM  3  //PD3

  //Not Connected for Cytron, Right PWM for IBT2
  #define PWM2_RPWM  9 //D9

  //--------------------------- Switch Input Pins ------------------------
  #define STEERSW_PIN 6 //PD6
  #define WORKSW_PIN 7  //PD7
  #define REMOTE_PIN 8  //PB0

  // MCP23017 I2C address is 0x20(32)
  #define Addr 0x20
  #define PortA 0x12
  #define PortB 0x13

  #include <Wire.h>
  #include <EEPROM.h> 
  #include "zADS1015.h"
  Adafruit_ADS1115 ads;     // Use this for the 16-bit version ADS1115

  //GY-45 (1C)
  //installed Sparkfun, Adafruit MMA8451 (1D)   
  #include "MMA8452_AOG.h"  //MMA inclinometer
  
  MMA8452 MMA1D(0x1D);   
  MMA8452 MMA1C(0x1C);
      
  uint16_t x_ , y_ , z_;        

  #include "BNO055_AOG.h"  // BNO055 IMU
  #define A 0X28             //I2C address selection pin LOW
  #define B 0x29             //                          HIGH
  #define RAD2GRAD 57.2957795
  BNO055 IMU(A);  // create an instance


  //cmps14 values
  #define CMPS11_ADDRESS 0x60   // Address of CMPS11 shifted right one bit for arduino wire library
  unsigned char high_bearing, low_bearing, high_roll, low_roll;
  char pitch8;
  int bearing16, roll16, pitch16, cosinus, sinus;
  long roll32;
  float lastXeRoll = 0;
  int mysine[] = {0,17,35,51,63,81,94,105,114,121,125,127,126,123,118,110,100,88,74,59,43,26,8,-9,-27,-44,-60,-75,-89,-101,-111,-119,-124,-127,-128,-126,-122,-115,-106,-95,-82,-68,-52,-36,-18};

 
  //loop time variables in microseconds  
  const unsigned int LOOP_TIME = 8;  // GoRoNb: set to 8 (= minimum delay for ADS1115) instead of 40;  //25Hz    
  unsigned long lastTime = LOOP_TIME;
  unsigned long currentTime = LOOP_TIME;
  unsigned long lastCtlMsg = -TIMEOUT; // time of last control msg received  // GoRoNb
  byte serialResetTimer = 100; //if serial buffer is getting full, empty it
  
  //inclinometer variables
  float rollK = 0, Pc = 0.0, G = 0.0, P = 1.0, Xp = 0.0, Zp = 0.0;
  float XeRoll = 0;
  float lastRoll=0, diff=0;
  int roll = 0;

  //Kalman control variances
  const float varRoll = 0.2; // variance, larger is more filtering
  const float varProcess = 0.01; //process, smaller is more filtering
  
  //Program flow
  bool isDataFound = false, isSettingFound = false, isMachineFound=false, isAogSettingsFound = false; 
  bool MMAinitialized = false, isRelayActiveHigh = true;
  int header = 0, tempHeader = 0, temp, EEread = 0;
  byte relay = 0, relayHi = 0, uTurn = 0;
  byte remoteSwitch = 0, workSwitch = 0, steerSwitch = 1, switchByte = 0;
  float distanceFromLine = 0, gpsSpeed = 0;
  
  //steering variables
  float steerAngleActual = 0;
  float steerAngleSetPoint = 0; //the desired angle from AgOpen
  int steeringPosition = 0; //from steering sensor
  float steerAngleError = 0; //setpoint - actual
  
    //pwm variables
  int pwmDrive = 0, pwmDisplay = 0;
  float pValue = 0;
  float errorAbs = 0;
  float highLowPerDeg = 0; 

  //Steer switch button  ***********************************************************************************************************
  byte currentState = 1;
  byte reading;
  byte previous = 0;

  byte pulseCount = 0; // Steering Wheel Encoder
  bool encEnable = false; //debounce flag
  byte thisEnc = 0, lastEnc = 0;

     //Variables for settings  
   struct Storage {
      float Ko = 0.0f;  //overall gain
      float Kp = 0.0f;  //proportional gain
      float lowPWM = 0.0f;  //band of no action
      float Kd = 0.0f;  //derivative gain 
      float steeringPositionZero = 3320.0;
      byte minPWM=0;
      byte highPWM=100;//max PWM value
      float steerSensorCounts=10;        
  };  Storage steerSettings;  //27 bytes

    //Variables for settings - 0 is false  
   struct Setup {
      byte InvertWAS = 0;
      byte InvertRoll = 0;
      byte MotorDriveDirection = 0;
      byte SingleInputWAS = 1;
      byte CytronDriver = 0;
      byte SteerSwitch = 0;
      byte UseMMA_X_Axis = 0;
      byte ShaftEncoder = 0;
      
      byte BNOInstalled = 0;
      byte InclinometerInstalled = 0;   // set to 0 for none
                                        // set to 1 if DOGS2 Inclinometer is installed and connected to ADS pin A2
                                        // set to 2 if MMA8452 installed GY-45 (1C)
                                        // set to 3 if MMA8452 installed Sparkfun, Adafruit MMA8451 (1D)      
      byte maxSteerSpeed = 20;
      byte minSteerSpeed = 1;
      byte PulseCountMax = 5; 
      byte AckermanFix = 100;     //sent as percent
      byte isRelayActiveHigh = 0; //if zero, active low (default)

  };  Setup aogSettings;          //15 bytes

  //reset function
  void(* resetFunc) (void) = 0;

void setup()
{ 
  //PWM rate settings. Set them both the same!!!!

  if (PWM_Frequency == 1) 
  { 
    TCCR2B = TCCR2B & B11111000 | B00000110;    // set timer 2 to 256 for PWM frequency of   122.55 Hz
    TCCR1B = TCCR1B & B11111000 | B00000100;    // set timer 1 to 256 for PWM frequency of   122.55 Hz
  }

  else if (PWM_Frequency == 2)
  {
    TCCR1B = TCCR1B & B11111000 | B00000010;    // set timer 1 to 8 for PWM frequency of  3921.16 Hz
    TCCR2B = TCCR2B & B11111000 | B00000010;    // set timer 2 to 8 for PWM frequency of  3921.16 Hx

  }
  
  //keep pulled high and drag low to activate, noise free safe   
  pinMode(WORKSW_PIN, INPUT_PULLUP); 
  pinMode(STEERSW_PIN, INPUT_PULLUP); 
  pinMode(REMOTE_PIN, INPUT_PULLUP); 
  pinMode(DIR1_RL_ENABLE, OUTPUT);
  
  if (aogSettings.CytronDriver) pinMode(PWM2_RPWM, OUTPUT); 
  
  //set up communication
  Wire.begin();
  Serial.begin(38400);

  //50Khz I2C
  TWBR = 144;
  
  //PortB configured as output
  //MCP_Write(0x01,0x00);  
  delay(300);

  EEPROM.get(0, EEread);              // read identifier
    
  if (EEread != EEP_Ident)   // check on first start and write EEPROM
  {           
    EEPROM.put(0, EEP_Ident);
    EEPROM.put(2, WAS_ZERO);
    EEPROM.put(10, steerSettings);   
    EEPROM.put(40, aogSettings);
  }
  else 
  { 
    //EEPROM.get(2, EEread);            // read SteerPosZero
    EEPROM.get(10, steerSettings);     // read the Settings
    EEPROM.get(40, aogSettings);
  }
  
  // for PWM High to Low interpolator
  highLowPerDeg = (steerSettings.highPWM - steerSettings.lowPWM) / LOW_HIGH_DEGREES;

  // BNO055 init
  if (aogSettings.BNOInstalled) 
  { 
    IMU.init();  
    IMU.setExtCrystalUse(true);   //use external 32K crystal
  }  
  	
  if (aogSettings.InclinometerInstalled == 2 )
  { 
      // MMA8452 (1) Inclinometer
      MMAinitialized = MMA1C.init();
      
      if (MMAinitialized)
      {
        MMA1C.setDataRate(MMA_12_5hz);
        MMA1C.setRange(MMA_RANGE_2G);
        MMA1C.setHighPassFilter(false); 
      }
      else Serial.println("MMA init fails!!");
  }
  else if (aogSettings.InclinometerInstalled == 3 )
  { 
      // MMA8452 (1) Inclinometer
      MMAinitialized = MMA1D.init();
        
      if (MMAinitialized)
      {
        MMA1D.setDataRate(MMA_12_5hz);
        MMA1D.setRange(MMA_RANGE_2G);
        MMA1D.setHighPassFilter(false); 
      }
      else Serial.println("MMA init fails!!");
  }  
  Serial.println("Waiting for AgOpenGPS");

  // GoRoNb: CMPS14 coefficients:
  uint16_t tempTwist = (myTwist + 356) % 360;
  byte interpol = tempTwist % 8;   // linear interpolation of sine table
  sinus = mysine[tempTwist >> 3] * (7 - interpol) + mysine[((tempTwist >> 3) +1) % 45] * interpol;
  sinus = sinus / 7;
  tempTwist = (tempTwist + 90) % 360;
  cosinus = mysine[tempTwist >> 3] * (7 - interpol) + mysine[((tempTwist >> 3) +1) % 45] * interpol;
  cosinus = cosinus / 7;

  //sinus = mysine[myTwist >> 3];
  //cosinus = mysine[((myTwist + 90) % 360) >> 3];

}// End of Setup

void loop()
{
	// Loop triggers every 100 msec and sends back gyro heading, and roll, steer angle etc	 
	currentTime = millis();
 
	if (currentTime - lastTime >= LOOP_TIME)
	{
		lastTime = currentTime;

    //reset debounce
    encEnable = true;
   
    //If connection lost to AgOpenGPS, the watchdog will count up and turn off steering
    //if (watchdogTimer++ > 250) watchdogTimer = 12;

    //clean out serial buffer to prevent buffer overflow
    if (serialResetTimer++ > 100)
    {
      while (Serial.available() > 0) char t = Serial.read();
      serialResetTimer = 0;
    }
    
    if (aogSettings.BNOInstalled)
        IMU.readIMU();
    
    //DOGS2 inclinometer
    if (aogSettings.InclinometerInstalled == 1)
    {
        roll = ((ads.readADC_SingleEnded(2))); // 24,000 to 2700
        roll = (roll - 13300);
        roll = roll >> 5; //-375 to 375 -25 deg to +25 deg

          // limit the differential  
          diff = roll - lastRoll;
          if (diff > ROLL_DSP_STEP ) roll = lastRoll + ROLL_DSP_STEP;      
          else if (diff < -ROLL_DSP_STEP) roll = lastRoll - ROLL_DSP_STEP;
          lastRoll = roll;    

        //if not positive when rolling to the right
        if (aogSettings.InvertRoll)
          roll *= -1.0; 

        //for Kalman            
        rollK = roll;        
    }  //-----------------------------------------------------------------------------
    
    // MMA8452 Inclinometer (1C)           
    else if (aogSettings.InclinometerInstalled == 2)
    {
        if (MMAinitialized)
        {
          MMA1C.getRawData(&x_, &y_, &z_);

          if (aogSettings.UseMMA_X_Axis) 
              roll= x_; //Conversion uint to int
          else roll = y_;
                   
          //16 counts per degree (good for 0 - +/-18 degrees) 
          if (roll > 4200)  roll =  4200;
          if (roll < -4200) roll = -4200;
          roll = roll >> 3;  //divide by 8  +-525
          
          // limit the differential  
          diff = roll - lastRoll;
          if (diff > ROLL_DSP_STEP ) roll = lastRoll + ROLL_DSP_STEP;      
          else if (diff < -ROLL_DSP_STEP) roll = lastRoll - ROLL_DSP_STEP;
          lastRoll = roll; 

          //divide by 2 -268 to +268    -17 to +17 degrees
          roll = roll >> 1;
      
          //if not positive when rolling to the right
          if (aogSettings.InvertRoll)
            roll *= -1.0;   
      
          //divide by 16
          rollK = roll;
        }
    } //----------------------------------------------------------------------
    
    // MMA8452 Inclinometer (1D)        
    else if (aogSettings.InclinometerInstalled == 3)
    {
        if (MMAinitialized)
        {
          MMA1D.getRawData(&x_, &y_, &z_);

          if (aogSettings.UseMMA_X_Axis) 
              roll= x_; //Conversion uint to int
          else roll = y_;
                   
          //16 counts per degree (good for 0 - +/-18 degrees) 
          if (roll > 4200)  roll =  4200;
          if (roll < -4200) roll = -4200;
          roll = roll >> 3;  //divide by 8  +-525
          
          // limit the differential  
          diff = roll - lastRoll;
          if (diff > ROLL_DSP_STEP ) roll = lastRoll + ROLL_DSP_STEP;      
          else if (diff < -ROLL_DSP_STEP) roll = lastRoll - ROLL_DSP_STEP;
          lastRoll = roll; 

          //divide by 2 -268 to +268    -17 to +17 degrees
          roll = roll >> 1;
      
          //if not positive when rolling to the right
          if (aogSettings.InvertRoll)
            roll *= -1.0;   
      
          //divide by 16
          rollK = roll;
        }
    } //---------------------------------------------------------------------------------
                
    //Kalman filter
    Pc = P + varProcess;
    G = Pc / (Pc + varRoll);
    P = (1 - G) * Pc;
    Xp = XeRoll;
    Zp = Xp;
    XeRoll = G * (rollK - Zp) + Xp;     

    if(BNO80) // begin cmps14
    {
      Wire.beginTransmission(CMPS11_ADDRESS);  //starts communication with CMPS11
      Wire.write(0x02);                        //Sends the register we wish to start reading from (Bearing16 + Pitch8)
      Wire.endTransmission(CMPS11_ADDRESS);
      // Request 3 bytes from the CMPS11: 16 bit bearing, 8 bit pitch
      Wire.requestFrom(CMPS11_ADDRESS, 3);       

      if(Wire.available() <= 3);      // Wait for all bytes to come back
      high_bearing = Wire.read();
      low_bearing = Wire.read();
      pitch8 = Wire.read();

      bearing16 = (high_bearing << 8) | low_bearing;      // Calculate 16 bit angle (10 digits per degree)
      bearing16 = (bearing16 + myTwist * 10) % 3600;      // max 3600 = 360°
                                                          // heading = bearing16 * 1.6 (16 digits per degree)
      bearing16 = bearing16 + (bearing16 >> 1) + (bearing16 >> 4) + (bearing16 >> 5) + (bearing16 >> 7);  
      
      if (Serial.available() > 7) XeRoll = lastXeRoll; else  // only continue if no packet received already
      {
        Wire.beginTransmission(CMPS11_ADDRESS);  //starts communication with CMPS11
        Wire.write(0x1c);                        //Sends the register we wish to start reading from (Roll16)
        Wire.endTransmission(CMPS11_ADDRESS);
        // Request 3 bytes from the CMPS11: 16 bit bearing, 8 bit pitch
        Wire.requestFrom(CMPS11_ADDRESS, 2);       
  
        if(Wire.available() <= 2);      // Wait for all bytes to come back
        high_roll = Wire.read();
        low_roll = Wire.read();
    
        roll16 = (high_roll << 8) | low_roll;     // 10 digit per degree
        pitch16 = pitch8;   // 1 digit per degree
        if (roll16 > 900)   // sensor upside down I?
        {
          pitch16 = -pitch8;
          roll16 = -1800 + roll16;
        }
        if (roll16 < -900)   // sensor upside down II?
        {
          pitch16 = -pitch8;
          roll16 = 1800 + roll16;
        }
        roll32  = (long) roll16 * cosinus + 10 * pitch16 * sinus;  // * 128 effectively
          
        XeRoll = (float)roll32 / 80;  // 16 digits per degree
        lastXeRoll = XeRoll;
      }
    }      //end cmps14


    //read all the switches
    workSwitch = digitalRead(WORKSW_PIN);  // read work switch
    
    if (aogSettings.SteerSwitch == 1) //steer switch on - off
    {
      steerSwitch = digitalRead(STEERSW_PIN); //read auto steer enable switch open = 0n closed = Off
    }
    else   //steer Button momentary
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
    
    if (aogSettings.ShaftEncoder && pulseCount >= aogSettings.PulseCountMax ) 
    {
      steerSwitch = 1; // reset values like it turned off
      currentState = 1;
      previous = HIGH;
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
    //MCP_Write(PortB,relay);   
  
    //get steering position       
    if (aogSettings.SingleInputWAS)   //Single Input ADS
    {
      steeringPosition = ads.readADC_SingleEnded(0);    //ADS1115 Single Mode 
      
       steeringPosition = (steeringPosition >> 2); //bit shift by 2  0 to 6640 is 0 to 5v
    }    
    else    //ADS1115 Differential Mode
    {
      steeringPosition = ads.readADC_Differential_0_1(); //ADS1115 Differential Mode
           
      steeringPosition = (steeringPosition >> 2); //bit shift by 2  0 to 6640 is 0 to 5v
    }
   
    //DETERMINE ACTUAL STEERING POSITION
    steeringPosition = (steeringPosition - steerSettings.steeringPositionZero);   //read the steering position sensor
          
      //convert position to steer angle. 32 counts per degree of steer pot position in my case
      //  ***** make sure that negative steer angle makes a left turn and positive value is a right turn *****
    if (aogSettings.InvertWAS)
        steerAngleActual = (float)(steeringPosition) / -steerSettings.steerSensorCounts;
    else
        steerAngleActual = (float)(steeringPosition) / steerSettings.steerSensorCounts; 

    //Ackerman fix
    if (steerAngleActual < 0) steerAngleActual = (steerAngleActual * aogSettings.AckermanFix)/100;
    
    if (currentTime - lastCtlMsg < TIMEOUT)  // GoRoNb    if (watchdogTimer < 10)
      { 
       //Disable H Bridge for IBT2, hyd aux, etc for cytron
        if (aogSettings.CytronDriver) 
        {
          if (aogSettings.isRelayActiveHigh) 
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
        if (aogSettings.CytronDriver) 
        {
          if (aogSettings.isRelayActiveHigh) 
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
        pulseCount=0;
     }           
  	} //end of timed loop

  //This runs continuously, not timed //// Serial Receive Data/Settings /////////////////
  //delay (5);  // GoRoNb

  if (encEnable)
  {
    thisEnc = digitalRead(REMOTE_PIN);
    if (thisEnc != lastEnc)
    {
      lastEnc = thisEnc;
      if ( lastEnc) EncoderFunc();
    }
  }
  
  if (Serial.available() > 0 && !isDataFound && !isSettingFound && !isMachineFound && !isAogSettingsFound) 
  {
    int temp = Serial.read();
    header = tempHeader << 8 | temp;               //high,low bytes to make int
    tempHeader = temp;                             //save for next time
    if (header == 32766) isDataFound = true;        //Do we have a match?
    else if (header == 32764) isSettingFound = true;     //Do we have a match?
    else if (header == 32762) isMachineFound = true;     //Do we have a match?    
    else if (header == 32763) isAogSettingsFound = true;     //Do we have a match?    
  }

  //Data Header has been found, so the next 8 bytes are the usbData
  if (Serial.available() > 7 && isDataFound)
  {
    isDataFound = false;
 
    //was section control lo byte
    Serial.read();
    gpsSpeed = Serial.read() * 0.25;  //actual speed times 4, single byte

    //distance from the guidance line in mm
    distanceFromLine = (float)(Serial.read() << 8 | Serial.read());   //high,low bytes

    //set point steer angle * 100 is sent
    steerAngleSetPoint = ((float)(Serial.read() << 8 | Serial.read()))*0.01; //high low bytes

    //auto Steer is off if 32020,Speed is too slow, motor pos or footswitch open
    if (distanceFromLine == 32020 | distanceFromLine == 32000 
          | gpsSpeed < aogSettings.minSteerSpeed | gpsSpeed > aogSettings.maxSteerSpeed 
          | steerSwitch == 1 )
      {
        lastCtlMsg = millis() - TIMEOUT;  // GoRoNb: turn off steering motor
        serialResetTimer = 0; //if serial buffer is getting full, empty it
      }
    else          //valid conditions to turn on autosteer
      {
        lastCtlMsg = millis();  // GoRoNb: reset serial timeout
        serialResetTimer = 0; //if serial buffer is getting full, empty it
      }      
    Serial.read();
    Serial.read();

    //////////////////////////////////////////////////////////////////////////////////////

          //Serial Send to agopenGPS **** you must send 10 numbers ****
      Serial.print("127,253,");
      Serial.print((int)(steerAngleActual * 100)); //The actual steering angle in degrees
      Serial.print(",");
      Serial.print((int)(steerAngleSetPoint * 100));   //the setpoint originally sent
      Serial.print(",");
  
      // *******  if there is no gyro installed send 0
      if (aogSettings.BNOInstalled)
      {
        Serial.print((int)IMU.euler.head);       //heading in degrees * 16 from BNO
        Serial.print(",");
      }
      else if(BNO80)
      {
         Serial.print((int)bearing16);
        Serial.print(","); 
      }
     
      else Serial.print("0,"); 
      //No IMU installed      
      
      //*******  if no roll is installed, send 0
      if (aogSettings.InclinometerInstalled)
      {
        Serial.print((int)XeRoll);          //roll in degrees * 16
        Serial.print(",");                 //no Inclinometer installed
      }
      else if(BNO80)
      {
         Serial.print((int)XeRoll);
         Serial.print(","); 
      }
       
      else Serial.print("0,"); 

      //the status of switch inputs
      Serial.print(switchByte); //steering switch status 
      Serial.print(","); 
      Serial.print(pwmDisplay); //steering switch status 
      Serial.println(",0,0");
      Serial.flush();   // flush out buffer
  }

  //Machine Header has been found, so the next 8 bytes are the usbData
  if (Serial.available() > 7 && isMachineFound)
  {
    isMachineFound = false;

    relayHi = Serial.read();
    relay = Serial.read();    
    gpsSpeed = Serial.read() * 0.25;  //actual speed times 4, single byte
    uTurn = Serial.read();    
    
    Serial.read();
    Serial.read();
    Serial.read();
    Serial.read();
  }

  //ArdSettings has been found, so the next 8 bytes are the usbData
  if (Serial.available() > 7 && isAogSettingsFound)
  {
    isAogSettingsFound = false;

    byte checksum = 0;
    byte reed = 0;

    reed = Serial.read();
    checksum += reed;
    byte sett = reed; //setting0
     
    if (bitRead(sett,0)) aogSettings.InvertWAS = 1; else aogSettings.InvertWAS = 0;
    if (bitRead(sett,1)) aogSettings.InvertRoll = 1; else aogSettings.InvertRoll = 0;
    if (bitRead(sett,2)) aogSettings.MotorDriveDirection = 1; else aogSettings.MotorDriveDirection = 0;
    if (bitRead(sett,3)) aogSettings.SingleInputWAS = 1; else aogSettings.SingleInputWAS = 0;
    if (bitRead(sett,4)) aogSettings.CytronDriver = 1; else aogSettings.CytronDriver = 0;
    if (bitRead(sett,5)) aogSettings.SteerSwitch = 1; else aogSettings.SteerSwitch = 0;
    if (bitRead(sett,6)) aogSettings.UseMMA_X_Axis = 1; else aogSettings.UseMMA_X_Axis = 0;
    if (bitRead(sett,7)) aogSettings.ShaftEncoder = 1; else aogSettings.ShaftEncoder = 0;

    //set1 
    reed = Serial.read();
    checksum += reed;
    sett = reed;  //setting1     
    if (bitRead(sett,0)) aogSettings.BNOInstalled = 1; else aogSettings.BNOInstalled = 0;
    if (bitRead(sett,1)) aogSettings.isRelayActiveHigh = 1; else aogSettings.isRelayActiveHigh = 0;

    reed = Serial.read();
    checksum += reed;
    aogSettings.maxSteerSpeed = reed;  //actual speed

    reed = Serial.read();
    checksum += reed;
    aogSettings.minSteerSpeed = reed;    

    reed = Serial.read();
    checksum += reed;
    byte inc = reed;
    aogSettings.InclinometerInstalled = inc & 192;
    aogSettings.InclinometerInstalled = aogSettings.InclinometerInstalled >> 6;
    aogSettings.PulseCountMax = inc & 63;

    reed = Serial.read();
    checksum += reed;
    aogSettings.AckermanFix = reed;  

    reed = Serial.read();
    checksum += reed;    
    
    //send usbData back - version number etc. 
    SendTwoThirty((byte)checksum);

    EEPROM.put(40, aogSettings);
    
    //reset the arduino
    resetFunc();
  }
    
  //Settings Header has been found, 8 bytes are the settings
  if (Serial.available() > 7 && isSettingFound)
  {
    byte checksum = 0;
    byte reed = 0;
    isSettingFound = false;  //reset the flag
    
    //change the factors as required for your own PID values
    reed = Serial.read();
    checksum += reed;
    steerSettings.Kp = ((float)reed);   // read Kp from AgOpenGPS
    
    reed = Serial.read();
    checksum += reed;
    steerSettings.lowPWM = (float)reed;   // read lowPWM from AgOpenGPS
    
    reed = Serial.read();
    checksum += reed;
    steerSettings.Kd = (float)reed;   // read Kd from AgOpenGPS
    
    reed = Serial.read();
    checksum += reed;
    steerSettings.Ko = (float)reed;   // read  from AgOpenGPS
    
    reed = Serial.read();
    checksum += reed;
    steerSettings.steeringPositionZero = WAS_ZERO + reed;  //read steering zero offset
    
    reed = Serial.read();
    checksum += reed;
    steerSettings.minPWM = reed; //read the minimum amount of PWM for instant on
    
    reed = Serial.read();
    checksum += reed;
    steerSettings.highPWM = reed; //
    
    reed = Serial.read();
    checksum += reed;
    steerSettings.steerSensorCounts = reed; //sent as 10 times the setting displayed in AOG

    //send usbData back - version number. 
    SendTwoThirty((byte)checksum);
    
    EEPROM.put(10, steerSettings);
    
    // for PWM High to Low interpolator
    highLowPerDeg = (steerSettings.highPWM - steerSettings.lowPWM) / LOW_HIGH_DEGREES;
  }  
} // end of main loop

void SendTwoThirty(byte check)
{
    //Serial Send to agopenGPS **** you must send 10 numbers ****
    Serial.print("127,230,");
    Serial.print(check); 
    Serial.print(",");
    
    //version in AOG ex. v4.1.13 -> 4+1+13=18
    Serial.print(aogVersion);  
     
    Serial.println(",0,0,0,0,0,0");
    
    Serial.flush();   
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

  void MCP_Write(byte MCPregister, byte MCPdata) 
  {
    // -----------------
  
    Wire.beginTransmission(Addr);
    Wire.write(MCPregister);
    Wire.write(MCPdata);
    Wire.endTransmission();
  }

  //TCCR2B = TCCR2B & B11111000 | B00000001;    // set timer 2 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000010;    // set timer 2 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000011;    // set timer 2 divisor to    32 for PWM frequency of   980.39 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000100;    // set timer 2 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  //TCCR2B = TCCR2B & B11111000 | B00000101;    // set timer 2 divisor to   128 for PWM frequency of   245.10 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000110;    // set timer 2 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000111;    // set timer 2 divisor to  1024 for PWM frequency of    30.64 Hz

  //TCCR1B = TCCR1B & B11111000 | B00000001;    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000010;    // set timer 1 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000011;    // set timer 1 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  //TCCR1B = TCCR1B & B11111000 | B00000100;    // set timer 1 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000101;    // set timer 1 divisor to  1024 for PWM frequency of    30.64 Hz
