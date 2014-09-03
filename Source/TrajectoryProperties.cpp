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


TrajectoryButtonComponent::TrajectoryButtonComponent (const String &propertyName, const String &buttonTextWhenTrue,
                           const String &buttonTextWhenFalse, ZirkOscjuceAudioProcessor* p_pProcessor, bool p_bIsTempo)
: BooleanPropertyComponent (propertyName, buttonTextWhenTrue, buttonTextWhenFalse){
    ourProcessor = p_pProcessor;
    m_bIsTempo = p_bIsTempo;
    refresh();
}

void TrajectoryButtonComponent::setState (const bool newState)
{
    //m_bState = newState;
    if (m_bIsTempo){
        ourProcessor->setIsSyncWTempo(newState);
    } else {
        //set isWriteTrajectory property in processor, only if not currently playing
        if (!ourProcessor->isCurrentlyPlaying()){
            ourProcessor->setIsWriteTrajectory(newState);
        }
    }
    refresh();
}

bool TrajectoryButtonComponent::getState () const
{
    if (m_bIsTempo){
        return ourProcessor->getIsSyncWTempo();
    } else {
        return ourProcessor->getIsWriteTrajectory();
    }
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




