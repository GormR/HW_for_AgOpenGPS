# Central Unit V 3.0 AgOpenGPS/QtOpenGuidance  
(Informationen in deutscher Sprache unten)
![pic](3D.png?raw=true)

This project includes all hardware function that are needed to run AgOpenGPS on one board. A 2nd board is only needed for Section Control, which can be placed inside the same housing. This setup uses the Teensy 4.1 board as control unit and is therefore not compatible with Brian's code. Nevertheless, there is software for Teensy (I'll add links later...)

![pic](documentation/Structure_Teensy.png?raw=true)

The [drawings/Structure_Teensy.png](full block diagram) is simple, because all controlling functions can be integrated inside the NXP controller, which is very powerfull. It would even be possible to run the complete AgOpenGPS on it and just provide a webbased interface for the tablet/mobile phone.

Functions' overview:

- I/Os needed for AOG
- connection to Ethernet
- USB host and device interfaces
- power supply
- motor driver incl. current feedback
- socket for one or two RTK units (Ardusimple)
- WAS input
- RS485 / Modbus
- 3 CAN


Special focus is given to the robust housing and industrial M12/M8 connetors (optional). PushIn connectors may also be used for single wire connections).

![pic](documentation/Full_M8_M12.jpg?raw=true)
![pic](documentation/Full_PushIn.jpg?raw=true)

# 160 x 100 mm² 
2-layer 35µm, 1,6mm thickness, technology: 0.15mm/0.15mm/0.3mm; SMD assembly: 72 different parts, 252 parts overall, THT: only 2.54mm headers and plugs of your choice

