/*
 ==============================================================================
 ZirkOSC2: VST and AU audio plug-in enabling spatial movement of sound sources in a dome of speakers.
 
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





#ifndef DEBUG
#define DEBUG
#endif
#undef DEBUG

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

        m_pAzimuthSlider        = addToList (new Slider(ZirkOSC_Azim_or_x_name[0]));
        m_pAzimuthLabel         = addToList (new Label( ZirkOSC_Azim_or_x_name[0]));
        
        m_pElevationSlider      = addToList (new Slider(ZirkOSC_Elev_or_y_name[0]));
        m_pElevationLabel       = addToList (new Label( ZirkOSC_Elev_or_y_name[0]));
        
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
        
        m_pDirectionComboBox     = addToList (new ComboBox());
        
        m_pReturnComboBox       = addToList (new ComboBox());
        
        m_pSyncWTempoComboBox   = addToList (new ComboBox());
        
        m_pCountLabel           = addToList (new Label());
        m_pCountTextEditor      = addToList (new TextEditor());

        m_pDurationLabel        = addToList (new Label());
        m_pDurationTextEditor   = addToList (new TextEditor());
        
        m_pWriteButton          = addToList(new TextButton());
        
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

ZirkOscjuceAudioProcessorEditor::ZirkOscjuceAudioProcessorEditor (ZirkOscjuceAudioProcessor* ownerFilter)
:   AudioProcessorEditor (ownerFilter)
,_LinkSpanButton("Link span")
,_OscActiveButton("OSC active")
,_TabComponent(TabbedButtonBar::TabsAtTop)
//strings in parameters are all used as juce::component names
,_FirstSourceIdLabel("channelNbr")
,_ZkmOscPortLabel("OscPort")
,_NbrSourceLabel("NbrSources")
,m_VersionLabel("version")
//,_IpadOutgoingOscPortLabel("OSCPortOutgoingIPad")
//,_IpadIncomingOscPortLabel("OSCIpadInco")
//,_IpadIpAddressLabel("ipadadressLabel")
,_FirstSourceIdTextEditor("channelNbr")
,_ZkmOscPortTextEditor("OscPort")
,_NbrSourceTextEditor("NbrSource")
//,_IpadOutgoingOscPortTextEditor("OSCPortOutgoingIPadTE")
//,_IpadIncomingOscPortTextEditor("OSCIpadIncoTE")
//,_IpadIpAddressTextEditor("ipaddress")
,_MovementConstraintComboBox("MovementConstraint")
{

    ourProcessor = getProcessor();
    
    //---------- RIGHT SIDE LABELS ----------
    _NbrSourceLabel.setText("Nbr sources",  dontSendNotification);
    
    String version = STRING(JUCE_APP_VERSION);
#ifdef JUCE_DEBUG
    version += " ";
    version += STRING(__TIME__);
#endif
    m_VersionLabel.setText("ZirkOSC" + version,  dontSendNotification);
    
    
    _NbrSourceTextEditor.setText(String(ourProcessor->getNbrSources()));
    addAndMakeVisible(&_NbrSourceLabel);
    addAndMakeVisible(&_NbrSourceTextEditor);
    
    addAndMakeVisible(&m_VersionLabel);
    
    _FirstSourceIdLabel.setText("1st source ID",  dontSendNotification);
    _FirstSourceIdTextEditor.setText(String(ourProcessor->getSources()[0].getChannel()));
    addAndMakeVisible(&_FirstSourceIdLabel);
    addAndMakeVisible(&_FirstSourceIdTextEditor);
    
    _ZkmOscPortLabel.setText("ZKM OSC port",  dontSendNotification);
    _ZkmOscPortTextEditor.setText(String(ourProcessor->getOscPortZirkonium()));
    addAndMakeVisible(&_ZkmOscPortLabel);
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
    _MovementConstraintComboBox.addItem("Independant",   Independant);
    _MovementConstraintComboBox.addItem("Circular",      Circular);
    _MovementConstraintComboBox.addItem("Equal Elevation",  FixedRadius);
    _MovementConstraintComboBox.addItem("Equal Azimuth",   FixedAngles);
    _MovementConstraintComboBox.addItem("Equal Elev+Azim",   FullyFixed);
    _MovementConstraintComboBox.addItem("Delta Lock",    DeltaLocked);
    int selected_id = ourProcessor->getSelectedMovementConstraint();
    _MovementConstraintComboBox.setSelectedId(selected_id);
    _MovementConstraintComboBox.addListener(this);
    addAndMakeVisible(&_MovementConstraintComboBox);
    
    
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

    //updateTrajectoryComboboxes();
    
    //TRAJECTORY DURATION EDITOR
    m_pTrajectoryDurationTextEditor = m_oTrajectoryTab->getDurationTextEditor();
    m_pTrajectoryDurationTextEditor->setText(String(ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoriesDuration_ParamId)));
    m_pTrajectoryDurationTextEditor->addListener(this);
    m_pTrajectoryDurationLabel = m_oTrajectoryTab->getDurationLabel();
    m_pTrajectoryDurationLabel->setText("per cycle",  dontSendNotification);
    
    //NBR TRAJECTORY TEXT EDITOR
    m_pTrajectoryCountTextEditor = m_oTrajectoryTab->getCountTextEditor();
    m_pTrajectoryCountTextEditor->setText(String(ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoryCount_ParamId)));
    m_pTrajectoryCountTextEditor->addListener(this);
    m_pTrajectoryCountLabel = m_oTrajectoryTab->getCountLabel();
    m_pTrajectoryCountLabel->setText("cycle(s)",  dontSendNotification);
    
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
    for(int i = firstSource; i<ourProcessor->getNbrSources()+firstSource; i++)
    {
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

void ZirkOscjuceAudioProcessorEditor::startEditorTimer(int intervalInMilliseconds){
    Timer::startTimer (intervalInMilliseconds);
}

ZirkOscjuceAudioProcessorEditor::~ZirkOscjuceAudioProcessorEditor() {
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

void ZirkOscjuceAudioProcessorEditor::updateTrajectoryComboboxes(){
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
}

void ZirkOscjuceAudioProcessorEditor::resized() {
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
    m_VersionLabel.setBounds(5,5,180,25);
    setLabelAndTextEditorPosition(iCurWidth-80 , 5,   80, 25, &_NbrSourceLabel, &_NbrSourceTextEditor);
    setLabelAndTextEditorPosition(iCurWidth-80 , 55,  80, 25, &_FirstSourceIdLabel, &_FirstSourceIdTextEditor);
    setLabelAndTextEditorPosition(iCurWidth-80 , 105, 80, 25, &_ZkmOscPortLabel, &_ZkmOscPortTextEditor);
//    setLabelAndTextEditorPosition(iCurWidth-80 , 155, 80, 25, &_IpadIncomingOscPortLabel, &_IpadIncomingOscPortTextEditor);
//    setLabelAndTextEditorPosition(iCurWidth-80 , 205, 80, 25, &_IpadOutgoingOscPortLabel, &_IpadOutgoingOscPortTextEditor);
//    setLabelAndTextEditorPosition(iCurWidth-80 , 255, 80, 25, &_IpadIpAddressLabel, &_IpadIpAddressTextEditor);

    // OSC button
    _OscActiveButton.setBounds(iCurWidth-80, 300, 80, 25);
    
    // link button
    _LinkSpanButton.setBounds(iCurWidth-80, 325, 80, 25);

    
    //------------ WALLCIRCLE ------------
    _ZirkOSC_Center_X = (iCurWidth -80)/2;
    _ZirkOSC_Center_Y = (iCurHeight-ZirkOSC_SlidersGroupHeight)/2;
    //assign smallest possible radius
    int iXRadius = (iCurWidth -85)/2;
    int iYRadius = (iCurHeight-ZirkOSC_SlidersGroupHeight-10)/2;
    ZirkOscjuceAudioProcessor::s_iDomeRadius = iXRadius <= iYRadius ? iXRadius: iYRadius;
    
    
    //------------ CONSTRAINT COMBO BOX ------------
    _MovementConstraintComboBox.setBounds(iCurWidth/2 - 220/2, iCurHeight - ZirkOSC_SlidersGroupHeight - ZirkOSC_ConstraintComboBoxHeight+20, 220, ZirkOSC_ConstraintComboBoxHeight);
    
    
    //------------ TABS ------------
    _TabComponent.setBounds(0, iCurHeight - ZirkOSC_SlidersGroupHeight + ZirkOSC_ConstraintComboBoxHeight, iCurWidth, ZirkOSC_SlidersGroupHeight);

    //------------ LABELS AND SLIDERS ------------
    setSliderAndLabelPosition(15, 15,     iCurWidth-40, 20, m_pGainSlider,          m_pGainLabel);
    setSliderAndLabelPosition(15, 15+30,  iCurWidth-40, 20, m_pAzimuthSlider,       m_pAzimuthLabel);
    setSliderAndLabelPosition(15, 15+60,  iCurWidth-40, 20, m_pElevationSlider,     m_pElevationLabel);
    setSliderAndLabelPosition(15, 15+90,  iCurWidth-40, 20, m_pAzimuthSpanSlider,   m_pAzimuthSpanLabel);
    setSliderAndLabelPosition(15, 15+120, iCurWidth-40, 20, m_pElevationSpanSlider, m_pElevationSpanLabel);
    
    //------------ TRAJECTORIES ------------
    m_pTrajectoryTypeComboBox->         setBounds(15,           15,    100, 25);
    m_pTrajectoryDirectionComboBox->    setBounds(15+100,       15,    130,  25);
    m_pTrajectoryReturnComboBox->       setBounds(15+230,       15,    75,  25);
    
    m_pTrajectoryDurationTextEditor->   setBounds(15,           15+25, 230, 25);
    m_pSyncWTempoComboBox->             setBounds(15+230,       15+25, 100, 25);
    m_pTrajectoryDurationLabel->        setBounds(15+230+100,   15+25, 65,  25);
  
    m_pTrajectoryCountTextEditor->      setBounds(15,           15+50, 230, 25);
    m_pTrajectoryCountLabel->           setBounds(15+230,       15+50, 75,  25);

    m_pWriteTrajectoryButton->          setBounds(iCurWidth-105, 125, 100, 25);
    mTrProgressBar->                    setBounds(iCurWidth-210, 125, 100, 25);
    
    //------------ INTERFACES ------------
    m_pTBEnableLeap                     ->setBounds(15, 15, 100, 25);
    m_pCBLeapSource                     ->setBounds(15, 15+25, 100, 25);
    m_pTBEnableJoystick                     ->setBounds(15, 15+50, 100, 25);
    
    m_pLBLeapState->setBounds(15+100, 15, 200, 25);
    m_pLBJoystickState->setBounds(15+100, 15+50, 200,25 );

}

//Automatic function to set label and Slider

/*!
* \param labelText : label's text
* \param slider : slider
* \param label : label
* \param min : minimum value of the slider
* \param max : maximum value of the slider
*/
 void ZirkOscjuceAudioProcessorEditor::setSliderAndLabel(String labelText, Slider* slider, Label* label, float min, float max){
    slider->setTextBoxStyle(Slider::TextBoxRight, false, 80, 20);
    label->setText(labelText,  dontSendNotification);
    slider->setRange (min, max, 0.01);
}

