//<<<<<<< HEAD
///*
// ==============================================================================
// ZirkOSC2: VST and AU audio plug-in enabling spatial movement of sound sources in a dome of speakers.
// 
// Copyright (C) 2015  GRIS-UdeM
// 
// Developers: Ludovic Laffineur, Vincent Berthiaume
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// ==============================================================================
// */
//
////
//
//#ifndef ZirkOSCJUCE_ZirkConstants_h
//#define ZirkOSCJUCE_ZirkConstants_h
//
//const int ZirkOSC_Max_Sources = 8;
//
//const String ZirkOSC_Azim_name [ZirkOSC_Max_Sources] = {"1 Azimuth", "2 Azimuth","3 Azimuth","4 Azimuth","5 Azimuth","6 Azimuth","7 Azimuth","8 Azimuth"};
//const float  ZirkOSC_Azim_Min = -180.0f;
//const float  ZirkOSC_Azim_Max = 180.0f;
//const float  ZirkOSC_Azim_Def = 0.0f;
//
//const String ZirkOSC_Elev_name [ZirkOSC_Max_Sources] = {"1 Elevation", "2 Elevation","3 Elevation","4 Elevation","5 Elevation","6 Elevation","7 Elevation","8 Elevation"};
//const float  ZirkOSC_Elev_Min = 0.0f;
//const float  ZirkOSC_Elev_Max = 90.0f;
//const float  ZirkOSC_Elev_Def = 0.0f;
//
//const String ZirkOSC_AzimDelta_name = "Azim delta";
//const float  ZirkOSC_AzimDelta_Min = 0.0f;
//const float  ZirkOSC_AzimDelta_Max = ZirkOSC_Azim_Max * 2.0;
//const float  ZirkOSC_AzimDelta_Def = 0.0f;
//
//const String ZirkOSC_ElevDelta_name = "Elevation Delta";
//const float  ZirkOSC_ElevDelta_Min = 0.0f;
//const float  ZirkOSC_ElevDelta_Max = 90;
//const float  ZirkOSC_ElevDelta_Def = 0.0f;
//
//const String ZirkOSC_AzimSpan_name[ZirkOSC_Max_Sources] = {"1 Azimuth Span", "2 Azimuth Span","3 Azimuth Span","4 Azimuth Span","5 Azimuth Span","6 Azimuth Span","7 Azimuth Span","8 Azimuth Span"};
//const float  ZirkOSC_AzimSpan_Min = 0.0f;
//const float  ZirkOSC_AzimSpan_Max = ZirkOSC_Azim_Max * 2.0;
//const float  ZirkOSC_AzimSpan_Def = 0.0f;
//
//const String ZirkOSC_ElevSpan_name [ZirkOSC_Max_Sources] = {"1 Elevation Span", "2 Elevation Span","3 Elevation Span","4 Elevation Span","5 Elevation Span","6 Elevation Span","7 Elevation Span","8 Elevation Span"};
//
//const float  ZirkOSC_ElevSpan_Min = 0.0f;
//const float  ZirkOSC_ElevSpan_Max = 90.0f;
//const float  ZirkOSC_ElevSpan_Def = 0.0f;
//
//const String ZirkOSC_Gain_name [ZirkOSC_Max_Sources] = {"1 Gain", "2 Gain","3 Gain","4 Gain","5 Gain","6 Gain","7 Gain","8 Gain"};
//const float  ZirkOSC_Gain_Min = 0.0f;
//const float  ZirkOSC_Gain_Max = 1.0f;
//const float  ZirkOSC_Gain_Def = 1.0f;
//
//const String ZirkOSC_Movement_Constraint_name = "Move_Constraint";
//const String ZirkOSC_isOscActive_name = "is_OSC_Active";
//const String ZirkOSC_isSpanLinked_name = "is_Span_Linked";
//const String ZirkOSC_SelectedTrajectory_name = "SelectedTrajectory";
//const String ZirkOSC_SelectedTrajectoryDirection_name = "SelectedTrajectoryDirection";
//const String ZirkOSC_SelectedTrajectoryReturn_name = "SelectedTrajectoryReturn";
//const String ZirkOSC_NbrTrajectories_name = "NbrTrajectories";
//const String ZirkOSC_DurationTrajectories_name = "DurationTrajectories";
//const String ZirkOSC_isSyncWTempo_name = "SyncWTempo";
//const String ZirkOSC_isWriteTrajectory_name = "WriteTrajectory";
//
//const String ZirkOSC_Channel_name [ZirkOSC_Max_Sources] = {"1 Channel", "2 Channel","3 Channel","4 Channel","5 Channel","6 Channel","7 Channel","8 Channel"};
//
////const int ZirkOSC_TrajectoryGroupHeight = 190;
//const int ZirkOSC_SlidersGroupHeight = 220;
//
//const int ZirkOSC_ConstraintComboBoxHeight = 25;
//
//const int ZirkOSC_Window_Default_Width  = 430;
//const int ZirkOSC_Window_Default_Height = 350 + ZirkOSC_ConstraintComboBoxHeight + ZirkOSC_SlidersGroupHeight;
//
//const float ZirkOSC_MarksAngles[] =
//{
//	22.5, 90 - 22.5, 90 + 22.5, 180 - 22.5, 180 + 22.5, 270 - 22.5, 270 + 22.5, 360 - 22.5
//};
//const int ZirkOSC_NumMarks = sizeof(ZirkOSC_MarksAngles)/sizeof(ZirkOSC_MarksAngles[0]);
//
////! Enum of the movement constraints
//enum AllConstraints {
//    Independant = 1,/*!< Independant mode */
//    Circular,        /*!< Circular */
//    FixedRadius,    /*!< All sources' radius are fixed */
//    FixedAngles,    /*!< Angle between sources are fixed */
//    FullyFixed,     /*!< FixedRadius and fixedAngles */
//    DeltaLocked,    /*!< Delta lock mode */
//    TotalNumberConstraints
//};
//
//enum AllTrajectoryTypes {
//    Circle = 1,
//    Ellipse,
//    Spiral,
//    Pendulum,
//    Random,
//    TotalNumberTrajectories    
//};
//
//enum AllTrajectoryDirections {
//    CW,
//    CCW,
//    In,
//    Out,
//    Crossover,
//    InCW,
//    InCCW,
//    OutCW,
//    OutCCW,
//    Slow,
//    Mid,
//    Fast
//};
//
//enum AllSyncOptions {
//    SyncWTempo = 1,
//    SyncWTime
//};
//
//static float PercentToHR(float percent, float min, float max){
//    return percent*(max-min)+min;
//}
//
//static float HRToPercent(float HRValue, float min, float max){
//    return (HRValue-min)/(max-min);
//}
//
//static int PercentToIntStartsAtOne(float percent, int max){
//    return percent * (max-1) + 1;
//}
//
////max here represent the total range of numbers. Max defaut value = ZirkOscjuceAudioProcessorEditor::TotalNumberConstraints
//static float IntToPercentStartsAtOne(int integer, int max){
//    return static_cast<float>((integer-1)) / (max - 1);
//}
//
////max here represent the total range of numbers. Max defaut value = ZirkOscjuceAudioProcessorEditor::TotalNumberConstraints
//static float IntToPercentStartsAtZero(int integer, int max){
//    return static_cast<float>(integer) / max;
//}
//
//static int PercentToIntStartsAtZero(float percent, int max){
//    return percent * max;
//}
//
//
//#endif
//=======
/*
 ==============================================================================
 ZirkOSC2: VST and AU audio plug-in enabling spatial movement of sound sources in a dome of speakers.
 
 Copyright (C) 2015  GRIS-UdeM
 
 Developers: Ludovic Laffineur, Vincent Berthiaume
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
 */

