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

bool ZirkOscAudioProcessor::s_bUseNewColorScheme = true;

bool ZirkOscAudioProcessor::s_bForceConstraintAutomation = false;

ZirkOscAudioProcessor::ZirkOscAudioProcessor()
:
m_iNbrSources(1)
,_SelectedTrajectory(.0f)
,m_fSelectedTrajectoryDirection(.0f)
,m_fSelectedTrajectoryReturn(.0f)
,m_iSelectedSource(0)
,_OscPortZirkonium(18032)
,_isOscActive(true)
,_isSpanLinked(true)
,m_dTrajectoryCount(1)
,_TrajectoriesDuration(5)
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
,m_bIsEqualElev(false)
,m_bIsRecordingAutomation(false)
,m_iNeedToResetToActualConstraint(-1)
{
    setMovementConstraint(Independent);
    
    initSources();

    char port[32];
    snprintf(port, sizeof(port), "%d", _OscPortZirkonium);
    _OscZirkonium = lo_address_new("127.0.0.1", port);
    
    //default values for ui dimensions
    _LastUiWidth  = ZirkOSC_Window_Default_Width;
    _LastUiHeight = ZirkOSC_Window_Default_Height;
    
    m_pSourceUpdateThread = new SourceUpdateThread(this);
}

void ZirkOscAudioProcessor::initSources(){
    for(int i = 0; i < 8; ++i){
        m_oAllSources[i] = SoundSource(0.0+((float)i/8.0),0.0);
        m_fSourceOldX01[i]          = m_oAllSources[i].getX01();
        m_fSourceOldY01[i]          = m_oAllSources[i].getY01();
        m_fSourceOldAzim01[i]       = m_oAllSources[i].getAzimuth01();
        m_fROverflow[i]             = s_iDomeRadius;
        m_bIsElevationOverflow[i]   = false;
    }
}

void ZirkOscAudioProcessor::updateSourcesSendOsc(){
    if (/*m_bCurrentlyPlaying && */!m_bIsRecordingAutomation && m_iMovementConstraint != Independent && m_iSourceLocationChanged != -1) {
        if (m_iMovementConstraint == DeltaLocked){
            JUCE_COMPILER_WARNING("it is very likely that the getX and getY here do not return values for the same position")
            moveSourcesWithDelta(m_iSourceLocationChanged, m_oAllSources[m_iSourceLocationChanged].getX(), m_oAllSources[m_iSourceLocationChanged].getY());
        } else {
            moveCircular(m_iSourceLocationChanged, m_oAllSources[m_iSourceLocationChanged].getX(), m_oAllSources[m_iSourceLocationChanged].getY(), m_bIsEqualElev);
        }
        
        m_iSourceLocationChanged = -1;
    }
    if (_isOscActive){
        sendOSCValues();
    }
}

void ZirkOscAudioProcessor::move(int p_iSource, float p_fX, float p_fY){
    if (p_iSource > getNbrSources()){
        return;
    }
    
    float fX01 = HRToPercent(p_fX, -s_iDomeRadius, s_iDomeRadius);
    float fY01 = HRToPercent(p_fY, -s_iDomeRadius, s_iDomeRadius);

    //cout << "writing " << fX01 << ", " << fY01 << newLine;
    
    setParameterNotifyingHost (ZirkOscAudioProcessor::ZirkOSC_X_ParamId + p_iSource*5, fX01);
    setParameterNotifyingHost (ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + p_iSource*5, fY01);

    if(m_iMovementConstraint == Independent){
        m_fSourceOldX01[p_iSource] = fX01;
        m_fSourceOldY01[p_iSource] = fY01;
        float fAzim01, fElev01;
        SoundSource::XY01toAzimElev01(fX01, fY01, fAzim01, fElev01);
        m_fSourceOldAzim01[p_iSource] = fAzim01;
    } else if (getNbrSources()>1){
        if (m_iMovementConstraint == EqualAzim){
            m_bIsEqualElev = false;
            moveEqualAzim(p_iSource, p_fX, p_fY);
        }
        else if (m_iMovementConstraint == EqualElev){
            m_bIsEqualElev = true;
            moveCircular(p_iSource, p_fX, p_fY, m_bIsEqualElev);
        }
        else if (m_iMovementConstraint == Circular){
            m_bIsEqualElev = false;
            moveCircular(p_iSource, p_fX, p_fY, m_bIsEqualElev);
        }
        else if (m_iMovementConstraint == EqualAzimElev){
            m_bIsEqualElev = true;
            moveEqualAzimElev(p_iSource, p_fX, p_fY);
        }
        else if (m_iMovementConstraint == DeltaLocked){
            m_bIsEqualElev = false;
            moveSourcesWithDelta(p_iSource, p_fX, p_fY);
        }
    }
}


