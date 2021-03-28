/**************************************************************************/
/*!
    @file     AgriNode.cpp
    @author   G. Rose
    @license  GPL (see license.txt)

    Board Support Package for Generic AgriNode

    This is a library for the generic AgriNode board
    ----> https://github.com/GormR/HW_for_AgOpenGPS/tree/main/agri_node

    @section  HISTORY

    v1.0 - First release

    Ressources:
    Timer0: 10ms timer interrupt
    Timer1: RS485 data direction and Modbus response check
*/
/**************************************************************************/

#include "AgriNode.h"

//##########################################################################
//##           application-specific part (section control)                ##
//##########################################################################

uint32_t _flowmeter = 0;  // flowmeter pulses
uint32_t _lastSpeedPulse = 0;  // flowmeter pulses
uint32_t _speedPulse = 0;  // flowmeter pulses
uint32_t _leftUStime = 0; // timestamp US pulse right sensor
uint32_t _middleUStime = 0; // timestamp US pulse right sensor
uint32_t _rightUStime = 0; // timestamp US pulse right sensor
uint8_t _intState = 0;

//##########################################################################
//##               board-specific part (generic AgriNode)                 ##
//##########################################################################

#define _Hardware_Version_1_0_

Adafruit_ADS1115 _ads;     // Use this for the 16-bit version ADS1115

#ifndef _Hardware_Version_1_0_
static const int spiClk = 1000000; // 1 MHz SPI clock
#endif
uint32_t _eOut;
uint32_t _eIn;
uint8_t  _MBsent[10];  // data send to Modbus client
uint8_t  _MBok;

//uninitalised pointers to SPI objects
//SPIClass * SPIeIO = NULL; // not usable for V1.0

//##########################################################################
//##           application-specific part (section control)                ##
//##########################################################################

//#######################    Interrupts   ##################################

hw_timer_t * _Timer10ms = NULL;
hw_timer_t * _TimerRS485 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile byte testvar = 0;

/**************************************************************************/
/*!
    @brief  RS485 _OE disable timer interrupt

    - service SPI I/Os
    - read ADC and set next channel
*/
/**************************************************************************/
void IRAM_ATTR _RS485Timer()
{
    switch (_intState)
    {
        case 0:             // no idea why the timer starts immediately..
            _intState++;
            break;
        case 1:             // time to disable the transmit enable 
            digitalWrite(_OE, LOW);  // driver disable
            _intState++;
            timerAlarmWrite(_TimerRS485, 200, false);   // write time till answer
            break;
        default:
            //_MBok = myNode.RTUChkResp()
            timerAlarmDisable(_TimerRS485);
    }
}

/**************************************************************************/
/*!
    @brief  flowmeter interrupt
*/
/**************************************************************************/
void IRAM_ATTR FlowISR() 
{
    portENTER_CRITICAL_ISR(&timerMux);
    _flowmeter++;
    portEXIT_CRITICAL_ISR(&timerMux);
    if (_flowmeter & 0x01) digitalWrite(DO1, LOW); else digitalWrite(DO1, HIGH);
}

/**************************************************************************/
/*!
    @brief  speed pulse interrupt
*/
/**************************************************************************/
void IRAM_ATTR SpeedISR() 
{
    uint32_t mytime = micros();    
    portENTER_CRITICAL_ISR(&timerMux);
    _lastSpeedPulse = _speedPulse;
    _speedPulse = mytime;
    portEXIT_CRITICAL_ISR(&timerMux);
}

/**************************************************************************/
/*!
    @brief  speed pulse interrupt
*/
/**************************************************************************/
void IRAM_ATTR leftUS() 
{
    uint32_t mytime = micros();    
    portENTER_CRITICAL_ISR(&timerMux);
    _leftUStime = mytime;
    portEXIT_CRITICAL_ISR(&timerMux);



    Serial.println("left");

    
}

/**************************************************************************/
/*!
    @brief  speed pulse interrupt
*/
/**************************************************************************/
void IRAM_ATTR middleUS() 
{
    uint32_t mytime = micros();    
    portENTER_CRITICAL_ISR(&timerMux);
    _middleUStime = mytime;
    portEXIT_CRITICAL_ISR(&timerMux);



    Serial.println("middle");

}

/**************************************************************************/
/*!
    @brief  speed pulse interrupt
*/
/**************************************************************************/
void IRAM_ATTR rightUS() 
{
    uint32_t mytime = micros();    
    portENTER_CRITICAL_ISR(&timerMux);
    _rightUStime = mytime;
    portEXIT_CRITICAL_ISR(&timerMux);



    Serial.println("right");

    
}

