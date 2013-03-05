/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

//       lo_send(mOsc, "/pan/az", "i", ch);

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <string.h>
#include <sstream>

// using stringstream constructors.
#include <iostream>

using namespace std;
//==============================================================================
ZirkOscjuceAudioProcessor::ZirkOscjuceAudioProcessor():
currentSource()
{
    nbrSources      = 8;
    selectedSource  = 0;
    gain            = ZirkOSC_Gain_Def;
    azimuth         = ZirkOSC_Azim_Def;
    azimuth_delta   = ZirkOSC_AzimDelta_Def;
    azimuth_span    = ZirkOSC_AzimSpan_Def;
    elevation       = ZirkOSC_Elev_Def;
    elevation_delta = ZirkOSC_ElevDelta_Def;
    elevation_span  = ZirkOSC_ElevSpan_Def;
    mOsc            = lo_address_new("127.0.0.1", "10029");
    
 //   listeSource.push_back(*new SoundSource(10.0,0.0));
  //  currentSource = listeSource.begin();
    for(int i=0; i<8; i++)
        tabSource[i]=SoundSource(0.0,0.0);
    sendOSCValues();
   // lastPosInfo.resetToDefault();
    
}

ZirkOscjuceAudioProcessor::~ZirkOscjuceAudioProcessor()
{
    //lo_address_free(mOsc);
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

float ZirkOscjuceAudioProcessor::getParameter (int index)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
    for(int i = 0; i<nbrSources;i++){
        if      (ZirkOSC_Azim_Param + (i*7) == index)       return tabSource[i].getAzimuth();
        else if (ZirkOSC_AzimSpan_Param + (i*7) == index)   return tabSource[i].getAzimuth_span();
        else if (ZirkOSC_Channel_Param + (i*7) == index)    return tabSource[i].getChannel();
        else if (ZirkOSC_Elev_Param + (i*7) == index)       return tabSource[i].getElevation();
        else if (ZirkOSC_ElevSpan_Param + (i*7) == index)   return tabSource[i].getElevation_span();
        else if (ZirkOSC_Gain_Param + (i*7) == index)       return tabSource[i].getGain();
        else;
    }
}

void ZirkOscjuceAudioProcessor::setParameter (int index, float newValue)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
    for(int i = 0; i<nbrSources;i++){
        if      (ZirkOSC_Azim_Param + (i*7) == index)       {tabSource[i].setAzimuth(newValue); break;}
        else if (ZirkOSC_AzimSpan_Param + (i*7) == index)   {tabSource[i].setAzimuth_span(newValue); break;}
        else if (ZirkOSC_Channel_Param + (i*7) == index)    {tabSource[i].setChannel(newValue); break;}
        else if (ZirkOSC_Elev_Param + (i*7) == index)       {tabSource[i].setElevation(newValue); break;}
        else if (ZirkOSC_ElevSpan_Param + (i*7) == index)   {tabSource[i].setElevation_span(newValue); break;}
        else if (ZirkOSC_Gain_Param + (i*7) == index)       {tabSource[i].setGain(newValue); break;}
        else;
    }
    sendOSCValues();

}

const String ZirkOscjuceAudioProcessor::getParameterName (int index)
{
    for(int i = 0; i<nbrSources;i++){
        if      (ZirkOSC_Azim_Param + (i*7) == index)       return ZirkOSC_Azim_name[i];
        else if (ZirkOSC_AzimSpan_Param + (i*7) == index)   return ZirkOSC_AzimSpan_name[i];
        else if (ZirkOSC_Channel_Param + (i*7) == index)    return ZirkOSC_Channel_name[i];
        else if (ZirkOSC_Elev_Param + (i*7) == index)       return ZirkOSC_Elev_name[i];
        else if (ZirkOSC_ElevSpan_Param + (i*7) == index)   return ZirkOSC_ElevSpan_name[i];
        else if (ZirkOSC_Gain_Param + (i*7) == index)       return ZirkOSC_Gain_name[i];
        else;
    }
    
    
    return String::empty;
}



void ZirkOscjuceAudioProcessor::sendOSCValues(){
    for(int ch=0;ch<nbrSources;ch++){
        float azim_osc = PercentToHR(tabSource[ch].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max) /180.;
        float elev_osc = PercentToHR(tabSource[ch].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max)/180.;
        float azimspan_osc = PercentToHR(tabSource[ch].getAzimuth_span(), ZirkOSC_AzimSpan_Min,ZirkOSC_AzimSpan_Max)/180.;
        float elevspan_osc = PercentToHR(tabSource[ch].getElevation_span(), ZirkOSC_ElevSpan_Min, ZirkOSC_Elev_Max)/180.;
        float gain_osc = tabSource[ch].getGain();
        lo_send(mOsc, "/pan/az", "ifffff", ch, azim_osc, elev_osc, azimspan_osc, elevspan_osc, gain_osc);
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
    return 0;
}

int ZirkOscjuceAudioProcessor::getCurrentProgram()
{
    return 0;
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

void ZirkOscjuceAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    for (int channel = 0; channel < getNumInputChannels(); ++channel)
    {
        float* channelData = buffer.getSampleData (channel);
        buffer.applyGain (channel, 0, buffer.getNumSamples(), gain);
        // ..do something to the data...
    }

    // In case we have more outputs than inputs, we'll clear any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }
    
}

//==============================================================================
bool ZirkOscjuceAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ZirkOscjuceAudioProcessor::createEditor()
{
    return new ZirkOscjuceAudioProcessorEditor (this);
}

//==============================================================================
void ZirkOscjuceAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    XmlElement xml ("MYPLUGINSETTINGS");
    xml.setAttribute (ZirkOSC_Gain_name[0], gain);
    xml.setAttribute (ZirkOSC_Azim_name[0], azimuth);
    xml.setAttribute (ZirkOSC_Elev_name[0], elevation);
    copyXmlToBinary (xml, destData);
}

void ZirkOscjuceAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    // This getXmlFromBinary() helper function retrieves our XML from the binary blob..
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState != nullptr)
    {
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName ("MYPLUGINSETTINGS"))
        {
            // ok, now pull out our parameters..
            
            gain  = (float) xmlState->getDoubleAttribute (ZirkOSC_Gain_name[0], gain);
            elevation  = (float) xmlState->getDoubleAttribute (ZirkOSC_Elev_name[0], elevation);
            azimuth  = (float) xmlState->getDoubleAttribute (ZirkOSC_Azim_name[0], azimuth);

        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ZirkOscjuceAudioProcessor();

}
