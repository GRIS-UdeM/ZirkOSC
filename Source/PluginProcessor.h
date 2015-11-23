/*
 ==============================================================================
 ZirkOSC: VST and AU audio plug-in enabling spatial movement of sound sources in a dome of speakers.
 
 Copyright (C) 2015  GRIS-UdeM
 
 Developers: Ludovic Laffineur, Vincent Berthiaume
 
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

#ifndef __PLUGINPROCESSOR_H_F70DA35D__
#define __PLUGINPROCESSOR_H_F70DA35D__

#include "../JuceLibraryCode/JuceHeader.h"

//#include "ZirkConstants.h"
#include "lo.h"
#include "SoundSource.h"
#include "Trajectories.h"

class SourceUpdateThread;

typedef struct
{
    int i;
    float a;
} IndexedAngle;

int IndexedAngleCompare(const void *a, const void *b);

/**
 The processor class of the plug in
 */
class ZirkOscAudioProcessor  : public AudioProcessor
{
public:
    
    //==============================================================================
    //! Builder
    ZirkOscAudioProcessor();
    //! Destroyer
    ~ZirkOscAudioProcessor();

    
    void move(const int &p_iSource, const float &p_fX, const float &p_fY, const float &p_azim01 = -1, const float &p_elev01 = -1);
    
    //==============================================================================
    //! Called before playback starts, to let the filter prepare itself. 
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    //! Called after playback has stopped, to let the filter free up any resources it no longer needs. 
    void releaseResources();
    //static     int receivePositionUpdate(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
    
    //! Renders the next block. 
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

    //==============================================================================
    //! Creates the editor and store it
    AudioProcessorEditor* createEditor();
    //! returns true if the processor has an editor
    bool hasEditor() const;

    //==============================================================================
    //! Returns the name of this processor. 
    const String getName() const;

    //! This must return the correct value immediately after the object has been created, and mustn't change the number of parameters later.
    int getNumParameters();
    //! Called by the host to find out the value of one of the filter's parameters.
    float getParameter (int index);
    //! The host will call this method to change the value of one of the filter's parameters. 
    void setParameter (int index, float newValue);
    //! Returns the name of a particular parameter. 
    const String getParameterName (int index);
    //! Returns the value of a parameter as a text string. 
    const String getParameterText (int index);
    
    //! Returns the number of input channels that the host will be sending the filter.
    const String getInputChannelName (int channelIndex) const;
    //! Returns the number of output channels that the host will be sending the filter. 
    const String getOutputChannelName (int channelIndex) const;
    //! Returns true if the specified channel is part of a stereo pair with its neighbour. 
    bool isInputChannelStereoPair (int index) const;
    //!Returns true if the specified channel is part of a stereo pair with its neighbour.
    bool isOutputChannelStereoPair (int index) const;

    //! Returns true if the processor wants midi messages. 
    bool acceptsMidi() const;
    //! Returns true if the processor produces midi messages. 
    bool producesMidi() const;
    //! Returns true if a silent input always produces a silent output.
    bool silenceInProducesSilenceOut() const;
    // We don't have sound so we just return 0.0
    double getTailLengthSeconds() const { return 0.0; }
    
    //! Projects sphere coords to polar coords 
    Point<float> domeToScreen(Point<float>);
    
    
    //==============================================================================
    //! Returns the number of preset programs the filter supports. 
    int getNumPrograms();
    //! Returns the number of the currently active program.
    int getCurrentProgram();
    //! Called by the host to change the current program. 
    void setCurrentProgram (int index);
    //! Must return the name of a given program. 
    const String getProgramName (int index);
    //! Called by the host to rename a program. 
    void changeProgramName (int index, const String& newName);


    //==============================================================================
    //! The host will call this method when it wants to save the filter's internal state, ie SAVE PRESET.
    void getStateInformation (MemoryBlock& destData);
    //! This must restore the filter's state from a block of data previously created using getStateInformation() IE RESTORE PRESET.
    void setStateInformation (const void* data, int sizeInBytes);
    