void ZirkOscAudioProcessor::moveSourcesWithDelta(const int &p_iSource, const float &p_fX, const float &p_fY){
    
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
            m_fSourceOldAzim01[p_iSource] = SoundSource::XYtoAzim01(p_fX, p_fY);
            continue;
        }
        
        float newX01 = getSources()[iCurSrc].getX01() + fSelectedDeltaX01;
        float newY01 = getSources()[iCurSrc].getY01() + fSelectedDeltaY01;
        
        m_oAllSources[iCurSrc].setX01(newX01);
        m_oAllSources[iCurSrc].setY01(newY01);
        m_fSourceOldX01[iCurSrc] = newX01;
        m_fSourceOldY01[iCurSrc] = newY01;
        float fAzim01, fElev01;
        SoundSource::XY01toAzimElev01(newX01, newY01, fAzim01, fElev01);
        m_fSourceOldAzim01[p_iSource] = fAzim01;
    }
}

void ZirkOscAudioProcessor::moveCircular(const int &p_iSource, const float &p_fSelectedNewX, const float &p_fSelectedNewY, bool p_bIsElevEqual){
    
    float fSelectedOldAzim01, fSelectedOldElev01, fSelectedNewAzim01, fSelectedNewElev01;
    
    //calculate old coordinates for selected source.
    float fSelectedOldX01 = m_fSourceOldX01[p_iSource];
    float fSelectedOldY01 = m_fSourceOldY01[p_iSource];
    //convert x,y[0,1] to azim,elev[0,1]
    SoundSource::XY01toAzimElev01(fSelectedOldX01, fSelectedOldY01, fSelectedOldAzim01, fSelectedOldElev01);
    
    //calculate new azim elev coordinates for selected source.
    fSelectedNewAzim01 = SoundSource::XYtoAzim01(p_fSelectedNewX, p_fSelectedNewY);
    fSelectedNewElev01 = SoundSource::XYtoElev01(p_fSelectedNewX, p_fSelectedNewY);
    
    if (fSelectedNewAzim01 == fSelectedOldAzim01 && fSelectedNewElev01 == fSelectedOldElev01){
        return; //we got nothing to move here, so return
    }
    
    //calculate deltas for selected source.
    float fSelectedDeltaAzim01 = fSelectedNewAzim01 - fSelectedOldAzim01;
    float fSelectedDeltaElev01 = fSelectedNewElev01 - fSelectedOldElev01;
    
    //move non-selected sources using the deltas
    for (int iCurSource = 0; iCurSource < getNbrSources(); ++iCurSource) {
        
        if (iCurSource == p_iSource){
            //save new values as old values for next time
            m_fSourceOldX01[p_iSource] = HRToPercent(p_fSelectedNewX, -s_iDomeRadius, s_iDomeRadius);
            m_fSourceOldY01[p_iSource] = HRToPercent(p_fSelectedNewY, -s_iDomeRadius, s_iDomeRadius);
            m_fSourceOldAzim01[p_iSource] = SoundSource::XYtoAzim01(p_fSelectedNewX, p_fSelectedNewY);
            continue;
        }
        
        
        //---------------------- GET CURRENT VALUES ---------------------
        float fX = getParameter(ZirkOscAudioProcessor::ZirkOSC_X_ParamId + (iCurSource*5)) * 2 * s_iDomeRadius - s_iDomeRadius;
        float fY = getParameter(ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + (iCurSource*5)) * 2 * s_iDomeRadius - s_iDomeRadius;
        float fCurAzim01;
        if (m_oAllSources[iCurSource].wasElevationMaxed()){
            fCurAzim01 = m_fSourceOldAzim01[iCurSource];
        } else {
            fCurAzim01 = SoundSource::XYtoAzim01(fX, fY);
        }
        
        
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
            m_oAllSources[iCurSource].setX01(fX01);
            m_oAllSources[iCurSource].setY01(fY01);
        }
        //if elev is not equal, set all elevation to be current elevation +/- deltaY
        else {
            //if azimuth is reversed, ie, on the other side of the dome's middle point
            float fNewElev01;
//            if(getSources()[iCurSource].isAzimReverse()){
//                fNewElev01 = fCurElev01 - fSelectedDeltaElev01;
//            } else {
                fNewElev01 = fCurElev01 + fSelectedDeltaElev01;
//            }
            
            if (fNewElev01 > 1){
//                if (!getSources()[iCurSource].isAzimReverse()){
//                    getSources()[iCurSource].setAzimReverse(true);
//                } else {
//                    getSources()[iCurSource].setAzimReverse(false);
//                }
//                fNewElev01 = 1 + (1-fNewElev01);
//                float fAzimDegrees = PercentToHR(fNewAzim01, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
//                fAzimDegrees += (fAzimDegrees < 0 ? 180 : -180);
//                fNewAzim01 = HRToPercent(fAzimDegrees, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
                fNewElev01 = 1;
                m_oAllSources[iCurSource].setElevationWasMaxed(true);
                cout << fNewAzim01 << newLine;
            } else {
                m_oAllSources[iCurSource].setElevationWasMaxed(false);
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
            
            m_oAllSources[iCurSource].setX01(fX01);
            m_oAllSources[iCurSource].setY01(fY01);
        }
        //save new values as old values for next time
        m_fSourceOldX01[iCurSource] = fX01;
        m_fSourceOldY01[iCurSource] = fY01;
        float fAzim01, fElev01;
        SoundSource::XY01toAzimElev01(fX01, fY01, fAzim01, fElev01);
        m_fSourceOldAzim01[p_iSource] = fAzim01;
    }
}