void ZirkOscjuceAudioProcessorEditor::setSliderAndLabelPosition(int x, int y, int width, int height, Slider* slider, Label* label){
    int iLabelWidth = 75; //70;
    label->setBounds (x,    y, iLabelWidth, height);
    slider->setBounds(x+iLabelWidth, y, width-iLabelWidth,  height);
}

void ZirkOscjuceAudioProcessorEditor::setLabelAndTextEditorPosition(int x, int y, int width, int height, Label* p_label, TextEditor* p_textEditor){
    p_label->setBounds(x, y, width, height);
    p_textEditor->setBounds(x, y+20, width, height);
}

void ZirkOscjuceAudioProcessorEditor::paint (Graphics& g){
    g.fillAll (Colours::lightgrey);
    paintWallCircle(g);     //this is the big, main circle in the gui
    paintCrosshairs(g);
    paintCoordLabels(g);
    paintCenterDot(g);
    paintSpanArc(g);
    paintSourcePoint(g);
    paintAzimuthLine(g);
    paintZenithCircle(g);
}


//Drawing Span Arc
void ZirkOscjuceAudioProcessorEditor::paintSpanArc (Graphics& g){
    
    int selectedSource = ourProcessor->getSelectedSource();
    
    float HRElevSpan = PercentToHR(ourProcessor->getSources()[selectedSource].getElevationSpan(), ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max);
    float HRAzimSpan = PercentToHR(ourProcessor->getSources()[selectedSource].getAzimuthSpan(), ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);

    //return if there is no span arc to paint
    if (HRElevSpan == 0.f && HRAzimSpan == 0.f){
        return;
    }
    
    //get current azim+elev in angles
    float HRAzim = PercentToHR(ourProcessor->getSources()[selectedSource].getAzimuth()  , ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    float HRElev = PercentToHR(ourProcessor->getSources()[selectedSource].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    
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
    
    g.setColour(Colours::lightgrey);
    g.fillPath(myPath);
    
    g.setColour(Colours::black);
    PathStrokeType strokeType = PathStrokeType(1.0);
    g.strokePath(myPath, strokeType);
}

//void ZirkOscjuceAudioProcessorEditor::paintSourcePoint (Graphics& g){
//    Point<float> sourcePositionOnScreen;
//    float HRAzim, HRElev;
//    int iXOffset = 0, iYOffset = 0;
//    for (int i=0; i<ourProcessor->getNbrSources(); ++i) {
//        
//        JUCE_COMPILER_WARNING("remove all azim elev in here")
//        HRAzim = PercentToHR(ourProcessor->getSources()[i].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
//        HRElev = PercentToHR(ourProcessor->getSources()[i].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
//        sourcePositionOnScreen = degreeToXy(Point<float> (HRAzim, HRElev));
//        
//        if (i == 0 ) cout << sourcePositionOnScreen.getX() << ", " << sourcePositionOnScreen.getY() << "\n";
//        
//        //draw source circle
//        g.drawEllipse(_ZirkOSC_Center_X + sourcePositionOnScreen.getX()-4, _ZirkOSC_Center_Y + sourcePositionOnScreen.getY()-4, 8, 8,2);
//        
//        if (sourcePositionOnScreen.getX() > ZirkOscjuceAudioProcessor::s_iDomeRadius - 25 ){
//            iXOffset = -20;
//            iYOffset = 10;
//        } else {
//            iXOffset = 4;
//            iYOffset = -2;
//        }
//        
//        //draw source labels
//        if(!_isSourceBeingDragged){
//             g.drawText(String(ourProcessor->getSources()[i].getChannel()), _ZirkOSC_Center_X + sourcePositionOnScreen.getX()+iXOffset, _ZirkOSC_Center_Y + sourcePositionOnScreen.getY()+iYOffset, 25, 10, Justification::centred, false);
//        }
//    }
//}

void ZirkOscjuceAudioProcessorEditor::paintSourcePoint (Graphics& g){
    float fX, fY;
    int iXOffset = 0, iYOffset = 0;
    g.setColour(Colours::black);
    
    for (int i=0; i<ourProcessor->getNbrSources(); ++i) {
        
        ourProcessor->getSources()[i].getXY(fX, fY);
        
        JUCE_COMPILER_WARNING("HEXA not clear to me why we need to clamp again, while in the previous version of this function we didn't. Also not clear that this new function is more performant")
        float fCurR = hypotf(fX, fY);
        if ( fCurR > ZirkOscjuceAudioProcessor::s_iDomeRadius){
            float fExtraRatio = ZirkOscjuceAudioProcessor::s_iDomeRadius / fCurR;
            
            fX *= fExtraRatio;
            fY *= fExtraRatio;
        }
        
        //draw source circle
        g.drawEllipse(_ZirkOSC_Center_X + fX-4, _ZirkOSC_Center_Y + fY-4, 8, 8,2);
        
        if (fX > ZirkOscjuceAudioProcessor::s_iDomeRadius - 25 ){
            iXOffset = -20;
            iYOffset = 10;
        } else {
            iXOffset = 4;
            iYOffset = -2;
        }
        
        //draw source labels
        if(!_isSourceBeingDragged){
            g.drawText(String(ourProcessor->getSources()[i].getChannel()), _ZirkOSC_Center_X + fX+iXOffset, _ZirkOSC_Center_Y + fY+iYOffset, 25, 10, Justification::centred, false);
        }
    }
}

void ZirkOscjuceAudioProcessorEditor::paintWallCircle (Graphics& g){
    g.setColour(Colours::white);
    g.fillEllipse(_ZirkOSC_Center_X-ZirkOscjuceAudioProcessor::s_iDomeRadius, _ZirkOSC_Center_Y-ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius * 2, ZirkOscjuceAudioProcessor::s_iDomeRadius * 2);
    g.setColour(Colours::black);
    g.drawEllipse(_ZirkOSC_Center_X-ZirkOscjuceAudioProcessor::s_iDomeRadius, _ZirkOSC_Center_Y-ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius * 2, ZirkOscjuceAudioProcessor::s_iDomeRadius * 2, 1.0f);
}

void ZirkOscjuceAudioProcessorEditor::paintCenterDot (Graphics& g){
    g.setColour(Colours::red);
    g.fillEllipse(_ZirkOSC_Center_X - 3.0f, _ZirkOSC_Center_Y - 3.0f, 6.0f,6.0f );
}

void ZirkOscjuceAudioProcessorEditor::paintAzimuthLine (Graphics& g){
    int selectedSource = ourProcessor->getSelectedSource();
    g.setColour(Colours::red);
    float fX, fY;
    ourProcessor->getSources()[selectedSource].getXY(fX, fY);
    g.drawLine(_ZirkOSC_Center_X, _ZirkOSC_Center_Y, _ZirkOSC_Center_X + fX, _ZirkOSC_Center_Y + fY );
}

void ZirkOscjuceAudioProcessorEditor::paintZenithCircle (Graphics& g){
    int selectedSource = ourProcessor->getSelectedSource();
    float fX, fY;
    ourProcessor->getSources()[selectedSource].getXY(fX, fY);
    float radiusZenith = sqrtf(fX*fX + fY*fY);
    g.drawEllipse(_ZirkOSC_Center_X-radiusZenith, _ZirkOSC_Center_Y-radiusZenith, radiusZenith*2, radiusZenith*2, 1.0);
}

void ZirkOscjuceAudioProcessorEditor::paintCrosshairs (Graphics& g){
    g.setColour(Colours::grey);
    float radianAngle=0.0f;
    float fraction = 0.9f;
    Point<float> axis = Point<float>();
    for (int i= 0; i<ZirkOSC_NumMarks; ++i) {
        radianAngle = degreeToRadian(ZirkOSC_MarksAngles[i]);
        axis = {cosf(radianAngle), sinf(radianAngle)};
        g.drawLine(_ZirkOSC_Center_X+(ZirkOscjuceAudioProcessor::s_iDomeRadius*fraction)*axis.getX(), _ZirkOSC_Center_Y+(ZirkOscjuceAudioProcessor::s_iDomeRadius*fraction)*axis.getY(),_ZirkOSC_Center_X+(ZirkOscjuceAudioProcessor::s_iDomeRadius)*axis.getX(), _ZirkOSC_Center_Y+(ZirkOscjuceAudioProcessor::s_iDomeRadius)*axis.getY(),1.0f);
    }
}

void ZirkOscjuceAudioProcessorEditor::paintCoordLabels (Graphics& g){
    g.setColour(Colours::black);
    g.drawLine(_ZirkOSC_Center_X - ZirkOscjuceAudioProcessor::s_iDomeRadius, _ZirkOSC_Center_Y, _ZirkOSC_Center_X + ZirkOscjuceAudioProcessor::s_iDomeRadius, _ZirkOSC_Center_Y ,0.5f);
    g.drawLine(_ZirkOSC_Center_X , _ZirkOSC_Center_Y - ZirkOscjuceAudioProcessor::s_iDomeRadius, _ZirkOSC_Center_X , _ZirkOSC_Center_Y + ZirkOscjuceAudioProcessor::s_iDomeRadius,0.5f);
}


/*Conversion function*/

/*!
* \param p : Point <float> (Azimuth,Elevation) in degree, xy in range [-r, r]
*/
Point <float> ZirkOscjuceAudioProcessorEditor::degreeToXy (Point <float> p){
    float x,y;
    x = -ZirkOscjuceAudioProcessor::s_iDomeRadius * sinf(degreeToRadian(p.getX())) * cosf(degreeToRadian(p.getY()));
    y = -ZirkOscjuceAudioProcessor::s_iDomeRadius * cosf(degreeToRadian(p.getX())) * cosf(degreeToRadian(p.getY()));
    return Point <float> (x, y);
}

/*!
 * Function called every 50ms to refresh value from Host
 * repaint only if the user is not moving any source (_isSourceBeingDragged)
 */
void ZirkOscjuceAudioProcessorEditor::timerCallback(){
    
#if defined(DEBUG)
    clock_t begin = clock();
    clock_t proc = clock();
#endif
    
    JUCE_COMPILER_WARNING("this update (and any other update) should only be done when there's actually something to change")
    updateSliders();
    
    switch(mTrState)
    {
        case kTrWriting:
        {
            Trajectory::Ptr t = ourProcessor->getTrajectory();
            if (t)
            {
                mTrProgressBar->setValue(t->progress());
            }
            else if (mTrState != kTrReady)
            {
                m_pWriteTrajectoryButton->setButtonText("Ready");
                mTrProgressBar->setVisible(false);
                m_pWriteTrajectoryButton->setToggleState(false, dontSendNotification);
                
                mTrState = kTrReady;
                startEditorTimer(ZirkOSC_reg_timerDelay);
            }
        }
            break;
    }

#if defined(DEBUG)
    clock_t sliders = clock();
#endif
    if (ourProcessor->hasToRefreshGui()){
        refreshGui();
        ourProcessor->setRefreshGui(false);
    }
    
#if defined(DEBUG)
    clock_t gui = clock();
#endif
    
    //if(!_isSourceBeingDragged){
          repaint();
    //}
    
#if defined(TIMING_TESTS)
    clock_t end = clock();
    cout << "processor:\t" << proc - begin <<"ms"<< endl;
    cout << "sliders:\t" << sliders - proc <<"ms"<< endl;
    cout << "ref gui:\t" << gui - sliders <<"ms"<< endl;
    cout << "repaint:\t" << end - gui <<"ms"<< endl;
    cout << "whole thing:\t" << end - begin <<"ms"<< endl;
#endif
    
}

void ZirkOscjuceAudioProcessorEditor::updateSliders(){
    int selectedSource = ourProcessor->getSelectedSource();
    
    //based on selected source, update all sliders
    m_pGainSlider->setValue (ourProcessor->getSources()[selectedSource].getGain(), dontSendNotification);
    
    float HRValue = PercentToHR(ourProcessor->getSources()[selectedSource].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    m_pAzimuthSlider->setValue(HRValue,dontSendNotification);
    //cout << "update sliders: " << ourProcessor->getSources()[selectedSource].getAzimuth() << "\n";
    
    HRValue = PercentToHR(ourProcessor->getSources()[selectedSource].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    m_pElevationSlider->setValue(HRValue,dontSendNotification);
    
    HRValue = PercentToHR(ourProcessor->getSources()[selectedSource].getAzimuthSpan(), ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);
    m_pAzimuthSpanSlider->setValue(HRValue,dontSendNotification);
    
    HRValue = PercentToHR(ourProcessor->getSources()[selectedSource].getElevationSpan(), ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max);
    m_pElevationSpanSlider->setValue(HRValue,dontSendNotification);
}

void ZirkOscjuceAudioProcessorEditor::refreshGui(){
    _ZkmOscPortTextEditor.setText(String(ourProcessor->getOscPortZirkonium()));
    _NbrSourceTextEditor.setText(String(ourProcessor->getNbrSources()));
    _FirstSourceIdTextEditor.setText(String(ourProcessor->getSources()[0].getChannel()));
    _MovementConstraintComboBox.setSelectedId(ourProcessor->getSelectedMovementConstraint());
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
    m_pTrajectoryCountTextEditor->setText(String(ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoryCount_ParamId)));
    m_pTrajectoryDurationTextEditor->setText(String(ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoriesDuration_ParamId)));
    
//    _IpadIncomingOscPortTextEditor.setText(ourProcessor->getOscPortIpadIncoming());
//    _IpadOutgoingOscPortTextEditor.setText(ourProcessor->getOscPortIpadOutgoing());
//    _IpadIpAddressTextEditor.setText(ourProcessor->getOscAddressIpad());
}

void ZirkOscjuceAudioProcessorEditor::buttonClicked (Button* button){
    
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
            
            t->stop();
            refreshGui();
        }
        else
        {
            ourProcessor->setIsWriteTrajectory(true);
            float duration = m_pTrajectoryDurationTextEditor->getText().getFloatValue();
            bool beats = m_pSyncWTempoComboBox->getSelectedId() == 1;
            int type = m_pTrajectoryTypeComboBox->getSelectedId();
            
            unique_ptr<AllTrajectoryDirections> direction = Trajectory::getTrajectoryDirection(type, m_pTrajectoryDirectionComboBox->getSelectedId()-1);
            
            bool bReturn = m_pTrajectoryReturnComboBox->getSelectedId() == 2;
            
            float repeats = m_pTrajectoryCountTextEditor->getText().getFloatValue();
            int source = ourProcessor->getSelectedSource();
            
            ourProcessor->setTrajectory(Trajectory::CreateTrajectory(type, ourProcessor, duration, beats, *direction, bReturn, repeats, source));
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
        ourProcessor->setIsJoystickEnabled(bIsJoystickEnabled);
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
                ourProcessor->setIsJoystickEnabled(false);
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
}





void ZirkOscjuceAudioProcessorEditor::sliderDragStarted (Slider* slider) {
    
    if (ourProcessor->getIsWriteTrajectory()){
        return;
    }
    
    int selectedSource = ourProcessor->getSelectedSource();                             //get selected source
    int selectedConstraint = ourProcessor->getSelectedMovementConstraint();    //get selected movement constraint
    bool isSpanLinked = ourProcessor->getIsSpanLinked();
    
    if (slider == m_pGainSlider) {
        ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Gain_ParamId + (selectedSource*5) );
    }
    else if (slider == m_pAzimuthSlider) {
        if (selectedConstraint == Independant){
            ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (selectedSource*5));
        } else {
            for(int i = 0;i<ourProcessor->getNbrSources(); ++i){
                ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + i*5);
            }
        }
    }
    else if (slider == m_pElevationSlider) {
        if (selectedConstraint == Independant){
            ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (selectedSource*5));
        } else {
            for(int i = 0;i<ourProcessor->getNbrSources(); ++i){
                ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + i*5);
            }
        }
    }
    else if (slider == m_pElevationSpanSlider) {
        if(isSpanLinked){
            for(int i=0 ; i<ourProcessor->getNbrSources(); ++i){
                ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + (i*5));
            }
        } else{
            ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + (selectedSource*5));
        }
        
    }
    else if (slider == m_pAzimuthSpanSlider) {
        if(isSpanLinked){
            for(int i=0 ; i<ourProcessor->getNbrSources(); ++i){
                ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + (i*5));
            }
        } else{
            ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + (selectedSource*5));
        }
    }
}

