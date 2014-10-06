/*
 ==============================================================================
 ZirkOSC2: VST and AU audio plug-in enabling spatial movement of sound sources in a dome of speakers.
 
 Copyright (C) 2014  GRIS-UdeM
 
 Developers: Vincent Berthiaume
 
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

#ifndef ZirkOSC2_TrajectoryProperties_hpp
#define ZirkOSC2_TrajectoryProperties_hpp

#include "../JuceLibraryCode/JuceHeader.h"
//#include "PluginProcessor.h"
class ZirkOscjuceAudioProcessor;

class TrajectoryComboBoxComponent : public ChoicePropertyComponent{
    
public:
    
//    TrajectoryGroupComponent(const Value &valueToControl, const String &propertyName, const StringArray &choices,
//                             const Array< var > &correspondingValues, ZirkOscjuceAudioProcessor* p_pProcessor)
//    :  ChoicePropertyComponent (valueToControl, propertyName, choices, correspondingValues) {
//        ourProcessor = p_pProcessor;
//    }

    TrajectoryComboBoxComponent(const String &propertyName, ZirkOscjuceAudioProcessor* p_pProcessor);
    
    
    void setIndex(int newIndex) override;
    
    int getIndex () const override;
    
    
private:

    ZirkOscjuceAudioProcessor* ourProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectoryComboBoxComponent)
};


//--------------------------------------------------------------------------------------------
class TrajectoryTempoButtonComponent : public BooleanPropertyComponent
{
public:
    
    TrajectoryTempoButtonComponent (const String &propertyName, const String &buttonTextWhenTrue,
                               const String &buttonTextWhenFalse, ZirkOscjuceAudioProcessor* p_pProcessor);
    
    void setState (const bool newState) override;
    
    bool getState () const override;
    
private:
    ZirkOscjuceAudioProcessor* ourProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectoryTempoButtonComponent)
};

//--------------------------------------------------------------------------------------------
class TrajectoryWriteButtonComponent : public BooleanPropertyComponent
{
public:
    
    TrajectoryWriteButtonComponent (const String &propertyName, const String &buttonTextWhenTrue,
                               const String &buttonTextWhenFalse, ZirkOscjuceAudioProcessor* p_pProcessor);
    
    void setState (const bool newState) override;
    
    bool getState () const override;
    
private:
    ZirkOscjuceAudioProcessor* ourProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectoryWriteButtonComponent)
};


//--------------------------------------------------------------------------------------------
class TrajectoryPreviewButtonComponent : public BooleanPropertyComponent
{
public:
    
    TrajectoryPreviewButtonComponent (const String &propertyName, const String &buttonTextWhenTrue,
                               const String &buttonTextWhenFalse, ZirkOscjuceAudioProcessor* p_pProcessor);
    
    void setState (const bool newState) override;
    
    bool getState () const override;
    
private:
    //bool m_bState;
    
    //whether this instance is a tempo or a write toggle button
    bool m_bIsTempo;
    ZirkOscjuceAudioProcessor* ourProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectoryPreviewButtonComponent)
};


//--------------------------------------------------------------------------------------------
class TrajectoryTextComponent : public TextPropertyComponent
{
public:
    TrajectoryTextComponent (const Value &valueToControl, const String &propertyName, int maxNumChars,
                             bool isMultiLine, ZirkOscjuceAudioProcessor* p_pProcessor, bool p_bIsCount);
    
    void setText (const String &newText) override;
    
    String getText() const override;
    
private:
    //String text;
    bool m_bIsCount;
    ZirkOscjuceAudioProcessor* ourProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectoryTextComponent)
};

#endif
