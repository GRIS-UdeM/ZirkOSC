/*
 ==============================================================================
 ZirkOSC: VST and AU audio plug-in enabling spatial movement of sound sources in a dome of speakers.
 
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

//==============================================================================
class SourceUpdateThread : public Thread
{
public:
    SourceUpdateThread(ZirkOscAudioProcessor* p_pProcessor)
    : Thread ("SourceUpdateThread")
    ,m_iInterval(25)
    ,m_pProcessor(p_pProcessor) {
        
        startThread ();
    }
    
    ~SourceUpdateThread() {
        // allow the thread .5 second to stop cleanly - should be plenty of time.
        stopThread (500);
    }
    
    void run() override {
       
        // threadShouldExit() returns true when the stopThread() method has been called
        while (! threadShouldExit()) {

            // sleep a bit so the threads don't all grind the CPU to a halt..
            wait (m_iInterval);

            // because this is a background thread, we mustn't do any UI work without first grabbing a MessageManagerLock..
            const MessageManagerLock mml (Thread::getCurrentThread());
            
            if (! mml.lockWasGained())  // if something is trying to kill this job, the lock
                return;                 // will fail, in which case we'd better return..
            
            // now we've got the UI thread locked, we can mess about with the components
            m_pProcessor->updateSourcesSendOsc();
        }
    }
    
private:
    int m_iInterval;
    ZirkOscAudioProcessor* m_pProcessor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceUpdateThread)
};

int ZirkOscAudioProcessor::s_iDomeRadius = 172;

bool ZirkOscAudioProcessor::s_bForceConstraintAutomation = false;   //this was to force reaper to write an automation of the constraint combobox value when starting playback/record

ZirkOscAudioProcessor::ZirkOscAudioProcessor()
:m_iNbrSources(2)
,m_fSelectedTrajectory(.0f)
,m_fSelectedTrajectoryDirection(.0f)
,m_fSelectedTrajectoryReturn(.0f)
,m_iSelectedSource(0)
,m_bIsOscActive(true)
,m_bIsSpanLinked(true)
,m_dTrajectoryCount(1.)
,m_dTrajectoriesDuration(5.)
,m_dTrajectoryTurns(1.)
,m_dTrajectoryDeviation(0.)
,m_dTrajectoryDampening(0.)
//,_TrajectoriesPhiAsin(0)
//,_TrajectoriesPhiAcos(0)
,m_bIsSyncWTempo(false)
,m_bIsWriteTrajectory(false)
,m_iSelectedSourceForTrajectory(0)
,m_iSourceLocationChanged(-1)
,m_bCurrentlyPlaying(false)
,m_bDetectedPlayingStart(false)
,m_bDetectedPlayingEnd(true)
,m_bStartedConstraintAutomation(false)
,m_bIsRecordingAutomation(false)
,m_iNeedToResetToActualConstraint(-1)
{
    setMovementConstraint(Independent);
    
    initSources();

    //OSC-------------------------------
//    char port[32];
//    snprintf(port, sizeof(port), "%d", m_iOscPortZirkonium);
//    _OscZirkonium = lo_address_new("127.0.0.1", port);
    connectOsc(18032);
    //OSC-------------------------------
    
    //default values for ui dimensions
    _LastUiWidth  = ZirkOSC_Window_Default_Width;
    _LastUiHeight = ZirkOSC_Window_Default_Height;
    
    m_pSourceUpdateThread = new SourceUpdateThread(this);
    
    m_fEndLocationXY = make_pair(0, 0);
}

//OSC-----------------------
void ZirkOscAudioProcessor::connectOsc(int p_iNewPort){
    if(p_iNewPort<0 || p_iNewPort>100000){
        p_iNewPort = m_iOscPortZirkonium;//18032;
    }
    
    m_iOscPortZirkonium = p_iNewPort;
    mOscIpAddress = "127.0.0.1";
    mOscSender.disconnect();
    if(!mOscSender.connect(mOscIpAddress, m_iOscPortZirkonium)){
        DBG("OSC cannot connect to " + mOscIpAddress);
    }
}
//void ZirkOscAudioProcessor::changeZirkoniumOSCPort(int newPort){
//    
//    if(newPort<0 || newPort>100000){
//        newPort = m_iOscPortZirkonium;//18032;
//    }
//    
//    lo_address osc = _OscZirkonium;
//    m_iOscPortZirkonium = newPort;
//    _OscZirkonium = NULL;
//    lo_address_free(osc);
//    
//    char port[32];
//    snprintf(port, sizeof(port), "%d", newPort);
//    _OscZirkonium = lo_address_new("127.0.0.1", port);
//    
//}
//OSC-----------------------

void ZirkOscAudioProcessor::initSources(){
    int i = 0, iId = 0;
    m_oAllSources[i++] = SoundSource(HRToPercent(  22.5, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max), 0, ++iId);
    m_oAllSources[i++] = SoundSource(HRToPercent( -22.5, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max), 0, ++iId);
    m_oAllSources[i++] = SoundSource(HRToPercent(  67.5, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max), 0, ++iId);
    m_oAllSources[i++] = SoundSource(HRToPercent( -67.5, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max), 0, ++iId);
    m_oAllSources[i++] = SoundSource(HRToPercent( 112.5, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max), 0, ++iId);
    m_oAllSources[i++] = SoundSource(HRToPercent(-112.5, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max), 0, ++iId);
    m_oAllSources[i++] = SoundSource(HRToPercent( 157.5, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max), 0, ++iId);
    m_oAllSources[i++] = SoundSource(HRToPercent(-157.5, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max), 0, ++iId);
    
    for(i = 0; i < 8; ++i){
        m_oAllSources[i].setPrevLoc01(m_oAllSources[i].getX01(), m_oAllSources[i].getY01());
    }
}

void ZirkOscAudioProcessor::updateSourcesSendOsc(){
    if (/*m_bCurrentlyPlaying && */!m_bIsRecordingAutomation && m_iMovementConstraint != Independent && m_iSourceLocationChanged != -1) {
        if (m_iMovementConstraint == DeltaLocked){
            moveDelta(m_iSourceLocationChanged, m_oAllSources[m_iSourceLocationChanged].getX(), m_oAllSources[m_iSourceLocationChanged].getY());
        } else if (m_iMovementConstraint == SymmetricX || m_iMovementConstraint == SymmetricY){
            moveSymmetric(m_iSourceLocationChanged, m_oAllSources[m_iSourceLocationChanged].getX(), m_oAllSources[m_iSourceLocationChanged].getY());
        } else {
            moveCircular(m_iSourceLocationChanged, m_oAllSources[m_iSourceLocationChanged].getX(), m_oAllSources[m_iSourceLocationChanged].getY());
        }        
        m_iSourceLocationChanged = -1;
    }
    //OSC---------------------------
    if (m_bIsOscActive){
        sendOSCValues();
    }
    //OSC---------------------------
}

