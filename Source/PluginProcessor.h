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
#include <list.h>

#include "SoundSource.h";

//==============================================================================
/**
*/
class ZirkOscjuceAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    ZirkOscjuceAudioProcessor();
    ~ZirkOscjuceAudioProcessor();



    list<SoundSource> listeSource;
    list<SoundSource>::iterator currentSource;

    SoundSource tabSource [8];
    int selectedSource;
    int nbrSources;
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();

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
    long int frame = 0;
    enum Parameters
    {
        ZirkOSC_Azim_Param          = 0,
        ZirkOSC_Elev_Param          = 1,
        ZirkOSC_Channel_Param       = 2,
        ZirkOSC_ElevDelta_Param     = 3,
        ZirkOSC_AzimSpan_Param      = 4,
        ZirkOSC_ElevSpan_Param      = 5,
        ZirkOSC_Gain_Param          = 6,
        ZirkOSC_Azim_Param_1        = 7,
        ZirkOSC_Elev_Param_1        = 8,
        ZirkOSC_Channel_Param_1     = 9,
        ZirkOSC_ElevDelta_Param_1   = 10,
        ZirkOSC_AzimSpan_Param_1    = 11,
        ZirkOSC_ElevSpan_Param_1    = 12,
        ZirkOSC_Gain_Param_1        = 13,
        ZirkOSC_Azim_Param_2        = 14,
        ZirkOSC_Elev_Param_2        = 15,
        ZirkOSC_Channel_Param_2     = 16,
        ZirkOSC_ElevDelta_Param_2   = 17,
        ZirkOSC_AzimSpan_Param_2    = 18,
        ZirkOSC_ElevSpan_Param_2    = 19,
        ZirkOSC_Gain_Param_2        = 20,
        ZirkOSC_Azim_Param_3        = 21,
        ZirkOSC_Elev_Param_3        = 22,
        ZirkOSC_Channel_Param_3     = 23,
        ZirkOSC_ElevDelta_Param_3   = 24,
        ZirkOSC_AzimSpan_Param_3    = 25,
        ZirkOSC_ElevSpan_Param_3    = 26,
        ZirkOSC_Gain_Param_3        = 27,
        ZirkOSC_Azim_Param_4        = 28,
        ZirkOSC_Elev_Param_4        = 29,
        ZirkOSC_Channel_Param_4     = 30,
        ZirkOSC_ElevDelta_Param_4   = 31,
        ZirkOSC_AzimSpan_Param_4    = 32,
        ZirkOSC_ElevSpan_Param_4    = 33,
        ZirkOSC_Gain_Param_4        = 34,
        ZirkOSC_Azim_Param_5        = 35,
        ZirkOSC_Elev_Param_5        = 36,
        ZirkOSC_Channel_Param_5     = 37,
        ZirkOSC_ElevDelta_Param_5   = 38,
        ZirkOSC_AzimSpan_Param_5    = 39,
        ZirkOSC_ElevSpan_Param_5    = 40,
        ZirkOSC_Gain_Param_5        = 41,
        ZirkOSC_Azim_Param_6        = 42,
        ZirkOSC_Elev_Param_6        = 43,
        ZirkOSC_Channel_Param_6     = 44,
        ZirkOSC_ElevDelta_Param_6   = 45,
        ZirkOSC_AzimSpan_Param_6    = 46,
        ZirkOSC_ElevSpan_Param_6    = 47,
        ZirkOSC_Gain_Param_6        = 48,
        ZirkOSC_Azim_Param_7        = 49,
        ZirkOSC_Elev_Param_7        = 50,
        ZirkOSC_Channel_Param_7     = 51,
        ZirkOSC_ElevDelta_Param_7   = 52,
        ZirkOSC_AzimSpan_Param_7    = 53,
        ZirkOSC_ElevSpan_Param_7    = 54,
        ZirkOSC_Gain_Param_7        = 55,
        totalNumParams              = 56
    };
    float gain, azimuth, elevation, azimuth_delta, elevation_delta, azimuth_span, elevation_span;
    void sendOSCValues();


private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZirkOscjuceAudioProcessor)
    lo_address mOsc;
    TextButton* button1;
    Slider* slider1;
    Label* label;

};

#endif  // __PLUGINPROCESSOR_H_F70DA35D__
