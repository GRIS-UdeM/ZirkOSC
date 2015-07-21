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


#include "SoundSource.h"
#include "ZirkConstants.h"
#include "PluginProcessor.h"


SoundSource::SoundSource(){
    m_bIsAzimReversed    = false;
}

SoundSource::SoundSource(float azimuth, float elevation) : SoundSource(){
    
    initAzimuthAndElevation(azimuth,elevation);
    
}

SoundSource::~SoundSource(){
    
}

float   SoundSource::getGain(){
    return _Gain;
}

void    SoundSource::setGain(float gain){
    _Gain=gain;
}

int   SoundSource::getChannel(){
    return _Channel;
}

void    SoundSource::setChannel(int channel){
    this->_Channel = channel;
}

bool    SoundSource::isAzimReverse(){
    return m_bIsAzimReversed;
}

void    SoundSource::setAzimReverse(bool azimr){
    m_bIsAzimReversed = azimr;
}

bool    SoundSource::contains(Point <float> p){
    return (p.getX()< getX()+5 && p.getX()> getX()-5 && p.getY()< getY()+5 && p.getY()> getY()-5 );
}

//range for both fX and fY is [-r,r]
void SoundSource::getXY(float &fX, float &fY){
    fX = getX();
    fY = getY();
}


bool    SoundSource::isStillInTheDome(Point<float> move){
    Point<float> p = Point<float>(this->getX() + move.getX(), this->getY() + move.getY());
    float dist= sqrt(p.getX()* p.getX() + p.getY()*p.getY());
    return (fabs(dist)< ZirkOscjuceAudioProcessor::s_iDomeRadius);
}

float   SoundSource::getAzimuthSpan(){
    return _AzimuthSpan;
}

float   SoundSource::getElevationSpan(){
    return _ElevationSpan;
}

void    SoundSource::setAzimuthSpan(float azimuth_span){
    
    this->_AzimuthSpan=azimuth_span;
}

void    SoundSource::setElevationSpan(float elevation_span){
    
    this->_ElevationSpan = elevation_span;
}



//------------------------------------------------
//returned x is [-r,r]
float SoundSource::getX(){
    
    
    return m_fX;
    
}

//returned y is [-r,r]
float SoundSource::getY(){
    
    return m_fY;
    
}

float SoundSource::getX01(){
    return HRToPercent(m_fX, -ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius);
}

float SoundSource::getY01(){
    return HRToPercent(m_fY, -ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius);
}

//------------------------------------------------
//x and y are [-r,r]
void    SoundSource::setXY(Point <float> p){
    
    m_fX = p.x;
    m_fY = p.y;
    
}