//

#ifndef ZirkOSCJUCE_ZirkConstants_h
#define ZirkOSCJUCE_ZirkConstants_h

const int ZirkOSC_Max_Sources = 8;

const String ZirkOSC_Azim_name [ZirkOSC_Max_Sources] = {"1 Azimuth", "2 Azimuth","3 Azimuth","4 Azimuth","5 Azimuth","6 Azimuth","7 Azimuth","8 Azimuth"};
const String ZirkOSC_X_name = " X";
const float  ZirkOSC_Azim_Min = -180.0f;
const float  ZirkOSC_Azim_Max = 180.0f;
const float  ZirkOSC_Azim_Def = 0.0f;

const String ZirkOSC_Elev_name [ZirkOSC_Max_Sources] = {"1 Elevation", "2 Elevation","3 Elevation","4 Elevation","5 Elevation","6 Elevation","7 Elevation","8 Elevation"};
const String ZirkOSC_Y_name = " Y";
const float  ZirkOSC_Elev_Min = 0.0f;
const float  ZirkOSC_Elev_Max = 90.0f;
const float  ZirkOSC_Elev_Def = 0.0f;

//const String ZirkOSC_AzimDelta_name = "Azim delta";
//const float  ZirkOSC_AzimDelta_Min = 0.0f;
//const float  ZirkOSC_AzimDelta_Max = ZirkOSC_Azim_Max * 2.0;
//const float  ZirkOSC_AzimDelta_Def = 0.0f;