[Link to EasyEDA project](https://easyeda.com/GoRoNb/agopengps-main-box_copy). The pick-and-place data in the [production data](production_data) is already rotated correctly for JLC.

# Functions in-cabin-unit for autosteering (all SMD + THT connectors)
- Teensy socket
- ADC (ADS1115) for wheel angle sensor (WAS)
- BNO085 option (as CMPS14 board)
- motor driver (H-bridge of IBT-2) with 2-channel switch-off
- current feedback from motor driver (may be used to quit autosteering on driver interaction)
- 2 protected digital inputs (WORK usable as analog input, too)
- one optional digital optoisolated output
- mounting holes for RKT unit board(s) (for dual-RTK)
- 3 CAN 
- many LEDs
- Ethernet/UDP option 
- WIFI option via Wifi-Stick

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
5. connect WAS, buttons, motor and do a test with AgOpenGPS

debugging: use terminal like HTerm and find out the correct COM port: connect to each possible one, set "DTR" and release "DTR". When releasing, the red LED blinks 3 times (bootloader). The green LED next to U3 will blink when sending any serial data to the Nano; this is also true when sending with the Arduino IDE (serial monitor)

Further information are also provided in the upper folder.

There is also an [English user group](t.me/agopengpsinternational) and a [multilingual forum](https://agopengps.discourse.group/t/smd-pcb-project-for-an-all-in-one-compact-pcb-for-aog-qog/3640). 

----------------------------------------------------------------------------------------------------------------------------

# Functional Description (to be continued..)

The part including roll sensor, ADC and digital inputs is a copy of what BrianTee sugested. Many thanks for his great work!

Dual-RTK and up to 3 CANs are provided. 

The motor driver is the one of the IBT-2. It comes with separated 2-channel power lines at the power plug, so that a double-pole switch with positively opening contacts may be used for safe switch-off. This is especially mandatory for hydraulic steering. Furthermore, a DCDC converter may be used to generate 24V for driving the common Phidgets-Motor with increased dynamic.

The fifth pin of the power plug may either be used for a protected digital output (jumper on 1-2 of J8) or as on-switching signal (jumper on 2-3 of J8). No high currents have to be switched – a small SPST switch is ok. A switch inside the housing would be connected to 3-4 of J8. If an external switch is used to cut of the supply directly, apply a jumper to 2-3 of J8.

All digital inputs are protected and made of constant current sinks. Therefore, the input is inverted, so that either NC pushbuttons may be used or the Arduino firmware may be modified.

All voltage and I/O functions are equipped with LEDs.

The PCB project was set up with the EDA program „EasyEDA“, which is freely usable. The Altium export has not been checked. 

Milling the aluminum housing, if using M8/M12 (see [drawings](mechanical_data)):
- let the CNC machine drill the 4 holes
- fix the cover with 4 screws M2.5x15
- mill the holes and the counterboring (tool: D=2mm, feed: 300mm/min, 12000RPM, advance: 2mm)
- repeat this step for the other cover

# Functional Safety Advice

Steering systems have to meet at least the agricultural performance level AgrPL d acc. UN ECE R79 (see professional systems like [this](https://www.mobil-elektronik.com/uploads/media/Prospekt_Brochure_ME_Agricultural_Vehicles_ENG.pdf))!

Especially with hydraulic control, the vehicle may likely get uncontrollable on a failure of the valve-controlling electronic which may result in fatal casualties or serious injuries!

For AgOpenGPS, the required safety level may be archieved by the fail-safe absense of power for the valve/motor controlling unit. This function must have at least the same safety/performance level.

The mandatory application for approval may base on the following arguments based on ISO 13849: 

- emergency-stop key meets IEC 60947-5-5/ISO 13850, in line with fuse meeting IEC60269-1 (both acc. to table D.3, ISO 13849-2)
- use of MELF type resistors for all 8.2kOhm resistors to meet table D.14, ISO 13849-2
- module is mounted inside a IP54 housing (acc. table D.5, ISO 13849-2)
- wiring acc. IEC 60204-1 (acc. table D.4, ISO 13849-2) 
- measure against tin whiskers are taken (acc. D.2.2, ISO 13849-2), e. g. use of 60/40 solder paste
- futhermore the software shall check the values of the signal „safesense“

(some ordering information underneath the German text)

# (German copy) Zusammenfassung

Dieses Projekt fasst alle Hardwarefunktionen, die zum Betrieb von AgOpenGPS notwendig sind, auf einem Board zusammen. Lediglich für Section Control kann eine weitere Leiterplatte hinzugefügt werden, die wahlweise aber auch im selben Gehäuse untergebracht werden kann. Ziel war es nicht, neue Hardwarefunktionen bereitzustellen, sondern eine robuste, monolitische, von professionellen Bestückern herstellbaren Einheit zu entwicklern. Es wird ein Teensy-µC-Board verwendet, das wesentlich leistungsfähiger als Arduino-Boards ist und alle notwendigen Funktionen aufnehmen kann bis hin zur vollständigen Integration von AgOpenGPS (in dem Fall würde das User-Interface als Website auf einem Tablet/Handy dargestellt). Das Board ist _nicht_ zu Brians Software kompatible. Die bestehenden Teensy-Implementierungen muss ich noch verlinken.

Es werden alle Steuerungsfunktionen für DC-Motoren und Hydraulikventile bereitgestellt, sowie die dafür notwendige Sensorik. Eine Anbindung ist über USB (beforzugt), Ethernet und WiFi möglich. Die Software von Brian und MTZ kann ohne Änderungen übernommen werden, wenn USB verwendet wird. 

![pic](documentation/Structure_Teensy.png?raw=true)

Durch den Einsatz des Teensy als zentralen µC ergibt sich eine einfache Struktur. 

Besonderer Wert wurde auf ein robustes Gehäuse und insbesondere robuste Steckverbinder und Kabel gelegt. Es sind M12/M8-Steckverbinder gerade oder abgewinkelt vorgesehen. Alternativ sind über auch steckbare PushIn-Steckverbinder bestückbar.

Folgende Funktionen sind vorgesehen:

- Teensy-Steckplatz
- Steckplätze für (dual-)RTK
- ADC (ADS1115) für Lenkwinkelsensor
- Neigungssensor (BNO085/CMPS14)
- Motortreiber (H-Brücke wie IBT-2) mit zweikanaliger Abschaltung
- geschützte Digitaleingänge (als Konstantstromsenke)
- CAN2.0 (max 3 Schnittstellen)
- RS485/ModbusRTU
- ganz viele Leuchtdioden

Folgende Anschlüsse sind vorgesehen:
- Stromversorgung 12V/24V (optional für Motor)
- USB-C zu Tablet/Notebook
- Lenkwinkelsensor
- Digitaleingänge
- I²C via M8 - Kabel zu IMU
- Lenkradmotor bzw. Hydraulikeinheit
- CAN2.0
- RS485/ModbusRTU
- Inkrementalgeber für Lenkradsensor
- USB-C 
- QWIIC für I²C-Kram

# Im Detail 

(wird fortlaufend ergänzt, allerdings meistens zunächst der englische Teil)

Der Motortreiber entspricht dem IBT-2. Er verfügt über eine getrennte, zweikanalig ausgeführte Stromversorgung am Spannungsversorgungsstecker, sodass hier ein geeigneter zweikanaliger zwangsöffnender Sicherheitsschalter angeschlossen werden kann. Insbesondere bei Hydraulikeinbindung ist dies zwingend. Auch kann so ein DC/DC-Wandler 12V > 24V vorgeschaltet werden, um eine höhere Lenkdynamik z. B. für den beliebten Phidgets-Motor zu erreichen.

Die 5. Leitung des Spannungsversorgungssteckers kann entweder mit einem geschützten Digitalausgang belegt werden (Jumper auf 1-2 von J8) oder für den Ein-Schalter verwendet werden (Jumper auf 2-3 von J8). Es wird keine Leistung geschaltet – ein kleiner Schalter reicht. Ein Schalter im Gehäuse wird an die 3-4 von J8 angeschlossen. Wird ein externer Leistungsschalter verwendet, müssen diese Kontakte mit einem Jumper überbrückt werden.

Die Digitaleingänge sind als Konstantstromsenke ausgeführt. Dadurch ist die Logik allerdings invertiert, also bitte Öffnerkontakte verwenden oder die Arduino-Firmware entsprechend anpassen.

Alle Spannungen und I/O-Funktionen verfügen über Leuchtdioden.

Neben den I/O-Steckverbindern sollte mindestens P1 (Stiftleiste 2x5pol RM 2,54mm) bestückt werden, um die initiale Programmierung mit Hilfe eines Arduino Uno durchführen zu können. Notfalls können dort aber auch Kabel an- und nach erfolgter Programmierung wieder abgelötet werden. Je nach Einschaltoption wird noch J8 (Stiftleiste 1x4pol RM 2,54mm) und ein Jumper benötigt.

Das Leiterplattenprojekt wurde im frei verfügbaren EDA-Programm „EasyEDA“ erstellt.

[Link zum EasyEDA-Projekt](https://easyeda.com/GoRoNb/agopengps-main-box_copy).

Weitere allgemeine Informationen finden sich auch im übergeordneten Ordner.

Hilfestellung gibt es auch in der  [deutschsprachigen Nutzergruppe](t.me/AGOpenGPSGerman) und im [mehrsprachigen Forum](). 

Speziell zur Leiterplatteninbetriebnahme gibt es auch ein [Video](https://www.youtube.com/watch?v=w10TqOEIxdU).

# Erstinbetriebnahme
1. Ggf. fehlende SMD-Bauteile [nachbestücken](https://www.youtube.com/watch?v=dLczChhmDCY)
2. gewünschte Stiftleisten und Stecker auflöten
3. langsam die Spannung an CN4/POWER mit einer Strombegrenzung von 100mA hochregeln und gleichzeit die 5V-Spannung im Auge behalten (Eingangsstrom: 0mA bis ca. 4V, 20mA @ 4.5V, 12mA @ 12V; +5V darf nicht über 5,1V ansteigen!)
4. +3.3V überprüfen (sollte im Bereich 3.2..3.4V sein)
5. Lenkwinkelsensor (auch als WAS oder LWS bezeichnet), Schalter, Motor anschliessen und mit AgOpenGPS testen

debugging: use terminal like HTerm and find out the correct COM port: connect to each possible one, set "DTR" and release "DTR". When releasing, the red LED blinks 3 times (bootloader). The green LED next to U3 will blink when sending any serial data to the Nano; this is also true when sending with the Arduino IDE (serial monitor)

Aktuell (2/2021) nicht bei JLCPCB bestückbar: U2, U10 (jeweils Lager leer), U11 (nicht bei LCSC verfügbar) und Steckverbinder (prinzipiell)

Fräsen des Alugehäuses, falls M8/M12 verwendet werden (siehe auch [Zeichnungen](mechanical_data)):
- mit der CNC-Maschine 4 Löcher bohren
- Seitenteil mit 4 Schrauben M2.5x15 dort befestigen
- Löcher und Senkung fräsen (Fräser: D=2mm, Vorschub: 300mm/min, 12000RPM, Zustellung: 2mm)
- Mit dem anderen Seitenteil die anderen Fräsdaten verwenden

# Hinweis zur Funktionalen Sicherheit

Fest verbaute Lenksystem, die z. B. in die Hydraulik eingreifen, müssen den Agricultural performance level AgrPL d nach UN ECE R79 (siehe z. B. professionelle Systeme wie [dieses](https://www.mobil-elektronik.com/uploads/media/Prospekt_Brochure_ME_Agrarfahrzeuge_DEU.pdf)) erfüllen!

Im Falle eines Ausfalls und dadurch ausgelösten Lenkbewegungen kann es zu tödlichen Unfällen oder schweren Verletzungen kommen!

Mit AgOpenGPS kann der notwendige Sicherheitslevel durch die fehlersichere Abschaltung der elektrischen Leistung für die Lenkaktoren erfolgen. Diese Abschaltung muss mindestens diesen Sicherheitslevel erfüllen.

Bei passender Installation mag eine Abnahme mag auf folgenden Argumenten basieren, die sich auf die EN-ISO 13849 stützen: 

- Schalter für die Stromversorgung der Lenkaktorik entspricht EN-IEC 60947-5-5/EN-ISO 13850, die Vorsicherung entspricht IEC60269-1 (beides entsprechend Tabelle D.3, EN-ISO 13849-2)
- die Leiterplatte enthält MELF-Widerstände (8.2kOhm), die Tabelle D.14, EN-ISO 13849-2 erfüllen (standardmäßig ist hier 1206 bestückt)
- die Leiterplatte ist in ein Gehäuse eingebaut, das mindestens IP54 erfüllt (Tabelle D.5, EN-ISO 13849-2)
- Die Verkabelung ist geschützt gemäß EN-IEC 60204-1 (Tabelle D.4, EN-ISO 13849-2) 
- Massnahmen gegen "tin whiskers" wurden getroffen, z. B. mit 60/40 Lötzinn gelötet (nach D.2.2, EN-ISO 13849-2)
- die Firmware sollte die Spannung des Signals „safesense“ messen und plausibilisieren

Alle Daten diese Repositories stehen unter der CERN Open-Hardware-Lizenz 1.2. Jede Haftung für Personenschäden und Sachschäden auf Grund Fehlern der Beschreibung oder Fehlern der Leiterplatte oder Software wird ausgeschlossen! 

# Component Sources (examples) / Beispiele Bezugsquellen:

M12/M8, general components:	    [Mouser](https://www.mouser.com/Connectors/Circular-Connectors/Circular-Metric-Connectors/_/N-76q5i?Keyword=conec&FS=True), [Reichelt](https://www.reichelt.de/de/en/sensor-actor-connectors-c7505.html?MANUFACTURER=CONEC&START=0&OFFSET=16&LANGUAGE=EN&&r=1) - [Reichelt list](https://www.reichelt.de/my/1816987), [cable e. g.](https://catalog.weidmueller.com/catalog/Start.do?localeId=en&ObjectID=9457610150) 

(see [BOM-THT](production_data) for further information)

PushIn:		  Weidmueller [S2C-SMT 3.50/04/180G 3.5SN BK BX](https://catalog.weidmueller.com/catalog/Start.do?localeId=en&ObjectID=1290030000) + [B2CF 3.50/04/180 SN OR BX](https://catalog.weidmueller.com/catalog/Start.do?localeId=en&ObjectID=1277270000) via [Conrad](https://www.conrad.com)/[digikey](https://www.digikey.com)

(see [BOM-THT](production_data) for further information)

RTK receiver: 
- [Ardusimple](https://www.ardusimple.com/product/simplertk2b-basic-starter-kit-ip65/) (state-of-the-art receiver - mounting holes provided / Standardempfänger - Montagelöcher hierfür sind vorgesehen)
