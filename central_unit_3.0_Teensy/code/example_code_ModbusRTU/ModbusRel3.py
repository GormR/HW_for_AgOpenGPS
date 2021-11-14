#! python
__author__ = 'GoRoNb'

import serial


def setRel(module_address, number_of_setRel, on_off): 
    # send: address_of_node, command, mem address (16bit), on/off (16bit), CRC (16bit)
    ModbusRTUmessage = [module_address, 6, 0, number_of_setRel, 2 - on_off, 0]
    CRC = 0xffff
    for i in range(0, len(ModbusRTUmessage)):
        CRC ^= ModbusRTUmessage[i]
        for j in range(0, 8):
            if (CRC & 0x0001) != 0:
                CRC >>= 1
                CRC ^= 0xa001
            else:
                CRC >>= 1
    ModbusRTUmessage.append(CRC & 0x00ff)
    ModbusRTUmessage.append((CRC & 0xff00) >> 8)
    ser = serial.Serial('COM45', 9600)  # open serial port @ 9600 Baud
    ser.write(ModbusRTUmessage)         # send command
    ser.close()                         # close port


# Examples:

# Module with address 2, setRel 7 on
setRel(2, 7, 1)

# Module with address 2, setRel 3 off
setRel(2, 3, 0)

