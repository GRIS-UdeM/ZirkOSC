/*
 ==============================================================================

 This file was auto-generated!

 It contains the basic startup code for a Juce application.

 ==============================================================================
 */

#ifndef __PLUGINPROCESSOR_H_F70DA35D__
#define __PLUGINPROCESSOR_H_F70DA35D__

#include "../JuceLibraryCode/JuceHeader.h"

#include "ZirkConstants.h"
#include "lo.h"     
#include "SoundSource.h"

//==============================================================================
/**
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
    
    
    //bool beginGesture =false;
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();
    //static     int receivePositionUpdate(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

    //==============================================================================
    AudioProcessorEditor* createEditor();
    
    bool hasEditor() const;

    //==============================================================================
    const String getName() const;

    int getNumParameters();


    float getParameter (int index);
    void setParameter (int index, float newValue);

    const String getParameterName (int index);
    const String getParameterText (int index);

    const String getInputChannelName (int channelIndex) const;
    const String getOutputChannelName (int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;


    bool acceptsMidi() const;
    bool producesMidi() const;
    bool silenceInProducesSilenceOut() const;
    Point<float> domeToScreen(Point<float>);
    //==============================================================================
    int getNumPrograms();
    int getCurrentProgram();
    void setCurrentProgram (int index);
    const String getProgramName (int index);
    void changeProgramName (int index, const String& newName);


    //==============================================================================
    void getStateInformation (MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);

    AudioPlayHead::CurrentPositionInfo lastPosInfo;
     //OSC Port Zirkonium
    
    
    
    enum Parameters
    {
        ZirkOSC_Azim_Param = 0,
        ZirkOSC_Elev_Param,
        ZirkOSC_Channel_Param,
        ZirkOSC_ElevDelta_Param,
        ZirkOSC_AzimSpan_Param,
        ZirkOSC_ElevSpan_Param,
        ZirkOSC_Gain_Param,
        ZirkOSC_Azim_Param_1,
        ZirkOSC_Elev_Param_1,
        ZirkOSC_Channel_Param_1,
        ZirkOSC_ElevDelta_Param_1,
        ZirkOSC_AzimSpan_Param_1,
        ZirkOSC_ElevSpan_Param_1,
        ZirkOSC_Gain_Param_1,
        ZirkOSC_Azim_Param_2,
        ZirkOSC_Elev_Param_2,
        ZirkOSC_Channel_Param_2,
        ZirkOSC_ElevDelta_Param_2,
        ZirkOSC_AzimSpan_Param_2,
        ZirkOSC_ElevSpan_Param_2,
        ZirkOSC_Gain_Param_2,
        ZirkOSC_Azim_Param_3,
        ZirkOSC_Elev_Param_3,
        ZirkOSC_Channel_Param_3,
        ZirkOSC_ElevDelta_Param_3,
        ZirkOSC_AzimSpan_Param_3,
        ZirkOSC_ElevSpan_Param_3,
        ZirkOSC_Gain_Param_3,
        ZirkOSC_Azim_Param_4,
        ZirkOSC_Elev_Param_4,
        ZirkOSC_Channel_Param_4,
        ZirkOSC_ElevDelta_Param_4,
        ZirkOSC_AzimSpan_Param_4,
        ZirkOSC_ElevSpan_Param_4,
        ZirkOSC_Gain_Param_4,
        ZirkOSC_Azim_Param_5,
        ZirkOSC_Elev_Param_5,
        ZirkOSC_Channel_Param_5,
        ZirkOSC_ElevDelta_Param_5,
        ZirkOSC_AzimSpan_Param_5,
        ZirkOSC_ElevSpan_Param_5,
        ZirkOSC_Gain_Param_5,
        ZirkOSC_Azim_Param_6,
        ZirkOSC_Elev_Param_6,
        ZirkOSC_Channel_Param_6,
        ZirkOSC_ElevDelta_Param_6,
        ZirkOSC_AzimSpan_Param_6,
        ZirkOSC_ElevSpan_Param_6,
        ZirkOSC_Gain_Param_6,
        ZirkOSC_Azim_Param_7,
        ZirkOSC_Elev_Param_7,
        ZirkOSC_Channel_Param_7,
        ZirkOSC_ElevDelta_Param_7,
        ZirkOSC_AzimSpan_Param_7,
        ZirkOSC_ElevSpan_Param_7,
        ZirkOSC_Gain_Param_7,
        totalNumParams
    };
    void sendOSCValues();
    //! Getter constrain type
    int getSelectedConstrain();
    //! Setter constrain type
    void setSelectedContrain(int constrain);
    void changeOSCPort(int newPort);
    void sendOSCConfig();
    void sendOSCMovementType();
    
    void changeOSCSendIPad(int newPort, String newAddress);
    void changeOSCPortReceive(int port);

private:
    
    

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZirkOscjuceAudioProcessor)
    bool _RefreshGui = false;
    
    int _NbrSources;
    int _SelectedConstrain=1;
    int _SelectedSource;
    
    int _OscPortZirkonium;
    
    AudioProcessorEditor* _Editor;
    
    SoundSource _TabSource [8];
    lo_server_thread _St;
    lo_address _OscZirkonium;
    lo_address _OscIpad;
    String _OscPortIpadOutgoing = "10112";
    String _OscAddressIpad = "10.0.1.3";
    String _OscPortIpadIncoming = "10114";
   /* TextButton* button1;
    Slider* slider1;
    Label* label;*/

};

#endif  // __PLUGINPROCESSOR_H_F70DA35D__