void ZirkOscAudioProcessor::moveEqualAzim(const int &p_iSource, const float &p_fX, const float &p_fY){
    moveCircular(p_iSource, p_fX, p_fY, false);
}

void ZirkOscAudioProcessor::moveEqualAzimElev(const int &p_iSource, const float &p_fX, const float &p_fY){
    moveCircular(p_iSource, p_fX, p_fY, true);
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
        float fX01, fY01;
        SoundSource::azimElev01toXY01(fCurAngle, fCurElev, fX01, fY01);
        setCurrentAndOldLocation(iCurSrc, fX01, fY01);
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
        float fX01, fY01;
        SoundSource::azimElev01toXY01(fCurAngle, fSelElev, fX01, fY01);
        setCurrentAndOldLocation(iCurSrc, fX01, fY01);
    }

}

void ZirkOscAudioProcessor::setEqualElevForAllSrc(){
    float fSelElevation01 = m_oAllSources[m_iSelectedSource].getElevation01();
    for(int iCurSrc = 0; iCurSrc < getNbrSources() ; ++iCurSrc){
        float fCurAzim01 = m_oAllSources[iCurSrc].getAzimuth01();
        float fX01, fY01;
        SoundSource::azimElev01toXY01(fCurAzim01, fSelElevation01, fX01, fY01);
        setCurrentAndOldLocation(iCurSrc, fX01, fY01);
    }
}

void ZirkOscAudioProcessor::setCurrentAndOldLocation(const int &p_iSrc, const float &p_fX01, const float &p_fY01){
    setParameterNotifyingHost (ZirkOscAudioProcessor::ZirkOSC_X_ParamId + (p_iSrc*5), p_fX01);
    setParameterNotifyingHost (ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + (p_iSrc*5), p_fY01);
    m_fSourceOldX01[p_iSrc] = p_fX01;
    m_fSourceOldY01[p_iSrc] = p_fY01;
    float fAzim01, fElev01;
    SoundSource::XY01toAzimElev01(p_fX01, p_fY01, fAzim01, fElev01);
    m_fSourceOldAzim01[p_iSrc] = fAzim01;
}

