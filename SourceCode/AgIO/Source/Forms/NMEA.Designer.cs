﻿using System;
using System.Globalization;

namespace AgIO
{
    public partial class FormLoop
    {
        private string[] rawBuffer = new string [2];
        private string[] words;
        private string nextNMEASentence = "";

        private bool isNMEAToSend = false;
        public bool isDual = false;

        public string[] vtgSentence = new string [2];
        public string[] ggaSentence = new string[2];
        public string hdtSentence, avrSentence, paogiSentence, 
            hpdSentence, rmcSentence, pandaSentence;

        public float altitude = float.MaxValue, headingTrue = float.MaxValue,
            headingTrueDual = float.MaxValue, speed = float.MaxValue, roll = float.MaxValue;
        public float rollData, headingTrueData, headingTrueDualData;
        public float[] altitudeData = new float [2];
        public float[] speedData = new float[2];
        public float[] ageData = new float[2];
        public float[] hdopData = new float[2];

        public double latitudeSend = double.MaxValue, longitudeSend = double.MaxValue, latitude, longitude, GPSroll, GPSheading;

        public double[] myLongitude = new double[2];  // definitions for dual RTK with two F9P
        public double[] myLattitude = new double[2];
        public double[] myAltitude = new double[2];
        public int[] myTime = new int[2];
        public double antennaDist = 0;  // distance between the two antennas

        public ushort satellitesTracked = ushort.MaxValue, hdopX100 = ushort.MaxValue, 
            ageX100 = ushort.MaxValue;
        public ushort[] satellitesData = new ushort[2];

        //imu data
        public ushort imuHeadingData, imuHeading = ushort.MaxValue;
        public short imuRollData, imuRoll = short.MaxValue;

        public byte fixQuality = byte.MaxValue;
        public byte[] fixQualityData = new byte [2];

        private float rollK, Pc, G, Xp, Zp, XeRoll, P = 1.0f;
        private readonly float varRoll = 0.1f, varProcess = 0.0003f;

        double LastUpdateUTC = 0;

        //Convert Fix value to Text
        public string FixQuality
        {
            get
            {
                if (fixQualityData[0] == 0) return "Invalid: ";
                else if (fixQualityData[0] == 1) return "GPS single: ";
                else if (fixQualityData[0] == 2) return "DGPS : ";
                else if (fixQualityData[0] == 3) return "PPS : ";
                else if (fixQualityData[0] == 4) return "RTK fix: ";
                else if (fixQualityData[0] == 5) return "Float: ";
                else if (fixQualityData[0] == 6) return "Estimate: ";
                else if (fixQualityData[0] == 7) return "Man IP: ";
                else if (fixQualityData[0] == 8) return "Sim: ";
                else return "Unknown: ";
            }
        }

        public string FixQuality2
        {
            get
            {
                if (fixQualityData[1] == 0) return "Invalid: ";
                else if (fixQualityData[1] == 1) return "GPS single: ";
                else if (fixQualityData[1] == 2) return "DGPS : ";
                else if (fixQualityData[1] == 3) return "PPS : ";
                else if (fixQualityData[1] == 4) return "RTK fix: ";
                else if (fixQualityData[1] == 5) return "Float: ";
                else if (fixQualityData[1] == 6) return "Estimate: ";
                else if (fixQualityData[1] == 7) return "Man IP: ";
                else if (fixQualityData[1] == 8) return "Sim: ";
                else return "Unknown: ";
            }
        }

        public string Parse(ref string buffer)
        {
            string sentence;
            do
            {
                //double check for valid sentence
                // Find start of next sentence
                int start = buffer.IndexOf("$", StringComparison.Ordinal);
                if (start == -1) return null;
                buffer = buffer.Substring(start);

                // Find end of sentence
                int end = buffer.IndexOf("\r", StringComparison.Ordinal);
                if (end == -1) return null;

                //the NMEA sentence to be parsed
                sentence = buffer.Substring(0, end + 1);

                //remove the processed sentence from the rawBuffer
                buffer = buffer.Substring(end + 1);
            }

            //if sentence has valid checksum, its all good
            while (!ValidateChecksum(sentence));

            // Remove trailing checksum and \r\n and return
            sentence = sentence.Substring(0, sentence.IndexOf("*", StringComparison.Ordinal));

            return sentence;
        }