    //! returns the sources
    inline SoundSource* getSources(){ return m_oAllSources; }
    //! returns the number of sources on the screen.
    int getNbrSources() { return m_iNbrSources; }
    //! Set the number of sources.
    void setNbrSources(int newValue) {
        if ( newValue >0 && newValue < 9){
            if (newValue < m_iNbrSources &&  newValue < getSelectedSource()+1){
                setSelectedSource(0);
            }
            m_iNbrSources = newValue;
            m_bNeedToRefreshGui = true;
        }
    }
    
    //!set wheter plug is sending osc messages to zirkonium
    void setIsOscActive(bool isOscActive);
    //!wheter plug is sending osc messages to zirkonium
    bool getIsOscActive();

    //!set wheter plug is sending osc messages to zirkonium
    void setIsSpanLinked(bool isSpanLinked);
    //!wheter plug is sending osc messages to zirkonium
    bool getIsSpanLinked();

    //!set wheter programmed trajectories are written in sync with host tempo
    void setIsSyncWTempo(bool isSyncWTempo);
    //!get wheter programmed trajectories are written in sync with host tempo
    bool getIsSyncWTempo();
    
    //!set wheter to write programmed trajectory on next host play
    void setIsWriteTrajectory(bool isWriteTrajectory);
    //!get wheter to write programmed trajectory on next host play
    bool getIsWriteTrajectory();
    
    void setSelectedSourceForTrajectory(int iSelectedSource);
    
    int getSelectedSourceForTrajectory();
    
    //! return the tab position of the selectedSource 
    int getSelectedSource() { return m_iSelectedSource; }
    //! Set the selected source 
    void setSelectedSource(int selected){
        if ( selected >-1 && selected < 8){
            m_iSelectedSource = selected;
            if(m_iMovementConstraint == EqualAzim){
                setEqualAzimForAllSrc();
            } else if (m_iMovementConstraint == EqualAzimElev){
                setEqualAzimElevForAllSrc();
            } else if (m_iMovementConstraint == EqualElev){
                setEqualElevForAllSrc();
            }
            m_bNeedToRefreshGui = true;
        }
    }
    //! Returns the Osc Port for the Zirkonium sending
    int getOscPortZirkonium(){return m_iOscPortZirkonium;}
    
    enum ParameterIds
    {
        ZirkOSC_X_ParamId = 0,
        ZirkOSC_Y_ParamId,
        ZirkOSC_AzimSpan_ParamId,
        ZirkOSC_ElevSpan_ParamId,
        ZirkOSC_Gain_ParamId,
        ZirkOSC_Azim_ParamId_1,
        ZirkOSC_Elev_ParamId_1,
        ZirkOSC_AzimSpan_ParamId_1,
        ZirkOSC_ElevSpan_ParamId_1,
        ZirkOSC_Gain_ParamId_1,
        ZirkOSC_Azim_ParamId_2,     //10
        ZirkOSC_Elev_ParamId_2,
        ZirkOSC_AzimSpan_ParamId_2,
        ZirkOSC_ElevSpan_ParamId_2,
        ZirkOSC_Gain_ParamId_2,
        ZirkOSC_Azim_ParamId_3,
        ZirkOSC_Elev_ParamId_3,
        ZirkOSC_AzimSpan_ParamId_3,
        ZirkOSC_ElevSpan_ParamId_3,
        ZirkOSC_Gain_ParamId_3,
        ZirkOSC_Azim_ParamId_4,     //20
        ZirkOSC_Elev_ParamId_4,
        ZirkOSC_AzimSpan_ParamId_4,
        ZirkOSC_ElevSpan_ParamId_4,
        ZirkOSC_Gain_ParamId_4,
        ZirkOSC_Azim_ParamId_5,
        ZirkOSC_Elev_ParamId_5,
        ZirkOSC_AzimSpan_ParamId_5,
        ZirkOSC_ElevSpan_ParamId_5,
        ZirkOSC_Gain_ParamId_5,
        ZirkOSC_Azim_ParamId_6,     //30
        ZirkOSC_Elev_ParamId_6,
        ZirkOSC_AzimSpan_ParamId_6,
        ZirkOSC_ElevSpan_ParamId_6,
        ZirkOSC_Gain_ParamId_6,
        ZirkOSC_Azim_ParamId_7,
        ZirkOSC_Elev_ParamId_7,
        ZirkOSC_AzimSpan_ParamId_7,
        ZirkOSC_ElevSpan_ParamId_7,
        ZirkOSC_Gain_ParamId_7,
        ZirkOSC_MovementConstraint_ParamId, //40
        ZirkOSC_isSpanLinked_ParamId,
        ZirkOSC_isOscActive_ParamId,
        ZirkOSC_SelectedTrajectory_ParamId,
        ZirkOSC_SelectedTrajectoryDirection_ParamId,
        ZirkOSC_SelectedTrajectoryReturn_ParamId,
        ZirkOSCm_dTrajectoryCount_ParamId,
        ZirkOSC_TrajectoriesDuration_ParamId,
        ZirkOSC_SyncWTempo_ParamId,
        ZirkOSC_WriteTrajectories_ParamId,
        totalNumParams                      //50
    };
    