void ZirkOscAudioProcessor::move(const int &p_iSource, const float &p_fX, const float &p_fY, const float &p_fAzim01, const float &p_fElev01){
    if (p_iSource > getNbrSources()){
        return;
    }
    //move selected source
    float fX01, fY01;
    JUCE_COMPILER_WARNING("if we have an azim and an elev, we ignore (and overwrite) the provided x and y. That's because we can always retreive valid x,y for any azim,elev")
    if (p_fAzim01 == -1 && p_fElev01 == -1){
        fX01 = HRToPercent(p_fX, -s_iDomeRadius, s_iDomeRadius);
        fY01 = HRToPercent(p_fY, -s_iDomeRadius, s_iDomeRadius);
        setParameterNotifyingHost (ZirkOscAudioProcessor::ZirkOSC_X_ParamId + p_iSource*5, fX01);
        setParameterNotifyingHost (ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + p_iSource*5, fY01);

    } else {
        SoundSource::azimElev01toXY01(p_fAzim01, p_fElev01, fX01, fY01);        
        setParameterNotifyingHost (ZirkOscAudioProcessor::ZirkOSC_X_ParamId + p_iSource*5, fX01);
        setParameterNotifyingHost (ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + p_iSource*5, fY01);
        JUCE_COMPILER_WARNING("critical that this is after the setParam because it overwrites the values (?)")
        m_oAllSources[p_iSource].setAzimuth01(p_fAzim01);
        m_oAllSources[p_iSource].setElevation01(p_fElev01);

        m_bNeedToRefreshGui = true;
    }
    
    
    //if non-independent, move non-selected sources
    if(m_iMovementConstraint == Independent || getNbrSources() == 1){
        m_oAllSources[p_iSource].setPrevLoc01(fX01, fY01, p_fAzim01, p_fElev01);
    } else if (m_iMovementConstraint == DeltaLocked){
        JUCE_COMPILER_WARNING("move with delta should use azim and elev")
        moveDelta(p_iSource, p_fX, p_fY);
    } else if (m_iMovementConstraint == SymmetricX || m_iMovementConstraint == SymmetricY){
        JUCE_COMPILER_WARNING("this should also use azim and elev?")
        moveSymmetric(p_iSource, p_fX, p_fY);
    } else {
        moveCircular(p_iSource, p_fX, p_fY, p_fAzim01, p_fElev01);
    }
    
    dynamic_cast<ZirkOscAudioProcessorEditor*>(m_oEditor)->updatePositionTrace(p_fX, p_fY);
}

void ZirkOscAudioProcessor::moveCircular(const int &p_iSelSource, const float &p_fSelectedNewX, const float &p_fSelectedNewY, const float &p_fAzim01, const float &p_fElev01){
    //calculate delta azim+elev for selected source
    float fSelectedDeltaAzim01, fSelectedDeltaElev01;
    tie(fSelectedDeltaAzim01,fSelectedDeltaElev01) = getDeltasForSelectedSource(p_iSelSource, p_fSelectedNewX, p_fSelectedNewY, p_fAzim01, p_fElev01);
    //return if no delta
    if (abs(fSelectedDeltaAzim01) < .000001 && abs(fSelectedDeltaElev01) < .000001){
//    if (fSelectedDeltaAzim01 == 0 && fSelectedDeltaElev01 == 0){
        return;
    } else {
        float prevX01, prevY01;
        m_oAllSources[p_iSelSource].getPrevXY01(prevX01, prevY01);
        tie(fSelectedDeltaAzim01,fSelectedDeltaElev01) = getDeltasForSelectedSource(p_iSelSource, p_fSelectedNewX, p_fSelectedNewY, p_fAzim01, p_fElev01);
    }
    
    
    
    
    //move non-selected sources using the deltas
    JUCE_COMPILER_WARNING("instead of applying the delta from the selected source linearly, we could probably do it with relation to the center, so that when going towards center, non-selected sources reach the middle at the same time as the selected source")
    for (int iCurSource = 0; iCurSource < getNbrSources(); ++iCurSource) {
        if (iCurSource == p_iSelSource){
            //save new values as old values for next time
            m_oAllSources[p_iSelSource].setPrevLoc01(HRToPercent(p_fSelectedNewX, -s_iDomeRadius, s_iDomeRadius), HRToPercent(p_fSelectedNewY, -s_iDomeRadius, s_iDomeRadius), p_fAzim01, p_fElev01);
            continue;
        }
        //get current, non-selected source position
        float fCurAzim01, fCurElev01;
        tie(fCurAzim01, fCurElev01) = getCurrentSourcePosition(iCurSource);
        //calculate new position
        float fNewX01, fNewY01, fNewAzim01, fNewElev01;
        tie(fNewX01, fNewY01, fNewAzim01, fNewElev01) = getNewSourcePosition(p_iSelSource, fSelectedDeltaAzim01, fSelectedDeltaElev01, iCurSource, fCurAzim01, fCurElev01);
        //move source
        m_oAllSources[iCurSource].setXYAzimElev01(fNewX01, fNewY01, fNewAzim01, fNewElev01);
        JUCE_COMPILER_WARNING("there needs to be a way to do those 2 lines")
//        m_oAllSources[iCurSource].setAzimuth01(p_fAzim01);
//        m_oAllSources[iCurSource].setElevation01(p_fElev01);
    }
}

pair<float, float> ZirkOscAudioProcessor::getDeltasForSelectedSource(const int &p_iSource, const float &p_fSelectedNewX, const float &p_fSelectedNewY, const float &p_fAzim01, const float &p_fElev01){
    //get previous position
    float fSelectedPrevAzim01 = m_oAllSources[p_iSource].getPrevAzim01();
    float fSelectedPrevElev01 = m_oAllSources[p_iSource].getPrevElev01();
    //get new position
    float fSelectedNewAzim01, fSelectedNewElev01;
    if (p_fAzim01 == -1 && p_fElev01 == -1){
        fSelectedNewAzim01 = SoundSource::XYtoAzim01(p_fSelectedNewX, p_fSelectedNewY);
        fSelectedNewElev01 = SoundSource::XYtoElev01(p_fSelectedNewX, p_fSelectedNewY);
    } else {
        fSelectedNewAzim01 = p_fAzim01;
        fSelectedNewElev01 = p_fElev01;
    }
    //return the delta
    return make_pair(fSelectedNewAzim01 - fSelectedPrevAzim01, fSelectedNewElev01 - fSelectedPrevElev01);
}