        public void ParseIMU()
        {
            if (spIMU.IsOpen && isRVC)    // RVC mode on BNO085: do on-the-fly decoding to safe performance
            {
                short yaw = 0, pitch = 0, roll = 0;
                double yawSum = 0, pitchSum = 0, rollSum = 0;
                uint NoOfPackets = 0;
                int bytesNow = spIMU.BytesToRead;          // no. of bytes in buffer
                int bytesGot = 0;
                byte dummy = 0;

                traffic.cntrIMUIn = 0;
                if (bytesNow > 21 * RVCPacketLength)       // too many bytes in buffer => flush it
                    spIMU.DiscardInBuffer();
                else
                {
                    while (bytesNow - bytesGot >= RVCPacketLength - 3)  // while there is at least one packet in buffer, do..
                    {
                        while (spIMU.ReadByte() != 0xaa && bytesNow - bytesGot >= RVCPacketLength - 3)   // look out for 1st 0xaa
                            bytesGot++;
                        if (bytesNow - bytesGot >= RVCPacketLength - 4 && spIMU.ReadByte() == 0xaa)      // look out for 2nd 0xaa
                        {
                            NoOfPackets++;                                                          // ok, let's decode it
                            dummy = (byte)spIMU.ReadByte();   // running counter (we don't care for the moment)
                            yaw = (short)((byte)spIMU.ReadByte() | (byte)spIMU.ReadByte() << 8);   // 1/100 deg
                            yawSum += Convert.ToDouble(yaw);
                            pitch = (short)((byte)spIMU.ReadByte() | (byte)spIMU.ReadByte() << 8); // 1/100 deg
                            pitchSum += Convert.ToDouble(pitch);
                            roll = (short)((byte)spIMU.ReadByte() | (byte)spIMU.ReadByte() << 8);  // 1/100 deg
                            rollSum += Convert.ToDouble(roll);
                            dummy = (byte)spIMU.ReadByte();   // accelaration x low
                            dummy = (byte)spIMU.ReadByte();   // accelaration x high
                            dummy = (byte)spIMU.ReadByte();   // accelaration y low
                            dummy = (byte)spIMU.ReadByte();   // accelaration y high
                            dummy = (byte)spIMU.ReadByte();   // accelaration z low
                            dummy = (byte)spIMU.ReadByte();   // accelaration z high
                            bytesGot += 14;  // we don't care for the rest here
                        }
                    }
                    if (NoOfPackets != 0)
                    {
                        GPSroll = rollSum / (100 * NoOfPackets);
                        GPSheading = 180 + yawSum / (100 * NoOfPackets);
                        traffic.cntrIMUIn = (int)(100 * RVCPacketLength);  // 19 bytes per packet, 100 packets per second
                    }
                }
            }
        }

