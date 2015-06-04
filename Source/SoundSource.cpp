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
//#include "PluginEditor.h"
#include "PluginProcessor.h"


SoundSource::SoundSource(){
    m_bUseXY        = false;
    _AzimReverse    = false;
}

SoundSource::SoundSource(float azimuth, float elevation) : SoundSource(){
  
    setAzimuth(azimuth);
    setElevation(elevation);
    
    float x = getX();
    float y = getY();
    std::cout << "soundSource() x = " << x << ", y = " << y << ", azim = " << azimuth << ", elev = " << elevation << "\n";
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
    return _AzimReverse;
}

bool    SoundSource::contains(Point <float> p){
    return (p.getX()< getX()+5 && p.getX()> getX()-5 && p.getY()< getY()+5 && p.getY()> getY()-5 );
}

Point <float> SoundSource::getPositionXY (){
    return *(new Point <float> (getX(), getY()));
}

bool    SoundSource::isStillInTheDome(Point<float> move){
    Point<float> p = Point<float>(this->getX() + move.getX(), this->getY() + move.getY());
    float dist= sqrt(p.getX()* p.getX() + p.getY()*p.getY());
    return (fabs(dist)< ZirkOscjuceAudioProcessor::s_iDomeRadius);
}

//------------------------------------------------

float SoundSource::getX(){
    
    if (m_bUseXY){
        std::cout << "get m_fX: " << m_fX << "\n";
        return m_fX;
    } else {
        JUCE_COMPILER_WARNING("do we need to do those every single time?")
        float HRAzimuth = PercentToHR(_Azimuth, ZirkOSC_Azim_Min,ZirkOSC_Azim_Max);
        float HRElevation = PercentToHR(_Elevation, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
        
        return (- ZirkOscjuceAudioProcessor::s_iDomeRadius * sinf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
    }
}

float   SoundSource::getY(){
    if (m_bUseXY){
        std::cout << "get m_fY: " << m_fY << "\n";
        return m_fY;
    } else {
        float HRAzimuth = PercentToHR(_Azimuth,ZirkOSC_Azim_Min,ZirkOSC_Azim_Max);
        float HRElevation = PercentToHR(_Elevation, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
        return (-ZirkOscjuceAudioProcessor::s_iDomeRadius * cosf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
    }
}

float   SoundSource::getAzimuth(){
    if (m_bUseXY){
        float azim = atan2f(m_fY, m_fX);
        std::cout << "get azim: " << azim << "\n";
        return azim;
    } else {
        return _Azimuth;
    }
}

float   SoundSource::getAzimuthSpan(){
        return _AzimuthSpan;
}

float   SoundSource::getElevation(){
    if (m_bUseXY){
        float fElevation = acosf( sqrtf( (pow (m_fX,2) + pow(m_fY,2)) / ZirkOscjuceAudioProcessor::s_iDomeRadius));
        if (fElevation < 0.f){
            std::cout << "get elev: " << 0.f << "\n";
            return 0.f;
        } else {
            std::cout << "get elev: " << fElevation << "\n";
            return fElevation;
        }
    } else {
        if (_Elevation<0.0f){
            return 0.0f;
        }
        return _Elevation;
    }
}

float   SoundSource::getElevationRawValue(){
    if (m_bUseXY){
        float ret = acosf( sqrtf( (pow (m_fX,2) + pow(m_fY,2)) / ZirkOscjuceAudioProcessor::s_iDomeRadius));
        std::cout << "get elev raw: " << ret << "\n";
        return ret;
    } else {
        return _Elevation;
    }
}

float   SoundSource::getElevationSpan(){
        return _ElevationSpan;
}







void    SoundSource::setAzimuth(float azimuth){
    
    if (azimuth>1 && !_AzimReverse)
        azimuth = azimuth - 1.0f;
    else if (azimuth<0.0f){
        azimuth += 1;
    }
    
    if (m_bUseXY){
        
        float HRAzimuth = PercentToHR(azimuth, ZirkOSC_Azim_Min,ZirkOSC_Azim_Max);
        float HRElevation = PercentToHR(getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
        
        m_fX = (- ZirkOscjuceAudioProcessor::s_iDomeRadius * sinf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
        m_fY = (-ZirkOscjuceAudioProcessor::s_iDomeRadius * cosf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
        
        std::cout << "set azim: " << m_fX << " - " << m_fY << "\n";
        
    } else {
        this->_Azimuth=azimuth;
    }
}

void    SoundSource::setAzimuthSpan(float azimuth_span){

    this->_AzimuthSpan=azimuth_span;
}

void    SoundSource::setElevation(float elevation){
    
    if (elevation>1 && !_AzimReverse){
        elevation = (1-(elevation-1));
        setAzimuth(_Azimuth - 0.5f);
        _AzimReverse=true;
    }
    else if (elevation>1 && _AzimReverse){
        elevation = (1-(elevation-1));
        setAzimuth(_Azimuth - 0.5f);
        _AzimReverse=false;
    }
    
    if (m_bUseXY){
        float HRAzimuth = PercentToHR(getAzimuth(), ZirkOSC_Azim_Min,ZirkOSC_Azim_Max);
        float HRElevation = PercentToHR(elevation, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
        
        m_fX = (- ZirkOscjuceAudioProcessor::s_iDomeRadius * sinf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
        m_fY = (-ZirkOscjuceAudioProcessor::s_iDomeRadius * cosf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
        
        std::cout << "set elev: " << m_fX << " - " << m_fY << "\n";
    } else {

        this->_Elevation = elevation;
    }
}

void    SoundSource::setElevationSpan(float elevation_span){

    this->_ElevationSpan = elevation_span;
}

void    SoundSource::setPositionXY(Point <float> p){
    if (m_bUseXY){
        m_fX = p.x;
        m_fY = p.y;
        
        std::cout << "set pos XY: " << m_fX << " - " << m_fY << "\n";
    } else {
        float dist = sqrt(p.getX()* p.getX() + p.getY()*p.getY());
        if (fabs(dist)> ZirkOscjuceAudioProcessor::s_iDomeRadius){
            _Elevation = 0.0f;
        }
        else{
            _Elevation = HRToPercent(radianToDegree(acos(dist/ZirkOscjuceAudioProcessor::s_iDomeRadius)), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max) ;
        }
        float HRAzimuth = - radianToDegree(M_PI/2 + atan2(p.getY(),p.getX()));
        if(HRAzimuth< -180){
            HRAzimuth= 360 + HRAzimuth;
        }
        _Azimuth = HRToPercent(HRAzimuth, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    }
}

void    SoundSource::setAzimReverse(bool azimr){
    _AzimReverse = azimr;
}


//-----------------------------------------------------------------------

JUCE_COMPILER_WARNING("These functions should be made static and put in ZirkConstant.h")
inline float SoundSource::degreeToRadian (float degree){
    return ((degree/360.0f)*2*3.1415);
}

inline float SoundSource::radianToDegree(float radian){
    return (radian/(2*3.1415)*360.0f);
}
