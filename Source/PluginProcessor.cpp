/*
 ==============================================================================

 ==============================================================================
 */

//lo_send(mOsc, "/pan/az", "i", ch);

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <string.h>
#include <sstream>
#include <regex.h>
#include <arpa/inet.h>  //for inet_pton


// using stringstream constructors.
#include <iostream>

using namespace std;
//==============================================================================
void error(int num, const char *m, const char *path);
int receivePositionUpdate(const char *path, const char *types, lo_arg **argv, int argc,void *data, void *user_data);
int receiveBeginTouch(const char *path, const char *types, lo_arg **argv, int argc,void *data, void *user_data);
int receiveEndTouch(const char *path, const char *types, lo_arg **argv, int argc,void *data, void *user_data);
int receiveAzimuthSpanUpdate(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
int receiveAzimuthSpanBegin(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
int receiveAzimuthSpanEnd(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
int receiveElevationSpanUpdate(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
int receiveElevationSpanBegin(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
int receiveElevationSpanEnd(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);

int ZirkOscjuceAudioProcessor::s_iDomeRadius = 150;

ZirkOscjuceAudioProcessor::ZirkOscjuceAudioProcessor()
:_NbrSources(1),
_SelectedMovementConstraint(.0f),
_SelectedTrajectory(.0f),
_SelectedSource(0),
_OscPortZirkonium(18032),
_OscPortIpadOutgoing("10112"),
_OscAddressIpad("10.0.1.3"),
_OscPortIpadIncoming("10114"),
_isOscActive(true),
_isSpanLinked(true),
_TrajectoryCount(0),
_TrajectoriesDuration(0),
_TrajectoryAllDrawn(false),
_TrajectoryBeginTime(0),
_TrajectoryInitialAzimuth(0),
_TrajectoryInitialElevation(0),
_TrajectorySingleLength(0),
m_bTrajectoryElevationDecreasing(false),
m_bTrajectoryAddPositive(true),
_TrajectoryJustCompletedSingle(false),
iProcessBlockCounter(0),
_isSyncWTempo(false),
_isWriteTrajectory(false),
_SelectedSourceForTrajectory(0),
_WasPlayingOnPrevFrame(false),
_JustsEndedPlaying(false)
{
  
    for(int i=0; i<8; ++i){
        _AllSources[i]=SoundSource(0.0+((float)i/10.0),0.0);
    }
    _OscZirkonium   = lo_address_new("127.0.0.1", "10001");
    _OscIpad        = lo_address_new("10.0.1.3", "10114");
    _St             = lo_server_thread_new("10116", error);
    if(_St){
        lo_server_thread_add_method(_St, "/pan/az", "ifffff", receivePositionUpdate, this);
        lo_server_thread_add_method(_St, "/begintouch", "i", receiveBeginTouch, this);
        lo_server_thread_add_method(_St, "/endtouch", "i", receiveEndTouch, this);
        lo_server_thread_add_method(_St, "/moveAzimSpan", "if", receiveAzimuthSpanUpdate, this);
        lo_server_thread_add_method(_St, "/beginAzimSpanMove", "i", receiveAzimuthSpanBegin, this);
        lo_server_thread_add_method(_St, "/endAzimSpanMove", "i", receiveAzimuthSpanEnd, this);
        lo_server_thread_add_method(_St, "/moveElevSpan", "if", receiveElevationSpanUpdate, this);
        lo_server_thread_add_method(_St, "/beginElevSpanMove", "i", receiveElevationSpanBegin, this);
        lo_server_thread_add_method(_St, "/endElevSpanMove", "i", receiveElevationSpanEnd, this);
        
        lo_server_thread_start(_St);
    }
    
    //default values for ui dimensions
    _LastUiWidth = ZirkOSC_Window_Default_Width;
    _LastUiHeight = ZirkOSC_Window_Default_Height;
    
    startTimer (50);
}


void error(int num, const char *m, const char *path){
    printf("liblo server error %d in path %s: %s\n", num, path, m);
    fflush(stdout);
}

void ZirkOscjuceAudioProcessor::timerCallback(){
    const MessageManagerLock mmLock;
    sendOSCValues();
}


ZirkOscjuceAudioProcessor::~ZirkOscjuceAudioProcessor()
{
    if (_St){
        lo_server st2 = _St;
        lo_server_thread_stop(st2);
        lo_server_thread_free(st2);
        _St = NULL;
    }
    lo_address osc = _OscZirkonium;
    lo_address osc2 = _OscIpad;
    if (osc){
        lo_address_free(osc);
    }
    if (osc2){
        lo_address_free(osc2);
    }
    _OscZirkonium = NULL;
    _OscIpad = NULL;
}

//this will only be called in logic when actually processing sound, ie need to toggle I button in track
void ZirkOscjuceAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    //if write trajectory is enabled (button toggled on in editor)
    if (_isWriteTrajectory){
        
        //get current playhead info
        playHead->getCurrentPosition(_CurrentPlayHeadInfo);
        
        if(_CurrentPlayHeadInfo.isPlaying){
        
            if (!_TrajectoryAllDrawn){
                
                //this is just to let Logic time to clear its buffers, to get a clean, up-to-date _CurrentPlayHeadInfo
                if (iProcessBlockCounter < 3){
                    ++iProcessBlockCounter;
                    return;
                }
                
                //if we were not playing on previous frame, this is the first playing frame.
                if (!_WasPlayingOnPrevFrame){
                    
                    
                    //store begin time
                    _TrajectoryBeginTime = _CurrentPlayHeadInfo.timeInSeconds;
                    
                    //store initial parameter value
                    _TrajectoryInitialAzimuth   = getParameter(_SelectedSourceForTrajectory*5);
                    _TrajectoryInitialElevation = getParameter((_SelectedSourceForTrajectory*5)+1);
                    
                    _TrajectorySingleLength = _TrajectoriesDuration / _TrajectoryCount;
                    
                    _WasPlayingOnPrevFrame = true;
                    
                    cout << "____________________________BEGIN PARAMETER CHANGE______________________________________________\n";
                    
                    //get automation started on currently selected source
                    _SelectedSourceForTrajectory = getSelectedSource();
                    
                    
                    switch (getSelectedTrajectoryAsInteger()) {
                        case ZirkOscjuceAudioProcessorEditor::Circle :
                            beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + (_SelectedSourceForTrajectory*5));
                            break;
                            
                        case ZirkOscjuceAudioProcessorEditor::Pendulum :
                            //to make sure that the pendulum movement goes inward (towards top of dome), transpose _TrajectoryInitialElevation to [0, 1/4], so that sin starts between [0, pi/2]
                            _TrajectoryInit rialElevation *= .25;
                        case ZirkOscjuceAudioProcessorEditor::Spiral :
                            beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + (_SelectedSourceForTrajectory*5));
                            beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + (_SelectedSourceForTrajectory*5));
                            break;
                            
                        default:
                            break;
                    }
                



                    cout << "_TrajectoryBeginTime: " << _TrajectoryBeginTime << endl;
                    cout << "_TrajectoriesDuration: " << _TrajectoriesDuration << endl;
                    cout << "_TrajectoryCount: " << _TrajectoryCount << endl;
                    cout << "_TrajectoryInitialAzimuth: " << _TrajectoryInitialAzimuth << endl;
                    
                }
                
                if (_isSyncWTempo) {
                    
                } else {
                    
                    
                    //store current time
                    double dCurrentTime = _CurrentPlayHeadInfo.timeInSeconds;
                    
                    //if we still need to write automation (currentTime smaller than begin time + whole duration
                    if (dCurrentTime < (_TrajectoryBeginTime + _TrajectoriesDuration)){
                        
                        //calculate new position
                        float newAzimuth, newElevation;
                        float integralPart; //useless here
                        float lowerLimit = .02f;
                        switch (getSelectedTrajectoryAsInteger()) {

                            case ZirkOscjuceAudioProcessorEditor::Circle :
                                newAzimuth = modf((dCurrentTime - _TrajectoryBeginTime) / _TrajectorySingleLength, &integralPart);
                                newAzimuth = modf(_TrajectoryInitialAzimuth + newAzimuth, &integralPart);
                                setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + (_SelectedSourceForTrajectory*5), newAzimuth);
                                break;


                            case ZirkOscjuceAudioProcessorEditor::Pendulum :
                                
                                newElevation = ((dCurrentTime - _TrajectoryBeginTime) / _TrajectorySingleLength) ;
                                newElevation = modf(abs(newElevation + _TrajectoryInitialElevation), &integralPart);
                                newElevation = abs( sin(newElevation * 2 * M_PI) );
                                
                                
//                                newElevation = abs(sin ( ((dCurrentTime - _TrajectoryBeginTime) / _TrajectorySingleLength) * 2 * M_PI ));
//                                newElevation = modf(_TrajectoryInitialElevation + newElevation, &integralPart);
                                setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + (_SelectedSourceForTrajectory*5), newElevation);
                                
                                //when we get to the top of the dome, we need to get back down
                                if (!m_bTrajectoryElevationDecreasing && newElevation > .98){
                                        (_TrajectoryInitialAzimuth > 0.5) ? _TrajectoryInitialAzimuth -= .5 : _TrajectoryInitialAzimuth += .5;
                                         m_bTrajectoryElevationDecreasing = true;
                                }
                                
                                if (host.isReaper()){
                                    lowerLimit = .15;
                                } else if (host.isLogic()){                                
                                    _TrajectoryInitialAzimuth += getSmallAlternatingValue();
                                }
                                
                                if (m_bTrajectoryElevationDecreasing && newElevation < lowerLimit){
                                    m_bTrajectoryElevationDecreasing = false;
                                }
                                
                                cout << "newElevation: " << newElevation;
                                cout << "\t\t\t_TrajectoryInitialAzimuth: " << _TrajectoryInitialAzimuth;
                                cout << "\t\t\t_TrajectoryInitialElevation: " << _TrajectoryInitialElevation << endl;

                                setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + (_SelectedSourceForTrajectory*5), _TrajectoryInitialAzimuth);
                                break;
                                
                            case ZirkOscjuceAudioProcessorEditor::Spiral :
                                newAzimuth = modf((dCurrentTime - _TrajectoryBeginTime) / _TrajectorySingleLength, &integralPart);
                                newElevation = modf(_TrajectoryInitialElevation + newAzimuth, &integralPart);
                                newAzimuth = modf(_TrajectoryInitialAzimuth + newAzimuth, &integralPart);
                                setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + (_SelectedSourceForTrajectory*5), newAzimuth);
                                setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + (_SelectedSourceForTrajectory*5), newElevation);
                                break;
                                
                            default:
                                break;
                        }
                        
                    } else {
                        _TrajectoryAllDrawn = true;
                    }
                }
            }
            //if we were playing on prev frame, this is the first frame after we stopped, so need to stop the automation... not sure this will work in logic
            else if (_WasPlayingOnPrevFrame){
                
                cout << "____________________________END PARAMETER CHANGE______________________________________________\n";
                
                switch (getSelectedTrajectoryAsInteger()) {
                    case ZirkOscjuceAudioProcessorEditor::Circle :
                        endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + (_SelectedSourceForTrajectory*5));
                        break;
                    case ZirkOscjuceAudioProcessorEditor::Pendulum :
                    case ZirkOscjuceAudioProcessorEditor::Spiral :
                        endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + (_SelectedSourceForTrajectory*5));
                        endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + (_SelectedSourceForTrajectory*5));
                        break;
                        
                    default:
                        break;
                }
                
                _WasPlayingOnPrevFrame = false;
                _TrajectoryAllDrawn = false;
                iProcessBlockCounter = 0;
                
                //untoggle write trajectory button
                _isWriteTrajectory = false;
                _RefreshGui=true;
            }
        }
    }
}