void ZirkOscjuceAudioProcessorEditor::sliderDragEnded (Slider* slider) {
    
    if (ourProcessor->getIsWriteTrajectory()){
        return;
    }
    
    int selectedSource = ourProcessor->getSelectedSource();                             //get selected source
    int selectedConstraint = ourProcessor->getSelectedMovementConstraint();    //get selected movement constraint
    bool isSpanLinked = ourProcessor->getIsSpanLinked();
    
    if (slider == m_pGainSlider) {
        ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Gain_ParamId + (selectedSource*5) );
    }
    else if (slider == m_pAzimuthSlider) {
        if (selectedConstraint == Independant){
            ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (selectedSource*5));
        } else {
            for(int i = 0;i<ourProcessor->getNbrSources(); ++i){
                ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + i*5);
            }
        }
    }
    else if (slider == m_pElevationSlider) {
        if (selectedConstraint == Independant){
            ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (selectedSource*5));
        } else {
            for(int i = 0;i<ourProcessor->getNbrSources(); ++i){
                ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + i*5);
            }
        }
    }
    else if (slider == m_pElevationSpanSlider) {
        if(isSpanLinked){
            for(int i=0 ; i<ourProcessor->getNbrSources(); ++i){
                ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + (i*5));
            }
        } else{
            ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + (selectedSource*5));
        }
        
    }
    else if (slider == m_pAzimuthSpanSlider) {
        if(isSpanLinked){
            for(int i=0 ; i<ourProcessor->getNbrSources(); ++i){
                ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + (i*5));
            }
        } else{
            ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + (selectedSource*5));
        }
    }

}

