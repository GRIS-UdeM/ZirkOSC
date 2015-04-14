/*
 ==============================================================================
 ZirkOSC2: VST and AU audio plug-in enabling spatial movement of sound sources in a dome of speakers.
 
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
#include "ZirkConstants.h"

using namespace std;

// ==============================================================================
void Trajectory::start()
{
	spInit();
	mStarted = true;
    
    AudioPlayHead::CurrentPositionInfo cpi;
    ourProcessor->getPlayHead()->getCurrentPosition(cpi);
    
    m_dTrajectoryTimeDone = .0;
    _TrajectoryBeginTime = .0;
    _TrajectorySingleBeginTime = .0;
    
    if (_isSyncWTempo) {
        //convert measure count to a duration
        double dMesureLength = cpi.timeSigNumerator * (4 / cpi.timeSigDenominator) *  60 / cpi.bpm;
        _TrajectoriesDurationBuffer = m_TotalTrajectoriesDuration * dMesureLength;
    } else {
        _TrajectoriesDurationBuffer = m_TotalTrajectoriesDuration;
    }
    
    _TrajectoriesDurationBuffer *= _TrajectoryCount;
    _TrajectorySingleLength = _TrajectoriesDurationBuffer / _TrajectoryCount;
    
    //get automation started on currently selected source
    _SelectedSourceForTrajectory = ourProcessor->getSelectedSource();
    
    //store initial parameter value
    _TrajectoryInitialAzimuth   = ourProcessor->getParameter(_SelectedSourceForTrajectory*5);
    _TrajectoryInitialElevation = ourProcessor->getParameter((_SelectedSourceForTrajectory*5)+1);
    
    ourProcessor->storeCurrentLocations();
    
    //convert current elevation as a radian offset for trajectories that use sin/cos
    //    _TrajectoriesPhiAsin = asin(_TrajectoryInitialElevation);
    //    _TrajectoriesPhiAcos = acos(_TrajectoryInitialElevation);
    
    if (ourProcessor->getSelectedMovementConstraintAsInteger() == Independant){
        ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + (_SelectedSourceForTrajectory*5));
        ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + (_SelectedSourceForTrajectory*5));
    } else {
        for(int i = 0;i < ourProcessor->getNbrSources(); ++i){
            ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + (i*5));
            ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + (i*5));
        }
    }
}

bool Trajectory::process(float seconds, float beats)
{
	if (mStopped) return true;
	if (!mStarted) start();
	if (mDone == m_TotalTrajectoriesDuration)
	{
		spProcess(0, 0);
		stop();
		return true;
	}

	float duration = _isSyncWTempo ? beats : seconds;
	spProcess(duration, seconds);
	
	mDone += duration;
	if (mDone > m_TotalTrajectoriesDuration)
		mDone = m_TotalTrajectoriesDuration;
	
	return false;
}

float Trajectory::progress()
{
	return mDone / m_TotalTrajectoriesDuration;
}

void Trajectory::stop()
{
	if (!mStarted || mStopped) return;
	mStopped = true;

    if (ourProcessor->getSelectedMovementConstraintAsInteger() == Independant){
        ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + (_SelectedSourceForTrajectory*5));
        ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + (_SelectedSourceForTrajectory*5));
    } else {
        for(int i = 0;i < ourProcessor->getNbrSources(); ++i){
            ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + (i*5));
            ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + (i*5));
        }
    }
    
    //reset everything
    ourProcessor->restoreCurrentLocations();
    m_dTrajectoryTimeDone = .0;
    _isWriteTrajectory = false;
    
    ourProcessor->askForGuiRefresh();
}

Trajectory::Trajectory(ZirkOscjuceAudioProcessor *filter, float duration, bool syncWTempo, float times, int source)
:
	ourProcessor(filter),
	mStarted(false),
	mStopped(false),
	mDone(0),
	mDurationSingleTrajectory(duration),
//	mSource(source),
    _TrajectoryCount(times),
	_isSyncWTempo(syncWTempo)
{
	if (mDurationSingleTrajectory < 0.0001) mDurationSingleTrajectory = 0.0001;
	if (_TrajectoryCount < 0.0001) _TrajectoryCount = 0.0001;
    
	m_TotalTrajectoriesDuration = mDurationSingleTrajectory * _TrajectoryCount;
}

void Trajectory::move (float newAzimuth, float newElevation){
    if (ourProcessor->getSelectedMovementConstraintAsInteger() == Independant){
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + (_SelectedSourceForTrajectory*5), newAzimuth);
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + (_SelectedSourceForTrajectory*5), newElevation);
    } else {
        SoundSource newLocationSource(newAzimuth, newElevation);
        Point<float> newLocation = newLocationSource.getPositionXY();
        ourProcessor->moveTrajectoriesWithConstraints(newLocation);
    }
}

// ==============================================================================
class CircleTrajectory : public Trajectory
{
public:
	CircleTrajectory(ZirkOscjuceAudioProcessor *filter, float duration, bool beats, float times, int source, bool ccw)
	: Trajectory(filter, duration, beats, times, source), mCCW(ccw) {}
	
protected:
	void spProcess(float duration, float seconds)
	{
        float newAzimuth;
        float integralPart; //useless here
        
        newAzimuth = mDone / mDurationSingleTrajectory; //modf((m_dTrajectoryTimeDone - _TrajectoryBeginTime) / _TrajectorySingleLength, &integralPart);
        
        if (mCCW) newAzimuth = - newAzimuth;
        newAzimuth = modf(_TrajectoryInitialAzimuth + newAzimuth, &integralPart);
        
        move(newAzimuth, _TrajectoryInitialElevation);
	}
	
private:
	bool mCCW;
};

// ==============================================================================
class PendulumTrajectory : public Trajectory
{
public:
	PendulumTrajectory(ZirkOscjuceAudioProcessor *filter, float duration, bool beats, float times, int source, bool in, bool rt)
	: Trajectory(filter, duration, beats, times, source), mIn(in), mRT(rt) {
        oldElevationBuffer = -1.f;
        
        if (mIn)    m_bTrajectoryElevationDecreasing = false;
        else        m_bTrajectoryElevationDecreasing = true;
    }
	
protected:
	void spInit()
	{
//		for (int i = 0; i < mFilter->getNumberOfSources(); i++)
//			mSourcesInitRT.add(mFilter->getSourceRT(i));
	}
	void spProcess(float duration, float seconds)
	{
        
        float newElevation = mDone / mDurationSingleTrajectory, integralPart;
        //this just grows linearly with time from 0 to _TrajectoryCount
        
        if (mRT){
            if (mIn){
                newElevation = abs( sin(newElevation * 2 * M_PI) );  //varies between [0,1]
                newElevation = newElevation * (1-_TrajectoryInitialElevation) + _TrajectoryInitialElevation; //scale [0,1] to [_TrajectoryInitialElevation, 1]
                
                if (oldElevationBuffer != -1.f){
                    //when we get to the top of the dome, we need to get back down
                    if (!m_bTrajectoryElevationDecreasing && newElevation < oldElevationBuffer){
                        (_TrajectoryInitialAzimuth > 0.5) ? _TrajectoryInitialAzimuth -= .5 : _TrajectoryInitialAzimuth += .5;
                        m_bTrajectoryElevationDecreasing = true;
                    }
                    //reset flag m_bTrajectoryElevationDecreasing as we go down
                    if (m_bTrajectoryElevationDecreasing && newElevation > oldElevationBuffer){
                        m_bTrajectoryElevationDecreasing = false;
                    }
                }
                oldElevationBuffer = newElevation;
            } else {
                //new way, simply oscilates between _TrajectoryInitialElevation and 0
                newElevation = abs( _TrajectoryInitialElevation * cos(newElevation * M_PI /*+ _TrajectoriesPhiAcos*/) );  //only positive cos wave with phase _TrajectoriesPhi
            }
            
        } else {
            newElevation = modf(newElevation, &integralPart); //should be [0,1]
            if (mIn){
                newElevation = newElevation * (1 - _TrajectoryInitialElevation) + _TrajectoryInitialElevation;
            } else {
                newElevation = _TrajectoryInitialElevation*(1-newElevation);
            }
        
        }
        move (_TrajectoryInitialAzimuth, newElevation);        
	}
	