float ZirkOscjuceAudioProcessor::getSmallAlternatingValue(){

    if(m_bTrajectoryAddPositive){
        m_bTrajectoryAddPositive = false;
        return 0.004;

    } else {
        m_bTrajectoryAddPositive = true;
        return - 0.004;
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

juce::AudioPlayHead::CurrentPositionInfo &ZirkOscjuceAudioProcessor::getCurrentPlayHeadInfo(){
    return _CurrentPlayHeadInfo;
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

void ZirkOscjuceAudioProcessor::setSelectedSourceForTrajectory(int iSelectedSource){
    _SelectedSourceForTrajectory = iSelectedSource;
}

int ZirkOscjuceAudioProcessor::getSelectedSourceForTrajectory(){
    return _SelectedSourceForTrajectory;
}

//==============================================================================
bool ZirkOscjuceAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ZirkOscjuceAudioProcessor::createEditor()
{
    _Editor = new ZirkOscjuceAudioProcessorEditor (this);
    return _Editor;
}


void ZirkOscjuceAudioProcessor::setLastUiWidth(int lastUiWidth)
{
    _LastUiWidth = lastUiWidth;
}

int ZirkOscjuceAudioProcessor::getLastUiWidth()
{
    return _LastUiWidth;
}
void ZirkOscjuceAudioProcessor::setLastUiHeight(int lastUiHeight)
{
    _LastUiHeight = lastUiHeight;
}

int ZirkOscjuceAudioProcessor::getLastUiHeight()
{
    return _LastUiHeight;
}


//set wheter plug is sending osc messages to zirkonium
void ZirkOscjuceAudioProcessor::setIsOscActive(bool isOscActive){
    _isOscActive = isOscActive;
}

//wheter plug is sending osc messages to zirkonium
bool ZirkOscjuceAudioProcessor::getIsOscActive(){
    return _isOscActive;
}

void ZirkOscjuceAudioProcessor::setIsSyncWTempo(bool isSyncWTempo){
    _isSyncWTempo = isSyncWTempo;
}

bool ZirkOscjuceAudioProcessor::getIsSyncWTempo(){
    return _isSyncWTempo;
}


void ZirkOscjuceAudioProcessor::setIsSpanLinked(bool isSpanLinked){
    _isSpanLinked = isSpanLinked;
}

bool ZirkOscjuceAudioProcessor::getIsSpanLinked(){
    return _isSpanLinked;
}

void ZirkOscjuceAudioProcessor::setIsWriteTrajectory(bool isWriteTrajectory){
    _isWriteTrajectory = isWriteTrajectory;
}

bool ZirkOscjuceAudioProcessor::getIsWriteTrajectory(){
    return _isWriteTrajectory;
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

// This method will be called by the host, probably on the audio thread, so
// it's absolutely time-critical. Don't use critical sections or anything
// UI-related, or anything at all that may block in any way!
float ZirkOscjuceAudioProcessor::getParameter (int index)
{
    switch (index){
        case ZirkOSC_MovementConstraint_ParamId:
            return _SelectedMovementConstraint;
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
        case ZirkOSC_TrajectoryCount_ParamId:
            return _TrajectoryCount;
        case ZirkOSC_TrajectoriesDuration_ParamId:
            return _TrajectoriesDuration;
        case ZirkOSC_SyncWTempo_ParamId:
            if (_isSyncWTempo)
                return 1.0f;
            else
                return 0.0f;
        case ZirkOSC_WriteTrajectories_ParamId:
            if (_isWriteTrajectory)
                return 1.0f;
            else
                return 0.0f;
    }
    
    for(int i = 0; i<8;++i){
        if      (ZirkOSC_Azim_ParamId + (i*5) == index)       return _AllSources[i].getAzimuth();
        else if (ZirkOSC_AzimSpan_ParamId + (i*5) == index)   return _AllSources[i].getAzimuthSpan();
        else if (ZirkOSC_Elev_ParamId + (i*5) == index)       return _AllSources[i].getElevation();
        else if (ZirkOSC_ElevSpan_ParamId + (i*5) == index)   return _AllSources[i].getElevationSpan();
        else if (ZirkOSC_Gain_ParamId + (i*5) == index)       return _AllSources[i].getGain();
    }
    cerr << endl << "wrong parameter id: " << index << "in ZirkOscjuceAudioProcessor::getParameter" << endl;
    return -1.f;
}

// This method will be called by the host, probably on the audio thread, so
// it's absolutely time-critical. Don't use critical sections or anything
// UI-related, or anything at all that may block in any way!
void ZirkOscjuceAudioProcessor::setParameter (int index, float newValue)
{
    switch (index){
        case ZirkOSC_MovementConstraint_ParamId:
            _SelectedMovementConstraint = newValue;
            return;
        case ZirkOSC_isOscActive_ParamId:
            if (newValue > .5f)
                _isOscActive = true;
            else
                _isOscActive = false;
            return;
        case ZirkOSC_isSpanLinked_ParamId:
            if (newValue > .5f)
                _isSpanLinked = true;
            else
                _isSpanLinked = false;
            return;
        case ZirkOSC_SelectedTrajectory_ParamId:
            _SelectedTrajectory = newValue;
            return;
        case ZirkOSC_TrajectoryCount_ParamId:
#warning TODO: not sure that newValue can be above 1.0f
            _TrajectoryCount = newValue;
            return;
        case ZirkOSC_TrajectoriesDuration_ParamId:
            _TrajectoriesDuration = newValue;
            return;
        case ZirkOSC_SyncWTempo_ParamId:
            if (newValue > .5f)
                _isSyncWTempo = true;
            else
                _isSyncWTempo = false;
            return;
        case ZirkOSC_WriteTrajectories_ParamId:
            if (newValue > .5f)
                _isWriteTrajectory = true;
            else
                _isWriteTrajectory = false;
            return;
    }
    
    for(int i = 0; i<8; ++i){
        if      (ZirkOSC_Azim_ParamId + (i*5) == index)       {_AllSources[i].setAzimuth(newValue); return;}
        else if (ZirkOSC_AzimSpan_ParamId + (i*5) == index)   {_AllSources[i].setAzimuthSpan(newValue); return;}
        else if (ZirkOSC_Elev_ParamId + (i*5) == index)       {_AllSources[i].setElevation(newValue); return;}
        else if (ZirkOSC_ElevSpan_ParamId + (i*5) == index)   {_AllSources[i].setElevationSpan(newValue); return;}
        else if (ZirkOSC_Gain_ParamId + (i*5) == index)       {_AllSources[i].setGain(newValue); return;}
    }
    cerr << endl << "wrong parameter id: " << index << " in ZirkOscjuceAudioProcessor::setParameter" << endl;
}

const String ZirkOscjuceAudioProcessor::getParameterName (int index)
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
        case ZirkOSC_TrajectoryCount_ParamId:
            return ZirkOSC_NbrTrajectories_name;
        case ZirkOSC_TrajectoriesDuration_ParamId:
            return ZirkOSC_DurationTrajectories_name;
        case ZirkOSC_SyncWTempo_ParamId:
            return ZirkOSC_isSyncWTempo_name;
        case ZirkOSC_WriteTrajectories_ParamId:
            return ZirkOSC_isWriteTrajectory_name;
    }
    
    
    for(int i = 0; i<8;++i){
        if      (ZirkOSC_Azim_ParamId + (i*5) == index)       return ZirkOSC_Azim_name[i];
        else if (ZirkOSC_AzimSpan_ParamId + (i*5) == index)   return ZirkOSC_AzimSpan_name[i];
        else if (ZirkOSC_Elev_ParamId + (i*5) == index)       return ZirkOSC_Elev_name[i];
        else if (ZirkOSC_ElevSpan_ParamId + (i*5) == index)   return ZirkOSC_ElevSpan_name[i];
        else if (ZirkOSC_Gain_ParamId + (i*5) == index)       return ZirkOSC_Gain_name[i];
    }
    return String::empty;
}


//==============================================================================
void ZirkOscjuceAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    XmlElement xml ("ZIRKOSCJUCESETTINGS");
    xml.setAttribute ("uiWidth", _LastUiWidth);
    xml.setAttribute ("uiHeight", _LastUiHeight);
    xml.setAttribute("PortOSC", _OscPortZirkonium);
    xml.setAttribute("IncPort", _OscPortIpadIncoming);
    xml.setAttribute("OutPort", _OscPortIpadOutgoing);
    xml.setAttribute("AddIpad", _OscAddressIpad);
    xml.setAttribute("NombreSources", _NbrSources);
    xml.setAttribute("MovementConstraint", _SelectedMovementConstraint);
    xml.setAttribute("isSpanLinked", _isSpanLinked);
    xml.setAttribute("isOscActive", _isOscActive);
    xml.setAttribute("selectedTrajectory", _SelectedTrajectory);
    xml.setAttribute("nbrTrajectory", _TrajectoryCount);
    xml.setAttribute("durationTrajectory", _TrajectoriesDuration);
    xml.setAttribute("isSyncWTempo", _isSyncWTempo);
    xml.setAttribute("isWriteTrajectory", _isWriteTrajectory);
    
    for(int i =0;i<8;++i){
        String channel = "Channel";
        String azimuth = "Azimuth";
        String elevation = "Elevation";
        String azimuthSpan = "AzimuthSpan";
        String elevationSpan = "ElevationSpan";
        String gain = "Gain";
        channel.append(String(i), 10);
        xml.setAttribute(channel, _AllSources[i].getChannel());
        azimuth.append(String(i), 10);
        xml.setAttribute(azimuth, _AllSources[i].getAzimuth());
        elevation.append(String(i), 10);
        xml.setAttribute(elevation, _AllSources[i].getElevationRawValue());
        azimuthSpan.append(String(i), 10);
        xml.setAttribute(azimuthSpan, _AllSources[i].getAzimuthSpan());
        elevationSpan.append(String(i), 10);
        xml.setAttribute(elevationSpan, _AllSources[i].getElevationSpan());
        gain.append(String(i), 10);
        xml.setAttribute(gain, _AllSources[i].getGain());
        
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
            // ok, now pull out our parameters. format is getIntAttribute("AttributeName: defaultValue);
            _LastUiWidth  = xmlState->getIntAttribute ("uiWidth", _LastUiWidth);
            _LastUiHeight = xmlState->getIntAttribute ("uiHeight", _LastUiHeight);
            _OscPortZirkonium = xmlState->getIntAttribute("PortOSC", 18032);
            _OscPortIpadIncoming = xmlState->getStringAttribute("IncPort", "10114");
            _OscPortIpadOutgoing = xmlState->getStringAttribute("OutPort", "10112");
            _OscAddressIpad = xmlState -> getStringAttribute("AddIpad", "10.0.1.3");
            _NbrSources = xmlState->getIntAttribute("NombreSources", 1);
            _SelectedMovementConstraint = static_cast<float>(xmlState->getDoubleAttribute("MovementConstraint", .2f));
            _isOscActive = xmlState->getBoolAttribute("isOscActive", true);
            _isSpanLinked = xmlState->getBoolAttribute("isSpanLinked", false);
            
            _SelectedTrajectory = static_cast<float>(xmlState->getDoubleAttribute("selectedTrajectory", 1.0f));
            _TrajectoryCount = xmlState->getIntAttribute("nbrTrajectory", 0);
            _TrajectoriesDuration = static_cast<float>(xmlState->getDoubleAttribute("durationTrajectory", 0.0f));
            _isSyncWTempo = xmlState->getBoolAttribute("isSyncWTempo", false);
            _isWriteTrajectory = xmlState->getBoolAttribute("isWriteTrajectory", false);
            
            
            for (int i=0;i<8;++i){
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
                _AllSources[i].setChannel(xmlState->getIntAttribute(channel , 0));
                _AllSources[i].setAzimuth((float) xmlState->getDoubleAttribute(azimuth,0));
                _AllSources[i].setElevation((float) xmlState->getDoubleAttribute(elevation,0));
                _AllSources[i].setAzimuthSpan((float) xmlState->getDoubleAttribute(azimuthSpan,0));
                _AllSources[i].setElevationSpan((float) xmlState->getDoubleAttribute(elevationSpan,0));
                float fGain = (float) xmlState->getDoubleAttribute(gain,1 );
                _AllSources[i].setGain(fGain);
            }
            
            changeZirkoniumOSCPort(_OscPortZirkonium);
            changeOSCReceiveIpad(_OscPortIpadIncoming.getIntValue());
            changeOSCSendIPad(_OscPortIpadOutgoing.getIntValue(), _OscAddressIpad);
            sendOSCValues();
            _RefreshGui=true;
        }
    }
}