//starting from the selected source, cycle through the other sources to find in which order they are
vector<int> ZirkOscAudioProcessor::getOrderSources(int selected, SoundSource tab [], int nbrSources){
    
    vector<int> order(nbrSources);
    int firstItem = selected;
    order[0] = selected;    //selected source is at order[0]
    int count  = 1;
    do{
        int current = (selected + 1)%nbrSources; //current is the next one after the seleted one
        
        int bestItem = current;
        float bestDelta = tab[current].getAzimuth01() - tab[selected].getAzimuth01(); //difference between current and selected
        if (bestDelta<0){
            bestDelta+=1;
        }
        
        while (current != selected) {
            float currentAzimuth;
            if (tab[current].getAzimuth01() - tab[selected].getAzimuth01()>0 ){
                currentAzimuth = tab[current].getAzimuth01();
            }
            else{
                currentAzimuth = tab[current].getAzimuth01()+1;
            }
            if (currentAzimuth - tab[selected].getAzimuth01() < bestDelta) {
                bestItem = current;
                bestDelta = currentAzimuth - tab[selected].getAzimuth01();
                
            }
            current = (current +1) % nbrSources;
        }
        
        order[count++]=bestItem;
        selected = bestItem;
    } while (selected != firstItem && count < nbrSources);
    return order;
}



//vector<float> ZirkOscAudioProcessor::getOrderSources(){
//    
//    std::vector<float> vSourcesAngularOrder(m_iNbrSources);
//    
//    IndexedAngle * ia = new IndexedAngle[m_iNbrSources];
//    
//    for (int iCurSrc = 0; iCurSrc < m_iNbrSources; iCurSrc++) {
//        ia[iCurSrc].i = iCurSrc;
//        ia[iCurSrc].a = m_oAllSources[iCurSrc].getAzimuth01();
//    }
//    
//    qsort(ia, m_iNbrSources, sizeof(IndexedAngle), IndexedAngleCompare);
//    
//    int b;
//    for (b = 0; b < m_iNbrSources && ia[b].i != m_iSelectedSource; b++) ;
//    
//    if (b == m_iNbrSources) {
//        printf("error!\n");
//        b = 0;
//    }
//    
//    for (int j = 1; j < m_iNbrSources; j++) {
//        int o = (b + j) % m_iNbrSources;
//        o = ia[o].i;
//        vSourcesAngularOrder[o] = (M_PI * 2. * j) / m_iNbrSources;
//    }
//    
//    delete[] ia;
//    return vSourcesAngularOrder;
//}


//vector<IndexedAngle> ZirkOscAudioProcessor::getOrderSources(){
//    //gather unsorted information
//    std::vector<IndexedAngle> indexedAngles (m_iNbrSources);
//    for (int iCurSrc = 0; iCurSrc < m_iNbrSources; iCurSrc++) {
//        indexedAngles[iCurSrc].i = iCurSrc;
//        indexedAngles[iCurSrc].a = m_oAllSources[iCurSrc].getAzimuth01();
//    }
//    //sort and return sorted angles
//    qsort(&indexedAngles[0], m_iNbrSources, sizeof(IndexedAngle), IndexedAngleCompare);
//    return indexedAngles;

//    //this just checks that the selected source is at the last index, ie, unsortedAngles[m_iNbrSources]
//    int b;
//    for (b = 0; b < m_iNbrSources && indexedAngles[b].i != m_iSelectedSource; b++) ;
//    
//    if (b == m_iNbrSources) {
//        printf("error!\n");
//        b = 0;
//    }
//    
//    //convert angles[0,1] to rad angles, and put in return vector. useless in our case, we can just use the indexes (and/or raw angles) from unsortedAngles
//    std::vector<IndexedAngle> vSourcesWithSortedAngles (m_iNbrSources);
//    for (int j = 1; j < m_iNbrSources; j++) {
//        int o = (b + j) % m_iNbrSources;
//        o = indexedAngles[o].i;
//        vSourcesWithSortedAngles[o].i = o;
//        vSourcesWithSortedAngles[o].a = (M_PI * 2. * j) / m_iNbrSources;
//    }
//    
//    return indexedAngles;
//}


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
    _isOscActive = false;
    lo_address osc = _OscZirkonium;
    if (osc){
        lo_address_free(osc);
    }
    _OscZirkonium = NULL;
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

void ZirkOscAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
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
    if (trajectory)
    {
        if (cpi.isPlaying && cpi.timeInSamples != mLastTimeInSamples)
        {
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
        _AllSourcesBuffer[iCurSource] = m_oAllSources[iCurSource];
    }
}