//#######################     Functions   ##################################


uint8_t _SoilDistanceRead()
{
  
}


//##########################################################################
//##               board-specific part (generic AgriNode)                 ##
//##########################################################################

//#######################    Interrupts   ##################################

/**************************************************************************/
/*!
    @brief  10ms timer interrupt

    - service SPI I/Os
    - read ADC and set next channel
*/
/**************************************************************************/
void IRAM_ATTR _10msTimer()
{
    uint32_t mask = 0x80000000;
    _eIn = 0;

#ifdef _Hardware_Version_1_0_
    //  workaround for V1.0 (have to write and read separately):
    //  write out first
    for (uint8_t loop=0; loop<32; loop++)
    {
        if (_eOut & mask) digitalWrite(_MOSI, HIGH); else digitalWrite(_MOSI, LOW);
        digitalWrite(_CSeIO, LOW);  // load inputs to shift reg
        mask >>= 1;
        digitalWrite(_CSeIO, HIGH);  // clock into shift register with this edge
    }
    // read back input in a 2nd step
    mask = 0x00800000;
    for (uint8_t loop=0; loop<24; loop++)
    {
        digitalWrite(_SCLK, LOW);  // this is actually a delay
        if (digitalRead(_MISO)) _eIn |= mask;         
        digitalWrite(_SCLK, HIGH); // shift next bit in
        mask >>= 1;
        digitalWrite(_SCLK, LOW);  
    }
#else
    // SCLK and CSeIO have to be swapped with revision 1.1!
    digitalWrite(_CSeIO, LOW);  // load inputs to shift reg
    digitalWrite(_CSeIO, HIGH);
    for (uint8_t loop=0; loop<32; loop++)
    {
        if (_eOut & mask) digitalWrite(_MOSI, HIGH); else digitalWrite(_MOSI, LOW);
        digitalWrite(_CSeIO, LOW);  // load inputs to shift reg
        digitalWrite(_CSeIO, HIGH);
        if (digitalRead(_MISO)) _eIn |= mask;         
        mask >>= 1;
        digitalWrite(_CSeIO, LOW);  // load inputs to shift reg
    }
    digitalWrite(_CSeIO, LOW);  // load outputs with shift reg  
    digitalWrite(_CSeIO, HIGH);
#endif



  _eOut = (_eIn >> 1) | 0xff008000; // debug ###############


}

//#######################     Functions   ##################################

/**************************************************************************/
/*!
    @brief  Instantiates a new AgriNode class w/ appropriate properties
*/
/**************************************************************************/
AgriNode::AgriNode()
{
    _eOut = 0;
    _eIn = 0;
    // set IO directions and init outputs
    pinMode(PWM1, OUTPUT);
    ledcSetup(0, 4000, 8);    // configure PWM functionalitites (ch0, 4kHz, 8bit)
    ledcAttachPin(PWM1, 0);   // bind PWM channel 0 to output PWM1
    ledcWrite(0, 0);          // init to ch0 (channel, value)
    pinMode(PWM2, OUTPUT);
    ledcSetup(1, 4000, 8);    // configure PWM functionalitites (ch1, 4kHz, 8bit)
    ledcAttachPin(PWM2, 1);   // bind PWM channel 1 to output PWM2
    ledcWrite(1, 0);          // init to ch1 (channel, value)
    pinMode(_CSEth, OUTPUT);
    digitalWrite(_CSEth, HIGH);
    pinMode(_CSeIO, OUTPUT);
    digitalWrite(_CSeIO, HIGH);
    pinMode(_MOSI, OUTPUT);
    pinMode(_SCLK, OUTPUT);
    pinMode(_MISO, INPUT);
    digitalWrite(_CSeIO, HIGH);
    pinMode(_OE, OUTPUT);
    digitalWrite(_OE, HIGH);
    
    pinMode(DO0, OUTPUT);
    digitalWrite(DO0, LOW);
    pinMode(DO1, OUTPUT);
    digitalWrite(DO1, LOW);
    _eOut = 0;

    //SPIeIO = new SPIClass(VSPI);
	  //SPIeIO->begin(_SCLK, _MISO, _MOSI);
    //extendedIO();             // init extended IOs

    digitalWrite(_OE, LOW);   // set output enable

    Wire.begin(_SDA, _SCL);   // start I²C
    Wire.write(0x55);

    
    // init serial port, e.g. for ModbusRTU 
    // (DE signal must be generated in software because pin is blocked for flash)
    Serial2.begin(9600, SERIAL_8N1, _RxD, _TxD);

    // create a hardware timer
    /* Use 1st (=0) timer of 4 for cyclic 10ms interrupt*/
    /* 1 tick take 1/(80MHZ/80) = 1us so we set divider 80 and count up */
    _Timer10ms = timerBegin(0, 80, true);
  
    /* Attach onTimer function to our timer */
    timerAttachInterrupt(_Timer10ms, &_10msTimer, true);
  
    /* Set alarm to call onTimer function every second 1 tick is 1us
    => 10ms is 10000us */
    /* Repeat the alarm (third parameter) */
    timerAlarmWrite(_Timer10ms, 10000, true);
  
    /* Start an alarm */
    timerAlarmEnable(_Timer10ms);

    /* Use 2nd (=1) timer of 4 for RS485 */
    /* 1 tick take 1/(80MHZ/80) = 1us so we set divider 80 and count up */
    _TimerRS485 = timerBegin(1, 8333, true);  // 8333 = 80MHz/9600Baud
  
    /* Attach onTimer function to our timer */
    timerAttachInterrupt(_TimerRS485, &_RS485Timer, true);

    pinMode(DI0, INPUT_PULLUP);
    attachInterrupt(DI0, FlowISR, FALLING);

    pinMode(DI1, INPUT_PULLUP);
    attachInterrupt(DI1, SpeedISR, FALLING);

    pinMode(Ain7, INPUT_PULLUP);
    attachInterrupt(Ain7, leftUS, FALLING);
    
    pinMode(Ain5, INPUT_PULLUP);
    attachInterrupt(Ain5, middleUS, FALLING);

    pinMode(Ain6, INPUT_PULLUP);
    attachInterrupt(Ain6, rightUS, FALLING);
    }

