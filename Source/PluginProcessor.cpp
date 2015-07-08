/*
 ==============================================================================
 ZirkOSC2: VST and AU audio plug-in enabling spatial movement of sound sources in a dome of speakers.
 
 Copyright (C) 2015  GRIS-UdeM
 
 Developers: Ludovic Laffineur, Vincent Berthiaume, Antoine Landrieu
 
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
 
 Description :
 
Handling all the sound processing and directing.
 */

#ifndef DEBUG
#define DEBUG
#endif
#undef DEBUG

//lo_send(mOsc, "/pan/az", "i", ch);

#include "PluginEditor.h"
#include "ZirkConstants.h"
#include <string.h>
#include <sstream>
#include <regex.h>
#include <arpa/inet.h>  //for inet_pton

// using stringstream constructors.
#include <iostream>


using namespace std;

int ZirkOscjuceAudioProcessor::s_iDomeRadius = 172;

bool ZirkOscjuceAudioProcessor::s_bUseXY = true;

bool s_bSourceUnique = false;

ZirkOscjuceAudioProcessor::ZirkOscjuceAudioProcessor()
:
_NbrSources(1)
,_SelectedMovementConstraint(.0f)
,m_iSelectedMovementConstraint(Independant)
,_SelectedTrajectory(.0f)
,m_fSelectedTrajectoryDirection(.0f)
,m_fSelectedTrajectoryReturn(.0f)
,_SelectedSource(0)
,_OscPortZirkonium(18032)
,_isOscActive(true)
,_isSpanLinked(true)
,_TrajectoryCount(0)
,_TrajectoriesDuration(0)
//,_TrajectoriesPhiAsin(0)
//,_TrajectoriesPhiAcos(0)
,_isSyncWTempo(false)
,_isWriteTrajectory(false)
,_SelectedSourceForTrajectory(0)\
,m_iSourceLocationChanged(-1)
,m_fSourceLocationChangedX(-1.f)
,m_fSourceLocationChangedY(-1.f)
{
    
    initSources();

    _OscZirkonium   = lo_address_new("127.0.0.1", "10001");
    
    //default values for ui dimensions
    _LastUiWidth = ZirkOSC_Window_Default_Width;
    _LastUiHeight = ZirkOSC_Window_Default_Height;
    
    startTimer (50);
}



void ZirkOscjuceAudioProcessor::initSources(){
    for(int i=0; i<8; ++i){
        _AllSources[i]=SoundSource(0.0+((float)i/8.0),0.0);
    }
}


void error(int num, const char *m, const char *path){
    printf("liblo server error %d in path %s: %s\n", num, path, m);
    fflush(stdout);
}

void ZirkOscjuceAudioProcessor::timerCallback(){
    const MessageManagerLock mmLock;
    if (s_bSourceUnique && m_fSourceLocationChangedX != -1  && m_fSourceLocationChangedY != -1){
        JUCE_COMPILER_WARNING("radius flag needs to be set to something sensible")
        moveCircular(m_iSourceLocationChanged, m_fSourceLocationChangedX, m_fSourceLocationChangedY, false);
        m_fSourceLocationChangedX = -1;
    }
    sendOSCValues();
}


ZirkOscjuceAudioProcessor::~ZirkOscjuceAudioProcessor()
{
    lo_address osc = _OscZirkonium;
    if (osc){
        lo_address_free(osc);
    }
    _OscZirkonium = NULL;
}

void ZirkOscjuceAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    double sampleRate = getSampleRate();
    unsigned int oriFramesToProcess = buffer.getNumSamples();
    
    Trajectory::Ptr trajectory = mTrajectory;
    if (trajectory)
    {
        AudioPlayHead::CurrentPositionInfo cpi;
        getPlayHead()->getCurrentPosition(cpi);
        
        if (cpi.isPlaying && cpi.timeInSamples != mLastTimeInSamples)
        {
            // we're playing!
            mLastTimeInSamples = cpi.timeInSamples;
            
            double bps = cpi.bpm / 60;
            float seconds = oriFramesToProcess / sampleRate;
            float beats = seconds * bps;
            
            bool done = trajectory->process(seconds, beats);
            if (done){
                mTrajectory = NULL;
                _isWriteTrajectory = false;
            }
        }
    }

}

