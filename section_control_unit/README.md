# Section Control Unit AgOpenGPS/QtOpenGuidance  
(Informationen in deutscher Sprache unten)

! This description is a draft and needs improvement...  ;) !

This project is a section control unit for AgOpenGPS. It is intended to be used together with the Central Unit 2.0 or Central Unit 3.0 (=Teensy) in the same or a separate housing (can be found here in Git as well). It may connected by either Ethernet, WiFi or RS485.

Brian's Concept only provided drivers for the sections w/o the possibility of manual interference. In that case, no rocker switches will be mounted, and the unit may also be located directly on the trailer.

MTZ8302 and WEder suggested a manual steering option in addition, so in general the functions provided by a Müller-Elektronik  control plus AgOpenGPS integration. To use that one or two of this PCB can be used, because both functions can either be provided on a single board or divided into two boards, of wich one contains the rocker switches and the µC and the other one (e. g. located on the trailer) the relays (and optionally another µC). If using a single µC-dual PCB solution, both units are conneced via a HDMI cable. If using two µC, options are USB, RS485, Ethernet or WiFi. 

Brian's software will work quite out-of-the box with minimal mods. MTZ8302's needs more adaptations, mainly due to the fact, that each pin can both drive the section and read back the status of the rocker switch.

- USB-C 
- Atmel section with I/Os needed for AOG
- connection to Ethernet via external module is possible
- power supply
- valves output
- RS485 / Modbus option
- ESP32 option (alternative to Atmel)

This setup does not include new functions but provides a robust, monolitic unit that may be manufactured by professionel EMS providers. 

Special focus is given to the robust housing and industrial PushIn connetors. 

# 150 x 100 mm² 
2-layer 70 or 35µm, 1,6mm thickness, technology: 0.2mm/0.2mm/0.3mm; SMD assembly: 72 different parts, 252 parts overall, THT: only relays, 2.54mm headers and plugs 