pair<float, float> ZirkOscAudioProcessor::getCurrentSourcePosition(int iCurSource){
    float fCurAzim01, fCurElev01;
    float fCurX = getParameter(ZirkOscAudioProcessor::ZirkOSC_X_ParamId + (iCurSource*5)) * 2 * s_iDomeRadius - s_iDomeRadius;
    float fCurY = getParameter(ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + (iCurSource*5)) * 2 * s_iDomeRadius - s_iDomeRadius;
    
    ElevationStatus elevationStatus = m_oAllSources[iCurSource].getElevationStatus();
    if (elevationStatus == normalRange){
        fCurElev01 = SoundSource::XYtoElev01(fCurX, fCurY);
        fCurAzim01 = SoundSource::XYtoAzim01(fCurX, fCurY);
    } else  {
        float fCurElevOverflow = (m_oAllSources[iCurSource].getElevOverflow() - s_iDomeRadius) / s_iDomeRadius;
        if (elevationStatus == over1){
            fCurAzim01 = m_oAllSources[iCurSource].getPrevAzim01();
            fCurElev01 = radianToDegree(acos(fCurElevOverflow));
        } else if (elevationStatus == under0){
            fCurAzim01 = SoundSource::XYtoAzim01(fCurX, fCurY);
            fCurElev01 = radianToDegree(-asin(fCurElevOverflow));                       //need to convert output of asin which is is radians, to degree
        } else {
            jassert(0);
        }
        fCurElev01 = HRToPercent(fCurElev01, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);   //then to percent
    }
    return make_pair(fCurAzim01, fCurElev01);
}

tuple<float, float, float, float> ZirkOscAudioProcessor::getNewSourcePosition(const int &p_iSelSource, const float &fSelectedDeltaAzim01, const float &fSelectedDeltaElev01, const int &iCurSource, const float &fCurAzim01, const float &fCurElev01){
    float fNewX01, fNewY01, fCurElevOverflow;
    //figure azim
    float fNewAzim01 = checkAndFixAzim01Bounds(fCurAzim01 + fSelectedDeltaAzim01);

    //figure elevation
    float fNewElev01 = fCurElev01 + fSelectedDeltaElev01;
    if (fNewElev01 >= 1 - std::numeric_limits<float>::epsilon()){
        m_oAllSources[iCurSource].setElevationStatus(over1);
        fCurElevOverflow = s_iDomeRadius + s_iDomeRadius * cos(degreeToRadian(PercentToHR(fNewElev01, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max)));
        fNewElev01 = 1;
        SoundSource::azimElev01toXY01(fNewAzim01, fNewElev01, fNewX01, fNewY01, fCurElevOverflow);
    }
    else if (fNewElev01 < 0){                   //moving selected source moves this source out of the dome. need to calculate overflow
        m_oAllSources[iCurSource].setElevationStatus(under0);
        fCurElevOverflow = s_iDomeRadius - s_iDomeRadius * sin(degreeToRadian(PercentToHR(fNewElev01, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max)));
        fNewElev01 = 0;
        SoundSource::azimElev01toXY01(fNewAzim01, fNewElev01, fNewX01, fNewY01, fCurElevOverflow);
    }
    else {  //normal range
        m_oAllSources[iCurSource].setElevationStatus(normalRange);
        SoundSource::azimElev01toXY01(fNewAzim01, fNewElev01, fNewX01, fNewY01);
        fCurElevOverflow = s_iDomeRadius;
    }
    
    m_oAllSources[iCurSource].setElevOverflow(fCurElevOverflow);
    m_oAllSources[iCurSource].setPrevLoc01(fNewX01, fNewY01, fNewAzim01, fNewElev01);
    return make_tuple(fNewX01, fNewY01, fNewAzim01, fNewElev01);
}

void ZirkOscAudioProcessor::moveDelta(const int &p_iSource, const float &p_fX, const float &p_fY){
    //calculate delta for selected source, which was already moved in ::move()
    float fSelectedOldX01, fSelectedOldY01;
    m_oAllSources[p_iSource].getPrevXY01(fSelectedOldX01, fSelectedOldY01);
    float fSelectedDeltaX01 = HRToPercent(p_fX, -s_iDomeRadius, s_iDomeRadius) - fSelectedOldX01;
    float fSelectedDeltaY01 = HRToPercent(p_fY, -s_iDomeRadius, s_iDomeRadius) - fSelectedOldY01;
    
    //move unselected sources to their current position + deltamove
    for(int iCurSrc=0; iCurSrc<getNbrSources(); ++iCurSrc){
        
        if (iCurSrc == p_iSource){
            //save old values for selected source
            m_oAllSources[p_iSource].setPrevLoc01(HRToPercent(p_fX, -s_iDomeRadius, s_iDomeRadius), HRToPercent(p_fY, -s_iDomeRadius, s_iDomeRadius));
            continue;
        }
        
        float newX01 = getSources()[iCurSrc].getX01() + fSelectedDeltaX01;
        float newY01 = getSources()[iCurSrc].getY01() + fSelectedDeltaY01;

        m_oAllSources[iCurSrc].setXYAzimElev01(newX01, newY01);
        m_oAllSources[iCurSrc].setPrevLoc01(newX01, newY01);
    }
}
//source here is the selected source, p_fX and p_fY is the position that it was already moved to, in ::move()
void ZirkOscAudioProcessor::moveSymmetric(const int &p_iSource, const float &p_fX, const float &p_fY){

    //selected source position in 01 range
    float fSelectedX01 = HRToPercent(p_fX, -s_iDomeRadius, s_iDomeRadius);
    float fSelectedY01 = HRToPercent(p_fY, -s_iDomeRadius, s_iDomeRadius);
    
    //move the other, unselected source
    for(int iCurSrc = 0; iCurSrc<getNbrSources(); ++iCurSrc){
        if (iCurSrc == p_iSource){
            //save old values for selected source
            m_oAllSources[p_iSource].setPrevLoc01(HRToPercent(p_fX, -s_iDomeRadius, s_iDomeRadius), HRToPercent(p_fY, -s_iDomeRadius, s_iDomeRadius));
            continue;
        }

        float newX01 = fSelectedX01;
        float newY01 = fSelectedY01;
        
        if (getMovementConstraint() == SymmetricX){
            newY01 = 1-newY01;
        } else {
            newX01 = 1-newX01;
        }
        
        m_oAllSources[iCurSrc].setXYAzimElev01(newX01, newY01);
        m_oAllSources[iCurSrc].setPrevLoc01(newX01, newY01);
    }
}