void ZirkOscjuceAudioProcessorEditor::sliderValueChanged (Slider* slider) {
    
    if (ourProcessor->getIsWriteTrajectory()){
        return;
    }
    
    int selectedSource = ourProcessor->getSelectedSource();
    bool isSpanLinked = ourProcessor->getIsSpanLinked();
    float percentValue=0;
    float fX, fY;
    
    if (slider == m_pGainSlider) {
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Gain_ParamId + (selectedSource*5), (float) m_pGainSlider->getValue());
    }
    else if (slider == m_pAzimuthSlider ){
        //figure out where the slider should move the point
        percentValue = HRToPercent((float) m_pAzimuthSlider->getValue(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
        SoundSource::azimElev01toXY(percentValue, ourProcessor->getSources()[selectedSource].getElevation(), fX, fY);
        move(selectedSource, fX, fY);
    }
    else if (slider == m_pElevationSlider){
        percentValue = HRToPercent((float) m_pElevationSlider->getValue(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
        SoundSource::azimElev01toXY(ourProcessor->getSources()[selectedSource].getAzimuth(), percentValue, fX, fY);
        move(selectedSource, fX, fY);
    }
    else if (slider == m_pElevationSpanSlider) {
        percentValue = HRToPercent((float) m_pElevationSpanSlider->getValue(), ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max);
        if(isSpanLinked){
            for(int i=0 ; i<ourProcessor->getNbrSources(); ++i){
                ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + (i*5), percentValue);
            }
        }
        else{
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + (selectedSource*5), percentValue);
        }
    }
    else if (slider == m_pAzimuthSpanSlider) {
        percentValue = HRToPercent((float) m_pAzimuthSpanSlider->getValue(), ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);
        if(isSpanLinked){
            for(int i=0 ; i<ourProcessor->getNbrSources(); ++i){
                ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + (i*5), percentValue);
            }
        }
        else{
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + (selectedSource*5), percentValue);
        }
    }
}