void SoundSource::setXYUsingAzimElev(float p_fAzim01, float p_fElev01){
    float HRAzimuth = PercentToHR(p_fAzim01, ZirkOSC_Azim_Min,ZirkOSC_Azim_Max);
    float HRElevation = PercentToHR(p_fElev01, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    
    m_fX = (- ZirkOscjuceAudioProcessor::s_iDomeRadius * sinf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
    m_fY = (-ZirkOscjuceAudioProcessor::s_iDomeRadius * cosf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
}

void SoundSource::setX01(float p_x){
    m_fX = PercentToHR(p_x, -ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius);
}

void SoundSource::setY01(float p_y){
    m_fY = PercentToHR(p_y, -ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius);
}

//------------------------------------------------
//azimuth range [0,1]
float   SoundSource::getAzimuth(){
    
    return XYtoAzim01(m_fX, m_fY);
}

//elevation range [0,1]
float   SoundSource::getElevation(){
    return XYtoElev01(m_fX, m_fY);
}

float   SoundSource::getElevationRawValue(){
    
    double dArg = sqrt( m_fX*m_fX + m_fY*m_fY) / ZirkOscjuceAudioProcessor::s_iDomeRadius;
    if (dArg > 1) {
        dArg =  1.;
    }
    float ret = static_cast<float>( acos(dArg));
    return ret;
}


//----------------------------
void    SoundSource::initAzimuthAndElevation(float p_fAzim, float p_fElev){
    if (p_fAzim>1 && !m_bIsAzimReversed)
        p_fAzim = p_fAzim - 1.0f;
    else if (p_fAzim<0.0f){
        p_fAzim += 1;
    }
    setXYUsingAzimElev(p_fAzim, p_fElev);
}


void  SoundSource::setAzimuth(float azimuth01){
    
    if (azimuth01>1 && !m_bIsAzimReversed)
        azimuth01 = azimuth01 - 1.0f;
    else if (azimuth01<0.0f){
        azimuth01 += 1;
    }
    
    setXYUsingAzimElev(azimuth01, getElevation());
}

void SoundSource::setElevation(float elevation01){
    //check if we need to reverse the azimuth
    if (elevation01>1 && !m_bIsAzimReversed){
        elevation01 = (1-(elevation01-1));
        setAzimuth(_Azimuth - 0.5f);
        m_bIsAzimReversed=true;
    }
    else if (elevation01>1 && m_bIsAzimReversed){
        elevation01 = (1-(elevation01-1));
        setAzimuth(_Azimuth - 0.5f);
        m_bIsAzimReversed=false;
    }
    
    setXYUsingAzimElev(getAzimuth(), elevation01);
}

//--------------------------
//STATIC CONVERSION FUNCTIONS
void SoundSource::azimElev01toXY01(const float &p_fAzimuth01, const float &p_fElevation01, float &p_fX, float &p_fY){
    
    float HRAzimuth   = PercentToHR(p_fAzimuth01,   ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    float HRElevation = PercentToHR(p_fElevation01, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    
    p_fX = -ZirkOscjuceAudioProcessor::s_iDomeRadius * sin(degreeToRadian(HRAzimuth)) * cos(degreeToRadian(HRElevation));
    p_fX = (p_fX + ZirkOscjuceAudioProcessor::s_iDomeRadius) / (2.f*ZirkOscjuceAudioProcessor::s_iDomeRadius);
    p_fY = -ZirkOscjuceAudioProcessor::s_iDomeRadius * cos(degreeToRadian(HRAzimuth)) * cos(degreeToRadian(HRElevation));
    p_fY = (p_fY + ZirkOscjuceAudioProcessor::s_iDomeRadius) / (2.f*ZirkOscjuceAudioProcessor::s_iDomeRadius);
}

void SoundSource::azimElev01toXY01(const float &p_fAzimuth01, const float &p_fElevation01, float &p_fX, float &p_fY, const float& fNewR){
    
    float HRAzimuth   = PercentToHR(p_fAzimuth01,   ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    float HRElevation = PercentToHR(p_fElevation01, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    
    p_fX = -fNewR * sin(degreeToRadian(HRAzimuth)) * cos(degreeToRadian(HRElevation));
    p_fX = (p_fX + ZirkOscjuceAudioProcessor::s_iDomeRadius) / (2.f*ZirkOscjuceAudioProcessor::s_iDomeRadius);
    p_fY = -fNewR * cos(degreeToRadian(HRAzimuth)) * cos(degreeToRadian(HRElevation));
    p_fY = (p_fY + ZirkOscjuceAudioProcessor::s_iDomeRadius) / (2.f*ZirkOscjuceAudioProcessor::s_iDomeRadius);
}

void SoundSource::azimElev01toXY(const float &p_fAzimuth01, const float &p_fElevation01, float &p_fX, float &p_fY){
    float HRAzimuth   = PercentToHR(p_fAzimuth01,   ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    float HRElevation = PercentToHR(p_fElevation01, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    
    p_fX = -ZirkOscjuceAudioProcessor::s_iDomeRadius * sinf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation));
    p_fY = -ZirkOscjuceAudioProcessor::s_iDomeRadius * cosf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation));
}

//XY are [0,1] and azim+elev are [0,1]
void SoundSource::XY01toAzimElev01(const float &p_fX, const float &p_fY, float &p_fAzim, float &p_fElev){
    float fX = p_fX * 2 * ZirkOscjuceAudioProcessor::s_iDomeRadius - ZirkOscjuceAudioProcessor::s_iDomeRadius;
    float fY = p_fY * 2 * ZirkOscjuceAudioProcessor::s_iDomeRadius - ZirkOscjuceAudioProcessor::s_iDomeRadius;
    p_fAzim = XYtoAzim01(fX, fY);
    p_fElev = XYtoElev01(fX, fY);
}

// azim elev in degrees and xy in range [-r, r]
void SoundSource::azimElevToXy (const float &p_fAzimuth, const float &p_fElevation, float &p_fX, float &p_fY){
    p_fX = -ZirkOscjuceAudioProcessor::s_iDomeRadius * sinf(degreeToRadian(p_fAzimuth)) * cosf(degreeToRadian(p_fElevation));
    p_fY = -ZirkOscjuceAudioProcessor::s_iDomeRadius * cosf(degreeToRadian(p_fAzimuth)) * cosf(degreeToRadian(p_fElevation));
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
    
    azim *= 180/M_PI;
    azim /= 360;
    return azim;
}

float SoundSource::XYtoElev01(const float &p_fX, const float &p_fY){
    double dArg = sqrt(p_fX*p_fX + p_fY*p_fY) / ZirkOscjuceAudioProcessor::s_iDomeRadius;
    
    if (dArg > 1) {
        dArg =  1.;
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
    p_fx = clamp(p_fx, -static_cast<float>(ZirkOscjuceAudioProcessor::s_iDomeRadius), static_cast<float>(ZirkOscjuceAudioProcessor::s_iDomeRadius));
    p_fy = clamp(p_fy, -static_cast<float>(ZirkOscjuceAudioProcessor::s_iDomeRadius), static_cast<float>(ZirkOscjuceAudioProcessor::s_iDomeRadius));
}