void ZirkOscjuceAudioProcessor::moveCircular(const int &p_iSource, const float &p_fX, const float &p_fY, bool p_bIsRadiusFixed){
    if (!ZirkOscjuceAudioProcessor::s_bUseXY){
        Point<float> pointRelativeCenter(p_fX, p_fY);
        JUCE_COMPILER_WARNING("this needs to be uncommented and work");
        //moveCircularAzimElev(pointRelativeCenter, p_bIsRadiusFixed);
        return;
    }
    
    //convert x,y[0,1] to azim,elev[0,1]
    float fSelectedX = getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (p_iSource*5));
    float fSelectedY = getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (p_iSource*5));
    float fSelectedAzim, fSelectedElev;
    SoundSource::XY01toAzimElev01(fSelectedX, fSelectedY, fSelectedAzim, fSelectedElev);
    
    //set selectedSource to its position
    setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + p_iSource*5, HRToPercent(p_fX, -ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius));
    setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + p_iSource*5, HRToPercent(p_fY, -ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius));
    
    //calculate deltas
    float fDeltaAzim = SoundSource::XYtoAzim01(p_fX, p_fY) - fSelectedAzim;
    float fDeltaElev = SoundSource::XYtoElev01(p_fX, p_fY) - fSelectedElev;
    
    //for all other sources
    for (int iCurSource = 0; iCurSource < getNbrSources(); ++iCurSource) {
        
        if (iCurSource == p_iSource){
            continue;
        }
        
        float fCurX = getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (iCurSource*5));
        float fCurY = getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (iCurSource*5));
        fCurX = fCurX * 2 * ZirkOscjuceAudioProcessor::s_iDomeRadius - ZirkOscjuceAudioProcessor::s_iDomeRadius;
        fCurY = fCurY * 2 * ZirkOscjuceAudioProcessor::s_iDomeRadius - ZirkOscjuceAudioProcessor::s_iDomeRadius;
        float fCurAzim = SoundSource::XYtoAzim01(fCurX, fCurY);
        float fCurElev = SoundSource::XYtoElev01(fCurX, fCurY);
        
        float fNewAzim = fCurAzim+fDeltaAzim;
        
        //if radius is fixed, set all elevation to the same thing
        if (p_bIsRadiusFixed){
            float fX, fY;
            SoundSource::azimElev01toXY01(fNewAzim, fSelectedElev, fX, fY);
            setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (iCurSource*5), fX);
            setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (iCurSource*5), fY);
        }
        //if radius is not fixed, set all elevation to be current elevation +/- deltaY
        else {
            //if azimuth is NOT reversed, ie, NOT on the other side of the dome's middle point
            if(!getSources()[iCurSource].isAzimReverse()){
                float fX, fY;
                SoundSource::azimElev01toXY01(fNewAzim, fCurElev + fDeltaElev, fX, fY);
                setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (iCurSource*5), fX);
                setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (iCurSource*5), fY);
            } else {
                float fX, fY;
                SoundSource::azimElev01toXY01(fNewAzim, fCurElev - fDeltaElev, fX, fY);
                setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (iCurSource*5), fX);
                setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (iCurSource*5), fY);
            }
        }
    }
}

