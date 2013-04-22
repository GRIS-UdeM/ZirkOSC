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
void error(int num, const char *m, const char *path);
int receivePositionUpdate(const char *path, const char *types, lo_arg **argv, int argc,void *data, void *user_data);

ZirkOscjuceAudioProcessor::ZirkOscjuceAudioProcessor():
currentSource()
{
    nbrSources      = 1;
    selectedSource  = 0;
    gain            = ZirkOSC_Gain_Def;
    azimuth         = ZirkOSC_Azim_Def;
    azimuth_delta   = ZirkOSC_AzimDelta_Def;
    azimuth_span    = ZirkOSC_AzimSpan_Def;
    elevation       = ZirkOSC_Elev_Def;
    elevation_delta = ZirkOSC_ElevDelta_Def;
    elevation_span  = ZirkOSC_ElevSpan_Def;
    mOsc            = lo_address_new("127.0.0.1", "10001");
    mOscIpad        = lo_address_new("10.0.1.3", "10114");
    st              = lo_server_thread_new("10116", error);
    lo_server_thread_add_method(st, "/pan/az", "ifffff", receivePositionUpdate, this);

    //   listeSource.push_back(*new SoundSource(10.0,0.0));
    //  currentSource = listeSource.begin();
    for(int i=0; i<8; i++)
        tabSource[i]=SoundSource(0.0,0.0);
    
    lo_server_thread_start(st);
    // lastPosInfo.resetToDefault();

}

void error(int num, const char *m, const char *path){
    printf("liblo server error %d in path %s: %s\n", num, path, m);
    fflush(stdout);
}

ZirkOscjuceAudioProcessor::~ZirkOscjuceAudioProcessor()
{
    lo_server st2 = st;

    lo_server_thread_stop(st2);
    lo_server_thread_free(st2);
    st = NULL;
    lo_address osc = mOsc;
    lo_address osc2 = mOscIpad;
    mOsc = NULL;
    mOscIpad = NULL;
    lo_address_free(osc);
    lo_address_free(osc2);
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
    for(int i = 0; i<8;i++){
        if      (ZirkOSC_Azim_Param + (i*7) == index)       return tabSource[i].getAzimuth();
        else if (ZirkOSC_AzimSpan_Param + (i*7) == index)   return tabSource[i].getAzimuth_span();
        //else if (ZirkOSC_Channel_Param + (i*7) == index)    return tabSource[i].getChannel();
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
    for(int i = 0; i<8;i++){
        if      (ZirkOSC_Azim_Param + (i*7) == index)       {tabSource[i].setAzimuth(newValue); break;}
        else if (ZirkOSC_AzimSpan_Param + (i*7) == index)   {tabSource[i].setAzimuth_span(newValue); break;}
        // else if (ZirkOSC_Channel_Param + (i*7) == index)    {tabSource[i].setChannel(newValue); break;}
        else if (ZirkOSC_Elev_Param + (i*7) == index)       {tabSource[i].setElevation(newValue); break;}
        else if (ZirkOSC_ElevSpan_Param + (i*7) == index)   {tabSource[i].setElevation_span(newValue); break;}
        else if (ZirkOSC_Gain_Param + (i*7) == index)       {tabSource[i].setGain(newValue); break;}
        else;
    }
    //sendOSCValues();

}

const String ZirkOscjuceAudioProcessor::getParameterName (int index)
{
    for(int i = 0; i<8;i++){
        if      (ZirkOSC_Azim_Param + (i*7) == index)       return ZirkOSC_Azim_name[i];
        else if (ZirkOSC_AzimSpan_Param + (i*7) == index)   return ZirkOSC_AzimSpan_name[i];
        //else if (ZirkOSC_Channel_Param + (i*7) == index)    return ZirkOSC_Channel_name[i];
        else if (ZirkOSC_Elev_Param + (i*7) == index)       return ZirkOSC_Elev_name[i];
        else if (ZirkOSC_ElevSpan_Param + (i*7) == index)   return ZirkOSC_ElevSpan_name[i];
        else if (ZirkOSC_Gain_Param + (i*7) == index)       return ZirkOSC_Gain_name[i];
        else;
    }


    return String::empty;
}



void ZirkOscjuceAudioProcessor::sendOSCValues(){
    for(int i=0;i<nbrSources;i++){
        float azim_osc = PercentToHR(tabSource[i].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max) /180.;
        float elev_osc = PercentToHR(tabSource[i].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max)/180.;
        float azimspan_osc = PercentToHR(tabSource[i].getAzimuth_span(), ZirkOSC_AzimSpan_Min,ZirkOSC_AzimSpan_Max)/180.;
        float elevspan_osc = PercentToHR(tabSource[i].getElevation_span(), ZirkOSC_ElevSpan_Min, ZirkOSC_Elev_Max)/180.;
        int channel_osc = tabSource[i].getChannel()-1;
        float gain_osc = tabSource[i].getGain();
        lo_send(mOsc, "/pan/az", "ifffff", channel_osc, azim_osc, elev_osc, azimspan_osc, elevspan_osc, gain_osc);
        azim_osc = azim_osc * M_PI;
        elev_osc = elev_osc * M_PI;
        azimspan_osc = azimspan_osc * M_PI;
        elevspan_osc = elevspan_osc * M_PI;
        lo_send(mOscIpad, "/pan/az", "ifffff", channel_osc+1, azim_osc, elev_osc, azimspan_osc, elevspan_osc, gain_osc);
    }
}

void ZirkOscjuceAudioProcessor::sendOSCConfig(){
    lo_send(mOscIpad, "/maxsource", "iiiiiiiii", nbrSources, tabSource[0].getChannel(), tabSource[1].getChannel(), tabSource[2].getChannel(), tabSource[3].getChannel(), tabSource[4].getChannel(), tabSource[5].getChannel(), tabSource[6].getChannel(), tabSource[7].getChannel());
    
}

void ZirkOscjuceAudioProcessor::changeOSCPort(int newPort){

    lo_address osc = mOsc;
    moscPort = newPort;
	mOsc = NULL;
    lo_address_free(osc);
	char port[32];
	snprintf(port, sizeof(port), "%d", newPort);
	mOsc = lo_address_new("127.0.0.1", port);

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
    editor = new ZirkOscjuceAudioProcessorEditor (this);
    return editor;
}

//==============================================================================
void ZirkOscjuceAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    XmlElement xml ("ZIRKOSCJUCESETTINGS");
    xml.setAttribute("PortOSC", moscPort);
   
    xml.setAttribute("NombreSources", nbrSources);
    for(int i =0;i<8;i++){
        String channel = "Channel";
        String azimuth = "Azimuth";
        String elevation = "Elevation";
        String azimuthSpan = "AzimuthSpan";
        String elevationSpan = "ElevationSpan";
        String gain = "Gain";
        channel.append(String(i), 10);
        xml.setAttribute(channel, tabSource[i].getChannel());
        azimuth.append(String(i), 10);
        xml.setAttribute(azimuth, tabSource[i].getAzimuth());
        elevation.append(String(i), 10);
        xml.setAttribute(elevation, tabSource[i].getElevationRawValue());
        azimuthSpan.append(String(i), 10);
        xml.setAttribute(azimuthSpan, tabSource[i].getAzimuth_span());
        elevationSpan.append(String(i), 10);
        xml.setAttribute(elevationSpan, tabSource[i].getElevation_span());
        gain.append(String(i), 10);
        xml.setAttribute(gain, tabSource[i].getChannel());
        
    }
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
        if (xmlState->hasTagName ("ZIRKOSCJUCESETTINGS"))
        {
            // ok, now pull out our parameters..
            moscPort = xmlState->getIntAttribute("PortOSC", 20000);
            if(moscPort<0 || moscPort>100000){
                moscPort = 20000;
            }
            nbrSources = xmlState->getIntAttribute("NombreSources", 1);
            
            for (int i=0;i<8;i++){
                String channel = "Channel";
                String azimuth = "Azimuth";
                String elevation = "Elevation";
                String azimuthSpan = "AzimuthSpan";
                String elevationSpan = "ElevationSpan";
                String gain = "Gain";
                channel.append(String(i), 10);
                azimuth.append(String(i), 10);
                elevation.append(String(i), 10);
                azimuthSpan.append(String(i), 10);
                elevationSpan.append(String(i), 10);
                gain.append(String(i), 10);
                tabSource[i].setChannel(xmlState->getIntAttribute(channel , 0));
                tabSource[i].setAzimuth((float) xmlState->getDoubleAttribute(azimuth,0));
                tabSource[i].setElevation((float) xmlState->getDoubleAttribute(elevation,0));
                tabSource[i].setAzimuth_span((float) xmlState->getDoubleAttribute(azimuthSpan,0));
                tabSource[i].setElevation_span((float) xmlState->getDoubleAttribute(elevationSpan,0));
                tabSource[i].setGain((float) xmlState->getDoubleAttribute(gain,1 ));
            }
            changeOSCPort(moscPort);
            sendOSCValues();
            refreshGui=true;
        }
    }
}

