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
,_SelectedSourceForTrajectory(0)
,m_iSourceLocationChanged(-1)
,m_bNeedToSetFixedAngles(false)
,m_bFollowSelectedSource(false)
,m_bIsRadiusFixed(false)
{
    initSources();

    _OscZirkonium = lo_address_new("127.0.0.1", "10001");
    
    //default values for ui dimensions
    _LastUiWidth  = ZirkOSC_Window_Default_Width;
    _LastUiHeight = ZirkOSC_Window_Default_Height;
    
    startTimer (50);
}

void ZirkOscjuceAudioProcessor::initSources(){
    for(int i = 0; i < 8; ++i){
        _AllSources[i] = SoundSource(0.0+((float)i/8.0),0.0);
        JUCE_COMPILER_WARNING("probably we will need to do this or something similar when we change the number of sources... or not, not sure")
        m_fSourceOldX[i] = -1.f;
        m_fSourceOldY[i] = -1.f;
    }
}

void ZirkOscjuceAudioProcessor::timerCallback(){
    const MessageManagerLock mmLock;
    if (m_bFollowSelectedSource && m_iSourceLocationChanged != -1 && m_fSourceOldX[m_iSourceLocationChanged] != -1 && m_fSourceOldY[m_iSourceLocationChanged] != -1) {
//        moveCircular(m_iSourceLocationChanged, _AllSources[m_iSourceLocationChanged].getX01(), _AllSources[m_iSourceLocationChanged].getY01(), m_bIsRadiusFixed);
        moveCircular(m_iSourceLocationChanged, _AllSources[m_iSourceLocationChanged].getX(), _AllSources[m_iSourceLocationChanged].getY(), m_bIsRadiusFixed);
        m_iSourceLocationChanged = -1.f;
    }
    sendOSCValues();
}

void ZirkOscjuceAudioProcessor::move(int p_iSource, float p_fX, float p_fY){
    if (p_iSource > getNbrSources()){
        return;
    }
    
    int selectedConstraint = getSelectedMovementConstraint();
    //if(selectedConstraint == Independant) {
        m_bFollowSelectedSource = false;
        setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + p_iSource*5, HRToPercent(p_fX, -ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius));
        setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + p_iSource*5, HRToPercent(p_fY, -ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius));
    //}
    
    //not-independent
    //else {
    if(selectedConstraint != Independant){
        if (selectedConstraint == FixedAngles){
            m_bIsRadiusFixed = false;
            moveFixedAngles(p_iSource, p_fX, p_fY);
        }
        else if (selectedConstraint == FixedRadius){
            m_bIsRadiusFixed = true;
            moveCircular(p_iSource, p_fX, p_fY, m_bIsRadiusFixed);
        }
        else if (selectedConstraint == Circular){
            m_bIsRadiusFixed = false;
            moveCircular(p_iSource, p_fX, p_fY, m_bIsRadiusFixed);
        }
        else if (selectedConstraint == FullyFixed){
            m_bIsRadiusFixed = true;
            moveFullyFixed(p_iSource, p_fX, p_fY);
        }
        else if (selectedConstraint == DeltaLocked){
            m_bIsRadiusFixed = false;
            float oldX, oldY;
            getSources()[p_iSource].getXY(oldX,oldY);
            float deltax = p_fX - oldX;
            float deltay = p_fY - oldY;
            moveSourcesWithDelta(p_iSource, deltax, deltay);
        }
    }
}

