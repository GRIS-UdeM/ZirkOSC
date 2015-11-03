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

#ifndef __ZirkOSCJUCE__SoundSource__
#define __ZirkOSCJUCE__SoundSource__

#include <iostream>
#include "../JuceLibraryCode/JuceHeader.h"
#include "ZirkConstants.h"
class SoundSource{
public:
    SoundSource();
    SoundSource(float,float, int p_iSrcId);
    void    initAzimuthAndElevation(float p_fAzim, float p_fElev);
    ~SoundSource();
    //POSITION FUNCTIONS
    void    setXY(Point <float>);       //set x and y, both are [-r,r]
    void    getXY(float &fX, float &fY);
    void    setX01(float x);
    void    setY01(float y);
    float   getX01();
    float   getY01();
    void    setXYAzimElev01(const float &x01, const float &y01, const float &p_fAzim01 = -1, const float &p_fElev01 = -1);
    float   getAzimuth01();
    void    setAzimuth01(float);
    float   getElevation01();
    void    setElevation01(float);
    void    setPrevLoc01(const float &p_fX01, const float &p_fY01, const float &p_fPrevAzim01 = -1, const float &p_fPrevElev01 = -1);
    void    getPrevXY01(float &p_fX01, float &p_fY01);
    float   getPrevAzim01();
    float   getPrevElev01();
    void    updateAzimElev();
    void    updatePosition();
    
    //TRIVIAL SETTERS AND GETTERS
    float getGain01(){
        return m_fGain;
    }
    void setGain01(float gain){
        m_fGain=gain;
    }
    int getSourceId(){
        return m_iSourceId;
    }
    void setSourceId(int iSourceId){
        m_iSourceId = iSourceId;
    }
    void setElevationStatus(ElevationStatus status){
        m_iElevationStatus = status;
    }
    ElevationStatus getElevationStatus(){
        return m_iElevationStatus;
    }
    void setElevOverflow(const float &p_fElevOverflow){
        m_fElevOverflow = p_fElevOverflow;
    }
    float getElevOverflow(){
        return m_fElevOverflow;
    }
    float getAzimuthSpan(){
        return m_fAzimuthSpan;
    }
    
    float getElevationSpan(){
        return m_fElevationSpan;
    }
    void setAzimuthSpan(float azimuth_span){
        m_fAzimuthSpan=azimuth_span;
    }
    void setElevationSpan(float elevation_span){
        m_fElevationSpan = elevation_span;
    }
    bool contains(Point <float> p){
        return (p.getX()< getX()+10 && p.getX()> getX()-10 && p.getY()< getY()+10 && p.getY()> getY()-10 );
    }
    //returned x is [-r,r]
    float getX(){
        return m_fX;
    }
    //returned y is [-r,r]
    float getY(){
        return m_fY;
    }

    //STATIC CONVERTION FONCTIONS
    static float XYtoAzim01(const float &x, const float &y);
    static float XYtoElev01(const float &x, const float &y);
    static void XY01toAzimElev01(const float &x, const float &y, float  &azim, float &elev);
    static void azimElev01toXY01(const float &p_fAzim, const float &p_fElev, float &p_fX, float &p_fY);
    static void azimElev01toXY01(const float &p_fAzimuth01, const float &p_fElevation01, float &p_fX, float &p_fY, const float& fNewR);
    static void azimElev01toXY(const float &p_fAzim, const float &p_fElev, float &p_fX, float &p_fY);
    static void azimElevToXy (const float &p_fAzimuth, const float &p_fElevation, float &p_fX, float &p_fY);
    static void clampXY(float &x, float &y);
private:
    int   m_iSourceId;         //! Source id sent to Zirkonium
    float m_fGain;
    float m_fAzimuthSpan;
    float m_fElevationSpan;
    //position parameters (x,y and azim,elev are redundant with each other)
    float m_fX;
    float m_fY;
    float m_fAzim01;
    float m_fElev01;
    void  setAzim01SanityCheck(float p_fAzim01);
    void  setElev01SanityCheck(float p_fElev01);
    //old position parameters, for calculating deltas
    float m_fPrevX01;
    float m_fPrevY01;
    float m_fPrevAzim01;
    float m_fPrevElev01;
    
    ElevationStatus  m_iElevationStatus;
    float   m_fElevOverflow;
    bool    m_bElevationWasMaxed;
    
    void setXYUsingAzimElev01(float azim01, float elev01);
};



#endif /* defined(__ZirkOSCJUCE__SoundSource__) */