        public void ParseNMEA(ref string buffer, byte GPS012)  // GPS012: 0, 1 = from different serial COM or via UDP
        {
            if (rawBuffer[GPS012] == null) return;

            //find end of a sentence
            int cr = rawBuffer[GPS012].IndexOf("\r", StringComparison.Ordinal);
            if (cr == -1) return; // No end found, wait for more data

            // Find start of next sentence
            int dollar = rawBuffer[GPS012].IndexOf("$", StringComparison.Ordinal);
            if (dollar == -1) return;

            //if the $ isn't first, get rid of the tail of corrupt sentence
            if (dollar >= cr) rawBuffer[GPS012] = rawBuffer[GPS012].Substring(dollar);

            cr = rawBuffer[GPS012].IndexOf("\r", StringComparison.Ordinal);
            if (cr == -1) return; // No end found, wait for more data

            if (rawBuffer[GPS012].Length > 250)
            {
                if (isLogNMEA)
                {
                    logNMEASentence.Append("\r\n" + 
                        DateTime.UtcNow.ToString(" ->>  mm:ss.fff ", CultureInfo.InvariantCulture) + "\r\n" + rawBuffer[GPS012] + "\r\n");
                }

                rawBuffer[GPS012] = "";
                return;
            }

            //now we have a complete sentence or more somewhere in the portData
            while (true)
            {
                //extract the next NMEA single sentence
                nextNMEASentence = Parse(ref buffer);
                if (nextNMEASentence == null) break;

                words = nextNMEASentence.Split(',');

                if (isLogNMEA)
                {
                    string timNow = DateTime.UtcNow.ToString("HHmmss.fff ", CultureInfo.InvariantCulture);
                    logNMEASentence.Append(timNow + " " + nextNMEASentence + "\r\n");

                    //double timD = Convert.ToDouble(timNow);
                    //double timS = Convert.ToDouble(words[1]);

                    //logNMEASentence.Append((timD-timS).ToString("N3", CultureInfo.InvariantCulture) + " ")
                    //    .Append(timNow + " " + nextNMEASentence + "\r\n");

                }

                //parse them accordingly
                words = nextNMEASentence.Split(',');
                if (words.Length < 3) break;

                if ((words[0] == "$GPGGA" || words[0] == "$GNGGA") && words.Length > 13)
                {
                    ParseGGA(GPS012);
                    if (isGPSSentencesOn) ggaSentence[GPS012] = nextNMEASentence;

                    if (spGPS.IsOpen && spGPS2.IsOpen || isDual)   // dual serial F9P: calculate roll and heading
                    {
                        if ((myTime[GPS012] - myTime[GPS012 ^ 1]) < 20)  
                            // true if value decoded right now is less than 10ms newer than the one on the other COM => calulate heading
                        {
                            latitude = (myLattitude[GPS012] + myLattitude[GPS012 ^ 1]) / 2;     // middle of the two antennas
                            longitude = (myLongitude[GPS012] + myLongitude[GPS012 ^ 1]) / 2;
                            latitude += 1.237463278;        // camouflage my location for videos
                            longitude += 1.39485723948;
                            double distanceDeg = 40080000 / 360;  // circumference of the earth in [m] / 360°
                            antennaDist = Math.Sqrt(Math.Pow((myLattitude[0] - myLattitude[1]), 2) + 
                                                    Math.Pow(((myLongitude[0] - myLongitude[1]) * Math.Cos(myLattitude[0] * Math.PI / 180)), 2)) * distanceDeg; 
                                                    // Pythagoras
                            GPSheading = 180 - Math.Atan2((myLattitude[0] - myLattitude[1]) * Math.PI / 180, 
                                                          (myLongitude[0] - myLongitude[1]) * Math.PI / 180 * Math.Cos(myLattitude[0] * Math.PI / 180)) * 180 / Math.PI;
                            GPSroll = Math.Atan((myAltitude[0] - myAltitude[1]) / antennaDist) * 180 / Math.PI;
                            //GPSheading = (myLattitude[0] - myLattitude[1]) * 10000000;  // debug outputs
                            //GPSroll = (myLongitude[0] - myLongitude[1])  * Math.Cos(myLattitude[0] * Math.PI / 180) * 10000000;
                            //antennaDist = myAltitude[0] - myAltitude[1];
                            ParseIMU();  
                        }
                    }
                    else  // only one F9P => copy directly
                    {
                        latitude = myLattitude[GPS012];
                        longitude = myLongitude[GPS012];
                        ParseIMU();  
                    }
                }

                else if ((words[0] == "$GPVTG" || words[0] == "$GNVTG") && words.Length > 7)
                {
                    ParseVTG(GPS012);
                    if (isGPSSentencesOn) vtgSentence[GPS012] = nextNMEASentence;
                }

                //else if (words[0] == "$GPRMC" || words[0] == "$GNRMC")
                //{
                //    ParseRMC();
                //    if (isGPSSentencesOn) rmcSentence = nextNMEASentence;
                //}

                else if (words[0] == "$KSXT")
                {
                    ParseKSXT();
                    if (isGPSSentencesOn) ksxtSentence = nextNMEASentence;
                }

                else if (words[0] == "$GPHPD")
                {
                    ParseHPD(GPS012);
                    if (isGPSSentencesOn) hpdSentence = nextNMEASentence;
                }

                else if (words[0] == "$PAOGI" && words.Length > 14)
                {
                    ParseOGI(GPS012);
                    if (isGPSSentencesOn) paogiSentence = nextNMEASentence;
                }

                else if (words[0] == "$PANDA" && words.Length > 14)
                {
                    ParsePANDA(GPS012);
                    if (isGPSSentencesOn) pandaSentence = nextNMEASentence;
                }

                else if (words[0] == "$GPHDT" || words[0] == "$GNHDT")
                {
                    ParseHDT(GPS012);
                    if (isGPSSentencesOn) hdtSentence = nextNMEASentence;
                }

                else if (words[0] == "$PTNL" && words.Length > 8)
                {
                    ParseAVR(GPS012);
                    if (isGPSSentencesOn) avrSentence = nextNMEASentence;
                }

                else if (words[0] == "$GNTRA")
                {
                    ParseTRA(GPS012);
                }

                else if (words[0] == "$PSTI" && words[1] == "032")
                {
                    ParseSTI032(GPS012); //there is also an $PSTI,030,... wich contains different data!
                }
            }// while still data

            if (isNMEAToSend)
            {
                isNMEAToSend = false;

                byte[] nmeaPGN = new byte[57];

                nmeaPGN[0] = 0x80;
                nmeaPGN[1] = 0x81;
                nmeaPGN[2] = 0x7C;
                nmeaPGN[3] = 0xD6;
                nmeaPGN[4] = 0x33; // nmea total array count minus 6

                //longitude
                Buffer.BlockCopy(BitConverter.GetBytes(longitudeSend), 0, nmeaPGN, 5, 8);
                longitudeSend = double.MaxValue;

                //latitude
                Buffer.BlockCopy(BitConverter.GetBytes(latitudeSend), 0, nmeaPGN, 13, 8);
                latitudeSend = double.MaxValue;

                //the different dual antenna headings
                Buffer.BlockCopy(BitConverter.GetBytes(headingTrueDual), 0, nmeaPGN, 21, 4);
                headingTrueDual = float.MaxValue;

                //single antenna heading in degrees
                Buffer.BlockCopy(BitConverter.GetBytes(headingTrue), 0, nmeaPGN, 25, 4);
                headingTrue = float.MaxValue;

                //speed converted to kmh from knots
                Buffer.BlockCopy(BitConverter.GetBytes(speed), 0, nmeaPGN, 29, 4);
                speed = float.MaxValue;

                //roll value in degrees
                Buffer.BlockCopy(BitConverter.GetBytes(roll), 0, nmeaPGN, 33, 4);
                this.roll = float.MaxValue;

                //altitude in meters
                Buffer.BlockCopy(BitConverter.GetBytes(altitude), 0, nmeaPGN, 37, 4);
                this.altitude = float.MaxValue;

                Buffer.BlockCopy(BitConverter.GetBytes(satellitesTracked), 0, nmeaPGN, 41, 2);
                satellitesTracked = ushort.MaxValue;

                nmeaPGN[43] = (byte)fixQuality;
                fixQuality = byte.MaxValue;

                Buffer.BlockCopy(BitConverter.GetBytes(hdopX100), 0, nmeaPGN, 44, 2);
                hdopX100 = ushort.MaxValue;

                Buffer.BlockCopy(BitConverter.GetBytes(ageX100), 0, nmeaPGN, 46, 2);
                ageX100 = ushort.MaxValue;

                Buffer.BlockCopy(BitConverter.GetBytes(imuHeading), 0, nmeaPGN, 48, 2);
                imuHeading = ushort.MaxValue;

                Buffer.BlockCopy(BitConverter.GetBytes(imuRoll), 0, nmeaPGN, 50, 2);
                imuRoll = short.MaxValue;

                Buffer.BlockCopy(BitConverter.GetBytes(imuPitch), 0, nmeaPGN, 52, 2);
                imuPitch = short.MaxValue;

                Buffer.BlockCopy(BitConverter.GetBytes(imuYawRate), 0, nmeaPGN, 54, 2);
                imuYawRate = short.MaxValue;


                int CK_A = 0;
                for (int j = 2; j < nmeaPGN.Length; j++)
                {
                    CK_A += nmeaPGN[j];
                }

                //checksum
                nmeaPGN[56] = (byte)CK_A;

                //Send nmea to AgOpenGPS
                SendToLoopBackMessageAOG(nmeaPGN);

                //Send nmea to autosteer module 8888
                if (isSendNMEAToUDP) SendUDPMessage(nmeaPGN);
            }
        }