int IndexedAngleCompare(const void *a, const void *b){
    IndexedAngle *ia = (IndexedAngle*)a;
    IndexedAngle *ib = (IndexedAngle*)b;
    return (ia->a < ib->a) ? -1 : ((ia->a > ib->a) ? 1 : 0);
}

void ZirkOscAudioProcessor::setEqualAzimForAllSrc(){
    
    vector<int> fSortedAzims = getOrderSources();
    
    int   fSelPos   = fSortedAzims[m_iSelectedSource];
    float fSelAzim  = m_oAllSources[m_iSelectedSource].getAzimuth01();
    float fEqualDelta    = 1.f / m_iNbrSources;
    
    for(int iCurSrc = 0; iCurSrc < m_iNbrSources; ++iCurSrc){
        if (iCurSrc == m_iSelectedSource){
            continue;
        }
        
        int iCurPos         = fSortedAzims[iCurSrc];
        int iCurDistance    = iCurPos - fSelPos;
        if (iCurDistance < 1) iCurDistance += m_iNbrSources;
        float fCurDelta     = iCurDistance * fEqualDelta;
        float fCurAngle     = fmodf(fSelAzim + fCurDelta, 1);
        float fCurElev = m_oAllSources[iCurSrc].getElevation01();
        setCurrentAndOldLocation(iCurSrc, fCurAngle, fCurElev);
    }
}

void ZirkOscAudioProcessor::setEqualAzimElevForAllSrc(){
    
    vector<int> fSortedAzims = getOrderSources();
    
    int   fSelPos   = fSortedAzims[m_iSelectedSource];
    float fSelAzim  = m_oAllSources[m_iSelectedSource].getAzimuth01();
    float fSelElev  = m_oAllSources[m_iSelectedSource].getElevation01();
    float fEqualDelta    = 1.f / m_iNbrSources;
    
    for(int iCurSrc = 0; iCurSrc < m_iNbrSources; ++iCurSrc){
        if (iCurSrc == m_iSelectedSource){
            continue;
        }
        
        int iCurPos         = fSortedAzims[iCurSrc];
        int iCurDistance    = iCurPos - fSelPos;
        if (iCurDistance < 1) iCurDistance += m_iNbrSources;
        float fCurDelta     = iCurDistance * fEqualDelta;
        float fCurAngle     = fmodf(fSelAzim + fCurDelta, 1);
        setCurrentAndOldLocation(iCurSrc, fCurAngle, fSelElev);
    }

}

void ZirkOscAudioProcessor::setEqualElevForAllSrc(){
    float fSelElevation01 = m_oAllSources[m_iSelectedSource].getElevation01();
    for(int iCurSrc = 0; iCurSrc < getNbrSources() ; ++iCurSrc){
        float fCurAzim01 = m_oAllSources[iCurSrc].getAzimuth01();
        setCurrentAndOldLocation(iCurSrc, fCurAzim01, fSelElevation01);
    }
}

void ZirkOscAudioProcessor::setSymmetricForAllSrc(){
    float fX01 = m_oAllSources[m_iSelectedSource].getX01();
    float fY01 = m_oAllSources[m_iSelectedSource].getY01();
    for(int iCurSrc = 0; iCurSrc < getNbrSources(); ++iCurSrc){
        if (iCurSrc != m_iSelectedSource){
            if (getMovementConstraint() == SymmetricX){
                fY01 = 1-fY01;
            } else {
                fX01 = 1-fX01;
            }
            setParameterNotifyingHost (ZirkOscAudioProcessor::ZirkOSC_X_ParamId + (iCurSrc*5), fX01);
            setParameterNotifyingHost (ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + (iCurSrc*5), fY01);
            m_oAllSources[iCurSrc].setPrevLoc01(fX01, fY01);
        }
    }
}



void ZirkOscAudioProcessor::setCurrentAndOldLocation(const int &p_iSrc, const float &p_fAzim01, const float &p_fElev01){
    float fX01, fY01;
    SoundSource::azimElev01toXY01(p_fAzim01, p_fElev01, fX01, fY01);
    setParameterNotifyingHost (ZirkOscAudioProcessor::ZirkOSC_X_ParamId + (p_iSrc*5), fX01);
    setParameterNotifyingHost (ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + (p_iSrc*5), fY01);
    m_oAllSources[p_iSrc].setPrevLoc01(fX01, fY01, p_fAzim01, p_fElev01);
}




vector<int> ZirkOscAudioProcessor::getOrderSources(){
    //gather unsorted information
    std::vector<IndexedAngle> indexedAngles (m_iNbrSources);
    for (int iCurSrc = 0; iCurSrc < m_iNbrSources; iCurSrc++) {
        indexedAngles[iCurSrc].i = iCurSrc;
        indexedAngles[iCurSrc].a = m_oAllSources[iCurSrc].getAzimuth01();
    }
    //sort and return sorted angles
    qsort(&indexedAngles[0], m_iNbrSources, sizeof(IndexedAngle), IndexedAngleCompare);
    
    std::vector<int> sourceOrder(m_iNbrSources);
    for (int iCurPos = 0; iCurPos < m_iNbrSources; ++iCurPos) {
        int iCurSrc = indexedAngles[iCurPos].i;
        sourceOrder[iCurSrc] = iCurPos;
    }
    return sourceOrder;
}


ZirkOscAudioProcessor::~ZirkOscAudioProcessor()
{
    //OSC------------
//    m_bIsOscActive = false;
//    lo_address osc = _OscZirkonium;
//    if (osc){
//        lo_address_free(osc);
//    }
//    _OscZirkonium = NULL;
    //OSC------------

    if (m_bStartedConstraintAutomation){
        endParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_MovementConstraint_ParamId);
    }
    
    if (m_pSourceUpdateThread){
        delete m_pSourceUpdateThread;
    }
}

//==============================================================================
void ZirkOscAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    if (s_bForceConstraintAutomation && host.isReaper()){
        beginParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_MovementConstraint_ParamId);
    }
}

void ZirkOscAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages){

    AudioPlayHead::CurrentPositionInfo cpi;
    getPlayHead()->getCurrentPosition(cpi);
    
    if (cpi.isPlaying){
        if (!m_bDetectedPlayingStart){
            m_bCurrentlyPlaying = true;
            m_bDetectedPlayingStart = true;
            m_bDetectedPlayingEnd = false;
            if (s_bForceConstraintAutomation && host.isReaper()){
                m_iActualConstraint = getMovementConstraint();
                int iTempConstraint = (m_iActualConstraint == DeltaLocked) ? m_iActualConstraint -1 : m_iActualConstraint +1;
                setParameterNotifyingHost((ZirkOscAudioProcessor::ZirkOSC_MovementConstraint_ParamId), IntToPercentStartsAtOne(iTempConstraint, TotalNumberConstraints));
                m_iNeedToResetToActualConstraint = 25;
            }
        } else if (s_bForceConstraintAutomation &&  host.isReaper() && --m_iNeedToResetToActualConstraint == 0){
            setParameterNotifyingHost((ZirkOscAudioProcessor::ZirkOSC_MovementConstraint_ParamId), IntToPercentStartsAtOne(m_iActualConstraint, TotalNumberConstraints));
            m_iNeedToResetToActualConstraint = -1;
        }
    } else if (!cpi.isPlaying && !m_bDetectedPlayingEnd){
        m_bCurrentlyPlaying = false;
        m_bDetectedPlayingEnd = true;
        m_bDetectedPlayingStart = false;
    }

    Trajectory::Ptr trajectory = mTrajectory;
    if (trajectory) {
        if (cpi.isPlaying && cpi.timeInSamples != mLastTimeInSamples) {
            // we're playing!
            mLastTimeInSamples = cpi.timeInSamples;
            
            double bps = cpi.bpm / 60;
            double sampleRate = getSampleRate();
            unsigned int oriFramesToProcess = buffer.getNumSamples();
            float seconds = oriFramesToProcess / sampleRate;
            float beats = seconds * bps;
            
            bool done = trajectory->process(seconds, beats);
            if (done){
                mTrajectory = NULL;
                m_bIsWriteTrajectory = false;
            }
        }
    }
}

void ZirkOscAudioProcessor::updatePositions(){
    for (int iCurSource = 0; iCurSource<8; ++iCurSource){
        m_oAllSources[iCurSource].updatePosition();
    }
}

void ZirkOscAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    if (s_bForceConstraintAutomation && host.isReaper()){
        endParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_MovementConstraint_ParamId);
    }
}

void ZirkOscAudioProcessor::storeCurrentLocations(){
    for (int iCurSource = 0; iCurSource<8; ++iCurSource){
        m_oAllSourcesBuffer[iCurSource] = m_oAllSources[iCurSource];
    }
}

void ZirkOscAudioProcessor::restoreCurrentLocations(){
    for (int iCurSource = 0; iCurSource<8; ++iCurSource){
        m_oAllSources[iCurSource] = m_oAllSourcesBuffer[iCurSource];
    }
}

const String ZirkOscAudioProcessor::getParameterText (int index)
{
    return String (getParameter (index), 2);
}

const String ZirkOscAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String ZirkOscAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool ZirkOscAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool ZirkOscAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool ZirkOscAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool ZirkOscAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool ZirkOscAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

int ZirkOscAudioProcessor::getNumPrograms()
{
    return 1;
}

int ZirkOscAudioProcessor::getCurrentProgram()
{
    return 1;
}

void ZirkOscAudioProcessor::setCurrentProgram (int index)
{
}

const String ZirkOscAudioProcessor::getProgramName (int index)
{
    return String::empty;
}

void ZirkOscAudioProcessor::changeProgramName (int index, const String& newName)
{

}

void ZirkOscAudioProcessor::setSelectedSourceForTrajectory(int iSelectedSource){
    m_iSelectedSourceForTrajectory = iSelectedSource;
}

int ZirkOscAudioProcessor::getSelectedSourceForTrajectory(){
    return m_iSelectedSourceForTrajectory;
}

//==============================================================================
bool ZirkOscAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ZirkOscAudioProcessor::createEditor()
{
    m_oEditor = new ZirkOscAudioProcessorEditor (this);
    return m_oEditor;
}


void ZirkOscAudioProcessor::setLastUiWidth(int lastUiWidth)
{
    _LastUiWidth = lastUiWidth;
}

int ZirkOscAudioProcessor::getLastUiWidth()
{
    return _LastUiWidth;
}
void ZirkOscAudioProcessor::setLastUiHeight(int lastUiHeight)
{
    _LastUiHeight = lastUiHeight;
}

int ZirkOscAudioProcessor::getLastUiHeight()
{
    return _LastUiHeight;
}


//set wheter plug is sending osc messages to zirkonium
void ZirkOscAudioProcessor::setIsOscActive(bool isOscActive){
    m_bIsOscActive = isOscActive;
}

//wheter plug is sending osc messages to zirkonium
bool ZirkOscAudioProcessor::getIsOscActive(){
    return m_bIsOscActive;
}

void ZirkOscAudioProcessor::setIsSyncWTempo(bool isSyncWTempo){
    m_bIsSyncWTempo = isSyncWTempo;
}

bool ZirkOscAudioProcessor::getIsSyncWTempo(){
    return m_bIsSyncWTempo;
}


void ZirkOscAudioProcessor::setIsSpanLinked(bool isSpanLinked){
    m_bIsSpanLinked = isSpanLinked;
}

bool ZirkOscAudioProcessor::getIsSpanLinked(){
    return m_bIsSpanLinked;
}

void ZirkOscAudioProcessor::setIsWriteTrajectory(bool isWriteTrajectory){
    m_bIsWriteTrajectory = isWriteTrajectory;
}

bool ZirkOscAudioProcessor::getIsWriteTrajectory(){
    return m_bIsWriteTrajectory;
}

//==============================================================================
const String ZirkOscAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int ZirkOscAudioProcessor::getNumParameters()
{
    return totalNumParams;
}

