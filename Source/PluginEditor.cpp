/*
 ==============================================================================
 ZirkOSC: VST and AU audio plug-in enabling spatial movement of sound sources in a dome of speakers.
 
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
 
 Description:
 Creation and modelisation of the interface with the juice componements, adding listener and handling event linked to the interface.
 
 Notes: 
 - all parameter preceeded by HR are Human Readable
 - parameter without HR are in percent
 - Points in spheric coordinates  : x -> azimuth, y -> elevation
 */



#ifndef TIMING_TESTS
#define TIMING_TESTS
#endif
#undef TIMING_TESTS


#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ZirkConstants.h"
#include "HIDDelegate.h"
#include "HID_Utilities_External.h"
#include "ZirkLeap.h"
#include <cstdlib>
#include <string>
#include <string.h>
#include <sstream>
#include <iomanip>
#include <istream>
#include <math.h>
#include <ctime>

using namespace std;

class MiniProgressBar : public Component
{
public:
    MiniProgressBar() : mValue(0) {}
    void paint(Graphics &g)
    {
        Rectangle<int> box = getLocalBounds();
        
        g.setColour(Colours::black);
        g.fillRect(box);
        
        g.setColour(Colour::fromRGB(0,255,0));
        box.setWidth(box.getWidth() * mValue);
        g.fillRect(box);
    }
    void setValue(float v) { mValue = v; repaint(); }
private:
    float mValue;
};

class SlidersTab : public Component{
    
    Slider* m_pGainSlider;
    Label*  m_pGainLabel;
    
    Slider* m_pAzimuthSlider;
    Label*  m_pAzimuthLabel;

    Slider* m_pElevationSlider;
    Label*  m_pElevationLabel;

    Slider* m_pAzimuthSpanSlider;
    Label*  m_pAzimuthSpanLabel;

    Slider* m_pElevationSpanSlider;
    Label*  m_pElevationSpanLabel;

    OwnedArray<Component> components;
    
    template <typename ComponentType> ComponentType* addToList (ComponentType* newComp){
        components.add (newComp);
        addAndMakeVisible (newComp);
        return newComp;
    }
    
public:
    
    Slider* getGainSlider() { return m_pGainSlider; }
    Label*  getGainLabel()  { return m_pGainLabel;  }
    
    Slider* getAzimuthSlider(){ return m_pAzimuthSlider; }
    Label*  getAzimuthLabel() { return m_pAzimuthLabel;  }
    
    Slider* getElevationSlider(){ return m_pElevationSlider; }
    Label*  getElevationLabel() { return m_pElevationLabel;  }
    
    Slider* getAzimuthSpanSlider(){ return m_pAzimuthSpanSlider; }
    Label*  getAzimuthSpanLabel() { return m_pAzimuthSpanLabel;  }
    
    Slider* getElevationSpanSlider(){ return m_pElevationSpanSlider; }
    Label*  getElevationSpanLabel() { return m_pElevationSpanLabel;  }

    
    SlidersTab(){
        m_pGainSlider           = addToList (new Slider(ZirkOSC_Gain_name[0]));
        m_pGainLabel            = addToList (new Label( ZirkOSC_Gain_name[0]));

        m_pAzimuthSlider        = addToList (new Slider(ZirkOSC_X_name[0]));
        m_pAzimuthLabel         = addToList (new Label( ZirkOSC_X_name[0]));
        
        m_pElevationSlider      = addToList (new Slider(ZirkOSC_Y_name[0]));
        m_pElevationLabel       = addToList (new Label( ZirkOSC_Y_name[0]));
        
        m_pAzimuthSpanSlider    = addToList (new Slider(ZirkOSC_AzimSpan_name[0]));
        m_pAzimuthSpanLabel     = addToList (new Label( ZirkOSC_AzimSpan_name[0]));
        
        m_pElevationSpanSlider  = addToList (new Slider(ZirkOSC_ElevSpan_name[0]));
        m_pElevationSpanLabel   = addToList (new Label( ZirkOSC_ElevSpan_name[0]));

    }
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SlidersTab)
};

class TrajectoryTab : public Component{
    
    ComboBox*   m_pTypeComboBox;
    
    ComboBox*   m_pDirectionComboBox;
    
    ComboBox*   m_pReturnComboBox;
    
    ComboBox*   m_pSyncWTempoComboBox;
    
    Label*      m_pCountLabel;
    TextEditor* m_pCountTextEditor;
    
    Label*      m_pDurationLabel;
    TextEditor* m_pDurationTextEditor;
    
    TextButton* m_pWriteButton;
    
    TextButton* m_pEndButton;
    Label*      m_pEndLabel;
    
    TextEditor* m_pEndAzimTextEditor;
    TextEditor* m_pEndElevTextEditor;
    TextButton* m_pResetEndButton;
    
    Label*      m_pTurnsLabel;
    TextEditor* m_pTurnsTextEditor;

    Label*      m_pDeviationLabel;
    TextEditor* m_pDeviationTextEditor;

    
    Label*      m_pDampeningLabel;
    TextEditor* m_pDampeningTextEditor;
    
    MiniProgressBar* mTrProgressBarTab;
    
    OwnedArray<Component> components;
    
    template <typename ComponentType> ComponentType* addToList (ComponentType* newComp){
        components.add (newComp);
        addAndMakeVisible (newComp);
        return newComp;
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectoryTab)
    
public:
    TrajectoryTab(){

        m_pTypeComboBox         = addToList (new ComboBox());
        
        m_pDirectionComboBox    = addToList (new ComboBox());
        
        m_pReturnComboBox       = addToList (new ComboBox());
        
        m_pSyncWTempoComboBox   = addToList (new ComboBox());
        
        m_pCountLabel           = addToList (new Label());
        m_pCountTextEditor      = addToList (new TextEditor());

        m_pDurationLabel        = addToList (new Label());
        m_pDurationTextEditor   = addToList (new TextEditor());
        
        m_pWriteButton          = addToList(new TextButton());
        m_pEndButton            = addToList(new TextButton());
        m_pEndLabel             = addToList(new Label());
        
        m_pEndAzimTextEditor    = addToList(new TextEditor());
        m_pEndElevTextEditor    = addToList(new TextEditor());
        m_pResetEndButton       = addToList(new TextButton());
        
        m_pTurnsLabel           = addToList (new Label());
        m_pTurnsTextEditor      = addToList (new TextEditor());

        m_pDeviationLabel           = addToList (new Label());
        m_pDeviationTextEditor      = addToList (new TextEditor());
        
        m_pDampeningLabel           = addToList (new Label());
        m_pDampeningTextEditor      = addToList (new TextEditor());
        
        
        mTrProgressBarTab       = addToList(new MiniProgressBar());
    }
    
    ComboBox*       getTypeComboBox(){      return m_pTypeComboBox;}
    ComboBox*       getDirectionComboBox(){ return m_pDirectionComboBox;}
    ComboBox*       getReturnComboBox(){    return m_pReturnComboBox;}

    ComboBox*       getSyncWTempoComboBox(){return m_pSyncWTempoComboBox;}
    
    Label*          getCountLabel(){        return m_pCountLabel;}
    TextEditor*     getCountTextEditor(){   return m_pCountTextEditor;}
    
    Label*          getDurationLabel(){     return m_pDurationLabel;}
    TextEditor*     getDurationTextEditor(){return m_pDurationTextEditor;}
    
    TextButton*     getWriteButton(){       return m_pWriteButton;}
    TextButton*     getEndButton(){         return m_pEndButton;}
    Label*          getEndLabel(){          return m_pEndLabel;}

    TextEditor*     getEndAzimTextEditor(){ return m_pEndAzimTextEditor;}
    TextEditor*     getEndElevTextEditor(){ return m_pEndElevTextEditor;}
    TextButton*     getResetEndButton(){    return m_pResetEndButton;}
    
    Label*          getTurnsLabel(){        return m_pTurnsLabel;}
    TextEditor*     getTurnsTextEditor(){   return m_pTurnsTextEditor;}

    Label*          getDeviationLabel(){        return m_pDeviationLabel;}
    TextEditor*     getDeviationTextEditor(){   return m_pDeviationTextEditor;}
    
    Label*          getDampeningLabel(){        return m_pDampeningLabel;}
    TextEditor*     getDampeningTextEditor(){   return m_pDampeningTextEditor;}
    
    MiniProgressBar* getProgressBar(){      return mTrProgressBarTab;}
    
};

class InterfaceTab : public Component{
    
    ToggleButton* m_pEnableLeap;
    ToggleButton* m_pEnableJoystick;
    Label* m_pLeapState;
    Label* m_pJoystickState;
    ComboBox* m_pLeapSourceCombo;
    OwnedArray<Component> components;
    template <typename ComponentType> ComponentType* addToList (ComponentType* newComp){
        components.add (newComp);
        addAndMakeVisible (newComp);
        return newComp;
    }
    
public:
    InterfaceTab(){
        m_pEnableLeap = addToList(new ToggleButton());
        m_pEnableJoystick = addToList(new ToggleButton());
        m_pLeapState = addToList(new Label());
        m_pJoystickState = addToList(new Label());
        m_pLeapSourceCombo = addToList(new ComboBox());
        
    }
    
    ToggleButton* getLeapButton() {return m_pEnableLeap;}
    ToggleButton* getJoystickButton() {return m_pEnableJoystick;};
    Label* getLeapState(){return m_pLeapState;};
    Label* getJoystickState(){return m_pJoystickState;};
    ComboBox* getLeapSourceComboBox(){return m_pLeapSourceCombo;};
    
};

#define STRING2(x) #x
#define STRING(x) STRING2(x)


/*!
*  \param ownerFilter : the processor processor
*/

