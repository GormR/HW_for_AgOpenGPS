/*
 * tcpm_driver.c  // Type C Port Manager
 *
 * Created: 11/11/2017 18:42:26
 *  Author: jason
 */ 

#include "tcpm_driver.h"
#include "Arduino.h"
#include <Wire.h>

extern const struct tcpc_config_t tcpc_config[CONFIG_USB_PD_PORT_COUNT];

extern "C" {
  //#include <Wire.h>
const int debug0 = 11;
const int debug1 = 12;

  void WirebeginTransmission(int addr)
  {
    Wire1.beginTransmission(addr);
  }

  void Wirewrite(int value)
  {
    Wire1.write(value);
  }
  
  void WireendTransmission(int stopBit)
  {
    Wire1.endTransmission(stopBit);
  }

  void WirerequestFrom(int addr, int quantity, int stopBit)
  {
    Wire1.requestFrom(addr, quantity, stopBit);
  }

  int Wireread()
  {
    return Wire1.read();
  }
}

/* I2C wrapper functions - get I2C port / slave addr from config struct. */
int tcpc_write(int port, int reg, int val)
{
  digitalWrite(debug0, HIGH);
  WirebeginTransmission(fusb302_I2C_SLAVE_ADDR);
  Wirewrite(reg & 0xFF);
  Wirewrite(val & 0xFF);
  WireendTransmission(true);
  digitalWrite(debug0, LOW);
  
  Serial.print("[0x");
  Serial.print(reg < 16 ? "0" : "");
  Serial.print(reg, HEX);
  Serial.print("]=0x");  
  Serial.print(val < 16 ? "0" : "");
  Serial.println(val, HEX);

  return 0;
}

int tcpc_write16(int port, int reg, int val)
{
  WirebeginTransmission(fusb302_I2C_SLAVE_ADDR);
  Wirewrite(reg & 0xFF);
  Wirewrite(val & 0xFF);
  Wirewrite((val >> 8) & 0xFF);
  WireendTransmission(true);

  Serial.print("w16: ");
  Serial.print(reg);
  Serial.print(" := ");
  Serial.println(val);
  
  return 0;
}

int tcpc_read(int port, int reg, int *val)
{
  digitalWrite(debug1, HIGH);
  WirebeginTransmission(fusb302_I2C_SLAVE_ADDR);
  Wirewrite(reg & 0xFF);
  WireendTransmission(false);
  WirerequestFrom(fusb302_I2C_SLAVE_ADDR, 1, true);
  *val = Wireread();
  digitalWrite(debug1, LOW);

  Serial.print("          [0x");
  Serial.print(reg < 16 ? "0" : "");
  Serial.print(reg, HEX);
  Serial.print("]: 0x");  
  Serial.print(*val < 16 ? "0" : "");
  Serial.println(*val, HEX);

  return 0;
}

int tcpc_read16(int port, int reg, int *val)
{
  WirebeginTransmission(fusb302_I2C_SLAVE_ADDR);
  Wirewrite(reg & 0xFF);
  WireendTransmission(false);
  WirerequestFrom(fusb302_I2C_SLAVE_ADDR, 1, true);
  *val  = Wireread();
  *val |= (Wireread() << 8);

  Serial.print("r16: ");
  Serial.print(reg);
  Serial.print(": ");
  Serial.println(*val);

  return 0;
}

int tcpc_xfer(int port,
	const uint8_t *out, int out_size,
	uint8_t *in, int in_size,
	int flags)
{
  if (out_size)
  {
    WirebeginTransmission(fusb302_I2C_SLAVE_ADDR);
    for (; out_size>0; out_size--) {
      Wirewrite(*out);
      out++;
    }
    if (flags & I2C_XFER_STOP)
    {
      WireendTransmission(true);
    }
    else
    {
      WireendTransmission(false);
    }
  }

  if (in_size) {
    WirerequestFrom(fusb302_I2C_SLAVE_ADDR, in_size, (flags & I2C_XFER_STOP));
    for (; in_size>0; in_size--) {
        *in = Wireread();
        in++;
    }
  }

  return 0;
}
