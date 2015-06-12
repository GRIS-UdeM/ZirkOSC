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
    _AzimReverse    = false;
}

SoundSource::SoundSource(float azimuth, float elevation) : SoundSource(){
  
    if (g_bUseXY){
        initAzimuthAndElevation(azimuth,elevation);
    } else {
        setAzimuth(azimuth);
        setElevation(elevation);
    }
    
//    float x = getX();
//    float y = getY();
//    float azim = getAzimuth();
//    float elev = getElevation();
//    std::cout << "soundSource(), XY =" << g_bUseXY << ", r = " << ZirkOscjuceAudioProcessor::s_iDomeRadius << ", x = " << x << ", y = " << y << ", azim = " << azim << ", elev = " << elev << "\n";
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
    
    if (g_bUseXY){
        //std::cout << "get m_fX: " << m_fX << "\n";
        return m_fX;
    } else {
        float HRAzimuth = PercentToHR(_Azimuth, ZirkOSC_Azim_Min,ZirkOSC_Azim_Max);
        float HRElevation = PercentToHR(_Elevation, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
        
        return (- ZirkOscjuceAudioProcessor::s_iDomeRadius * sinf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
    }
}

float SoundSource::getY(){
    if (g_bUseXY){
        //std::cout << "get m_fY: " << m_fY << "\n";
        //convert
        return m_fY;
    } else {
        float HRAzimuth = PercentToHR(_Azimuth,ZirkOSC_Azim_Min,ZirkOSC_Azim_Max);
        float HRElevation = PercentToHR(_Elevation, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
        return (-ZirkOscjuceAudioProcessor::s_iDomeRadius * cosf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
    }
}

float SoundSource::getX01(){
    return HRToPercent(m_fX, -ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius);
}

float SoundSource::getY01(){
    return HRToPercent(m_fY, -ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius);
}

void SoundSource::setX01(float p_x){
    m_fX = PercentToHR(p_x, -ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius);
}

void SoundSource::setY01(float p_y){
    m_fY = PercentToHR(p_y, -ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius);
}

float   SoundSource::getAzimuth(){
    if (g_bUseXY){
        JUCE_COMPILER_WARNING("should probably do something similar for other things in here")
        return XYtoAzim(m_fX, m_fY);
    } else {
        return _Azimuth;
    }
}
//this func is static, XY are [-r,r] and azim is [0,1]
float SoundSource::XYtoAzim(const float &p_fX, const float &p_fY){
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

float   SoundSource::getAzimuthSpan(){
        return _AzimuthSpan;
}

float   SoundSource::getElevation(){
    if (g_bUseXY){
        return XYtoElev(m_fX, m_fY);
    } else {
        //std::cout << "getElevation: " << _Elevation << "\n";
        if (_Elevation<0.0f){
            return 0.0f;
        }
        return _Elevation;
    }
}

float SoundSource::XYtoElev(const float &p_fX, const float &p_fY){
    double dArg = sqrt(p_fX*p_fX + p_fY*p_fY) / ZirkOscjuceAudioProcessor::s_iDomeRadius;
    if (dArg > 1) {
        std::cout << "XYtoElev() adjusted dArg from " << dArg << " to 1.\n";
        dArg =  1.;
    }
    
    float fElevation = static_cast<float>(acos(dArg)) ;
    if (fElevation < 0.001){
        return 0.f;
    } else {
        //output of acos is [0,pi/2], so we need to normalize that to [0,1]
        return fElevation / M_PI_2;
    }
}

float   SoundSource::getElevationRawValue(){
    if (g_bUseXY){
        double dArg = sqrt( m_fX*m_fX + m_fY*m_fY) / ZirkOscjuceAudioProcessor::s_iDomeRadius;
        if (dArg > 1) {
            std::cout << "getElevationRawValue() adjusted dArg from " << dArg << " to 1.\n";
            dArg =  1.;
        }
        float ret = static_cast<float>( acos(dArg));
        //std::cout << "get elev raw: " << ret << "\n";
        return ret;
    } else {
        return _Elevation;
    }
}

float   SoundSource::getElevationSpan(){
        return _ElevationSpan;
}



void    SoundSource::initAzimuthAndElevation(float p_fAzim, float p_fElev){
    
    JUCE_COMPILER_WARNING("this will cause problems , we can't call setAzimuth in here");
//    if (p_fElev>1 && !_AzimReverse){
//        p_fElev = (1-(p_fElev-1));
//        jassert(0);
//        setAzimuth(_Azimuth - 0.5f);
//        _AzimReverse=true;
//    }
//    else if (p_fElev>1 && _AzimReverse){
//        p_fElev = (1-(p_fElev-1));
//        jassert(0);
//        setAzimuth(_Azimuth - 0.5f);
//        _AzimReverse=false;
//    }
    
    if (p_fAzim>1 && !_AzimReverse)
        p_fAzim = p_fAzim - 1.0f;
    else if (p_fAzim<0.0f){
        p_fAzim += 1;
    }
    
    float HRAzimuth = PercentToHR(p_fAzim, ZirkOSC_Azim_Min,ZirkOSC_Azim_Max);
    float HRElevation = PercentToHR(p_fElev, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    
    m_fX = (- ZirkOscjuceAudioProcessor::s_iDomeRadius * sinf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
    m_fY = (-ZirkOscjuceAudioProcessor::s_iDomeRadius * cosf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
    
    //std::cout << "set azim and elev, x = " << m_fX << ", y = " << m_fY << "\n";
}



void    SoundSource::setAzimuth(float azimuth){
    
    if (azimuth>1 && !_AzimReverse)
        azimuth = azimuth - 1.0f;
    else if (azimuth<0.0f){
        azimuth += 1;
    }
    
    if (g_bUseXY){
        
        float HRAzimuth = PercentToHR(azimuth, ZirkOSC_Azim_Min,ZirkOSC_Azim_Max);
        float HRElevation = PercentToHR(getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
        
        m_fX = (- ZirkOscjuceAudioProcessor::s_iDomeRadius * sinf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
        m_fY = (-ZirkOscjuceAudioProcessor::s_iDomeRadius * cosf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
        
        //std::cout << "set azim: " << m_fX << " - " << m_fY << "\n";
        
    } else {
        this->_Azimuth=azimuth;
    }
}



void    SoundSource::setAzimuthSpan(float azimuth_span){

    this->_AzimuthSpan=azimuth_span;
}

void    SoundSource::setElevation(float elevation){
    
    if (elevation>1 && !_AzimReverse){//ok, this part is important, that's when we realize that we need to inverse the azimuth
        elevation = (1-(elevation-1));
        setAzimuth(_Azimuth - 0.5f);
        _AzimReverse=true;
    }
    else if (elevation>1 && _AzimReverse){
        elevation = (1-(elevation-1));
        setAzimuth(_Azimuth - 0.5f);
        _AzimReverse=false;
    }
    
    if (g_bUseXY){
        float HRAzimuth = PercentToHR(getAzimuth(), ZirkOSC_Azim_Min,ZirkOSC_Azim_Max);
        float HRElevation = PercentToHR(elevation, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
        m_fX = (- ZirkOscjuceAudioProcessor::s_iDomeRadius * sinf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
        m_fY = (-ZirkOscjuceAudioProcessor::s_iDomeRadius * cosf(degreeToRadian(HRAzimuth)) * cosf(degreeToRadian(HRElevation)));
        
        //std::cout << "set elev: " << m_fX << " - " << m_fY << "\n";
    } else {

        this->_Elevation = elevation;
    }
}

void    SoundSource::setElevationSpan(float elevation_span){

    this->_ElevationSpan = elevation_span;
}

void    SoundSource::setPositionXY(Point <float> p){
    if (g_bUseXY){
        m_fX = p.x;
        m_fY = p.y;
    } else {
        float dist = sqrt(p.getX()* p.getX() + p.getY()*p.getY());
        if (fabs(dist)> ZirkOscjuceAudioProcessor::s_iDomeRadius){
            _Elevation = 0.0f;
        }   else{
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

