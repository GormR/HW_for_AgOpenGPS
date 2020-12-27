# AgOpenGPS/QtOpenGuidance Autosteer and RTK units
Status: Draft - to be tested!

These two PCBAs bring all electronic functions for autosteering with the AgOpenGPS / QtOpenGuidance project and an dual RTK-GNSS receiver as a separate unit.

This setup does not include new functions but provides a robust, monolitic unit  that may be manufactured by professionel EMS providers. The central unit is compatible to BrianTee‘s Nano firmware and the roof top unit to Matthias (MTZ8302) dual-RTK firmware. 

Special focus is given to the robust housing and industrial M12/M8 connetors (optional). PushIn connectors may also be used for single wire connections).

This functions are provided:
- integrated Arduino-Nano hardware
- option for ESP32-Module (Arduino Firmware has to be adapted)
- ADC (ADS1115) for wheel angle sensor (WAS)
- roll sensor (MMA8452)
- motor driver (H-bridge of IBT-2) with 2-channel switch-off
- protected digital inputs (as constant current sink)
- USB hub (4-port)
- power supply also for tablet/notebook via USB-C
- integrated CANtact dardware (2x CAN2.0)
- virtual serial port for RS485/ModbusRTU
- may LEDs

This connectors are provided:
- Supply 12V/24V (optional for motor)
- USB-C to tablet/notebook
- Wheel angle sensor (WAS)
- Digital inputs
- USB via M12-D cable to RTK-GNSS unit
- Steering wheel motor / hydraulic unit
- 2x CAN2.0
- RS485/ModbusRTU
- Incremental sensor for steering wheel
- 2x USB-C (tablet + DC converter/Aux)

Roof Top Unit:
- Dual RTK-GNSS receiver
- BNO085 option
- ESP32