private:
//	Array<FPoint> mSourcesInitRT;
	bool mIn, mRT;
    float oldElevationBuffer;
    bool m_bTrajectoryElevationDecreasing;
};

// ==============================================================================
class SpiralTrajectory : public Trajectory
{
public:
    SpiralTrajectory(ZirkOscjuceAudioProcessor *filter, float duration, bool beats, float times, int source, bool ccw, bool in, bool rt)
    : Trajectory(filter, duration, beats, times, source), mCCW(ccw), mIn(in), mRT(rt) {}
    
protected:
    void spInit()
    {
        //		for (int i = 0; i < mFilter->getNumberOfSources(); i++)
        //			mSourcesInitRT.add(mFilter->getSourceRT(i));
    }
    void spProcess(float duration, float seconds)
    {
        float newAzimuth, newElevation, theta;
        float integralPart; //useless here
        
        newElevation = mDone / mDurationSingleTrajectory;
        theta = modf(newElevation, &integralPart);                                          //result from this modf is theta [0,1]
        
        //UP AND DOWN SPIRAL
        if (mRT){
            if (mIn){
                newElevation = abs( (1 - _TrajectoryInitialElevation) * sin(newElevation * M_PI) ) + _TrajectoryInitialElevation;
            } else {
                newElevation = abs( _TrajectoryInitialElevation * cos(newElevation * M_PI) );  //only positive cos wave with phase _TrajectoriesPhi
            }
            
            if (!mCCW) theta = -theta;
            theta *= 2;
            
        } else {
            
            //***** kinda like archimedian spiral r = a + b * theta , but azimuth does not reset at the top
            newElevation = theta * (1 - _TrajectoryInitialElevation) + _TrajectoryInitialElevation;         //newElevation is a mapping of theta[0,1] to [_TrajectoryInitialElevation, 1]
            
            if (!mIn)
                newElevation = _TrajectoryInitialElevation * (1 - newElevation) / (1-_TrajectoryInitialElevation);  //map newElevation from [_TrajectoryInitialElevation, 1] to [_TrajectoryInitialElevation, 0]
            
            if (!mCCW) theta = -theta;
        }
        newAzimuth = modf(_TrajectoryInitialAzimuth + theta, &integralPart);                        //this is like adding a to theta
        move(newAzimuth, newElevation);
    }
    
private:
    //	Array<FPoint> mSourcesInitRT;
    bool mCCW, mIn, mRT;
};


