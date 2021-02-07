

import minimalmodbus
import serial

# assign RS-485 interface to serial port (use "COM.." for Windows, "/dev/ttyUSB.." for Linux)
AOG_Modbus = minimalmodbus.Instrument("/dev/ttyUSB1", 1)
AOG_Modbus.serial.baudrate = 9600

Relay = 5       # relay# 5
State = 0x0100  # 0x0100 = on, 0x0200 = off

status = AOG_Modbus.write_register(Relay, State, 0, 6, False)
