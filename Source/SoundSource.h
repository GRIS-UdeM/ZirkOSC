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
    SoundSource(float,float);
    ~SoundSource();
    
    void getXY(float &fX, float &fY);
    //! returns the channel (id in the Zirkonium)
    int     getSourceId();
    //! sets the channel (id in the Zirkonium)
    void    setSourceId(int);
    //! returns the X position, range [-r,r]
    float   getX();
    //! retunrs the Y position, range [-r,r]
    float   getY();
    
    //set x and y, both are [-r,r]
    void    setXY(Point <float>);
    
    void setXYUsingAzimElev(float azim01, float elev01);
    
    //set X (range [-r,r]) using parameter x in percent, ie,  [0,1]
    void setX01(float x);
    
    void setY01(float y);
    
    void setXY01(float x, float y);
    
    float getX01();
    
    float getY01();
    
    static float XYtoAzim01(const float &x, const float &y);

    static float XYtoElev01(const float &x, const float &y);
    
    static void XY01toAzimElev01(const float &x, const float &y, float  &azim, float &elev);
    
    static void azimElev01toXY01(const float &p_fAzim, const float &p_fElev, float &p_fX, float &p_fY);
    
    static void azimElev01toXY01(const float &p_fAzimuth01, const float &p_fElevation01, float &p_fX, float &p_fY, const float& fNewR);

    static void azimElev01toXY(const float &p_fAzim, const float &p_fElev, float &p_fX, float &p_fY);
    
    static void azimElevToXy (const float &p_fAzimuth, const float &p_fElevation, float &p_fX, float &p_fY);

    static void clampXY(float &x, float &y);
    
    
    void    initAzimuthAndElevation(float p_fAzim, float p_fElev);
    //! returns the gain [0,1]
    float   getGain();
    //! sets the gain 
    void    setGain(float);
    //! gets the Azimuth [0,1]
    float   getAzimuth01();
    //! sets the azimuth
    void    setAzimuth01(float);
    //! returns the azimuth span
    float   getAzimuthSpan();
    //! sets the azimuth span
    void    setAzimuthSpan(float);
    //! returns the elevation span
    float   getElevationSpan();
    //! sets the elevation span
    void    setElevationSpan(float);
    //! returns the elevation [0,1]
    float   getElevation01();
    //! returns the elevation [-1,1] from memory.
    float   getElevationRawValue();
    //! set the elevation 
    void    setElevation01(float);
    //! returns true if the point is inside the source. Point is relative to the center of the dome
    bool    contains(Point<float>);
    //! returns true if the azimuth has been reversed (elevation >1)
    bool    isAzimReverse();
    //! set true if the azimuth will be reversed
    void    setAzimReverse(bool);
    //! Check if the movement lets the source in the dome
    bool isStillInTheDome( Point<float> move);
   
    void setOldLoc01(const float &p_fX01, const float &p_fY01, const float &p_fOldAzim01 = -1){
        m_fOldX01 = p_fX01;
        m_fOldY01 = p_fY01;
        if (p_fOldAzim01 == -1){
            XY01toAzimElev01(m_fOldX01, m_fOldY01, m_fOldAzim01, m_fOldElev01);
        } else {
            m_fOldAzim01 = p_fOldAzim01;
        }
    }
    
    void getOldXY01(float &p_fX01, float &p_fY01){
        p_fX01 = m_fOldX01;
        p_fY01 = m_fOldY01;
    }
    
    float getOldAzim01(){
        return m_fOldAzim01;
    }
    
    void setElevationStatus(ElevationStatus status){
        m_iElevationStatus = status;
    }
    
    ElevationStatus getElevationStatus(){
        return m_iElevationStatus;
    }

        
private:

    float m_fX;
    
    float m_fY;
    
    //! If source Elevation is over 90° you have to reverse the azim
    bool m_bIsAzimReversed;
    //! Source channel id id send to Zirkonium
    int m_iSourceId =1;
    //! Gain parameter stored in percent (see HRToPercent function).
    float _Gain=1;
    //! Azimuth parameter stored in percent (see HRToPercent function).
    float _Azimuth=0;
    //! Elevation parameter stored in percent (see HRToPercent function).
    float _Elevation=0;
    //! Azimuth Span parameter stored in percent (see HRToPercent function).
    float _AzimuthSpan=0;
    //! Elevation Span parameter stored in percent (see HRToPercent function).
    float _ElevationSpan=0;

    bool m_bElevationWasMaxed;
    
    float m_fOldX01;
    float m_fOldY01;
    float m_fOldAzim01;
    float m_fOldElev01;
    ElevationStatus  m_iElevationStatus;

    
//    bool m_bPositionWas00;
//    
//    float m_fLastAzim;
    
};



#endif /* defined(__ZirkOSCJUCE__SoundSource__) */