void ZirkOscjuceAudioProcessor::sendOSCMovementType(){ //should be void with no argument if movement is included in the processor!!!!!
    //lo_send(_OscIpad, "/movementmode", "i", _SelectedMovementConstraint);
    lo_send(_OscIpad, "/movementmode", "f", _SelectedMovementConstraint);

}

void ZirkOscjuceAudioProcessor::sendOSCValues(){
    if (_isOscActive){
        for(int i=0;i<_NbrSources;++i){
            float azim_osc = PercentToHR(_AllSources[i].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max) /180.;
            float elev_osc = PercentToHR(_AllSources[i].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max)/180.;
            float azimspan_osc = PercentToHR(_AllSources[i].getAzimuthSpan(), ZirkOSC_AzimSpan_Min,ZirkOSC_AzimSpan_Max)/180.;
            float elevspan_osc = PercentToHR(_AllSources[i].getElevationSpan(), ZirkOSC_ElevSpan_Min, ZirkOSC_Elev_Max)/180.;
            int channel_osc = _AllSources[i].getChannel()-1;
            float gain_osc = _AllSources[i].getGain();
            lo_send(_OscZirkonium, "/pan/az", "ifffff", channel_osc, azim_osc, elev_osc, azimspan_osc, elevspan_osc, gain_osc);
            azim_osc = azim_osc * M_PI;
            elev_osc = elev_osc * M_PI;
            azimspan_osc = azimspan_osc * M_PI;
            elevspan_osc = elevspan_osc * M_PI;
            lo_send(_OscIpad, "/pan/az", "ifffff", channel_osc+1, azim_osc, elev_osc, azimspan_osc, elevspan_osc, gain_osc);
        }
    }
}