/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
*/
/**************************************************************************/
void AgriNode::_digitalWrite(uint8_t bitno, uint8_t value)
{
	if (bitno < 64)  //  I/O  0..39   I/O of ESP32 acc. Espressif datasheet
  {
      digitalWrite(bitno, value);  // original Arduino function
  }
	else if (bitno < 96)  // I/O 64..95   (digital only) extended I/Os via shift registers
  {
      if (value == HIGH)
        _eOut |= 1 << (bitno & 0x1f);
      else
        _eOut &= ~(1 << (bitno & 0x1f));
  }
  else if (bitno < 100)  //  I/O 96..99   (analog only) mapping to ADC1115 inputs
  {
    
  }
  else if (bitno < 104)     /*  I/O 100      IBT-2 (I): analogWrite lets the motor turn (0 = brake)
                             *  I/O 101      IBT-2 (I): analogWrite lets the motor turn the other direction (0 =IBT-2 (I) tristated)
                             *  I/O 102      IBT-2 (II): analogWrite lets the motor turn (0 = brake)
                             *  I/O 103      IBT-2 (II): analogWrite lets the motor turn the other direction (0 =IBT-2 (II) tristated)*/ 
  {
      if (value == LOW)
      {
        /////////////// tristate IBT-2
      }
      else
      {
        /////////////// brake IBT-2
      }
  }
  else if (bitno < 196)  //  I/O 128..195   external Modbus relays
  {
      if (value == LOW)       RTURelOff(((bitno >> 4) & 0x03) + 1, (bitno & 0x0f) + 1);
      else                    RTURelOn (((bitno >> 4) & 0x03) + 1, (bitno & 0x0f) + 1); 
  }
}

/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
*/
/**************************************************************************/
uint8_t AgriNode::_digitalRead(uint8_t bitno)
{
  if (bitno < 64)  //  I/O  0..39   I/O of ESP32 acc. Espressif datasheet
  {
      return digitalRead(bitno);  // original Arduino function
  }
  else
  if (bitno < 96)
	{
    	if (_eIn & (1 << (bitno & 0x1f)))
    		return(HIGH);
    	else
    		return(LOW);
	}
}

/**************************************************************************/
/*!
    @brief  Reads 32-bits extended I/O data
*/
/**************************************************************************/
uint32_t AgriNode::digitalReadExtIO()
{
  return _eIn;
}

/**************************************************************************/
/*!
    @brief  Writes 32-bits extended I/O data
*/
/**************************************************************************/
void AgriNode::digitalWriteExtIO(uint32_t value)
{
  _eOut = value;
}

/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
*/
/**************************************************************************/
void AgriNode::_analogWrite(uint8_t bitno, uint8_t value)  // there is only one DAC - no PWM here
{
    if (bitno < 104) dacWrite(DAC, value); 
    else if (bitno < 128) //  I/O 100..103    IBT-2: analogWrite lets the motor turn
    {
        ledcWrite((bitno >> 1) & 0x01, value);     // set PWM value
        if (bitno & 0x02)
        {
          /////////////// set direction IBT-2 (II)
        }
        else
        {
          /////////////// set direction IBT-2 (I)
        }
    }
    else if (bitno < 196) //  I/O 128..196  Modbus relay timer
    {
        RTURelDly(((bitno >> 4) & 0x03) + 1, (bitno & 0x0f) + 1, value);
    }
}