void ZirkOscjuceAudioProcessor::moveCircular(const int &p_iSource, const float &p_fSelectedNewX, const float &p_fSelectedNewY, bool p_bIsRadiusFixed){
    
    float fSelectedOldAzim, fSelectedOldElev, fSelectedNewAzim, fSelectedNewElev;
    
    //calculate old coordinates for selected source.
    //first time this is run, m_fSourceOldX was set by setParameter
    JUCE_COMPILER_WARNING("THIS WILL NOT WORK IN NON-UNIQUE CASES")
    float fSelectedOldX = m_fSourceOldX[p_iSource];    //    float fSelectedOldX = getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (p_iSource*5));
    float fSelectedOldY = m_fSourceOldY[p_iSource];    //    float fSelectedOldY = getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (p_iSource*5));
    //convert x,y[0,1] to azim,elev[0,1]
    SoundSource::XY01toAzimElev01(fSelectedOldX, fSelectedOldY, fSelectedOldAzim, fSelectedOldElev);
    
    //calculate new azim elev coordinates for selected source.
    //SoundSource::XY01toAzimElev01(p_fSelectedNewX, p_fSelectedNewY, fSelectedNewAzim, fSelectedNewElev);
    fSelectedNewAzim = SoundSource::XYtoAzim01(p_fSelectedNewX, p_fSelectedNewY);
    fSelectedNewElev = SoundSource::XYtoElev01(p_fSelectedNewX, p_fSelectedNewY);
    
    //calculate deltas for selected source.
    float fSelectedDeltaAzim = fSelectedNewAzim - fSelectedOldAzim;
    float fSelectedDeltaElev = fSelectedNewElev - fSelectedOldElev;
    
    cout << "fSelectedDeltaAzim " << fSelectedDeltaAzim << "\n";
    cout << "fSelectedDeltaElev " << fSelectedDeltaElev << "\n";
    
    //move non-selected sources using the deltas
    for (int iCurSource = 0; iCurSource < getNbrSources(); ++iCurSource) {
        
        if (iCurSource == p_iSource){
            continue;
        }
        float fCurAzim, fCurElev;
        SoundSource::XY01toAzimElev01(getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (iCurSource*5)), getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (iCurSource*5)), fCurAzim, fCurElev);
    
        float fNewAzim = fCurAzim + fSelectedDeltaAzim;
        
        //if radius is fixed, set all elevation to the same thing
        if (p_bIsRadiusFixed){
            float fX, fY;
            SoundSource::azimElev01toXY01(fNewAzim, fSelectedOldElev, fX, fY);
            _AllSources[iCurSource].setX01(fX);
            _AllSources[iCurSource].setY01(fY);
            
        }
        //if radius is not fixed, set all elevation to be current elevation +/- deltaY
        else {
            //if azimuth is NOT reversed, ie, NOT on the other side of the dome's middle point
            if(!getSources()[iCurSource].isAzimReverse()){
                float fX, fY;
                SoundSource::azimElev01toXY01(fNewAzim, fCurElev + fSelectedDeltaElev, fX, fY);
                _AllSources[iCurSource].setX01(fX);
                _AllSources[iCurSource].setY01(fY);
                
            } else {
                float fX, fY;
                SoundSource::azimElev01toXY01(fNewAzim, fCurElev - fSelectedDeltaElev, fX, fY);
                _AllSources[iCurSource].setX01(fX);
                _AllSources[iCurSource].setY01(fY);
            }
        }
    }
    JUCE_COMPILER_WARNING("what we need is to have an array of those for all sources")
    m_fSourceOldX[p_iSource] = HRToPercent(p_fSelectedNewX, -ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius);
    m_fSourceOldY[p_iSource] = HRToPercent(p_fSelectedNewY, -ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius);
}

JUCE_COMPILER_WARNING("this was an unfinished attempt at simplifying moveCircular(), but we sill need to convert to azim elev when radius is fixed, so not clear that this will improve performance")
//void ZirkOscjuceAudioProcessor::moveCircular(const int &p_iSelSource, const float &p_fNewSelX, const float &p_fNewSelY, bool p_bIsRadiusFixed){
//
//    //convert x,y[0,1] to azim,elev[0,1]
//    float fOldSelX = getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (p_iSelSource*5));
//    float fOldSelY = getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (p_iSelSource*5));
//
//    //set selectedSource to its xy01 position
//    setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + p_iSelSource*5, HRToPercent(p_fNewSelX, -ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius));
//    setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + p_iSelSource*5, HRToPercent(p_fNewSelY, -ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius));
//
//    //calculate deltas in 01
//    float fDeltaSelX = p_fNewSelX - fOldSelX;
//    float fDeltaSelY = p_fNewSelY - fOldSelY;
//
//    //for all other sources
//    for (int iCurSource = 0; iCurSource < getNbrSources(); ++iCurSource) {
//
//        if (iCurSource == p_iSelSource){
//            continue;
//        }
//
//        //convert xy01 to azim elev01
//        float fCurX = getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (iCurSource*5));
//        float fCurY = getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (iCurSource*5));
//
//        //calculate new azimuth
//        float fNewX = fCurX + fDeltaSelX;
//        float fNewY = fCurY + fDeltaSelY;
//
//        //if radius is fixed, set all elevation to the same xy
//        if (p_bIsRadiusFixed){
////            float fX, fY;
////            SoundSource::azimElev01toXY01(fNewAzim, fSelectedElev, fX, fY);
////            setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (iCurSource*5), fX);
////            setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (iCurSource*5), fY);
//
//            SoundSource::XY01toAzimElev01();
//
//        }
//        //if radius is not fixed, set all elevation to be current elevation +/- deltaY
//        else {
//            //if azimuth is NOT reversed, ie, NOT on the other side of the dome's middle point
//            if(!getSources()[iCurSource].isAzimReverse()){
//                float fX, fY;
//                SoundSource::azimElev01toXY01(fNewAzim, fCurElev + fDeltaElev, fX, fY);
//                setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (iCurSource*5), fX);
//                setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (iCurSource*5), fY);
//            } else {
//                float fX, fY;
//                SoundSource::azimElev01toXY01(fNewAzim, fCurElev - fDeltaElev, fX, fY);
//                setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (iCurSource*5), fX);
//                setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (iCurSource*5), fY);
//            }
//        }
//    }
//}

