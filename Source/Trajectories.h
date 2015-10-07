/*
 ==============================================================================
 ZirkOSC: VST and AU audio plug-in enabling spatial movement of sound sources in a dome of speakers.
 
 Copyright (C) 2015  GRIS-UdeM
 
 Trajectories.h
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

#ifndef TRAJECTORIES_H_INCLUDED
#define TRAJECTORIES_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "ZirkConstants.h"

class ZirkOscAudioProcessor;

class Trajectory : public ReferenceCountedObject
{
public:
	typedef ReferenceCountedObjectPtr<Trajectory> Ptr;
	static int NumberOfTrajectories();
	static String GetTrajectoryName(int i);
    static Trajectory::Ptr CreateTrajectory(int i, ZirkOscAudioProcessor *filter, float duration, bool beats, AllTrajectoryDirections direction, bool bReturn,
                                            float times, int source, const std::pair<float, float> &endPoint, float fTurns, float fNbrOscil, float fDeviation);
	
public:
	virtual ~Trajectory() {
        stop();
    }
	
	bool process(float seconds, float beats);
	float progress();
	void stop();
    
    static std::unique_ptr<std::vector<String>> getTrajectoryPossibleDirections(int p_iTrajectory);
    static std::unique_ptr<AllTrajectoryDirections> getTrajectoryDirection(int p_iSelectedTrajectory, int p_iSelectedDirection);
    
    static std::unique_ptr<std::vector<String>> getTrajectoryPossibleReturns(int p_iTrajectory);

protected:
	virtual void spInit() {}
	virtual void spProcess(float duration, float seconds) = 0;
    void move (const float &newAzimuth, const float &newElevation);
    void moveXY (const float &p_fNewX, const float &p_fNewY);
	
private:
	void start();
//    int m_iSkip;
	
protected:
	Trajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source);
	
	ZirkOscAudioProcessor *ourProcessor;

	bool mStarted, mStopped;

	float mDone;
    
    float mDurationSingleTrajectory;
    
    //! Number of trajectories to draw in trajectory section
    double m_dTrajectoryCount;
    
    //! Duration of trajectory movement
    double m_TotalTrajectoriesDuration;
    
    double m_dTrajectoriesDurationBuffer;
    
    //    double _TrajectoriesPhiAsin;
    //  double _TrajectoriesPhiAcos;
    
    double m_dTrajectoryBeginTime;
    
    float m_fTrajectoryInitialAzimuth01;
    float m_fTrajectoryInitialElevation01;

    std::pair<float, float> m_fStartPair;
    
    double m_dTrajectorySingleLength;
    
    double m_dTrajectoryTimeDone;
    
    //!Whether to sync trajectories with tempo
    bool m_bIsSyncWTempo;
    //!Whether to write trajectory or not
    bool m_bIsWriteTrajectory;
    
    int m_iSelectedSourceForTrajectory;
};

#endif  // TRAJECTORIES_H_INCLUDED