void ZirkOscjuceAudioProcessor::sendOSCConfig(){
    if (_isOscActive){
        lo_send(_OscIpad, "/maxsource", "iiiiiiiii", _NbrSources, _AllSources[0].getChannel(), _AllSources[1].getChannel(), _AllSources[2].getChannel(), _AllSources[3].getChannel(), _AllSources[4].getChannel(), _AllSources[5].getChannel(), _AllSources[6].getChannel(), _AllSources[7].getChannel());
    }
}

void ZirkOscjuceAudioProcessor::changeZirkoniumOSCPort(int newPort){
    
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

bool validateIpAddress(const string &ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
    return result != 0;
}

void ZirkOscjuceAudioProcessor::changeOSCSendIPad(int newPort, String newAddress){
    
    //if port is outside range, assign previous
    if(newPort<0 || newPort>100000){
        newPort = _OscPortIpadOutgoing.getIntValue();//10112;
    }
    
    //if address is invalid, assign previous
    if (!validateIpAddress(newAddress.toStdString())){
        newAddress = _OscAddressIpad;//"10.0.1.3";
    }
    
    lo_address osc = _OscIpad;
    _OscPortIpadOutgoing = String(newPort);
    _OscIpad = NULL;
    lo_address_free(osc);
	char port[32];
	snprintf(port, sizeof(port), "%d", newPort);
    regex_t regex;
    int reti;
    reti = regcomp(&regex, "\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?).){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\b", 0);
    reti = regexec(&regex, newAddress.toUTF8(), 0, NULL, 0);
    //if( !reti ){
    _OscAddressIpad = newAddress;
    //}
	_OscIpad = lo_address_new(_OscAddressIpad.toUTF8(), port);
}

void ZirkOscjuceAudioProcessor::changeOSCReceiveIpad(int newPort){
    
    if(newPort<0 || newPort>100000){
        newPort = _OscPortIpadIncoming.getIntValue();//10114;
    }
    
    if(_St){
        lo_server st2 = _St;
        lo_server_thread_stop(st2);
        lo_server_thread_free(st2);
    }
    _OscPortIpadIncoming = String(newPort);
    char port[32];
	snprintf(port, sizeof(port), "%d", newPort);
    _St = lo_server_thread_new(port, error);
    if (_St){
        lo_server_thread_add_method(_St, "/pan/az", "ifffff", receivePositionUpdate, this);
        lo_server_thread_add_method(_St, "/begintouch", "i", receiveBeginTouch, this);
        lo_server_thread_add_method(_St, "/endtouch", "i", receiveEndTouch, this);
        lo_server_thread_add_method(_St, "/moveAzimSpan", "if", receiveAzimuthSpanUpdate, this);
        lo_server_thread_add_method(_St, "/beginAzimSpanMove", "i", receiveAzimuthSpanBegin, this);
        lo_server_thread_add_method(_St, "/endAzimSpanMove", "i", receiveAzimuthSpanEnd, this);
        lo_server_thread_add_method(_St, "/moveElevSpan", "if", receiveElevationSpanUpdate, this);
        lo_server_thread_add_method(_St, "/beginElevSpanMove", "i", receiveElevationSpanBegin, this);
        lo_server_thread_add_method(_St, "/endElevSpanMove", "i", receiveElevationSpanEnd, this);
        lo_server_thread_start(_St);
    }
    
}

int receiveBeginTouch(const char *path, const char *types, lo_arg **argv, int argc,void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    ZirkOscjuceAudioProcessorEditor* editor = (ZirkOscjuceAudioProcessorEditor*) processor->getEditor();
    //printf("Receive BEGIN");
    int channel_osc = argv[0]->i;
    int i =0;
    for(i=0;i<processor->getNbrSources();++i){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    
    if (processor->getSelectedMovementConstraintAsInteger() == ZirkOscjuceAudioProcessorEditor::Independant){
        processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId+ i*5);
        processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId+ i*5);
    }
    else{
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId+ j*5);
            processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId+ j*5);
            
        }
    }
    editor->setDraggableSource(true);
    
    return 0;
}