void ZirkOscAudioProcessor::restoreCurrentLocations(){
    for (int iCurSource = 0; iCurSource<8; ++iCurSource){
        m_oAllSources[iCurSource] = _AllSourcesBuffer[iCurSource];
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
    _Editor = new ZirkOscAudioProcessorEditor (this);
    return _Editor;
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
    _isOscActive = isOscActive;
}

//wheter plug is sending osc messages to zirkonium
bool ZirkOscAudioProcessor::getIsOscActive(){
    return _isOscActive;
}

void ZirkOscAudioProcessor::setIsSyncWTempo(bool isSyncWTempo){
    m_bIsSyncWTempo = isSyncWTempo;
}

bool ZirkOscAudioProcessor::getIsSyncWTempo(){
    return m_bIsSyncWTempo;
}


void ZirkOscAudioProcessor::setIsSpanLinked(bool isSpanLinked){
    _isSpanLinked = isSpanLinked;
}

bool ZirkOscAudioProcessor::getIsSpanLinked(){
    return _isSpanLinked;
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
        case ZirkOSCm_dTrajectoryCount_ParamId:
            return m_dTrajectoryCount;
        case ZirkOSC_TrajectoriesDuration_ParamId:
            return _TrajectoriesDuration;
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
            return m_oAllSources[iCurSrc].getGain();
        }
    }
    cerr << "\n" << "wrong parameter id: " << index << "in ZirkOscAudioProcessor::getParameter" << "\n";
    return -1.f;
}

// This method will be called by the host, probably on the audio thread, so
// it's absolutely time-critical. Don't use critical sections or anything
// UI-related, or anything at all that may block in any way!
void ZirkOscAudioProcessor::setParameter (int index, float newValue){
    bool bFoundParameter = false;
    switch (index){
        case ZirkOSC_MovementConstraint_ParamId:
            setMovementConstraint(newValue);
            bFoundParameter = true;
            break;
        case ZirkOSC_isOscActive_ParamId:
            if (newValue > .5f)
                _isOscActive = true;
            else
                _isOscActive = false;
            bFoundParameter = true;
            break;
        case ZirkOSC_isSpanLinked_ParamId:
            if (newValue > .5f)
                _isSpanLinked = true;
            else
                _isSpanLinked = false;
            bFoundParameter = true;
            break;
        case ZirkOSC_SelectedTrajectory_ParamId:
            _SelectedTrajectory = newValue;
            bFoundParameter = true;
            break;
        case ZirkOSC_SelectedTrajectoryDirection_ParamId:
            m_fSelectedTrajectoryDirection = newValue;
            bFoundParameter = true;

        case ZirkOSC_SelectedTrajectoryReturn_ParamId:
            m_fSelectedTrajectoryReturn = newValue;
            bFoundParameter = true;
            break;
        case ZirkOSCm_dTrajectoryCount_ParamId:
            m_dTrajectoryCount = newValue;
            bFoundParameter = true;
            break;
        case ZirkOSC_TrajectoriesDuration_ParamId:
            _TrajectoriesDuration = newValue;
            bFoundParameter = true;
            break;
        case ZirkOSC_SyncWTempo_ParamId:
            if (newValue > .5f)
                m_bIsSyncWTempo = true;
            else
                m_bIsSyncWTempo = false;
            bFoundParameter = true;
            break;
        case ZirkOSC_WriteTrajectories_ParamId:
            if (newValue > .5f)
                m_bIsWriteTrajectory = true;
            else
                m_bIsWriteTrajectory = false;
            bFoundParameter = true;
    }
    
    JUCE_COMPILER_WARNING("Should replace these 2 calls by a single one, maybe with Points?")
    for(int iCurSource = 0; iCurSource < 8; ++iCurSource){
        if  (ZirkOSC_X_ParamId + (iCurSource*5) == index) {
            if (newValue != m_oAllSources[iCurSource].getX01()){
                m_oAllSources[iCurSource].setX01(newValue);
                m_iSourceLocationChanged = iCurSource;
            }
            bFoundParameter = true;
        }
        else if (ZirkOSC_Y_ParamId + (iCurSource*5) == index) {
            if (newValue != m_oAllSources[iCurSource].getY01()){
                m_oAllSources[iCurSource].setY01(newValue);
                m_iSourceLocationChanged = iCurSource;
            }

            bFoundParameter = true;
        }
        else if (ZirkOSC_AzimSpan_ParamId + (iCurSource*5) == index){
            m_oAllSources[iCurSource].setAzimuthSpan(newValue); bFoundParameter = true;
        }
        else if (ZirkOSC_ElevSpan_ParamId + (iCurSource*5) == index){
            m_oAllSources[iCurSource].setElevationSpan(newValue); bFoundParameter = true;
        }
        else if (ZirkOSC_Gain_ParamId + (iCurSource*5) == index){
            m_oAllSources[iCurSource].setGain(newValue); bFoundParameter = true;
        }
    }
    if (!bFoundParameter){
        cerr << "wrong parameter id: " << index << " in ZirkOscAudioProcessor::setParameter\n";
    } else {
        m_bNeedToRefreshGui = true;
    }
}

