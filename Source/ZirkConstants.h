//
//  ZirkConstants.h
//  ZirkOSCJUCE
//
//  Created by Lud's on 21/02/13.
//
//

#ifndef ZirkOSCJUCE_ZirkConstants_h
#define ZirkOSCJUCE_ZirkConstants_h


const String ZirkOSC_Azim_name = "Azimuth";
const float  ZirkOSC_Azim_Min = -180.0f;
const float  ZirkOSC_Azim_Max = 180.0f;
const float  ZirkOSC_Azim_Def = 0.0f;

const String ZirkOSC_Elev_name = "Elevation";
const float  ZirkOSC_Elev_Min = 0.0f;
const float  ZirkOSC_Elev_Max = 90.0f;
const float  ZirkOSC_Elev_Def = 0.0f;

const String ZirkOSC_AzimDelta_name = "Azimuth Delta";
const float  ZirkOSC_AzimDelta_Min = 0.0f;
const float  ZirkOSC_AzimDelta_Max = ZirkOSC_Azim_Max * 2.0;
const float  ZirkOSC_AzimDelta_Def = 0.0f;

const String ZirkOSC_ElevDelta_name = "Elevation Delta";
const float  ZirkOSC_ElevDelta_Min = 0.0f;
const float  ZirkOSC_ElevDelta_Max = 90;
const float  ZirkOSC_ElevDelta_Def = 0.0f;

const String ZirkOSC_AzimSpan_name = "Azimuth Span";
const float  ZirkOSC_AzimSpan_Min = 0.0f;
const float  ZirkOSC_AzimSpan_Max = ZirkOSC_Azim_Max * 2.0;
const float  ZirkOSC_AzimSpan_Def = 0.0f;

const String ZirkOSC_ElevSpan_name = "Elevation Span";
const float  ZirkOSC_ElevSpan_Min = 0.0f;
const float  ZirkOSC_ElevSpan_Max = 90.0f;
const float  ZirkOSC_ElevSpan_Def = 0.0f;

const String ZirkOSC_Gain_name = "Gain";
const float  ZirkOSC_Gain_Min = 0.0f;
const float  ZirkOSC_Gain_Max = 1.0f;
const float  ZirkOSC_Gain_Def = 1.0f;


const int ZirkOSC_Window_Width  = 400;
const int ZirkOSC_Window_Height = 600;

const int ZirkOSC_Center_X = 160.0f;
const int ZirkOSC_Center_Y = 180.0f;

const int ZirkOSC_DomeRadius = 150.0f;

const float ZirkOSC_MarksAngles[] =
{
	22.5, 90 - 22.5, 90 + 22.5, 180 - 22.5, 180 + 22.5, 270 - 22.5, 270 + 22.5, 360 - 22.5
};
const int ZirkOSC_NumMarks = sizeof(ZirkOSC_MarksAngles)/sizeof(ZirkOSC_MarksAngles[0]);


#endif
