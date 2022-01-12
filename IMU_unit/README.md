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
3. slowly increase input voltage on CN4/POWER with current limit of 100mA while checking the 5V voltage (should be 0mA till about 4V, 20mA @ 4.5V, 12mA @ 12V; +5V must not rise up to more than 5.1V!)
4. check +3.3V (should be in the range 3.2..3.4V)
5. connect Arduino Uno board and flash bootloader via PRGBOOT (https://www.arduino.cc/en/Tutorial/BuiltInExamples/ArduinoISP)* 
6. disconnect Uno board and connect USB (either CN2 or CN13/CN8/USB-C): The PC should display a new USB hub at least one several serial device 
7. flash [BrianTee's Arduino code](https://github.com/farmerbriantee/AgOpenGPS/blob/master/Support_Files.zip) via Arduino software => blue TXD3-LED starts flashing
8. connect WAS and do a test with AgOpenGPS

Some currents:
5V 25mA

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


