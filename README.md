# Central Autosteer Unit and Rooftop Unit for AgOpenGPS/QtOpenGuidance  
(Informationen in deutscher Sprache unten)

# Minimal system (description not up to date... see Central-Unit descriptions..
![pic](central_unit/documentation/Minimal_System.png?raw=true)
- Tablet/Laptop with Windows
- control electronic (e. g. this [central unit](central_unit))
- Ardusimple RTK reciever with antenna (or equivalent)
- Motor
- Mechanical parts and cables

These two units bring all electronic functions for autosteering with the ![AgOpenGPS](https://github.com/farmerbriantee/AgOpenGPS) / ![QtOpenGuidance](https://github.com/eringerli/QtOpenGuidance) project. 

The main unit is contains all functions for autosteering and also includes an 4-port USB hub.

The dual RTK-GNSS receiver, based on an idea of ![MTZ8302](https://github.com/mtz8302/AOG_GPS_ESP32), is a separate unit. This unit is not necessary; the Ardusimple module(s)/Bynav-module may also be mounted inside the central unit.

![pic](Overview.png?raw=true)

This setup does not include new functions but provides a robust, monolitic unit  that may be manufactured by professionel EMS providers. The central unit is compatible to [BrianTee‘s Nano firmware](https://github.com/farmerbriantee/AgOpenGPS/blob/master/Support_Files.zip) and the rooftop unit to [MTZ8302's dual-RTK firmware](https://github.com/mtz8302/AOG_GPS_ESP32). If the ESP32 option is used for autosteering or RTK, the pin definitions of [esp32-aog](https://github.com/eringerli/esp32-aog) and [AOG_Autosteer_ESP32](https://github.com/mtz8302/AOG_Autosteer_ESP32) or [MTZ8302's dual-RTK firmware](https://github.com/mtz8302/AOG_GPS_ESP32) must be adapted respectively.

Special focus is given to the robust housing and industrial M12/M8 connetors (optional). PushIn connectors may also be used for single wire connections).

The PCB project was set up with the EDA program „EasyEDA“, which is freely usable. The Altium export has not been checked. 

[Documentation Central Unit](central_unit/README.md)

[Documentation Rooftop Unit](rooftop_unit/README.md)

[Documentation Ethernet Adapter for UDP](ethernet_adapter/README.md)

# Order How-To JLCPCB
- browse to [JLCPCB](https://jlcpcb.com) and click "Quote now"
- upload Gerber file (either for [Centrol unit](central_unit/production_data/Gerber_Central_Unit_AgOpenGPS.zip) or [Rooftop Unit](rooftop_unit/production_data/Gerber_Rooftop_Unit_AgOpenGPS.zip)). The Gerber of the central unit is too big for a preview, so you also have to enter the DIMENSIONS in the next line after the upload process has fully finished: 150 x 92mm²
- enter the number of PCBs to order: 5, 10, 15, etc
- if you like, you may change the color of the solder mask printing. But not all colors are available for SMD manufacturing!
- for the central unit check switch "SMT assembly" and "Assemble top layer"
- Accept terms and click "NEXT" (you may log in now)
- Upload [BOM](central_unit/production_data/BOM_Central%20Unit%20for%20AgOpenGPS.csv) and [Pick and Place file](central_unit/production_data/PickAndPlace_Central_Unit_AgOpenGPS.csv)
- click NEXT
- browse through the list and see, what is not confirmed. For some you may find 2nd sources, some maybe not available from stock in that moment, some like the Nexperia controller are unavailable and some like connectors are not supported yet (see my [video](https://www.youtube.com/watch?v=dLczChhmDCY) how to solder the USB-C receptacle; soldering the USB-C receptacle like this is only necessary, if you want to use a single USB cable for charging and data. USB itself is also connectable via a dedicated PushIn connector)
- click NEXT
- check if all components are placed correctly (e. g. rotation information is sometimes incorrect in the libraries)
- order

# (German copy) Zusammenfassung

Für ein Autosteering-System benötigt man mindestens
- einen Laptop oder Tablet/Convertable mit Windows + Ladegerät für 12V
- einen RTK-Empfänger mit Antenne, bevorzugt auf ublox-Basis (z. B. [Ardusimple](https://www.ardusimple.com/product/simplertk2b-basic-starter-kit-ip65/))
- einen [Lenkradmotor](https://agopengps.discourse.group/t/dc-motor-model/784)
- eine Ansteuerelektronik wie die hier beschriebene Zentraleinheit

Dieser Leiterplattenentwurf integriert mit Ausnahme des RTK-GNSS-Empfängers alle elektronischen Funktionen, die für einen automatische Lenkung der Projekte AgOpenGPS und QtOpenGuidance notwendig sind. So ist es möglich, auch die zweikanalige RTK-Einheit von Matthias (MTZ8302) zu verwenden. Ziel war es nicht, neue Hardwarefunktionen bereitzustellen, sondern eine robuste, monolitische, von professionellen Bestückern herstellbaren Einheit zu entwicklern, auf der der aktuelle Stand der Arduino-Firmware von BrianTee kann unverändert verwendet werden. 

Besonderer Wert wurde auf ein robustes Gehäuse und insbesondere robuste Steckverbinder und Kabel gelegt. Es sind M12/M8-Steckverbinder gerade oder abgewinkelt vorgesehen. Alternativ sind über auch steckbare PushIn-Steckverbinder bestückbar.

Das Leiterplattenprojekt wurde im frei verfügbaren EDA-Programm „EasyEDA“ erstellt. Der Export nach Altium ist ungeprüft. 

[Dokumentation Zentraleinheit](central_unit/README.md)

[Dokumentation Dacheinheit (optional)](rooftop_unit/README.md)

# Bestellhinweise
1. SMD-bestückte Leiterplatte bestellen, z. B. beim JLCPCB für ca. 35€ wie oben im englischen Teil beschrieben. Deutschsprachige Alternativen: PCB-Pool, Aisler. Vorsicht bei JLCPCB: Es werden nur die Bauteile bestückt, die zum Bestellzeitpunkt verfügbar sind (wird aber deutlich angezeigt - Liste kann auch heruntergeladen werden).
2. Je nach gewünschter Verkabelungsvariante entweder M8 + M12-Steckverbinder und Kabel bestellen oder PushIn-Buchsen und Stecker und ein passendes Gehäuse
3. Gewünschte Zusatzmodule bestellen, z. B. Lagesensor CMPS14 als (verbesserte) Alternative zum Onboard-Lagesensor MMA8452, zweiter RTK-Empfänger, ESP32 für CAN oder Dual-RTK, hier ebenfalls beschriebene Dacheinheit, RS232-Schnittstellenwandler, etc
4. Kleinkram wie Schrauben, (USB-) Kabel, Kunststoffabstandshülsen etc bestellen

Selbst gelötet werden die Steckverbinder wie unter 2. beschrieben (einfache zu löten). 

Weitere Hinweise finden sich zum Teil bei den einzelnen Baugruppen.
