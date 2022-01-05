# Central Unit V 2.0 AgOpenGPS/QtOpenGuidance  
(Informationen in deutscher Sprache unten)
![pic](3D.png?raw=true)

This project includes all hardware function that are needed to run AgOpenGPS on one board. A 2nd board is only needed for Section Control, which can be placed inside the same housing. This setup does not include new functions but provides a robust, monolitic unit that may be manufactured by professionel EMS providers. The central unit V2.0 is compatible to [BrianTee‘s Nano firmware](https://github.com/farmerbriantee/AgOpenGPS/blob/master/Support_Files.zip) and to my [improved version](code/Autosteer_USB_CMPS14_low_jitter_twistable/). The ESP32 option is compatible with MTZ8002's code as long as USB is used. The project is an update of the Central Unit V1.x which may still be used for single-RTK AgOpenGPS setups w/o limitations.

![pic](documentation/Structure_USB.png?raw=true)

The [drawings/Structure_USB.png](block diagram) for USB shows the 4-port USB hub on the left side providing 4 links:

1. virtual COM-Port for Atmel (Steering; provides Arduino Nano functionalities)
2. RTK-GNSS for positioning; either one directly connected Ardusimple board or the ESP32 via a virtual COM port like suggested by [AOG_Autosteer_ESP32](https://github.com/mtz8302/AOG_Autosteer_ESP32).
3. virtual COM port for Section Control (routed to a connector to the Section Control module (see subfolder here in Github, please)
4. USB receptacle for general propose (e. g. 5G stick, external MEMS, etc)

![pic](documentation/Structure_Ethernet_WiFi.png?raw=true)

For [drawings/Structure_Ethernet_WiFi.png](Ethernet), the W5500 has to be mounted and the software has to be adapted. Possibles solutions are both a  single-µC solution and a dual-core solution with ESP32 and Atmel, in which the ESP32 has to distribute the data sets. In that case, the 12V-to-5V power supply must be mounted, too, to generate the power for the logic. The same is true for WiFi.

![pic](documentation/Structure_all.png?raw=true)

The [drawings/Structure_all.png](full block diagram) looks a little bit ugly. All components can be mounted anyway and the choice of the final setup can be made later on. By placing the USB receptacle for the Ardusimple, the virtual COM-Port for dual-RTK is automatically deactivated e. g. 

Functions' overview:

- USB hub, USB devices, USB-C 
- Atmel section with I/Os needed for AOG
- connection to Ethernet
- power supply
- motor driver incl. current feedback
- WAS input
- RS485 / Modbus
- CAN
- ESP32 option



Special focus is given to the robust housing and industrial M12/M8 connetors (optional). PushIn connectors may also be used for single wire connections).

![pic](documentation/Full_M8_M12.jpg?raw=true)
![pic](documentation/Full_PushIn.jpg?raw=true)

# 160 x 100 mm² 
2-layer 35µm, 1,6mm thickness, technology: 0.15mm/0.15mm/0.3mm; SMD assembly: 72 different parts, 252 parts overall, THT: only 2.54mm headers and plugs of your choice

[Link to EasyEDA project](https://easyeda.com/GoRoNb/aog_io-box). The pick-and-place data in the [production data](production_data) is already rotated correctly for JLC.

# Functions in-cabin-unit for autosteering (all SMD + THT connectors)
- integrated Arduino-Nano hardware
- ESP32-Module (Arduino Firmware has to be adapted)
- ADC (ADS1115) for wheel angle sensor (WAS)
- BNO085 option (as CMPS14 board)
- motor driver (H-bridge of IBT-2) with 2-channel switch-off
- current feedback from motor driver (may be used to quit autosteering on driver interaction)
- 2 protected digital inputs (WORK usable as analog input, too)
- one optional digital optoisolated output
- mounting holes for RKT unit board(s) (for dual-RTK)
- USB hub (4-port)
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

Some currents:
out of the box with Atmel and ESP32: 200mA@9V,   153mA@12V,   124mA@15V
with 2 additional Ardusimples:       310mA@9V,   238mA@12V,   195mA@15V
()if supplied via USB: 250mA and 470mA)

debugging: use terminal like HTerm and find out the correct COM port: connect to each possible one, set "DTR" and release "DTR". When releasing, the red LED blinks 3 times (bootloader). The green LED next to U3 will blink when sending any serial data to the Nano; this is also true when sending with the Arduino IDE (serial monitor)

Further information are also provided in the upper folder.

There is also an [English user group](t.me/agopengpsinternational) and a [multilingual forum](https://agopengps.discourse.group/t/smd-pcb-project-for-an-all-in-one-compact-pcb-for-aog-qog/3640). 

----------------------------------------------------------------------------------------------------------------------------

# Functional Description (to be continued..)

The Arduino part including roll sensor, ADC and digital inputs is a copy of what BrianTee sugested. Many thanks for his great work!

An ESP32 module is mountable to provide dual-RTK and CAN. 

The motor driver is the one of the IBT-2. It comes with separated 2-channel power lines at the power plug, so that a double-pole switch with positively opening contacts may be used for safe switch-off. This is especially mandatory for hydraulic steering. Furthermore, a DCDC converter may be used to generate 24V for driving the common Phidgets-Motor with increased dynamic.

The fifth pin of the power plug may either be used for a protected digital output (jumper on 1-2 of J8) or as on-switching signal (jumper on 2-3 of J8). No high currents have to be switched – a small SPST switch is ok. A switch inside the housing would be connected to 3-4 of J8. If an external switch is used to cut of the supply directly, apply a jumper to 2-3 of J8.

All digital inputs are protected and made of constant current sinks. Therefore, the input is inverted, so that either NC pushbuttons may be used or the Arduino firmware may be modified.

The RS485 port is like what is also available with a cheap USB-RS485 stick based on a CH340 driver. 

The 4 port USB hub connects the tablet/Notebook via either CN2 (PushIn) or CN8/CN13/TO_PC (USB-C) to the RTK-GNSS receiver and to the Nano or ESP32, the CANtact hardware, the RS-485 port or the internal 2 USB-A socket or the external USB-C socket. These alternatives are possible:

- USB1:  Arduino Nano Clone  

- USB2:  ESP32 - dual-RTK or single Ardusimple RTK board

- USB3:  Section control PCB addon

- USB4:    External 

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

Dieses Projekt fasst alle Hardwarefunktionen, die zum Betrieb von AgOpenGPS notwendig sind, auf einem Board zusammen. Lediglich für Section Control kann eine weitere Leiterplatte hinzugefügt werden, die wahlweise aber auch im selben Gehäuse untergebracht werden kann. Ziel war es nicht, neue Hardwarefunktionen bereitzustellen, sondern eine robuste, monolitische, von professionellen Bestückern herstellbaren Einheit zu entwicklern, auf der der aktuelle Stand der Arduino-Firmware von BrianTee kann unverändert verwendet werden. Dieses Projekt ist der Nachfolger den Central-Unit 1.x, die nach wie vor für single-RTK-Anwendungen ohne Einschränkungen verwendet werden kann.

Es werden alle Steuerungsfunktionen für DC-Motoren und Hydraulikventile bereitgestellt, sowie die dafür notwendige Sensorik. Eine Anbindung ist über USB (beforzugt), Ethernet und WiFi möglich. Die Software von Brian und MTZ kann ohne Änderungen übernommen werden, wenn USB verwendet wird. 

![pic](documentation/Structure_USB.png?raw=true)

Das [drawings/Structure_USB.png](Blockschaltbild) für USB zeigt links den mit dem Tablet verbundenen 4-fach USB-Hub, der folgende 4 Schnittstellen bereitstellt:

1. virtueller COM-Port für Atmel (Lenkung; entspricht der Funktionalität eines Arduino Nano)
2. RTK-GNSS zur Ortsbestimmung; entweder wird über USB direkt ein Ardusimple-Board angesprochen oder über einen virtuellen COM-Port ein ESP32 entsprechen dem Vorschlag von [AOG_Autosteer_ESP32](https://github.com/mtz8302/AOG_Autosteer_ESP32).
3. virtueller COM-Port für Section Control (wird auf einen Steckverbinder geführt für externes Section-Control-Modul (siehe Unterordner hier im Github)
4. eine USB-Buchse für allgemeine Verwendung (z. B. 5G-Stick, externen MEMS, etc)

![pic](documentation/Structure_Ethernet_WiFi.png?raw=true)

Für [drawings/Structure_Ethernet_WiFi.png](Ethernet) muss der W5500 bestückt werden und die Software angepasst werden. Es ist dann sowohl eine single-µC-Lösung mit dem ESP32 als auch eine kombinierte ESP32/Atmel-Lösung möglich, bei der der ESP32 die Daten zentral verteilen muss. Zusätzlich muss das Schaltnetzteil bestückt werden, da ja die Logikspannung nun aus 12V erzeugt werden muss. Gleiches gilt für eine WLAN-Anbindung.

![pic](documentation/Structure_all.png?raw=true)

Durch die vielfältigen Optionen sieht das [drawings/Structure_all.png](Gesamtblockschaltbild) recht unübersichtlich aus. Es können aber alle Bauteile bestückt werden und zu einem späteren Zeitpunkt die genaue Topologie festgelegt werden. Z. B. wird durch Bestücken der USB-Buchse für einen Ardusimple automatisch der zugeordnete virtuelle COM-Port für dual-RTK deaktiviert.

Besonderer Wert wurde auf ein robustes Gehäuse und insbesondere robuste Steckverbinder und Kabel gelegt. Es sind M12/M8-Steckverbinder gerade oder abgewinkelt vorgesehen. Alternativ sind über auch steckbare PushIn-Steckverbinder bestückbar.

Folgende Funktionen sind vorgesehen:

- integrierte Arduino-Nano-Hardware
- ESP32 für dual-RTK
- ADC (ADS1115) für Lenkwinkelsensor
- Neigungssensor (BNO085/CMPS14)
- Motortreiber (H-Brücke wie IBT-2) mit zweikanaliger Abschaltung
- geschützte Digitaleingänge (als Konstantstromsenke)
- USB-Hub (4-port)
- CAN2.0 via ESP32
- virtueller serieller Port für RS485/ModbusRTU
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

Der Arduino-Nano-Teil einschließlich der Neigungssensoren, des ADC und der Eingänge entspricht dem von BrianTee entwickelten Schaltplan. An dieser Stelle sei ihm für seine hervorragende Arbeit gedankt!

Eine ESP32-Leiterplatte ist alternativ bestückbar. In diesem Fall muss die Arduino-Software angepasst werden und der Nano dauerhaft im Reset-Zustand gehalten werden (Jumper auf 5, 6 von P1). Mit einem ESP32 steht dann hardwaremäßig auch ein CAN2.0 und ein RS485-Port zur Verfügung sowie Wifi und Bluetooth.

Der Motortreiber entspricht dem IBT-2. Er verfügt über eine getrennte, zweikanalig ausgeführte Stromversorgung am Spannungsversorgungsstecker, sodass hier ein geeigneter zweikanaliger zwangsöffnender Sicherheitsschalter angeschlossen werden kann. Insbesondere bei Hydraulikeinbindung ist dies zwingend. Auch kann so ein DC/DC-Wandler 12V > 24V vorgeschaltet werden, um eine höhere Lenkdynamik z. B. für den beliebten Phidgets-Motor zu erreichen.

Die 5. Leitung des Spannungsversorgungssteckers kann entweder mit einem geschützten Digitalausgang belegt werden (Jumper auf 1-2 von J8) oder für den Ein-Schalter verwendet werden (Jumper auf 2-3 von J8). Es wird keine Leistung geschaltet – ein kleiner Schalter reicht. Ein Schalter im Gehäuse wird an die 3-4 von J8 angeschlossen. Wird ein externer Leistungsschalter verwendet, müssen diese Kontakte mit einem Jumper überbrückt werden.

Die Digitaleingänge sind als Konstantstromsenke ausgeführt. Dadurch ist die Logik allerdings invertiert, also bitte Öffnerkontakte verwenden oder die Arduino-Firmware entsprechend anpassen.

Die Hardware des [CANtact-Projektes](https://cantact.io/cantact-pro/users-guide.html) auf Basis des Nexperia-µC LPC54616 ist optional bestückbar und stellt dann 2 CAN2.0-Schnittstellen zur Verfügung. Eine der beiden CAN2.0-Schnittstellen kann alternativ auch mit dem ESP32 angesteuert werden.

Die RS-485-Schnittstelle entspricht dem, was ein preiswerter USB-RS485-Stick auf Basis einen CH340 leistet. 

Alle Spannungen und I/O-Funktionen verfügen über Leuchtdioden.

Neben den I/O-Steckverbindern sollte mindestens P1 (Stiftleiste 2x5pol RM 2,54mm) bestückt werden, um die initiale Programmierung mit Hilfe eines Arduino Uno durchführen zu können. Notfalls können dort aber auch Kabel an- und nach erfolgter Programmierung wieder abgelötet werden. Je nach Einschaltoption wird noch J8 (Stiftleiste 1x4pol RM 2,54mm) und ein Jumper benötigt.

Das Leiterplattenprojekt wurde im frei verfügbaren EDA-Programm „EasyEDA“ erstellt. 

[Link zum EasyEDA-Projekt](https://easyeda.com/GoRoNb/aog_io-box)

Weitere allgemeine Informationen finden sich auch im übergeordneten Ordner.

Hilfestellung gibt es auch in der  [deutschsprachigen Nutzergruppe](t.me/AGOpenGPSGerman) und im [mehrsprachigen Forum](). 

Speziell zur Leiterplatteninbetriebnahme gibt es auch ein [Video](https://www.youtube.com/watch?v=w10TqOEIxdU).

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