ZirkOscAudioProcessorEditor::ZirkOscAudioProcessorEditor (ZirkOscAudioProcessor* ownerFilter)
:   AudioProcessorEditor (ownerFilter)
,_LinkSpanButton("Link span")
,_OscActiveButton("OSC active")
,_TabComponent(TabbedButtonBar::TabsAtTop)
//strings in parameters are all used as juce::component names
,m_oFirstSourceIdLabel()
,m_oZkmOscPortLabel()
,m_oNbrSourceLabel()
,m_VersionLabel()
,m_logoImage()
//,_IpadOutgoingOscPortLabel("OSCPortOutgoingIPad")
//,_IpadIncomingOscPortLabel("OSCIpadInco")
//,_IpadIpAddressLabel("ipadadressLabel")
,_FirstSourceIdTextEditor()
,_ZkmOscPortTextEditor()
,_NbrSourceTextEditor()
//,_IpadOutgoingOscPortTextEditor("OSCPortOutgoingIPadTE")
//,_IpadIncomingOscPortTextEditor("OSCIpadIncoTE")
//,_IpadIpAddressTextEditor("ipaddress")
,m_oMovementConstraintComboBox("MovementConstraint")
,m_fHueOffset(0.125)//(0.577251)
,m_oEndPointLabel()
,fStartPathX(-1)
,fStartPathY(-1)
,fEndPathX(-1)
,fEndPathY(-1)
{
    ourProcessor = getProcessor();
    
    m_oEndPointLabel.setText("Click anywhere on circle to set end point",  dontSendNotification);
    addAndMakeVisible(&m_oEndPointLabel);

    
    //---------- RIGHT SIDE LABELS ----------
    m_oNbrSourceLabel.setText("Nbr sources",  dontSendNotification);
    
    String version = STRING(JUCE_APP_VERSION);
#ifdef JUCE_DEBUG
    version += "\n";
    version += STRING(__TIME__);
#endif
    m_VersionLabel.setText("ZirkOSC" + version,  dontSendNotification);
    m_VersionLabel.setJustificationType(Justification(Justification::right));

   	m_logoImage.setImage(ImageFileFormat::loadFrom (BinaryData::logoGris_png, (size_t) BinaryData::logoGris_pngSize));
    
    _NbrSourceTextEditor.setText(String(ourProcessor->getNbrSources()));
    addAndMakeVisible(&m_oNbrSourceLabel);
    addAndMakeVisible(&_NbrSourceTextEditor);
    
    addAndMakeVisible(&m_VersionLabel);
    addAndMakeVisible(&m_logoImage);
    
    m_oFirstSourceIdLabel.setText("1st source ID",  dontSendNotification);
    _FirstSourceIdTextEditor.setText(String(ourProcessor->getSources()[0].getSourceId()));
    addAndMakeVisible(&m_oFirstSourceIdLabel);
    addAndMakeVisible(&_FirstSourceIdTextEditor);
    
    m_oZkmOscPortLabel.setText("ZKM OSC port",  dontSendNotification);
    _ZkmOscPortTextEditor.setText(String(ourProcessor->getOscPortZirkonium()));
    addAndMakeVisible(&m_oZkmOscPortLabel);
    addAndMakeVisible(&_ZkmOscPortTextEditor);
    
//    _IpadIncomingOscPortLabel.setText("Inc. port",  dontSendNotification);
//    _IpadIncomingOscPortTextEditor.setText(String(ourProcessor->getOscPortIpadIncoming()));
//    addAndMakeVisible(&_IpadIncomingOscPortLabel);
//    addAndMakeVisible(&_IpadIncomingOscPortTextEditor);
//    
//    _IpadOutgoingOscPortLabel.setText("Out. port",  dontSendNotification);
//    _IpadOutgoingOscPortTextEditor.setText(String(ourProcessor->getOscPortIpadOutgoing()));
//    addAndMakeVisible(&_IpadOutgoingOscPortLabel);
//    addAndMakeVisible(&_IpadOutgoingOscPortTextEditor);
//    
//    _IpadIpAddressLabel.setText("iPad IP add.",  dontSendNotification);
//    _IpadIpAddressTextEditor.setText(String(ourProcessor->getOscAddressIpad()));
//    addAndMakeVisible(&_IpadIpAddressLabel);
//    addAndMakeVisible(&_IpadIpAddressTextEditor);
    
//    m_bUseIpad = ourProcessor-> m_bUseIpad;
    
    //---------- TOGGLE BUTTONS ----------
    addAndMakeVisible(&_LinkSpanButton);
    _LinkSpanButton.addListener(this);
    _LinkSpanButton.setToggleState(ourProcessor->getIsSpanLinked(), dontSendNotification);
    
    addAndMakeVisible(&_OscActiveButton);
    _OscActiveButton.addListener(this);
    _OscActiveButton.setToggleState(ourProcessor->getIsOscActive(), dontSendNotification);
    
    //---------- CONSTRAINT COMBO BOX ----------
    m_oMovementConstraintComboBox.addListener(this);
    updateConstraintCombo();
    addAndMakeVisible(&m_oMovementConstraintComboBox);
    
    //---------- SETTING UP TABS ----------
    m_oSlidersTab = new SlidersTab();
    m_oTrajectoryTab = new TrajectoryTab();
    m_oInterfaceTab = new InterfaceTab();
    _TabComponent.addTab("Sliders", Colours::lightgrey, m_oSlidersTab, true);
    _TabComponent.addTab("Trajectories", Colours::lightgrey, m_oTrajectoryTab, true);
    _TabComponent.addTab("Interfaces", Colours::lightgrey, m_oInterfaceTab, true);
//    _TabComponent.addTab("Properties", Colours::lightgrey, &m_oPropertyPanel, true);
    addAndMakeVisible(_TabComponent);
    
    
    //---------- SLIDERS ----------
    m_pGainSlider = m_oSlidersTab->getGainSlider();
    m_pGainLabel  = m_oSlidersTab->getGainLabel();
    setSliderAndLabel("Gain", m_pGainSlider, m_pGainLabel, ZirkOSC_Gain_Min, ZirkOSC_Gain_Max);
    m_pGainSlider->addListener(this);
    
    m_pAzimuthSlider = m_oSlidersTab->getAzimuthSlider();
    m_pAzimuthLabel  = m_oSlidersTab->getAzimuthLabel();
    setSliderAndLabel("Azimuth", m_pAzimuthSlider, m_pAzimuthLabel, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    m_pAzimuthSlider->addListener(this);
    
    m_pElevationSlider = m_oSlidersTab->getElevationSlider();
    m_pElevationLabel  = m_oSlidersTab->getElevationLabel();
    setSliderAndLabel("Elevation", m_pElevationSlider, m_pElevationLabel, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    m_pElevationSlider->addListener(this);
    
    m_pElevationSpanSlider = m_oSlidersTab->getElevationSpanSlider();
    m_pElevationSpanLabel  = m_oSlidersTab->getElevationSpanLabel();
    setSliderAndLabel("Elev. span", m_pElevationSpanSlider, m_pElevationSpanLabel, ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max);
    m_pElevationSpanSlider->addListener(this);
    
    m_pAzimuthSpanSlider = m_oSlidersTab->getAzimuthSpanSlider();
    m_pAzimuthSpanLabel  = m_oSlidersTab->getAzimuthSpanLabel();
    setSliderAndLabel("Azim. span", m_pAzimuthSpanSlider, m_pAzimuthSpanLabel, ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);
    m_pAzimuthSpanSlider->addListener(this);
    
    updateSliders();

    //---------- TRAJECTORIES ----------
    m_pTrajectoryDirectionComboBox = m_oTrajectoryTab->getDirectionComboBox();
    m_pTrajectoryDirectionComboBox->addListener(this);
    m_pTrajectoryReturnComboBox = m_oTrajectoryTab->getReturnComboBox();
    m_pTrajectoryReturnComboBox->addListener(this);
    
    m_pTrajectoryTypeComboBox = m_oTrajectoryTab->getTypeComboBox();
    for (int index = 1; index <= Trajectory::NumberOfTrajectories(); ++index){
        m_pTrajectoryTypeComboBox->addItem(Trajectory::GetTrajectoryName(index), index);
    }
    
    m_pTrajectoryTypeComboBox->setSelectedId(ourProcessor->getSelectedTrajectory());
    m_pTrajectoryTypeComboBox->addListener(this);
    
    //TRAJECTORY DURATION EDITOR
    m_pTrajectoryDurationTextEditor = m_oTrajectoryTab->getDurationTextEditor();
    m_pTrajectoryDurationTextEditor->setText(kFirstRowSpacing + String(ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSC_TrajectoriesDuration_ParamId)));
    m_pTrajectoryDurationTextEditor->addListener(this);
    m_pTrajectoryDurationLabel = m_oTrajectoryTab->getDurationLabel();
    m_pTrajectoryDurationLabel->setText("per cycle",  dontSendNotification);
    
    //NBR TRAJECTORY TEXT EDITOR
    m_pTrajectoryCountTextEditor = m_oTrajectoryTab->getCountTextEditor();
    m_pTrajectoryCountTextEditor->setText(kFirstRowSpacing + String(ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSCm_dTrajectoryCount_ParamId)));
    m_pTrajectoryCountTextEditor->addListener(this);
    m_pTrajectoryCountLabel = m_oTrajectoryTab->getCountLabel();
    m_pTrajectoryCountLabel->setText("cycle(s)",  dontSendNotification);
    
    //TURNS
    m_pTrajectoryTurnsTextEditor = m_oTrajectoryTab->getTurnsTextEditor();
    m_pTrajectoryTurnsTextEditor->setText(String(ourProcessor->getTurns()));
    m_pTrajectoryTurnsTextEditor->addListener(this);
    m_pTrajectoryTurnsLabel = m_oTrajectoryTab->getTurnsLabel();
    m_pTrajectoryTurnsLabel->setText("turn(s)",  dontSendNotification);

    //DEVIATION
    m_pTrajectoryDeviationTextEditor = m_oTrajectoryTab->getDeviationTextEditor();
    m_pTrajectoryDeviationTextEditor->setText(String(ourProcessor->getDeviation()));
    m_pTrajectoryDeviationTextEditor->addListener(this);
    m_pTrajectoryDeviationLabel = m_oTrajectoryTab->getDeviationLabel();
    m_pTrajectoryDeviationLabel->setText("deviation",  dontSendNotification);
    
    //DAMPENING
    m_pTrajectoryDampeningTextEditor = m_oTrajectoryTab->getDampeningTextEditor();
    m_pTrajectoryDampeningTextEditor->setText(String(ourProcessor->getDampening()));
    m_pTrajectoryDampeningTextEditor->addListener(this);
    m_pTrajectoryDampeningLabel = m_oTrajectoryTab->getDampeningLabel();
    m_pTrajectoryDampeningLabel->setText("dampening",  dontSendNotification);
    
    //SYNC W TEMPO TOGGLE BUTTON
    m_pSyncWTempoComboBox = m_oTrajectoryTab->getSyncWTempoComboBox();
    m_pSyncWTempoComboBox->addItem("beat(s)",      SyncWTempo);
    m_pSyncWTempoComboBox->addItem("second(s)",    SyncWTime);
    ourProcessor->getIsSyncWTempo() ? m_pSyncWTempoComboBox->setSelectedId(SyncWTempo) : m_pSyncWTempoComboBox->setSelectedId(SyncWTime);
    m_pSyncWTempoComboBox->addListener(this);
    
    //WRITE TRAJECTORY BUTTON
    m_pWriteTrajectoryButton = m_oTrajectoryTab->getWriteButton();
    m_pWriteTrajectoryButton->setButtonText("Ready");
    m_pWriteTrajectoryButton->setClickingTogglesState(true);
    m_pWriteTrajectoryButton->setToggleState(ourProcessor->getIsWriteTrajectory(), dontSendNotification);
    m_pWriteTrajectoryButton->addListener(this);

    //END TRAJECTORY BUTTON
    m_pSetEndTrajectoryButton = m_oTrajectoryTab->getEndButton();
    m_pSetEndTrajectoryButton->setButtonText("Set end point");
    m_pSetEndTrajectoryButton->setClickingTogglesState(true);
    m_pSetEndTrajectoryButton->addListener(this);

    //END TRAJECTORY TextEditors
    float fBrightness = .6;
    m_pEndAzimTextEditor = m_oTrajectoryTab->getEndAzimTextEditor();
    m_pEndAzimTextEditor->setTextToShowWhenEmpty("  Azimuth", juce::Colour::greyLevel(fBrightness));
    m_pEndAzimTextEditor->setColour(TextEditor::textColourId, juce::Colour::greyLevel(fBrightness));
    m_pEndAzimTextEditor->setReadOnly(true);
    m_pEndAzimTextEditor->setCaretVisible(false);

    m_pEndElevTextEditor = m_oTrajectoryTab->getEndElevTextEditor();
    m_pEndElevTextEditor->setTextToShowWhenEmpty(" Elevation", juce::Colour::greyLevel(fBrightness));
    m_pEndElevTextEditor->setColour(TextEditor::textColourId, juce::Colour::greyLevel(fBrightness));
    m_pEndElevTextEditor->setReadOnly(true);
    m_pEndElevTextEditor->setCaretVisible(false);
    updateEndLocationTextEditors();
    
    //RESET END TRAJECTORY BUTTON
    m_pResetEndTrajectoryButton = m_oTrajectoryTab->getResetEndButton();
    m_pResetEndTrajectoryButton->setButtonText("Reset");
    m_pResetEndTrajectoryButton->addListener(this);
    
    //PROGRESS BAR
    mTrProgressBar = m_oTrajectoryTab->getProgressBar();
    mTrProgressBar->setVisible(false);
    
    //---------- INTERFACES ----------
    //JOYSTICK INFOS LABEL
    m_pLBJoystickState = m_oInterfaceTab->getJoystickState();
    
    //JOYSTICK TOGGLE BUTTON
    m_pTBEnableJoystick = m_oInterfaceTab->getJoystickButton();
    m_pTBEnableJoystick->setButtonText("Enable Joystick");
    m_pTBEnableJoystick->addListener(this);
    m_pTBEnableJoystick->setToggleState(false,dontSendNotification);
    
    
    //LEAP MOTION TOGGLE BUTTON
    m_pTBEnableLeap = m_oInterfaceTab->getLeapButton();
    m_pTBEnableLeap->setButtonText("Enable Leap");
    m_pTBEnableLeap->addListener(this);
    
    //LEAP MOTION INFOS LABEL
    m_pLBLeapState = m_oInterfaceTab->getLeapState();
    
    //LEAP MOTION SOURCE COMBOBOX
    m_pCBLeapSource = m_oInterfaceTab->getLeapSourceComboBox();
    
    int firstSource =_FirstSourceIdTextEditor.getText().getIntValue();
    int j=1;
    for(int i = firstSource; i<ourProcessor->getNbrSources()+firstSource; i++){
        m_pCBLeapSource->addItem((String)i, j);
        j++;
    }
    m_pCBLeapSource->setSelectedId(ourProcessor->getSelectedSource());
    m_pCBLeapSource->addListener(this);

    //---------- RESIZABLE CORNER ----------
    // add the triangular resizer component for the bottom-right of the UI
    addAndMakeVisible (_Resizer = new ResizableCornerComponent (this, &_ResizeLimits));
    //min dimensions are wallCircle radius (300) + offset in display (10,30) + padding (10)
    //_ResizeLimits.setSizeLimits (320, 340, 2*ZirkOSC_Window_Default_Width, 2*ZirkOSC_Window_Default_Height);
    _ResizeLimits.setSizeLimits (350, 350, 2*ZirkOSC_Window_Default_Width, 2*ZirkOSC_Window_Default_Height);
    //_ResizeLimits.setSizeLimits (ZirkOSC_Window_Default_Width, 600, 2*ZirkOSC_Window_Default_Width, 2*ZirkOSC_Window_Default_Height);
    
    // set our component's initial size to be the last one that was stored in the filter's settings
    //setSize (ownerFilter->getLastUiWidth(), ownerFilter->getLastUiHeight());
    //not sure that's a good idea, rather, use default size
    setSize (ZirkOSC_Window_Default_Width, ZirkOSC_Window_Default_Height);

    _FirstSourceIdTextEditor.addListener(this);
    _ZkmOscPortTextEditor.addListener(this);
    _NbrSourceTextEditor.addListener(this);
//    if (m_bUseIpad){
//        _IpadOutgoingOscPortTextEditor.addListener(this);
//        _IpadIncomingOscPortTextEditor.addListener(this);
//        _IpadIpAddressTextEditor.addListener(this);
//    }

    this->setFocusContainer(true);
    startEditorTimer(ZirkOSC_reg_timerDelay);    
}

