/*
 ==============================================================================
 ZirkOSC2: VST and AU audio plug-in enabling spatial movement of sound sources in a dome of speakers.
 
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

typedef Point<float> FPoint;


//import from octogris

int IndexedAngleCompare(const void *a, const void *b);

static const float kSmoothMin = 1;
static const float kSmoothMax = 200;
static const float kSmoothDefault = 10;

static const float kVolumeNearMin = 0;
static const float kVolumeNearMax = 30;
static const float kVolumeNearDefault = 0;

static const float kVolumeMidMin = -30;
static const float kVolumeMidMax = 10;
static const float kVolumeMidDefault = -6;

static const float kVolumeFarMin = -120;
static const float kVolumeFarMax = 0;
static const float kVolumeFarDefault = -40;

static const float kRadiusMax = 2;
static const float kThetaMax = M_PI * 2;
static const float kThetaLockRadius = 0.05;
static const float kThetaLockRampRadius = 0.025;
static const float kSourceMinDistance = 2.5 * 0.5;
static const float kSourceMaxDistance = 20 * 0.5;

static const float kMaxDistance = 2000;

static const float kFilterNearMin = kMaxDistance;
static const float kFilterNearMax = 0;
static const float kFilterNearDefault = 0;

static const float kFilterMidMin = kMaxDistance;
static const float kFilterMidMax = 0;
static const float kFilterMidDefault = 0;

static const float kFilterFarMin = kMaxDistance;
static const float kFilterFarMax = 0;
static const float kFilterFarDefault = kMaxDistance;

static const float kMaxSpanVolumeMin = 0;
static const float kMaxSpanVolumeMax = 20;
static const float kMaxSpanVolumeDefault = 0;

static inline float normalize(float min, float max, float value)
{
    return (value - min) / (max - min);
}

static inline float denormalize(float min, float max, float value)
{
    return min + value * (max - min);
}





enum
{
    kFreeVolumeMode = 0,
    kPanVolumeMode = 1,
    kPanSpanMode = 2,
    kNumberOfModes = 3
};
enum {
    kSourceX = 0,
    kSourceY,
    kSourceD,
    kSourceUnused,
    kParamsPerSource
};
// x, y, attenuation, mute
enum {
    kSpeakerX = 0,
    kSpeakerY,
    kSpeakerA,
    kSpeakerM,
    kSpeakerUnused,
    kParamsPerSpeakers };

#define kConstantOffset (JucePlugin_MaxNumInputChannels * kParamsPerSource + JucePlugin_MaxNumOutputChannels * kParamsPerSpeakers)

enum
{
    kLinkMovement =			0 + kConstantOffset,
    kSmooth =				1 + kConstantOffset,
    kVolumeNear =			2 + kConstantOffset,
    kVolumeMid =			3 + kConstantOffset,
    kVolumeFar =			4 + kConstantOffset,
    kFilterNear =			5 + kConstantOffset,
    kFilterMid =			6 + kConstantOffset,
    kFilterFar =			7 + kConstantOffset,
    kMaxSpanVolume =		8 + kConstantOffset,
    kConstantParameters =	9
};
#define kNumberOfParameters (kConstantParameters + kConstantOffset)

typedef struct
{
    int i;
    float a;
} IndexedAngle;





//==============================================================================
/**
 The processor class of the plug in
 */
class ZirkOscjuceAudioProcessor  : public AudioProcessor,public Timer
{
public:
    //==============================================================================
    //! Constructeur
    ZirkOscjuceAudioProcessor();
    //! Destructeur
    ~ZirkOscjuceAudioProcessor();
    
    //! Called every 50ms;
    void timerCallback();
    
    
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
    inline SoundSource* getSources(){ return _AllSources; }
    //! returns the number of sources on the screen.
    int getNbrSources() { return _NbrSources; }
    //! Set the number of sources.
    void setNbrSources(int newValue) { if ( newValue >-1 && newValue < 9) _NbrSources= newValue; }
    
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
    int getSelectedSource() { return _SelectedSource; }
    //! Set the selected source 
    void setSelectedSource(int selected){ if ( selected >-1 && selected < 8) _SelectedSource= selected;};
    //! Returns the Osc Port for the Zirkonium sending
    int getOscPortZirkonium(){return _OscPortZirkonium;}
    //! Retunrs the Osc port where the iPad messages are received
    String getOscPortIpadIncoming(){ return _OscPortIpadIncoming;}
    //! Returns the Osc iPad port where we send messages
    String getOscPortIpadOutgoing(){ return _OscPortIpadOutgoing;}
    //! Returns the iPad's IP address.
    String getOscAddressIpad() {return _OscAddressIpad; }
    
    
    