[Link to EasyEDA project](https://oshwlab.com/GoRoNb/aog_io-box_copy). The pick-and-place data in the [production data](production_data) is already rotated correctly for JLC.

# Functions in-cabin-unit for autosteering (all SMD + THT connectors)
- integrated Arduino-Nano hardware
- ESP32-Module 
- 8 valve outputs (relays)
- mounting holes
- USB-C / WiFi / RS485 / (Ethernet)
- CAN via ESP32
- virtual serial port for RS485/ModbusRTU
- many LEDs
- Ethernet/UDP option 
- WIFI option via ESP32

These [connections](Connections.pdf) are provided:
- Supply 12V/24V (optional for motor)
- USB-C to tablet/notebook
- Wheel angle sensor (WAS)
- Digital inputs
- USB for 5G stick for IMU
- Steering wheel motor / hydraulic unit
- CAN2.0
- RS485/ModbusRTU
- Incremental sensor for steering wheel
- USB-C
- QWIIC connector
- internal USB connectors
- RJ45 for Ethernet

# Initial Operation Instructions
1. if applicable: [assemble missing SMD parts](https://www.youtube.com/watch?v=dLczChhmDCY)
2. assemble alls THT parts: pin headers and connectors
3. slowly increase input voltage on CN4/POWER with current limit of 100mA while checking the 5V voltage (should be 0mA till about 4V, 20mA @ 4.5V, 12mA @ 12V; +5V must not rise up to more than 5.1V!)
4. check +3.3V (should be in the range 3.2..3.4V)
5. connect Arduino Uno board and flash bootloader via P1 (https://www.arduino.cc/en/Tutorial/BuiltInExamples/ArduinoISP) 
6. disconnect Uno board and connect USB (either CN2 or CN13/CN8/USB-C): The PC should display a new USB hub at least one several serial device 
7. flash [BrianTee's Arduino code](https://github.com/farmerbriantee/AgOpenGPS/blob/master/Support_Files.zip) via Arduino software
8. connect WAS, buttons, motor and do a test with AgOpenGPS

debugging: use terminal like HTerm and find out the correct COM port: connect to each possible one, set "DTR" and release "DTR". When releasing, the red LED blinks 3 times (bootloader). The green LED next to U3 will blink when sending any serial data to the Nano; this is also true when sending with the Arduino IDE (serial monitor)

Further information are also provided in the upper folder.

There is also an [English user group](t.me/agopengpsinternational) and a [multilingual forum](https://agopengps.discourse.group/t/smd-pcb-project-for-an-all-in-one-compact-pcb-for-aog-qog/3640). 

----------------------------------------------------------------------------------------------------------------------------

# Functional Description (to be continued..)

! Diese Beschreibung muss dringend noch erweitert werden...  ;) !

Bei diesem Board handelt es sich um eine Section-Control-Steuerung für AgOpenGPS. Sie kann zusammen mit der Central Unit 2.0 oder Central Unit 3.0 (=Teensy) in einem Gehäuse verbaut werden oder eben auch nicht. Ein Anbindung über Ethernet, WiFi oder RS485 ist auch möglich.

Brian's Konzept sieht lediglich eine Ansteuerung der Section ohne weitere Eingriffsmöglichkeit vor. In dem Fall werden keine Schalter bestückt, und die Einheit kann ggf. auf der Anhängespritze selbst angebaut werden.

MTZ8302 bzw WEder schlagen ergänzend die Möglichkeit einer alternativen Handsteuerung vor, also im Prinzip das, was eine Müller-Elektronik-Steuerung leistet plus die Einbindung in AgOpenGPS. Hierzu können ein- oder zwei Leiterplatten verwendet werden, da sich sowohl beide Funktionalitäten in einem Board bestücken lassen, als auch separat, also z. B. die Leiterplatte mit den Schaltern im Traktor und dieselbe Leiterplatte mit Relais bestückt im Trailer. Dabei kann sowohl eine ein-µC-Lösung gewählt werden, bei der das Relais-Modul via HDMI-Kabel verbunden wird, als auch eine zwei-µC-Lösung mit USB, RS485, Ethernet oder WiFi. 

Die Software von Brian läuft auf dem Atmel ohne große Änderungen. Die Software von MTZ8302 muss allerdings angepasst werden, da an einem Pin sowohl die passende section aktiviert wird, als auch der zugeordnete Schalter zurückgelesen wird.


# Erstinbetriebnahme
1. Ggf. fehlende SMD-Bauteile [nachbestücken](https://www.youtube.com/watch?v=dLczChhmDCY)
2. gewünschte Stiftleisten und Stecker auflöten
3. langsam die Spannung an CN4/POWER mit einer Strombegrenzung von 100mA hochregeln und gleichzeit die 5V-Spannung im Auge behalten (Eingangsstrom: 0mA bis ca. 4V, 20mA @ 4.5V, 12mA @ 12V; +5V darf nicht über 5,1V ansteigen!)
4. +3.3V überprüfen (sollte im Bereich 3.2..3.4V sein)
5. Arduino-Uno-Board mit P1 verbinden und Bootloader flashen (https://www.arduino.cc/en/Tutorial/BuiltInExamples/ArduinoISP) 
6. Uno-Board abstöpseln
7. über USB mit einem PC verbinden (entweder über CN2 oder CN13/CN8/USB-C): Der PC muss einen neuen Hub und mindestens eine neue serielle Schnittstelle anzeigen; bei mehrereren ist die des Arduino meistens die erste)
8. [BrianTee's Arduino-Code](https://github.com/farmerbriantee/AgOpenGPS/blob/master/Support_Files.zip) wie gewohnt mit der Arduino-Software flashen
9. Lenkwinkelsensor (auch als WAS oder LWS bezeichnet), Schalter, Motor anschliessen und mit AgOpenGPS testen

debugging: use terminal like HTerm and find out the correct COM port: connect to each possible one, set "DTR" and release "DTR". When releasing, the red LED blinks 3 times (bootloader). The green LED next to U3 will blink when sending any serial data to the Nano; this is also true when sending with the Arduino IDE (serial monitor)
