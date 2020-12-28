# Autosteer and RTK units for AgOpenGPS/QtOpenGuidance 
Status: Draft - to be tested!

These two PCBAs bring all electronic functions for autosteering with the AgOpenGPS / QtOpenGuidance project. The dual RTK-GNSS receiver is a separate unit.

This setup does not include new functions but provides a robust, monolitic unit  that may be manufactured by professionel EMS providers. The central unit is compatible to BrianTee‘s Nano firmware and the roof top unit to Matthias (MTZ8302) dual-RTK firmware. 

Special focus is given to the robust housing and industrial M12/M8 connetors (optional). PushIn connectors may also be used for single wire connections).

![pic](central_unit/3D.png?raw=true)
# Functions in-cabin-unit for autosteering
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

![pic](roof_top_unit/3D.png?raw=true)
# Functions roof top unit for positioning:
- Dual RTK-GNSS receiver
- BNO085 option
- based on ESP32 ![Thx to MTZ8302!](https://github.com/mtz8302/AOG_GPS_ESP32)

# Functional Description (to be continued..)

The Arduino part including roll sensor, ADC and digital inputs is a copy of what BrianTee sugested. Many thanks for his great work!

An ESP32 MINI module is mountable alternatively. In that case, the Arduino firmware has to be adapted and the Nano has to send to reset mode by using a jumper (pins 5+6 of P1).   With an ESP32, one CAN2.0 and the RS485 port is usable e. g. for external relay modules via ModbusRTU. Wifi and Bluetooth may also be used.

The motor driver is the one of the IBT-2. It comes with separated 2-channel power lines at the power plug, so that a double-pole switch with positively opening contacts may be used for safe switch-off. This is especially mandatory for hydraulic steering. Furthermore, a DCDC converter may be used to generate 24V for driving the common Phidgets-Motor with increased dynamic.

The fifth pin of the power plug may either be used for a protected digital output (jumper on 1-2 of J8) or as on-switching signal (jumper on 2-3 of J8). No high currents have to be switched – a small SPST switch is ok. A switch inside the housing would be connected to 3-4 of J8. If an external switch is used to cut of the supply directly, apply a jumper to 2-3 of J8.

All digital inputs are protected and made of constant current sinks. Therefore, the input is inverted, so that either NC pushbuttons may be used or the Arduino firmware may be modified.

The hardware of the CANtact project based on the Nexperia µC LPC54616 is optionally mountable and may provide two CAN2.0 ports under control of the tablet. For details please visit https://cantact.io/cantact-pro/users-guide.html. One the two ports may alternativly driven by the ESP32.

The RS485 port is like what is also available with a cheap USB-RS485 stick based on a CH340 driver. 

The 4 port USB hub connects the tablet/Notebook to the RTK-GNSS receiver and to the Nano or ESP32, the CANtact hardware, the RS-485 port or the internal 2 USB-A socket or the external USB-C socket. These alternatives are possible:

USB1    Arduino Nano    ESP32 (if U3 not mounted.)
USB2    RS485           Internal USB-A (if U5 not mounted)      External USB-C (if U5 not mounted)
USB3    CANtact         Internal USB-A (e. g. for ESP32 parallel to Nano)
USB4    External RTK receiver via M12-D

All voltage and I/O functions are equipped with LEDs.

Tablets with USB-C plug may be connected with a single USB-C wire for power and data, when a 12V USB-C DC converter is connected to the 2nd USB plug. Both USB-C plugs are mounted on a piggyback PCB on top of the M8 plugs.

The PCB project was set up with the EDA program „EasyEDA“, which is freely usable. The Altium export has not been checked. 

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

Die Hardware des CANtact-Projektes auf Basis des Nexperia-µC LPC54616 ist optional bestückbar und stellt dann 2 CAN2.0-Schnittstellen zur Verfügung. Details siehe https://cantact.io/cantact-pro/users-guide.html. Eine der beiden CAN2.0-Schnittstellen kann alternativ auch mit dem ESP32 angesteuert werden.

Die RS-485-Schnittstelle entspricht dem, was ein preiswerter USB-RS485-Stick auf Basis einen CH340 leistet. 

Der 4-Port-USB-Hub verbindet das Tablet/Notebook zum RTK-GNSS-Empfänger und wahlweise zum Nano bzw. ESP32, der CANtact-Hardware, der RS-485-Schnittstelle bzw. internen 2 USB-A-Buchsen einer externen USB-C-Buchse. Folgende Alternativen sind mit den 4 Ports möglich:

USB1    Arduino Nano      ESP32 (wenn U3 nicht bestückt)
USB2    RS485             Interne USB-A (wenn U5 nicht bestückt)    Externe USB-C (wenn U5 nicht bestückt)
USB3    CANtact           Interne USB-A (z. B. für ESP32 plus Nano)
USB4    RTK-Empfänger

Alle Spannungen und I/O-Funktionen verfügen über Leuchtdioden.

Tablets mit USB-C-Schnittstelle können mit einem einzigen Kabel für Daten und Stromversorgung angeschlossen werden, wenn ein 12V-USB-C-Ladeadapter an die zweite USB-C-Buchse angeschlossen wird. Die beiden USB-C-Buchsen sind auf Huckepack-Leiterplatten über den M8-Steckverbindern angeordnet.

Das Leiterplattenprojekt wurde im frei verfügbaren EDA-Programm „EasyEDA“ erstellt. Der Export nach Altium ist ungeprüft. 

# Component Sources:

M12/M8	    digikey, Reichelt, Aliexpress
PushIn		  Weidmüller (www.weidmueller.com) 1290030000 + 1277270000 via Conrad/digikey
ESP32 MINI	Aliexpress