    //Import from Octogris
    
    
    
    inline int getParamForSpeakerX(int index) const { return kSpeakerX + JucePlugin_MaxNumInputChannels * kParamsPerSource + index * kParamsPerSpeakers; }
    inline int getParamForSpeakerY(int index) const { return kSpeakerY + JucePlugin_MaxNumInputChannels * kParamsPerSource + index * kParamsPerSpeakers; }
    
    int getOscLeapSource() const { return mOscLeapSource; }
    void setOscLeapSource(int s) { mOscLeapSource = s; }
    
    bool getIsJoystickEnabled() const { return _isJoystickEnabled; }
    void setIsJoystickEnabled(int s) { _isJoystickEnabled = s; }
    
    float getSpeakerX(int index) const { return mParameters.getUnchecked(getParamForSpeakerX(index)); }
    float getSpeakerY(int index) const { return mParameters.getUnchecked(getParamForSpeakerY(index)); }
    float getSourceX(int index) const { return mParameters.getUnchecked(kSourceX + index * kParamsPerSource); }
    float getSourceY(int index) const { return mParameters.getUnchecked(kSourceY + index * kParamsPerSource); }
    float getSourceD(int index) const { return mParameters.getUnchecked(kSourceD + index * kParamsPerSource); }
    
    bool getShowGridLines() const { return mShowGridLines; }
    void setShowGridLines(bool s) { mShowGridLines = s; }
    
    
    int getProcessMode() const { return mProcessMode; }
    void setProcessMode(int s) { mProcessMode = s; jassert(mProcessMode >= 0 && mProcessMode < kNumberOfModes); }
    
    int getParamForSourceX(int index) const { return kSourceX + index * kParamsPerSource; }
    int getParamForSourceY(int index) const { return kSourceY + index * kParamsPerSource; }
    int getParamForSourceD(int index) const { return kSourceD + index * kParamsPerSource; }
    
    int getMovementMode() const { return mMovementMode; }
    void setMovementMode(int s) { mMovementMode = s; }
    
    bool getLinkMovement() { return getParameter(kLinkMovement) > 0.5; }
    void setLinkMovement(bool s) { setParameterNotifyingHost(kLinkMovement, s ? 1 : 0); }
    
    float getDenormedSourceD(int index) const { return denormalize(kSourceMinDistance, kSourceMaxDistance, getSourceD(index)); }
    
    void setSourceXY(int i, FPoint p)
    {
        float r = hypotf(p.x, p.y);
        if (r > kRadiusMax)
        {
            float c = kRadiusMax / r;
            p.x *= c;
            p.y *= c;
        }
        p.x = (p.x + kRadiusMax) / (kRadiusMax*2);
        p.y = (p.y + kRadiusMax) / (kRadiusMax*2);
        setParameterNotifyingHost(getParamForSourceX(i), p.x);
        setParameterNotifyingHost(getParamForSourceY(i), p.y);
    }
    
    
    FPoint getSourceXY(int i)
    {
        float x = getSourceX(i) * (2*kRadiusMax) - kRadiusMax;
        float y = getSourceY(i) * (2*kRadiusMax) - kRadiusMax;
        return FPoint(x, y);
    }
    
    FPoint getSourceXY01(int i)
    {
        float x = getSourceX(i);
        float y = getSourceY(i);
        return FPoint(x, y);
    }
    
    FPoint getSpeakerXY(int i)
    {
        float x = getSpeakerX(i) * (2*kRadiusMax) - kRadiusMax;
        float y = getSpeakerY(i) * (2*kRadiusMax) - kRadiusMax;
        if (mProcessMode != kFreeVolumeMode)
        {
            // force radius to 1
            float r = hypotf(x, y);
            if (r == 0) return FPoint(1, 0);
            x /= r;
            y /= r;
        }
        return FPoint(x, y);
    }
    