// ==============================================================================
class EllipseTrajectory : public Trajectory
{
public:
	EllipseTrajectory(ZirkOscjuceAudioProcessor *filter, float duration, bool beats, float times, int source, bool ccw)
	: Trajectory(filter, duration, beats, times, source), mCCW(ccw) {}
	
protected:
	void spInit()
	{
//		for (int i = 0; i < mFilter->getNumberOfSources(); i++)
//			mSourcesInitRT.add(mFilter->getSourceRT(i));
	}
    void spProcess(float duration, float seconds)
    {
        
        float integralPart; //useless here
        float theta = mDone / mDurationSingleTrajectory;   //goes from 0 to _TrajectoryCount
        theta = modf(theta, &integralPart); //does 0 -> 1 for _TrajectoryCount times
        if (!mCCW) theta = -theta;
        
        float newAzimuth = _TrajectoryInitialAzimuth + theta;
        
        float newElevation = _TrajectoryInitialElevation + (1-_TrajectoryInitialElevation)/2 * abs(sin(theta * 2 * M_PI));
        
        move(newAzimuth, newElevation);
    }
	
private:
//	Array<FPoint> mSourcesInitRT;
	bool mCCW;
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
	RandomTrajectory(ZirkOscjuceAudioProcessor *filter, float duration, bool beats, float times, int source, float speed)
	: Trajectory(filter, duration, beats, times, source), mClock(0), mSpeed(speed) {}
	
protected:
	void spProcess(float duration, float seconds)
	{
//		mClock += seconds;
//		while(mClock > 0.01)
//		{
//			mClock -= 0.01;
//			for (int i = 0; i < mFilter->getNumberOfSources(); i++)
//			if (mSource < 0 || mSource == i)
//			{
//				FPoint p = mFilter->getSourceXY(i);
//				float r1 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
//				float r2 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
//				p.x += (r1 - 0.5) * mSpeed;
//				p.y += (r2 - 0.5) * mSpeed;
//				mFilter->setSourceXY(i, p);
//			}
//		}
        
        mClock += seconds;
        while(mClock > 0.01)
        {
            mClock -= 0.01;
            float azimuth   = ourProcessor->getParameter(_SelectedSourceForTrajectory*5);
            float elevation = ourProcessor->getParameter((_SelectedSourceForTrajectory*5)+1);
            
            float r1 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
            float r2 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
            azimuth += (r1 - 0.5) * mSpeed;
            elevation += (r2 - 0.5) * mSpeed;
            move(azimuth, elevation);
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
	TargetTrajectory(ZirkOscjuceAudioProcessor *filter, float duration, bool beats, float times, int source)
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
	RandomTargetTrajectory(ZirkOscjuceAudioProcessor *filter, float duration, bool beats, float times, int source)
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
	SymXTargetTrajectory(ZirkOscjuceAudioProcessor *filter, float duration, bool beats, float times, int source)
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
	SymYTargetTrajectory(ZirkOscjuceAudioProcessor *filter, float duration, bool beats, float times, int source)
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
	ClosestSpeakerTargetTrajectory(ZirkOscjuceAudioProcessor *filter, float duration, bool beats, float times, int source)
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
int Trajectory::NumberOfTrajectories() { return 23; }
String Trajectory::GetTrajectoryName(int i)
{
	switch(i)
	{
		case 0: return "Circle (CW)";
		case 1: return "Circle (CCW)";
		case 2: return "Ellipse (CW)";
		case 3: return "Ellipse (CCW)";
		case 4: return "Spiral (In, RT, CW)";
		case 5: return "Spiral (In, RT, CCW)";
		case 6: return "Spiral (Out, RT, CW)";
		case 7: return "Spiral (Out, RT, CCW)";
		case 8: return "Spiral (In, OW, CW)";
		case 9: return "Spiral (In, OW, CCW)";
		case 10: return "Spiral (Out, OW, CW)";
		case 11: return "Spiral (Out, OW, CCW)";
		case 12: return "Pendulum (In, RT)";
		case 13: return "Pendulum (Out, RT)";
		case 14: return "Pendulum (In, OW)";
		case 15: return "Pendulum (Out, OW)";
		case 16: return "Random Target";
		case 17: return "Random (Slow)";
		case 18: return "Random (Mid)";
		case 19: return "Random (Fast)";
		case 20: return "Sym X Target";
		case 21: return "Sym Y Target";
		case 22: return "Closest Speaker Target";
	}
	jassert(0);
	return "";
}
Trajectory::Ptr Trajectory::CreateTrajectory(int i, ZirkOscjuceAudioProcessor *filter, float duration, bool beats, float times, int source)
{
	switch(i)
	{
		case 0: return new CircleTrajectory(filter, duration, beats, times, source, false);
		case 1: return new CircleTrajectory(filter, duration, beats, times, source, true);
		case 2: return new EllipseTrajectory(filter, duration, beats, times, source, false);
		case 3: return new EllipseTrajectory(filter, duration, beats, times, source, true);
		case 4: return new SpiralTrajectory(filter, duration, beats, times, source, false, true, true);
		case 5: return new SpiralTrajectory(filter, duration, beats, times, source, true, true, true);
		case 6: return new SpiralTrajectory(filter, duration, beats, times, source, false, false, true);
		case 7: return new SpiralTrajectory(filter, duration, beats, times, source, true, false, true);
		case 8: return new SpiralTrajectory(filter, duration, beats, times, source, false, true, false);
		case 9: return new SpiralTrajectory(filter, duration, beats, times, source, true, true, false);
		case 10: return new SpiralTrajectory(filter, duration, beats, times, source, false, false, false);
		case 11: return new SpiralTrajectory(filter, duration, beats, times, source, true, false, false);
		case 12: return new PendulumTrajectory(filter, duration, beats, times, source, true, true);
		case 13: return new PendulumTrajectory(filter, duration, beats, times, source, false, true);
		case 14: return new PendulumTrajectory(filter, duration, beats, times, source, true, false);
		case 15: return new PendulumTrajectory(filter, duration, beats, times, source, false, false);
		case 16: return new RandomTargetTrajectory(filter, duration, beats, times, source);
		case 17: return new RandomTrajectory(filter, duration, beats, times, source, 0.02);
		case 18: return new RandomTrajectory(filter, duration, beats, times, source, 0.06);
		case 19: return new RandomTrajectory(filter, duration, beats, times, source, 0.1);
		case 20: return new SymXTargetTrajectory(filter, duration, beats, times, source);
		case 21: return new SymYTargetTrajectory(filter, duration, beats, times, source);
		case 22: return new ClosestSpeakerTargetTrajectory(filter, duration, beats, times, source);
	}
	jassert(0);
	return NULL;
}