/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
*/
/**************************************************************************/
uint16_t AgriNode::_analogRead(uint8_t channel)
{
	switch (channel)
	{
		case 100: return(_ads.readADC_SingleEnded(0));
		case 101: return(_ads.readADC_SingleEnded(1));
		case 102: return(_ads.readADC_SingleEnded(2));
		case 103: return(_ads.readADC_SingleEnded(3));
		case Ain4: return(analogRead(Ain4));
		case Ain5: return(analogRead(Ain5));
		case Ain6: return(analogRead(Ain6));
		case Ain7: return(analogRead(Ain7));
	}
	return (0x8000);
}
    

/**************************************************************************/
/*!
    @brief  Calculate 16-bit CRC for Modbus RTU
*/
/**************************************************************************/
uint16_t AgriNode::generateCRC(uint8_t pData[], uint16_t messageLength)
{
    uint16_t crc = 0xFFFF;
    if((pData != NULL) && (messageLength >= 2))
    {
        uint16_t i = 0;       /*byte counter*/
        uint8_t j = 0;       /*bit counter*/

        for(uint8_t i=0; i<(uint16_t)((messageLength-2) & 0xFFFF); i++)
        {
            crc ^= (uint16_t)pData[i];
            for(j=8; j!=0; j--)
            {
                if((crc & 0x0001) != 0)
                {
                    crc >>= 1;
                    crc ^= 0xA001;
                }
                else
                {
                    crc >>= 1;
                }
            }
        }
    }
    return crc;
}

/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
*/
/**************************************************************************/
void AgriNode::RTUSnd06(uint8_t MB_ID, uint8_t MBAddr, uint16_t MBData)
{
	  _MBsent[0] = MB_ID;
	  _MBsent[1] = 6;
	  _MBsent[2] = 0;
	  _MBsent[3] = MBAddr;
	  _MBsent[4] = (uint8_t)((MBData>>8) & 0xff);
	  _MBsent[5] = (uint8_t)( MBData     & 0xff);
    _RS485sent = 8;
	  uint16_t CRC = generateCRC(_MBsent, _RS485sent);
	  _MBsent[6] = (uint8_t)( CRC        & 0xff);
	  _MBsent[7] = (uint8_t)((CRC >> 8)  & 0xff);

	  digitalWrite(_OE, HIGH);  // driver enable
    // Set alarm to call timer when _OE has to be reset
    timerAlarmWrite(_TimerRS485, 80, true);   // overall bits (=80)
    _intState = 0; 
    /* Start an alarm */
    timerAlarmEnable(_TimerRS485);

    digitalWrite(_OE, HIGH);  // driver enable

    for (uint8_t i=0; i<8; i++)
    {
		    Serial2.write(_MBsent[i]);
    }
}


/**************************************************************************/
/*!
    @brief  switches on Modbus relay on
*/
/**************************************************************************/
void AgriNode::RTURelOn(uint8_t address, uint8_t NoRel)
{
    RTUSnd06(address, NoRel, 0x0100);
}


/**************************************************************************/
/*!
    @brief  switches on Modbus relay off
*/
/**************************************************************************/
void AgriNode::RTURelOff(uint8_t address, uint8_t NoRel)
{
    RTUSnd06(address, NoRel, 0x0200);
}

/**************************************************************************/
/*!
    @brief  momentary function of Modbus relay
*/
/**************************************************************************/
void AgriNode::RTURelMom(uint8_t address, uint8_t NoRel)
{
    RTUSnd06(address, NoRel, 0x0500);
}


/**************************************************************************/
/*!
    @brief  delay function of Modbus relay
*/
/**************************************************************************/
void AgriNode::RTURelDly(uint8_t address, uint8_t NoRel, uint8_t mytime)
{
    RTUSnd06(address, NoRel, 0x0600 + mytime); 
}


/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
*/
/**************************************************************************/
uint8_t AgriNode::RTUChkResp()
{
    uint8_t rec[20];
    uint32_t i = 0;
    uint32_t j = 0;
    
    while (Serial2.available() && i < 20)
    {
        rec[i] = Serial2.read();
        i++;
    }
    for (j=_RS485sent; j>0; j--)
        if (_MBsent[_RS485sent-j] != rec[i-j])
            break;
    _RS485sent = 0;
    return j;
}            
 
