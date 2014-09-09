//
//  TrajectoryProperties.cpp
//  ZirkOSC2
//
//  Created by Nicolai on 2014-09-02.
//
//

#include "TrajectoryProperties.hpp"

#include "ZirkConstants.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"


TrajectoryComboBoxComponent::TrajectoryComboBoxComponent(const String &propertyName, ZirkOscjuceAudioProcessor* p_pProcessor)
:  ChoicePropertyComponent (propertyName) {
    ourProcessor = p_pProcessor;
    
    //TRAJECTORY COMBO BOX
    choices.add ("Upward Spiral");
    choices.add ("Downward Spiral");
    choices.add ("Up and Down Spiral");
    choices.add ("Down and Up Spiral");
    choices.add ("Pendulum");
    choices.add ("Circle");
    
}

void TrajectoryComboBoxComponent::setIndex(int newIndex) {
    //this index starts at 0, everything else starts at 1
    //++newIndex;
    float percent = IntToPercentStartsAtZero(newIndex, TotalNumberTrajectories);
    ourProcessor->setParameterNotifyingHost(ZirkOscjuceAudioProcessor::ZirkOSC_SelectedTrajectory_ParamId, percent);
    refresh();
}

int TrajectoryComboBoxComponent::getIndex () const {
    int value = PercentToIntStartsAtZero(ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_SelectedTrajectory_ParamId), TotalNumberTrajectories);
    return value;
}





//--------------------------------------------------------------------------------------------
TrajectoryTempoButtonComponent::TrajectoryTempoButtonComponent (const String &propertyName, const String &buttonTextWhenTrue,
                           const String &buttonTextWhenFalse, ZirkOscjuceAudioProcessor* p_pProcessor)
: BooleanPropertyComponent (propertyName, buttonTextWhenTrue, buttonTextWhenFalse){
    ourProcessor = p_pProcessor;
    refresh();
}

void TrajectoryTempoButtonComponent::setState (const bool newState)
{
    ourProcessor->setIsSyncWTempo(newState);
    refresh();
}

bool TrajectoryTempoButtonComponent::getState () const
{
    return ourProcessor->getIsSyncWTempo();
}

//--------------------------------------------------------------------------------------------
TrajectoryWriteButtonComponent::TrajectoryWriteButtonComponent (const String &propertyName, const String &buttonTextWhenTrue,
                                                      const String &buttonTextWhenFalse, ZirkOscjuceAudioProcessor* p_pProcessor)
: BooleanPropertyComponent (propertyName, buttonTextWhenTrue, buttonTextWhenFalse){
    ourProcessor = p_pProcessor;
    refresh();
}

void TrajectoryWriteButtonComponent::setState (const bool newState) {
    //set isWriteTrajectory property in processor, only if not currently playing
    if (!ourProcessor->isCurrentlyPlaying()){
        ourProcessor->setIsWriteTrajectory(newState);
    }
    refresh();
}

bool TrajectoryWriteButtonComponent::getState () const {
    return ourProcessor->getIsWriteTrajectory();
    
}


//--------------------------------------------------------------------------------------------


TrajectoryPreviewButtonComponent::TrajectoryPreviewButtonComponent (const String &propertyName, const String &buttonTextWhenTrue,
                                                      const String &buttonTextWhenFalse, ZirkOscjuceAudioProcessor* p_pProcessor)
: BooleanPropertyComponent (propertyName, buttonTextWhenTrue, buttonTextWhenFalse){
    ourProcessor = p_pProcessor;
    refresh();
}

void TrajectoryPreviewButtonComponent::setState (const bool newState){
    ourProcessor->setIsPreviewTrajectory(newState);
    refresh();
}

bool TrajectoryPreviewButtonComponent::getState () const {
    return ourProcessor->getIsPreviewTrajectory();
}


//---------------------------------------------------------------------------------------


TrajectoryTextComponent::TrajectoryTextComponent (const Value &valueToControl, const String &propertyName, int maxNumChars,
                         bool isMultiLine, ZirkOscjuceAudioProcessor* p_pProcessor, bool p_bIsCount)
: TextPropertyComponent (valueToControl, propertyName, maxNumChars, isMultiLine) {
    ourProcessor = p_pProcessor;
    // refresh();
    m_bIsCount = p_bIsCount;
}

void TrajectoryTextComponent::setText (const String &newText) {
    double doubleValue = newText.getDoubleValue();
    if (m_bIsCount){
        if ((doubleValue > 0 && doubleValue < 10000) || (doubleValue < 0 && doubleValue > -10000)){
            ourProcessor->setParameterNotifyingHost(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoryCount_ParamId, doubleValue);
        }
    } else {
        if (doubleValue > 0 && doubleValue < 10000){
            ourProcessor->setParameterNotifyingHost(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoriesDuration_ParamId, doubleValue);
        }
        
    }
    refresh();
}

String TrajectoryTextComponent::getText() const {
    if (m_bIsCount){
        return String(ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoryCount_ParamId));
    } else {
        return String(ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoriesDuration_ParamId));
    }
}