void ZirkOscAudioProcessorEditor::updateConstraintCombo(){
    m_oMovementConstraintComboBox.clear();
    m_oMovementConstraintComboBox.addItem("Independent",        Independent);
    m_oMovementConstraintComboBox.addItem("Circular",           Circular);
    m_oMovementConstraintComboBox.addItem("Equal Elevation",    EqualElev);
    m_oMovementConstraintComboBox.addItem("Equal Azimuth",      EqualAzim);
    m_oMovementConstraintComboBox.addItem("Equal Elev+Azim",    EqualAzimElev);
    m_oMovementConstraintComboBox.addItem("Delta Lock",         DeltaLocked);
    
    int selected_id = ourProcessor->getMovementConstraint();
    if (ourProcessor->getNbrSources() == 2){
        m_oMovementConstraintComboBox.addItem("Symmetric X",    SymmetricX);
        m_oMovementConstraintComboBox.addItem("Symmetric Y",    SymmetricY);
    } else if (selected_id == SymmetricX || selected_id == SymmetricY){
        selected_id = Independent;
//        ourProcessor->setMovementConstraint(Independent);
    }
    
    m_oMovementConstraintComboBox.setSelectedId(selected_id);
}

void ZirkOscAudioProcessorEditor::startEditorTimer(int intervalInMilliseconds){
    Timer::startTimer (intervalInMilliseconds);
}