    FPoint getSourceRT(int i)
    {
        FPoint p = getSourceXY(i);
        float r = hypotf(p.x, p.y);
        float t = atan2f(p.y, p.x);
        if (t < 0) t += kThetaMax;
        return FPoint(r, t);
    }
    void setSourceXY01(int i, FPoint p)
    {
        p = clampRadius01(p);
        setParameterNotifyingHost(getParamForSourceX(i), p.x);
        setParameterNotifyingHost(getParamForSourceY(i), p.y);
    }
    
    void setSourceRT(int i, FPoint p)
    {
        float x = p.x * cosf(p.y);
        float y = p.x * sinf(p.y);
        setSourceXY(i, FPoint(x, y));
    }
    
    FPoint clampRadius01(FPoint p)
    {
        float dx = p.x - 0.5f;
        float dy = p.y - 0.5f;
        float r = hypotf(dx, dy);
        if (r > 0.5f)
        {
            float c = 0.5f / r;
            dx *= c;
            dy *= c;
            p.x = dx + 0.5f;
            p.y = dy + 0.5f;
        }
        return p;
    }
    FPoint convertXY01(float r, float t)
    {
        float x = r * cosf(t);
        float y = r * sinf(t);
        return FPoint((x + kRadiusMax)/(kRadiusMax*2), (y + kRadiusMax)/(kRadiusMax*2));
    }
    FPoint convertRT(FPoint p)
    {
        float vx = p.x;
        float vy = p.y;
        float r = sqrtf(vx*vx + vy*vy) / kRadiusMax;
        if (r > 1) r = 1;
        float t = atan2f(vy, vx);
        if (t < 0) t += kThetaMax;
        t /= kThetaMax;
        return FPoint(r, t);
    }
    FPoint convertRT01(FPoint p)
    {
        return convertRT(FPoint(p.x * (kRadiusMax*2) - kRadiusMax, p.y * (kRadiusMax*2) - kRadiusMax));
    }
    void setSpeakerXY01(int i, FPoint p)
    {
        p = clampRadius01(p);
        //		setParameterNotifyingHost(getParamForSpeakerX(i), p.x);
        //		setParameterNotifyingHost(getParamForSpeakerY(i), p.y);
        setParameter(getParamForSpeakerX(i), p.x);
        setParameter(getParamForSpeakerY(i), p.y);
    }
    
    enum ParameterIds
    {
        ZirkOSC_Azim_ParamId = 0,
        ZirkOSC_Elev_ParamId,
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
        ZirkOSC_TrajectoryCount_ParamId,
        ZirkOSC_TrajectoriesDuration_ParamId,
        ZirkOSC_SyncWTempo_ParamId,
        ZirkOSC_WriteTrajectories_ParamId,
        totalNumParams                      //50
    };
    
    struct allParameters{
        SoundSource _AllSources [8];
        float   fMovementConstraint; //40
        bool    bisSpanLinked;
        bool    bisOscActive;
        float   fSelectedTrajectory;
        float   fSelectedSource;
        double  dTrajectoryCount;
        double  dTrajectoriesDuration;
        bool    bSyncWTempo;
        bool    bWriteTrajectories;
    };
    
    
    //! Send the current state to all the iPad and Zirkonium
    void sendOSCValues();
    
    //! Getter constrain type as integer, since parameters need to be stored as floats [0,1]
    int getSelectedMovementConstraintAsInteger();
    
    //! Getter for trajectory as integer, since parameters need to be stored as floats [0,1]
    int getSelectedTrajectoryAsInteger();
    
    int getSelectedTrajectoryDirection();
    
    int getSelectedTrajectoryReturn();
    