    //! Send the current state to all the iPad and Zirkonium
    void sendOSCValues();
    
    //! Getter constrain type as integer, since parameters need to be stored as floats [0,1]
    int getMovementConstraint();
    
    void setMovementConstraint(float newValue);
    
    void setMovementConstraint(int newValue);
    
    //! Getter for trajectory as integer, since parameters need to be stored as floats [0,1]
    int getSelectedTrajectory();
    
    float getSelectedTrajectoryDirection();
    
    float getSelectedTrajectoryReturn();
    
    //! Retunrs true if the Editor has to refresh the Gui.
    bool hasToRefreshGui(){return m_bNeedToRefreshGui;};
    //! If you want to refresh the Gui from the Processor.
    void setRefreshGui(bool gui) { m_bNeedToRefreshGui = gui;};
    //! Change the sending OSC port of the zirkonium
    void changeZirkoniumOSCPort(int newPort);

    //! Returns the Editor.
    AudioProcessorEditor* getEditor() {return m_oEditor;};
    //! Set the width that the UI was last set to
    void setLastUiWidth(int lastUiWidth);
    //! Get the width that the UI was last set to
    int getLastUiWidth();
    //! Set the height that the UI was last set to
    void setLastUiHeight(int lastUiHeight);
    //! Return the height that the UI was last set to
    int getLastUiHeight();
    
    void storeCurrentLocations();
    
    void restoreCurrentLocations();
    
    //this is to update positions on resize, based on new radius
    void updatePositions();
    
    //radius of the dome
    static int s_iDomeRadius;
    
    static bool s_bForceConstraintAutomation;
    
    bool isTrajectoryDone();
    
    //NEW TRAJECTORY CLASS METHODS
    void setTrajectory(Trajectory::Ptr t) { mTrajectory = t; }
  	Trajectory::Ptr getTrajectory() { return mTrajectory; }
    
    void askForGuiRefresh(){m_bNeedToRefreshGui=true;};
    
    void setIsRecordingAutomation(bool b){
        m_bIsRecordingAutomation = b;
    }
    
    //! get the source order by the angle value
    std::vector<int> getOrderSources();

    void setEqualAzimForAllSrc();
    void setEqualElevForAllSrc();
    
    void setEqualAzimElevForAllSrc();
    
    void setCurrentAndOldLocation(const int &p_iSrc, const float &p_fX01, const float &p_fY01);
    
    void updateSourcesSendOsc();
    
