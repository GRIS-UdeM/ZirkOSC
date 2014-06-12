//
//  ZirkConstants.h
//  ZirkOSCJUCE
//

//

#ifndef ZirkOSCJUCE_ZirkConstants_h
#define ZirkOSCJUCE_ZirkConstants_h
const int ZirkOSC_Max_Sources = 8;

const String ZirkOSC_Azim_name [ZirkOSC_Max_Sources] = {"1 Azimuth", "2 Azimuth","3 Azimuth","4 Azimuth","5 Azimuth","6 Azimuth","7 Azimuth","8 Azimuth"};
const float  ZirkOSC_Azim_Min = -180.0f;
const float  ZirkOSC_Azim_Max = 180.0f;
const float  ZirkOSC_Azim_Def = 0.0f;

const String ZirkOSC_Elev_name [ZirkOSC_Max_Sources] = {"1 Elevation", "2 Elevation","3 Elevation","4 Elevation","5 Elevation","6 Elevation","7 Elevation","8 Elevation"};
const float  ZirkOSC_Elev_Min = 0.0f;
const float  ZirkOSC_Elev_Max = 90.0f;
const float  ZirkOSC_Elev_Def = 0.0f;

const String ZirkOSC_AzimDelta_name = "Azim delta";
const float  ZirkOSC_AzimDelta_Min = 0.0f;
const float  ZirkOSC_AzimDelta_Max = ZirkOSC_Azim_Max * 2.0;
const float  ZirkOSC_AzimDelta_Def = 0.0f;

const String ZirkOSC_ElevDelta_name = "Elevation Delta";
const float  ZirkOSC_ElevDelta_Min = 0.0f;
const float  ZirkOSC_ElevDelta_Max = 90;
const float  ZirkOSC_ElevDelta_Def = 0.0f;

const String ZirkOSC_AzimSpan_name[ZirkOSC_Max_Sources] = {"1 Azimuth Span", "2 Azimuth Span","3 Azimuth Span","4 Azimuth Span","5 Azimuth Span","6 Azimuth Span","7 Azimuth Span","8 Azimuth Span"};
const float  ZirkOSC_AzimSpan_Min = 0.0f;
const float  ZirkOSC_AzimSpan_Max = ZirkOSC_Azim_Max * 2.0;
const float  ZirkOSC_AzimSpan_Def = 0.0f;

const String ZirkOSC_ElevSpan_name [ZirkOSC_Max_Sources] = {"1 Elevation Span", "2 Elevation Span","3 Elevation Span","4 Elevation Span","5 Elevation Span","6 Elevation Span","7 Elevation Span","8 Elevation Span"};

const float  ZirkOSC_ElevSpan_Min = 0.0f;
const float  ZirkOSC_ElevSpan_Max = 90.0f;
const float  ZirkOSC_ElevSpan_Def = 0.0f;

const String ZirkOSC_Gain_name [ZirkOSC_Max_Sources] = {"1 Gain", "2 Gain","3 Gain","4 Gain","5 Gain","6 Gain","7 Gain","8 Gain"};
const float  ZirkOSC_Gain_Min = 0.0f;
const float  ZirkOSC_Gain_Max = 1.0f;
const float  ZirkOSC_Gain_Def = 1.0f;

const String ZirkOSC_Movement_Constraint_name = "Move_Constraint";
const String ZirkOSC_isOscActive_name = "is_OSC_Active";
const String ZirkOSC_isSpanLinked_name = "is_Span_Linked";

const String ZirkOSC_Channel_name [ZirkOSC_Max_Sources] = {"1 Channel", "2 Channel","3 Channel","4 Channel","5 Channel","6 Channel","7 Channel","8 Channel"};

const int ZirkOSC_TrajectoryGroupHeight = 150;

const int ZirkOSC_Window_Default_Width  = 400;
const int ZirkOSC_Window_Default_Height = 600 + ZirkOSC_TrajectoryGroupHeight;

const float ZirkOSC_MarksAngles[] =
{
	22.5, 90 - 22.5, 90 + 22.5, 180 - 22.5, 180 + 22.5, 270 - 22.5, 270 + 22.5, 360 - 22.5
};
const int ZirkOSC_NumMarks = sizeof(ZirkOSC_MarksAngles)/sizeof(ZirkOSC_MarksAngles[0]);

#endif
