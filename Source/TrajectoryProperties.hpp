//
//  TrajectoryProperties.hpp
//  ZirkOSC2
//
//  Created by Nicolai on 2014-09-01.
//
//

#ifndef ZirkOSC2_TrajectoryProperties_hpp
#define ZirkOSC2_TrajectoryProperties_hpp

#include "PluginProcessor.h"
#include "PluginEditor.h"

enum AllTrajectories {
    UpwardSpiral = 1,
    DownwardSpiral,
    UpAndDownSpiral,
    DownAndUpSpiral,
    Pendulum,
    Circle,
    TotalNumberTrajectories
};

//max here represent the total range of numbers. Max defaut value = ZirkOscjuceAudioProcessorEditor::TotalNumberConstraints
static float IntToPercent2(int integer, int max){
    return static_cast<float>((integer-1)) / (max - 1);
}


class TrajectoryComboBoxComponent : public ChoicePropertyComponent{
    
public:
    
//    TrajectoryGroupComponent(const Value &valueToControl, const String &propertyName, const StringArray &choices,
//                             const Array< var > &correspondingValues, ZirkOscjuceAudioProcessor* p_pProcessor)
//    :  ChoicePropertyComponent (valueToControl, propertyName, choices, correspondingValues) {
//        ourProcessor = p_pProcessor;
//    }

    TrajectoryComboBoxComponent(const String &propertyName, ZirkOscjuceAudioProcessor* p_pProcessor)
    :  ChoicePropertyComponent (propertyName) {
        ourProcessor = p_pProcessor;
        
        //TRAJECTORY COMBO BOX
        choices.add ("Upward Spiral");
        choiceVars.add (UpwardSpiral);
        
        choices.add ("Downward Spiral");
        choiceVars.add (DownwardSpiral);
        
        choices.add ("Up and Down Spiral");
        choiceVars.add (UpAndDownSpiral);
        
        choices.add ("Down and Up Spiral");
        choiceVars.add (DownAndUpSpiral);
        
        choices.add ("Pendulum");
        choiceVars.add (Pendulum);
        
        choices.add ("Circle");
        choiceVars.add (Circle);
        
    }
    void setIndex(int newIndex) override{
        //m_iSelectedIndex = newIndex;
        ourProcessor->setParameterNotifyingHost(ZirkOscjuceAudioProcessor::ZirkOSC_SelectedTrajectory_ParamId, IntToPercent2(newIndex, TotalNumberTrajectories));
        refresh();
    }
    
    int getIndex () const override {
        //return m_iSelectedIndex;
        return ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_SelectedTrajectory_ParamId);
    }
    
    
private:
    //int m_iSelectedIndex;
    Array<var> choiceVars;
    ZirkOscjuceAudioProcessor* ourProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectoryComboBoxComponent)
};



class TrajectoryButtonComponent : public BooleanPropertyComponent
{
public:
    
    TrajectoryButtonComponent (const String &propertyName, const String &buttonTextWhenTrue,
                               const String &buttonTextWhenFalse, ZirkOscjuceAudioProcessor* p_pProcessor, bool p_bIsTempo)
    : BooleanPropertyComponent (propertyName, buttonTextWhenTrue, buttonTextWhenFalse){
        ourProcessor = p_pProcessor;
        m_bIsTempo = p_bIsTempo;
        refresh();
    }
    
    void setState (const bool newState) override
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
    
    bool getState () const override
    {
        if (m_bIsTempo){
            return ourProcessor->getIsSyncWTempo();
        } else {
            return ourProcessor->getIsWriteTrajectory();
        }
    }
    
private:
    //bool m_bState;
    bool m_bIsTempo;
    ZirkOscjuceAudioProcessor* ourProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectoryButtonComponent)
};



class TrajectoryTextComponent : public TextPropertyComponent
{
public:
    TrajectoryTextComponent (const Value &valueToControl, const String &propertyName, int maxNumChars,
                             bool isMultiLine, ZirkOscjuceAudioProcessor* p_pProcessor, bool p_bIsCount)
    : TextPropertyComponent (valueToControl, propertyName, maxNumChars, isMultiLine) {
        ourProcessor = p_pProcessor;
        // refresh();
        m_bIsCount = p_bIsCount;
    }
    
    void setText (const String &newText) override {
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
    
    String getText() const override {
        if (m_bIsCount){
            return String(ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoryCount_ParamId));
        } else {
            return String(ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoriesDuration_ParamId));
        }
    }
    
private:
    //String text;
    bool m_bIsCount;
    ZirkOscjuceAudioProcessor* ourProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectoryTextComponent)
};

#endif