    bool isCurrentlyPlaying(){
        return m_bCurrentlyPlaying;
    }
    std::pair<float, float> getEndLocationXY(){
        return m_fEndLocationXY;
    }
    void setEndLocationXY(std::pair<float, float> pair){
        m_fEndLocationXY = pair;
    }
    double getTurns(){
        return m_dTrajectoryTurns;
    }
    void setTurns(double turns){
        m_dTrajectoryTurns = turns;
    }
    double getDeviation(){
        return m_dTrajectoryDeviation;
    }
    void setDeviation(double dev){
        m_dTrajectoryDeviation = dev;
    }
    double getDampening(){
        return m_dTrajectoryDampening;
    }
    void setDampening(double Dampening){
        m_dTrajectoryDampening = Dampening;
    }
    
    
private:
    
    void initSources();
    void processTrajectories();
    void stopTrajectory();
    void moveCircular(const int &p_iSource, const float &p_fX, const float &p_fY, const float &p_fAzim01 = -1, const float &p_fElev01 = -1);
    void moveDelta(const int &p_iSource, const float &p_fX, const float &p_fY);
    bool setPositionParameters(int index, float newValue);
    bool setOtherParameters(int index, float newValue);

    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZirkOscAudioProcessor)
    //! Whether the editor has to refresh the GUI
    bool m_bNeedToRefreshGui = false;
    //! Current number of sources on the screnn
    int m_iNbrSources;
    //! float ID of the selected movement constraint
    float m_fMovementConstraint;
    //! int ID of the selected movement constraint IMPORTANT: need to be set manually whenever float version of parameter is changed
    int m_iMovementConstraint;
    //! float ID of the selected trajectory
    float m_fSelectedTrajectory;
    
    float m_fSelectedTrajectoryDirection;
    
    float m_fSelectedTrajectoryReturn;
    
    //! Tab position of the selected source
    int m_iSelectedSource;
    //! Osc port to send to the Zirkonium 
    int m_iOscPortZirkonium;
    //! The editor
    AudioProcessorEditor* m_oEditor;
    //! Sources array
    SoundSource m_oAllSources [8];
    //Copy of all sources to be able to save and restore locations before and after a trajectory
    SoundSource m_oAllSourcesBuffer [8];
    
    //! Zirkonium OSC address (sending)
    lo_address _OscZirkonium;

    //! last saved ui width
    int _LastUiWidth;
    //! last saved ui height
    int _LastUiHeight;
    //! Whether we're sending OSC messages to the zirkonium
    bool m_bIsOscActive;
    //! If the span are linked
    bool m_bIsSpanLinked;
    
    PluginHostType host;
    
    std::pair<float, float> getDeltasForSelectedSource(const int &p_iSource, const float &p_fSelectedNewX, const float &p_fSelectedNewY, const float &p_fAzim01, const float &p_fElev01);
    std::pair<float, float> getCurrentSourcePosition(int iCurSource);
    std::tuple<float, float, float, float> getNewSourcePosition(const int &p_iSource, const float &fSelectedDeltaAzim01, const float &fSelectedDeltaElev01,
                                            const int &iCurSource, const float &fCurAzim01, const float &fCurElev01);
    int m_iActualConstraint;
    
    double m_dTrajectoryCount;
    double m_dTrajectoriesDuration;
    double m_dTrajectoryTurns;
    double m_dTrajectoryDeviation;
    double m_dTrajectoryDampening;
    bool   m_bIsSyncWTempo;
    bool   m_bIsWriteTrajectory;
    
    int m_iSelectedSourceForTrajectory;
    
    Trajectory::Ptr mTrajectory;
    
    int64 mLastTimeInSamples;
    
    //the id of the source that was last changed
    int m_iSourceLocationChanged;
    bool m_bCurrentlyPlaying;
    bool m_bDetectedPlayingStart;
    bool m_bDetectedPlayingEnd;
    bool m_bStartedConstraintAutomation;
    bool m_bIsRecordingAutomation;
    int m_iNeedToResetToActualConstraint;
    SourceUpdateThread* m_pSourceUpdateThread;
    std::pair <float, float> m_fEndLocationXY;
};

#endif  // __PLUGINPROCESSOR_H_F70DA35D__
