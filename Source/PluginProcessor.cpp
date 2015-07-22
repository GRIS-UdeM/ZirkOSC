/*
 ==============================================================================
 ZirkOSC2: VST and AU audio plug-in enabling spatial movement of sound sources in a dome of speakers.
 
 Copyright (C) 2015  GRIS-UdeM
 
 Developers: Ludovic Laffineur, Vincent Berthiaume, Antoine Landrieu
 
 This program is free software: you can redistribute it and/or modify
π it under the terms of the GNU General Public License as published by
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
//#include <sstream>
//#include <regex.h>
//#include <arpa/inet.h>  //for inet_pton

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
,m_bNeedTosetEqualAngless(false)
,m_bFollowSelectedSource(false)
,m_bIsEqualElev(false)
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
        m_fSourceOldX01[i]          = -1.f;
        m_fSourceOldY01[i]          = -1.f;
        m_fROverflow[i]             = s_iDomeRadius;
        m_bIsElevationOverflow[i]   = false;
    }
}

void ZirkOscjuceAudioProcessor::timerCallback(){
    const MessageManagerLock mmLock;
    if (m_bFollowSelectedSource && m_iSourceLocationChanged != -1 && m_fSourceOldX01[m_iSourceLocationChanged] != -1 && m_fSourceOldY01[m_iSourceLocationChanged] != -1) {
        if (m_iSelectedMovementConstraint == DeltaLocked){
            moveSourcesWithDelta(m_iSourceLocationChanged, _AllSources[m_iSourceLocationChanged].getX(), _AllSources[m_iSourceLocationChanged].getY());
        } else {
            moveCircular(m_iSourceLocationChanged, _AllSources[m_iSourceLocationChanged].getX(), _AllSources[m_iSourceLocationChanged].getY(), m_bIsEqualElev);
        }
        m_iSourceLocationChanged = -1.f;
    }
    sendOSCValues();
}

void ZirkOscjuceAudioProcessor::move(int p_iSource, float p_fX, float p_fY){
    if (p_iSource > getNbrSources()){
        return;
    }
    
    //move selected source
    m_bFollowSelectedSource = false;
    float fX01 = HRToPercent(p_fX, -s_iDomeRadius, s_iDomeRadius);
    float fY01 = HRToPercent(p_fY, -s_iDomeRadius, s_iDomeRadius);

    setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_X_ParamId + p_iSource*5, fX01);
    setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Y_ParamId + p_iSource*5, fY01);

    if(m_iSelectedMovementConstraint == Independant){
        m_fSourceOldX01[p_iSource] = fX01;
        m_fSourceOldY01[p_iSource] = fY01;
    } else {
        m_bFollowSelectedSource = true;
        if (m_iSelectedMovementConstraint == EqualAngles){
            m_bIsEqualElev = false;
            moveEqualAngles(p_iSource, p_fX, p_fY);
        }
        else if (m_iSelectedMovementConstraint == EqualElev){
            m_bIsEqualElev = true;
            moveCircular(p_iSource, p_fX, p_fY, m_bIsEqualElev);
        }
        else if (m_iSelectedMovementConstraint == Circular){
            m_bIsEqualElev = false;
            moveCircular(p_iSource, p_fX, p_fY, m_bIsEqualElev);
        }
        else if (m_iSelectedMovementConstraint == FullyEqual){
            m_bIsEqualElev = true;
            moveFullyEqual(p_iSource, p_fX, p_fY);
        }
        else if (m_iSelectedMovementConstraint == DeltaLocked){
            m_bIsEqualElev = false;
            moveSourcesWithDelta(p_iSource, p_fX, p_fY);
        }
    }
}


void ZirkOscjuceAudioProcessor::moveSourcesWithDelta(const int &p_iSource, const float &p_fX, const float &p_fY){
    
    //calculate delta for selected source, which was already moved in ::move()
    float fSelectedOldX01 = m_fSourceOldX01[p_iSource];
    float fSelectedOldY01 = m_fSourceOldY01[p_iSource];
    
    float fSelectedDeltaX01 = HRToPercent(p_fX, -s_iDomeRadius, s_iDomeRadius) - fSelectedOldX01;
    float fSelectedDeltaY01 = HRToPercent(p_fY, -s_iDomeRadius, s_iDomeRadius) - fSelectedOldY01;
    
    //move unselected sources to their current position + deltamove
    for(int iCurSrc=0; iCurSrc<getNbrSources(); ++iCurSrc){
        
        if (iCurSrc == p_iSource){
            //save old values for selected source
            m_fSourceOldX01[p_iSource] = HRToPercent(p_fX, -s_iDomeRadius, s_iDomeRadius);
            m_fSourceOldY01[p_iSource] = HRToPercent(p_fY, -s_iDomeRadius, s_iDomeRadius);
            continue;
        }
        
        float newX01 = getSources()[iCurSrc].getX01() + fSelectedDeltaX01;
        float newY01 = getSources()[iCurSrc].getY01() + fSelectedDeltaY01;
        
        _AllSources[iCurSrc].setX01(newX01);
        _AllSources[iCurSrc].setY01(newY01);
        m_fSourceOldX01[iCurSrc] = newX01;
        m_fSourceOldY01[iCurSrc] = newY01;
    }
}

void ZirkOscjuceAudioProcessor::moveCircular(const int &p_iSource, const float &p_fSelectedNewX, const float &p_fSelectedNewY, bool p_bIsElevEqual){
    
    float fSelectedOldAzim01, fSelectedOldElev01, fSelectedNewAzim01, fSelectedNewElev01;
    
    //calculate old coordinates for selected source.
    float fSelectedOldX01 = m_fSourceOldX01[p_iSource];     //first time this is run, m_fSourceOldX01 was set by setParameter
    float fSelectedOldY01 = m_fSourceOldY01[p_iSource];
    //convert x,y[0,1] to azim,elev[0,1]
    SoundSource::XY01toAzimElev01(fSelectedOldX01, fSelectedOldY01, fSelectedOldAzim01, fSelectedOldElev01);
    
    //calculate new azim elev coordinates for selected source.
    fSelectedNewAzim01 = SoundSource::XYtoAzim01(p_fSelectedNewX, p_fSelectedNewY);
    fSelectedNewElev01 = SoundSource::XYtoElev01(p_fSelectedNewX, p_fSelectedNewY);
    
    //calculate deltas for selected source.
    float fSelectedDeltaAzim01 = fSelectedNewAzim01 - fSelectedOldAzim01;
    float fSelectedDeltaElev01 = fSelectedNewElev01 - fSelectedOldElev01;
    
    //move non-selected sources using the deltas
    for (int iCurSource = 0; iCurSource < getNbrSources(); ++iCurSource) {
        
        if (iCurSource == p_iSource){
            //save new values as old values for next time
            m_fSourceOldX01[p_iSource] = HRToPercent(p_fSelectedNewX, -s_iDomeRadius, s_iDomeRadius);
            m_fSourceOldY01[p_iSource] = HRToPercent(p_fSelectedNewY, -s_iDomeRadius, s_iDomeRadius);
            continue;
        }
        
        
        //---------------------- GET CURRENT VALUES ---------------------
        float fX = getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_X_ParamId + (iCurSource*5)) * 2 * s_iDomeRadius - s_iDomeRadius;
        float fY = getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_Y_ParamId + (iCurSource*5)) * 2 * s_iDomeRadius - s_iDomeRadius;
        float fCurAzim01 = SoundSource::XYtoAzim01(fX, fY);
        
        JUCE_COMPILER_WARNING("try a lambda function here")
        float fCurElev01;
        if (m_bIsElevationOverflow[iCurSource]){
            m_fROverflow[iCurSource] = m_fROverflow[iCurSource] - s_iDomeRadius;
            m_fROverflow[iCurSource] /= s_iDomeRadius;
            fCurElev01 = -asin(m_fROverflow[iCurSource]); //need to convert output of sinm which is is radians, to degree then hr to percent
            fCurElev01 = radianToDegree(fCurElev01);
            fCurElev01 = HRToPercent(fCurElev01, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
        } else {
            fCurElev01 = SoundSource::XYtoElev01(fX, fY);
        }
        
        //---------------------- ENDOF GET CURRENT VALUES ---------------------
        
        float fNewAzim01 = fCurAzim01 + fSelectedDeltaAzim01;
        
        float fX01, fY01;
        //if elev is equal, set all elevation to the same thing
        if (p_bIsElevEqual){
            SoundSource::azimElev01toXY01(fNewAzim01, fSelectedOldElev01, fX01, fY01);
            _AllSources[iCurSource].setX01(fX01);
            _AllSources[iCurSource].setY01(fY01);
        }
        //if elev is not equal, set all elevation to be current elevation +/- deltaY
        else {
            //if azimuth is reversed, ie, on the other side of the dome's middle point
            float fNewElev01;
            if(getSources()[iCurSource].isAzimReverse()){
                fNewElev01 = fCurElev01 - fSelectedDeltaElev01;
            } else {
                fNewElev01 = fCurElev01 + fSelectedDeltaElev01;
            }
            
            if (fNewElev01 > 1){
                if (!getSources()[iCurSource].isAzimReverse()){
                    getSources()[iCurSource].setAzimReverse(true);
                } else {
                    getSources()[iCurSource].setAzimReverse(false);
                }
                fNewElev01 = 1 + (1-fNewElev01);
                float fAzimDegrees = PercentToHR(fNewAzim01, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
                fAzimDegrees += (fAzimDegrees < 0 ? 180 : -180);
                fNewAzim01 = HRToPercent(fAzimDegrees, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
            }
            
            if (fNewElev01 < 0){
                m_fROverflow[iCurSource] = s_iDomeRadius * sin(degreeToRadian(PercentToHR(fNewElev01, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max)));
                m_fROverflow[iCurSource] = s_iDomeRadius - m_fROverflow[iCurSource];
                SoundSource::azimElev01toXY01(fNewAzim01, 0, fX01, fY01, m_fROverflow[iCurSource]);
                m_bIsElevationOverflow[iCurSource] = true;

            } else {
                SoundSource::azimElev01toXY01(fNewAzim01, fNewElev01, fX01, fY01);
                m_bIsElevationOverflow[iCurSource] = false;
            }
            
            _AllSources[iCurSource].setX01(fX01);
            _AllSources[iCurSource].setY01(fY01);
        }
        //save new values as old values for next time
        m_fSourceOldX01[iCurSource] = fX01;
        m_fSourceOldY01[iCurSource] = fY01;
    }
}

void ZirkOscjuceAudioProcessor::moveEqualAngles(const int &p_iSource, const float &p_fX, const float &p_fY){
    if (m_bNeedTosetEqualAngless){
        orderSourcesByAngle(getSelectedSource(),getSources());
        m_bNeedTosetEqualAngless = false;
    }
    moveCircular(p_iSource, p_fX, p_fY, false);
}

void ZirkOscjuceAudioProcessor::moveFullyEqual(const int &p_iSource, const float &p_fX, const float &p_fY){
    if (m_bNeedTosetEqualAngless){
        orderSourcesByAngle(getSelectedSource(),getSources());
        m_bNeedTosetEqualAngless = false;
    }
    moveCircular(p_iSource, p_fX, p_fY, true);
}

void ZirkOscjuceAudioProcessor::orderSourcesByAngle (int selected, SoundSource tab[]){
    m_bFollowSelectedSource = false;
    int nbrSources = getNbrSources();
    vector<int> order = getOrderSources(selected, tab, nbrSources);
    int count = 0;
    for(int i= 1; i < nbrSources ; ++i){
        
        float curangle = tab[order[0]].getAzimuth()+ (float)(++count)/(float) nbrSources;
        float fX, fY;
        SoundSource::azimElev01toXY01(curangle, tab[order[i]].getElevation(), fX, fY);
        setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_X_ParamId + (order[i]*5), fX);
        setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Y_ParamId + (order[i]*5), fY);

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

ZirkOscjuceAudioProcessor::~ZirkOscjuceAudioProcessor()
{
    stopTimer();
    _isOscActive = false;
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
    return m_bNeedTosetEqualAngless;
}

void ZirkOscjuceAudioProcessor::setEqualAngles(bool fixedAngle){
    m_bNeedTosetEqualAngless = fixedAngle;
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
    
    for(int iCurSrc = 0; iCurSrc< 8; ++iCurSrc){
        
        if (ZirkOSC_X_ParamId + (iCurSrc*5) == index) {
            return (_AllSources[iCurSrc].getX01());
        }
        else if (ZirkOSC_Y_ParamId + (iCurSrc*5) == index){
            return (_AllSources[iCurSrc].getY01());
        }
        else if (ZirkOSC_AzimSpan_ParamId + (iCurSrc*5) == index){
            return _AllSources[iCurSrc].getAzimuthSpan();
        }
        else if (ZirkOSC_ElevSpan_ParamId + (iCurSrc*5) == index){
            return _AllSources[iCurSrc].getElevationSpan();
        }
        else if (ZirkOSC_Gain_ParamId + (iCurSrc*5) == index){
            return _AllSources[iCurSrc].getGain();
        }
    }
    cerr << "\n" << "wrong parameter id: " << index << "in ZirkOscjuceAudioProcessor::getParameter" << "\n";
    return -1.f;
}

// This method will be called by the host, probably on the audio thread, so
// it's absolutely time-critical. Don't use critical sections or anything
// UI-related, or anything at all that may block in any way!
void ZirkOscjuceAudioProcessor::setParameter (int index, float newValue){
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
        if  (ZirkOSC_X_ParamId + (iCurSource*5) == index) {
            if (newValue != _AllSources[iCurSource].getX01()){
                _AllSources[iCurSource].setX01(newValue);
                m_iSourceLocationChanged = iCurSource;
                if (m_fSourceOldX01[iCurSource] == -1.f){
                    //if this was not initialized, we get our first value here
                    m_fSourceOldX01[iCurSource] = newValue;
                }
            }
            return;
        }
        else if (ZirkOSC_Y_ParamId + (iCurSource*5) == index) {
            if (newValue != _AllSources[iCurSource].getY01()){
                _AllSources[iCurSource].setY01(newValue);
                m_iSourceLocationChanged = iCurSource;
                if (m_fSourceOldY01[iCurSource] == -1.f){
                    m_fSourceOldY01[iCurSource] = newValue;
                }
            }
            return;
        }
        else if (ZirkOSC_AzimSpan_ParamId + (iCurSource*5) == index){
            _AllSources[iCurSource].setAzimuthSpan(newValue); return;
        }
        else if (ZirkOSC_ElevSpan_ParamId + (iCurSource*5) == index){
            _AllSources[iCurSource].setElevationSpan(newValue); return;
        }
        else if (ZirkOSC_Gain_ParamId + (iCurSource*5) == index){
            _AllSources[iCurSource].setGain(newValue); return;
        }
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
        if      (ZirkOSC_X_ParamId + (i*5) == index) {

                return ZirkOSC_X_name[i];
        }
        else if (ZirkOSC_AzimSpan_ParamId + (i*5) == index)   return ZirkOSC_AzimSpan_name[i];
        else if (ZirkOSC_Y_ParamId + (i*5) == index){

                return ZirkOSC_Y_name[i];
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
        String channel      = "Channel"         + to_string(iCurSrc);
        String azimuthSpan  = "AzimuthSpan"     + to_string(iCurSrc);
        String elevationSpan= "ElevationSpan"   + to_string(iCurSrc);
        String gain         = "Gain"            + to_string(iCurSrc);
        String strX = "X" + to_string(iCurSrc);
        String strY = "Y" + to_string(iCurSrc);
        
        xml.setAttribute(channel,       _AllSources[iCurSrc].getChannel());
        xml.setAttribute(azimuthSpan,   _AllSources[iCurSrc].getAzimuthSpan());
        xml.setAttribute(elevationSpan, _AllSources[iCurSrc].getElevationSpan());
        xml.setAttribute(gain,          _AllSources[iCurSrc].getGain());
        xml.setAttribute(strX,          _AllSources[iCurSrc].getX01());
        xml.setAttribute(strY,          _AllSources[iCurSrc].getY01());
    }
    copyXmlToBinary (xml, destData);
}


void ZirkOscjuceAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState != nullptr && xmlState->hasTagName ("ZIRKOSCJUCESETTINGS")) {
        
        int version = xmlState->getIntAttribute("presetDataVersion", 1);
        
        if (version == 1 ){
            String m;
            
            m << "You are attempting to load ZirkOSC with a preset from an older version." << newLine << newLine
            << "Stored source locations are unreadable by this version, so default values will be used.";
            
            DialogWindow::LaunchOptions options;
            Label* label = new Label();
            label->setText (m, dontSendNotification);
            //label->setColour (Label::textColourId, Colours::whitesmoke);
            options.content.setOwned (label);
            
            Rectangle<int> area (0, 0, 300, 100);
            options.content->setSize (area.getWidth(), area.getHeight());
            
            options.dialogTitle                   = "ZirkOSC";
            //options.dialogBackgroundColour        = Colour (0xff0e345a);
            options.escapeKeyTriggersCloseButton  = true;
            options.useNativeTitleBar             = true;
            options.resizable                     = false;
            
            const RectanglePlacement placement (RectanglePlacement::xRight + RectanglePlacement::yBottom + RectanglePlacement::doNotResize);
            
            DialogWindow* dw = options.launchAsync();
            dw->centreWithSize (300, 100);
        }
        
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
            String azimuthSpan  = "AzimuthSpan" + to_string(iCurSrc);
            String elevationSpan= "ElevationSpan" + to_string(iCurSrc);
            String gain         = "Gain" + to_string(iCurSrc);
            String strX         = "X" + to_string(iCurSrc);
            String strY         = "Y" + to_string(iCurSrc);

            _AllSources[iCurSrc].setChannel(xmlState->getIntAttribute(channel , 0));
            _AllSources[iCurSrc].setAzimuthSpan(    static_cast<float>(xmlState->getDoubleAttribute(azimuthSpan,0)));
            _AllSources[iCurSrc].setElevationSpan(  static_cast<float>(xmlState->getDoubleAttribute(elevationSpan,0)));
            _AllSources[iCurSrc].setGain(           static_cast<float>(xmlState->getDoubleAttribute(gain, 1)));
            
            float fDefaultX, fDefaultY;
            SoundSource::azimElev01toXY01(iCurSrc * .125, 0, fDefaultX, fDefaultY);
            
            setParameter (ZirkOscjuceAudioProcessor::ZirkOSC_X_ParamId + iCurSrc*5, static_cast<float>(xmlState->getDoubleAttribute(strX, fDefaultX)));
            setParameter (ZirkOscjuceAudioProcessor::ZirkOSC_Y_ParamId + iCurSrc*5, static_cast<float>(xmlState->getDoubleAttribute(strY, fDefaultY)));
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