void ZirkOscjuceAudioProcessorEditor::mouseDown (const MouseEvent &event){
    
    if (ourProcessor->getIsWriteTrajectory()){
        return;
    }
    
    int source=-1;

    //if event is within the wall circle, select source that is clicked on (if any)
    if (event.x>5 && event.x <20+ZirkOscjuceAudioProcessor::s_iDomeRadius*2 && event.y>5 && event.y< 40+ZirkOscjuceAudioProcessor::s_iDomeRadius*2) {
        source=getSourceFromPosition(Point<float>(event.x-_ZirkOSC_Center_X, event.y-_ZirkOSC_Center_Y));
    }
    
    //if a source is clicked on, flag _isSourceBeingDragged to true
    _isSourceBeingDragged = (source!=-1);


    
    if(_isSourceBeingDragged){

        //if sources are being dragged, tell host that their parameters are about to change (beginParameterChangeGesture). Logic needs this
        ourProcessor->setSelectedSource(source);
        int selectedConstraint = ourProcessor->getSelectedMovementConstraint();
        if (selectedConstraint == Independant) {
            ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + source*5);
            ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + source*5);
        }
        else {
            for(int i = 0;i<ourProcessor->getNbrSources(); ++i){
                ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + i*5);
                ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + i*5);
                ourProcessor->getSources()[i].setAzimReverse(false);
            }
        }
        //repaint();
    }
    //m_pGainSlider->grabKeyboardFocus();
    _MovementConstraintComboBox.grabKeyboardFocus();
}

int ZirkOscjuceAudioProcessorEditor::getSourceFromPosition(Point<float> p ){
    for (int i=0; i<ourProcessor->getNbrSources() ; ++i){
        if (ourProcessor->getSources()[i].contains(p)){
            return i;
        }
    }
    return -1;
}


void ZirkOscjuceAudioProcessorEditor::mouseDrag (const MouseEvent &event){
    if(_isSourceBeingDragged){
        
        if (ourProcessor->getIsWriteTrajectory()){
            return;
        }
        
        int selectedSource = ourProcessor->getSelectedSource();
        
        //get point of current event
        Point<float> pointRelativeCenter = Point<float>(event.x-_ZirkOSC_Center_X, event.y-_ZirkOSC_Center_Y);
        
        //need to clamp the point to the circle
        if (ZirkOscjuceAudioProcessor::s_bUseXY){
            float fCurR = hypotf(pointRelativeCenter.x, pointRelativeCenter.y);
            if ( fCurR > ZirkOscjuceAudioProcessor::s_iDomeRadius){
                float fExtraRatio = ZirkOscjuceAudioProcessor::s_iDomeRadius / fCurR;
                
                pointRelativeCenter.x *= fExtraRatio;
                pointRelativeCenter.y *= fExtraRatio;
            }
        }
        move(selectedSource, pointRelativeCenter.x, pointRelativeCenter.y);
    }
    _MovementConstraintComboBox.grabKeyboardFocus();
}

void ZirkOscjuceAudioProcessorEditor::move(int p_iSource, float p_fX, float p_fY){
    if (p_iSource > ourProcessor->getNbrSources()){
        return;
    }
    
    int selectedConstraint = ourProcessor->getSelectedMovementConstraint();
    if(selectedConstraint == Independant) {
        if (ZirkOscjuceAudioProcessor::s_bUseXY){
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + p_iSource*5, HRToPercent(p_fX, -ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius));
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + p_iSource*5, HRToPercent(p_fY, -ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius));
        } else {
            Point<float> newPoint(p_fX,p_fY);
            ourProcessor->getSources()[p_iSource].setXY(newPoint);
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + p_iSource*5, ourProcessor->getSources()[p_iSource].getAzimuth());
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + p_iSource*5, ourProcessor->getSources()[p_iSource].getElevation());
        }
        
    }
    
    //not-independent
    else {
        
        if (selectedConstraint == FixedAngles){
            moveFixedAngles(p_iSource, p_fX, p_fY);
        } else if (selectedConstraint == FixedRadius){
            //moveCircularWithFixedRadius(p_iSource, p_fX, p_fY);
            ourProcessor->moveCircular(p_iSource, p_fX, p_fY, true);
        } else if (selectedConstraint == FullyFixed){
            moveFullyFixed(p_iSource, p_fX, p_fY);
        } else if (selectedConstraint == DeltaLocked){
            float oldX, oldY;
            ourProcessor->getSources()[p_iSource].getXY(oldX,oldY);
            float deltax = p_fX - oldX;
            float deltay = p_fY - oldY;
            moveSourcesWithDelta(p_iSource, deltax, deltay);
        } else if (selectedConstraint == Circular){
            ourProcessor->moveCircular(p_iSource, p_fX, p_fY, false);
        }
    }
    
    JUCE_COMPILER_WARNING("HEXA: this should be the only time we're calling sendOSCValues, no? or maybe even we don't need to since the processor is doing it?")
    ourProcessor->sendOSCValues();
}