int receivePositionUpdate(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data){
 
    
    //channel send begin with 0; ifffff", channel_osc, azim_osc, elev_osc, azimspan_osc, elevspan_osc, gain_osc); tout les angles en RADIAN
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    int channel_osc = argv[0]->i;
    int i =0;
    for(i=0;i<processor->nbrSources;i++){
        if (processor->tabSource[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->nbrSources){
        return 0;
    }
    float azim_osc = argv[1]->f;
    float elev_osc = argv[2]->f;
 //   float azimspan_osc = argv[3]->f;
   // float elevspan_osc = argv[4]->f;
   // float gain_osc = argv[5]->f;

        processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param+ i*7);
        processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param + i*7,
                                                 HRToPercent(azim_osc, -M_PI, M_PI));
        processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param+ i*7);
        
        processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param+ i*7);
        processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + i*7,
                                              HRToPercent(elev_osc, 0.0, M_PI/2.0));
        processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param+ i*7);
     /*
        processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_Param+ i*7);
        processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_Param + i*7,
                                              HRToPercent(elevspan_osc, 0.0, M_PI/2.0));
        processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_Param+ i*7);
        
        //processor->tabSource[channel_osc].setAzimuth(HRToPercent(azim_osc, -M_PI, M_PI));
      //  processor->tabSource[channel_osc].setElevation(HRToPercent(elev_osc, 0.0, M_PI/2.0));
        //processor->tabSource[channel_osc].setElevation_span(HRToPercent(elevspan_osc, 0.0, M_PI/2.0));
        
        processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_Param+ i*7);
        processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_Param + i*7,
                                              HRToPercent(azimspan_osc, 0.0, 2*M_PI));
        processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_Param+ i*7);
      
        processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Gain_Param+ i*7);
        processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Gain_Param + i*7,
                                              gain_osc);
        processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Gain_Param+ i*7);*/
    
    //processor->editor->repaint();
    processor->sendOSCValues();
    return 0;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ZirkOscjuceAudioProcessor();

}