//const String ZirkOSC_ElevDelta_name = "Elevation Delta";
//const float  ZirkOSC_ElevDelta_Min = 0.0f;
//const float  ZirkOSC_ElevDelta_Max = 90;
//const float  ZirkOSC_ElevDelta_Def = 0.0f;

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
const String ZirkOSC_SelectedTrajectory_name = "SelectedTrajectory";
const String ZirkOSC_SelectedTrajectoryDirection_name = "SelectedTrajectoryDirection";
const String ZirkOSC_SelectedTrajectoryReturn_name = "SelectedTrajectoryReturn";
const String ZirkOSC_NbrTrajectories_name = "NbrTrajectories";
const String ZirkOSC_DurationTrajectories_name = "DurationTrajectories";
const String ZirkOSC_isSyncWTempo_name = "SyncWTempo";
const String ZirkOSC_isWriteTrajectory_name = "WriteTrajectory";

const String ZirkOSC_Channel_name [ZirkOSC_Max_Sources] = {"1 Channel", "2 Channel","3 Channel","4 Channel","5 Channel","6 Channel","7 Channel","8 Channel"};

//const int ZirkOSC_TrajectoryGroupHeight = 190;
const int ZirkOSC_SlidersGroupHeight = 220;

const int ZirkOSC_ConstraintComboBoxHeight = 25;

const int ZirkOSC_Window_Default_Width  = 430;
const int ZirkOSC_Window_Default_Height = 350 + ZirkOSC_ConstraintComboBoxHeight + ZirkOSC_SlidersGroupHeight;

const float ZirkOSC_MarksAngles[] =
{
	22.5, 90 - 22.5, 90 + 22.5, 180 - 22.5, 180 + 22.5, 270 - 22.5, 270 + 22.5, 360 - 22.5
};
const int ZirkOSC_NumMarks = sizeof(ZirkOSC_MarksAngles)/sizeof(ZirkOSC_MarksAngles[0]);

//! Enum of the movement constraints
enum AllConstraints {
    Independant = 1,/*!< Independant mode */
    Circular,        /*!< Circular */
    FixedRadius,    /*!< All sources' radius are fixed */
    FixedAngles,    /*!< Angle between sources are fixed */
    FullyFixed,     /*!< FixedRadius and fixedAngles */
    DeltaLocked,    /*!< Delta lock mode */
    TotalNumberConstraints
};

enum AllTrajectoryTypes {
    Circle = 1,
    Ellipse,
    Spiral,
    Pendulum,
    Random,
    TotalNumberTrajectories    
};

enum AllTrajectoryDirections {
    CW,
    CCW,
    In,
    Out,
    Crossover,
    InCW,
    InCCW,
    OutCW,
    OutCCW,
    Slow,
    Mid,
    Fast
};

enum AllSyncOptions {
    SyncWTempo = 1,
    SyncWTime
};

template <typename T>
inline T clamp(const T& n, const T& lower, const T& upper) {
    return std::max(lower, std::min(n, upper));
}

inline float PercentToHR(float percent, float min, float max){
    return percent*(max-min)+min;
}

inline float HRToPercent(float HRValue, float min, float max){
    return (HRValue-min)/(max-min);
}

inline int PercentToIntStartsAtOne(float percent, int max){
    return percent * (max-1) + 1;
}

//max here represent the total range of numbers. Max defaut value = ZirkOscjuceAudioProcessorEditor::TotalNumberConstraints
inline float IntToPercentStartsAtOne(int integer, int max){
    return static_cast<float>((integer-1)) / (max - 1);
}

//max here represent the total range of numbers. Max defaut value = ZirkOscjuceAudioProcessorEditor::TotalNumberConstraints
inline float IntToPercentStartsAtZero(int integer, int max){
    return static_cast<float>(integer) / max;
}

inline int PercentToIntStartsAtZero(float percent, int max){
    return percent * max;
}

inline float degreeToRadian (float degree){
    return ((degree/360.0f)*2*3.1415);
}

inline float radianToDegree(float radian){
    return (radian/(2*3.1415)*360.0f);
}

#endif
//>>>>>>> param