// This method will be called by the host, probably on the audio thread, so
// it's absolutely time-critical. Don't use critical sections or anything
// UI-related, or anything at all that may block in any way!
float ZirkOscAudioProcessor::getParameter (int index)
{
    switch (index){
        case ZirkOSC_MovementConstraint_ParamId:
            return m_fMovementConstraint;
        case ZirkOSC_isOscActive_ParamId:
            if (m_bIsOscActive)
                return 1.0f;
            else
                return 0.0f;
        case ZirkOSC_isSpanLinked_ParamId:
            if (m_bIsSpanLinked)
                return 1.0f;
            else
                return 0.0f;
        case ZirkOSC_SelectedTrajectory_ParamId:
            return m_fSelectedTrajectory;
        case ZirkOSC_SelectedTrajectoryDirection_ParamId:
            return m_fSelectedTrajectoryDirection;
        case ZirkOSC_SelectedTrajectoryReturn_ParamId:
            return m_fSelectedTrajectoryReturn;
        case ZirkOSCm_dTrajectoryCount_ParamId:
            return m_dTrajectoryCount;
        case ZirkOSC_TrajectoriesDuration_ParamId:
            return m_dTrajectoriesDuration;
        case ZirkOSC_SyncWTempo_ParamId:
            if (m_bIsSyncWTempo)
                return 1.0f;
            else
                return 0.0f;
        case ZirkOSC_WriteTrajectories_ParamId:
            if (m_bIsWriteTrajectory)
                return 1.0f;
            else
                return 0.0f;
    }
    
    for(int iCurSrc = 0; iCurSrc< 8; ++iCurSrc){
        
        if (ZirkOSC_X_ParamId + (iCurSrc*5) == index) {
            return (m_oAllSources[iCurSrc].getX01());
        }
        else if (ZirkOSC_Y_ParamId + (iCurSrc*5) == index){
            return (m_oAllSources[iCurSrc].getY01());
        }
        else if (ZirkOSC_AzimSpan_ParamId + (iCurSrc*5) == index){
            return m_oAllSources[iCurSrc].getAzimuthSpan();
        }
        else if (ZirkOSC_ElevSpan_ParamId + (iCurSrc*5) == index){
            return m_oAllSources[iCurSrc].getElevationSpan();
        }
        else if (ZirkOSC_Gain_ParamId + (iCurSrc*5) == index){
            return m_oAllSources[iCurSrc].getGain01();
        }
    }
    DBG("wrong parameter id: " << index << "in ZirkOscAudioProcessor::getParameter" << "\n");
    return -1.f;
}

// This method will be called by the host, probably on the audio thread, so
// it's absolutely time-critical. Don't use critical sections or anything
// UI-related, or anything at all that may block in any way!
void ZirkOscAudioProcessor::setParameter (int index, float newValue){
    if (!setPositionParameters(index, newValue) && !setOtherParameters(index, newValue)){
        m_bNeedToRefreshGui = false;
//        setPositionParameters(index, newValue);
//        setOtherParameters(index, newValue);
        DBG("wrong parameter id: " << index << " in ZirkOscAudioProcessor::setParameter\n");
    }
}

bool ZirkOscAudioProcessor::setOtherParameters(int index, float newValue){
    switch (index){
        case ZirkOSC_MovementConstraint_ParamId:
            if (m_fMovementConstraint != newValue){
                setMovementConstraint(newValue);
                m_bNeedToRefreshGui = true;
            }
            return true;
        case ZirkOSC_isOscActive_ParamId:
            if (newValue > .5f && !m_bIsOscActive){
                m_bIsOscActive = true;
                m_bNeedToRefreshGui = true;
            } else if (m_bIsOscActive){
                m_bIsOscActive = false;
                m_bNeedToRefreshGui = true;
            }
            return true;
        case ZirkOSC_isSpanLinked_ParamId:
            if (newValue > .5f && !m_bIsSpanLinked){
                m_bIsSpanLinked = true;
                m_bNeedToRefreshGui = true;
            } else if (m_bIsSpanLinked){
                m_bIsSpanLinked = false;
                m_bNeedToRefreshGui = true;
            }
            return true;
        case ZirkOSC_SelectedTrajectory_ParamId:
            if (m_fSelectedTrajectory != newValue){
                m_fSelectedTrajectory = newValue;
                m_bNeedToRefreshGui = true;
            }
            return true;
        case ZirkOSC_SelectedTrajectoryDirection_ParamId:
            if(m_fSelectedTrajectoryDirection != newValue){
                m_fSelectedTrajectoryDirection = newValue;
                m_bNeedToRefreshGui = true;
            }
            return true;
        case ZirkOSC_SelectedTrajectoryReturn_ParamId:
            if(m_fSelectedTrajectoryReturn != newValue){
                m_fSelectedTrajectoryReturn = newValue;
                m_bNeedToRefreshGui = true;
            }
            return true;
        case ZirkOSCm_dTrajectoryCount_ParamId:
            JUCE_COMPILER_WARNING("is this ever used??")
            if(m_dTrajectoryCount != newValue){
                m_dTrajectoryCount = newValue;
                m_bNeedToRefreshGui = true;
            }
            return true;
        case ZirkOSC_TrajectoriesDuration_ParamId:
            if(m_dTrajectoriesDuration != newValue){
                m_dTrajectoriesDuration = newValue;
                m_bNeedToRefreshGui = true;
            }
            return true;
        case ZirkOSC_SyncWTempo_ParamId:
            if (newValue > .5f && !m_bIsSyncWTempo){
                m_bIsSyncWTempo = true;
                m_bNeedToRefreshGui = true;
            } else if (m_bIsSyncWTempo){
                m_bIsSyncWTempo = false;
                m_bNeedToRefreshGui = true;
            }
            return true;
        case ZirkOSC_WriteTrajectories_ParamId:
            if (newValue > .5f)
                m_bIsWriteTrajectory = true;
            else
                m_bIsWriteTrajectory = false;
            return true;
    }
    return false;
}

bool ZirkOscAudioProcessor::setPositionParameters(int index, float newValue){
    for(int iCurSource = 0; iCurSource < 8; ++iCurSource){
        if(ZirkOSC_X_ParamId + (iCurSource*5) == index){
            if(newValue != m_oAllSources[iCurSource].getX01()) {
                m_oAllSources[iCurSource].setX01(newValue);
                m_iSourceLocationChanged = iCurSource;
                m_bNeedToRefreshGui = true;
            }
            return true;
        }
        else if (ZirkOSC_Y_ParamId + (iCurSource*5) == index){
            if(newValue != m_oAllSources[iCurSource].getY01()) {
                m_oAllSources[iCurSource].setY01(newValue);
                m_iSourceLocationChanged = iCurSource;
                m_bNeedToRefreshGui = true;
            }
            return true;
        } else if (ZirkOSC_AzimSpan_ParamId + (iCurSource*5) == index){
            if (newValue != m_oAllSources[iCurSource].getAzimuthSpan()){
            m_oAllSources[iCurSource].setAzimuthSpan(newValue);
            m_bNeedToRefreshGui = true;
            }
            return true;
        }
        else if (ZirkOSC_ElevSpan_ParamId + (iCurSource*5) == index){
            if (newValue != m_oAllSources[iCurSource].getElevationSpan()){
                m_oAllSources[iCurSource].setElevationSpan(newValue);
                m_bNeedToRefreshGui = true;
            }
            return true;
        }
        else if (ZirkOSC_Gain_ParamId + (iCurSource*5) == index){
            if (newValue != m_oAllSources[iCurSource].getGain01()){
                m_oAllSources[iCurSource].setGain01(newValue);
                m_bNeedToRefreshGui = true;
            }
            return true;
        }
    }
    return false;
}

