# Universal ESP32-based node for agricultural/automotive use  
Status: Work in progress!

![pic](3D.png?raw=true)
[Project link](https://oshwlab.com/GoRoNb/generic_agri_node)

The AgriNode is designed as an inexpensive base board for an ESP32 module, optimized for agricultural/automotive use.

Apart from the connectors and modules, the board may be ordered completely SMD-assembled e. g. from JLCPCB for less than US$80 for 10 PCBs (tax not included).

The following functions are provided:

- ESP32 module e. g. AZ-delivery (mind the pinning - unfortunately there is no standard!)

- Switch-mode power supply 6..20V

- 4x 16bit analog inputs Ain0..3, if ADS11145 module is mounted
   Input voltage range: 0..15V
   Impedance: voltage: 57kOhm, current: 150Ohm shunt to 0V

- 4x 12bit analog inputs Ain 4..7
   Input voltage range: 0..3.3V
   Impedance: 1MOhm

- 6x fast precise digital inputs: Din0..5 
    Input voltage range: 0..15V
    Threshold: 2.4V typ.
    Hytheresis: 0.3V typ.
    Impedance: 4.7kOhm

- 24x digital inputs: Din8..31 
    Input voltage range: 0..15V
    Input low, if input < 1.3V max.
    Input hight, if output > 4.5V min.
    Impedance: 4.7kOhm // 10kOhm clamping to 5V

- 7x fast digital outputs Dout0..6
    Output voltage 0V/5V
    Output impedance: 1kOhm

- 24x digital outputs Dout8..31
    Output voltage 0V/5V
    Output impedance: 1kOhm

Optional connectors:
- I/O with push-in (see picture underneath)
- I/O with 2.54mm pin headers (24 DI + 24 DO)
- QWIIC to further I²C devices
- 2x to IBT-2 module
- to RS-232 module
- CAN (driver already provided onboard)
- RS-485 (e. g. Modbus; driver already provided onboard)
- Ethernet/UDP w/ W5500 shield

The I/O area is user-defined by a matrix:

![pic](documentation/routing_area.png?raw=true)

Mind the [description of the connections](documentation/Schematic_Drawings_Node.pdf).