//void ZirkOscjuceAudioProcessor::moveCircular(const int &p_iSource, const float &p_fX, const float &p_fY, bool p_bIsRadiusFixed){
//    if (!ZirkOscjuceAudioProcessor::s_bUseXY){
//        Point<float> pointRelativeCenter(p_fX, p_fY);
//        JUCE_COMPILER_WARNING("this needs to be uncommented and work");
//        //moveCircularAzimElev(pointRelativeCenter, p_bIsRadiusFixed);
//        return;
//    }
//    
//    //convert x,y[0,1] to azim,elev[0,1]
//    float fSelectedX = getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (p_iSource*5));
//    float fSelectedY = getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (p_iSource*5));
//    float fSelectedAzim, fSelectedElev;
//    SoundSource::XY01toAzimElev01(fSelectedX, fSelectedY, fSelectedAzim, fSelectedElev);
//    
//    //calculate deltas
//    float fDeltaAzim = SoundSource::XYtoAzim01(p_fX, p_fY) - fSelectedAzim;
//    float fDeltaElev = SoundSource::XYtoElev01(p_fX, p_fY) - fSelectedElev;
//    
//    //for all other sources
//    for (int iCurSource = 0; iCurSource < getNbrSources(); ++iCurSource) {
//        
//        if (iCurSource == p_iSource){
//            continue;
//        }
//        
//        float fCurX = getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (iCurSource*5));
//        float fCurY = getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (iCurSource*5));
//        fCurX = fCurX * 2 * ZirkOscjuceAudioProcessor::s_iDomeRadius - ZirkOscjuceAudioProcessor::s_iDomeRadius;
//        fCurY = fCurY * 2 * ZirkOscjuceAudioProcessor::s_iDomeRadius - ZirkOscjuceAudioProcessor::s_iDomeRadius;
//        float fCurAzim = SoundSource::XYtoAzim01(fCurX, fCurY);
//        float fCurElev = SoundSource::XYtoElev01(fCurX, fCurY);
//        
//        float fNewAzim = fCurAzim+fDeltaAzim;
//        
//        //if radius is fixed, set all elevation to the same thing
//        if (p_bIsRadiusFixed){
//            float fX, fY;
//            SoundSource::azimElev01toXY01(fNewAzim, fSelectedElev, fX, fY);
//            _AllSources[iCurSource].setX01(fX);
//            _AllSources[iCurSource].setY01(fY);
//            
//        }
//        //if radius is not fixed, set all elevation to be current elevation +/- deltaY
//        else {
//            //if azimuth is NOT reversed, ie, NOT on the other side of the dome's middle point
//            if(!getSources()[iCurSource].isAzimReverse()){
//                float fX, fY;
//                SoundSource::azimElev01toXY01(fNewAzim, fCurElev + fDeltaElev, fX, fY);
//                _AllSources[iCurSource].setX01(fX);
//                _AllSources[iCurSource].setY01(fY);
//                
//            } else {
//                float fX, fY;
//                SoundSource::azimElev01toXY01(fNewAzim, fCurElev - fDeltaElev, fX, fY);
//                _AllSources[iCurSource].setX01(fX);
//                _AllSources[iCurSource].setY01(fY);
//            }
//        }
//    }
//}


void ZirkOscjuceAudioProcessor::storeCurrentLocations(){
    for (int iCurSource = 0; iCurSource<8; ++iCurSource){
        _AllSourcesBuffer[iCurSource] = _AllSources[iCurSource];
    }
}

void ZirkOscjuceAudioProcessor::restoreCurrentLocations(){
    for (int iCurSource = 0; iCurSource<8; ++iCurSource){
        _AllSources[iCurSource] = _AllSourcesBuffer[iCurSource];
    }
}

const String ZirkOscjuceAudioProcessor::getParameterText (int index)
{
    return String (getParameter (index), 2);
}

const String ZirkOscjuceAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String ZirkOscjuceAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool ZirkOscjuceAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool ZirkOscjuceAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool ZirkOscjuceAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool ZirkOscjuceAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool ZirkOscjuceAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

int ZirkOscjuceAudioProcessor::getNumPrograms()
{
    return 1;
}

int ZirkOscjuceAudioProcessor::getCurrentProgram()
{
    return 1;
}

void ZirkOscjuceAudioProcessor::setCurrentProgram (int index)
{
}

const String ZirkOscjuceAudioProcessor::getProgramName (int index)
{
    return String::empty;
}

void ZirkOscjuceAudioProcessor::changeProgramName (int index, const String& newName)
{

}