void ZirkOscAudioProcessor::setMovementConstraint(float p_fConstraint){
    m_fMovementConstraint = p_fConstraint;
    m_iMovementConstraint = PercentToIntStartsAtOne(m_fMovementConstraint, TotalNumberConstraints);
}

void ZirkOscAudioProcessor::setMovementConstraint(int p_iConstraint){
    m_iMovementConstraint = p_iConstraint;
    m_fMovementConstraint = IntToPercentStartsAtOne(m_iMovementConstraint, TotalNumberConstraints);
}

const String ZirkOscAudioProcessor::getParameterName (int index) {
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
        case ZirkOSCm_dTrajectoryCount_ParamId:
            return ZirkOSC_NbrTrajectories_name;
        case ZirkOSC_TrajectoriesDuration_ParamId:
            return ZirkOSC_DurationTrajectories_name;
        case ZirkOSC_SyncWTempo_ParamId:
            return ZirkOSCm_bIsSyncWTempo_name;
        case ZirkOSC_WriteTrajectories_ParamId:
            return ZirkOSCm_bIsWriteTrajectory_name;
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

static const int s_kiDataVersion = 4;

//==============================================================================
void ZirkOscAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    
    XmlElement xml ("ZIRKOSCJUCESETTINGS");
    xml.setAttribute("uiWidth", _LastUiWidth);
    xml.setAttribute("uiHeight", _LastUiHeight);
    xml.setAttribute("PortOSC", m_iOscPortZirkonium);
    xml.setAttribute("NombreSources", m_iNbrSources);
    xml.setAttribute("MovementConstraint", m_fMovementConstraint);
    xml.setAttribute("isSpanLinked", m_bIsSpanLinked);
    xml.setAttribute("isOscActive", m_bIsOscActive);
    xml.setAttribute("selectedTrajectory", m_fSelectedTrajectory);
    xml.setAttribute("nbrTrajectory", m_dTrajectoryCount);
    xml.setAttribute("durationTrajectory", m_dTrajectoriesDuration);
    xml.setAttribute("isSyncWTempo", m_bIsSyncWTempo);
    xml.setAttribute("isWriteTrajectory", m_bIsWriteTrajectory);
    xml.setAttribute("selectedTrajectoryDirection", m_fSelectedTrajectoryDirection);
    xml.setAttribute("selectedTrajectoryReturn", m_fSelectedTrajectoryReturn);
    xml.setAttribute("presetDataVersion", s_kiDataVersion);
    xml.setAttribute("endLocationAzim", m_fEndLocationXY.first);
    xml.setAttribute("endLocationElev", m_fEndLocationXY.second);
    xml.setAttribute("turns", m_dTrajectoryTurns);
    xml.setAttribute("deviation", m_dTrajectoryDeviation);
    xml.setAttribute("dampening", m_dTrajectoryDampening);
    
    for(int iCurSrc = 0; iCurSrc < 8; ++iCurSrc){
        String channel      = "Channel"         + to_string(iCurSrc);
        String azimuthSpan  = "AzimuthSpan"     + to_string(iCurSrc);
        String elevationSpan= "ElevationSpan"   + to_string(iCurSrc);
        String gain         = "Gain"            + to_string(iCurSrc);
        String strX         = "X"               + to_string(iCurSrc);
        String strY         = "Y"               + to_string(iCurSrc);
        String strAzim01    = "Azim01"          + to_string(iCurSrc);
        String strElev01    = "Elev01"          + to_string(iCurSrc);
        
        xml.setAttribute(channel,       m_oAllSources[iCurSrc].getSourceId());
        xml.setAttribute(azimuthSpan,   m_oAllSources[iCurSrc].getAzimuthSpan());
        xml.setAttribute(elevationSpan, m_oAllSources[iCurSrc].getElevationSpan());
        xml.setAttribute(gain,          m_oAllSources[iCurSrc].getGain01());
        xml.setAttribute(strX,          m_oAllSources[iCurSrc].getX01());
        xml.setAttribute(strY,          m_oAllSources[iCurSrc].getY01());
        xml.setAttribute(strAzim01,     m_oAllSources[iCurSrc].getAzimuth01());
        xml.setAttribute(strElev01,     m_oAllSources[iCurSrc].getElevation01());
    }
    copyXmlToBinary (xml, destData);
}

void ZirkOscAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState != nullptr && xmlState->hasTagName ("ZIRKOSCJUCESETTINGS")) {
        
        int version = xmlState->getIntAttribute("presetDataVersion", 1);
        
        if (version == 1 ){
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "ZirkOSC - Loading preset/state from an older version",
                                              "You are attempting to load ZirkOSC with a preset from an older version.\nStored source locations are unreadable by this version, so default values will be used.",
                                              "OK");
        } else if (version > s_kiDataVersion){
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "ZirkOSC - Loading preset/state from a newer version",
                                              "You are attempting to load ZirkOSC with a preset from an newer version.\nDefault values will be used for all parameters",
                                              "OK");
            return;
        }
        
        _LastUiWidth                    = xmlState->getIntAttribute ("uiWidth", _LastUiWidth);
        _LastUiHeight                   = xmlState->getIntAttribute ("uiHeight", _LastUiHeight);
        m_iOscPortZirkonium             = xmlState->getIntAttribute("PortOSC", 18032);
        m_iNbrSources                   = xmlState->getIntAttribute("NombreSources", 1);
        float fMovementConstraint       = xmlState->getDoubleAttribute("MovementConstraint", .0f);
        setMovementConstraint(fMovementConstraint >= 0 ? fMovementConstraint : 0);
        m_bIsOscActive                  = xmlState->getBoolAttribute("isOscActive", true);
        m_bIsSpanLinked                 = xmlState->getBoolAttribute("isSpanLinked", false);
        m_fSelectedTrajectory           = static_cast<float>(xmlState->getDoubleAttribute("selectedTrajectory", .0f));
        m_dTrajectoryCount              = xmlState->getIntAttribute("nbrTrajectory", 0);
        m_dTrajectoriesDuration         = static_cast<float>(xmlState->getDoubleAttribute("durationTrajectory", .0f));
        m_bIsSyncWTempo                 = xmlState->getBoolAttribute("isSyncWTempo", false);
        m_bIsWriteTrajectory            = xmlState->getBoolAttribute("isWriteTrajectory", false);
        m_fEndLocationXY.first          = xmlState->getDoubleAttribute("endLocationAzim", 180.0);
        m_fEndLocationXY.second         = xmlState->getDoubleAttribute("endLocationElev", 90.0);
        m_dTrajectoryTurns              = xmlState->getDoubleAttribute("turns", m_dTrajectoryTurns);
        m_dTrajectoryDeviation          = xmlState->getDoubleAttribute("deviation", m_dTrajectoryDeviation);
        m_dTrajectoryDampening          = xmlState->getDoubleAttribute("dampening", m_dTrajectoryDampening);
        
        for (int iCurSrc = 0; iCurSrc < 8; ++iCurSrc){
            String channel      = "Channel"         + to_string(iCurSrc);
            String azimuthSpan  = "AzimuthSpan"     + to_string(iCurSrc);
            String elevationSpan= "ElevationSpan"   + to_string(iCurSrc);
            String gain         = "Gain"            + to_string(iCurSrc);
            String strX         = "X"               + to_string(iCurSrc);
            String strY         = "Y"               + to_string(iCurSrc);
            String strAzim01    = "Azim01"          + to_string(iCurSrc);
            String strElev01    = "Elev01"          + to_string(iCurSrc);


            m_oAllSources[iCurSrc].setSourceId(xmlState->getIntAttribute(channel , 0));
            m_oAllSources[iCurSrc].setAzimuthSpan(    static_cast<float>(xmlState->getDoubleAttribute(azimuthSpan,0)));
            m_oAllSources[iCurSrc].setElevationSpan(  static_cast<float>(xmlState->getDoubleAttribute(elevationSpan,0)));
            m_oAllSources[iCurSrc].setGain01(           static_cast<float>(xmlState->getDoubleAttribute(gain, 1)));
            
            //calculate default value, in case we cannot find the actual values
            float fDefaultX, fDefaultY;
            SoundSource::azimElev01toXY01(iCurSrc * .125, 0, fDefaultX, fDefaultY);
            
            //fetch actual values
            float fActualX01 = static_cast<float>(xmlState->getDoubleAttribute(strX, fDefaultX));
            float fActualY01 = static_cast<float>(xmlState->getDoubleAttribute(strY, fDefaultY));
            float fDefaultAzim01, fDefaultElev01;
            SoundSource::XY01toAzimElev01(fActualX01, fActualY01, fDefaultAzim01, fDefaultElev01);
            float fAzim01    = static_cast<float>(xmlState->getDoubleAttribute(strAzim01, fDefaultAzim01));
            float fElev01    = static_cast<float>(xmlState->getDoubleAttribute(strElev01, fDefaultElev01));
            
            setParameter (ZirkOscAudioProcessor::ZirkOSC_X_ParamId + iCurSrc*5, fActualX01);
            setParameter (ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + iCurSrc*5, fActualY01);
            m_oAllSources[iCurSrc].setAzimuth01(fAzim01);
            m_oAllSources[iCurSrc].setElevation01(fElev01);
            m_oAllSources[iCurSrc].setPrevLoc01(fActualX01, fActualY01, fAzim01, fElev01);
        }
        
        m_fSelectedTrajectoryDirection = static_cast<float>(xmlState->getDoubleAttribute("selectedTrajectoryDirection", .0f));
        m_fSelectedTrajectoryReturn    = static_cast<float>(xmlState->getDoubleAttribute("selectedTrajectoryReturn", .0f));
        connectOsc(m_iOscPortZirkonium);
        m_bNeedToRefreshGui=true;
    }
}

//OSC-------------------------
void ZirkOscAudioProcessor::sendOSCValues(){
    for(int iCurSrc = 0; iCurSrc <m_iNbrSources; ++iCurSrc){
        int   channel_osc   = m_oAllSources[iCurSrc].getSourceId()-1;
        float azim_osc      = PercentToHR(m_oAllSources[iCurSrc].getAzimuth01(), -1, 1);        //-1 is in the back right and +1 in the back left. 0 is forward
        float elev_osc      = PercentToHR(m_oAllSources[iCurSrc].getElevation01(), 0, .5);      //0 is the edge of the dome, .5 is the top
        float azimspan_osc  = PercentToHR(m_oAllSources[iCurSrc].getAzimuthSpan(), 0, 2);       //min azim span is 0, max is 2
        float elevspan_osc  = PercentToHR(m_oAllSources[iCurSrc].getElevationSpan(), 0, .5);    //min elev span is 0, max is .5
        float gain_osc      = m_oAllSources[iCurSrc].getGain01();
        
//        lo_send(_OscZirkonium, "/pan/az", "ifffff", channel_osc, azim_osc, elev_osc, azimspan_osc, elevspan_osc, gain_osc);
        OSCAddressPattern oscPattern("/pan/az");
        OSCMessage message(oscPattern);
        
        message.addInt32(channel_osc);
        message.addFloat32(azim_osc);
        message.addFloat32(elev_osc);
        message.addFloat32(azimspan_osc);
        message.addFloat32(elevspan_osc);
        message.addFloat32(gain_osc);
        
        if (!mOscSender.send(message)) {
            DBG("Error: could not send OSC message.");
        }
    }
    //OSC-------------------------
}




int ZirkOscAudioProcessor::getMovementConstraint() {
    return m_iMovementConstraint;
}

int ZirkOscAudioProcessor::getSelectedTrajectory() {
    int value = PercentToIntStartsAtOne(m_fSelectedTrajectory, TotalNumberTrajectories);
    return value;
}

float ZirkOscAudioProcessor::getSelectedTrajectoryDirection() {
    return m_fSelectedTrajectoryDirection;
}

float ZirkOscAudioProcessor::getSelectedTrajectoryReturn() {
    return m_fSelectedTrajectoryReturn;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ZirkOscAudioProcessor();

}