void ZirkOscjuceAudioProcessor::moveFixedAngles(const int &p_iSource, const float &p_fX, const float &p_fY){
    if (m_bNeedToSetFixedAngles){
        orderSourcesByAngle(getSelectedSource(),getSources());
        m_bNeedToSetFixedAngles = false;
    }
    moveCircular(p_iSource, p_fX, p_fY, false);
}

void ZirkOscjuceAudioProcessor::moveFullyFixed(const int &p_iSource, const float &p_fX, const float &p_fY){
    if (m_bNeedToSetFixedAngles){
        orderSourcesByAngle(getSelectedSource(),getSources());
        m_bNeedToSetFixedAngles = false;
    }
    moveCircular(p_iSource, p_fX, p_fY, true);
}

void ZirkOscjuceAudioProcessor::orderSourcesByAngle (int selected, SoundSource tab[]){
    m_bFollowSelectedSource = false;
    int nbrSources = getNbrSources();
    vector<int> order = getOrderSources(selected, tab, nbrSources);
    int count = 0;
    for(int i= 1; i < nbrSources ; ++i){ //for(int i= 1; i != nbrSources ; ++i){
        float curangle = tab[order[0]].getAzimuth()+ (float)(++count)/(float) nbrSources;

            float fX, fY;
            SoundSource::azimElev01toXY01(curangle, tab[order[i]].getElevation(), fX, fY);
            setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (order[i]*5), fX);
            setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (order[i]*5), fY);

    }
    m_bFollowSelectedSource = true;
}

//starting from the selected source, cycle through the other sources to find in which order they are
vector<int> ZirkOscjuceAudioProcessor::getOrderSources(int selected, SoundSource tab [], int nbrSources){
    
    vector<int> order(nbrSources);
    int firstItem = selected;
    order[0] = selected;    //selected source is at order[0]
    int count  = 1;
    do{
        int current = (selected + 1)%nbrSources; //current is the next one after the seleted one
        
        int bestItem = current;
        float bestDelta = tab[current].getAzimuth() - tab[selected].getAzimuth(); //difference between current and selected
        if (bestDelta<0){
            bestDelta+=1;
        }
        
        while (current != selected) {
            float currentAzimuth;
            if (tab[current].getAzimuth() - tab[selected].getAzimuth()>0 ){
                currentAzimuth = tab[current].getAzimuth();
            }
            else{
                currentAzimuth = tab[current].getAzimuth()+1;
            }
            if (currentAzimuth - tab[selected].getAzimuth() < bestDelta) {
                bestItem = current;
                bestDelta = currentAzimuth - tab[selected].getAzimuth();
                
            }
            current = (current +1) % nbrSources;
        }
        
        order[count++]=bestItem;
        selected = bestItem;
    } while (selected != firstItem && count < nbrSources);
    return order;
}