void ZirkOscjuceAudioProcessorEditor::mouseUp (const MouseEvent &event){
    
    if (ourProcessor->getIsWriteTrajectory()){
        return;
    }
    
    if(_isSourceBeingDragged){
        int selectedConstrain = ourProcessor->getSelectedMovementConstraint();
        if(selectedConstrain == Independant){
            int selectedSource = ourProcessor->getSelectedSource();
            ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId+ selectedSource*5);
            ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + selectedSource*5);
            
        }
        else {
            for(int i = 0;i<ourProcessor->getNbrSources(); ++i){
                ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + i*5);
                ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + i*5);
            }
        }
        _isSourceBeingDragged=false;
    }
    _MovementConstraintComboBox.grabKeyboardFocus();
}

void ZirkOscjuceAudioProcessorEditor::moveFixedAngles(const int &p_iSource, const float &p_fX, const float &p_fY){
    if (_isNeedToSetFixedAngles){
        orderSourcesByAngle(ourProcessor->getSelectedSource(),ourProcessor->getSources());
        _isNeedToSetFixedAngles=false;
    }
    ourProcessor->moveCircular(p_iSource, p_fX, p_fY, false);
}

void ZirkOscjuceAudioProcessorEditor::moveFullyFixed(const int &p_iSource, const float &p_fX, const float &p_fY){
    if (_isNeedToSetFixedAngles){
        orderSourcesByAngle(ourProcessor->getSelectedSource(),ourProcessor->getSources());
        _isNeedToSetFixedAngles=false;
    }
    ourProcessor->moveCircular(p_iSource, p_fX, p_fY, true);
}

void ZirkOscjuceAudioProcessorEditor::orderSourcesByAngle (int selected, SoundSource tab[]){
    int nbrSources = ourProcessor->getNbrSources();
    vector<int> order = getOrderSources(selected, tab, nbrSources);
    int count = 0;
    for(int i= 1; i < nbrSources ; ++i){ //for(int i= 1; i != nbrSources ; ++i){
        float curangle = tab[order[0]].getAzimuth()+ (float)(++count)/(float) nbrSources;
        if (ZirkOscjuceAudioProcessor::s_bUseXY){
            float fX, fY;
            SoundSource::azimElev01toXY01(curangle, tab[order[i]].getElevation(), fX, fY);
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (order[i]*5), fX);
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (order[i]*5), fY);
        } else {
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (order[i]*5), curangle);
        }
    }
}

//starting from the selected source, cycle through the other sources to find in which order they are
vector<int> ZirkOscjuceAudioProcessorEditor::getOrderSources(int selected, SoundSource tab [], int nbrSources){

    vector<int> order(nbrSources);
    int firstItem = selected;
    order[0] = selected;    //selected source is at order[0]
    int count  = 1;
    do{
        int current = (selected + 1)%nbrSources; //current is the next one after the seleted one
        
        int bestItem = current;
        float bestDelta = tab[current].getAzimuth() - tab[selected].getAzimuth(); //difference between current and selected
        if (bestDelta<0){
            bestDelta+=1;
        }
        
        while (current != selected) {
            float currentAzimuth;
            if (tab[current].getAzimuth() - tab[selected].getAzimuth()>0 ){
                currentAzimuth = tab[current].getAzimuth();
            }
            else{
                currentAzimuth = tab[current].getAzimuth()+1;
            }
            if (currentAzimuth - tab[selected].getAzimuth() < bestDelta) {
                bestItem = current;
                bestDelta = currentAzimuth - tab[selected].getAzimuth();
                
            }
            current = (current +1) % nbrSources;
        }
        
        order[count++]=bestItem;
        selected = bestItem;
    } while (selected != firstItem && count < nbrSources);
    return order;
}




JUCE_COMPILER_WARNING("this was an unfinished attempt at simplifying moveCircular(), but we sill need to convert to azim elev when radius is fixed, so not clear that this will improve performance")
//void ZirkOscjuceAudioProcessorEditor::moveCircular(const int &p_iSelSource, const float &p_fNewSelX, const float &p_fNewSelY, bool p_bIsRadiusFixed){
//    if (!ZirkOscjuceAudioProcessor::s_bUseXY){
//        Point<float> pointRelativeCenter(p_fNewSelX, p_fNewSelY);
//        moveCircularAzimElev(pointRelativeCenter, p_bIsRadiusFixed);
//        return;
//    }
//    
//    //convert x,y[0,1] to azim,elev[0,1]
//    float fOldSelX = ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (p_iSelSource*5));
//    float fOldSelY = ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (p_iSelSource*5));
//    
//    //set selectedSource to its xy01 position
//    ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + p_iSelSource*5, HRToPercent(p_fNewSelX, -ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius));
//    ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + p_iSelSource*5, HRToPercent(p_fNewSelY, -ZirkOscjuceAudioProcessor::s_iDomeRadius, ZirkOscjuceAudioProcessor::s_iDomeRadius));
//    
//    //calculate deltas in 01
//    float fDeltaSelX = p_fNewSelX - fOldSelX;
//    float fDeltaSelY = p_fNewSelY - fOldSelY;
//    
//    //for all other sources
//    for (int iCurSource = 0; iCurSource < ourProcessor->getNbrSources(); ++iCurSource) {
//        
//        if (iCurSource == p_iSelSource){
//            continue;
//        }
//        
//        //convert xy01 to azim elev01
//        float fCurX = ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (iCurSource*5));
//        float fCurY = ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (iCurSource*5));
//        
//        //calculate new azimuth
//        float fNewX = fCurX + fDeltaSelX;
//        float fNewY = fCurY + fDeltaSelY;
//        
//        //if radius is fixed, set all elevation to the same xy
//        if (p_bIsRadiusFixed){
////            float fX, fY;
////            SoundSource::azimElev01toXY01(fNewAzim, fSelectedElev, fX, fY);
////            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (iCurSource*5), fX);
////            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (iCurSource*5), fY);
//            
//            SoundSource::XY01toAzimElev01();
//        
//        }
//        //if radius is not fixed, set all elevation to be current elevation +/- deltaY
//        else {
//            //if azimuth is NOT reversed, ie, NOT on the other side of the dome's middle point
//            if(!ourProcessor->getSources()[iCurSource].isAzimReverse()){
//                float fX, fY;
//                SoundSource::azimElev01toXY01(fNewAzim, fCurElev + fDeltaElev, fX, fY);
//                ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (iCurSource*5), fX);
//                ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (iCurSource*5), fY);
//            } else {
//                float fX, fY;
//                SoundSource::azimElev01toXY01(fNewAzim, fCurElev - fDeltaElev, fX, fY);
//                ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + (iCurSource*5), fX);
//                ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + (iCurSource*5), fY);
//            }
//        }
//    }
//}


