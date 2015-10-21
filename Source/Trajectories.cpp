/*
 ==============================================================================
 ZirkOSC: VST and AU audio plug-in enabling spatial movement of sound sources in a dome of speakers.
 
 Copyright (C) 2015  GRIS-UdeM
 
 Trajectories.cpp
 Created: 3 Aug 2014 11:42:38am
 
 Developers: Antoine Missout, Vincent Berthiaume
 
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


#include "Trajectories.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ZirkConstants.h"



using namespace std;

// ==============================================================================
Trajectory::Trajectory(ZirkOscAudioProcessor *filter, float duration, bool syncWTempo, float times, int source)
//:m_iSkip(0)
:ourProcessor(filter)
,mStarted(false)
,mStopped(false)
,mDone(0)
,mDurationSingleTrajectory(duration)
,m_dTrajectoryCount(times)
,m_bIsSyncWTempo(syncWTempo)
{
    if (mDurationSingleTrajectory < 0.0001) mDurationSingleTrajectory = 0.0001;
    if (m_dTrajectoryCount < 0.0001) m_dTrajectoryCount = 0.0001;
    
    m_TotalTrajectoriesDuration = mDurationSingleTrajectory * m_dTrajectoryCount;
    
    //get automation started on currently selected source
    m_iSelectedSourceForTrajectory = ourProcessor->getSelectedSource();
    
    //store initial parameter value
    m_fStartPair.first = ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSC_X_ParamId + m_iSelectedSourceForTrajectory*5);
    m_fStartPair.first = m_fStartPair.first*2*ZirkOscAudioProcessor::s_iDomeRadius - ZirkOscAudioProcessor::s_iDomeRadius;
    m_fStartPair.second = ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + m_iSelectedSourceForTrajectory*5);
    m_fStartPair.second = m_fStartPair.second*2*ZirkOscAudioProcessor::s_iDomeRadius - ZirkOscAudioProcessor::s_iDomeRadius;
    
    m_fTrajectoryInitialAzimuth01   = SoundSource::XYtoAzim01(m_fStartPair.first, m_fStartPair.second);
    m_fTrajectoryInitialElevation01 = SoundSource::XYtoElev01(m_fStartPair.first, m_fStartPair.second);
    ourProcessor->storeCurrentLocations();
}

void Trajectory::start() {
	spInit();
	mStarted = true;
    
    AudioPlayHead::CurrentPositionInfo cpi;
    ourProcessor->getPlayHead()->getCurrentPosition(cpi);
    
    m_dTrajectoryTimeDone  = .0;
    m_dTrajectoryBeginTime = .0;
    
    if (m_bIsSyncWTempo) {
        //convert measure count to a duration
        double dMesureLength = cpi.timeSigNumerator * (4 / cpi.timeSigDenominator) *  60 / cpi.bpm;
        m_dTrajectoriesDurationBuffer = m_TotalTrajectoriesDuration * dMesureLength;
    } else {
        m_dTrajectoriesDurationBuffer = m_TotalTrajectoriesDuration;
    }
    
    m_dTrajectoriesDurationBuffer *= m_dTrajectoryCount;
    m_dTrajectorySingleLength = m_dTrajectoriesDurationBuffer / m_dTrajectoryCount;
    
    ourProcessor->setIsRecordingAutomation(true);
    ourProcessor->beginParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_X_ParamId + m_iSelectedSourceForTrajectory*5);
    ourProcessor->beginParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + m_iSelectedSourceForTrajectory*5);
}

bool Trajectory::process(float seconds, float beats){
    if (mStopped) {
        return true;
    } else if (!mStarted){
        start();
    }
    if (mDone == m_TotalTrajectoriesDuration) {
		spProcess(0, 0);
		stop();
		return true;
	}
	float duration = m_bIsSyncWTempo ? beats : seconds;
    spProcess(duration, seconds);
	mDone += duration;
    if (mDone > m_TotalTrajectoriesDuration){
		mDone = m_TotalTrajectoriesDuration;
    }
	return false;
}

float Trajectory::progress(){
	return mDone / m_TotalTrajectoriesDuration;
}

void Trajectory::stop(){
	if (!mStarted || mStopped) return;
	mStopped = true;

    ourProcessor->endParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_X_ParamId + (m_iSelectedSourceForTrajectory*5));
    ourProcessor->endParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + (m_iSelectedSourceForTrajectory*5));
    ourProcessor->setIsRecordingAutomation(false);
    
    //reset everything
    ourProcessor->restoreCurrentLocations();
    m_dTrajectoryTimeDone = .0;
    m_bIsWriteTrajectory = false;
    
    ourProcessor->askForGuiRefresh();
}

void Trajectory::move (const float &p_fNewAzimuth, const float &p_fNewElevation){
    float fX, fY;
    SoundSource::azimElev01toXY(p_fNewAzimuth, p_fNewElevation, fX, fY);
    ourProcessor->move(m_iSelectedSourceForTrajectory, fX, fY);
}

void Trajectory::moveXY (const float &p_fNewX, const float &p_fNewY){
    ourProcessor->move(m_iSelectedSourceForTrajectory, p_fNewX, p_fNewY);
}


// ==============================================================================
class CircleTrajectory : public Trajectory
{
public:
	CircleTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source, bool ccw, float fTurns)
	:Trajectory(filter, duration, beats, times, source), mCCW(ccw)
    ,m_fTurns(fTurns)
    {}
protected:
	void spProcess(float duration, float seconds){
        float newAzimuth, integralPart;
        newAzimuth = mDone / mDurationSingleTrajectory;
        newAzimuth = modf(newAzimuth, &integralPart);
        if (!mCCW) newAzimuth = - newAzimuth;
        newAzimuth = modf(m_fTrajectoryInitialAzimuth01 + m_fTurns * newAzimuth, &integralPart);
        move(newAzimuth, m_fTrajectoryInitialElevation01);
	}
private:
	bool mCCW;
    float m_fTurns;
};

// ==============================================================================
class SpiralTrajectory : public Trajectory
{
public:
    SpiralTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source, bool ccw, bool rt, const std::pair<int, int> &endPoint, float fTurns)
    : Trajectory(filter, duration, beats, times, source)
    , mCCW(ccw)
    , m_bRT(rt)
    , m_fEndPair(endPoint)
    , m_fTurns(fTurns)
    { }
    
protected:
    void spInit(){
        //convert m_fTrajectoryInitialAzimuth01 + Elevation01 to m_fTransposedStartAzim01 + Elev01
        float fStartX, fStartY;
        SoundSource::azimElev01toXY(m_fTrajectoryInitialAzimuth01, m_fTrajectoryInitialElevation01, fStartX, fStartY);
        float fTransposedStartX = fStartX - m_fEndPair.first;
        float fTransposedStartY = fStartY - m_fEndPair.second;
        m_fTransposedStartAzim01 = SoundSource::XYtoAzim01(fTransposedStartX, fTransposedStartY);
        m_fTransposedStartElev01 = SoundSource::XYtoElev01(fTransposedStartX, fTransposedStartY);
    }
    void spProcess(float duration, float seconds){
        float newAzimuth01, theta, integralPart; //integralPart is only a temp buffer
        float newElevation01 = mDone / mDurationSingleTrajectory;
        theta = modf(newElevation01, &integralPart);                                          //result from this modf is theta [0,1]
        
        //UP AND DOWN SPIRAL
        if (m_bRT){
            if (mIn){
                newElevation01 = abs( (1 - m_fTransposedStartElev01) * sin(newElevation01 * M_PI) ) + m_fTransposedStartElev01;
            } else {
                JUCE_COMPILER_WARNING("mIn is always true; so either delete this or create another trajectory/mode for it")
                newElevation01 = abs( m_fTransposedStartElev01 * cos(newElevation01 * M_PI) );  //only positive cos wave with phase _TrajectoriesPhi
            }
            if (!mCCW) theta = -theta;
            theta *= 2;
        } else {
            //***** kinda like archimedian spiral r = a + b * theta , but azimuth does not reset at the top
            newElevation01 = theta * (1 - m_fTransposedStartElev01) + m_fTransposedStartElev01;                     //newElevation is a mapping of theta[0,1] to [m_fTransposedStartElev01, 1]
            if (!mIn){
                newElevation01 = m_fTransposedStartElev01 * (1 - newElevation01) / (1-m_fTransposedStartElev01);    //map newElevation from [m_fTransposedStartElev01, 1] to [m_fTransposedStartElev01, 0]
            }
            if (!mCCW) theta = -theta;
        }
        newAzimuth01 = modf(m_fTransposedStartAzim01 + m_fTurns * theta, &integralPart);                    //this is like adding a to theta
        //convert newAzim+Elev to XY
        float fNewX, fNewY;
        SoundSource::azimElev01toXY(newAzimuth01, newElevation01, fNewX, fNewY);
        fNewX += m_fEndPair.first;
        fNewY += m_fEndPair.second;
        moveXY(fNewX, fNewY);
    }
    
private:
    bool mCCW, mIn = true;
    bool m_bRT = false;
    std::pair<float, float> m_fEndPair;
    float m_fTransposedStartAzim01, m_fTransposedStartElev01;
    float m_fTurns;
};
// ==============================================================================
class PendulumTrajectory : public Trajectory
{
public:
    PendulumTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source, bool ccw,
                       bool rt,  const std::pair<float, float> &endPoint, float fDeviation, float p_fDampening)
    :Trajectory(filter, duration, beats, times, source)
    ,mCCW(ccw)
    ,m_bRT(rt)
    ,m_fEndPair(endPoint)
    ,m_fDeviation(fDeviation/360)
    ,m_fTotalDampening(p_fDampening)
    { }
    
protected:
    void spInit() {
        if (m_fEndPair.first != m_fStartPair.first){
            m_bYisDependent = true;
            m_fM = (m_fEndPair.second - m_fStartPair.second) / (m_fEndPair.first - m_fStartPair.first);
            m_fB = m_fStartPair.second - m_fM * m_fStartPair.first;
        } else {
            m_bYisDependent = false;
            m_fM = 0;
            m_fB = m_fStartPair.first;
        }
    }
    void spProcess(float duration, float seconds) {

        int iReturn = m_bRT ? 2:1;
        float fCurDampening = m_fTotalDampening * mDone / (mDurationSingleTrajectory * m_dTrajectoryCount);
        //pendulum part
        float newX, newY, temp, fCurrentProgress = modf((mDone / mDurationSingleTrajectory), &temp);

        if (m_bYisDependent){
            fCurrentProgress = (m_fEndPair.first - m_fStartPair.first) * (1-cos(fCurrentProgress * iReturn * M_PI)) / 2;
            newX = m_fStartPair.first + fCurrentProgress;
            newY = m_fM * newX + m_fB;
        } else {
            fCurrentProgress = (m_fEndPair.second - m_fStartPair.second) * (1-cos(fCurrentProgress * iReturn * M_PI)) / 2;
            newX = m_fStartPair.first;
            newY = m_fStartPair.second + fCurrentProgress;
        }
        newX = newX - newX*fCurDampening;
        newY = newY - newY*fCurDampening;
        float fPendulumAzim = SoundSource::XYtoAzim01(newX, newY);
        float fPendulumElev = SoundSource::XYtoElev01(newX, newY);
        
        //circle part
        float newAzimuth, integralPart;
        newAzimuth = mDone / (mDurationSingleTrajectory * m_dTrajectoryCount);
        newAzimuth = modf(newAzimuth, &integralPart);
        if (!mCCW) {
            newAzimuth = - newAzimuth;
        }
        
        newAzimuth *= m_fDeviation;
        move(fPendulumAzim + newAzimuth, fPendulumElev);
    }
private:
    bool mCCW, m_bRT, m_bYisDependent;
    std::pair<float, float> m_fEndPair;
    float m_fM;
    float m_fB;
    float m_fDeviation;
    float m_fTotalDampening;
};
// ==============================================================================
class DampedPendulumTrajectory : public Trajectory
{
public:
    DampedPendulumTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source,
                             bool ccw, bool rt, const std::pair<int, int> &endPoint, float p_fTurns, float p_fNumberOscillations)
    :Trajectory(filter, duration, beats, times, source)
    ,mCCW(ccw)
    ,m_bRT(rt)
    ,m_fEndPair(endPoint)
    ,m_fNumberOfOscillations(p_fNumberOscillations)
    ,m_fTurns(p_fTurns/2)   //somehow, because of interaction with number of oscillations, this has to be /2 to make more sense
    { }
protected:
    void spInit(){
        //calculate line equation
        if (m_fEndPair.first != m_fStartPair.first){
            m_bYisDependent = true;
            m_fM = (m_fEndPair.second - m_fStartPair.second) / (m_fEndPair.first - m_fStartPair.first);
            m_fB = m_fStartPair.second - m_fM * m_fStartPair.first;
        } else {
            m_bYisDependent = false;
            m_fM = 0;
            m_fB = m_fStartPair.first;
        }
        //buffer initial positions
        m_fStartInit.first  = m_fStartPair.first;
        m_fStartInit.second = m_fStartPair.second;
        m_fEndInit.first    = m_fEndPair  .first;
        m_fEndInit.second   = m_fEndPair  .second;
        //calculate far end of pendulum
        float fDeltaX = m_fEndPair.first - m_fStartPair.first;
        float fDeltaY = m_fEndPair.second - m_fStartPair.second;
        m_fFarEndPair.first  = m_fEndPair.first + fDeltaX;
        m_fFarEndPair.second = m_fEndPair.second + fDeltaY;
        //set current end point to end of pendulum
        m_fEndPair.first  = m_fFarEndPair.first;
        m_fEndPair.second = m_fFarEndPair.second;
    }
    
    void spProcess(float duration, float seconds){
        //pendulum part
        float fPendulumX, fPendulumY, temp, fPendulumProgress = modf((mDone / mDurationSingleTrajectory), &temp);
        if (m_bYisDependent){
            fPendulumProgress = (m_fEndPair.first - m_fStartPair.first) * (1-cos(fPendulumProgress * m_fNumberOfOscillations * M_PI)) / 2;
            fPendulumX = m_fStartPair.first + fPendulumProgress;
            fPendulumY = m_fM * fPendulumX + m_fB;
            m_fEndPair.first   = m_fFarEndPair.first + ((m_fEndInit.first - m_fFarEndPair.first) * mDone) / (mDurationSingleTrajectory * m_dTrajectoryCount);
            m_fStartPair.first = m_fStartInit.first  + ((m_fEndInit.first - m_fStartInit.first ) * mDone) / (mDurationSingleTrajectory * m_dTrajectoryCount);
        } else {
            fPendulumProgress = (m_fEndPair.second - m_fStartPair.second) * (1-cos(fPendulumProgress * m_fNumberOfOscillations * M_PI)) / 2;
            fPendulumX = m_fStartPair.first + fPendulumProgress;
            fPendulumY = m_fM * fPendulumX + m_fB;
            m_fEndPair.first   = m_fFarEndPair.first + ((m_fEndInit.first - m_fFarEndPair.first) * mDone) / (mDurationSingleTrajectory * m_dTrajectoryCount);
            m_fStartPair.first = m_fStartInit.first  + ((m_fEndInit.first - m_fStartInit.first ) * mDone) / (mDurationSingleTrajectory * m_dTrajectoryCount);
        }
        float fPendulumAzim = SoundSource::XYtoAzim01(fPendulumX, fPendulumY);
        float fPendulumElev = SoundSource::XYtoElev01(fPendulumX, fPendulumY);
        //circle part
        float newAzimuth, integralPart;
        newAzimuth = mDone / mDurationSingleTrajectory;
        newAzimuth = modf(newAzimuth, &integralPart);
        if (!mCCW) {
            newAzimuth = - newAzimuth;
        }
        newAzimuth = modf(m_fTrajectoryInitialAzimuth01 + m_fTurns * newAzimuth, &integralPart);
        //move using both parts
        move(fPendulumAzim + (newAzimuth - m_fTrajectoryInitialAzimuth01), fPendulumElev);
    }
    
private:
    //	Array<FPoint> mSourcesInitRT;
    bool mCCW;
    bool m_bRT = false, m_bYisDependent;
    std::pair<float, float> m_fEndPair;
    std::pair<float, float> m_fFarEndPair;
    std::pair<float, float> m_fStartInit;
    std::pair<float, float> m_fEndInit;
    float m_fM;
    float m_fB;
    float m_fNumberOfOscillations;
    float m_fTurns;
};

// ==============================================================================
class EllipseTrajectory : public Trajectory
{
public:
	EllipseTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source, bool ccw, float fTurns)
	:Trajectory(filter, duration, beats, times, source), mCCW(ccw)
    ,m_fTurns(fTurns)
    {}
	
protected:
	void spInit()
	{
//		for (int i = 0; i < mFilter->getNumberOfSources(); i++)
//			mSourcesInitRT.add(mFilter->getSourceRT(i));
	}
    void spProcess(float duration, float seconds)
    {
        
        float integralPart; //useless here
        float theta = mDone / mDurationSingleTrajectory;   //goes from 0 to m_dTrajectoryCount
        theta = modf(theta, &integralPart); //does 0 -> 1 for m_dTrajectoryCount times
        if (!mCCW) theta = -theta;
        
        theta *= m_fTurns;
        
        float newAzimuth = m_fTrajectoryInitialAzimuth01 + theta;
        
        float newElevation = m_fTrajectoryInitialElevation01 + (1-m_fTrajectoryInitialElevation01)/2 * abs(sin(theta * 2 * M_PI));
        
        move(newAzimuth, newElevation);
    }
	
private:
//	Array<FPoint> mSourcesInitRT;
	bool mCCW;
    float m_fTurns;
};

// ==============================================================================
// Mersenne Twister random number generator, this is now included in c++11, see here: http://en.cppreference.com/w/cpp/numeric/random
class MTRand_int32
{
public:
	MTRand_int32()
	{
		seed(rand());
	}
	uint32_t rand_uint32()
	{
		if (p == n) gen_state();
		unsigned long x = state[p++];
		x ^= (x >> 11);
		x ^= (x << 7) & 0x9D2C5680;
		x ^= (x << 15) & 0xEFC60000;
		return x ^ (x >> 18);
	}
	void seed(uint32_t s)
	{
		state[0] = s;
		for (int i = 1; i < n; ++i)
			state[i] = 1812433253 * (state[i - 1] ^ (state[i - 1] >> 30)) + i;

		p = n; // force gen_state() to be called for next random number
	}

private:
	static const int n = 624, m = 397;
	int p;
	unsigned long state[n];
	unsigned long twiddle(uint32_t u, uint32_t v)
	{
		return (((u & 0x80000000) | (v & 0x7FFFFFFF)) >> 1) ^ ((v & 1) * 0x9908B0DF);
	}
	void gen_state()
	{
		for (int i = 0; i < (n - m); ++i)
			state[i] = state[i + m] ^ twiddle(state[i], state[i + 1]);
		for (int i = n - m; i < (n - 1); ++i)
			state[i] = state[i + m - n] ^ twiddle(state[i], state[i + 1]);
		state[n - 1] = state[m - 1] ^ twiddle(state[n - 1], state[0]);
		
		p = 0; // reset position
	}
	// make copy constructor and assignment operator unavailable, they don't make sense
	MTRand_int32(const MTRand_int32&);
	void operator=(const MTRand_int32&);
};


class RandomTrajectory : public Trajectory
{
public:
	RandomTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source, float speed)
	: Trajectory(filter, duration, beats, times, source), mClock(0), mSpeed(speed) {}
	
protected:
//	void spProcess(float duration, float seconds)
//	{
//        mClock += seconds;
//        while(mClock > 0.01) {
//            float fAzimuth, fElevation;
//            mClock -= 0.01;
//            if (ZirkOscAudioProcessor::s_bUseXY){
//
//                float fX = ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSC_X_ParamId + m_iSelectedSourceForTrajectory*5);
//                float fY = ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + m_iSelectedSourceForTrajectory*5);
//                
//                SoundSource::XY01toAzimElev01(fX, fY, fAzimuth, fElevation);
//                
//            } else {
//                fAzimuth  = ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSC_X_ParamId + m_iSelectedSourceForTrajectory*5);
//                fElevation= ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + m_iSelectedSourceForTrajectory*5);
//            }
//            float r1 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
//            float r2 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
//            fAzimuth += (r1 - 0.5) * mSpeed;
//            fElevation += (r2 - 0.5) * mSpeed;
//            move(fAzimuth, fElevation);
//        }
//	}

    void spProcess(float duration, float seconds)
    {
        mClock += seconds;
        while(mClock > 0.01) {

            mClock -= 0.01;

            float fX01 = ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSC_X_ParamId + m_iSelectedSourceForTrajectory*5);
            float fY01 = ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + m_iSelectedSourceForTrajectory*5);
            
            float r1 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
            float r2 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
            
            fX01 += (r1 - 0.5) * mSpeed;
            fY01 += (r2 - 0.5) * mSpeed;
            
            float fX = PercentToHR(fX01, -ZirkOscAudioProcessor::s_iDomeRadius, ZirkOscAudioProcessor::s_iDomeRadius);
            float fY = PercentToHR(fY01, -ZirkOscAudioProcessor::s_iDomeRadius, ZirkOscAudioProcessor::s_iDomeRadius);
            
//            JUCE_COMPILER_WARNING("there has to be a way to have the random values be in the correct range without clamping...")
//            fX = clamp(fX, static_cast<float>(-ZirkOscAudioProcessor::s_iDomeRadius), static_cast<float>(ZirkOscAudioProcessor::s_iDomeRadius));
//            fY = clamp(fY, static_cast<float>(-ZirkOscAudioProcessor::s_iDomeRadius), static_cast<float>(ZirkOscAudioProcessor::s_iDomeRadius));
            
            moveXY(fX, fY);


        }
    }
private:
	MTRand_int32 mRNG;
	float mClock;
	float mSpeed;
};

// ==============================================================================
class TargetTrajectory : public Trajectory
{
public:
	TargetTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source)
	: Trajectory(filter, duration, beats, times, source), mCycle(-1) {}
	
protected:
//	virtual FPoint destinationForSource(int s, FPoint o) = 0;

	void spProcess(float duration, float seconds)
	{
//		float p = mDone / mDurationSingleTrajectory;
//		
//		int cycle = (int)p;
//		if (mCycle != cycle)
//		{
//			mCycle = cycle;
//			mSourcesOrigins.clearQuick();
//			mSourcesDestinations.clearQuick();
//			
//			for (int i = 0; i < mFilter->getNumberOfSources(); i++)
//			if (mSource < 0 || mSource == i)
//			{
//				FPoint o = mFilter->getSourceXY(i);
//				mSourcesOrigins.add(o);
//				mSourcesDestinations.add(destinationForSource(i, o));
//			}
//		}
//
//		float d = fmodf(p, 1);
//		for (int i = 0; i < mFilter->getNumberOfSources(); i++)
//		if (mSource < 0 || mSource == i)
//		{
//			FPoint a = mSourcesOrigins.getUnchecked(i);
//			FPoint b = mSourcesDestinations.getUnchecked(i);
//			FPoint p = a + (b - a) * d;
//			mFilter->setSourceXY(i, p);
//		}
	}
	
private:
//	Array<FPoint> mSourcesOrigins;
//	Array<FPoint> mSourcesDestinations;
	int mCycle;
};


// ==============================================================================
class RandomTargetTrajectory : public TargetTrajectory
{
public:
	RandomTargetTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source)
	: TargetTrajectory(filter, duration, beats, times, source) {}
	
protected:
//	FPoint destinationForSource(int s, FPoint o)
//	{
//		float r1 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
//		float r2 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
//		float x = r1 * (kRadiusMax*2) - kRadiusMax;
//		float y = r2 * (kRadiusMax*2) - kRadiusMax;
//		float r = hypotf(x, y);
//		if (r > kRadiusMax)
//		{
//			float c = kRadiusMax/r;
//			x *= c;
//			y *= c;
//		}
//		return FPoint(x,y);
//	}
	
private:
	MTRand_int32 mRNG;
};

// ==============================================================================
class SymXTargetTrajectory : public TargetTrajectory
{
public:
	SymXTargetTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source)
	: TargetTrajectory(filter, duration, beats, times, source) {}
	
protected:
//	FPoint destinationForSource(int s, FPoint o)
//	{
//		return FPoint(o.x,-o.y);
//	}
};

// ==============================================================================
class SymYTargetTrajectory : public TargetTrajectory
{
public:
	SymYTargetTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source)
	: TargetTrajectory(filter, duration, beats, times, source) {}
	
protected:
//	FPoint destinationForSource(int s, FPoint o)
//	{
//		return FPoint(-o.x,o.y);
//	}
};

// ==============================================================================
class ClosestSpeakerTargetTrajectory : public TargetTrajectory
{
public:
	ClosestSpeakerTargetTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source)
	: TargetTrajectory(filter, duration, beats, times, source) {}
	
protected:
//	FPoint destinationForSource(int s, FPoint o)
//	{
//		int bestSpeaker = 0;
//		float bestDistance = o.getDistanceFrom(mFilter->getSpeakerXY(0));
//		
//		for (int i = 1; i < mFilter->getNumberOfSpeakers(); i++)
//		{
//			float d = o.getDistanceFrom(mFilter->getSpeakerXY(i));
//			if (d < bestDistance)
//			{
//				bestSpeaker = i;
//				bestDistance = d;
//			}
//		}
//		
//		return mFilter->getSpeakerXY(bestSpeaker);
//	}
};

// ==============================================================================
int Trajectory::NumberOfTrajectories() { return TotalNumberTrajectories-1; }

String Trajectory::GetTrajectoryName(int i){
	switch(i){
        case Circle: return "Circle";
        case Ellipse: return "Ellipse";
        case Spiral: return "Spiral";
        case Pendulum: return "Pendulum";
        case AllTrajectoryTypes::Random: return "Random";
	}
	jassert(0);
	return "";
}

std::unique_ptr<vector<String>> Trajectory::getTrajectoryPossibleDirections(int p_iTrajectory){
    unique_ptr<vector<String>> vDirections (new vector<String>);
    switch(p_iTrajectory) {
        case Circle:
        case Ellipse:
        case Spiral:
        case Pendulum:
            vDirections->push_back("Clockwise");
            vDirections->push_back("Counter Clockwise");
            break;
        case AllTrajectoryTypes::Random:
            vDirections->push_back("Slow");
            vDirections->push_back("Mid");
            vDirections->push_back("Fast");
            break;
        default:
            jassert(0);
    }
    return vDirections;
}
unique_ptr<AllTrajectoryDirections> Trajectory::getTrajectoryDirection(int p_iSelectedTrajectory, int p_iSelectedDirection){
    unique_ptr<AllTrajectoryDirections> pDirection (new AllTrajectoryDirections);
    switch (p_iSelectedTrajectory) {
        case Circle:
        case Ellipse:
        case Spiral:
        case Pendulum:
            *pDirection = static_cast<AllTrajectoryDirections>(p_iSelectedDirection);
            break;
        case AllTrajectoryTypes::Random:
            *pDirection = static_cast<AllTrajectoryDirections>(p_iSelectedDirection+9);
            break;
        default:
            break;
    }
    return pDirection;
}
std::unique_ptr<vector<String>> Trajectory::getTrajectoryPossibleReturns(int p_iTrajectory){
    unique_ptr<vector<String>> vReturns (new vector<String>);
    switch(p_iTrajectory) {
        case Circle:
        case Ellipse:
        case AllTrajectoryTypes::Random:
            return nullptr;
        case Spiral:
        case Pendulum:
            vReturns->push_back("One Way");
            vReturns->push_back("Return");
            break;
        default:
            jassert(0);
    }
    return vReturns;
}
Trajectory::Ptr Trajectory::CreateTrajectory(int type, ZirkOscAudioProcessor *filter, float duration, bool beats, AllTrajectoryDirections direction,
                                             bool bReturn, float times, int source, const std::pair<float, float> &endPair, float fTurns, float fDeviation, float fDampening){
    bool ccw, in, cross;
    float speed;
    switch (direction) {
        case CW:
            ccw = false;
            break;
        case CCW:
            ccw = true;
            break;
        case In:
            in = true;
            cross = false;
            break;
        case Out:
            in = false;
            cross = false;
            break;
        case Crossover:
            in = true;
            cross = true;
            break;
        case InCW:
            in = true;
            ccw = false;
            break;
        case InCCW:
            in = true;
            ccw = true;
            break;
        case OutCW:
            in = false;
            ccw = false;
            break;
        case OutCCW:
            in = false;
            ccw = true;
            break;
        case Slow:
            speed = .02;
            break;
        case Mid:
            speed = .04;
            break;
        case Fast:
            speed = .06;
            break;
        default:
            break;
    }

    switch(type) {
        case Circle:                     return new CircleTrajectory    (filter, duration, beats, times, source, ccw, fTurns);
        case Ellipse:                    return new EllipseTrajectory   (filter, duration, beats, times, source, ccw, fTurns);
        case Spiral:                     return new SpiralTrajectory    (filter, duration, beats, times, source, ccw, bReturn, endPair, fTurns);
        case Pendulum:                   return new PendulumTrajectory  (filter, duration, beats, times, source, ccw, bReturn, endPair, fDeviation, fDampening);
        case AllTrajectoryTypes::Random: return new RandomTrajectory    (filter, duration, beats, times, source, speed);
            
            //      case 19: return new RandomTargetTrajectory(filter, duration, beats, times, source);
            //		case 20: return new SymXTargetTrajectory(filter, duration, beats, times, source);
            //		case 21: return new SymYTargetTrajectory(filter, duration, beats, times, source);
            //		case 22: return new ClosestSpeakerTargetTrajectory(filter, duration, beats, times, source);
    }
    jassert(0);
    return NULL;
}