        private void ParseKSXT()
        {
            if (!string.IsNullOrEmpty(words[1]) && !string.IsNullOrEmpty(words[2]) && !string.IsNullOrEmpty(words[3])
                && !string.IsNullOrEmpty(words[4]) && !string.IsNullOrEmpty(words[5]))
            {
                double.TryParse(words[2], NumberStyles.Float, CultureInfo.InvariantCulture, out longitude);
                longitudeSend = longitude;

                double.TryParse(words[3], NumberStyles.Float, CultureInfo.InvariantCulture, out latitude);
                latitudeSend = latitude;

                float.TryParse(words[4], NumberStyles.Float, CultureInfo.InvariantCulture, out altitude);
                altitudeData = altitude;

                float.TryParse(words[5], NumberStyles.Float, CultureInfo.InvariantCulture, out headingTrueDual);
                headingTrueDualData = headingTrueDual;

                float.TryParse(words[6], NumberStyles.Float, CultureInfo.InvariantCulture, out rollK);

                float.TryParse(words[8], NumberStyles.Float, CultureInfo.InvariantCulture, out speed);
                speedData = speed;

                byte.TryParse(words[10], NumberStyles.Float, CultureInfo.InvariantCulture, out fixQuality);
                if (fixQuality == 0) fixQualityData = 0;
                else if (fixQuality == 1) fixQualityData = 1;
                else if (fixQuality == 2) fixQualityData = 5;
                else if (fixQuality == 3) fixQualityData = 4;

                fixQuality = fixQualityData;

                int headingQuality;

                int.TryParse(words[11], NumberStyles.Float, CultureInfo.InvariantCulture, out headingQuality);

                if (headingQuality == 3)   // roll only when rtk 
                {
                    roll = (float)(rollK);
                    rollData = rollK;
                }
                else
                {
                    roll = float.MinValue;
                    rollData = 0;
                }

                ushort.TryParse(words[13], NumberStyles.Float, CultureInfo.InvariantCulture, out satellitesTracked);
                satellitesData = satellitesTracked;


                float.TryParse(words[20], NumberStyles.Float, CultureInfo.InvariantCulture, out ageData);
                ageX100 = (ushort)(ageData * 100.0);

                isNMEAToSend = true;
            }
        }

        private void ParseGGA(byte GPS012)
        {
            double longitude = 0, latitude = 0;  // hide values (only local)

            #region GGA Message
            //$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M ,  ,*47

            //GGA          Global Positioning System Fix Data
            //123519       Fix taken at 12:35:19 UTC
            //4807.038,N   Latitude 48 deg 07.038' N
            //01131.000,E  Longitude 11 deg 31.000' E
            //1            Fix quality: 0 = invalid
            //                          1 = GPS fix (SPS)
            //                          2 = DGPS fix
            //                          3 = PPS fix
            //                          4 = Real Time Kinematic
            //                          5 = Float RTK
            //                          6 = estimated (dead reckoning) (2.3 feature)
            //                          7 = Manual input mode
            //                          8 = Simulation mode
            //08           Number of satellites being tracked
            //0.9          Horizontal dilution of position
            //545.4,M      Altitude, Meters, above mean sea level
            //46.9,M       Height of geoid (mean sea level) above WGS84 ellipsoid
            //(empty field) time in seconds since last DGPS update
            //(empty field) DGPS station ID number
            //*47          the checksum data, always begins with *
            #endregion GGA Message

            if (!string.IsNullOrEmpty(words[1]) && !string.IsNullOrEmpty(words[2]) && !string.IsNullOrEmpty(words[3])
                && !string.IsNullOrEmpty(words[4]) && !string.IsNullOrEmpty(words[5]))
            {
                //double.TryParse(words[1], NumberStyles.Float, CultureInfo.InvariantCulture, out double UTC);
                //if ((UTC < LastUpdateUTC ? 240000 + UTC : UTC) - LastUpdateUTC > 0.045)
                //{

                //FixQuality
                byte.TryParse(words[6], NumberStyles.Float, CultureInfo.InvariantCulture, out fixQuality);
                fixQualityData[GPS012] = fixQuality;

                //satellites tracked
                ushort.TryParse(words[7], NumberStyles.Float, CultureInfo.InvariantCulture, out satellitesTracked);
                satellitesData[GPS012] = satellitesTracked;

                //hdop
                float.TryParse(words[8], NumberStyles.Float, CultureInfo.InvariantCulture, out hdopData[GPS012]);
                hdopX100 = (ushort)(hdopData[GPS012] * 100.0);

                //altitude
                float.TryParse(words[9], NumberStyles.Float, CultureInfo.InvariantCulture, out altitude);
                altitudeData[GPS012] = altitude;
                myAltitude[GPS012] = altitude;

                //age
                float.TryParse(words[13], NumberStyles.Float, CultureInfo.InvariantCulture, out ageData[GPS012]);
                ageX100 = (ushort)(ageData[GPS012]*100.0);

                //LastUpdateUTC = UTC;

                //get latitude and convert to decimal degrees
                int decim = words[2].IndexOf(".", StringComparison.Ordinal);
                if (decim == -1)
                {
                    words[2] += ".00";
                    decim = words[2].IndexOf(".", StringComparison.Ordinal);
                }

                decim -= 2;
                double.TryParse(words[2].Substring(0, decim), NumberStyles.Float, CultureInfo.InvariantCulture, out latitude);
                double.TryParse(words[2].Substring(decim), NumberStyles.Float, CultureInfo.InvariantCulture, out double temp);
                temp *= 0.01666666666667;  // = 1/60
                latitude += temp;
                if (words[3] == "S")
                    latitude *= -1;
                latitudeSend = latitude;
                myLattitude[GPS012] = latitude;

                //get longitude and convert to decimal degrees
                decim = words[4].IndexOf(".", StringComparison.Ordinal);
                if (decim == -1)
                {
                    words[4] += ".00";
                    decim = words[4].IndexOf(".", StringComparison.Ordinal);
                }

                decim -= 2;
                double.TryParse(words[4].Substring(0, decim), NumberStyles.Float, CultureInfo.InvariantCulture, out longitude);
                double.TryParse(words[4].Substring(decim), NumberStyles.Float, CultureInfo.InvariantCulture, out temp);
                longitude += temp * 0.0166666666667;  // = 1/60

                { if (words[5] == "W") longitude *= -1; }
                longitudeSend = longitude;
                myLongitude[GPS012] = longitude;

                if (lastSentence == "GGA") isNMEAToSend = true;

                myTime[GPS012] = DateTime.UtcNow.Millisecond;
            }
        }