void ZirkOscjuceAudioProcessorEditor::moveCircularAzimElev(Point<float> pointRelativeCenter, bool isRadiusFixed){
    int selectedSource = ourProcessor->getSelectedSource();
    
    //figure out new location that source should move to
    SoundSource sourceNewLocation = SoundSource();
    sourceNewLocation.setXY(pointRelativeCenter);
    
    float HRNewElevation = PercentToHR(sourceNewLocation.getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    float HRNewAzimuth   = PercentToHR(sourceNewLocation.getAzimuth(),   ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    
    //figure out previous/current location
    float currentElevation = ourProcessor->getSources()[selectedSource].getElevation();
    float currentAzimuth   = ourProcessor->getSources()[selectedSource].getAzimuth();
    float HRElevation   =       PercentToHR(currentElevation, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    float HRAzimuth     = 180 + PercentToHR(currentAzimuth,   ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    
    //calculate difference between new and previous location
    Point<float> deltaCircularMove = Point<float>(HRNewAzimuth-HRAzimuth, HRNewElevation-HRElevation);
    float deltaX = HRToPercent(deltaCircularMove.getX(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    float deltaY = HRToPercent(deltaCircularMove.getY(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    
    //for all sources
    for (int iCurSource = 0; iCurSource < ourProcessor->getNbrSources(); ++iCurSource) {
        
        //get current source Azimuth and elevation
        float curSourceAzim = ourProcessor->getSources()[iCurSource].getAzimuth();
        curSourceAzim += deltaX;
        float curSourceElevRaw = ourProcessor->getSources()[iCurSource].getElevationRawValue();
        
        //set new azimuth through host
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + iCurSource * 5, curSourceAzim);
        
        //if radius is fixed, set all elevation to the same thing
        if (isRadiusFixed){
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + iCurSource * 5, (currentElevation + HRToPercent(deltaCircularMove.getY(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max)));
        }
        //if radius is not fixed, set all elevation to be current elevation +/- deltaY
        else {
            //if azimuth is NOT reversed, ie, NOT on the other side of the dome's middle point
            if(!ourProcessor->getSources()[iCurSource].isAzimReverse()){
                ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + iCurSource * 5, curSourceElevRaw + deltaY);
            } else {
                ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + iCurSource * 5, curSourceElevRaw - deltaY);
            }
        }
    }
}


void ZirkOscjuceAudioProcessorEditor::moveSourcesWithDelta(const int &p_iSource, const float &p_fX, const float &p_fY){
    if (!ZirkOscjuceAudioProcessor::s_bUseXY){
        moveSourcesWithDeltaAzimElev(Point<float>(p_fX, p_fY));
        return;
    }
    
    //simply need to move all sources to their current position + deltamove
    for(int i=0; i<ourProcessor->getNbrSources(); ++i){
        
        float currentX, currentY;
        ourProcessor->getSources()[i].getXY(currentX, currentY);
        float newX = currentX + p_fX;
        float newY = currentY + p_fY;

        JUCE_COMPILER_WARNING("HEXA: if we clamp, sources that fall outside the circle don,t bounce back; but if we don't clamp, location parameters fall outside their correct range...")
        //SoundSource::clampXY(newPosition.x, newPosition.y);
        
        float fX01 = (newX + ZirkOscjuceAudioProcessor::s_iDomeRadius) / (2*ZirkOscjuceAudioProcessor::s_iDomeRadius);
        float fY01 = (newY + ZirkOscjuceAudioProcessor::s_iDomeRadius) / (2*ZirkOscjuceAudioProcessor::s_iDomeRadius);


        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + i * 5, fX01);
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + i * 5, fY01);
        
        //ourProcessor->sendOSCValues();
    }
}

void ZirkOscjuceAudioProcessorEditor::moveSourcesWithDeltaAzimElev(Point<float> DeltaMove){
    int nbrSources = ourProcessor->getNbrSources();
    bool inTheDome = true;
    for (int i =0; i<ourProcessor->getNbrSources()&&inTheDome; ++i) {
        inTheDome=ourProcessor->getSources()[i].isStillInTheDome(DeltaMove);
    }
    if (inTheDome){
        for(int i=0;i<nbrSources;++i){
            float currentx, currenty;
            ourProcessor->getSources()[i].getXY(currentx, currenty);
            float deltax = currentx+DeltaMove.x;
            float deltay = currentx+DeltaMove.y;
            ourProcessor->getSources()[i].setXYUsingAzimElev(deltax, deltay);
                ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_or_x_ParamId + i*5, ourProcessor->getSources()[i].getAzimuth());
                ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_or_y_ParamId + i*5, ourProcessor->getSources()[i].getElevationRawValue());
            //azimuthLabel.setText(String(ourProcessor->tabSource[i].getElevation()), false);
            //ourProcessor->sendOSCValues();
        }
    }
    //repaint();
}

void ZirkOscjuceAudioProcessorEditor::textEditorFocusLost (TextEditor &textEditor){
    _isReturnKeyPressedCalledFromFocusLost = true;
    textEditorReturnKeyPressed(textEditor);
    _isReturnKeyPressedCalledFromFocusLost = false;
}


void ZirkOscjuceAudioProcessorEditor::textEditorReturnKeyPressed (TextEditor &textEditor){
    
    String text = textEditor.getText();
    int intValue = textEditor.getText().getIntValue();

    if(&_NbrSourceTextEditor == &textEditor) {
        //if we have a valid number of sources, set it in processor
        if(intValue >=1 && intValue <= 8){
            ourProcessor->setNbrSources(intValue);
            
            //need to give those new sources IDs, so get first source ID
            int sourceId = ourProcessor->getSources()[0].getChannel();
            //updating leapSource Combobox
            m_pCBLeapSource->clear();
            int firstSource =_FirstSourceIdTextEditor.getText().getIntValue();
            int j = 1;
            for(int i = firstSource; i<intValue+firstSource; i++)
            {
                m_pCBLeapSource->addItem((String)i, j);
                j++;
            }
            m_pCBLeapSource->setSelectedId(ourProcessor->getSelectedSource()+1);
            m_pCBLeapSource->addListener(this);
            //then set all subsequent source IDs to subsequent numbers
            for (int iCurSource = 1; iCurSource < 8; ++iCurSource){
                ourProcessor->getSources()[iCurSource].setChannel(++sourceId);
            }
            
            //toggle fixed angle repositioning, if we need to
            int selectedConstraint = ourProcessor->getSelectedMovementConstraint();
            if(selectedConstraint == FixedAngles || selectedConstraint == FullyFixed){
                _isNeedToSetFixedAngles=true;
            }
        }
        //otherwise just ignore new value
        else{
            _NbrSourceTextEditor.setText(String(ourProcessor->getNbrSources()));
        }
    }
    
    else if(&_FirstSourceIdTextEditor == &textEditor ){
        
        //we only have room for 3 digits, so limit that field to 3 digits
        if (intValue > 999 - 8 || intValue < -99 ){
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
            ourProcessor->getSources()[iCurSource].setChannel(newChannel++);
        }
       //ourProcessor->sendOSCValues();
    }
    
    else if(&_ZkmOscPortTextEditor == &textEditor ){
        int newPort = intValue;
        ourProcessor->changeZirkoniumOSCPort(newPort);
        _ZkmOscPortTextEditor.setText(String(ourProcessor->getOscPortZirkonium()));
    }
    
    else if(m_pTrajectoryCountTextEditor == &textEditor ){
        double doubleValue = textEditor.getText().getDoubleValue();
        if (doubleValue >= 0 && doubleValue < 10000){
            ourProcessor->setParameter(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoryCount_ParamId, doubleValue);

        }
        m_pTrajectoryCountTextEditor->setText(String(ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoryCount_ParamId)));
    }
    
    else if(m_pTrajectoryDurationTextEditor == &textEditor){
        double doubleValue = textEditor.getText().getDoubleValue();
        if (doubleValue >= 0 && doubleValue < 10000){
            ourProcessor->setParameter(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoriesDuration_ParamId, doubleValue);
        }
        m_pTrajectoryDurationTextEditor->setText(String(ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoriesDuration_ParamId)));
    }
    