    //! Retunrs true if the Editor has to refresh the Gui.
    bool hasToRefreshGui(){return _RefreshGui;};
    //! If you want to refresh the Gui from the Processor.
    void setRefreshGui(bool gui) { _RefreshGui = gui;};
    //! Change the sending OSC port of the zirkonium
    void changeZirkoniumOSCPort(int newPort);
    //! Send the configuration to the iPad (assignment Position -> id, nbr source)
    void sendOSCConfig();
    //! Send the movement type, selected constrain.
    void sendOSCMovementType();
    //! Change the sending OSC port of the iPad
    void changeOSCSendIPad(int newPort, String newAddress);
    //! Change the receiving OSC port (server).
    void changeOSCReceiveIpad(int port);
    //! Returns the Editor.
    AudioProcessorEditor* getEditor() {return _Editor;};
    //! Set the width that the UI was last set to
    void setLastUiWidth(int lastUiWidth);
    //! Get the width that the UI was last set to
    int getLastUiWidth();
    //! Set the height that the UI was last set to
    void setLastUiHeight(int lastUiHeight);
    //! Return the height that the UI was last set to
    int getLastUiHeight();
    //! Return the size of the dome radius
    int getDomeRadius();
    //! Set the size of the dome radius
    void setDomeRadius(int iNewRadius);
    
    void storeCurrentLocations();
    void restoreCurrentLocations();
    
    //radius of the dome
    static int s_iDomeRadius;
    
    bool m_bUseIpad;
    
    bool isTrajectoryDone();
    
    
    //NEW TRAJECTORY CLASS METHODS
    void setTrajectory(Trajectory::Ptr t) { mTrajectory = t; }
  	Trajectory::Ptr getTrajectory() { return mTrajectory; }

    void moveTrajectoriesWithConstraints(Point<float> &newLocation);
    
    void askForGuiRefresh(){_RefreshGui=true;};
    
private:
    

    
    
    
    void processTrajectories();
    
    void stopTrajectory();
        
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZirkOscjuceAudioProcessor)
    //! Whether the editor has to refresh the GUI
    bool _RefreshGui = false;
    //! Current number of sources on the screnn
    int _NbrSources;
    //! float ID of the selected movement constraint
    float _SelectedMovementConstraint;
    //! int ID of the selected movement constraint IMPORTANT: need to be set manually whenever float version of parameter is changed
    int m_iSelectedMovementConstraint;
    //! float ID of the selected trajectory
    float _SelectedTrajectory;
    
    float m_fSelectedTrajectoryDirection;
    
    float m_fSelectedTrajectoryReturn;
    
    //! Tab position of the selected source
    int _SelectedSource;
    //! Osc port to send to the Zirkonium 
    int _OscPortZirkonium;
    //! The editor
    AudioProcessorEditor* _Editor;
    //! Sources array
    SoundSource _AllSources [8];
    //! Osc Sever thread (receiving)
    lo_server_thread _St;
    //! Zirkonium OSC address (sending)
    lo_address _OscZirkonium;
    //! Ipad OSC address (sending)
    lo_address _OscIpad;
    //! Outgoing port to the iPad 
    String _OscPortIpadOutgoing;
    //! iPad address
    String _OscAddressIpad;
    //! Zirkonium incoming port
    String _OscPortIpadIncoming;
    //! last saved ui width
    int _LastUiWidth;
    //! last saved ui height
    int _LastUiHeight;
    //! Whether we're sending OSC messages to the zirkonium
    bool _isOscActive;
    //! If the span are linked
    bool _isSpanLinked;
    
    PluginHostType host;
    
    allParameters m_parameterBuffer;
    
    //Import from octogris 
    
    int mMovementMode;
    Array<float> mParameters;
    int mProcessMode;
    bool mShowGridLines;
    int mOscLeapSource = 1;
    bool _isJoystickEnabled;


    //OLD TRAJECTORIES
    
    //! Number of trajectories to draw in trajectory section
    double _TrajectoryCount;
    
    //! Duration of trajectory movement
    double _TrajectoriesDuration;
    
    //!Whether to sync trajectories with tempo
    bool _isSyncWTempo;
    
    //!Whether to write trajectory or not
    bool _isWriteTrajectory;
    
    int _SelectedSourceForTrajectory;
    
    //NEW TRAJECTORY
    Trajectory::Ptr mTrajectory;
    
    int64 mLastTimeInSamples;

};

#endif  // __PLUGINPROCESSOR_H_F70DA35D__