//==============================================================================
void ZirkOscjuceAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void ZirkOscjuceAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void ZirkOscjuceAudioProcessor::setSelectedSourceForTrajectory(int iSelectedSource){
    _SelectedSourceForTrajectory = iSelectedSource;
}

int ZirkOscjuceAudioProcessor::getSelectedSourceForTrajectory(){
    return _SelectedSourceForTrajectory;
}

//==============================================================================
bool ZirkOscjuceAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ZirkOscjuceAudioProcessor::createEditor()
{
    _Editor = new ZirkOscjuceAudioProcessorEditor (this);
    return _Editor;
}


void ZirkOscjuceAudioProcessor::setLastUiWidth(int lastUiWidth)
{
    _LastUiWidth = lastUiWidth;
}

int ZirkOscjuceAudioProcessor::getLastUiWidth()
{
    return _LastUiWidth;
}
void ZirkOscjuceAudioProcessor::setLastUiHeight(int lastUiHeight)
{
    _LastUiHeight = lastUiHeight;
}

int ZirkOscjuceAudioProcessor::getLastUiHeight()
{
    return _LastUiHeight;
}


//set wheter plug is sending osc messages to zirkonium
void ZirkOscjuceAudioProcessor::setIsOscActive(bool isOscActive){
    _isOscActive = isOscActive;
}

//wheter plug is sending osc messages to zirkonium
bool ZirkOscjuceAudioProcessor::getIsOscActive(){
    return _isOscActive;
}

void ZirkOscjuceAudioProcessor::setIsSyncWTempo(bool isSyncWTempo){
    _isSyncWTempo = isSyncWTempo;
}

bool ZirkOscjuceAudioProcessor::getIsSyncWTempo(){
    return _isSyncWTempo;
}


void ZirkOscjuceAudioProcessor::setIsSpanLinked(bool isSpanLinked){
    _isSpanLinked = isSpanLinked;
}

bool ZirkOscjuceAudioProcessor::getIsSpanLinked(){
    return _isSpanLinked;
}

void ZirkOscjuceAudioProcessor::setIsWriteTrajectory(bool isWriteTrajectory){
    _isWriteTrajectory = isWriteTrajectory;
}

bool ZirkOscjuceAudioProcessor::getIsWriteTrajectory(){
    return _isWriteTrajectory;
}

//==============================================================================
const String ZirkOscjuceAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int ZirkOscjuceAudioProcessor::getNumParameters()
{
    return totalNumParams;
}

// This method will be called by the host, probably on the audio thread, so
// it's absolutely time-critical. Don't use critical sections or anything
// UI-related, or anything at all that may block in any way!
float ZirkOscjuceAudioProcessor::getParameter (int index)
{
    switch (index){
        case ZirkOSC_MovementConstraint_ParamId:
            return _SelectedMovementConstraint;
        case ZirkOSC_isOscActive_ParamId:
            if (_isOscActive)
                return 1.0f;
            else
                return 0.0f;
        case ZirkOSC_isSpanLinked_ParamId:
            if (_isSpanLinked)
                return 1.0f;
            else
                return 0.0f;
        case ZirkOSC_SelectedTrajectory_ParamId:
            return _SelectedTrajectory;
        case ZirkOSC_SelectedTrajectoryDirection_ParamId:
            return m_fSelectedTrajectoryDirection;
        case ZirkOSC_SelectedTrajectoryReturn_ParamId:
            return m_fSelectedTrajectoryReturn;
        case ZirkOSC_TrajectoryCount_ParamId:
            return _TrajectoryCount;
        case ZirkOSC_TrajectoriesDuration_ParamId:
            return _TrajectoriesDuration;
        case ZirkOSC_SyncWTempo_ParamId:
            if (_isSyncWTempo)
                return 1.0f;
            else
                return 0.0f;
        case ZirkOSC_WriteTrajectories_ParamId:
            if (_isWriteTrajectory)
                return 1.0f;
            else
                return 0.0f;
    }
    
    for(int i = 0; i<8;++i){
        if      (ZirkOSC_Azim_or_x_ParamId + (i*5) == index)       {
            if (s_bUseXY)
                return (_AllSources[i].getX01()/* + s_iDomeRadius) / (2.f*s_iDomeRadius*/); //we normalize this value to [0,1]
            else
                return _AllSources[i].getAzimuth();
        }
        else if (ZirkOSC_AzimSpan_ParamId + (i*5) == index)   return _AllSources[i].getAzimuthSpan();
        else if (ZirkOSC_Elev_or_y_ParamId + (i*5) == index)       {
            
            if (s_bUseXY)
                return (_AllSources[i].getY01()/* + s_iDomeRadius) / (2.f*s_iDomeRadius*/); //we normalize this value to [0,1]
            else
                return _AllSources[i].getElevation();
        }
        else if (ZirkOSC_ElevSpan_ParamId + (i*5) == index)   return _AllSources[i].getElevationSpan();
        else if (ZirkOSC_Gain_ParamId + (i*5) == index)       return _AllSources[i].getGain();
    }
    cerr << "\n" << "wrong parameter id: " << index << "in ZirkOscjuceAudioProcessor::getParameter" << "\n";
    return -1.f;
}