//    else if (&_IpadOutgoingOscPortTextEditor == &textEditor) {
//        int newIpadOutgoingPort = intValue;
//        String curIpAddress = ourProcessor->getOscAddressIpad();
//        
//        ourProcessor->changeOSCSendIPad(newIpadOutgoingPort, curIpAddress);
//        
//        _IpadOutgoingOscPortTextEditor.setText(ourProcessor->getOscPortIpadOutgoing());
//    }
//    
//    else if (&_IpadIpAddressTextEditor == &textEditor) {
//        String newIpAddress = text;
//        int curOscOutgoingPort = ourProcessor->getOscPortIpadOutgoing().getIntValue();
//        
//        ourProcessor->changeOSCSendIPad(curOscOutgoingPort, newIpAddress);
//        
//        _IpadIpAddressTextEditor.setText(ourProcessor->getOscAddressIpad());
//    }
//    
//    else if (&_IpadIncomingOscPortTextEditor == &textEditor) {
//        int newIpadIncomingPort = intValue;
//        ourProcessor->changeOSCReceiveIpad(newIpadIncomingPort);
//        _IpadIncomingOscPortTextEditor.setText(ourProcessor->getOscPortIpadIncoming());
//        
//    }
    ourProcessor->sendOSCValues();
    if (!_isReturnKeyPressedCalledFromFocusLost){
        //m_pGainSlider->grabKeyboardFocus();
        _MovementConstraintComboBox.grabKeyboardFocus();
    }
}

void ZirkOscjuceAudioProcessorEditor::comboBoxChanged (ComboBox* comboBoxThatHasChanged){

    if (comboBoxThatHasChanged == &_MovementConstraintComboBox){
        
        int selectedConstraint = comboBoxThatHasChanged->getSelectedId();
        
        float fSelectedConstraint = IntToPercentStartsAtOne(selectedConstraint, TotalNumberConstraints);
        
        ourProcessor->setParameterNotifyingHost(ZirkOscjuceAudioProcessor::ZirkOSC_MovementConstraint_ParamId, fSelectedConstraint);

        if(selectedConstraint == FixedAngles || selectedConstraint == FullyFixed){
            _isNeedToSetFixedAngles=true;
        }
    }
    
    else if (comboBoxThatHasChanged == m_pTrajectoryTypeComboBox){
        int iSelectedTraj = comboBoxThatHasChanged->getSelectedId();
        float fSelectedTraj = IntToPercentStartsAtOne(iSelectedTraj, TotalNumberTrajectories);
        ourProcessor->setParameterNotifyingHost(ZirkOscjuceAudioProcessor::ZirkOSC_SelectedTrajectory_ParamId, fSelectedTraj);
        
        updateTrajectoryComboboxes();
    }
    
    else if(comboBoxThatHasChanged == m_pSyncWTempoComboBox){
        if (comboBoxThatHasChanged->getSelectedId() == SyncWTempo){
            ourProcessor->setIsSyncWTempo(true);
        } else {
            ourProcessor->setIsSyncWTempo(false);
        }
    }
    else if (comboBoxThatHasChanged == m_pCBLeapSource)
    {
        ourProcessor->setSelectedSource(comboBoxThatHasChanged->getSelectedId()-1);
    }
    
    else if (comboBoxThatHasChanged == m_pTrajectoryDirectionComboBox){
        float fSelectedDirection = IntToPercentStartsAtOne(comboBoxThatHasChanged->getSelectedId(), getNumSelectedTrajectoryDirections());
        ourProcessor->setParameterNotifyingHost(ZirkOscjuceAudioProcessor::ZirkOSC_SelectedTrajectoryDirection_ParamId, fSelectedDirection);
    }
    
    else if (comboBoxThatHasChanged == m_pTrajectoryReturnComboBox){
        float fSelectedReturn = IntToPercentStartsAtOne(comboBoxThatHasChanged->getSelectedId(), getNumSelectedTrajectoryReturns());
        ourProcessor->setParameterNotifyingHost(ZirkOscjuceAudioProcessor::ZirkOSC_SelectedTrajectoryReturn_ParamId, fSelectedReturn);
    }
}

int ZirkOscjuceAudioProcessorEditor::getNumSelectedTrajectoryDirections(){
    int iSelectedTraj = m_pTrajectoryTypeComboBox->getSelectedId();
    auto allDirections = Trajectory::getTrajectoryPossibleDirections(iSelectedTraj);
    if (allDirections != nullptr){
        return allDirections->size();
    } else {
        return -1;
    }
    
}

int ZirkOscjuceAudioProcessorEditor::getNumSelectedTrajectoryReturns(){
    int iSelectedTraj = m_pTrajectoryTypeComboBox->getSelectedId();
    auto allReturns = Trajectory::getTrajectoryPossibleReturns(iSelectedTraj);
    if (allReturns != nullptr){
        return allReturns->size();
    } else {
        return -1;
    }
}


bool ZirkOscjuceAudioProcessorEditor::isFixedAngle(){
    return _isNeedToSetFixedAngles;
}

void ZirkOscjuceAudioProcessorEditor::setFixedAngle(bool fixedAngle){
    _isNeedToSetFixedAngles = fixedAngle;
}

void ZirkOscjuceAudioProcessorEditor::setDraggableSource(bool drag){
    _isSourceBeingDragged = drag;
}

bool ZirkOscjuceAudioProcessorEditor::isDraggableSource(){
    return _isSourceBeingDragged;
}

int ZirkOscjuceAudioProcessorEditor::getNbSources()
{
    return getProcessor()->getNbrSources();
}
void ZirkOscjuceAudioProcessorEditor::uncheckJoystickButton()
{
    m_pTBEnableJoystick->setToggleState(false, dontSendNotification);
    buttonClicked(m_pTBEnableJoystick);
}
int ZirkOscjuceAudioProcessorEditor::getCBSelectedSource()
{
    return m_pCBLeapSource->getSelectedId();
}






