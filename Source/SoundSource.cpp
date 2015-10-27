/*
 ==============================================================================
 ZirkOSC: VST and AU audio plug-in enabling spatial movement of sound sources in a dome of speakers.
 
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


#include "SoundSource.h"
#include "ZirkConstants.h"
#include "PluginProcessor.h"


SoundSource::SoundSource()
: m_iSourceId(1)
, m_fGain(1.f)
, m_fAzimuthSpan(0.f)
, m_fElevationSpan(0.f)
, m_bElevationWasMaxed(false)
{
    m_iElevationStatus = normalRange;
    m_fElevOverflow = ZirkOscAudioProcessor::s_iDomeRadius;
}

SoundSource::SoundSource(float azimuth, float elevation, int p_iSrcId)
: SoundSource()
{
    initAzimuthAndElevation(azimuth,elevation);
    m_iSourceId = p_iSrcId;
}
void SoundSource::initAzimuthAndElevation(float p_fAzim, float p_fElev){
    if (p_fAzim>1)
        p_fAzim = p_fAzim - 1.0f;
    else if (p_fAzim<0.0f){
        p_fAzim += 1;
    }
    setXYUsingAzimElev(p_fAzim, p_fElev);
}

SoundSource::~SoundSource(){
}

//------------------------------------------ SETTERS -------------------------------------------------
void SoundSource::setXY(Point <float> p){    //x and y are [-r,r]
    m_fX = p.x;
    m_fY = p.y;
}
void SoundSource::setXY01(float x01, float y01){
    setX01(x01);
    setY01(y01);
}
void SoundSource::setX01(float p_x01){
    if (m_fX == 0 && m_fY ==0){
        m_fAzim01 = getAzimuth01();
    } else {
        m_bPositionWas00 = false;
    }
    m_fX = PercentToHR(p_x01, -ZirkOscAudioProcessor::s_iDomeRadius, ZirkOscAudioProcessor::s_iDomeRadius);
    
}
void SoundSource::setY01(float p_y01){
    if (m_fX == 0 && m_fY ==0){
        m_bPositionWas00 = true;
        m_fAzim01 = getAzimuth01();
    } else {
        m_bPositionWas00 = false;
    }
    m_fY = PercentToHR(p_y01, -ZirkOscAudioProcessor::s_iDomeRadius, ZirkOscAudioProcessor::s_iDomeRadius);
}
void SoundSource::setXYUsingAzimElev(float p_fAzim01, float p_fElev01){
    float HRAzimuth = PercentToHR(p_fAzim01, ZirkOSC_Azim_Min,ZirkOSC_Azim_Max);
    float HRElevation = PercentToHR(p_fElev01, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    m_fX = (- ZirkOscAudioProcessor::s_iDomeRadius * sinf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
    m_fY = (-ZirkOscAudioProcessor::s_iDomeRadius * cosf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
}
//----------------------- AZIM + ELEV
void  SoundSource::setAzimuth01(float azimuth01){
    if (azimuth01>1)
        azimuth01 = azimuth01 - 1.0f;
    else if (azimuth01<0.0f){
        azimuth01 += 1;
    }
    setXYUsingAzimElev(azimuth01, getElevation01());
}
void SoundSource::setElevation01(float elevation01){
    setXYUsingAzimElev(getAzimuth01(), elevation01);
}

//store azimuth value, before it is overwritten by a setXY of (0,0), which will always give an azim of +180
void SoundSource::setLastAzim01(float p_fLastAzim01){
    JUCE_COMPILER_WARNING("THIS NEEDS TO BE IN OTHER SETTER FUNCTION")
    m_fAzim01 = p_fLastAzim01;
}
//-----------------------
void SoundSource::setOldLoc01(const float &p_fX01, const float &p_fY01, const float &p_fOldAzim01){
    m_fOldX01 = p_fX01;
    m_fOldY01 = p_fY01;
    if (p_fOldAzim01 == -1){
        XY01toAzimElev01(m_fOldX01, m_fOldY01, m_fOldAzim01, m_fOldElev01);
    } else {
        m_fOldAzim01 = p_fOldAzim01;
    }
}

//------------------------------------------ GETTERS -------------------------------------------------
float SoundSource::getX01(){
    return HRToPercent(m_fX, -ZirkOscAudioProcessor::s_iDomeRadius, ZirkOscAudioProcessor::s_iDomeRadius);
}
float SoundSource::getY01(){
    return HRToPercent(m_fY, -ZirkOscAudioProcessor::s_iDomeRadius, ZirkOscAudioProcessor::s_iDomeRadius);
}
float   SoundSource::getAzimuth01(){
    if (m_bPositionWas00){
        return m_fAzim01;
    } else {
        return XYtoAzim01(m_fX, m_fY);
    }
}
float   SoundSource::getElevation01(){
    return XYtoElev01(m_fX, m_fY);
}

void SoundSource::getOldXY01(float &p_fX01, float &p_fY01){
    p_fX01 = m_fOldX01;
    p_fY01 = m_fOldY01;
}













//-------------------------- STATIC CONVERSION FUNCTIONS --------------------------
void SoundSource::azimElev01toXY01(const float &p_fAzimuth01, const float &p_fElevation01, float &p_fX, float &p_fY){
    float HRAzimuth   = PercentToHR(p_fAzimuth01,   ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    float HRElevation = PercentToHR(p_fElevation01, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    
    p_fX = -ZirkOscAudioProcessor::s_iDomeRadius * sin(degreeToRadian(HRAzimuth)) * cos(degreeToRadian(HRElevation));
    p_fX = (p_fX + ZirkOscAudioProcessor::s_iDomeRadius) / (2.f*ZirkOscAudioProcessor::s_iDomeRadius);
    p_fY = -ZirkOscAudioProcessor::s_iDomeRadius * cos(degreeToRadian(HRAzimuth)) * cos(degreeToRadian(HRElevation));
    p_fY = (p_fY + ZirkOscAudioProcessor::s_iDomeRadius) / (2.f*ZirkOscAudioProcessor::s_iDomeRadius);
}

void SoundSource::azimElev01toXY01(const float &p_fAzimuth01, const float &p_fElevation01, float &p_fX, float &p_fY, const float& fNewR){
    float HRAzimuth   = PercentToHR(p_fAzimuth01,   ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    float HRElevation = PercentToHR(p_fElevation01, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    
    p_fX = -fNewR * sin(degreeToRadian(HRAzimuth)) * cos(degreeToRadian(HRElevation));
    p_fX = (p_fX + ZirkOscAudioProcessor::s_iDomeRadius) / (2.f*ZirkOscAudioProcessor::s_iDomeRadius);
    p_fY = -fNewR * cos(degreeToRadian(HRAzimuth)) * cos(degreeToRadian(HRElevation));
    p_fY = (p_fY + ZirkOscAudioProcessor::s_iDomeRadius) / (2.f*ZirkOscAudioProcessor::s_iDomeRadius);
}
void SoundSource::azimElev01toXY(const float &p_fAzimuth01, const float &p_fElevation01, float &p_fX, float &p_fY){
    float HRAzimuth   = PercentToHR(p_fAzimuth01,   ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    float HRElevation = PercentToHR(p_fElevation01, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    
    p_fX = -ZirkOscAudioProcessor::s_iDomeRadius * sinf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation));
    p_fY = -ZirkOscAudioProcessor::s_iDomeRadius * cosf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation));
}
//XY are [0,1] and azim+elev are [0,1]
void SoundSource::XY01toAzimElev01(const float &p_fX, const float &p_fY, float &p_fAzim, float &p_fElev){
    float fX = p_fX * 2 * ZirkOscAudioProcessor::s_iDomeRadius - ZirkOscAudioProcessor::s_iDomeRadius;
    float fY = p_fY * 2 * ZirkOscAudioProcessor::s_iDomeRadius - ZirkOscAudioProcessor::s_iDomeRadius;
    p_fAzim = XYtoAzim01(fX, fY);
    p_fElev = XYtoElev01(fX, fY);
}
// azim elev in degrees and xy in range [-r, r]
void SoundSource::azimElevToXy (const float &p_fAzimuth, const float &p_fElevation, float &p_fX, float &p_fY){
    p_fX = -ZirkOscAudioProcessor::s_iDomeRadius * sinf(degreeToRadian(p_fAzimuth)) * cosf(degreeToRadian(p_fElevation));
    p_fY = -ZirkOscAudioProcessor::s_iDomeRadius * cosf(degreeToRadian(p_fAzimuth)) * cosf(degreeToRadian(p_fElevation));
}
//XY are [-r,r] and azim is [0,1]
float SoundSource::XYtoAzim01(const float &p_fX, const float &p_fY){
    float azim;
    if (p_fX > 0){
        azim = atan2f(p_fX, p_fY);
    } else {
        azim = atan2f(-p_fX, p_fY);
        azim = 2*M_PI-azim;
    }
//    if(azim < .002){
//        if (azim > .00001){
//            //in this range, we mean 0
//            return 0;
//        } else {
//            //and in this range, we mean 1 ;)
//            return 1;
//        }
//    } else {
        azim *= 180/M_PI;
        azim /= 360;
        return azim;
//    }
}
float SoundSource::XYtoElev01(const float &p_fX, const float &p_fY){
    double dArg = sqrt(p_fX*p_fX + p_fY*p_fY) / ZirkOscAudioProcessor::s_iDomeRadius;
    if (dArg > 1) {
        dArg =  1.;
    } else if (dArg < .001){
        dArg = 0.;
    }
    float fElevation = static_cast<float>(acos(dArg)) ;
    if (fElevation < 0.001){
        return 0.f;
    } else {
        return (fElevation) / M_PI_2;
    }
}
//clamp x and y to [-r. r]
void SoundSource::clampXY(float &p_fx, float &p_fy){
    p_fx = clamp(p_fx, -static_cast<float>(ZirkOscAudioProcessor::s_iDomeRadius), static_cast<float>(ZirkOscAudioProcessor::s_iDomeRadius));
    p_fy = clamp(p_fy, -static_cast<float>(ZirkOscAudioProcessor::s_iDomeRadius), static_cast<float>(ZirkOscAudioProcessor::s_iDomeRadius));
}