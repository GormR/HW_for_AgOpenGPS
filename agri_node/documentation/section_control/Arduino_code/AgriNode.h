/**************************************************************************/
/*!
    @file     AgriNode.h
    @author   G. Rose
    @license  GPL (see license.txt)

    Board Support Package for Generic AgriNode

    This is a library for the generic AgriNode board
    ----> https://github.com/GormR/HW_for_AgOpenGPS/tree/main/agri_node

    @section  HISTORY

    v1.0 - First release
*/

/*  numbering scheme of I/Os:
 *  I/O  0..39   I/O of ESP32 acc. Espressif datasheet
 *  I/O 64..95   (digital only) extended I/Os via shift registers 
 *  I/O 96..99   (analog only) mapping to ADC1115 inputs
 *  I/O 100      IBT-2 (I): analogWrite lets the motor turn, digitalWrite(LOW) = tristated, (HIGH) = brake
 *  I/O 101      IBT-2 (I): analogWrite lets the motor turn the other direction
 *  I/O 102      IBT-2 (II): ditto
 *  I/O 103      IBT-2 (II): ditto
 *  I/O 128..143 Relays via Modbus address 1 (HIGH = on, LOW = off, analogWrite = timed on)
 *  I/O 144..159 Relays via Modbus address 2
 *  I/O 160..175 Relays via Modbus address 3
 *  I/O 176..191 Relays via Modbus address 4
 */

#ifndef _AgriNode_h_
#define _AgriNode_h_

//##########################################################################
//##           application-specific part (section control)                ##
//##########################################################################

/////////////////////////////////////////////
// definitions dedicated to section control:
#define NoNoz 7         // number of nozzles

// results of soil distance:
#define TOK 0           // tilt ok
#define TTL 1           // tilt to left
#define TTR 2           // tilt to right
#define TIm 3           // tilt implausible (middle ultrasonic sensor != avarage of left and right)
#define TNA 4           // 4..7: error: at least one of the sensors did not answer (bits 0..2 contain which)

// outputs: 
#define SectionLED 0    // 0..6: LED section active (blinking if automatic wants to have it active, but nozzle is switched off)
#define GeneralLED 8    // on if any nozzle is active
#define AutoLED    9    // on if in automatic mode
#define ErrorLED  10    // LED to indicate any error, e. g. no flow when expected

// relay outputs (each set or reset command is a write to the Modbus RTU)
#define SectionRel 0    // 0..13: two relays for each nozzle: both off = nozzle off, both on = nozzle on
#define ExtPower  14    // switches the power to sensors, etc on
#define MagValve  15    // magnetic main valve: on = nozzles enabled

// inputs:
#define SectionAct 0    // 0..6: section active switch (high = nozzle active)
#define GeneralAct 7    // main switch for all nozzels
#define Auto_Man   8    // switch: active = automatic mode
#define plus10     9    // increase pressure by 10% in manual mode 
#define minus10   10    // decrease pressure by 10% in manual mode 

// IBTs:
#define MotRoll    0    // motor correcting the roll (CW = right down, CCW = left down)
#define MotPress   1    // motor of the pressure regulating valve (CW = more, CCW = less pressure)

// analog:
#define NoAnCh     1    // number of analog channels = 1 (only pressure)
//#define Pressure   0    // pump pressure signal to be adjusted with the help of MotPress

// interrupts:
#define Flow       1    // flowmeter interrupt
#define Speed      2    // counting wheel nuts of trailer..
#define Tank     255    // tank level sensor (not present for me)

//##########################################################################
//##               board-specific part (generic AgriNode)                 ##
//##########################################################################

/*=========================================================================
    SPI (extended I/O and Ethernet W5500)
    -----------------------------------------------------------------------*/
#define _MOSI  23
#define _MISO  19
#define _SCLK  18
#define _CSEth  5
#define _CSeIO 15
// SCLK and CSeIO have to be swapped with the next revision

/*=========================================================================*/

/*=========================================================================
    IBT-2
    -----------------------------------------------------------------------*/
#define PWM1    4
#define PWM2    2
/*=========================================================================*/

/*=========================================================================
    I²C
    -----------------------------------------------------------------------*/