// This method will be called by the host, probably on the audio thread, so
// it's absolutely time-critical. Don't use critical sections or anything
// UI-related, or anything at all that may block in any way!
void ZirkOscjuceAudioProcessor::setParameter (int index, float newValue)
{
#if defined(DEBUG)
    cout << "setParameter() with index: " << index << " and newValue: " << newValue << "\n";
#endif
    switch (index){
        case ZirkOSC_MovementConstraint_ParamId:
            _SelectedMovementConstraint = newValue;
            //m_iSelectedMovementConstraint = _SelectedMovementConstraint * (TotalNumberConstraints-1) + 1;
            m_iSelectedMovementConstraint = PercentToIntStartsAtOne(_SelectedMovementConstraint, TotalNumberConstraints);
            return;
        case ZirkOSC_isOscActive_ParamId:
            if (newValue > .5f)
                _isOscActive = true;
            else
                _isOscActive = false;
            return;
        case ZirkOSC_isSpanLinked_ParamId:
            if (newValue > .5f)
                _isSpanLinked = true;
            else
                _isSpanLinked = false;
            return;
        case ZirkOSC_SelectedTrajectory_ParamId:
            _SelectedTrajectory = newValue;
            return;
            
        case ZirkOSC_SelectedTrajectoryDirection_ParamId:
            m_fSelectedTrajectoryDirection = newValue;
            return;

        case ZirkOSC_SelectedTrajectoryReturn_ParamId:
            m_fSelectedTrajectoryReturn = newValue;
            return;
            
        case ZirkOSC_TrajectoryCount_ParamId:
            _TrajectoryCount = newValue;
            return;
        case ZirkOSC_TrajectoriesDuration_ParamId:
            _TrajectoriesDuration = newValue;
            return;
        case ZirkOSC_SyncWTempo_ParamId:
            if (newValue > .5f)
                _isSyncWTempo = true;
            else
                _isSyncWTempo = false;
            return;
        case ZirkOSC_WriteTrajectories_ParamId:
            if (newValue > .5f)
                _isWriteTrajectory = true;
            else
                _isWriteTrajectory = false;
            return;
    }
    //cout << "setParameter: " << index << " with value: " << newValue << "\n";
    JUCE_COMPILER_WARNING("more efficient to not have loop?")
    for(int iCurSource = 0; iCurSource < 8; ++iCurSource){
        if  (ZirkOSC_Azim_or_x_ParamId + (iCurSource*5) == index) {
            if (s_bUseXY) {
                _AllSources[iCurSource].setX01(newValue);
            } else {
                _AllSources[iCurSource].setAzimuth(newValue);
            }
            JUCE_COMPILER_WARNING("potentially have 2 variables for source id, in case we get x and y for different sources")
            m_iSourceLocationChanged = iCurSource;
            m_fSourceLocationChangedX = newValue;
            return;
        }
        else if (ZirkOSC_Elev_or_y_ParamId + (iCurSource*5) == index) {
            if (s_bUseXY){
                _AllSources[iCurSource].setY01(newValue);
            } else {
                _AllSources[iCurSource].setElevation(newValue);
            }
            m_iSourceLocationChanged = iCurSource;
            m_fSourceLocationChangedY = newValue;
            return;
        }
        else if (ZirkOSC_AzimSpan_ParamId + (iCurSource*5) == index) {_AllSources[iCurSource].setAzimuthSpan(newValue); return;}
        else if (ZirkOSC_ElevSpan_ParamId + (iCurSource*5) == index) {_AllSources[iCurSource].setElevationSpan(newValue); return;}
        else if (ZirkOSC_Gain_ParamId + (iCurSource*5) == index)     {_AllSources[iCurSource].setGain(newValue); return;}
    }
    cerr << "\n" << "wrong parameter id: " << index << " in ZirkOscjuceAudioProcessor::setParameter" << "\n";
}


