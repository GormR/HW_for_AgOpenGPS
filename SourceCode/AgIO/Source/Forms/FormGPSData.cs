﻿//Please, if you use this give me some credit
//Copyright BrianTee, copy right out of it.

using System;
using System.Windows.Forms;

namespace AgIO
{
    public partial class FormGPSData : Form
    {
        private readonly FormLoop mf = null;

        public FormGPSData(Form callingForm)
        {
            mf = callingForm as FormLoop;
            InitializeComponent();
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            lblLatitude.Text = mf.myLattitude[0].ToString("N7");
            lblLongitude.Text = mf.myLongitude[0].ToString("N7");
            lblLatitude2.Text = mf.myLattitude[1].ToString("N7");
            lblLongitude2.Text = mf.myLongitude[1].ToString("N7");
            //     lblCurrentHeading.Text = mf.GPSheading.ToString("N7");
            //     lblCurrentRoll.Text = mf.GPSroll.ToString("N7");

            ////other sat and GPS info
            lblFixQuality.Text = mf.FixQuality;
            lblFixQuality2.Text = mf.FixQuality2;
            lblSatsTracked.Text = mf.satellitesData[0].ToString();
            lblSatsTracked2.Text = mf.satellitesData[1].ToString();
            lblHDOP.Text = mf.hdopData[0].ToString();
            lblHDOP2.Text = mf.hdopData[1].ToString();
            lblSpeed.Text = mf.speedData[0].ToString("N2");
            lblSpeed2.Text = mf.speedData[1].ToString("N2");

            lblRoll.Text = mf.rollData.ToString("N2");
            lblIMURoll.Text = mf.imuRollData.ToString();
            lblIMUPitch.Text = mf.imuPitchData.ToString();
            lblIMUYawRate.Text = mf.imuYawRateData.ToString();
            lblIMUHeading.Text = mf.imuHeadingData.ToString();

            lblAge.Text = mf.ageData[0].ToString("N1");
            lblAge2.Text = mf.ageData[1].ToString("N1");

            lblGPSHeading.Text = mf.headingTrueData.ToString("N2");
            lblDualHeading.Text = mf.headingTrueDualData.ToString("N2");

            lblAltitude.Text = mf.altitudeData[0].ToString("N3");
            lblAltitude2.Text = mf.altitudeData[1].ToString("N3");

            lblAntDist.Text = mf.antennaDist.ToString("N3");
            lblHeadDual.Text = mf.GPSheading.ToString("N1");
            lblRollDual.Text = mf.GPSroll.ToString("N1");

            tboxVTG.Text = mf.vtgSentence[0];
            tboxGGA.Text = mf.ggaSentence[0];
            tboxPAOGI.Text = mf.paogiSentence;
            tboxAVR.Text = mf.avrSentence;
            tboxHDT.Text = mf.hdtSentence;
            //tboxRMC.Text = mf.rmcSentence;
            tboxHPD.Text = mf.hpdSentence;
            tboxPANDA.Text = mf.pandaSentence;
            tboxKSXT.Text = mf.ksxtSentence;
            tboxVTG2.Text = mf.vtgSentence[1];
            tboxGGA2.Text = mf.ggaSentence[1];
        }

        private void FormGPSData_Load(object sender, EventArgs e)
        {
            tboxGGA.Text = "";
            tboxVTG.Text = "";
            tboxHDT.Text = "";
            tboxAVR.Text = "";
            tboxPAOGI.Text = "";
            tboxHPD.Text = "";
            tboxPANDA.Text = "";
            tboxKSXT.Text = "";
        }

        private void FormGPSData_FormClosing(object sender, FormClosingEventArgs e)
        {
            mf.isGPSSentencesOn = false;
        }

        private void lblLongitude_Click(object sender, EventArgs e)
        {

        }

        private void label13_Click(object sender, EventArgs e)
        {

        }

        private void label20_Click(object sender, EventArgs e)
        {

        }

        private void lblAge2_Click(object sender, EventArgs e)
        {

        }
    }
}


//lblAreaAppliedMinusOverlap.Text = ((fd.actualAreaCovered * glm.m2ac).ToString("N2"));
//lblAreaMinusActualApplied.Text = (((mf.fd.areaBoundaryOuterLessInner - mf.fd.actualAreaCovered) * glm.m2ac).ToString("N2"));
//lblOverlapPercent.Text = (fd.overlapPercent.ToString("N2")) + "%";
//lblAreaOverlapped.Text = (((fd.workedAreaTotal - fd.actualAreaCovered) * glm.m2ac).ToString("N3"));

//lblAreaAppliedMinusOverlap.Text = ((fd.actualAreaCovered * glm.m2ha).ToString("N2"));
//lblAreaMinusActualApplied.Text = (((mf.fd.areaBoundaryOuterLessInner - mf.fd.actualAreaCovered) * glm.m2ha).ToString("N2"));
//lblOverlapPercent.Text = (fd.overlapPercent.ToString("N2")) + "%";
//lblAreaOverlapped.Text = (((fd.workedAreaTotal - fd.actualAreaCovered) * glm.m2ha).ToString("N3"));


//lblLookOnLeft.Text = mf.tool.lookAheadDistanceOnPixelsLeft.ToString("N0");
//lblLookOnRight.Text = mf.tool.lookAheadDistanceOnPixelsRight.ToString("N0");
//lblLookOffLeft.Text = mf.tool.lookAheadDistanceOffPixelsLeft.ToString("N0");
//lblLookOffRight.Text = mf.tool.lookAheadDistanceOffPixelsRight.ToString("N0");

//lblLeftToolSpd.Text = (mf.tool.toolFarLeftSpeed*3.6).ToString("N1");
//lblRightToolSpd.Text = (mf.tool.toolFarRightSpeed*3.6).ToString("N1");

//lblSectSpdLeft.Text = (mf.section[0].speedPixels*0.36).ToString("N1");
//lblSectSpdRight.Text = (mf.section[mf.tool.numOfSections-1].speedPixels*0.36).ToString("N1");