int receiveEndTouch(const char *path, const char *types, lo_arg **argv, int argc,void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    ZirkOscjuceAudioProcessorEditor* theEditor = (ZirkOscjuceAudioProcessorEditor* ) processor->getEditor();
    int i =0;
    printf("Receive END");
    int channel_osc = argv[0]->i;
    for(i=0;i<processor->getNbrSources();++i){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    if (processor->getSelectedMovementConstraintAsInteger() != ZirkOscjuceAudioProcessorEditor::Independant){
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId+ j*5);
            processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId+ j*5);
            
        }
        theEditor->setFixedAngle(false);
    }
    else{
        if (processor->getSelectedMovementConstraintAsInteger() == ZirkOscjuceAudioProcessorEditor::Independant){
            processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId+ i*5);
            processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId+ i*5);
            printf("OUT");
        }
    }
    theEditor->setDraggableSource(false);
    return 0;
}

int receiveAzimuthSpanUpdate(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    int channel_osc = argv[0]->i;
    int i;
    float value = argv[1]->f;
    for(i=0;i<processor->getNbrSources();++i){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    if (processor->getSelectedMovementConstraintAsInteger() != ZirkOscjuceAudioProcessorEditor::Independant){
        
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + j*5,
                                                  HRToPercent(value, ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max));
            
        }
    }
    else{
        if (processor->getSelectedMovementConstraintAsInteger() == ZirkOscjuceAudioProcessorEditor::Independant){
            processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + i*5,
                                                  HRToPercent(value, ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max));
            printf("I am right there : %d, %f", channel_osc,value);
        }
    }
    return 0;
}

