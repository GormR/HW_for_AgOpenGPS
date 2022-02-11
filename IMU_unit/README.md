# IMU Unit for AgOpenGPS/QtOpenGuidance  
(Informationen in deutscher Sprache unten)
![pic](3D.png?raw=true)

This project is an IMU unit for AgOpenGPS, working via a fool-proof two-wire twisted pair bus for power and data. It will be connectable to the central units 2 & 3 in the future.


Functions' overview:

- Atmel section 
- power supply
- WAS input
- IMU

Special focus is given to the robust housing. PushIn connectors may also be used for single wire connections).

# 42 x 42 mm² 
2-layer 35µm, 1,6mm thickness, technology: 0.15mm/0.15mm/0.3mm; SMD assembly

[Link to EasyEDA project](https://oshwlab.com/GoRoNb/imu-for-agopengps). The pick-and-place data in the [production data](production_data) is already rotated correctly for JLC.

# Functions in-cabin-unit for autosteering (all SMD + THT connectors)
- integrated Arduino-Nano hardware
- ADC (ADS1115) for wheel angle sensor (WAS)
- BNO085 option (as CMPS14 board)
- many LEDs

These [connections](Connections.pdf) are provided:
- Wheel angle sensor (WAS)
- 2-wire bus

# Initial Operation Instructions
0. [read changelog](changelog.txt)
1. if applicable: [assemble missing SMD parts](https://www.youtube.com/watch?v=dLczChhmDCY)
2. assemble alls THT parts: pin headers and connectors
3. slowly increase input voltage on +5V with a current limit of 30mA while checking the 3.3V voltage; +3.3V must not rise up to more than 3.4V!)
4. connect Arduino Uno board and flash bootloader via PRGBOOT connector (https://www.arduino.cc/en/Tutorial/BuiltInExamples/ArduinoISP)* 
5. disconnect Uno board and connect an USB-serial-adapter with 3.3V signals to the connector "PRGARDU"
6. flash [firmware](code) via Arduino software => blue TXD3-LED starts flashing (the firmware is a copy of [BrianTee's Arduino code](https://github.com/farmerbriantee/AgOpenGPS/blob/master/Support_Files.zip) with mods to support RS485 data flow steering)
7. test with AgOpenGPS with the help of an [powerlink USB stick](https://github.com/GormR/RS485-OOK-USB-Adapter) or [Central Unit 2.x](https://github.com/GormR/HW_for_AgOpenGPS/tree/main/central_unit_2.0) or [Central Unit 3.x](https://github.com/GormR/HW_for_AgOpenGPS/tree/main/central_unit_3.0_Teensy)

*_ either with a 3.3V-Arduino or Uno with adapter ( [1k in each signal line + additional diode in _reset line](documentation/adapter_for_Arduino_Uno.jpg) ) 

There is also an [English user group](t.me/agopengpsinternational) and a [multilingual forum](https://github.com/GormR/HW_for_AgOpenGPS/tree/main/central_unit_2.0). 

----------------------------------------------------------------------------------------------------------------------------

# Functional Description (to be continued..)


All voltage and I/O functions are equipped with LEDs.

The PCB project was set up with the EDA program „EasyEDA“, which is freely usable. 

# (German copy) Zusammenfassung

Dieses Projekt ist eine kompakte IMU-Unit für AgOpenGPS.

Besonderer Wert wurde auf ein robustes Gehäuse gelegt. 

Folgende Funktionen sind vorgesehen:

- integrierte Arduino-Nano-Hardware
- ADC (ADS1115) für Lenkwinkelsensor
- Neigungssensor (BNO085/CMPS14)
- QWIIC für I²C-Kram

# Im Detail 

(wird fortlaufend ergänzt, allerdings meistens zunächst der englische Teil)


Das Leiterplattenprojekt wurde im frei verfügbaren EDA-Programm „EasyEDA“ erstellt. 

[Link zum EasyEDA-Projekt](https://oshwlab.com/GoRoNb/imu-for-agopengps)

Weitere allgemeine Informationen finden sich auch im übergeordneten Ordner.

Hilfestellung gibt es auch in der  [deutschsprachigen Nutzergruppe](t.me/AGOpenGPSGerman) und im [mehrsprachigen Forum](). 

# Erstinbetriebnahme
1. Ggf. fehlende SMD-Bauteile [nachbestücken](https://www.youtube.com/watch?v=dLczChhmDCY)
2. gewünschte Stiftleisten und Stecker auflöten
3. langsam die Spannung an CN4/POWER mit einer Strombegrenzung von 100mA hochregeln und gleichzeit die 5V-Spannung im Auge behalten (Eingangsstrom: 0mA bis ca. 4V, 20mA @ 4.5V, 12mA @ 12V; +5V darf nicht über 5,1V ansteigen!)
4. +3.3V überprüfen (sollte im Bereich 3.2..3.4V sein)
5. Arduino-Uno-Board mit P1 verbinden und Bootloader flashen (https://www.arduino.cc/en/Tutorial/BuiltInExamples/ArduinoISP) 
6. Uno-Board abstöpseln
7. [BrianTee's Arduino-Code](https://github.com/farmerbriantee/AgOpenGPS/blob/master/Support_Files.zip) wie gewohnt mit der Arduino-Software flashen
9. Lenkwinkelsensor (auch als WAS oder LWS bezeichnet) anschliessen und mit AgOpenGPS testen


# Component Sources (examples) / Beispiele Bezugsquellen:

(see [BOM-THT](production_data) for further information)

PushIn:		  Weidmueller [S2C-SMT 3.50/04/180G 3.5SN BK BX](https://catalog.weidmueller.com/catalog/Start.do?localeId=en&ObjectID=1290030000) + [B2CF 3.50/04/180 SN OR BX](https://catalog.weidmueller.com/catalog/Start.do?localeId=en&ObjectID=1277270000) via [Conrad](https://www.conrad.com)/[digikey](https://www.digikey.com)

(see [BOM-THT](production_data) for further information)


