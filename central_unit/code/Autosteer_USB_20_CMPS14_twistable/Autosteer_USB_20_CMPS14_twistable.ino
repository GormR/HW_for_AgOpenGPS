  /*
  * USB Autosteer code For AgOpenGPS
  * 4 Feb 2021, Brian Tischler
  * Like all Arduino code - copied from somewhere else :)
  * So don't claim it as your own
  */
  
////////////////// User Settings /////////////////////////  

  //How many degrees before decreasing Max PWM
  #define LOW_HIGH_DEGREES 5.0

  /*  PWM Frequency -> 
   *   490hz (default) = 0
   *   122hz = 1
   *   3921hz = 2
   */
  #define PWM_Frequency 0
  
/////////////////////////////////////////////

  //#################################### CMPS14 settings ########################################
  #define myTwist 180   // twist of CMPS14 around vertical axis (range: 0..359)                 ##
                       // Example: 45 => CMPS14 heading is twisted 45° CCW vs tractor heading) ##
  //#############################################################################################

  // if not in eeprom, overwrite 
  #define EEP_Ident 4411 

  //version in AOG ex. v4.6.20
  #define aogVersion 20

  // Address of CMPS14 shifted right one bit for arduino wire library
  #define CMPS14_ADDRESS    0x60
  #define CMPS14_BearingReg 0x02  // (0x02/0x03: Bearing, 0x04: Pitch)
  #define CMPS14_RollReg    0x1c

  // BNO08x definitions
  #define REPORT_INTERVAL 90 //Report interval in ms (same as the delay at the bottom)

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

  #include <Wire.h>
  #include <EEPROM.h> 
  #include "zADS1115.h"
  ADS1115_lite adc(ADS1115_DEFAULT_ADDRESS);     // Use this for the 16-bit version ADS1115
  #include "BNO08x_AOG.h"
 
  //loop time variables in microseconds  
  const unsigned int LOOP_TIME = 20;  //50Hz    
  unsigned long lastTime = LOOP_TIME;
  unsigned long currentTime = LOOP_TIME;
  
  const unsigned int WATCHDOG_THRESHOLD = 100;
  const unsigned int WATCHDOG_FORCE_VALUE = WATCHDOG_THRESHOLD + 2; // Should be greater than WATCHDOG_THRESHOLD
  byte watchdogTimer = WATCHDOG_FORCE_VALUE;
  
   //Parsing PGN
  bool isPGNFound = false, isHeaderFound = false;
  byte pgn = 0, dataLength = 0, idx = 0;
  int tempHeader = 0;

  //show life in AgIO
  byte helloAgIO[] = {0x80,0x81, 0x7f, 0xC7, 1, 0, 0x47 };
  byte helloCounter=0;

  //fromAutoSteerData FD 253 - ActualSteerAngle*100 -5,6, Heading-7,8, 
        //Roll-9,10, SwitchByte-11, pwmDisplay-12, CRC 13
  byte AOG[] = {0x80,0x81, 0x7f, 0xFD, 8, 0, 0, 0, 0, 0,0,0,0, 0xCC };
  int AOGSize = sizeof(AOG);

  // booleans to see if we are using CMPS or BNO08x
  bool useCMPS = false;
  bool useBNO08x = false;

  // BNO08x address variables to check where it is
  const byte bno08xAddresses[] = {0x4A,0x4B};
  const int nrBNO08xAdresses = sizeof(bno08xAddresses)/sizeof(bno08xAddresses[0]);
  byte bno08xAddress;
  BNO080 bno08x;

  //cmps14 values  # GoRoNb
  #define CMPS14_ADDRESS 0x60   // Address of CMPS14 shifted right one bit for arduino wire library
  unsigned char high_bearing, low_bearing, high_roll, low_roll;
  char pitch8;
  int bearing16, roll16, pitch16, cosinus, sinus;
  long roll32;
  int mysine[] = {0,17,35,51,63,81,94,105,114,121,125,127,126,123,118,110,100,88,74,59,43,26,8,-9,-27,-44,-60,-75,-89,-101,-111,-119,-124,-127,-128,-126,-122,-115,-106,-95,-82,-68,-52,-36,-18};

  float bno08xHeading = 0;
  double bno08xRoll = 0;
  double bno08xPitch = 0;

  int bno08xHeading10x = 0;
  int bno08xRoll10x = 0;
  
  //EEPROM
  int EEread = 0;
 
  //Relays
  bool isRelayActiveHigh = true;
  byte relay = 0, relayHi = 0, uTurn = 0;
  byte tram = 0;
  
  //Switches
  byte remoteSwitch = 0, workSwitch = 0, steerSwitch = 1, switchByte = 0;

  //On Off
  byte guidanceStatus = 0;

  //speed sent as *10
  float gpsSpeed = 0;
  
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
  byte currentState = 1, reading, previous = 0;
  byte pulseCount = 0; // Steering Wheel Encoder
  bool encEnable = false; //debounce flag
  byte thisEnc = 0, lastEnc = 0;

   //Variables for settings  
   struct Storage {
      byte Kp = 40;  //proportional gain
      byte lowPWM = 10;  //band of no action
      int wasOffset = 0;
      byte minPWM = 9;
      byte highPWM = 60;//max PWM value
      float steerSensorCounts = 30;        
      float AckermanFix = 1;     //sent as percent
  };  Storage steerSettings;  //14 bytes

   //Variables for settings - 0 is false  
   struct Setup {
      byte InvertWAS = 0;
      byte isRelayActiveHigh = 0; //if zero, active low (default)
      byte MotorDriveDirection = 0;
      byte SingleInputWAS = 1;
      byte CytronDriver = 1;
      byte SteerSwitch = 0;  //0 button, 1 on-off switch
      byte ShaftEncoder = 0; 
      byte PulseCountMax = 5;
      byte isDanfoss = 0; 
  };  Setup steerConfig;          //9 bytes

  //reset function
  void(* resetFunc) (void) = 0;

  void setup()
  { 
    //PWM rate settings. Set them both the same!!!!
  
    if (PWM_Frequency == 1) 
    { 
      TCCR2B = TCCR2B & (B11111000 | B00000110);    // set timer 2 to 256 for PWM frequency of   122.55 Hz
      TCCR1B = TCCR1B & (B11111000 | B00000100);    // set timer 1 to 256 for PWM frequency of   122.55 Hz
    }
  
    else if (PWM_Frequency == 2)
    {
      TCCR1B = TCCR1B & (B11111000 | B00000010);    // set timer 1 to 8 for PWM frequency of  3921.16 Hz
      TCCR2B = TCCR2B & (B11111000 | B00000010);    // set timer 2 to 8 for PWM frequency of  3921.16 Hx
  
    }
    
    //keep pulled high and drag low to activate, noise free safe   
    pinMode(WORKSW_PIN, INPUT_PULLUP); 
    pinMode(STEERSW_PIN, INPUT_PULLUP); 
    pinMode(REMOTE_PIN, INPUT_PULLUP); 
    pinMode(DIR1_RL_ENABLE, OUTPUT);
    
    if (steerConfig.CytronDriver) pinMode(PWM2_RPWM, OUTPUT); 
    
    //set up communication
    Wire.begin();
    Serial.begin(38400);
  
    //test if CMPS working
    byte error;
    Wire.beginTransmission(CMPS14_ADDRESS);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.println("Error = 0");
      Serial.print("CMPS14 ADDRESs: 0x");
      Serial.println(CMPS14_ADDRESS, HEX);
      Serial.println("CMPS14 Ok.");
      useCMPS = true;

      // GoRoNb: CMPS14 coefficients:
      uint16_t tempTwist = (myTwist + 356) % 360;
      byte interpol = tempTwist % 8;   // linear interpolation of sine table
      sinus = mysine[tempTwist >> 3] * (7 - interpol) + mysine[((tempTwist >> 3) +1) % 45] * interpol;
      sinus = sinus / 7;
      tempTwist = (tempTwist + 90) % 360;
      cosinus = mysine[tempTwist >> 3] * (7 - interpol) + mysine[((tempTwist >> 3) +1) % 45] * interpol;
      cosinus = cosinus / 7;
    }
    else 
    {
      Serial.println("Error = 4");
      Serial.println("CMPS not Connected or Found");
      useCMPS = false;
    }

    // Check for BNO08x
    if(!useCMPS)
    {
      for(int i = 0; i < nrBNO08xAdresses; i++)
      {
        bno08xAddress = bno08xAddresses[i];
        
        Serial.print("\r\nChecking for BNO08X on ");
        Serial.println(bno08xAddress, HEX);
        Wire.beginTransmission(bno08xAddress);
        error = Wire.endTransmission();
    
        if (error == 0)
        {
          Serial.println("Error = 0");
          Serial.print("BNO08X ADDRESs: 0x");
          Serial.println(bno08xAddress, HEX);
          Serial.println("BNO08X Ok.");
          
          // Initialize BNO080 lib        
          if (bno08x.begin(bno08xAddress))
          {
            Wire.setClock(400000); //Increase I2C data rate to 400kHz
  
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

    adc.setSampleRate(ADS1115_REG_CONFIG_DR_128SPS); //128 samples per second
    adc.setGain(ADS1115_REG_CONFIG_PGA_6_144V);
    
  }// End of Setup

  void loop()
  {
  	// Loop triggers every 100 msec and sends back steer angle etc	 
  	currentTime = millis();
   
  	if (currentTime - lastTime >= LOOP_TIME)  // do motor control every LOOP_TIME msec
  	{
  		lastTime = currentTime;
  
      //reset debounce
      encEnable = true;
     
      //If connection lost to AgOpenGPS, the watchdog will count up and turn off steering
      if (watchdogTimer++ > 250) watchdogTimer = WATCHDOG_FORCE_VALUE;
  
      //read all the switches
      workSwitch = digitalRead(WORKSW_PIN);  // read work switch
      
      if (steerConfig.SteerSwitch == 1) //steer switch on - off
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
      
      if (steerConfig.ShaftEncoder && pulseCount >= steerConfig.PulseCountMax ) 
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
          if (steerConfig.isRelayActiveHigh) 
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
          if (steerConfig.isRelayActiveHigh) 
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

      //send empty pgn to AgIO to show activity
      if (++helloCounter > 10)
      {
        Serial.write(helloAgIO,sizeof(helloAgIO));
        helloCounter = 0;
      }
    } //end of timed loop
  
    //This runs continuously, not timed //// Serial Receive Data/Settings /////////////////
  
    // Serial Receive
    //Do we have a match with 0x8081?    
    if (Serial.available() > 1 && !isHeaderFound && !isPGNFound) 
    {
      byte temp = Serial.read();
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
      if (pgn == 254) //FE AutoSteerData
      {
        //bit 5,6
        gpsSpeed = ((float)(Serial.read()| Serial.read() << 8 ))*0.1;
        
        //bit 7
        guidanceStatus = Serial.read();

        //Bit 8,9    set point steer angle * 100 is sent
        steerAngleSetPoint = ((float)(Serial.read()| Serial.read() << 8 ))*0.01; //high low bytes
        
        if ((bitRead(guidanceStatus,0) == 0) || (gpsSpeed < 0.5) || (steerSwitch == 1) )
        { 
          watchdogTimer = WATCHDOG_FORCE_VALUE; //turn off steering motor
        }
        else          //valid conditions to turn on autosteer
        {
          watchdogTimer = 0;  //reset watchdog
        }
        
        //Bit 10 Tram 
        tram = Serial.read();
        
        //Bit 11 section 1 to 8
        relay = Serial.read();
        
        //Bit 12 section 9 to 16
        relayHi = Serial.read();
        
        
        //Bit 13 CRC
        Serial.read();
        
        //reset for next pgn sentence
        isHeaderFound = isPGNFound = false;
        pgn=dataLength=0;      

                   //----------------------------------------------------------------------------
        //Serial Send to agopenGPS
        
        int sa = (int)(steerAngleActual*100);
        AOG[5] = (byte)sa;
        AOG[6] = sa >> 8;
        
        if (useCMPS)
        {
          Wire.beginTransmission(CMPS14_ADDRESS);  //starts communication with CMPS14
          Wire.write(CMPS14_BearingReg);           //Sends the register we wish to start reading from (Bearing16 + Pitch8)
          Wire.endTransmission();
          // Request 3 bytes from the CMPS14: 16 bit bearing, 8 bit pitch
          Wire.requestFrom(CMPS14_ADDRESS, 3);       

          if(Wire.available() <= 3);      // Wait for all bytes to come back
          high_bearing = Wire.read();
          low_bearing = Wire.read();
          pitch8 = Wire.read();

          bearing16 = (high_bearing << 8) | low_bearing;      // Calculate 16 bit angle (10 digits per degree)
          bearing16 = (bearing16 + myTwist * 10) % 3600;      // max 3600 = 360°
          //the heading x10
          AOG[7] = (uint8_t) bearing16;
          AOG[8] = (uint8_t)(bearing16 >> 8);
      
          Wire.beginTransmission(CMPS14_ADDRESS);  //starts communication with CMPS14
          Wire.write(CMPS14_RollReg);              //Sends the register we wish to start reading from (Roll16)
          Wire.endTransmission();
          // Request 2 bytes from the CMPS14: 16 bit roll
          Wire.requestFrom(CMPS14_ADDRESS, 2);       
      
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
           //the roll x10
          AOG[9]  = (uint8_t)(roll32 >> 7);
          AOG[10] = (uint8_t)(roll32>> 15);
        }
        else if(useBNO08x)
        {
          if (bno08x.dataAvailable() == true)
          {
            bno08xHeading = (bno08x.getYaw()) * 180.0 / PI; // Convert yaw / heading to degrees
            bno08xHeading = -bno08xHeading; //BNO085 counter clockwise data to clockwise data
            
            if (bno08xHeading < 0 && bno08xHeading >= -180) //Scale BNO085 yaw from [-180°;180°] to [0;360°]
            {
              bno08xHeading = bno08xHeading + 360;
            }
                
            bno08xRoll = (bno08x.getRoll()) * 180.0 / PI; //Convert roll to degrees
            bno08xPitch = (bno08x.getPitch())* 180.0 / PI; // Convert pitch to degrees
    
            bno08xHeading10x = (int)(bno08xHeading * 10);
            bno08xRoll10x = (int)(bno08xRoll * 10);
  
            Serial.print(bno08xHeading10x);
            Serial.print(",");
            Serial.println(bno08xRoll10x); 
            
            //the heading x10
            AOG[7] = (byte)bno08xHeading10x;
            AOG[8] = bno08xHeading10x >> 8;
            
    
            //the roll x10
            AOG[9] = (byte)bno08xRoll10x;
            AOG[10] = bno08xRoll10x >> 8;
          }
        }
        else
        { 
          //heading         
          AOG[7] = (byte)9999;        
          AOG[8] = 9999 >> 8;
          
          //roll
          AOG[9] = (byte)8888;  
          AOG[10] = 8888 >> 8;
        }        
        
        AOG[11] = switchByte;
        AOG[12] = (byte)pwmDisplay;
        
        //add the checksum
        int CK_A = 0;
        for (byte i = 2; i < AOGSize - 1; i++)
        {
          CK_A = (CK_A + AOG[i]);
        }
        
        AOG[AOGSize - 1] = CK_A;
        
        Serial.write(AOG, AOGSize);

        // Stop sending the helloAgIO message
        helloCounter = 0;
        //--------------------------------------------------------------------------              
      }
              
      else if (pgn==252) //FC AutoSteerSettings
      {         
        //PID values
        steerSettings.Kp = ((float)Serial.read());   // read Kp from AgOpenGPS
        steerSettings.Kp*=0.5;
        
        steerSettings.highPWM = Serial.read();
        
        steerSettings.lowPWM = (float)Serial.read();   // read lowPWM from AgOpenGPS
                
        steerSettings.minPWM = Serial.read(); //read the minimum amount of PWM for instant on
        
        steerSettings.steerSensorCounts = Serial.read(); //sent as setting displayed in AOG
        
        steerSettings.wasOffset = (Serial.read());  //read was zero offset Hi
               
        steerSettings.wasOffset |= (Serial.read() << 8);  //read was zero offset Lo
        
        steerSettings.AckermanFix = (float)Serial.read() * 0.01; 

        //crc
        //udpData[13];        //crc
        Serial.read();
    
        //store in EEPROM
        EEPROM.put(10, steerSettings);           
    
        // for PWM High to Low interpolator
        highLowPerDeg = ((float)(steerSettings.highPWM - steerSettings.lowPWM)) / LOW_HIGH_DEGREES;
        
        //reset for next pgn sentence
        isHeaderFound = isPGNFound = false;
        pgn=dataLength=0;
      }
    
      else if (pgn == 251) //FB - steerConfig
      {       
        byte sett = Serial.read();
         
        if (bitRead(sett,0)) steerConfig.InvertWAS = 1; else steerConfig.InvertWAS = 0;
        if (bitRead(sett,1)) steerConfig.isRelayActiveHigh = 1; else steerConfig.isRelayActiveHigh = 0;
        if (bitRead(sett,2)) steerConfig.MotorDriveDirection = 1; else steerConfig.MotorDriveDirection = 0;
        if (bitRead(sett,3)) steerConfig.SingleInputWAS = 1; else steerConfig.SingleInputWAS = 0;
        if (bitRead(sett,4)) steerConfig.CytronDriver = 1; else steerConfig.CytronDriver = 0;
        if (bitRead(sett,5)) steerConfig.SteerSwitch = 1; else steerConfig.SteerSwitch = 0;
        if (bitRead(sett,6)) steerConfig.ShaftEncoder = 1; else steerConfig.ShaftEncoder = 0;
        
        steerConfig.PulseCountMax = Serial.read();

        //was speed
        Serial.read(); 
        
         //Danfoss type hydraulics
        steerConfig.isDanfoss = Serial.read(); //byte 8
              
        Serial.read(); //byte 9
        Serial.read(); //byte 10
         
        Serial.read(); //byte 11
        Serial.read(); //byte 12
      
        //crc byte 13
        Serial.read();
                
        EEPROM.put(40, steerConfig);
      
        //reset for next pgn sentence
        isHeaderFound = isPGNFound = false;
        pgn=dataLength=0; 
      
        //reset the arduino
        resetFunc();
      }
    
      //clean up strange pgns
      else
      {
          //reset for next pgn sentence
          isHeaderFound = isPGNFound = false;
          pgn=dataLength=0; 
      }        
  
    } //end if (Serial.available() > dataLength && isHeaderFound && isPGNFound)      
  
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

  //ISR Steering Wheel Encoder
  void EncoderFunc()
  {        
     if (encEnable) 
     {
        pulseCount++; 
        encEnable = false;
     }            
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
=======
  /*
  * USB Autosteer code For AgOpenGPS
  * 4 Feb 2021, Brian Tischler
  * Like all Arduino code - copied from somewhere else :)
  * So don't claim it as your own
  */
  
////////////////// User Settings /////////////////////////  

  //How many degrees before decreasing Max PWM
  #define LOW_HIGH_DEGREES 5.0

  /*  PWM Frequency -> 
   *   490hz (default) = 0
   *   122hz = 1
   *   3921hz = 2
   */
  #define PWM_Frequency 0
  
/////////////////////////////////////////////

  //#################################### CMPS14 settings ########################################
  #define myTwist 180   // twist of CMPS14 around vertical axis (range: 0..359)                 ##
                       // Example: 45 => CMPS14 heading is twisted 45° CCW vs tractor heading) ##
  //#############################################################################################

  // if not in eeprom, overwrite 
  #define EEP_Ident 4411 

  //version in AOG ex. v4.6.20
  #define aogVersion 20

  // Address of CMPS14 shifted right one bit for arduino wire library
  #define CMPS14_ADDRESS    0x60
  #define CMPS14_BearingReg 0x02  // (0x02/0x03: Bearing, 0x04: Pitch)
  #define CMPS14_RollReg    0x1c

  // BNO08x definitions
  #define REPORT_INTERVAL 90 //Report interval in ms (same as the delay at the bottom)

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

  #include <Wire.h>
  #include <EEPROM.h> 
  #include "zADS1115.h"
  ADS1115_lite adc(ADS1115_DEFAULT_ADDRESS);     // Use this for the 16-bit version ADS1115
  #include "BNO08x_AOG.h"
 
  //loop time variables in microseconds  
  const unsigned int LOOP_TIME = 20;  //50Hz    
  unsigned long lastTime = LOOP_TIME;
  unsigned long currentTime = LOOP_TIME;
  
  const unsigned int WATCHDOG_THRESHOLD = 100;
  const unsigned int WATCHDOG_FORCE_VALUE = WATCHDOG_THRESHOLD + 2; // Should be greater than WATCHDOG_THRESHOLD
  byte watchdogTimer = WATCHDOG_FORCE_VALUE;
  
   //Parsing PGN
  bool isPGNFound = false, isHeaderFound = false;
  byte pgn = 0, dataLength = 0, idx = 0;
  int tempHeader = 0;

  //show life in AgIO
  byte helloAgIO[] = {0x80,0x81, 0x7f, 0xC7, 1, 0, 0x47 };
  byte helloCounter=0;

  //fromAutoSteerData FD 253 - ActualSteerAngle*100 -5,6, Heading-7,8, 
        //Roll-9,10, SwitchByte-11, pwmDisplay-12, CRC 13
  byte AOG[] = {0x80,0x81, 0x7f, 0xFD, 8, 0, 0, 0, 0, 0,0,0,0, 0xCC };
  int AOGSize = sizeof(AOG);

  // booleans to see if we are using CMPS or BNO08x
  bool useCMPS = false;
  bool useBNO08x = false;

  // BNO08x address variables to check where it is
  const byte bno08xAddresses[] = {0x4A,0x4B};
  const int nrBNO08xAdresses = sizeof(bno08xAddresses)/sizeof(bno08xAddresses[0]);
  byte bno08xAddress;
  BNO080 bno08x;

  //cmps14 values  # GoRoNb
  #define CMPS11_ADDRESS 0x60   // Address of CMPS11 shifted right one bit for arduino wire library
  unsigned char high_bearing, low_bearing, high_roll, low_roll;
  char pitch8;
  int bearing16, roll16, pitch16, cosinus, sinus;
  long roll32;
  int mysine[] = {0,17,35,51,63,81,94,105,114,121,125,127,126,123,118,110,100,88,74,59,43,26,8,-9,-27,-44,-60,-75,-89,-101,-111,-119,-124,-127,-128,-126,-122,-115,-106,-95,-82,-68,-52,-36,-18};

  float bno08xHeading = 0;
  double bno08xRoll = 0;
  double bno08xPitch = 0;

  int bno08xHeading10x = 0;
  int bno08xRoll10x = 0;
  
  //EEPROM
  int EEread = 0;
 
  //Relays
  bool isRelayActiveHigh = true;
  byte relay = 0, relayHi = 0, uTurn = 0;
  byte tram = 0;
  
  //Switches
  byte remoteSwitch = 0, workSwitch = 0, steerSwitch = 1, switchByte = 0;

  //On Off
  byte guidanceStatus = 0;

  //speed sent as *10
  float gpsSpeed = 0;
  
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
  byte currentState = 1, reading, previous = 0;
  byte pulseCount = 0; // Steering Wheel Encoder
  bool encEnable = false; //debounce flag
  byte thisEnc = 0, lastEnc = 0;

   //Variables for settings  
   struct Storage {
      byte Kp = 40;  //proportional gain
      byte lowPWM = 10;  //band of no action
      int wasOffset = 0;
      byte minPWM = 9;
      byte highPWM = 60;//max PWM value
      float steerSensorCounts = 30;        
      float AckermanFix = 1;     //sent as percent
  };  Storage steerSettings;  //14 bytes

   //Variables for settings - 0 is false  
   struct Setup {
      byte InvertWAS = 0;
      byte isRelayActiveHigh = 0; //if zero, active low (default)
      byte MotorDriveDirection = 0;
      byte SingleInputWAS = 1;
      byte CytronDriver = 1;
      byte SteerSwitch = 0;  //0 button, 1 on-off switch
      byte ShaftEncoder = 0; 
      byte PulseCountMax = 5;
      byte isDanfoss = 0; 
  };  Setup steerConfig;          //9 bytes

  //reset function
  void(* resetFunc) (void) = 0;

  void setup()
  { 
    //PWM rate settings. Set them both the same!!!!
  
    if (PWM_Frequency == 1) 
    { 
      TCCR2B = TCCR2B & (B11111000 | B00000110);    // set timer 2 to 256 for PWM frequency of   122.55 Hz
      TCCR1B = TCCR1B & (B11111000 | B00000100);    // set timer 1 to 256 for PWM frequency of   122.55 Hz
    }
  
    else if (PWM_Frequency == 2)
    {
      TCCR1B = TCCR1B & (B11111000 | B00000010);    // set timer 1 to 8 for PWM frequency of  3921.16 Hz
      TCCR2B = TCCR2B & (B11111000 | B00000010);    // set timer 2 to 8 for PWM frequency of  3921.16 Hx
  
    }
    
    //keep pulled high and drag low to activate, noise free safe   
    pinMode(WORKSW_PIN, INPUT_PULLUP); 
    pinMode(STEERSW_PIN, INPUT_PULLUP); 
    pinMode(REMOTE_PIN, INPUT_PULLUP); 
    pinMode(DIR1_RL_ENABLE, OUTPUT);
    
    if (steerConfig.CytronDriver) pinMode(PWM2_RPWM, OUTPUT); 
    
    //set up communication
    Wire.begin();
    Serial.begin(38400);
  
    //test if CMPS working
    byte error;
    Wire.beginTransmission(CMPS14_ADDRESS);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.println("Error = 0");
      Serial.print("CMPS14 ADDRESs: 0x");
      Serial.println(CMPS14_ADDRESS, HEX);
      Serial.println("CMPS14 Ok.");
      useCMPS = true;

      // GoRoNb: CMPS14 coefficients:
      uint16_t tempTwist = (myTwist + 356) % 360;
      byte interpol = tempTwist % 8;   // linear interpolation of sine table
      sinus = mysine[tempTwist >> 3] * (7 - interpol) + mysine[((tempTwist >> 3) +1) % 45] * interpol;
      sinus = sinus / 7;
      tempTwist = (tempTwist + 90) % 360;
      cosinus = mysine[tempTwist >> 3] * (7 - interpol) + mysine[((tempTwist >> 3) +1) % 45] * interpol;
      cosinus = cosinus / 7;
    }
    else 
    {
      Serial.println("Error = 4");
      Serial.println("CMPS not Connected or Found");
      useCMPS = false;
    }

    // Check for BNO08x
    if(!useCMPS)
    {
      for(int i = 0; i < nrBNO08xAdresses; i++)
      {
        bno08xAddress = bno08xAddresses[i];
        
        Serial.print("\r\nChecking for BNO08X on ");
        Serial.println(bno08xAddress, HEX);
        Wire.beginTransmission(bno08xAddress);
        error = Wire.endTransmission();
    
        if (error == 0)
        {
          Serial.println("Error = 0");
          Serial.print("BNO08X ADDRESs: 0x");
          Serial.println(bno08xAddress, HEX);
          Serial.println("BNO08X Ok.");
          
          // Initialize BNO080 lib        
          if (bno08x.begin(bno08xAddress))
          {
            Wire.setClock(400000); //Increase I2C data rate to 400kHz
  
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

    adc.setSampleRate(ADS1115_REG_CONFIG_DR_128SPS); //128 samples per second
    adc.setGain(ADS1115_REG_CONFIG_PGA_6_144V);
    
  }// End of Setup

  void loop()
  {
  	// Loop triggers every 100 msec and sends back steer angle etc	 
  	currentTime = millis();
   
  	if (currentTime - lastTime >= LOOP_TIME)  // do motor control every LOOP_TIME msec
  	{
  		lastTime = currentTime;
  
      //reset debounce
      encEnable = true;
     
      //If connection lost to AgOpenGPS, the watchdog will count up and turn off steering
      if (watchdogTimer++ > 250) watchdogTimer = WATCHDOG_FORCE_VALUE;
  
      //read all the switches
      workSwitch = digitalRead(WORKSW_PIN);  // read work switch
      
      if (steerConfig.SteerSwitch == 1) //steer switch on - off
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
      
      if (steerConfig.ShaftEncoder && pulseCount >= steerConfig.PulseCountMax ) 
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
          if (steerConfig.isRelayActiveHigh) 
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
          if (steerConfig.isRelayActiveHigh) 
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

      //send empty pgn to AgIO to show activity
      if (++helloCounter > 10)
      {
        Serial.write(helloAgIO,sizeof(helloAgIO));
        helloCounter = 0;
      }
    } //end of timed loop
  
    //This runs continuously, not timed //// Serial Receive Data/Settings /////////////////
  
    // Serial Receive
    //Do we have a match with 0x8081?    
    if (Serial.available() > 1 && !isHeaderFound && !isPGNFound) 
    {
      byte temp = Serial.read();
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
      if (pgn == 254) //FE AutoSteerData
      {
        //bit 5,6
        gpsSpeed = ((float)(Serial.read()| Serial.read() << 8 ))*0.1;
        
        //bit 7
        guidanceStatus = Serial.read();

        //Bit 8,9    set point steer angle * 100 is sent
        steerAngleSetPoint = ((float)(Serial.read()| Serial.read() << 8 ))*0.01; //high low bytes
        
        if ((bitRead(guidanceStatus,0) == 0) || (gpsSpeed < 0.5) || (steerSwitch == 1) )
        { 
          watchdogTimer = WATCHDOG_FORCE_VALUE; //turn off steering motor
        }
        else          //valid conditions to turn on autosteer
        {
          watchdogTimer = 0;  //reset watchdog
        }
        
        //Bit 10 Tram 
        tram = Serial.read();
        
        //Bit 11 section 1 to 8
        relay = Serial.read();
        
        //Bit 12 section 9 to 16
        relayHi = Serial.read();
        
        
        //Bit 13 CRC
        Serial.read();
        
        //reset for next pgn sentence
        isHeaderFound = isPGNFound = false;
        pgn=dataLength=0;      

                   //----------------------------------------------------------------------------
        //Serial Send to agopenGPS
        
        int sa = (int)(steerAngleActual*100);
        AOG[5] = (byte)sa;
        AOG[6] = sa >> 8;
        
        if (useCMPS)
        {
          Wire.beginTransmission(CMPS11_ADDRESS);  //starts communication with CMPS14
          Wire.write(CMPS14_BearingReg);           //Sends the register we wish to start reading from (Bearing16 + Pitch8)
          Wire.endTransmission();
          // Request 3 bytes from the CMPS11: 16 bit bearing, 8 bit pitch
          Wire.requestFrom(CMPS11_ADDRESS, 3);       

          if(Wire.available() <= 3);      // Wait for all bytes to come back
          high_bearing = Wire.read();
          low_bearing = Wire.read();
          pitch8 = Wire.read();

          bearing16 = (high_bearing << 8) | low_bearing;      // Calculate 16 bit angle (10 digits per degree)
          bearing16 = (bearing16 + myTwist * 10) % 3600;      // max 3600 = 360°
          //the heading x10
          AOG[7] = (uint8_t) bearing16;
          AOG[8] = (uint8_t)(bearing16 >> 8);
      
          Wire.beginTransmission(CMPS11_ADDRESS);  //starts communication with CMPS14
          Wire.write(CMPS14_RollReg);              //Sends the register we wish to start reading from (Roll16)
          Wire.endTransmission();
          // Request 2 bytes from the CMPS11: 16 bit roll
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
           //the roll x10
          AOG[9]  = (uint8_t)(roll32 >> 7);
          AOG[10] = (uint8_t)(roll32>> 15);
        }
        else if(useBNO08x)
        {
          if (bno08x.dataAvailable() == true)
          {
            bno08xHeading = (bno08x.getYaw()) * 180.0 / PI; // Convert yaw / heading to degrees
            bno08xHeading = -bno08xHeading; //BNO085 counter clockwise data to clockwise data
            
            if (bno08xHeading < 0 && bno08xHeading >= -180) //Scale BNO085 yaw from [-180°;180°] to [0;360°]
            {
              bno08xHeading = bno08xHeading + 360;
            }
                
            bno08xRoll = (bno08x.getRoll()) * 180.0 / PI; //Convert roll to degrees
            bno08xPitch = (bno08x.getPitch())* 180.0 / PI; // Convert pitch to degrees
    
            bno08xHeading10x = (int)(bno08xHeading * 10);
            bno08xRoll10x = (int)(bno08xRoll * 10);
  
            Serial.print(bno08xHeading10x);
            Serial.print(",");
            Serial.println(bno08xRoll10x); 
            
            //the heading x10
            AOG[7] = (byte)bno08xHeading10x;
            AOG[8] = bno08xHeading10x >> 8;
            
    
            //the roll x10
            AOG[9] = (byte)bno08xRoll10x;
            AOG[10] = bno08xRoll10x >> 8;
          }
        }
        else
        { 
          //heading         
          AOG[7] = (byte)9999;        
          AOG[8] = 9999 >> 8;
          
          //roll
          AOG[9] = (byte)8888;  
          AOG[10] = 8888 >> 8;
        }        
        
        AOG[11] = switchByte;
        AOG[12] = (byte)pwmDisplay;
        
        //add the checksum
        int CK_A = 0;
        for (byte i = 2; i < AOGSize - 1; i++)
        {
          CK_A = (CK_A + AOG[i]);
        }
        
        AOG[AOGSize - 1] = CK_A;
        
        Serial.write(AOG, AOGSize);

        // Stop sending the helloAgIO message
        helloCounter = 0;
        //--------------------------------------------------------------------------              
      }
              
      else if (pgn==252) //FC AutoSteerSettings
      {         
        //PID values
        steerSettings.Kp = ((float)Serial.read());   // read Kp from AgOpenGPS
        steerSettings.Kp*=0.5;
        
        steerSettings.highPWM = Serial.read();
        
        steerSettings.lowPWM = (float)Serial.read();   // read lowPWM from AgOpenGPS
                
        steerSettings.minPWM = Serial.read(); //read the minimum amount of PWM for instant on
        
        steerSettings.steerSensorCounts = Serial.read(); //sent as setting displayed in AOG
        
        steerSettings.wasOffset = (Serial.read());  //read was zero offset Hi
               
        steerSettings.wasOffset |= (Serial.read() << 8);  //read was zero offset Lo
        
        steerSettings.AckermanFix = (float)Serial.read() * 0.01; 

        //crc
        //udpData[13];        //crc
        Serial.read();
    
        //store in EEPROM
        EEPROM.put(10, steerSettings);           
    
        // for PWM High to Low interpolator
        highLowPerDeg = ((float)(steerSettings.highPWM - steerSettings.lowPWM)) / LOW_HIGH_DEGREES;
        
        //reset for next pgn sentence
        isHeaderFound = isPGNFound = false;
        pgn=dataLength=0;
      }
    
      else if (pgn == 251) //FB - steerConfig
      {       
        byte sett = Serial.read();
         
        if (bitRead(sett,0)) steerConfig.InvertWAS = 1; else steerConfig.InvertWAS = 0;
        if (bitRead(sett,1)) steerConfig.isRelayActiveHigh = 1; else steerConfig.isRelayActiveHigh = 0;
        if (bitRead(sett,2)) steerConfig.MotorDriveDirection = 1; else steerConfig.MotorDriveDirection = 0;
        if (bitRead(sett,3)) steerConfig.SingleInputWAS = 1; else steerConfig.SingleInputWAS = 0;
        if (bitRead(sett,4)) steerConfig.CytronDriver = 1; else steerConfig.CytronDriver = 0;
        if (bitRead(sett,5)) steerConfig.SteerSwitch = 1; else steerConfig.SteerSwitch = 0;
        if (bitRead(sett,6)) steerConfig.ShaftEncoder = 1; else steerConfig.ShaftEncoder = 0;
        
        steerConfig.PulseCountMax = Serial.read();

        //was speed
        Serial.read(); 
        
         //Danfoss type hydraulics
        steerConfig.isDanfoss = Serial.read(); //byte 8
              
        Serial.read(); //byte 9
        Serial.read(); //byte 10
         
        Serial.read(); //byte 11
        Serial.read(); //byte 12
      
        //crc byte 13
        Serial.read();
                
        EEPROM.put(40, steerConfig);
      
        //reset for next pgn sentence
        isHeaderFound = isPGNFound = false;
        pgn=dataLength=0; 
      
        //reset the arduino
        resetFunc();
      }
    
      //clean up strange pgns
      else
      {
          //reset for next pgn sentence
          isHeaderFound = isPGNFound = false;
          pgn=dataLength=0; 
      }        
  
    } //end if (Serial.available() > dataLength && isHeaderFound && isPGNFound)      
  
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

  //ISR Steering Wheel Encoder
  void EncoderFunc()
  {        
     if (encEnable) 
     {
        pulseCount++; 
        encEnable = false;
     }            
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
