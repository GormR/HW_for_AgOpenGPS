#! python
__author__ = 'GoRoNb'

import serial


def relay(number, state):  # number in 1..8, state in 0..1 (= off, on)
    CRC = [0xD9, 0x6A, 0x29, 0x6A, 0x78, 0xAA, 0xC9, 0x6B, 0x98, 0xAB, 0x68, 0xAB, 0x39, 0x6B, 0x09, 0x68,
           0xD9, 0x9A, 0x29, 0x9A, 0x78, 0x5A, 0xC9, 0x9B, 0x98, 0x5B, 0x68, 0x5B, 0x39, 0x9B, 0x09, 0x98]
    ser = serial.Serial('COM45', 9600)  # open serial port @ 9600 Baud
    # send: address_of_node, command, mem address (16bit), on/off (16bit), CRC (16bit)
    ser.write([1, 6, 0, number, 2 - state, 0, CRC[2 * number + 16 * state - 2], CRC[2 * number + 16 * state - 1]])
    ser.close()                   # close port


# Examples:

# relay 7 on
relay(7, 1)

# relay 3 off
relay(3, 0)

