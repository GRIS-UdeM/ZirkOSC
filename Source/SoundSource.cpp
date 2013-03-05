//
//  SoundSource.cpp
//  ZirkOSCJUCE
//
//  Created by Lud's on 26/02/13.
//
//

#include "SoundSource.h"
#include "ZirkConstants.h"
#include "PluginEditor.h"


SoundSource::SoundSource(){

}

SoundSource::SoundSource(float azimuth, float elevation){
    SoundSource();
    this->azimuth = azimuth;
    this->elevation = elevation;
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
float   SoundSource::getX(){
    float HRAzimuth = PercentToHR(azimuth,ZirkOSC_Azim_Min,ZirkOSC_Azim_Max);
    float HRElevation = PercentToHR(elevation, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    return (-ZirkOSC_DomeRadius * sinf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
}

void    SoundSource::setPositionXY(Point <float> p){
    float dist= sqrt(p.getX()* p.getX() + p.getY()*p.getY());
    if (fabs(dist)> ZirkOSC_DomeRadius){
        elevation = 0.0f;
    }
    else{
        elevation = HRToPercent(radianToDegree(acos(dist/ZirkOSC_DomeRadius)), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max) ;
    }
    float HRAzimuth = - radianToDegree(M_PI/2 + atan2(p.getY(),p.getX()));
    if(HRAzimuth< -180){
        HRAzimuth= 360 +HRAzimuth;
    }
    azimuth = HRToPercent(HRAzimuth, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);

}

float   SoundSource::getY(){
    float HRAzimuth = PercentToHR(azimuth,ZirkOSC_Azim_Min,ZirkOSC_Azim_Max);
    float HRElevation = PercentToHR(elevation, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    return (-ZirkOSC_DomeRadius * cosf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
}

float   SoundSource::getGain(){
    return gain;
}

void    SoundSource::setGain(float gain){
    this->gain=gain;
}

float   SoundSource::getAzimuth(){
    return azimuth;
}

void    SoundSource::setAzimuth(float azimuth){
    this->azimuth=azimuth;
}

float   SoundSource::getAzimuth_span(){
    return azimuth_span;
}

void    SoundSource::setAzimuth_span(float azimuth_span){
    this->azimuth_span=azimuth_span;
}

float   SoundSource::getElevation_span(){
    return elevation_span;
}

void    SoundSource::setElevation_span(float elevation_span){
    this->elevation_span = elevation_span;
}

float   SoundSource::getElevation(){
    return elevation;
}

void    SoundSource::setElevation(float elevation){
    this->elevation = elevation;
}

int   SoundSource::getChannel(){
    return channel;
}

void    SoundSource::setChannel(int channel){
    this->channel = channel;
}