ZirkOscAudioProcessorEditor::~ZirkOscAudioProcessorEditor() {
    //stopTimer();
    if(m_pTBEnableJoystick->getToggleState())
    {
        IOHIDManagerUnscheduleFromRunLoop(gIOHIDManagerRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        IOHIDManagerRegisterInputValueCallback(gIOHIDManagerRef, NULL,this);
        IOHIDManagerClose(gIOHIDManagerRef, kIOHIDOptionsTypeNone);
        gIOHIDManagerRef = NULL;
        gDeviceCFArrayRef = NULL;
        gElementCFArrayRef = NULL;
    }
    mHIDDel = NULL;
    if(mController)
    {
        mController->enableGesture(Leap::Gesture::TYPE_INVALID);
        mController->removeListener(*mleap);
        mController=NULL;
        gIsLeapConnected = 0;
    }
}

void ZirkOscAudioProcessorEditor::updateTrajectoryComponents(){
    int iSelectedTrajectory = ourProcessor->getSelectedTrajectory();
    
    unique_ptr<vector<String>> allDirections = Trajectory::getTrajectoryPossibleDirections(iSelectedTrajectory);
    if (allDirections != nullptr){
        m_pTrajectoryDirectionComboBox->clear();
        for(auto it = allDirections->begin(); it != allDirections->end(); ++it){
            m_pTrajectoryDirectionComboBox->addItem(*it, it - allDirections->begin()+1);
        }
        m_pTrajectoryDirectionComboBox->setVisible(true);
        m_pTrajectoryDirectionComboBox->setSelectedId(PercentToIntStartsAtOne(ourProcessor->getSelectedTrajectoryDirection(), getNumSelectedTrajectoryDirections()));
    } else {
        m_pTrajectoryDirectionComboBox->setVisible(false);
    }
    
    unique_ptr<vector<String>> allReturns = Trajectory::getTrajectoryPossibleReturns(iSelectedTrajectory);
    if (allReturns != nullptr){
        m_pTrajectoryReturnComboBox->clear();
        for(auto it = allReturns->begin(); it != allReturns->end(); ++it){
            m_pTrajectoryReturnComboBox->addItem(*it, it - allReturns->begin()+1);
        }
        m_pTrajectoryReturnComboBox->setVisible(true);
        m_pTrajectoryReturnComboBox->setSelectedId(PercentToIntStartsAtOne(ourProcessor->getSelectedTrajectoryReturn(), getNumSelectedTrajectoryReturns()));
    } else {
        m_pTrajectoryReturnComboBox->setVisible(false);
    }
    
    if (iSelectedTrajectory == Pendulum || iSelectedTrajectory == Spiral){
        m_pSetEndTrajectoryButton   ->setVisible(true);
        m_pEndAzimTextEditor        ->setVisible(true);
        m_pEndElevTextEditor        ->setVisible(true);
        m_pResetEndTrajectoryButton ->setVisible(true);
    } else {
        m_pSetEndTrajectoryButton   ->setVisible(false);
        m_pEndAzimTextEditor        ->setVisible(false);
        m_pEndElevTextEditor        ->setVisible(false);
        m_pResetEndTrajectoryButton ->setVisible(false);
    }
    
    if (iSelectedTrajectory == Spiral || iSelectedTrajectory == Circle || iSelectedTrajectory == Ellipse){
        m_pTrajectoryTurnsLabel->setVisible(true);
        m_pTrajectoryTurnsTextEditor->setVisible(true);
    } else {
        m_pTrajectoryTurnsLabel->setVisible(false);
        m_pTrajectoryTurnsTextEditor->setVisible(false);
    }
    
    if (iSelectedTrajectory == Pendulum){
        setDefaultPendulumEndpoint();
        updateEndLocationTextEditors();
        
        m_pTrajectoryDampeningTextEditor->setVisible(true);
        m_pTrajectoryDampeningLabel->setVisible(true);
        m_pTrajectoryDeviationLabel->setVisible(true);
        m_pTrajectoryDeviationTextEditor->setVisible(true);
    } else {
        m_pTrajectoryDampeningTextEditor->setVisible(false);
        m_pTrajectoryDampeningLabel->setVisible(false);
        m_pTrajectoryDeviationLabel->setVisible(false);
        m_pTrajectoryDeviationTextEditor->setVisible(false);
    }
}

void ZirkOscAudioProcessorEditor::setDefaultPendulumEndpoint(){
    int iSelectedSrc = ourProcessor->getSelectedSource();
    float fCurAzim01 = ourProcessor->getSources()[iSelectedSrc].getAzimuth01();
    float fCurElev01 = ourProcessor->getSources()[iSelectedSrc].getElevation01();

    float fCurAzim = 180 + PercentToHR(fCurAzim01, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    if (fCurAzim > 180){
        fCurAzim -= 360;
    }
    fCurAzim01 = HRToPercent(fCurAzim, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    float fEndX, fEndY;
    SoundSource::azimElev01toXY(fCurAzim01, fCurElev01, fEndX, fEndY);
    ourProcessor->setEndLocationXY(make_pair(fEndX, fEndY));
}

void ZirkOscAudioProcessorEditor::resized() {
    int iCurWidth  = getWidth();
    int iCurHeight = getHeight();

    ourProcessor->setLastUiWidth(iCurWidth);
    ourProcessor->setLastUiHeight(iCurHeight);

    _Resizer->setBounds (iCurWidth - 16, iCurHeight - 16, 16, 16);
    
    //if dimensions are smaller than default size, keep components in place, so that with smallest windown, we can only see the circle
    if (iCurHeight < ZirkOSC_Window_Default_Height){
        iCurHeight = ZirkOSC_Window_Default_Height;
    }
    
    if (iCurWidth < ZirkOSC_Window_Default_Width){
        iCurWidth = ZirkOSC_Window_Default_Width;
    }
    
    //------------ LABELS ON RIGHT SIDE +version label------------
    m_logoImage.setBounds(5,5,55,55);
    m_VersionLabel.setBounds        (iCurWidth-180,5,100,30);
    setLabelAndTextEditorPosition   (iCurWidth-80, 5,   80, 25, &m_oNbrSourceLabel, &_NbrSourceTextEditor);
    setLabelAndTextEditorPosition   (iCurWidth-80, 55,  80, 25, &m_oFirstSourceIdLabel, &_FirstSourceIdTextEditor);
    setLabelAndTextEditorPosition   (iCurWidth-80, 105, 80, 25, &m_oZkmOscPortLabel, &_ZkmOscPortTextEditor);
    _OscActiveButton.setBounds      (iCurWidth-80, 155, 80, 25);
    _LinkSpanButton.setBounds       (iCurWidth-80, 180, 80, 25);
    _LinkSpanButton.toBack();
    _OscActiveButton.toBack();
    //    setLabelAndTextEditorPosition(iCurWidth-80 , 155, 80, 25, &_IpadIncomingOscPortLabel, &_IpadIncomingOscPortTextEditor);
    //    setLabelAndTextEditorPosition(iCurWidth-80 , 205, 80, 25, &_IpadOutgoingOscPortLabel, &_IpadOutgoingOscPortTextEditor);
    //    setLabelAndTextEditorPosition(iCurWidth-80 , 255, 80, 25, &_IpadIpAddressLabel, &_IpadIpAddressTextEditor);

    //------------ WALLCIRCLE ------------
    updateWallCircleSize(iCurWidth, iCurHeight);
    //------------ CONSTRAINT COMBO BOX ------------
    m_oMovementConstraintComboBox.setBounds(iCurWidth/2 - 220/2, iCurHeight - ZirkOSC_SlidersGroupHeight - ZirkOSC_ConstraintComboBoxHeight+20, 220, ZirkOSC_ConstraintComboBoxHeight);
    //------------ TABS ------------
    _TabComponent.setBounds(0, iCurHeight - ZirkOSC_SlidersGroupHeight + ZirkOSC_ConstraintComboBoxHeight, iCurWidth, ZirkOSC_SlidersGroupHeight);

    //------------ LABELS + SLIDERS TAB ------------
    setSliderAndLabelPosition(kiLM, kiTM,     iCurWidth-40, 20, m_pGainSlider,          m_pGainLabel);
    setSliderAndLabelPosition(kiLM, kiTM+30,  iCurWidth-40, 20, m_pAzimuthSlider,       m_pAzimuthLabel);
    setSliderAndLabelPosition(kiLM, kiTM+60,  iCurWidth-40, 20, m_pElevationSlider,     m_pElevationLabel);
    setSliderAndLabelPosition(kiLM, kiTM+90,  iCurWidth-40, 20, m_pAzimuthSpanSlider,   m_pAzimuthSpanLabel);
    setSliderAndLabelPosition(kiLM, kiTM+120, iCurWidth-40, 20, m_pElevationSpanSlider, m_pElevationSpanLabel);
    
    //------------ TRAJECTORIES TAB------------
    updateTrajectoryTabSize(iCurWidth, iCurHeight);
    
    //------------ INTERFACES TAB ------------
    m_pTBEnableLeap->                   setBounds(kiLM,       kiTM,     100, 25);
    m_pCBLeapSource->                   setBounds(kiLM,       kiTM+25,  100, 25);
    m_pTBEnableJoystick->               setBounds(kiLM,       kiTM+50,  100, 25);
    m_pLBLeapState->                    setBounds(kiLM+100,   kiTM,     200, 25);
    m_pLBJoystickState->                setBounds(kiLM+100,   kiTM+50,  200, 25);
}

void ZirkOscAudioProcessorEditor::updateWallCircleSize(int iCurWidth, int iCurHeight){
    //------------ SAVE SOURCE ELEVATION ---------------
    float fAllElev01[8];
    for (int iCurSrc = 0; iCurSrc < ourProcessor->getNbrSources(); ++iCurSrc){
        fAllElev01[iCurSrc] = ourProcessor->getSources()[iCurSrc].getElevation01();
    }
    //------------ WALLCIRCLE ------------
    _ZirkOSC_Center_X = (iCurWidth - 80)/2;
    _ZirkOSC_Center_Y = (iCurHeight-ZirkOSC_SlidersGroupHeight)/2;
    //assign smallest possible radius
    int iXRadius = (iCurWidth -85 -kiSrcDiameter)/2;
    int iYRadius = (iCurHeight-ZirkOSC_SlidersGroupHeight-10)/2;
    
    
    ourProcessor->setIsRecordingAutomation(true);   //not sure if this does anything, but better safe than sorry
    ZirkOscAudioProcessor::s_iDomeRadius = iXRadius <= iYRadius ? iXRadius: iYRadius;
    ourProcessor->updatePositions();
    ourProcessor->setIsRecordingAutomation(false);
    
    
    int w = 250;
    m_oEndPointLabel.setBounds(_ZirkOSC_Center_X-w/2, _ZirkOSC_Center_Y+ZirkOscAudioProcessor::s_iDomeRadius, w, 15);
    m_oEndPointLabel.setVisible(false);
    //------------ RESTORE SOURCE ELEVATION ---------------
    for (int iCurSrc = 0; iCurSrc < ourProcessor->getNbrSources(); ++iCurSrc){
        ourProcessor->getSources()[iCurSrc].setElevation01(fAllElev01[iCurSrc]);
    }
}
void ZirkOscAudioProcessorEditor::updateTrajectoryTabSize(int iCurWidth, int iCurHeight){
    int iCol1w = 90, iCol2w = 140, iCol3w = 80, iCol4w = 110, iRowH = 25;
    //row 1                                       x                                 y               width           height
    m_pTrajectoryTypeComboBox->         setBounds(kiLM,                             kiTM,           iCol1w,         iRowH);
    m_pTrajectoryDirectionComboBox->    setBounds(kiLM+iCol1w,                      kiTM,           iCol2w,         iRowH);
    m_pTrajectoryReturnComboBox->       setBounds(kiLM+iCol1w + iCol2w,             kiTM,           iCol3w,         iRowH);
    //row 1 col 3.5 and 4 are either of these things
    m_pTrajectoryDeviationTextEditor->  setBounds(kiLM+iCol1w+iCol2w + iCol3w,      kiTM,           30,             iRowH);
    m_pTrajectoryDeviationLabel->       setBounds(kiLM+iCol1w+iCol2w + iCol3w+30,   kiTM,           iCol4w-30,      iRowH);
    updateTurnsWidgetLocation();
    //row 2
    m_pTrajectoryDurationTextEditor->   setBounds(kiLM,                             kiTM+iRowH,     iCol1w,         iRowH);
    m_pSyncWTempoComboBox->             setBounds(kiLM+iCol1w,                      kiTM+iRowH,     3*iCol2w/4,     iRowH);
    m_pTrajectoryDurationLabel->        setBounds(kiLM+iCol1w + 3*iCol2w/4,         kiTM+iRowH,     iCol3w,         iRowH);
    m_pTrajectoryDampeningTextEditor->  setBounds(kiLM+iCol1w+iCol2w + iCol3w,      kiTM+iRowH,     30,             iRowH);
    m_pTrajectoryDampeningLabel->       setBounds(kiLM+iCol1w+iCol2w + iCol3w+30,   kiTM+iRowH,     iCol4w-30,      iRowH);
    //row3
    m_pTrajectoryCountTextEditor->      setBounds(kiLM,                             kiTM+2*iRowH,   iCol1w,         iRowH);
    m_pTrajectoryCountLabel->           setBounds(kiLM+iCol1w,                      kiTM+2*iRowH,   iCol2w/2,       iRowH);
    //row 4: end location
    m_pSetEndTrajectoryButton->         setBounds(kiLM,                             kiTM+3*iRowH,   iCol1w,         iRowH);
    m_pEndAzimTextEditor->              setBounds(kiLM+iCol1w,                      kiTM+3*iRowH,   iCol2w/2,       iRowH);
    m_pEndElevTextEditor->              setBounds(kiLM+iCol1w + iCol2w/2,           kiTM+3*iRowH,   iCol2w/2,       iRowH);
    m_pResetEndTrajectoryButton->       setBounds(kiLM+iCol1w + iCol2w,             kiTM+3*iRowH,   iCol3w,         iRowH);
    //row 5: write and progress bar line
    m_pWriteTrajectoryButton->          setBounds(iCurWidth-105, 125, 100, iRowH);
    mTrProgressBar->                    setBounds(iCurWidth-210, 125, 100, iRowH);
}

void ZirkOscAudioProcessorEditor::updateTurnsWidgetLocation(){
    int iCol1w = 90, iCol2w = 140, iCol3w = 80, iCol4w = 110, iRowH = 25;
    if (ourProcessor->getSelectedTrajectory() == Circle || ourProcessor->getSelectedTrajectory() == Ellipse){
        m_pTrajectoryTurnsTextEditor->  setBounds(kiLM+iCol1w+iCol2w,      kiTM,           30,             iRowH);
        m_pTrajectoryTurnsLabel->       setBounds(kiLM+iCol1w+iCol2w+30,   kiTM,           iCol4w-30,      iRowH);
    } else {
        m_pTrajectoryTurnsTextEditor->  setBounds(kiLM+iCol1w+iCol2w + iCol3w,      kiTM,           30,             iRowH);
        m_pTrajectoryTurnsLabel->       setBounds(kiLM+iCol1w+iCol2w + iCol3w+30,   kiTM,           iCol4w-30,      iRowH);
    }
}

//Automatic function to set label and Slider
/*!
* \param labelText : label's text
* \param slider : slider
* \param label : label
* \param min : minimum value of the slider
* \param max : maximum value of the slider
*/
 void ZirkOscAudioProcessorEditor::setSliderAndLabel(String labelText, Slider* slider, Label* label, float min, float max){
    slider->setTextBoxStyle(Slider::TextBoxRight, false, 80, 20);
    label->setText(labelText,  dontSendNotification);
    slider->setRange (min, max, 0.01);
}

void ZirkOscAudioProcessorEditor::setSliderAndLabelPosition(int x, int y, int width, int height, Slider* slider, Label* label){
    int iLabelWidth = 75; //70;
    label->setBounds (x,    y, iLabelWidth, height);
    slider->setBounds(x+iLabelWidth, y, width-iLabelWidth,  height);
}

void ZirkOscAudioProcessorEditor::setLabelAndTextEditorPosition(int x, int y, int width, int height, Label* p_label, TextEditor* p_textEditor){
    p_label->setBounds(x, y, width, height);
    p_textEditor->setBounds(x, y+20, width, height);
}

void ZirkOscAudioProcessorEditor::paint (Graphics& g){
    g.fillAll (Colours::lightgrey);
    paintWallCircle(g);     //this is the big, main circle in the gui
    paintCoordLabels(g);
    paintCenterDot(g);
    for (int iCurSrc = 0; iCurSrc < ourProcessor->getNbrSources(); ++iCurSrc) {
        paintSpanArc(g, iCurSrc);
    }
    //draw line and circle for selected source
    int iSelectedSrc = ourProcessor->getSelectedSource();
    float hue = (float)iSelectedSrc / 8 + m_fHueOffset;
    if (hue > 1) hue -= 1;
    g.setColour(Colour::fromHSV(hue, 1, 1, 0.8f));
    float fX, fY;
    ourProcessor->getSources()[iSelectedSrc].getXY(fX, fY);
    g.drawLine(_ZirkOSC_Center_X, _ZirkOSC_Center_Y, _ZirkOSC_Center_X + fX, _ZirkOSC_Center_Y + fY );
    float radiusZenith = sqrtf(fX*fX + fY*fY);
    g.drawEllipse(_ZirkOSC_Center_X-radiusZenith, _ZirkOSC_Center_Y-radiusZenith, radiusZenith*2, radiusZenith*2, 1.0);
    //draw sources
    paintSourcePoint(g);
    if (fStartPathX != -1 && fEndPathX != -1){
        m_oTrajectoryPath.startNewSubPath (fStartPathX, fStartPathY);
        m_oTrajectoryPath.lineTo (fEndPathX, fEndPathY);
        g.strokePath (m_oTrajectoryPath, PathStrokeType (2.0f));
    }
}

//Drawing Span Arc
void ZirkOscAudioProcessorEditor::paintSpanArc (Graphics& g, int iSrc){
    float HRElevSpan = PercentToHR(ourProcessor->getSources()[iSrc].getElevationSpan(), ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max);
    float HRAzimSpan = PercentToHR(ourProcessor->getSources()[iSrc].getAzimuthSpan(), ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);
    
    //return if there is no span arc to paint
    if (HRElevSpan == 0.f && HRAzimSpan == 0.f){
        return;
    }
    
    //get current azim+elev in angles
    float HRAzim = PercentToHR(ourProcessor->getSources()[iSrc].getAzimuth01()  , ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    float HRElev = PercentToHR(ourProcessor->getSources()[iSrc].getElevation01(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    
    //calculate max and min elevation in degrees
    Point<float> maxElev = {HRAzim, HRElev+HRElevSpan/2};
    Point<float> minElev = {HRAzim, HRElev-HRElevSpan/2};
    
    if(minElev.getY() < ZirkOSC_ElevSpan_Min){
        maxElev.setY(maxElev.getY()+ ZirkOSC_ElevSpan_Min-minElev.getY());
        minElev.setY(ZirkOSC_ElevSpan_Min);
    }
    
    //convert max min elev to xy
    Point<float> screenMaxElev = degreeToXy(maxElev);
    Point<float> screenMinElev = degreeToXy(minElev);
    
    //form minmax elev, calculate minmax radius
    float maxRadius = sqrtf(screenMaxElev.getX()*screenMaxElev.getX() + screenMaxElev.getY()*screenMaxElev.getY());
    float minRadius = sqrtf(screenMinElev.getX()*screenMinElev.getX() + screenMinElev.getY()*screenMinElev.getY());
    
    //drawing the path for spanning
    Path myPath;
    float x = screenMinElev.getX();
    float y = screenMinElev.getY();
    myPath.startNewSubPath(_ZirkOSC_Center_X+x,_ZirkOSC_Center_Y+y);
    
    //half first arc center
    myPath.addCentredArc(_ZirkOSC_Center_X, _ZirkOSC_Center_Y, minRadius, minRadius, 0.0, degreeToRadian(-HRAzim), degreeToRadian(-HRAzim + HRAzimSpan/2 ));
    
    if (maxElev.getY()> ZirkOSC_ElevSpan_Max) { // if we are over the top of the dome we draw the adjacent angle
        myPath.addCentredArc(_ZirkOSC_Center_X, _ZirkOSC_Center_Y, maxRadius, maxRadius, 0.0, M_PI+degreeToRadian(-HRAzim + HRAzimSpan/2), M_PI+degreeToRadian(-HRAzim - HRAzimSpan/2));
    } else {
        myPath.addCentredArc(_ZirkOSC_Center_X, _ZirkOSC_Center_Y, maxRadius, maxRadius, 0.0, degreeToRadian(-HRAzim+HRAzimSpan/2), degreeToRadian(-HRAzim-HRAzimSpan/2));
    }
    myPath.addCentredArc(_ZirkOSC_Center_X, _ZirkOSC_Center_Y, minRadius, minRadius, 0.0, degreeToRadian(-HRAzim-HRAzimSpan/2), degreeToRadian(-HRAzim));
    myPath.closeSubPath();
    
    float hue = (float)iSrc / 8 + m_fHueOffset;
    if (hue > 1) hue -= 1;
    g.setColour(Colour::fromHSV(hue, 1, 1, 0.1));
    g.fillPath(myPath);
    g.setColour(Colour::fromHSV(hue, 1, 1, 0.5));
    PathStrokeType strokeType = PathStrokeType(2.5);
    g.strokePath(myPath, strokeType);
}

void ZirkOscAudioProcessorEditor::paintSourcePoint (Graphics& g){
    float fX, fY;
    for (int i=0; i<ourProcessor->getNbrSources(); ++i) {
        
        ourProcessor->getSources()[i].getXY(fX, fY);
        //----------------------------------------------------------------------------------------------
        float hue = (float)i / 8 + m_fHueOffset;

        if (hue > 1) hue -= 1;
        
        //---- draw source point
        //fill source point
        g.setColour(Colour::fromHSV(hue, 1, 1, 1));
        
        float fCurR = hypotf(fX, fY);
        if ( fCurR > ZirkOscAudioProcessor::s_iDomeRadius+5){
            float fExtraRatio = ZirkOscAudioProcessor::s_iDomeRadius / fCurR;
            
            g.setColour(Colours::grey);
            
            fX *= fExtraRatio;
            fY *= fExtraRatio;
        }

        g.fillEllipse(_ZirkOSC_Center_X + fX-kiSrcRadius, _ZirkOSC_Center_Y + fY-kiSrcRadius, kiSrcDiameter, kiSrcDiameter);
        //draw outside of source point
        g.setColour(Colours::red);
        g.drawEllipse(_ZirkOSC_Center_X + fX-kiSrcRadius, _ZirkOSC_Center_Y + fY-kiSrcRadius, kiSrcDiameter, kiSrcDiameter, 1);
        
        //---- draw source label
        //draw it in black
        g.setColour(Colours::black);
        g.drawText(String(ourProcessor->getSources()[i].getSourceId()), _ZirkOSC_Center_X + fX-kiSrcRadius+1, _ZirkOSC_Center_Y + fY-kiSrcRadius+1, kiSrcDiameter, kiSrcDiameter, Justification(Justification::centred), false);
        
        //then in white, to create 3d effect
        g.setColour(Colours::white);
        g.drawText(String(ourProcessor->getSources()[i].getSourceId()), _ZirkOSC_Center_X + fX-kiSrcRadius, _ZirkOSC_Center_Y + fY-kiSrcRadius, kiSrcDiameter, kiSrcDiameter, Justification(Justification::centred), false);
    }
}

void ZirkOscAudioProcessorEditor::paintWallCircle (Graphics& g){
    uint8 grey = 80;
    g.setColour(Colour(grey, grey, grey));
    g.fillEllipse(_ZirkOSC_Center_X-ZirkOscAudioProcessor::s_iDomeRadius, _ZirkOSC_Center_Y-ZirkOscAudioProcessor::s_iDomeRadius, ZirkOscAudioProcessor::s_iDomeRadius * 2, ZirkOscAudioProcessor::s_iDomeRadius * 2);
}

void ZirkOscAudioProcessorEditor::paintCenterDot (Graphics& g){
    int iSelectedSrc = ourProcessor->getSelectedSource();
    float hue = (float)iSelectedSrc / 8 + m_fHueOffset;
    if (hue > 1) hue -= 1;
    g.setColour(Colour::fromHSV(hue, 1, 1, 0.8f));
    g.fillEllipse(_ZirkOSC_Center_X - 3.0f, _ZirkOSC_Center_Y - 3.0f, 6.0f, 6.0f );
}

void ZirkOscAudioProcessorEditor::paintCoordLabels (Graphics& g){
    g.setColour(Colours::white);
    g.drawLine(_ZirkOSC_Center_X - ZirkOscAudioProcessor::s_iDomeRadius, _ZirkOSC_Center_Y, _ZirkOSC_Center_X + ZirkOscAudioProcessor::s_iDomeRadius, _ZirkOSC_Center_Y ,0.5f);
    g.drawLine(_ZirkOSC_Center_X , _ZirkOSC_Center_Y - ZirkOscAudioProcessor::s_iDomeRadius, _ZirkOSC_Center_X , _ZirkOSC_Center_Y + ZirkOscAudioProcessor::s_iDomeRadius,0.5f);
}

/*Conversion function*/

/*!
* \param p : Point <float> (Azimuth,Elevation) in degree, xy in range [-r, r]
*/
Point <float> ZirkOscAudioProcessorEditor::degreeToXy (Point <float> p){
    float x,y;
    x = -ZirkOscAudioProcessor::s_iDomeRadius * sinf(degreeToRadian(p.getX())) * cosf(degreeToRadian(p.getY()));
    y = -ZirkOscAudioProcessor::s_iDomeRadius * cosf(degreeToRadian(p.getX())) * cosf(degreeToRadian(p.getY()));
    return Point <float> (x, y);
}

/*!
 * Function called every 50ms to refresh value from Host
 * repaint only if the user is not moving any source (m_bIsSourceBeingDragged)
 */
void ZirkOscAudioProcessorEditor::timerCallback(){
    if (mTrState ==  kTrWriting){
        Trajectory::Ptr t = ourProcessor->getTrajectory();
        if (t) {
            mTrProgressBar->setValue(t->progress());
        } else {
            m_pWriteTrajectoryButton->setButtonText("Ready");
            mTrProgressBar->setVisible(false);
            m_pWriteTrajectoryButton->setToggleState(false, dontSendNotification);
            mTrState = kTrReady;
            startEditorTimer(ZirkOSC_reg_timerDelay);
            clearTrajectoryPath();
        }
    }
    if (ourProcessor->hasToRefreshGui()){
//#if defined(TIMING_TESTS)
//        clock_t begin = clock();
//        clock_t proc = clock();
//#endif
        updateSliders();
//#if defined(TIMING_TESTS)
//        clock_t sliders = clock();
//#endif
        refreshGui();
//#if defined(TIMING_TESTS)
//        clock_t gui = clock();
//#endif
        repaint();
//#if defined(TIMING_TESTS)
//        clock_t end = clock();
//        cout << "processor:\t" << proc - begin <<"ms"<< endl;
//        cout << "sliders:\t" << sliders - proc <<"ms"<< endl;
//        cout << "ref gui:\t" << gui - sliders <<"ms"<< endl;
//        cout << "repaint:\t" << end - gui <<"ms"<< endl;
//        cout << "whole thing:\t" << end - begin <<"ms"<< endl;
//#endif
        ourProcessor->setRefreshGui(false);
    }
}

void ZirkOscAudioProcessorEditor::updateSliders(){
    int selectedSource = ourProcessor->getSelectedSource();
    //based on selected source, update all sliders
    m_pGainSlider->setValue (ourProcessor->getSources()[selectedSource].getGain01(), dontSendNotification);
    float elevation = PercentToHR(ourProcessor->getSources()[selectedSource].getElevation01(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    m_pElevationSlider->setValue(elevation,dontSendNotification);
    float azimuth = PercentToHR(ourProcessor->getSources()[selectedSource].getAzimuth01(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    m_pAzimuthSlider->setValue(azimuth,dontSendNotification);
    float azimSpan = PercentToHR(ourProcessor->getSources()[selectedSource].getAzimuthSpan(), ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);
    m_pAzimuthSpanSlider->setValue(azimSpan,dontSendNotification);
    float elevSpan = PercentToHR(ourProcessor->getSources()[selectedSource].getElevationSpan(), ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max);
    m_pElevationSpanSlider->setValue(elevSpan,dontSendNotification);
}

void ZirkOscAudioProcessorEditor::refreshGui(){
    _ZkmOscPortTextEditor.setText(String(ourProcessor->getOscPortZirkonium()));
    _NbrSourceTextEditor.setText(String(ourProcessor->getNbrSources()));
    _FirstSourceIdTextEditor.setText(String(ourProcessor->getSources()[0].getSourceId()));
    m_oMovementConstraintComboBox.setSelectedId(ourProcessor->getMovementConstraint());
    _OscActiveButton.setToggleState(ourProcessor->getIsOscActive(), dontSendNotification);
    _LinkSpanButton.setToggleState(ourProcessor->getIsSpanLinked(), dontSendNotification);

    m_pTrajectoryTypeComboBox->setSelectedId(ourProcessor->getSelectedTrajectory());
    
    int iCurTrajDirection = getNumSelectedTrajectoryDirections();
    if (iCurTrajDirection != -1){
        m_pTrajectoryDirectionComboBox->setSelectedId(PercentToIntStartsAtOne(ourProcessor->getSelectedTrajectoryDirection(), iCurTrajDirection));
    }
    int iCurTrajReturn = getNumSelectedTrajectoryReturns();
    if (iCurTrajReturn != -1){
        m_pTrajectoryReturnComboBox->setSelectedId(PercentToIntStartsAtOne(ourProcessor->getSelectedTrajectoryReturn(), iCurTrajReturn));
    }
    
    ourProcessor->getIsSyncWTempo() ? m_pSyncWTempoComboBox->setSelectedId(SyncWTempo) : m_pSyncWTempoComboBox->setSelectedId(SyncWTime);
    m_pTrajectoryCountTextEditor->setText(kFirstRowSpacing + String(ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSCm_dTrajectoryCount_ParamId)));
    m_pTrajectoryDurationTextEditor->setText(kFirstRowSpacing + String(ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSC_TrajectoriesDuration_ParamId)));
    
//    _IpadIncomingOscPortTextEditor.setText(ourProcessor->getOscPortIpadIncoming());
//    _IpadOutgoingOscPortTextEditor.setText(ourProcessor->getOscPortIpadOutgoing());
//    _IpadIpAddressTextEditor.setText(ourProcessor->getOscAddressIpad());
}

void ZirkOscAudioProcessorEditor::clearTrajectoryPath(){
    fStartPathX = -1, fEndPathX = -1, fStartPathY = -1, fEndPathY = -1;
    m_oTrajectoryPath.clear();
}

void ZirkOscAudioProcessorEditor::buttonClicked (Button* button){
    
    if(button == &_LinkSpanButton){
        ourProcessor->setIsSpanLinked(_LinkSpanButton.getToggleState());
    }
    else if(button == &_OscActiveButton){
        ourProcessor->setIsOscActive(_OscActiveButton.getToggleState());
    }
    else if(button == m_pWriteTrajectoryButton){
        
        Trajectory::Ptr t = ourProcessor->getTrajectory();
        //if there's already a trajectory, we are cancelling it
        if (t)
        {
            ourProcessor->setTrajectory(NULL);
            ourProcessor->setIsWriteTrajectory(false);
            m_pWriteTrajectoryButton->setButtonText("Ready");
            mTrProgressBar->setVisible(false);
            m_pWriteTrajectoryButton->setToggleState(false, dontSendNotification);

            mTrState = kTrReady;
            startEditorTimer(ZirkOSC_reg_timerDelay);
            clearTrajectoryPath();
            t->stop();  //this calls restoreCurrentLocations
            refreshGui();
        }
        else
        {
            ourProcessor->setIsWriteTrajectory(true);
            float duration = m_pTrajectoryDurationTextEditor->getText().getFloatValue();
            bool beats = (m_pSyncWTempoComboBox->getSelectedId() == 1);
            int type = m_pTrajectoryTypeComboBox->getSelectedId();
            
            unique_ptr<AllTrajectoryDirections> direction = Trajectory::getTrajectoryDirection(type, m_pTrajectoryDirectionComboBox->getSelectedId()-1);
            
            bool  bReturn    = (m_pTrajectoryReturnComboBox->getSelectedId() == 2);
            float repeats    = m_pTrajectoryCountTextEditor->getText().getFloatValue();
            int   source     = ourProcessor->getSelectedSource();
            float fTurns     = m_pTrajectoryTurnsTextEditor->getText().getFloatValue();
            float fDeviation = m_pTrajectoryDeviationTextEditor->getText().getFloatValue();
            float fDampening = m_pTrajectoryDampeningTextEditor->getText().getFloatValue();
            
            ourProcessor->setTrajectory(Trajectory::CreateTrajectory(type, ourProcessor, duration, beats, *direction, bReturn, repeats, source, ourProcessor->getEndLocationXY(), fTurns, fDeviation, fDampening));
            m_pWriteTrajectoryButton->setButtonText("Cancel");
            
            mTrState = kTrWriting;
            startEditorTimer(ZirkOSC_traj_timerDelay);
            
            mTrProgressBar->setValue(0);
            mTrProgressBar->setVisible(true);
        }
    }
    else if(button == m_pTBEnableLeap) {
        if (m_pTBEnableLeap->getToggleState()) {
            if (!gIsLeapConnected) {
                m_pLBLeapState->setText("Leap not connected", dontSendNotification);
                mController = new Leap::Controller();
                if(!mController) {
                    printf("Could not create leap controler");
                } else {
                    mleap = ZirkLeap::CreateLeapComponent(ourProcessor, this);
                    if(mleap) {
                        JUCE_COMPILER_WARNING("gIsLeapConnected was created because no other options of the Leap Motion permitted to know if it was already connected but it's not the good way")
                        gIsLeapConnected = 1;
                        mController->addListener(*mleap);
                    } else {
                        m_pLBLeapState->setText("Leap not connected", dontSendNotification);
                    }
                }
            } else {
                m_pLBLeapState->setText("Leap option used by another ZirkOSC", dontSendNotification);
                m_pTBEnableLeap->setToggleState(false, dontSendNotification); 
            }
        } else if(gIsLeapConnected) {
            mController->enableGesture(Leap::Gesture::TYPE_INVALID);
            mController->removeListener(*mleap);
            mController = NULL;
            gIsLeapConnected = 0;
            m_pLBLeapState->setText("", dontSendNotification);
        }
    }
    else if(button == m_pTBEnableJoystick) {
        bool bIsJoystickEnabled = m_pTBEnableJoystick->getToggleState();
        if (bIsJoystickEnabled) {
            if (!gIOHIDManagerRef) {
                m_pLBJoystickState->setText("Joystick not connected", dontSendNotification);
                gIOHIDManagerRef = IOHIDManagerCreate(CFAllocatorGetDefault(),kIOHIDOptionsTypeNone);
                if(!gIOHIDManagerRef) {
                    printf("Could not create IOHIDManager");
                } else {
                    mHIDDel = HIDDelegate::CreateHIDDelegate(ourProcessor, this);
                    mHIDDel->Initialize_HID(this);
                    if(mHIDDel->getDeviceSetRef()) {
                        m_pLBJoystickState->setText("Joystick connected", dontSendNotification);
                    } else {
                        m_pLBJoystickState->setText("Joystick not connected", dontSendNotification);
                        m_pTBEnableJoystick->setToggleState(false, dontSendNotification);
                        gIOHIDManagerRef = NULL;
                    }
                }
            } else {
                m_pTBEnableJoystick->setToggleState(false, dontSendNotification);
                m_pLBJoystickState->setText("Joystick connected to another ZirkOSC", dontSendNotification);
            }
        } else if(gIOHIDManagerRef) {
            IOHIDManagerUnscheduleFromRunLoop(gIOHIDManagerRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
            IOHIDManagerRegisterInputValueCallback(gIOHIDManagerRef, NULL,this);
            IOHIDManagerClose(gIOHIDManagerRef, kIOHIDOptionsTypeNone);
            gIOHIDManagerRef = NULL;
            gDeviceCFArrayRef = NULL;
            gElementCFArrayRef = NULL;
            mHIDDel = NULL;
            m_pLBJoystickState->setText("", dontSendNotification);
        }
    }
    else if (button == m_pSetEndTrajectoryButton){
        if (m_pSetEndTrajectoryButton->getToggleState()){
            m_pSetEndTrajectoryButton->setButtonText("Cancel");
            m_oEndPointLabel.setVisible(true);
                        
            m_pEndAzimTextEditor->setText("");
            m_pEndElevTextEditor->setText("");
        } else {
            m_pSetEndTrajectoryButton->setButtonText("Set end point");
            m_oEndPointLabel.setVisible(false);
            updateEndLocationTextEditors();
        }
    }
    else if (button == m_pResetEndTrajectoryButton){
        if (ourProcessor->getSelectedTrajectory() == Pendulum){
            setDefaultPendulumEndpoint();
        } else {
            pair<float, float> pair = make_pair(0, 0);
            ourProcessor->setEndLocationXY(pair);
        }
        updateEndLocationTextEditors();
    }
}


void ZirkOscAudioProcessorEditor::sliderDragStarted (Slider* slider) {
    
    if (ourProcessor->getIsWriteTrajectory()){
        return;
    }
    
    int selectedSource = ourProcessor->getSelectedSource();                             //get selected source
    bool isSpanLinked = ourProcessor->getIsSpanLinked();
    
    if (slider == m_pGainSlider) {
        ourProcessor->beginParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_Gain_ParamId + (selectedSource*5) );
    }
    else if (slider == m_pAzimuthSlider) {
        ourProcessor->setIsRecordingAutomation(true);
        ourProcessor->beginParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_X_ParamId + (selectedSource*5));
        ourProcessor->beginParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + (selectedSource*5));
    }
    else if (slider == m_pElevationSlider) {
        ourProcessor->setIsRecordingAutomation(true);
        ourProcessor->beginParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_X_ParamId + (selectedSource*5));
        ourProcessor->beginParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + (selectedSource*5));
    }
    else if (slider == m_pElevationSpanSlider) {
        if(isSpanLinked){
            for(int i=0 ; i<ourProcessor->getNbrSources(); ++i){
                ourProcessor->beginParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_ElevSpan_ParamId + (i*5));
            }
        } else{
            ourProcessor->beginParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_ElevSpan_ParamId + (selectedSource*5));
        }
        
    }
    else if (slider == m_pAzimuthSpanSlider) {
        if(isSpanLinked){
            for(int i=0 ; i<ourProcessor->getNbrSources(); ++i){
                ourProcessor->beginParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_AzimSpan_ParamId + (i*5));
            }
        } else{
            ourProcessor->beginParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_AzimSpan_ParamId + (selectedSource*5));
        }
    }
}