int ZirkOscjuceAudioProcessor::getSelectedMovementConstraintAsInteger() {
    return _SelectedMovementConstraint * (ZirkOscjuceAudioProcessorEditor::TotalNumberConstraints-1) + 1;
}

int ZirkOscjuceAudioProcessor::getSelectedTrajectoryAsInteger() {
    return _SelectedTrajectory * (ZirkOscjuceAudioProcessorEditor::TotalNumberTrajectories-1) + 1;
}


int receiveAzimuthSpanBegin(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    int channel_osc = argv[0]->i;
    int i;
    for(i=0;i<processor->getNbrSources();++i){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    if (processor->getSelectedMovementConstraintAsInteger() != ZirkOscjuceAudioProcessorEditor::Independant){
        
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + j*5);
            
        }
    }
    else{
        if (processor->getSelectedMovementConstraintAsInteger() == ZirkOscjuceAudioProcessorEditor::Independant){
            processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + i*5);
        }
    }
    return 0;
}

int receiveAzimuthSpanEnd(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    int channel_osc = argv[0]->i;
    int i;
    for(i=0;i<processor->getNbrSources();++i){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    if (processor->getSelectedMovementConstraintAsInteger() != ZirkOscjuceAudioProcessorEditor::Independant){
        
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + j*5);
            
        }
    }
    else{
        if (processor->getSelectedMovementConstraintAsInteger() == ZirkOscjuceAudioProcessorEditor::Independant){
            processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + i*5);
        }
    }
    return 0;
}

