//
//  SoundSource.h
//  ZirkOSCJUCE
//
//  Created by Lud's on 26/02/13.
//
//

#ifndef __ZirkOSCJUCE__SoundSource__
#define __ZirkOSCJUCE__SoundSource__

#include <iostream>
#include "../JuceLibraryCode/JuceHeader.h"
class SoundSource{
public:
    SoundSource();
    SoundSource(float,float);
    ~SoundSource();
    //getter setter;
    Point<float> getPositionXY();
    int     getChannel();
    void    setChannel(int);
    float   getX();
    void    setPositionXY(Point <float>);
    float   getY();
    float   getGain();
    void    setGain(float);
    float   getAzimuth();
    void    setAzimuth(float);
    float   getAzimuthSpan();
    void    setAzimuthSpan(float);
    float   getElevationSpan();
    void    setElevationSpan(float);
    float   getElevation();
    float   getElevationRawValue();
    void    setElevation(float);
    //! returns true if the point is inside the source. Point is relative to the center of the dome
    bool    contains(Point<float>);
    bool    isAzimReverse();
    void    setAzimReverse(bool);
        
private:
    //! If source Elevation is over 90° you have to reverse the azim
    bool _AzimReverse =false;
    //! Source channel id id send to Zirkonium
    int _Channel =0;
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
    //! Convert degreeToRadian
    inline float degreeToRadian (float);
    //! Convert radianToDegree
    inline float radianToDegree (float);
    
};



#endif /* defined(__ZirkOSCJUCE__SoundSource__) */