const String ZirkOscjuceAudioProcessor::getParameterName (int index)
{
    switch (index){
        case ZirkOSC_MovementConstraint_ParamId:
            return ZirkOSC_Movement_Constraint_name;
        case ZirkOSC_isOscActive_ParamId:
            return ZirkOSC_isOscActive_name;
        case ZirkOSC_isSpanLinked_ParamId:
            return ZirkOSC_isSpanLinked_name;
        case ZirkOSC_SelectedTrajectory_ParamId:
            return ZirkOSC_SelectedTrajectory_name;
        case ZirkOSC_SelectedTrajectoryDirection_ParamId:
            return ZirkOSC_SelectedTrajectoryDirection_name;
        case ZirkOSC_SelectedTrajectoryReturn_ParamId:
            return ZirkOSC_SelectedTrajectoryReturn_name;
        case ZirkOSC_TrajectoryCount_ParamId:
            return ZirkOSC_NbrTrajectories_name;
        case ZirkOSC_TrajectoriesDuration_ParamId:
            return ZirkOSC_DurationTrajectories_name;
        case ZirkOSC_SyncWTempo_ParamId:
            return ZirkOSC_isSyncWTempo_name;
        case ZirkOSC_WriteTrajectories_ParamId:
            return ZirkOSC_isWriteTrajectory_name;
    }
    
    
    for(int i = 0; i<8; ++i){
        if      (ZirkOSC_Azim_or_x_ParamId + (i*5) == index) {

                return ZirkOSC_Azim_or_x_name[i];
        }
        else if (ZirkOSC_AzimSpan_ParamId + (i*5) == index)   return ZirkOSC_AzimSpan_name[i];
        else if (ZirkOSC_Elev_or_y_ParamId + (i*5) == index){

                return ZirkOSC_Elev_or_y_name[i];
        }
        else if (ZirkOSC_ElevSpan_ParamId + (i*5) == index)   return ZirkOSC_ElevSpan_name[i];
        else if (ZirkOSC_Gain_ParamId + (i*5) == index)       return ZirkOSC_Gain_name[i];
    }
    return String::empty;
}

static const int s_kiDataVersion = 3;