int receiveElevationSpanUpdate(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    int channel_osc = argv[0]->i;
    int i;
    float value = argv[1]->f;
    for(i=0;i<processor->getNbrSources();++i){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    if (processor->getSelectedMovementConstraintAsInteger() != ZirkOscjuceAudioProcessorEditor::Independant){
        
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + j*5,
                                                  HRToPercent(value, ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max));
            
        }
    }
    else{
        if (processor->getSelectedMovementConstraintAsInteger() == ZirkOscjuceAudioProcessorEditor::Independant){
            processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + i*5,
                                                  HRToPercent(value, ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max));
        }
    }
    return 0;

}

int receiveElevationSpanBegin(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    int channel_osc = argv[0]->i;
    int i;
    for(i=0;i<processor->getNbrSources();++i){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    if (processor->getSelectedMovementConstraintAsInteger() != ZirkOscjuceAudioProcessorEditor::Independant){
        
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->beginParameterChangeGesture (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + j*5);
        }
    }
    else{
        if (processor->getSelectedMovementConstraintAsInteger() == ZirkOscjuceAudioProcessorEditor::Independant){
            processor->beginParameterChangeGesture (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + i*5);
        }
    }
    return 0;
    

}

int receiveElevationSpanEnd(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    int channel_osc = argv[0]->i;
    int i;
    for(i=0;i<processor->getNbrSources();++i){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    if (processor->getSelectedMovementConstraintAsInteger() != ZirkOscjuceAudioProcessorEditor::Independant){
        
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->endParameterChangeGesture (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + j*5);
        }
    }
    else{
        if (processor->getSelectedMovementConstraintAsInteger() == ZirkOscjuceAudioProcessorEditor::Independant){
            processor->endParameterChangeGesture (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + i*5);
        }
    }
    return 0;
}