void ZirkOscjuceAudioProcessor::moveSourcesWithDelta(const int &p_iSource, const float &p_fX, const float &p_fY){
    
    //simply need to move all sources to their current position + deltamove
    for(int i=0; i<getNbrSources(); ++i){
        
        float currentX, currentY;
        getSources()[i].getXY(currentX, currentY);
        float newX = currentX + p_fX;
        float newY = currentY + p_fY;
        
        float fX01 = (newX + ZirkOscjuceAudioProcessor::s_iDomeRadius) / (2*ZirkOscjuceAudioProcessor::s_iDomeRadius);
        float fY01 = (newY + ZirkOscjuceAudioProcessor::s_iDomeRadius) / (2*ZirkOscjuceAudioProcessor::s_iDomeRadius);
        
        setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + i * 5, fX01);
        setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + i * 5, fY01);
    }
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

bool ZirkOscjuceAudioProcessor::isFixedAngle(){
    return m_bNeedToSetFixedAngles;
}

void ZirkOscjuceAudioProcessor::setFixedAngle(bool fixedAngle){
    m_bNeedToSetFixedAngles = fixedAngle;
}

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
                return (_AllSources[i].getX01()/* + s_iDomeRadius) / (2.f*s_iDomeRadius*/); //we normalize this value to [0,1]

        }
        else if (ZirkOSC_AzimSpan_ParamId + (i*5) == index)   return _AllSources[i].getAzimuthSpan();
        else if (ZirkOSC_Elev_or_y_ParamId + (i*5) == index)       {
            

                return (_AllSources[i].getY01()/* + s_iDomeRadius) / (2.f*s_iDomeRadius*/); //we normalize this value to [0,1]

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
    switch (index){
        case ZirkOSC_MovementConstraint_ParamId:
            _SelectedMovementConstraint = newValue;
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

    JUCE_COMPILER_WARNING("probably need to pause Processor::timercallback when x and y are set one after the other, externally")
    for(int iCurSource = 0; iCurSource < 8; ++iCurSource){
        if  (ZirkOSC_Azim_or_x_ParamId + (iCurSource*5) == index) {

                _AllSources[iCurSource].setX01(newValue);


            m_iSourceLocationChanged = iCurSource;
            if (m_fSourceOldX[iCurSource] == -1.f){
                //if this was not initialized, we get our first value here
                m_fSourceOldX[iCurSource] = newValue;
            }
            return;
        }
        else if (ZirkOSC_Elev_or_y_ParamId + (iCurSource*5) == index) {

                _AllSources[iCurSource].setY01(newValue);


            m_iSourceLocationChanged = iCurSource;
            if (m_fSourceOldY[iCurSource] == -1.f){
                m_fSourceOldY[iCurSource] = newValue;
            }
            return;
        }
        else if (ZirkOSC_AzimSpan_ParamId + (iCurSource*5) == index) {_AllSources[iCurSource].setAzimuthSpan(newValue); return;}
        else if (ZirkOSC_ElevSpan_ParamId + (iCurSource*5) == index) {_AllSources[iCurSource].setElevationSpan(newValue); return;}
        else if (ZirkOSC_Gain_ParamId + (iCurSource*5) == index)     {_AllSources[iCurSource].setGain(newValue); return;}
    }
    cerr << "wrong parameter id: " << index << " in ZirkOscjuceAudioProcessor::setParameter\n";
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
        

            String strX = "X" + to_string(iCurSrc);
            String strY = "Y" + to_string(iCurSrc);
            xml.setAttribute(strX, _AllSources[iCurSrc].getX());
            xml.setAttribute(strY, _AllSources[iCurSrc].getY());

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

                _AllSources[iCurSrc].setAzimuth  ((float) xmlState->getDoubleAttribute(azimuth,0));
                _AllSources[iCurSrc].setElevation((float) xmlState->getDoubleAttribute(elevation,0));
            }
            
            _AllSources[iCurSrc].setChannel(xmlState->getIntAttribute(channel , 0));
            _AllSources[iCurSrc].setAzimuthSpan((float) xmlState->getDoubleAttribute(azimuthSpan,0));
            _AllSources[iCurSrc].setElevationSpan((float) xmlState->getDoubleAttribute(elevationSpan,0));
            float fGain = (float) xmlState->getDoubleAttribute(gain, 1);
            _AllSources[iCurSrc].setGain(fGain);
        }
        
        m_fSelectedTrajectoryDirection = static_cast<float>(xmlState->getDoubleAttribute("selectedTrajectoryDirection", .0f));
        m_fSelectedTrajectoryReturn    = static_cast<float>(xmlState->getDoubleAttribute("selectedTrajectoryReturn", .0f));
        changeZirkoniumOSCPort(_OscPortZirkonium);
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