//==============================================================================
void ZirkOscjuceAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    
    XmlElement xml ("ZIRKOSCJUCESETTINGS");
    xml.setAttribute ("uiWidth", _LastUiWidth);
    xml.setAttribute ("uiHeight", _LastUiHeight);
    xml.setAttribute("PortOSC", _OscPortZirkonium);
    xml.setAttribute("NombreSources", _NbrSources);
    xml.setAttribute("MovementConstraint", _SelectedMovementConstraint);
    xml.setAttribute("isSpanLinked", _isSpanLinked);
    xml.setAttribute("isOscActive", _isOscActive);
    xml.setAttribute("selectedTrajectory", _SelectedTrajectory);
    xml.setAttribute("nbrTrajectory", _TrajectoryCount);
    xml.setAttribute("durationTrajectory", _TrajectoriesDuration);
    xml.setAttribute("isSyncWTempo", _isSyncWTempo);
    xml.setAttribute("isWriteTrajectory", _isWriteTrajectory);
    xml.setAttribute("selectedTrajectoryDirection", m_fSelectedTrajectoryDirection);
    xml.setAttribute("selectedTrajectoryReturn", m_fSelectedTrajectoryReturn);
    xml.setAttribute("presetDataVersion", s_kiDataVersion);
    
    for(int iCurSrc = 0; iCurSrc < 8; ++iCurSrc){
        String channel      = "Channel" + to_string(iCurSrc);
        String azimuth      = "Azimuth" + to_string(iCurSrc);
        String elevation    = "Elevation" + to_string(iCurSrc);
        String azimuthSpan  = "AzimuthSpan" + to_string(iCurSrc);
        String elevationSpan= "ElevationSpan" + to_string(iCurSrc);
        String gain         = "Gain" + to_string(iCurSrc);
        
        xml.setAttribute(channel, _AllSources[iCurSrc].getChannel());
        xml.setAttribute(azimuthSpan, _AllSources[iCurSrc].getAzimuthSpan());
        xml.setAttribute(elevationSpan, _AllSources[iCurSrc].getElevationSpan());
        xml.setAttribute(gain, _AllSources[iCurSrc].getGain());
        
        if (s_bUseXY){
            String strX = "X" + to_string(iCurSrc);
            String strY = "Y" + to_string(iCurSrc);
            xml.setAttribute(strX, _AllSources[iCurSrc].getX());
            xml.setAttribute(strY, _AllSources[iCurSrc].getY());
        } else {
            xml.setAttribute(azimuth, _AllSources[iCurSrc].getAzimuth());
            xml.setAttribute(elevation, _AllSources[iCurSrc].getElevationRawValue());
        }
    }
    copyXmlToBinary (xml, destData);
}


void ZirkOscjuceAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState != nullptr && xmlState->hasTagName ("ZIRKOSCJUCESETTINGS")) {
        
        int version                     = xmlState->getIntAttribute("presetDataVersion", 1);
        _LastUiWidth                    = xmlState->getIntAttribute ("uiWidth", _LastUiWidth);
        _LastUiHeight                   = xmlState->getIntAttribute ("uiHeight", _LastUiHeight);
        _OscPortZirkonium               = xmlState->getIntAttribute("PortOSC", 18032);
        _NbrSources                     = xmlState->getIntAttribute("NombreSources", 1);
        _SelectedMovementConstraint     = static_cast<float>(xmlState->getDoubleAttribute("MovementConstraint", .2f));
        m_iSelectedMovementConstraint   = PercentToIntStartsAtOne(_SelectedMovementConstraint, TotalNumberConstraints);
        _isOscActive                    = xmlState->getBoolAttribute("isOscActive", true);
        _isSpanLinked                   = xmlState->getBoolAttribute("isSpanLinked", false);
        _SelectedTrajectory             = static_cast<float>(xmlState->getDoubleAttribute("selectedTrajectory", .0f));
        _TrajectoryCount                = xmlState->getIntAttribute("nbrTrajectory", 0);
        _TrajectoriesDuration           = static_cast<float>(xmlState->getDoubleAttribute("durationTrajectory", .0f));
        _isSyncWTempo                   = xmlState->getBoolAttribute("isSyncWTempo", false);
        _isWriteTrajectory              = xmlState->getBoolAttribute("isWriteTrajectory", false);
        
        for (int iCurSrc = 0; iCurSrc < 8; ++iCurSrc){
            String channel      = "Channel" + to_string(iCurSrc);
            String azimuth      = "Azimuth" + to_string(iCurSrc);
            String elevation    = "Elevation" + to_string(iCurSrc);
            String azimuthSpan  = "AzimuthSpan" + to_string(iCurSrc);
            String elevationSpan= "ElevationSpan" + to_string(iCurSrc);
            String gain         = "Gain" + to_string(iCurSrc);
            
            if (version == 1 ){
                //in version 1, we were storing azimuth and elevation instead of x and y
                s_bUseXY = false;
                _AllSources[iCurSrc].setAzimuth  ((float) xmlState->getDoubleAttribute(azimuth,0));
                _AllSources[iCurSrc].setElevation((float) xmlState->getDoubleAttribute(elevation,0));
            } else {
                s_bUseXY = true;
                String strX = "X" + to_string(iCurSrc);
                String strY = "Y" + to_string(iCurSrc);
                Point<float> p((float) xmlState->getDoubleAttribute(strX,0), (float) xmlState->getDoubleAttribute(strY,0));
                _AllSources[iCurSrc].setXY(p);
            }
            
            _AllSources[iCurSrc].setChannel(xmlState->getIntAttribute(channel , 0));
            _AllSources[iCurSrc].setAzimuthSpan((float) xmlState->getDoubleAttribute(azimuthSpan,0));
            _AllSources[iCurSrc].setElevationSpan((float) xmlState->getDoubleAttribute(elevationSpan,0));
            float fGain = (float) xmlState->getDoubleAttribute(gain,1 );
            _AllSources[iCurSrc].setGain(fGain);
        }
        
        m_fSelectedTrajectoryDirection = static_cast<float>(xmlState->getDoubleAttribute("selectedTrajectoryDirection", .0f));
        m_fSelectedTrajectoryReturn    = static_cast<float>(xmlState->getDoubleAttribute("selectedTrajectoryReturn", .0f));
        changeZirkoniumOSCPort(_OscPortZirkonium);
        sendOSCValues();
        _RefreshGui=true;
    }
}


