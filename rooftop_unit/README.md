# Autosteer and RTK units for AgOpenGPS/QtOpenGuidance  
Status: Draft - to be tested!

The dual RTK-GNSS receiver, based on an idea of ![MTZ8302](https://github.com/mtz8302/AOG_GPS_ESP32).

This setup does not include new functions but provides a robust, monolitic unit  that may be manufactured by professionel EMS providers. The rooftop unit is similar to [MTZ8302's dual-RTK firmware](https://github.com/mtz8302/AOG_GPS_ESP32). 

Special focus is given to the robust housing and industrial M12/M8 connetors (optional). 

![pic](3D.png?raw=true)

[Project link](https://easyeda.com/GoRoNb/autosteer-for-agopengps_copy_copy_copy_copy)
# Functions roof top unit for positioning (only THT):
- Dual RTK-GNSS receiver
- BNO085 option (as CMPS14 board)
- QWIIC connector
- based on ESP32 [Thx to MTZ8302](https://github.com/mtz8302/AOG_GPS_ESP32) and ![Jeep1945](https://github.com/Jeep1945/Dualheading) (pinning taken from Jeep1945)
- RS232 output option for sprayer (NMEA string output or speed pulse)
- SPI connector for those who like to use an Ethernet shield
- 3 LEDs, 2 key inputs
- option for W5500 Ethernet board for UDP

The PCB project was set up with the EDA program „EasyEDA“, which is freely usable. The Altium export has not been checked. 

# (German copy) Zusammenfassung

Dieser Leiterplattenentwurf integriert mit Ausnahme des RTK-GNSS-Empfängers alle elektronischen Funktionen, die für einen automatische Lenkung der Projekte AgOpenGPS und QtOpenGuidance notwendig sind. So ist es möglich, auch die zweikanalige RTK-Einheit von Matthias (MTZ8302) zu verwenden. Ziel war es nicht, neue Hardwarefunktionen bereitzustellen, sondern eine robuste, monolitische, von professionellen Bestückern herstellbaren Einheit zu entwicklern, auf der der aktuelle Stand der Arduino-Firmware von BrianTee kann unverändert verwendet werden. 

Besonderer Wert wurde auf ein robustes Gehäuse und insbesondere robuste Steckverbinder und Kabel gelegt. Es sind M12/M8-Steckverbinder gerade oder abgewinkelt vorgesehen. Alternativ sind über auch steckbare PushIn-Steckverbinder bestückbar.

Das Leiterplattenprojekt wurde im frei verfügbaren EDA-Programm „EasyEDA“ erstellt. Der Export nach Altium ist ungeprüft. 

# Component Sources (examples):

M12/M8:	    [digikey](https://www.digikey.com), [Reichelt](https://www.reichelt.de/de/en/sensor-actor-connectors-c7505.html?MANUFACTURER=CONEC&START=0&OFFSET=16&LANGUAGE=EN&&r=1)

ESP32 MINI:	[Aliexpress](https://de.aliexpress.com/wholesale?catId=0&SearchText=esp32+mini)

Housing Rooftop Unit: [Amazon](http://www.amazon.de/dp/B076KJZJN5/)

RTK receiver: [GNSS store](https://www.gnss.store/gnss-gps-modules/105-ublox-zed-f9p-rtk-gnss-receiver-board-with-sma-base-or-rover.html)

Ethernet: [W5500 board](https://de.aliexpress.com/item/32598945210.html)