void ZirkOscAudioProcessor::setMovementConstraint(float p_fConstraint){
    m_fMovementConstraint = p_fConstraint;
    m_iMovementConstraint = PercentToIntStartsAtOne(m_fMovementConstraint, TotalNumberConstraints);
}

void ZirkOscAudioProcessor::setMovementConstraint(int p_iConstraint){
    m_iMovementConstraint = p_iConstraint;
    m_fMovementConstraint = IntToPercentStartsAtOne(m_iMovementConstraint, TotalNumberConstraints);
}

const String ZirkOscAudioProcessor::getParameterName (int index)
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

static const int s_kiDataVersion = 3;

//==============================================================================
void ZirkOscAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    
    XmlElement xml ("ZIRKOSCJUCESETTINGS");
    xml.setAttribute ("uiWidth", _LastUiWidth);
    xml.setAttribute ("uiHeight", _LastUiHeight);
    xml.setAttribute("PortOSC", _OscPortZirkonium);
    xml.setAttribute("NombreSources", m_iNbrSources);
    xml.setAttribute("MovementConstraint", m_fMovementConstraint);
    xml.setAttribute("isSpanLinked", _isSpanLinked);
    xml.setAttribute("isOscActive", _isOscActive);
    xml.setAttribute("selectedTrajectory", _SelectedTrajectory);
    xml.setAttribute("nbrTrajectory", m_dTrajectoryCount);
    xml.setAttribute("durationTrajectory", _TrajectoriesDuration);
    xml.setAttribute("isSyncWTempo", m_bIsSyncWTempo);
    xml.setAttribute("isWriteTrajectory", m_bIsWriteTrajectory);
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
        
        xml.setAttribute(channel,       m_oAllSources[iCurSrc].getSourceId());
        xml.setAttribute(azimuthSpan,   m_oAllSources[iCurSrc].getAzimuthSpan());
        xml.setAttribute(elevationSpan, m_oAllSources[iCurSrc].getElevationSpan());
        xml.setAttribute(gain,          m_oAllSources[iCurSrc].getGain());
        xml.setAttribute(strX,          m_oAllSources[iCurSrc].getX01());
        xml.setAttribute(strY,          m_oAllSources[iCurSrc].getY01());
    }
    copyXmlToBinary (xml, destData);
}


void ZirkOscAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
        
        JUCE_COMPILER_WARNING("we don't even use those values to load the editor, why store them?")
        _LastUiWidth                    = xmlState->getIntAttribute ("uiWidth", _LastUiWidth);
        _LastUiHeight                   = xmlState->getIntAttribute ("uiHeight", _LastUiHeight);
        _OscPortZirkonium               = xmlState->getIntAttribute("PortOSC", 18032);
        m_iNbrSources                     = xmlState->getIntAttribute("NombreSources", 1);
        float fMovementConstraint       = xmlState->getDoubleAttribute("MovementConstraint", .0f);
        setMovementConstraint(fMovementConstraint >= 0 ? fMovementConstraint : 0);
        _isOscActive                    = xmlState->getBoolAttribute("isOscActive", true);
        _isSpanLinked                   = xmlState->getBoolAttribute("isSpanLinked", false);
        _SelectedTrajectory             = static_cast<float>(xmlState->getDoubleAttribute("selectedTrajectory", .0f));
        m_dTrajectoryCount                = xmlState->getIntAttribute("nbrTrajectory", 0);
        _TrajectoriesDuration           = static_cast<float>(xmlState->getDoubleAttribute("durationTrajectory", .0f));
        m_bIsSyncWTempo                   = xmlState->getBoolAttribute("isSyncWTempo", false);
        m_bIsWriteTrajectory              = xmlState->getBoolAttribute("isWriteTrajectory", false);
        
        for (int iCurSrc = 0; iCurSrc < 8; ++iCurSrc){
            String channel      = "Channel" + to_string(iCurSrc);
            String azimuthSpan  = "AzimuthSpan" + to_string(iCurSrc);
            String elevationSpan= "ElevationSpan" + to_string(iCurSrc);
            String gain         = "Gain" + to_string(iCurSrc);
            String strX         = "X" + to_string(iCurSrc);
            String strY         = "Y" + to_string(iCurSrc);

            m_oAllSources[iCurSrc].setSourceId(xmlState->getIntAttribute(channel , 0));
            m_oAllSources[iCurSrc].setAzimuthSpan(    static_cast<float>(xmlState->getDoubleAttribute(azimuthSpan,0)));
            m_oAllSources[iCurSrc].setElevationSpan(  static_cast<float>(xmlState->getDoubleAttribute(elevationSpan,0)));
            m_oAllSources[iCurSrc].setGain(           static_cast<float>(xmlState->getDoubleAttribute(gain, 1)));
            
            //calculate default value, in case we cannot find the actual values
            float fDefaultX, fDefaultY;
            SoundSource::azimElev01toXY01(iCurSrc * .125, 0, fDefaultX, fDefaultY);
            
            //fetch actual values
            float fActualX01 = static_cast<float>(xmlState->getDoubleAttribute(strX, fDefaultX));
            float fActualY01 = static_cast<float>(xmlState->getDoubleAttribute(strY, fDefaultY));

            setParameter (ZirkOscAudioProcessor::ZirkOSC_X_ParamId + iCurSrc*5, fActualX01);
            setParameter (ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + iCurSrc*5, fActualY01);
            m_fSourceOldX01[iCurSrc] = fActualX01;
            m_fSourceOldY01[iCurSrc] = fActualY01;
            float fAzim01, fElev01;
            SoundSource::XY01toAzimElev01(fActualX01, fActualY01, fAzim01, fElev01);
            m_fSourceOldAzim01[iCurSrc] = fAzim01;
        }
        
        m_fSelectedTrajectoryDirection = static_cast<float>(xmlState->getDoubleAttribute("selectedTrajectoryDirection", .0f));
        m_fSelectedTrajectoryReturn    = static_cast<float>(xmlState->getDoubleAttribute("selectedTrajectoryReturn", .0f));
        changeZirkoniumOSCPort(_OscPortZirkonium);
        m_bNeedToRefreshGui=true;
    }
}

void ZirkOscAudioProcessor::sendOSCValues(){
    
    for(int iCurSrc = 0; iCurSrc <m_iNbrSources; ++iCurSrc){
        float azim_osc      = PercentToHR(m_oAllSources[iCurSrc].getAzimuth01(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max) /180.;
        float elev_osc      = PercentToHR(m_oAllSources[iCurSrc].getElevation01(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max)/180.;
        float azimspan_osc  = PercentToHR(m_oAllSources[iCurSrc].getAzimuthSpan(), ZirkOSC_AzimSpan_Min,ZirkOSC_AzimSpan_Max)/180.;
        float elevspan_osc  = PercentToHR(m_oAllSources[iCurSrc].getElevationSpan(), ZirkOSC_ElevSpan_Min, ZirkOSC_Elev_Max)/180.;
        int   channel_osc   = m_oAllSources[iCurSrc].getSourceId()-1;
        float gain_osc      = m_oAllSources[iCurSrc].getGain();
        
        lo_send(_OscZirkonium, "/pan/az", "ifffff", channel_osc, azim_osc, elev_osc, azimspan_osc, elevspan_osc, gain_osc);
        
        //cout << channel_osc << ", " <<  azim_osc << ", " <<  elev_osc << newLine;
    }
}


void ZirkOscAudioProcessor::changeZirkoniumOSCPort(int newPort){
    
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

int ZirkOscAudioProcessor::getMovementConstraint() {
    return m_iMovementConstraint;
}

int ZirkOscAudioProcessor::getSelectedTrajectory() {
    int value = PercentToIntStartsAtOne(_SelectedTrajectory, TotalNumberTrajectories);
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