        private void ParseVTG(byte GPS012)
        {
            #region VTG Message
            //$GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48

            //VTG          Track made good and ground speed
            //054.7,T      True track made good (degrees)
            //034.4,M      Magnetic track made good
            //005.5,N      Ground speed, knots
            //010.2,K      Ground speed, Kilometers per hour
            //*48          Checksum
            #endregion VTG Message


            if (!string.IsNullOrEmpty(words[7]))
            {
                //kph for speed
                float.TryParse(words[7], NumberStyles.Float, CultureInfo.InvariantCulture, out speed);
                speedData[GPS012] = speed;
            }
            else
            {
                speed = speedData[GPS012] = 0;
            }

            if (!string.IsNullOrEmpty(words[1]))
            { 
                //True heading
                float.TryParse(words[1], NumberStyles.Float, CultureInfo.InvariantCulture, out headingTrue);
                headingTrueData = headingTrue;
            }

            if (lastSentence == "VTG") isNMEAToSend = true;
        }

        private void ParseAVR(byte GPS012)
        {
            #region AVR Message
            // $PTNL,AVR,145331.50,+35.9990,Yaw,-7.8209,Tilt,-0.4305,Roll,444.232,3,1.2,17 * 03

            //0 Message ID $PTNL,AVR
            //1 UTC of vector fix
            //2 Yaw angle, in degrees
            //3 Yaw
            //4 Tilt angle, in degrees
            //5 Tilt
            //6 Roll angle, in degrees
            //7 Roll
            //8 Range, in meters
            //9 GPS quality indicator:
            // 0: Fix not available or invalid
            // 1: Autonomous GPS fix
            // 2: Differential carrier phase solution RTK(Float)
            // 3: Differential carrier phase solution RTK(Fix)
            // 4: Differential code-based solution, DGPS
            //10 PDOP
            //11 Number of satellites used in solution
            //12 The checksum data, always begins with *
            #endregion AVR Message

            if (!string.IsNullOrEmpty(words[1]))
            {
                float.TryParse(words[8] == "Roll" ? words[7] : words[5], NumberStyles.Float, CultureInfo.InvariantCulture, out rollK);

                //Kalman filter
                Pc = P + varProcess;
                G = Pc / (Pc + varRoll);
                P = (1 - G) * Pc;
                Xp = XeRoll;
                Zp = Xp;
                XeRoll = (G * (rollK - Zp)) + Xp;
                rollData = XeRoll;

                roll = (float)(XeRoll);

                if (lastSentence == "AVR") isNMEAToSend = true;
            }
        }

        private void ParseHPD(byte GPS012)
        {
            if (!string.IsNullOrEmpty(words[1]))
            {
                //Dual heading
                float.TryParse(words[3], NumberStyles.Float, CultureInfo.InvariantCulture, out headingTrueDual);
                headingTrueDualData = headingTrueDual;

                float.TryParse(words[4], NumberStyles.Float, CultureInfo.InvariantCulture, out rollK);

                double.TryParse(words[18], NumberStyles.Float, CultureInfo.InvariantCulture, out double baseline);

                //int.TryParse(words[21], NumberStyles.Float, CultureInfo.InvariantCulture, out int solheading);

                if (baseline > 0)   // roll only when rtk and baseline
                {
                    roll = (float)(rollK);
                    rollData = rollK;
                }
                else
                {
                    roll = float.MinValue;
                    rollData = 0;
                }

                if (lastSentence == "HPD") isNMEAToSend = true;
            }
        }