int receivePositionUpdate(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data){
 
    
    //channel send ifffff", channel_osc, azim_osc, elev_osc, azimspan_osc, elevspan_osc, gain_osc); all angles in RADIAN
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    int channel_osc = argv[0]->i;
    int i =0;
    for(i=0;i<processor->getNbrSources();++i){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    float azim_osc = argv[1]->f;
    float elev_osc = argv[2]->f;
    Point<float> pointRelativeCenter = Point<float>(processor->domeToScreen(Point<float>(azim_osc,elev_osc)));
    ZirkOscjuceAudioProcessorEditor* theEditor =(ZirkOscjuceAudioProcessorEditor*) (processor->getEditor());
    if(processor->getSelectedMovementConstraintAsInteger() == ZirkOscjuceAudioProcessorEditor::Independant){
        processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + i*5,
                                              HRToPercent(azim_osc, -M_PI, M_PI));
        processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + i*5,
                                              HRToPercent(elev_osc, 0.0, M_PI/2.0));
    }
    else{
        processor->setSelectedSource(i);

        if(processor->getSelectedMovementConstraintAsInteger() == ZirkOscjuceAudioProcessorEditor::Circular){
            theEditor->moveCircular(pointRelativeCenter);
            
        }
        else if(processor->getSelectedMovementConstraintAsInteger()  == ZirkOscjuceAudioProcessorEditor::DeltaLocked){
            Point<float> DeltaMove = pointRelativeCenter - processor->getSources()[processor->getSelectedSource()].getPositionXY();
            theEditor->moveSourcesWithDelta(DeltaMove);
        }
        else if(processor->getSelectedMovementConstraintAsInteger()  == ZirkOscjuceAudioProcessorEditor::FixedAngles){
            theEditor->moveFixedAngles(pointRelativeCenter);
        }
        else if(processor->getSelectedMovementConstraintAsInteger() == ZirkOscjuceAudioProcessorEditor::FixedRadius){
            theEditor->moveCircularWithFixedRadius(pointRelativeCenter);
        }
        else if(processor->getSelectedMovementConstraintAsInteger()  == ZirkOscjuceAudioProcessorEditor::FullyFixed){
            theEditor->moveFullyFixed(pointRelativeCenter); 
        }
        
    }
    const MessageManagerLock mmLock;
    theEditor->repaint();
    //processor->sendOSCValues();
    return 0;
}

Point <float> ZirkOscjuceAudioProcessor::domeToScreen (Point <float> p){
    float x,y;
    x = -s_iDomeRadius * sinf((p.getX())) * cosf((p.getY()));
    y = -s_iDomeRadius * cosf((p.getX())) * cosf((p.getY()));
    return Point <float> (x, y);
}
//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ZirkOscjuceAudioProcessor();

}