#define _SCL    22
#define _SDA    21
/*=========================================================================*/

/*=========================================================================
    RS485(Modbus) XOR RS232
    -----------------------------------------------------------------------*/
#define _TxD   17
#define _RxD   16
#define _OE     0  // for both extended outputs (active low) and RS485-OE (active high)
/*=========================================================================*/

/*=========================================================================
    CAN2.0
    -----------------------------------------------------------------------*/
#define _RxCAN 25
#define _TxCAN 27
/*=========================================================================*/

/*=========================================================================
    digital I/O
    -----------------------------------------------------------------------*/
#define DI0    32
#define DI1    33
#define DI2    14
#define DI3    12
#define DI4    13  // use 13 (XOR to DO1) or 36 (XOR to Ain7)
#define DI5    39  // XOR to Ain6 - set jumper
#define DO0     5  // use 5 for USB and 1 for Ethernet and set jumper
#define DO1    13
/*=========================================================================*/

/*=========================================================================
    analog I/O
    -----------------------------------------------------------------------*/
#define Ain4   35
#define Ain5   34
#define Ain6   39
#define Ain7   36
#define DAC    26
/*=========================================================================*/

/*=========================================================================
    bits for IBT-2s and DO2..4
    -----------------------------------------------------------------------*/
#define R_EN2   0		  // = DO2
#define DO2	    0
#define L_EN2   1		  // = DO3
#define DO3     1
#define R_EN1   2
#define L_EN1   3  	      
#define PWM2_RPWM2 4  // = DO4
#define DO4        4
#define PWM2_LPWM2 5
#define PWM1_RPWM1 6
#define PWM1_LPWM1 7
/*=========================================================================*/

// modes for H-bridge:
#define idle  0
#define CCW   1
#define CW    2
#define brake 3

// modes for relay:
#define Ron    255
#define Roff   254
//value in 1..15: timer for relay on in seconds 

#include "Arduino.h"
#include "SPI.h"
#include "zADS1015.h"
#include <Wire.h>     // I²C for Atmel

//##########################################################################
//##           application-specific part (section control)                ##
//##########################################################################

void     _HbridgeWrite(uint8_t number, uint8_t Hmode, uint16_t PWMval);
uint8_t  _SoilDistanceRead();

//##########################################################################
//##               board-specific part (generic AgriNode)                 ##
//##########################################################################

class AgriNode
{
	  protected:
        // Instance-specific properties
     //   uint32_t _eOut;
     //   uint32_t _eIn;
	  public:
        uint8_t  _RS485sent = 0; // flag for RS485 package sent
        uint32_t _TimeRS485End;  // at this moment in time, the _OE can be deactivated
        uint32_t _TimeRS485Resp; // at this moment in time, the response is expected
        uint8_t  _MBsent[10];    // data send to Modbus client
        uint32_t _eOut;
        uint32_t _eIn;
/*=========================================================================
    I/O related functions
    -----------------------------------------------------------------------*/
		    AgriNode();

   	    void     _digitalWrite(uint8_t bitno, uint8_t value);       // services I/O instead of original Arduino one
		    uint8_t  _digitalRead(uint8_t bitno);
		    uint32_t digitalReadExtIO(); 
        void     digitalWriteExtIO(uint32_t value); 
        void     _analogWrite(uint8_t bitno, uint8_t value);  // there is only one DAC - no PWM here
		    uint16_t _analogRead(uint8_t bitno);
    
/*=========================================================================
    ModbusRTU related functions
    -----------------------------------------------------------------------*/
		    void     RTURelOn (uint8_t address, uint8_t NoRel);
		    void     RTURelOff(uint8_t address, uint8_t NoRel);
		    void     RTURelMom(uint8_t address, uint8_t NoRel);
        void     RTURelDly(uint8_t address, uint8_t NoRel, uint8_t mytime);
		    uint16_t generateCRC(uint8_t pData[], uint16_t messageLength);
        void     RTUSnd06 (uint8_t MB_ID, uint8_t MBAddr, uint16_t MBData);
		    uint8_t  RTUChkResp();

    private:
};
#endif