        private void ParseOGI(byte GPS012)
        {
            #region PAOGI Message
            /*
            $PAOGI
            (1) 123519 Fix taken at 1219 UTC

            Roll corrected position
            (2,3) 4807.038,N Latitude 48 deg 07.038' N
            (4,5) 01131.000,E Longitude 11 deg 31.000' E

            (6) 1 Fix quality: 
                0 = invalid
                1 = GPS fix(SPS)
                2 = DGPS fix
                3 = PPS fix
                4 = Real Time Kinematic
                5 = Float RTK
                6 = estimated(dead reckoning)(2.3 feature)
                7 = Manual input mode
                8 = Simulation mode
            (7) Number of satellites being tracked
            (8) 0.9 Horizontal dilution of position
            (9) 545.4 Altitude (ALWAYS in Meters, above mean sea level)
            (10) 1.2 time in seconds since last DGPS update

            (11) 022.4 Speed over the ground in knots - can be positive or negative

            FROM AHRS:
            (12) Heading in degrees
            (13) Roll angle in degrees(positive roll = right leaning - right down, left up)
            (14) Pitch angle in degrees(Positive pitch = nose up)
            (15) Yaw Rate in Degrees / second

            * CHKSUM
            */
            #endregion PAOGI Message

            if (!string.IsNullOrEmpty(words[1]) && !string.IsNullOrEmpty(words[2]) && !string.IsNullOrEmpty(words[3])
                && !string.IsNullOrEmpty(words[4]) && !string.IsNullOrEmpty(words[5]))
            {
                //double.TryParse(words[1], NumberStyles.Float, CultureInfo.InvariantCulture, out double UTC);
                //if ((UTC < LastUpdateUTC ? 240000 + UTC : UTC) - LastUpdateUTC > 0.045)
                //{

                //FixQuality
                byte.TryParse(words[6], NumberStyles.Float, CultureInfo.InvariantCulture, out fixQuality);
                fixQualityData[GPS012] = fixQuality;

                //satellites tracked
                ushort.TryParse(words[7], NumberStyles.Float, CultureInfo.InvariantCulture, out satellitesTracked);
                satellitesData[GPS012] = satellitesTracked;

                //hdop
                float.TryParse(words[8], NumberStyles.Float, CultureInfo.InvariantCulture, out hdopData[GPS012]);
                hdopX100 = (ushort)(hdopData[GPS012] * 100.0);

                //altitude
                float.TryParse(words[9], NumberStyles.Float, CultureInfo.InvariantCulture, out altitude);
                altitudeData[GPS012] = altitude;

                //kph for speed - knots read
                float.TryParse(words[11], NumberStyles.Float, CultureInfo.InvariantCulture, out speed);
                speed *= 1.852f;
                speedData[GPS012] = speed;

                //Dual antenna derived heading
                float.TryParse(words[12], NumberStyles.Float, CultureInfo.InvariantCulture, out headingTrueDual);
                headingTrueDualData = headingTrueDual;

                //roll
                float.TryParse(words[13], NumberStyles.Float, CultureInfo.InvariantCulture, out roll);
                rollData = roll;

                //get latitude and convert to decimal degrees
                int decim = words[2].IndexOf(".", StringComparison.Ordinal);
                if (decim == -1)
                {
                    words[2] += ".00";
                    decim = words[2].IndexOf(".", StringComparison.Ordinal);
                }

                //age
                float.TryParse(words[10], NumberStyles.Float, CultureInfo.InvariantCulture, out ageData[GPS012]);
                ageX100 = (ushort)(ageData[GPS012] * 100.0);

                decim -= 2;
                double.TryParse(words[2].Substring(0, decim), NumberStyles.Float, CultureInfo.InvariantCulture, out latitude);
                double.TryParse(words[2].Substring(decim), NumberStyles.Float, CultureInfo.InvariantCulture, out double temp);
                temp *= 0.01666666666666666666666666666667;
                latitude += temp;
                if (words[3] == "S")
                    latitude *= -1;

                latitudeSend = latitude;

                //get longitude and convert to decimal degrees
                decim = words[4].IndexOf(".", StringComparison.Ordinal);
                if (decim == -1)
                {
                    words[4] += ".00";
                    decim = words[4].IndexOf(".", StringComparison.Ordinal);
                }

                decim -= 2;
                double.TryParse(words[4].Substring(0, decim), NumberStyles.Float, CultureInfo.InvariantCulture, out longitude);
                double.TryParse(words[4].Substring(decim), NumberStyles.Float, CultureInfo.InvariantCulture, out temp);
                longitude += temp * 0.01666666666666666666666666666667;

                { if (words[5] == "W") longitude *= -1; }
                longitudeSend = longitude;

                //always send because its probably the only one.
                isNMEAToSend = true;
                //}
            }
        }