void ZirkOscAudioProcessorEditor::sliderDragEnded (Slider* slider) {
    
    if (ourProcessor->getIsWriteTrajectory()){
        return;
    }
    
    int selectedSource = ourProcessor->getSelectedSource();                             //get selected source
    bool isSpanLinked = ourProcessor->getIsSpanLinked();
    
    if (slider == m_pGainSlider) {
        ourProcessor->endParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_Gain_ParamId + (selectedSource*5) );
    }
    else if (slider == m_pAzimuthSlider) {
        ourProcessor->endParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_X_ParamId + (selectedSource*5));
        ourProcessor->endParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + (selectedSource*5));
        ourProcessor->setIsRecordingAutomation(false);
    }
    else if (slider == m_pElevationSlider) {
        ourProcessor->endParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_X_ParamId + (selectedSource*5));
        ourProcessor->endParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + (selectedSource*5));
        ourProcessor->setIsRecordingAutomation(false);
    }
    else if (slider == m_pElevationSpanSlider) {
        if(isSpanLinked){
            for(int i=0 ; i<ourProcessor->getNbrSources(); ++i){
                ourProcessor->endParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_ElevSpan_ParamId + (i*5));
            }
        } else{
            ourProcessor->endParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_ElevSpan_ParamId + (selectedSource*5));
        }
        
    }
    else if (slider == m_pAzimuthSpanSlider) {
        if(isSpanLinked){
            for(int i=0 ; i<ourProcessor->getNbrSources(); ++i){
                ourProcessor->endParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_AzimSpan_ParamId + (i*5));
            }
        } else{
            ourProcessor->endParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_AzimSpan_ParamId + (selectedSource*5));
        }
    }

}

