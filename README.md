# Central Autosteer Unit and Rooftop Unit for AgOpenGPS/QtOpenGuidance  
Status: Draft - to be tested!

These two units bring all electronic functions for autosteering with the ![AgOpenGPS](https://github.com/farmerbriantee/AgOpenGPS) / ![QtOpenGuidance](https://github.com/eringerli/QtOpenGuidance) project. The dual RTK-GNSS receiver, based on an idea of ![MTZ8302](https://github.com/mtz8302/AOG_GPS_ESP32), is a separate unit.

![pic](Overview.png?raw=true)

This setup does not include new functions but provides a robust, monolitic unit  that may be manufactured by professionel EMS providers. The central unit is compatible to [BrianTee‘s Nano firmware](https://github.com/farmerbriantee/AgOpenGPS/blob/master/Support_Files.zip) and the rooftop unit to [MTZ8302's dual-RTK firmware](https://github.com/mtz8302/AOG_GPS_ESP32). If the ESP32 option is used for the autosteer unit, the pin definitions of [esp32-aog](https://github.com/eringerli/esp32-aog) and [AOG_Autosteer_ESP32](https://github.com/mtz8302/AOG_Autosteer_ESP32) must be adapted.

Special focus is given to the robust housing and industrial M12/M8 connetors (optional). PushIn connectors may also be used for single wire connections).

The PCB project was set up with the EDA program „EasyEDA“, which is freely usable. The Altium export has not been checked. 

[Documentation Central Unit](central_unit/README.md)
[Documentation Central Unit](rooftop_unit/README.md)

# (German copy) Zusammenfassung

Dieser Leiterplattenentwurf integriert mit Ausnahme des RTK-GNSS-Empfängers alle elektronischen Funktionen, die für einen automatische Lenkung der Projekte AgOpenGPS und QtOpenGuidance notwendig sind. So ist es möglich, auch die zweikanalige RTK-Einheit von Matthias (MTZ8302) zu verwenden. Ziel war es nicht, neue Hardwarefunktionen bereitzustellen, sondern eine robuste, monolitische, von professionellen Bestückern herstellbaren Einheit zu entwicklern, auf der der aktuelle Stand der Arduino-Firmware von BrianTee kann unverändert verwendet werden. 

Besonderer Wert wurde auf ein robustes Gehäuse und insbesondere robuste Steckverbinder und Kabel gelegt. Es sind M12/M8-Steckverbinder gerade oder abgewinkelt vorgesehen. Alternativ sind über auch steckbare PushIn-Steckverbinder bestückbar.

Das Leiterplattenprojekt wurde im frei verfügbaren EDA-Programm „EasyEDA“ erstellt. Der Export nach Altium ist ungeprüft. 

[Dokumentation Central Unit](central_unit/README.md)
[Dokumentation Central Unit](rooftop_unit/README.md)