        private void ParsePANDA(byte GPS012)
        {
            #region PANDA Message
            /*
            $PANDA
            (1) Time of fix

            position
            (2,3) 4807.038,N Latitude 48 deg 07.038' N
            (4,5) 01131.000,E Longitude 11 deg 31.000' E

            (6) 1 Fix quality: 
                0 = invalid
                1 = GPS fix(SPS)
                2 = DGPS fix
                3 = PPS fix
                4 = Real Time Kinematic
                5 = Float RTK
                6 = estimated(dead reckoning)(2.3 feature)
                7 = Manual input mode
                8 = Simulation mode
            (7) Number of satellites being tracked
            (8) 0.9 Horizontal dilution of position
            (9) 545.4 Altitude (ALWAYS in Meters, above mean sea level)
            (10) 1.2 time in seconds since last DGPS update
            (11) Speed in knots

            FROM IMU:
            (12) Heading in degrees
            (13) Roll angle in degrees(positive roll = right leaning - right down, left up)
            
            (14) Pitch angle in degrees(Positive pitch = nose up)
            (15) Yaw Rate in Degrees / second

            * CHKSUM
            */
            #endregion PANDA Message

            if (!string.IsNullOrEmpty(words[1]) && !string.IsNullOrEmpty(words[2]) && !string.IsNullOrEmpty(words[3])
                && !string.IsNullOrEmpty(words[3]) && !string.IsNullOrEmpty(words[0]))
            {

                //get latitude and convert to decimal degrees
                int decim = words[2].IndexOf(".", StringComparison.Ordinal);
                if (decim == -1)
                {
                    words[2] += ".00";
                    decim = words[2].IndexOf(".", StringComparison.Ordinal);
                }

                decim -= 2;
                double.TryParse(words[2].Substring(0, decim), NumberStyles.Float, CultureInfo.InvariantCulture, out latitude);
                double.TryParse(words[2].Substring(decim), NumberStyles.Float, CultureInfo.InvariantCulture, out double temp);
                temp *= 0.01666666666666666666666666666667;
                latitude += temp;
                if (words[3] == "S")
                    latitude *= -1;

                latitudeSend = latitude;

                //get longitude and convert to decimal degrees
                decim = words[4].IndexOf(".", StringComparison.Ordinal);
                if (decim == -1)
                {
                    words[4] += ".00";
                    decim = words[4].IndexOf(".", StringComparison.Ordinal);
                }

                decim -= 2;
                double.TryParse(words[4].Substring(0, decim), NumberStyles.Float, CultureInfo.InvariantCulture, out longitude);
                double.TryParse(words[4].Substring(decim), NumberStyles.Float, CultureInfo.InvariantCulture, out temp);
                longitude += temp * 0.01666666666666666666666666666667;

                { if (words[5] == "W") longitude *= -1; }
                longitudeSend = longitude;

                //FixQuality
                byte.TryParse(words[6], NumberStyles.Float, CultureInfo.InvariantCulture, out fixQuality);
                fixQualityData[GPS012] = fixQuality;

                //satellites tracked
                ushort.TryParse(words[7], NumberStyles.Float, CultureInfo.InvariantCulture, out satellitesTracked);
                satellitesData[GPS012] = satellitesTracked;

                //hdop
                float.TryParse(words[8], NumberStyles.Float, CultureInfo.InvariantCulture, out hdopData[GPS012]);
                hdopX100 = (ushort)(hdopData[GPS012] * 100.0);

                //altitude
                float.TryParse(words[9], NumberStyles.Float, CultureInfo.InvariantCulture, out altitude);
                altitudeData[GPS012] = altitude;

                //age
                float.TryParse(words[10], NumberStyles.Float, CultureInfo.InvariantCulture, out ageData[GPS012]);
                ageX100 = (ushort)(ageData[GPS012] * 100.0);

                //kph for speed - knots read
                float.TryParse(words[11], NumberStyles.Float, CultureInfo.InvariantCulture, out speed);
                speed *= 1.852f;
                speedData[GPS012] = speed;

                //imu heading
                ushort.TryParse(words[12], NumberStyles.Float, CultureInfo.InvariantCulture, out imuHeading);
                imuHeadingData = imuHeading;

                //roll
                short.TryParse(words[13], NumberStyles.Float, CultureInfo.InvariantCulture, out imuRoll);
                imuRollData = imuRoll;

                //Pitch
                short.TryParse(words[14], NumberStyles.Float, CultureInfo.InvariantCulture, out imuPitch);
                imuPitchData = imuPitch;

                //YawRate
                short.TryParse(words[15], NumberStyles.Float, CultureInfo.InvariantCulture, out imuYawRate);
                imuYawRateData = imuYawRate;

                //always send because its probably the only one.
                isNMEAToSend = true;
                //}
            }
        }

        private void ParseHDT(byte GPS012)
        {
            #region HDT Message
            //$GNHDT,123.456,T * 00

            //(0)   Message ID $GNHDT
            //(1)   Heading in degrees
            //(2)   T: Indicates heading relative to True North
            //(3)   The checksum data, always begins with *
            #endregion HDT Message

            if (!string.IsNullOrEmpty(words[1]))
            {
                //True heading
                float.TryParse(words[1], NumberStyles.Float, CultureInfo.InvariantCulture, out headingTrueDual);
                headingTrueDualData = headingTrueDual;

                if (lastSentence == "HDT") isNMEAToSend = true;
            }
        }

        private void ParseSTI032(byte GPS012) //heading and roll from SkyTraQ receiver
        {
            #region STI0 Message
            //$PSTI,032,033010.000,111219,A,R,‐4.968,‐10.817,‐1.849,12.046,204.67,,,,,*39

            //(1) 032 Baseline Data indicator
            //(2) UTC time hhmmss.sss
            //(3) UTC date ddmmyy
            //(4) Status:
            //    V = Void
            //    A = Active
            //(5) Mode Indicator:
            //    F = Float RTK
            //    R = fixed RTK
            //(6) East-projection of baseline, meters
            //(7) North-projection of baseline, meters
            //(8) Up-projection of baseline, meters
            //(9) Baseline length, meters
            //(10) Baseline course: angle between baseline vector and north direction, degrees
            //(11) - (15) Reserved
            //(16) * Checksum
            #endregion STI0 Message

            if (!string.IsNullOrEmpty(words[10]))
            {
                //baselineCourse: angle between baseline vector (from kinematic base to rover) and north direction, degrees
                float.TryParse(words[10], NumberStyles.Float, CultureInfo.InvariantCulture, out float baselineCourse);
                headingTrueDual = ((baselineCourse < 270.0f) ? (baselineCourse + 90.0f) : (baselineCourse - 270.0f)); //Rover Antenna on the left, kinematic base on the right!!!
            }

            if (!string.IsNullOrEmpty(words[8]) && !string.IsNullOrEmpty(words[9]))
            {
                double.TryParse(words[8], NumberStyles.Float, CultureInfo.InvariantCulture, out double upProjection); //difference in hight of both antennas (rover - kinematic base)
                double.TryParse(words[9], NumberStyles.Float, CultureInfo.InvariantCulture, out double baselineLength); //distance between kinematic base and rover
                rollK = (float)glm.toDegrees(Math.Atan(upProjection / baselineLength)); //roll to the right is positiv (rover left, kinematic base right!)

                //Kalman filter
                Pc = P + varProcess;
                G = Pc / (Pc + varRoll);
                P = (1 - G) * Pc;
                Xp = XeRoll;
                Zp = Xp;
                XeRoll = (G * (rollK - Zp)) + Xp;
                rollData = XeRoll;

                roll = (float)(XeRoll);

                if (lastSentence == "STI") isNMEAToSend = true;
            }
        }