void ZirkOscAudioProcessorEditor::sliderValueChanged (Slider* slider) {
    
    if (ourProcessor->getIsWriteTrajectory()){
        return;
    }
    
    int selectedSource = ourProcessor->getSelectedSource();
    bool isSpanLinked  = ourProcessor->getIsSpanLinked();
    float fX, fY;
    
    if (slider == m_pGainSlider) {
        ourProcessor->setParameterNotifyingHost (ZirkOscAudioProcessor::ZirkOSC_Gain_ParamId + (selectedSource*5), (float) m_pGainSlider->getValue());
    
    } else if (slider == m_pAzimuthSlider || slider == m_pElevationSlider){
        //figure out where the slider should move the point
        float newAzim01 = HRToPercent(m_pAzimuthSlider->getValue(),   ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
        float newElev01 = HRToPercent(m_pElevationSlider->getValue(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
        SoundSource::azimElev01toXY(newAzim01, newElev01, fX, fY);
        move(selectedSource, fX, fY, newAzim01, newElev01);
    } else if (slider == m_pElevationSpanSlider) {
        float fElevSpan01 = HRToPercent((float) m_pElevationSpanSlider->getValue(), ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max);
        if(isSpanLinked){
            for(int i=0 ; i<ourProcessor->getNbrSources(); ++i){
                ourProcessor->setParameterNotifyingHost (ZirkOscAudioProcessor::ZirkOSC_ElevSpan_ParamId + (i*5), fElevSpan01);
            }
        } else {
            ourProcessor->setParameterNotifyingHost (ZirkOscAudioProcessor::ZirkOSC_ElevSpan_ParamId + (selectedSource*5), fElevSpan01);
        }
    } else if (slider == m_pAzimuthSpanSlider) {
        float fAzimSpan01 = HRToPercent((float) m_pAzimuthSpanSlider->getValue(), ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);
        if(isSpanLinked){
            for(int i=0 ; i<ourProcessor->getNbrSources(); ++i){
                ourProcessor->setParameterNotifyingHost (ZirkOscAudioProcessor::ZirkOSC_AzimSpan_ParamId + (i*5), fAzimSpan01);
            }
        } else {
            ourProcessor->setParameterNotifyingHost (ZirkOscAudioProcessor::ZirkOSC_AzimSpan_ParamId + (selectedSource*5), fAzimSpan01);
        }
    }
}

void ZirkOscAudioProcessorEditor::mouseDown (const MouseEvent &event){
    if (ourProcessor->getIsWriteTrajectory()){
        return;
    }
    int source = -1;
    //if event is within the wall circle, select source that is clicked on (if any)
    if (event.x>5 && event.x <20+ZirkOscAudioProcessor::s_iDomeRadius*2 && event.y>5 && event.y< 40+ZirkOscAudioProcessor::s_iDomeRadius*2) {
        source = getSourceFromPosition(Point<float>(event.x-_ZirkOSC_Center_X, event.y-_ZirkOSC_Center_Y));
    }
    
    //if a source is clicked on, flag m_bIsSourceBeingDragged to true
    m_bIsSourceBeingDragged = (source!=-1);
    if(m_bIsSourceBeingDragged){
        //if sources are being dragged, tell host that their parameters are about to change.
        ourProcessor->setIsRecordingAutomation(true);
        ourProcessor->setSelectedSource(source);
        ourProcessor->beginParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_X_ParamId + source*5);
        ourProcessor->beginParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + source*5);
    }
    m_oMovementConstraintComboBox.grabKeyboardFocus();
}

int ZirkOscAudioProcessorEditor::getSourceFromPosition(Point<float> p ){
    for (int i = 0; i < ourProcessor->getNbrSources(); ++i){
        if (ourProcessor->getSources()[i].contains(p)){
            return i;
        }
    }
    return -1;
}


void ZirkOscAudioProcessorEditor::mouseDrag (const MouseEvent &event){
    if(m_bIsSourceBeingDragged){
        if (ourProcessor->getIsWriteTrajectory()){
            return;
        }
        //get point of current event
        float fX = event.x-_ZirkOSC_Center_X;
        float fY = event.y-_ZirkOSC_Center_Y;
        //need to clamp the point to the circle
            float fCurR = hypotf(fX, fY);
            if ( fCurR > ZirkOscAudioProcessor::s_iDomeRadius){
                float fExtraRatio = ZirkOscAudioProcessor::s_iDomeRadius / fCurR;
                fX *= fExtraRatio;
                fY *= fExtraRatio;
            }
        move(ourProcessor->getSelectedSource(), fX, fY);
//        //draw drag path
//        if (fEndPathX == -1){
//            fStartPathX = event.x;
//            fStartPathY = event.y;
//        } else {
//            fStartPathX = fEndPathX;
//            fStartPathY = fEndPathY;
//        }
//        fEndPathX = event.x;
//        fEndPathY = event.y;
//        repaint();
    }
    //grab focus
    m_oMovementConstraintComboBox.grabKeyboardFocus();
}

void ZirkOscAudioProcessorEditor::move(int p_iSource, float p_fX, float p_fY, float p_fAzim01, float p_fElev01){
    ourProcessor->move(p_iSource, p_fX, p_fY, p_fAzim01, p_fElev01);
}

void ZirkOscAudioProcessorEditor::updatePositionTrace(float p_fX, float p_fY){
    float fAbsoluteX = p_fX + _ZirkOSC_Center_X;
    float fAbsoluteY = p_fY + _ZirkOSC_Center_Y;
    //draw drag path
    if (fEndPathX == -1){
        fStartPathX = fAbsoluteX;
        fStartPathY = fAbsoluteY;
    } else {
        fStartPathX = fEndPathX;
        fStartPathY = fEndPathY;
    }
    fEndPathX = fAbsoluteX;
    fEndPathY = fAbsoluteY;
    //repaint();
}

void ZirkOscAudioProcessorEditor::mouseUp (const MouseEvent &event){
    
    if (ourProcessor->getIsWriteTrajectory()){
        return;
    }
    else if(m_bIsSourceBeingDragged){
        int selectedSource = ourProcessor->getSelectedSource();
        ourProcessor->endParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_X_ParamId+ selectedSource*5);
        ourProcessor->endParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + selectedSource*5);
        ourProcessor->setIsRecordingAutomation(false);
        m_bIsSourceBeingDragged = false;
    }
    
    //if assigning end location
    else if (m_pSetEndTrajectoryButton->getToggleState() &&  event.x>5 && event.x <20+ZirkOscAudioProcessor::s_iDomeRadius*2 && event.y>5 && event.y< 40+ZirkOscAudioProcessor::s_iDomeRadius*2) {
        //get point of current event
        float fCenteredX = event.x-_ZirkOSC_Center_X;
        float fCenteredY = event.y-_ZirkOSC_Center_Y;
        ourProcessor->setEndLocationXY(make_pair (fCenteredX, fCenteredY));
        updateEndLocationTextEditors();
        m_pSetEndTrajectoryButton->setToggleState(false, dontSendNotification);
        m_pSetEndTrajectoryButton->setButtonText("Set end point");
        m_oEndPointLabel.setVisible(false);
    }
    clearTrajectoryPath();
    
    repaint();
    m_oMovementConstraintComboBox.grabKeyboardFocus();
}

