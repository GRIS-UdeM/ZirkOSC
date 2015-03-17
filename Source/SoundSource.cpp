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

}

SoundSource::SoundSource(float azimuth, float elevation){
    SoundSource();
    this->_Azimuth = azimuth;
    this->_Elevation = elevation;
}

SoundSource::~SoundSource(){

}

// converters
Point <float> SoundSource::getPositionXY (){
    return *(new Point <float> (getX(), getY()));
}

inline float SoundSource::degreeToRadian (float degree){
    return ((degree/360.0f)*2*3.1415);
}

inline float SoundSource::radianToDegree(float radian){
    return (radian/(2*3.1415)*360.0f);
}

bool    SoundSource::contains(Point <float> p){
    return (p.getX()< getX()+5 && p.getX()> getX()-5 && p.getY()< getY()+5 && p.getY()> getY()-5 );
}


//getter setter;
float SoundSource::getX(){
    float HRAzimuth = PercentToHR(_Azimuth, ZirkOSC_Azim_Min,ZirkOSC_Azim_Max);
    float HRElevation = PercentToHR(_Elevation, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    
    return (- ZirkOscjuceAudioProcessor::s_iDomeRadius * sinf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
}

void    SoundSource::setPositionXY(Point <float> p){
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

float   SoundSource::getY(){
    float HRAzimuth = PercentToHR(_Azimuth,ZirkOSC_Azim_Min,ZirkOSC_Azim_Max);
    float HRElevation = PercentToHR(_Elevation, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    return (-ZirkOscjuceAudioProcessor::s_iDomeRadius * cosf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
}

float   SoundSource::getGain(){
    return _Gain;
}

void    SoundSource::setGain(float gain){
    _Gain=gain;
}

bool    SoundSource::isStillInTheDome(Point<float> move){
    Point<float> p = Point<float>(this->getX() + move.getX(), this->getY() + move.getY());
    float dist= sqrt(p.getX()* p.getX() + p.getY()*p.getY());
    return (fabs(dist)< ZirkOscjuceAudioProcessor::s_iDomeRadius);
}

float   SoundSource::getAzimuth(){
    return _Azimuth;
}


//
void    SoundSource::setAzimuth(float azimuth){
    if (azimuth>1 && !_AzimReverse)
        azimuth = azimuth - 1.0f;
    else if (azimuth<0.0f){
        azimuth += 1;
    }
    this->_Azimuth=azimuth;
}

float   SoundSource::getAzimuthSpan(){
    return _AzimuthSpan;
}

void    SoundSource::setAzimuthSpan(float azimuth_span){
    this->_AzimuthSpan=azimuth_span;
}

float   SoundSource::getElevationSpan(){
    return _ElevationSpan;
}

void    SoundSource::setElevationSpan(float elevation_span){
    
    this->_ElevationSpan = elevation_span;
}

float   SoundSource::getElevation(){
    if (_Elevation<0.0f){
        return 0.0f;
    }
    else return _Elevation;
}

float   SoundSource::getElevationRawValue(){
    return _Elevation;
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
    this->_Elevation = elevation;
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
void    SoundSource::setAzimReverse(bool azimr){
    _AzimReverse = azimr;
}