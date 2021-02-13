# Central Unit AgOpenGPS/QtOpenGuidance  
Status: Draft - to be tested!

This setup does not include new functions but provides a robust, monolitic unit  that may be manufactured by professionel EMS providers. The central unit is compatible to [BrianTee‘s Nano firmware](https://github.com/farmerbriantee/AgOpenGPS/blob/master/Support_Files.zip). If the ESP32 option is used, the pin definitions of [esp32-aog](https://github.com/eringerli/esp32-aog) and [AOG_Autosteer_ESP32](https://github.com/mtz8302/AOG_Autosteer_ESP32) must be adapted.

Special focus is given to the robust housing and industrial M12/M8 connetors (optional). PushIn connectors may also be used for single wire connections).

![pic](3D.png?raw=true)
[Project link](https://oshwlab.com/GoRoNb/agopengps-main-box)
# Functions in-cabin-unit for autosteering (all SMD + THT connectors)
- integrated Arduino-Nano hardware
- option for ESP32-Module (Arduino Firmware has to be adapted)
- ADC (ADS1115) for wheel angle sensor (WAS)
- roll sensor (MMA8452)
- BNO085 option (as CMPS14 board)
- motor driver (H-bridge of IBT-2) with 2-channel switch-off
- current feedback from motor driver (may be used to quit autosteering on driver interaction)
- 3 protected digital inputs (2oo3 usable as analog inputs on ESP32)
- one optional digital protected 12V output
- mounting holes for Ardusimple board(s)
- connector for Bynav-C1 board
- mounting holes for rooftop unit board (for dual-RTK)
- USB hub (4-port)
- USB and power supply for tablet/notebook via one USB-C (external DC adapter needed)
- integrated CANtact hardware (2x CAN2.0)
- virtual serial port for RS485/ModbusRTU
- many LEDs
- Ethernet/UDP option via shield (see folder "ethernet_adapter")

These #[connections](Connections.pdf) are provided:
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
- QWIIC connector
- internal USB connectors
- internal Bynav-C1 connector
- connector to Ethernet shield

Some functions are shown in [my video](https://www.youtube.com/watch?v=BdSSeIprusM). [German version](https://www.youtube.com/watch?v=w10TqOEIxdU).

----------------------------------------------------------------------------------------------------------------------------


# Functional Description (to be continued..)

The Arduino part including roll sensor, ADC and digital inputs is a copy of what BrianTee sugested. Many thanks for his great work!

An ESP32 MINI module is mountable alternatively. In that case, the Arduino firmware has to be adapted and the Nano has to send to reset mode by using a jumper (pins 5+6 of P1).   With an ESP32, one CAN2.0 and the RS485 port is usable e. g. for external relay modules via ModbusRTU. Wifi and Bluetooth may also be used.

The motor driver is the one of the IBT-2. It comes with separated 2-channel power lines at the power plug, so that a double-pole switch with positively opening contacts may be used for safe switch-off. This is especially mandatory for hydraulic steering. Furthermore, a DCDC converter may be used to generate 24V for driving the common Phidgets-Motor with increased dynamic.

The fifth pin of the power plug may either be used for a protected digital output (jumper on 1-2 of J8) or as on-switching signal (jumper on 2-3 of J8). No high currents have to be switched – a small SPST switch is ok. A switch inside the housing would be connected to 3-4 of J8. If an external switch is used to cut of the supply directly, apply a jumper to 2-3 of J8.

All digital inputs are protected and made of constant current sinks. Therefore, the input is inverted, so that either NC pushbuttons may be used or the Arduino firmware may be modified.

The hardware of the ![CANtact project](https://cantact.io/cantact-pro/users-guide.html) based on the Nexperia µC LPC54616 is optionally mountable and may provide two CAN2.0 ports under control of the tablet. One the two ports may alternativly driven by the ESP32.

The RS485 port is like what is also available with a cheap USB-RS485 stick based on a CH340 driver. 

The 4 port USB hub connects the tablet/Notebook to the RTK-GNSS receiver and to the Nano or ESP32, the CANtact hardware, the RS-485 port or the internal 2 USB-A socket or the external USB-C socket. These alternatives are possible:

- USB1:  Arduino Nano Clone  (XOR)   ESP32 (if U3 not mounted.)

- USB2:    RS485      (XOR)     Internal USB-A (if U5 not mounted) (XOR)     External USB-C (if U5 not mounted)

- USB3:    CANtact   (XOR)      Internal USB-A (e. g. for ESP32 parallel to Nano)

- USB4:    External RTK receiver via M12-D (if U19 not mounted) (XOR) Bynav-C1 board (XOR) internal USB:

Option1: USB via M12-D: U19, R100, R101 not mounted
Option2: I²C via M12-D, serial device for RS232 - converter or Bynav RTK receiver @ U16: R43, R46 not mounted
Option3: I²C via M12-D, additional internal USB (USB1): R43, R46, U19 not mounted

All voltage and I/O functions are equipped with LEDs.

Tablets with USB-C plug may be connected with a single USB-C wire for power and data, when a 12V USB-C DC converter is connected to the 2nd USB plug. Both USB-C plugs are mounted on a piggyback PCB on top of the M8 plugs.

The PCB project was set up with the EDA program „EasyEDA“, which is freely usable. The Altium export has not been checked. 

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

# (German copy) Zusammenfassung

Dieser Leiterplattenentwurf integriert mit Ausnahme des RTK-GNSS-Empfängers alle elektronischen Funktionen, die für einen automatische Lenkung der Projekte AgOpenGPS und QtOpenGuidance notwendig sind. So ist es möglich, auch die zweikanalige RTK-Einheit von Matthias (MTZ8302) zu verwenden. Ziel war es nicht, neue Hardwarefunktionen bereitzustellen, sondern eine robuste, monolitische, von professionellen Bestückern herstellbaren Einheit zu entwicklern, auf der der aktuelle Stand der Arduino-Firmware von BrianTee kann unverändert verwendet werden. 

Besonderer Wert wurde auf ein robustes Gehäuse und insbesondere robuste Steckverbinder und Kabel gelegt. Es sind M12/M8-Steckverbinder gerade oder abgewinkelt vorgesehen. Alternativ sind über auch steckbare PushIn-Steckverbinder bestückbar.

Folgende Funktionen sind vorgesehen:
- integrierte Arduino-Nano-Hardware
- Option für ESP32-Leiterplatte (Arduino-Firmware muss angepasst werden)
- ADC (ADS1115) für Lenkwinkelsensor
- Neigungssensor (MMA8452)
- Motortreiber (H-Brücke wie IBT-2) mit zweikanaliger Abschaltung
- geschützte Digitaleingänge (als Konstantstromsenke)
- USB-Hub (4-port)
- Spannungsversorgung auch für Tablet/Notebook via USB-C
- integrierte CANtact-Hardware (2x CAN2.0)
- virtueller serieller Port für RS485/ModbusRTU
- ganz viele Leuchtdioden

Folgende Anschlüsse sind vorgesehen:
- Stromversorgung 12V/24V (optional für Motor)
- USB-C zu Tablet/Notebook
- Lenkwinkelsensor
- Digitaleingänge
- USB via M12-D - Kabel zu RTK-GNSS-Einheit
- Lenkradmotor bzw. Hydraulikeinheit
- 2x CAN2.0
- RS485/ModbusRTU
- Inkrementalgeber für Lenkradsensor
- 2x USB-C (Tablet + DC-Wandler/Aux)

# Im Detail (wird fortlaufend ergänzt)

Der Arduino-Nano-Teil einschließlich der Neigungssensoren, des ADC und der Eingänge entspricht dem von BrianTee entwickelten Schaltplan. An dieser Stelle sei ihm für seine hervorragende Arbeit gedankt!

Eine ESP32-Leiterplatte ist alternativ bestückbar. In diesem Fall muss die Arduino-Software angepasst werden und der Nano dauerhaft im Reset-Zustand gehalten werden (Jumper auf 5, 6 von P1). Mit einem ESP32 steht dann hardwaremäßig auch ein CAN2.0 und ein RS485-Port zur Verfügung sowie Wifi und Bluetooth.

Der Motortreiber entspricht dem IBT-2. Er verfügt über eine getrennte, zweikanalig ausgeführte Stromversorgung am Spannungsversorgungsstecker, sodass hier ein geeigneter zweikanaliger zwangsöffnender Sicherheitsschalter angeschlossen werden kann. Insbesondere bei Hydraulikeinbindung ist dies zwingend. Auch kann so ein DC/DC-Wandler 12V > 24V vorgeschaltet werden, um eine höhere Lenkdynamik z. B. für den beliebten Phidgets-Motor zu erreichen.

Die 5. Leitung des Spannungsversorgungssteckers kann entweder mit einem geschützten Digitalausgang belegt werden (Jumper auf 1-2 von J8) oder für den Ein-Schalter verwendet werden (Jumper auf 2-3 von J8). Es wird keine Leistung geschaltet – ein kleiner Schalter reicht. Ein Schalter im Gehäuse wird an die 3-4 von J8 angeschlossen. Wird ein externer Leistungsschalter verwendet, müssen diese Kontakte mit einem Jumper überbrückt werden.

Die Digitaleingänge sind als Konstantstromsenke ausgeführt. Dadurch ist die Logik allerdings invertiert, also bitte Öffnerkontakte verwenden oder die Arduino-Firmware entsprechend anpassen.

Die Hardware des [CANtact-Projektes](https://cantact.io/cantact-pro/users-guide.html) auf Basis des Nexperia-µC LPC54616 ist optional bestückbar und stellt dann 2 CAN2.0-Schnittstellen zur Verfügung. Eine der beiden CAN2.0-Schnittstellen kann alternativ auch mit dem ESP32 angesteuert werden.

Die RS-485-Schnittstelle entspricht dem, was ein preiswerter USB-RS485-Stick auf Basis einen CH340 leistet. 

Der 4-Port-USB-Hub verbindet das Tablet/Notebook zum RTK-GNSS-Empfänger und wahlweise zum Nano bzw. ESP32, der CANtact-Hardware, der RS-485-Schnittstelle bzw. internen 2 USB-A-Buchsen einer externen USB-C-Buchse. Folgende Alternativen sind mit den 4 Ports möglich:

USB1:    Arduino Nano   (XOR)   ESP32 (wenn U3 nicht bestückt)

USB2:    RS485       (XOR)      Interne USB-A (wenn U5 nicht bestückt)   (XOR) Externe USB-C (wenn U5 nicht bestückt)

USB3:    CANtact  (XOR)         Interne USB-A (z. B. für ESP32 plus Nano)

USB4:    RTK-Empfänger

Alle Spannungen und I/O-Funktionen verfügen über Leuchtdioden.

Tablets mit USB-C-Schnittstelle können mit einem einzigen Kabel für Daten und Stromversorgung angeschlossen werden, wenn ein 12V-USB-C-Ladeadapter an die zweite USB-C-Buchse angeschlossen wird. Die beiden USB-C-Buchsen sind auf Huckepack-Leiterplatten über den M8-Steckverbindern angeordnet.

Das Leiterplattenprojekt wurde im frei verfügbaren EDA-Programm „EasyEDA“ erstellt. Der Export nach Altium ist ungeprüft. 

# Component Sources (examples):

M12/M8:	    [digikey](https://www.digikey.com), [Reichelt](https://www.reichelt.de/de/en/sensor-actor-connectors-c7505.html?MANUFACTURER=CONEC&START=0&OFFSET=16&LANGUAGE=EN&&r=1)

PushIn:		  Weidmueller [S2C-SMT 3.50/04/180G 3.5SN BK BX](https://catalog.weidmueller.com/catalog/Start.do?localeId=en&ObjectID=1290030000) + [B2CF 3.50/04/180 SN OR BX](https://catalog.weidmueller.com/catalog/Start.do?localeId=en&ObjectID=1277270000) via [Conrad](https://www.conrad.com)/[digikey](https://www.digikey.com)

ESP32 MINI:	[Aliexpress](https://de.aliexpress.com/wholesale?catId=0&SearchText=esp32+mini)

Housing: [Aliexpress](https://de.aliexpress.com/item/32967598546.html)

RTK receiver: [Bynav](https://www.bynav.com/en/products/gnss-boards/c1.html)

RTK receiver: [GNSS store](https://www.gnss.store/gnss-gps-modules/105-ublox-zed-f9p-rtk-gnss-receiver-board-with-sma-base-or-rover.html)

