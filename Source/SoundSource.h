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
    float   getAzimuth_span();
    void    setAzimuth_span(float);
    float   getElevation_span();
    void    setElevation_span(float);
    float   getElevation();
    float   getElevationRawValue();
    void    setElevation(float);
    bool    azim_reverse =false;
    bool    contains(Point<float>);
    bool    beginGesture =false;
    
private:
    Point<float> spherePosition; //x = azimuth; y = elevation
    int channel =0;
    float gain=1, azimuth=0, elevation=0, azimuth_span=0, elevation_span=0;
    //Point <float> domeToScreen (Point <float> p);
    inline float degreeToRadian (float );
    inline float radianToDegree (float);
    
};



#endif /* defined(__ZirkOSCJUCE__SoundSource__) */