void ZirkOscAudioProcessorEditor::updateEndLocationTextEditors(){
    std::pair<float, float> endLocation = ourProcessor->getEndLocationXY();
    float fAzim = PercentToHR(SoundSource::XYtoAzim01(endLocation.first, endLocation.second), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    float fElev01 = SoundSource::XYtoElev01(endLocation.first, endLocation.second);
    float fElev = PercentToHR(fElev01, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    {
        ostringstream oss;
        oss << std::fixed << std::right << std::setw( 4 ) << setprecision(1) << std::setfill( ' ' ) << "         " <<  fAzim;
        m_pEndAzimTextEditor->setText(oss.str());
    }
    {
        ostringstream oss;
        oss << std::fixed << std::right << std::setw( 4 ) << setprecision(1) << std::setfill( ' ' ) << "         " << fElev;
        m_pEndElevTextEditor->setText(oss.str());
    }
}

void ZirkOscAudioProcessorEditor::textEditorFocusLost (TextEditor &textEditor){
    _isReturnKeyPressedCalledFromFocusLost = true;
    textEditorReturnKeyPressed(textEditor);
    _isReturnKeyPressedCalledFromFocusLost = false;
}


void ZirkOscAudioProcessorEditor::textEditorReturnKeyPressed (TextEditor &textEditor){
    
    String text = textEditor.getText();
    int intValue = textEditor.getText().getIntValue();

    if(&_NbrSourceTextEditor == &textEditor) {
        //if we have a valid number of sources, set it in processor
        if(intValue >=1 && intValue <= 8){
            ourProcessor->setNbrSources(intValue);
            
            //need to give those new sources IDs, so get first source ID
            int sourceId = ourProcessor->getSources()[0].getSourceId();
            //updating leapSource Combobox
            m_pCBLeapSource->clear();
            int firstSource =_FirstSourceIdTextEditor.getText().getIntValue();
            int iId = 1;
            for(int i = firstSource; i<intValue+firstSource; i++) {
                m_pCBLeapSource->addItem((String)i, iId++);

            }
            m_pCBLeapSource->setSelectedId(ourProcessor->getSelectedSource()+1);
            m_pCBLeapSource->addListener(this);
            //then set all subsequent source IDs to subsequent numbers
            for (int iCurSource = 1; iCurSource < 8; ++iCurSource){
                ourProcessor->getSources()[iCurSource].setSourceId(++sourceId);
            }
            
            //toggle fixed angle repositioning, if we need to
            int selectedConstraint = ourProcessor->getMovementConstraint();
            if(selectedConstraint == EqualAzim){
                ourProcessor->setEqualAzimForAllSrc();
            } else if (selectedConstraint == EqualAzimElev){
                ourProcessor->setEqualAzimElevForAllSrc();
            }
            
            updateConstraintCombo();
        }
        //otherwise just ignore new value
        else{
            _NbrSourceTextEditor.setText(String(ourProcessor->getNbrSources()));
        }
    }
    
    else if(&_FirstSourceIdTextEditor == &textEditor ){
        //we only have room for 2, positive digits, so limit that field to 3 digits
        if (intValue > 99 - 8 || intValue < 0 ){
            return;
        }
        //updating leapSource Combobox
        m_pCBLeapSource->clear();
        int nbSource =ourProcessor->getNbrSources();
        int j =1;
        for(int i = intValue; i<intValue+nbSource; i++)
        {
            m_pCBLeapSource->addItem((String)i, j);
            j++;
        }
        ourProcessor->setSelectedSource(0);
        m_pCBLeapSource->setSelectedId(1);
        m_pCBLeapSource->addListener(this);
        
        int newChannel = intValue;
    
        //set the ID of the first source to intValue, then set all subsequent source IDs to subsequent numbers
        for (int iCurSource = 0; iCurSource < 8; ++iCurSource){
            ourProcessor->getSources()[iCurSource].setSourceId(newChannel++);
        }
    }
    
    else if(&_ZkmOscPortTextEditor == &textEditor ){
        int newPort = intValue;
        ourProcessor->changeZirkoniumOSCPort(newPort);
        _ZkmOscPortTextEditor.setText(String(ourProcessor->getOscPortZirkonium()));
    }
    
    else if(m_pTrajectoryCountTextEditor == &textEditor ){
        double doubleValue = textEditor.getText().getDoubleValue();
        if (doubleValue >= 0 && doubleValue < 10000){
            ourProcessor->setParameter(ZirkOscAudioProcessor::ZirkOSCm_dTrajectoryCount_ParamId, doubleValue);

        }
        m_pTrajectoryCountTextEditor->setText(kFirstRowSpacing + String(ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSCm_dTrajectoryCount_ParamId)));
    }
    
    else if(m_pTrajectoryDurationTextEditor == &textEditor){
        double doubleValue = textEditor.getText().getDoubleValue();
        if (doubleValue >= 0 && doubleValue < 10000){
            ourProcessor->setParameter(ZirkOscAudioProcessor::ZirkOSC_TrajectoriesDuration_ParamId, doubleValue);
        }
        m_pTrajectoryDurationTextEditor->setText(kFirstRowSpacing + String(ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSC_TrajectoriesDuration_ParamId)));
    } else if (m_pTrajectoryTurnsTextEditor == &textEditor){
        updateTurnsTextEditor();
    } else if (m_pTrajectoryDeviationTextEditor == &textEditor){
        double doubleValue = textEditor.getText().getDoubleValue();
        if (doubleValue >= 0 && doubleValue <= 360){
            ourProcessor->setDeviation(doubleValue);
        }
        m_pTrajectoryDeviationTextEditor->setText(String(ourProcessor->getDeviation()));
    } else if (m_pTrajectoryDampeningTextEditor == &textEditor){
        double doubleValue = textEditor.getText().getDoubleValue();
        if (doubleValue >= 0 && doubleValue <= 1){
            ourProcessor->setDampening(doubleValue);
        }
        m_pTrajectoryDampeningTextEditor->setText(String(ourProcessor->getDampening()));
    }
//    else if (&_IpadOutgoingOscPortTextEditor == &textEditor) { }
//    else if (&_IpadIpAddressTextEditor == &textEditor) { }
//    else if (&_IpadIncomingOscPortTextEditor == &textEditor) {   }
    if (!_isReturnKeyPressedCalledFromFocusLost){
        m_oMovementConstraintComboBox.grabKeyboardFocus();
    }
}