void ZirkOscjuceAudioProcessor::sendOSCValues(){
    if (_isOscActive){
        for(int i=0;i<_NbrSources;++i){
            float azim_osc = PercentToHR(_AllSources[i].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max) /180.;
            float elev_osc = PercentToHR(_AllSources[i].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max)/180.;
            float azimspan_osc = PercentToHR(_AllSources[i].getAzimuthSpan(), ZirkOSC_AzimSpan_Min,ZirkOSC_AzimSpan_Max)/180.;
            float elevspan_osc = PercentToHR(_AllSources[i].getElevationSpan(), ZirkOSC_ElevSpan_Min, ZirkOSC_Elev_Max)/180.;
            int channel_osc = _AllSources[i].getChannel()-1;
            float gain_osc = _AllSources[i].getGain();
            
            lo_send(_OscZirkonium, "/pan/az", "ifffff", channel_osc, azim_osc, elev_osc, azimspan_osc, elevspan_osc, gain_osc);
            
        }
    }
}


void ZirkOscjuceAudioProcessor::changeZirkoniumOSCPort(int newPort){
    
    if(newPort<0 || newPort>100000){
        newPort = _OscPortZirkonium;//18032;
    }
    
    lo_address osc = _OscZirkonium;
    _OscPortZirkonium = newPort;
	_OscZirkonium = NULL;
    lo_address_free(osc);
	char port[32];
	snprintf(port, sizeof(port), "%d", newPort);
	_OscZirkonium = lo_address_new("127.0.0.1", port);
    
}

int ZirkOscjuceAudioProcessor::getSelectedMovementConstraint() {
    return m_iSelectedMovementConstraint;
}

int ZirkOscjuceAudioProcessor::getSelectedTrajectory() {
    int value = PercentToIntStartsAtOne(_SelectedTrajectory, TotalNumberTrajectories);
    return value;
}

float ZirkOscjuceAudioProcessor::getSelectedTrajectoryDirection() {
    return m_fSelectedTrajectoryDirection;
}

float ZirkOscjuceAudioProcessor::getSelectedTrajectoryReturn() {
    return m_fSelectedTrajectoryReturn;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ZirkOscjuceAudioProcessor();

}