        private void ParseTRA(byte GPS012)  //tra contains hdt and roll for the ub482 receiver
        {
            if (!string.IsNullOrEmpty(words[1]))
            {
                float.TryParse(words[2], NumberStyles.Float, CultureInfo.InvariantCulture, out headingTrueDual);
                headingTrueDualData = headingTrueDual;

                //  Console.WriteLine(HeadingForced);
                float.TryParse(words[3], NumberStyles.Float, CultureInfo.InvariantCulture, out rollK);
                // Console.WriteLine(nRoll);

                int.TryParse(words[5], NumberStyles.Float, CultureInfo.InvariantCulture, out int trasolution);
                if (trasolution != 4) rollK = 0;
                rollData = rollK;
                roll = (float)(rollK);

                if (lastSentence == "TRA") isNMEAToSend = true;

            }
        }

        private void ParseRMC(byte GPS012)
        {
            #region RMC Message
            //$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A

            //RMC          Recommended Minimum sentence C
            //123519       Fix taken at 12:35:19 UTC
            //A            Status A=active or V=Void.
            //4807.038,N   Latitude 48 deg 07.038' N
            //01131.000,E  Longitude 11 deg 31.000' E
            //022.4        Speed over the ground in knots
            //084.4        Track angle in degrees True
            //230394       Date - 23rd of March 1994
            //003.1,W      Magnetic Variation
            //*6A          * Checksum
            #endregion RMC Message

            if (!string.IsNullOrEmpty(words[1]) && !string.IsNullOrEmpty(words[3]) && !string.IsNullOrEmpty(words[4])
                && !string.IsNullOrEmpty(words[5]) && !string.IsNullOrEmpty(words[6]))
            {
                //Convert from knots to kph for speed
                float.TryParse(words[7], NumberStyles.Float, CultureInfo.InvariantCulture, out speed);
                speed *= 1.852f;
                speedData[GPS012] = speed;

                //True heading
                float.TryParse(words[8], NumberStyles.Float, CultureInfo.InvariantCulture, out headingTrueDual);
                headingTrueDualData = headingTrueDual;

                double.TryParse(words[1], NumberStyles.Float, CultureInfo.InvariantCulture, out double UTC);
                if ((UTC < LastUpdateUTC ? 240000 + UTC : UTC) - LastUpdateUTC > 0.045)
                {
                    LastUpdateUTC = UTC;

                    //get latitude and convert to decimal degrees
                    int decim = words[3].IndexOf(".", StringComparison.Ordinal);
                    if (decim == -1)
                    {
                        words[3] += ".00";
                        decim = words[3].IndexOf(".", StringComparison.Ordinal);
                    }

                    decim -= 2;
                    double.TryParse(words[3].Substring(0, decim), NumberStyles.Float, CultureInfo.InvariantCulture, out latitude);
                    double.TryParse(words[3].Substring(decim), NumberStyles.Float, CultureInfo.InvariantCulture, out double temp);
                    latitude += temp * 0.01666666666666666666666666666667;

                    if (words[4] == "S")
                        latitude *= -1;
                    latitudeSend = latitude;

                    //get longitude and convert to decimal degrees
                    decim = words[5].IndexOf(".", StringComparison.Ordinal);
                    if (decim == -1)
                    {
                        words[5] += ".00";
                        decim = words[5].IndexOf(".", StringComparison.Ordinal);
                    }

                    decim -= 2;
                    double.TryParse(words[5].Substring(0, decim), NumberStyles.Float, CultureInfo.InvariantCulture, out longitude);
                    double.TryParse(words[5].Substring(decim), NumberStyles.Float, CultureInfo.InvariantCulture, out temp);
                    longitude += temp * 0.01666666666666666666666666666667;

                    if (words[6] == "W") longitude *= -1;
                    longitudeSend = longitude;

                    isNMEAToSend = true;
                }
            }
        }

        public bool ValidateChecksum(string Sentence)
        {
            int sum = 0;
            try
            {
                char[] sentenceChars = Sentence.ToCharArray();
                // All character xor:ed results in the trailing hex checksum
                // The checksum calc starts after '$' and ends before '*'

                int inx = Sentence.IndexOf("*", StringComparison.Ordinal);

                if (sentenceChars.Length - inx == 4)
                {

                    for (inx = 1; ; inx++)
                    {
                        if (inx >= sentenceChars.Length) // No checksum found
                            return false;
                        var tmp = sentenceChars[inx];
                        // Indicates end of data and start of checksum
                        if (tmp == '*') break;
                        sum ^= tmp;    // Build checksum
                    }

                    // Calculated checksum converted to a 2 digit hex string
                    string sumStr = string.Format("{0:X2}", sum);

                    // Compare to checksum in sentence
                    return sumStr.Equals(Sentence.Substring(inx + 1, 2));
                }
                else
                {
                    //CRC code goes here - return true for now if $KS
                    if(sentenceChars[0] == 36 && sentenceChars[1] == 75 && sentenceChars[2] == 83) return true;
                    else return false;  
                }
            }
            catch (Exception)
            {
                //mf.WriteErrorLog("Validate Checksum" + e);
                return false;
            }
        }
    }
}
