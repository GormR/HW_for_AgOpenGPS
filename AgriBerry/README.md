![pic](AgriBerry_full.png?raw=true)

# AgriBerry  
(Informationen in deutscher Sprache unten)

Pilot implementation of hardware for automatic steering of agricultural vehicles - Software missing yet!

Why copyright?
This project and its logo shall remain a free and public project. Although intended people selling professional service, the hard- and software shall always be free for the sake of everybody's information.

# Hardware features:
- analog and digital inputs
- motor/valve driver incl. aux. for 8/2
- Pi power supply from 14V nom.
- RS485(Modbus) e. g.for relay modules (section control, etc.)
- 2 CAN
- 4 GPDI + 2 GPDO
- Ardusimple board as piggyback
- made for HifiBerry plastic housing

The AgriBerry bases on BrianTee's great and altruistic [AgOpenGPS](https://github.com/farmerbriantee/AgOpenGPS) and transforms this onto a Raspberry Pi 4 base.

The user should benefit from the ease of simplicity:
- each component is available preassembled w/o the need of any SMD soldering
- software setup is as simple as putting an image to an SD card

It is designed as a robust, monolitic unit  that may be manufactured by professionel EMS providers. 

![pic](3D.png?raw=true)

# 59.5 x 91 mm² 
2-layer 35µm, 1,6mm thickness, technology: 0.15mm/0.15mm/0.3mm; SMD assembly: about 70 different parts, 180 parts overall, THT: only 2.54mm connectors

[Link to EasyEDA project](https://oshwlab.com/GoRoNb/AgriNode). Estimated price of a single PCB assembly @ JLC: $35 

# Software architecture (suggestion)

![pic](AgriBerry_software_structure.jpeg?raw=true)

# Initial Operation Instructions
1. if applicable: [assemble missing SMD parts](https://www.youtube.com/watch?v=dLczChhmDCY)
2. assemble all THT parts: pin headers and connectors
3. connect 3-4 @ J8 (on)
4. slowly increase input voltage on CN4/POWER with current limit of 100mA while checking the 5V voltage (should be 0mA till about 4V, 20mA @ 4.5V, 12mA @ 12V; +5V must not rise up to more than 5.1V!)
5. connect Raspberry PI 4 
8. insert micro-SD card with software image
9. connect WAS, buttons, motor and do a test 

There is also an [English user group](t.me/agopengpsinternational) and a [multilingual forum](https://agopengps.discourse.group/t/agriberry-hardware-draft/4357). 

----------------------------------------------------------------------------------------------------------------------------

# Functional Description (to be continued..)

The hardware including roll sensor, ADC and digital inputs is a following BrianTee's suggestion for AgOpenGPS. Many thanks for his great work!

The motor driver is the one of the IBT-2, but with genuine and automotive quality parts. It comes with separated 2-channel power lines at the power plug, so that a double-pole switch with positively opening contacts may be used for safe switch-off. This is especially mandatory for hydraulic steering. Furthermore, a DCDC converter may be used to generate 24V for driving the common Phidgets-Motor with increased dynamic. An auxilliary output for a 8/2 valve is provided.

All digital inputs are protected and both digital and analog readable.

All voltage and I/O functions are equipped with LEDs.

The PCB project was set up with the EDA program „EasyEDA“, which is freely usable. It is designed to fit into the HifiBerry plastic housing.

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

Dieser Leiterplattenentwurf integriert mit Ausnahme des RTK-GNSS-Empfängers alle elektronischen Funktionen, die für einen automatische Lenkung mit einem Raspberry PI 4 notwendig sind. Er orientiert sich an den Projekten AgOpenGPS und QtOpenGuidance. Ein Ardusimple kann aufgesteckt werden, aber es ist auch möglich, die zweikanalige RTK-Einheit von Matthias (MTZ8302) als externe Einheit zu verwenden. 

Besonderer Wert wurde ein industriell automatisch fertigbares kompaktes Design mit industriellen steckbaren PushIn-Steckverbindern gelegt.

# Hardwarefeatures:
- Ardusimple aufsteckbar
- analoge and digitale Eingänge
- Motor/Vetiltreiber incl. Hilfausgang für 8/2
- Pi Stromversorgung aus 14V nom.
- RS485(Modbus) z. B. für Relaismodule (section control, etc.)
- 2 CAN
- 4 GPDI + 2 GPDO
- Formfaktor for HifiBerry Kunststoffgehäuse

Das Leiterplattenprojekt wurde im frei verfügbaren EDA-Programm „EasyEDA“ erstellt.

Hilfestellung gibt es auch in der  [deutschsprachigen Nutzergruppe](t.me/AGOpenGPSGerman) und im [mehrsprachigen Forum](https://agopengps.discourse.group/t/agriberry-hardware-draft/4357). 

# Erstinbetriebnahme
1. Ggf. fehlende SMD-Bauteile [nachbestücken](https://www.youtube.com/watch?v=dLczChhmDCY)
2. gewünschte Stiftleisten und Stecker auflöten
3. Pins 3-4 @ J8 brücken (on)
4. langsam die Spannung an CN4/POWER mit einer Strombegrenzung von 100mA hochregeln und gleichzeit die 5V-Spannung im Auge behalten (Eingangsstrom: 0mA bis ca. 4V, 20mA @ 4.5V, 12mA @ 12V; +5V darf nicht über 5,1V ansteigen!)
5. Raspberry PI 4 anschliessen
6. Micro-SD-Karte mit Softwareimage einstecken
7. Lenkwinkelsensor (auch als WAS oder LWS bezeichnet), Schalter, Motor anschliessen und testen

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

PushIn:		  Weidmueller [S2C-SMT 3.50/24/90LF 1.5SN BK BX](https://catalog.weidmueller.com/catalog/Start.do?localeId=en&ObjectID=1289960000) + [B2CF 3.50/24/180LH SN BK BX](https://catalog.weidmueller.com/catalog/Start.do?localeId=en&ObjectID=1372870000) via [Conrad](https://www.conrad.com)/[digikey](https://www.digikey.com)

Housing: [HifiBerry](https://www.hifiberry.com/shop/cases/hifiberry-universal-case-black-pi4/)

RTK receiver: 
- [Ardusimple](https://www.ardusimple.com/product/simplertk2b-basic-starter-kit-ip65/) (state-of-the-art receiver - mounting holes provided / Standardempfänger - Montagelöcher hierfür sind vorgesehen)