void ZirkOscAudioProcessorEditor::updateTurnsTextEditor(){
    double doubleValue = m_pTrajectoryTurnsTextEditor->getText().getDoubleValue();
    double dUpperLimit = 10.0;
    if (ourProcessor->getSelectedTrajectory() == Circle || ourProcessor->getSelectedTrajectory() == Ellipse){
        dUpperLimit = 1.0;
    }
    if (doubleValue < 0 ){
        doubleValue = 0;
    } else if (doubleValue > dUpperLimit){
        doubleValue = dUpperLimit;
    }
    ourProcessor->setTurns(doubleValue);
    m_pTrajectoryTurnsTextEditor->setText(String(ourProcessor->getTurns()));
}

void ZirkOscAudioProcessorEditor::comboBoxChanged (ComboBox* comboBoxThatHasChanged){
    if (comboBoxThatHasChanged == &m_oMovementConstraintComboBox){
        int selectedConstraint = comboBoxThatHasChanged->getSelectedId();
        float fSelectedConstraint = IntToPercentStartsAtOne(selectedConstraint, TotalNumberConstraints);
        ourProcessor->setParameterNotifyingHost(ZirkOscAudioProcessor::ZirkOSC_MovementConstraint_ParamId, fSelectedConstraint);
        if(selectedConstraint == EqualAzim){
            ourProcessor->setEqualAzimForAllSrc();
        } else if (selectedConstraint == EqualAzimElev){
            ourProcessor->setEqualAzimElevForAllSrc();
        } else if (selectedConstraint == EqualElev){
            ourProcessor->setEqualElevForAllSrc();
        } else if (selectedConstraint == SymmetricX || selectedConstraint == SymmetricY){
            ourProcessor->setSymmetricForAllSrc();
        }
    } else if (comboBoxThatHasChanged == m_pTrajectoryTypeComboBox){
        int iSelectedTraj = comboBoxThatHasChanged->getSelectedId();
        float fSelectedTraj = IntToPercentStartsAtOne(iSelectedTraj, TotalNumberTrajectories);
        ourProcessor->setParameterNotifyingHost(ZirkOscAudioProcessor::ZirkOSC_SelectedTrajectory_ParamId, fSelectedTraj);
        updateTrajectoryComponents();
        updateTurnsTextEditor();
        updateTurnsWidgetLocation();
    } else if(comboBoxThatHasChanged == m_pSyncWTempoComboBox){
        if (comboBoxThatHasChanged->getSelectedId() == SyncWTempo){
            ourProcessor->setIsSyncWTempo(true);
        } else {
            ourProcessor->setIsSyncWTempo(false);
        }
    } else if (comboBoxThatHasChanged == m_pCBLeapSource) {
        ourProcessor->setSelectedSource(comboBoxThatHasChanged->getSelectedId()-1);
    } else if (comboBoxThatHasChanged == m_pTrajectoryDirectionComboBox){
        float fSelectedDirection = IntToPercentStartsAtOne(comboBoxThatHasChanged->getSelectedId(), getNumSelectedTrajectoryDirections());
        ourProcessor->setParameterNotifyingHost(ZirkOscAudioProcessor::ZirkOSC_SelectedTrajectoryDirection_ParamId, fSelectedDirection);
    } else if (comboBoxThatHasChanged == m_pTrajectoryReturnComboBox){
        float fSelectedReturn = IntToPercentStartsAtOne(comboBoxThatHasChanged->getSelectedId(), getNumSelectedTrajectoryReturns());
        ourProcessor->setParameterNotifyingHost(ZirkOscAudioProcessor::ZirkOSC_SelectedTrajectoryReturn_ParamId, fSelectedReturn);
    }
}
int ZirkOscAudioProcessorEditor::getNumSelectedTrajectoryDirections(){
    int iSelectedTraj = m_pTrajectoryTypeComboBox->getSelectedId();
    auto allDirections = Trajectory::getTrajectoryPossibleDirections(iSelectedTraj);
    if (allDirections != nullptr){
        return allDirections->size();
    } else {
        return -1;
    }
    
}
int ZirkOscAudioProcessorEditor::getNumSelectedTrajectoryReturns(){
    int iSelectedTraj = m_pTrajectoryTypeComboBox->getSelectedId();
    auto allReturns = Trajectory::getTrajectoryPossibleReturns(iSelectedTraj);
    if (allReturns != nullptr){
        return allReturns->size();
    } else {
        return -1;
    }
}
int ZirkOscAudioProcessorEditor::getNbSources(){
    return getProcessor()->getNbrSources();
}
void ZirkOscAudioProcessorEditor::uncheckJoystickButton(){
    m_pTBEnableJoystick->setToggleState(false, dontSendNotification);
    buttonClicked(m_pTBEnableJoystick);
}
int ZirkOscAudioProcessorEditor::getCBSelectedSource(){
    return m_pCBLeapSource->getSelectedId();
}