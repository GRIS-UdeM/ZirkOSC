/*
 ==============================================================================

 Lexic :
 - all parameter preceeded by HR are Human Readable
 - parameter without HR are in percent
 - Points in spheric coordinates  : x -> azimuth, y -> elevation

 ==============================================================================
 */

#ifndef DEBUG
    #define DEBUG
#endif
#undef DEBUG

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ZirkConstants.h"
#include <cstdlib>
#include <string>
#include <string.h>
#include <sstream>
#include <istream>
#include <math.h>
#include <ctime>

using namespace std;

bool ZirkOscjuceAudioProcessorEditor::_AlreadySetTrajectorySource = false;



class TrajectoryTab : public Component{
    
    OwnedArray<Component> components;
    
    template <typename ComponentType> ComponentType* addToList (ComponentType* newComp){
        components.add (newComp);
        addAndMakeVisible (newComp);
        return newComp;
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectoryTab)
    
public:
    TrajectoryTab(){
        
    }
};




class SlidersTab : public Component{
    
    OwnedArray<Component> components;
    
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


public:
    template <typename ComponentType> ComponentType* addToList (ComponentType* newComp){
        components.add (newComp);
        addAndMakeVisible (newComp);
        return newComp;
    }
    
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

        m_pAzimuthSlider        = addToList (new Slider(ZirkOSC_Azim_name[0]));
        m_pAzimuthLabel         = addToList (new Label( ZirkOSC_Azim_name[0]));
        
        m_pElevationSlider      = addToList (new Slider(ZirkOSC_Elev_name[0]));
        m_pElevationLabel       = addToList (new Label( ZirkOSC_Elev_name[0]));
        
        m_pAzimuthSpanSlider    = addToList (new Slider(ZirkOSC_AzimSpan_name[0]));
        m_pAzimuthSpanLabel     = addToList (new Label( ZirkOSC_AzimSpan_name[0]));
        
        m_pElevationSpanSlider  = addToList (new Slider(ZirkOSC_ElevSpan_name[0]));
        m_pElevationSpanLabel   = addToList (new Label( ZirkOSC_ElevSpan_name[0]));

    }
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SlidersTab)
};


/*!
*  \param ownerFilter : the processor processor
*/

ZirkOscjuceAudioProcessorEditor::ZirkOscjuceAudioProcessorEditor (ZirkOscjuceAudioProcessor* ownerFilter)
:   AudioProcessorEditor (ownerFilter),
_LinkSpanButton("Link Span"),
_OscActiveButton("OSC active"),
//_SyncWTempoButton("Sync with Tempo"),
//_WriteTrajectoryButton("Write Trajectory"),
_TabComponent(TabbedButtonBar::TabsAtTop),
//_SliderComponent(),
//_TrajectoryComponent(),

//_isReturnKeyPressedCalledFromFocusLost(false),
//_AzimuthSlider(ZirkOSC_AzimSpan_name[0]),
//_ElevationSlider(ZirkOSC_ElevSpan_name[0]),
//_ElevationSpanSlider(ZirkOSC_ElevSpan_name[0]),
//_AzimuthLabel(ZirkOSC_Azim_name[0]),
//_AzimuthSpanLabel(ZirkOSC_AzimSpan_name[0]),
//_ElevationLabel(ZirkOSC_Elev_name[0]),
//_GainSlider (ZirkOSC_Gain_name[0]),
//_GainLabel(ZirkOSC_Gain_name[0]),

//strings in parameters are all used as juce::component names
_FirstSourceIdLabel("channelNbr"),
_ZkmOscPortLabel("OscPort"),
_NbrSourceLabel("NbrSources"),
_IpadOutgoingOscPortLabel("OSCPortOutgoingIPad"),
_IpadIncomingOscPortLabel("OSCIpadInco"),
_IpadIpAddressLabel("ipadadressLabel"),


_TrajectoryCountLabel("trajectoryCountLabel"),
_TrajectoryDurationLabel("trajectoryDurationLabel"),
_FirstSourceIdTextEditor("channelNbr"),
_ZkmOscPortTextEditor("OscPort"),
_NbrSourceTextEditor("NbrSource"),
_IpadOutgoingOscPortTextEditor("OSCPortOutgoingIPadTE"),
_IpadIncomingOscPortTextEditor("OSCIpadIncoTE"),
_IpadIpAddressTextEditor("ipaddress"),
//_TrajectoryCountTextEditor("trajectoryCountTE"),
//_TrajectoryDurationTextEditor("trajectoryDurationTE"),
_MovementConstraintComboBox("MovementConstraint")
//_TrajectoryGroup("trajectoryGroup", "Programmed Trajectories")
//_TrajectoryComboBox("Trajectory")
{

    
    //---------- RIGHT SIDE LABELS ----------
    _NbrSourceLabel.setText("Nbr Sources",  dontSendNotification);
    _NbrSourceTextEditor.setText(String(getProcessor()->getNbrSources()));
    addAndMakeVisible(&_NbrSourceLabel);
    addAndMakeVisible(&_NbrSourceTextEditor);
    
    _FirstSourceIdLabel.setText("1st source ID",  dontSendNotification);
    _FirstSourceIdTextEditor.setText(String(getProcessor()->getSources()[0].getChannel()));
    addAndMakeVisible(&_FirstSourceIdLabel);
    addAndMakeVisible(&_FirstSourceIdTextEditor);
    
    _ZkmOscPortLabel.setText("ZKM OSC Port",  dontSendNotification);
    _ZkmOscPortTextEditor.setText(String(getProcessor()->getOscPortZirkonium()));
    addAndMakeVisible(&_ZkmOscPortLabel);
    addAndMakeVisible(&_ZkmOscPortTextEditor);
    
    _IpadIncomingOscPortLabel.setText("Inc. port",  dontSendNotification);
    _IpadIncomingOscPortTextEditor.setText(String(getProcessor()->getOscPortIpadIncoming()));
    addAndMakeVisible(&_IpadIncomingOscPortLabel);
    addAndMakeVisible(&_IpadIncomingOscPortTextEditor);
    
    _IpadOutgoingOscPortLabel.setText("Out. port",  dontSendNotification);
    _IpadOutgoingOscPortTextEditor.setText(String(getProcessor()->getOscPortIpadOutgoing()));
    addAndMakeVisible(&_IpadOutgoingOscPortLabel);
    addAndMakeVisible(&_IpadOutgoingOscPortTextEditor);
    
    _IpadIpAddressLabel.setText("iPad IP add.",  dontSendNotification);
    _IpadIpAddressTextEditor.setText(String(getProcessor()->getOscAddressIpad()));
    addAndMakeVisible(&_IpadIpAddressLabel);
    addAndMakeVisible(&_IpadIpAddressTextEditor);
    
    
    
    //---------- SETTING UP TABS ----------
    slidersTab = new SlidersTab();
    _TabComponent.addTab("Sliders", Colours::lightgrey, slidersTab, true);
//    _TabComponent.addTab("Trajectories", Colours::lightgrey, &_TrajectoryComponent, true);
    _TabComponent.addTab("Properties", Colours::lightgrey, &m_oPropertyPanel, true);
    addAndMakeVisible(_TabComponent);
    
    
    
    //---------- SLIDERS ----------
    m_pGainSlider = slidersTab->getGainSlider();
    m_pGainLabel  = slidersTab->getGainLabel();
    setSliderAndLabel("Gain", m_pGainSlider, m_pGainLabel, ZirkOSC_Gain_Min, ZirkOSC_Gain_Max);
    m_pGainSlider->addListener(this);
    
    m_pAzimuthSlider = slidersTab->getAzimuthSlider();
    m_pAzimuthLabel  = slidersTab->getAzimuthLabel();
    setSliderAndLabel("Azimuth", m_pAzimuthSlider, m_pAzimuthLabel, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    m_pAzimuthSlider->addListener(this);
    
    m_pElevationSlider = slidersTab->getElevationSlider();
    m_pElevationLabel  = slidersTab->getElevationLabel();
    setSliderAndLabel("Elevation", m_pElevationSlider, m_pElevationLabel, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    m_pElevationSlider->addListener(this);
    
    m_pElevationSpanSlider = slidersTab->getElevationSpanSlider();
    m_pElevationSpanLabel  = slidersTab->getElevationSpanLabel();
    setSliderAndLabel("ElevationSpan", m_pElevationSpanSlider, m_pElevationSpanLabel, ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max);
    m_pElevationSpanSlider->addListener(this);
    
    m_pAzimuthSpanSlider = slidersTab->getAzimuthSpanSlider();
    m_pAzimuthSpanLabel  = slidersTab->getAzimuthSpanLabel();
    setSliderAndLabel("AzimuthSpan", m_pAzimuthSpanSlider, m_pAzimuthSpanLabel, ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);
    m_pAzimuthSpanSlider->addListener(this);
    
    
    
    

//    setSliderAndLabel("Azimuth", &_AzimuthSlider ,&_AzimuthLabel, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
//    _SliderComponent.addAndMakeVisible(&_AzimuthSlider);
//    _SliderComponent.addAndMakeVisible(&_AzimuthLabel);
//    _AzimuthSlider.addListener(this);
//
//    setSliderAndLabel("Elevation", &_ElevationSlider, &_ElevationLabel, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
//    _SliderComponent.addAndMakeVisible(&_ElevationSlider);
//    _SliderComponent.addAndMakeVisible(&_ElevationLabel);
//    _ElevationSlider.addListener(this);
//
//    setSliderAndLabel("Elev. Span.", &_ElevationSpanSlider, &_ElevationSpanLabel, ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max);
//    _SliderComponent.addAndMakeVisible(&_ElevationSpanSlider);
//    _SliderComponent.addAndMakeVisible(&_ElevationSpanLabel);
//    _ElevationSpanSlider.addListener(this);
//
//    setSliderAndLabel("Azim. Span.", &_AzimuthSpanSlider, &_AzimuthSpanLabel, ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);
//    _SliderComponent.addAndMakeVisible(&_AzimuthSpanSlider);
//    _SliderComponent.addAndMakeVisible(&_AzimuthSpanLabel);
//    _AzimuthSpanSlider.addListener(this);
    
    
    
    
    


    

    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    
    m_bUseIpad = ourProcessor-> m_bUseIpad;
   
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
    int selected_id = ourProcessor->getSelectedMovementConstraintAsInteger();
    _MovementConstraintComboBox.setSelectedId(selected_id);
    _MovementConstraintComboBox.addListener(this);
#warning make sure this is displayed above the tabs
    addAndMakeVisible(&_MovementConstraintComboBox);
    
    
    
    
    
    //---------- TRAJECTORY COMPONENTS ----------
    
    //addAndMakeVisible (m_oPropertyPanel);

    
    Array<PropertyComponent*> allProperties;
    
    
    //this needs to be replaced by a call to ChoicePropertyComponent::setIndex()
    //    _TrajectoryComboBox.addListener(this);
    
    //COMBO BOX

    m_oTrajectoryComboBoxProperty = new TrajectoryComboBoxComponent("Trajectory:", ourProcessor);
    m_oTrajectoryComboBoxProperty->setIndex(ourProcessor->getSelectedTrajectoryAsInteger());
    allProperties.add (m_oTrajectoryComboBoxProperty);

    
    
    
    //NBR TRAJECTORY TEXT EDITOR
    bool bAllowsMultiLine = false;
    m_oTrajectoryCountTextProperty = new TrajectoryTextComponent (Value ("useless"), "Nbr Trajectories", 8, bAllowsMultiLine, ourProcessor, true);
    m_oTrajectoryCountTextProperty->setText(String(ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoryCount_ParamId)));
    allProperties.add (m_oTrajectoryCountTextProperty);

    //TRAJECTORY DURATION EDITOR
    m_oTrajectoryDurationTextProperty = new TrajectoryTextComponent(Value ("useless"), "Duration", 8, bAllowsMultiLine, ourProcessor, false);
    m_oTrajectoryDurationTextProperty->setText(String(ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoriesDuration_ParamId)));
    allProperties.add (m_oTrajectoryDurationTextProperty);
    
    //SYNC W TEMPO
    m_oTrajectoryTempoButtonProperty = new TrajectoryTempoButtonComponent("Count duration in...", "measures", "seconds", ourProcessor);
    m_oTrajectoryTempoButtonProperty->setState(ourProcessor->getIsSyncWTempo());
    allProperties.add (m_oTrajectoryTempoButtonProperty);
    
    //WRITE TOGGLE BUTTON
    m_oTrajectoryWriteButtonProperty = new TrajectoryWriteButtonComponent("Write Trajectory...", "will write", "will NOT write", ourProcessor);
    m_oTrajectoryWriteButtonProperty->setState(ourProcessor->getIsWriteTrajectory());
    allProperties.add (m_oTrajectoryWriteButtonProperty);
    
    //PREVIEW TOGGLE BUTTON
    m_oTrajectoryPreviewButtonProperty = new TrajectoryPreviewButtonComponent("Preview Trajectory", "Previewing...", "", ourProcessor);
    m_oTrajectoryPreviewButtonProperty->setState(false);
    allProperties.add (m_oTrajectoryPreviewButtonProperty);
    
    m_oPropertyPanel.addSection ("Trajectories", allProperties);
    
//    addAndMakeVisible(_TrajectoryGroup);
//TRAJECTORY COMBO BOX
//    _TrajectoryComboBox.addItem("Upward Spiral",   UpwardSpiral);
//    _TrajectoryComboBox.addItem("Downward Spiral",   DownwardSpiral);
//    _TrajectoryComboBox.addItem("Up and Down Spiral",   UpAndDownSpiral);
//    _TrajectoryComboBox.addItem("Down and Up Spiral",   DownAndUpSpiral);
//    _TrajectoryComboBox.addItem("Pendulum",   Pendulum);
//    _TrajectoryComboBox.addItem("Circle",   Circle);
//    selected_id = ourProcessor->getSelectedTrajectoryAsInteger();
//    _TrajectoryComboBox.setSelectedId(selected_id);
//    _TrajectoryComboBox.addListener(this);
//    addAndMakeVisible(&_TrajectoryComboBox);
    //_TrajectoryComboBox->addListener(this);
    
////NBR TRAJECTORY TEXT EDITOR
//    _TrajectoryCountLabel.setText("Nbr Trajectories",  dontSendNotification);
//    _TrajectoryCountTextEditor.setText(String(ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoryCount_ParamId)));
//    addAndMakeVisible(&_TrajectoryCountLabel);
//    addAndMakeVisible(&_TrajectoryCountTextEditor);
//    _TrajectoryCountTextEditor.addListener(this);
//
////TRAJECTORY DURATION EDITOR
//    _TrajectoryDurationLabel.setText("Duration/Nbr Measures",  dontSendNotification);
//    _TrajectoryDurationTextEditor.setText(String(ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoriesDuration_ParamId)));
//    addAndMakeVisible(&_TrajectoryDurationLabel);
//    addAndMakeVisible(&_TrajectoryDurationTextEditor);
//    _TrajectoryDurationTextEditor.addListener(this);
    
//SYNC W TEMPO TOGGLE BUTTON
//    addAndMakeVisible(&_SyncWTempoButton);
//    _SyncWTempoButton.addListener(this);
//    _SyncWTempoButton.setToggleState(ourProcessor->getIsSyncWTempo(), dontSendNotification);
//    _SyncWTempoButton.addListener(this);
    
//WRITE TRAJECTORY BUTTON
//    addAndMakeVisible(&_WriteTrajectoryButton);
//    _WriteTrajectoryButton.addListener(this);
//    _WriteTrajectoryButton.setClickingTogglesState(true);
//    bool isWriteTrajectory = ourProcessor->getIsWriteTrajectory();
//    _WriteTrajectoryButton.setToggleState(isWriteTrajectory, dontSendNotification);
//    _WriteTrajectoryButton.addListener(this);
    
    
    
    
    
    //SCROLL BAR
    //m_oViewport.setViewedComponent(this);
    
    
    

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
        if (m_bUseIpad){
        _IpadOutgoingOscPortTextEditor.addListener(this);
        _IpadIncomingOscPortTextEditor.addListener(this);
        _IpadIpAddressTextEditor.addListener(this);
    }

    this->setFocusContainer(true);
    
    startTimer (100);
}

ZirkOscjuceAudioProcessorEditor::~ZirkOscjuceAudioProcessorEditor() {
    //stopTimer();
}


void ZirkOscjuceAudioProcessorEditor::resized() {
    int iCurWidth  = getWidth();
    int iCurHeight = getHeight();

    getProcessor()->setLastUiWidth(iCurWidth);
    getProcessor()->setLastUiHeight(iCurHeight);

    _Resizer->setBounds (iCurWidth - 16, iCurHeight - 16, 16, 16);
    
    //if dimensions are smaller than default size, keep components in place, so that with smallest windown, we can only see the circle
    if (iCurHeight < ZirkOSC_Window_Default_Height){
        iCurHeight = ZirkOSC_Window_Default_Height;
    }
    
    if (iCurWidth < ZirkOSC_Window_Default_Width){
        iCurWidth = ZirkOSC_Window_Default_Width;
    }
    
    //------------ LABELS ON RIGHT SIDE ------------
    setLabelAndTextEditorPosition(iCurWidth-80 , 5, 80, 25, &_NbrSourceLabel, &_NbrSourceTextEditor);
    setLabelAndTextEditorPosition(iCurWidth-80 , 55, 80, 25, &_FirstSourceIdLabel, &_FirstSourceIdTextEditor);
    setLabelAndTextEditorPosition(iCurWidth-80 , 105, 80, 25, &_ZkmOscPortLabel, &_ZkmOscPortTextEditor);
    setLabelAndTextEditorPosition(iCurWidth-80 , 155, 80, 25, &_IpadIncomingOscPortLabel, &_IpadIncomingOscPortTextEditor);
    setLabelAndTextEditorPosition(iCurWidth-80 , 205, 80, 25, &_IpadOutgoingOscPortLabel, &_IpadOutgoingOscPortTextEditor);
    setLabelAndTextEditorPosition(iCurWidth-80 , 255, 80, 25, &_IpadIpAddressLabel, &_IpadIpAddressTextEditor);

    // OSC button
    _OscActiveButton.setBounds(iCurWidth-80, 300, 80, 25);
    
    // link button
    //_LinkSpanButton.setBounds(15, iCurHeight-ZirkOSC_TrajectoryGroupHeight-ZirkOSC_SlidersGroupHeight+146, 80, 30);
    _LinkSpanButton.setBounds(iCurWidth-80, 325, 80, 25);

    
    //------------ WALLCIRCLE ------------
    _ZirkOSC_Center_X = (iCurWidth -80)/2;
    _ZirkOSC_Center_Y = (iCurHeight-ZirkOSC_SlidersGroupHeight)/2;
    //assign smallest possible radius
    int iXRadius = (iCurWidth -85)/2;
    int iYRadius = (iCurHeight-ZirkOSC_SlidersGroupHeight-10)/2;
    ZirkOscjuceAudioProcessor::s_iDomeRadius = iXRadius <= iYRadius ? iXRadius: iYRadius;
    
    //------------ CONSTRAINT COMBO BOX ------------
    //_MovementConstraintComboBox.setBounds(100, iCurHeight-ZirkOSC_TrajectoryGroupHeight-ZirkOSC_SlidersGroupHeight+150, 220, 25);
    _MovementConstraintComboBox.setBounds(iCurWidth/2 - 220/2, iCurHeight - ZirkOSC_SlidersGroupHeight - ZirkOSC_ConstraintComboBoxHeight+20, 220, ZirkOSC_ConstraintComboBoxHeight);

    
    
    //------------ TABS ------------
    _TabComponent.setBounds(0, iCurHeight - ZirkOSC_SlidersGroupHeight + ZirkOSC_ConstraintComboBoxHeight, iCurWidth, ZirkOSC_SlidersGroupHeight);
//    _TrajectoryComponent.setBounds(15, iCurHeight-ZirkOSC_TrajectoryGroupHeight-ZirkOSC_SlidersGroupHeight, iCurWidth-30, ZirkOSC_SlidersGroupHeight);
//    _SliderComponent.setBounds(15, iCurHeight-ZirkOSC_TrajectoryGroupHeight-ZirkOSC_SlidersGroupHeight, iCurWidth-30, ZirkOSC_SlidersGroupHeight);

    //------------ LABELS AND SLIDERS ------------
    //setSliderAndLabelPosition(15, iCurHeight-ZirkOSC_TrajectoryGroupHeight-ZirkOSC_SlidersGroupHeight, iCurWidth-40, 20, m_pGainSlider, m_pGainLabel);
    setSliderAndLabelPosition(15, 15, iCurWidth-40, 20, m_pGainSlider, m_pGainLabel);
    setSliderAndLabelPosition(15, 15+30,  iCurWidth-40, 20, m_pAzimuthSlider,       m_pAzimuthLabel);
    setSliderAndLabelPosition(15, 15+60,  iCurWidth-40, 20, m_pElevationSlider,     m_pElevationLabel);
    setSliderAndLabelPosition(15, 15+90,  iCurWidth-40, 20, m_pAzimuthSpanSlider,   m_pAzimuthSpanLabel);
    setSliderAndLabelPosition(15, 15+120, iCurWidth-40, 20, m_pElevationSpanSlider, m_pElevationSpanLabel);

    
    
    // stuff related to trajectories
    //_TrajectoryGroup.setBounds (15, iCurHeight-ZirkOSC_TrajectoryGroupHeight, iCurWidth-30, ZirkOSC_TrajectoryGroupHeight-10);
    //m_oPropertyPanel.setBounds (15, iCurHeight-ZirkOSC_TrajectoryGroupHeight, iCurWidth-30, ZirkOSC_TrajectoryGroupHeight-10);

    //traj combo box
    //_TrajectoryComboBox.setBounds(30, iCurHeight-ZirkOSC_TrajectoryGroupHeight+25, 230, 25);
    //nbr trajectory text editor
    //setLabelAndTextEditorPosition(30, iCurHeight-ZirkOSC_TrajectoryGroupHeight+50, 230, 25, &_TrajectoryDurationLabel, &_TrajectoryDurationTextEditor);
    //trajectory count text editor
    //setLabelAndTextEditorPosition(iCurWidth-150, iCurHeight-ZirkOSC_TrajectoryGroupHeight+5, 125, 25, &_TrajectoryCountLabel, &_TrajectoryCountTextEditor);
    //sync with tempo button
    //_SyncWTempoButton.setBounds(iCurWidth-150, iCurHeight-ZirkOSC_TrajectoryGroupHeight+50, 125, 25);
    //write button
    //_WriteTrajectoryButton.setBounds(iCurWidth-150, iCurHeight-ZirkOSC_TrajectoryGroupHeight+75, 125, 25);
    
    //m_oViewport.visibleAreaChanged(Rectangle<int>(ZirkOSC_Window_Default_Width, ZirkOSC_Window_Default_Height, 16, 16));
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
    label->setBounds (x,    y, 70, height);
    slider->setBounds(x+70, y, width-70,  height);
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
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    int selectedSource = ourProcessor->getSelectedSource();
    float HRAzim = PercentToHR(ourProcessor->getSources()[selectedSource].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max),
    HRElev = PercentToHR(ourProcessor->getSources()[selectedSource].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max),
    HRElevSpan = PercentToHR(ourProcessor->getSources()[selectedSource].getElevationSpan(), ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max),
    HRAzimSpan = PercentToHR(ourProcessor->getSources()[selectedSource].getAzimuthSpan(), ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);

    Point<float> maxElev = {HRAzim, HRElev+HRElevSpan/2};
    Point<float> minElev = {HRAzim, HRElev-HRElevSpan/2};

    if(minElev.getY() < ZirkOSC_ElevSpan_Min){
        maxElev.setY(maxElev.getY()+ ZirkOSC_ElevSpan_Min-minElev.getY());
        minElev.setY(ZirkOSC_ElevSpan_Min);
    }

    Point<float> screenMaxElev = degreeToXy(maxElev);
    Point<float> screenMinElev = degreeToXy(minElev);
    float maxRadius = sqrtf(screenMaxElev.getX()*screenMaxElev.getX() + screenMaxElev.getY()*screenMaxElev.getY());
    float minRadius = sqrtf(screenMinElev.getX()*screenMinElev.getX() + screenMinElev.getY()*screenMinElev.getY());
    //drawing the path for spanning
    Path myPath;
    myPath.startNewSubPath(_ZirkOSC_Center_X+screenMinElev.getX(),_ZirkOSC_Center_Y+screenMinElev.getY());

    //half first arc center
    myPath.addCentredArc(_ZirkOSC_Center_X, _ZirkOSC_Center_Y, minRadius, minRadius, 0.0, degreeToRadian(-HRAzim), degreeToRadian(-HRAzim + HRAzimSpan/2 ));

    if (maxElev.getY()> ZirkOSC_ElevSpan_Max) { // if we are over the top of the dome we draw the adjacent angle

        myPath.addCentredArc(_ZirkOSC_Center_X, _ZirkOSC_Center_Y, maxRadius, maxRadius, 0.0, M_PI+degreeToRadian(-HRAzim + HRAzimSpan/2), M_PI+degreeToRadian(-HRAzim - HRAzimSpan/2 ));
    }
    else{
        myPath.addCentredArc(_ZirkOSC_Center_X, _ZirkOSC_Center_Y, maxRadius, maxRadius, 0.0, degreeToRadian(-HRAzim+HRAzimSpan/2), degreeToRadian(-HRAzim-HRAzimSpan/2 ));
    }
    myPath.addCentredArc(_ZirkOSC_Center_X, _ZirkOSC_Center_Y, minRadius, minRadius, 0.0, degreeToRadian(-HRAzim-HRAzimSpan/2), degreeToRadian(-HRAzim ));
    myPath.closeSubPath();
    g.setColour(Colours::lightgrey);
    g.fillPath(myPath);
    g.setColour(Colours::black);
    
    //PathStrokeType strokeType = PathStrokeType( 1.0, juce::PathStrokeType::JointStyle::curved);
    PathStrokeType strokeType = PathStrokeType( 1.0);
    g.strokePath(myPath, strokeType);

    //g.strokePath(myPath, PathStrokeType::JointStyle::curved);

}

void ZirkOscjuceAudioProcessorEditor::paintSourcePoint (Graphics& g){
    Point<float> sourcePositionOnScreen;
    float HRAzim, HRElev;
    int iXOffset = 0, iYOffset = 0;
    for (int i=0; i<getProcessor()->getNbrSources(); ++i) {
        HRAzim = PercentToHR(getProcessor()->getSources()[i].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
        HRElev = PercentToHR(getProcessor()->getSources()[i].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
        sourcePositionOnScreen = degreeToXy(Point<float> (HRAzim, HRElev));
        
        //draw source circle
        g.drawEllipse(_ZirkOSC_Center_X + sourcePositionOnScreen.getX()-4, _ZirkOSC_Center_Y + sourcePositionOnScreen.getY()-4, 8, 8,2);
        
        if (sourcePositionOnScreen.getX() > ZirkOscjuceAudioProcessor::s_iDomeRadius - 25 ){
            iXOffset = -20;
            iYOffset = 10;
        } else {
            iXOffset = 4;
            iYOffset = -2;
        }
        
        //draw source labels
        if(!_isSourceBeingDragged){
             g.drawText(String(getProcessor()->getSources()[i].getChannel()), _ZirkOSC_Center_X + sourcePositionOnScreen.getX()+iXOffset, _ZirkOSC_Center_Y + sourcePositionOnScreen.getY()+iYOffset, 25, 10, Justification::centred, false);
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
    ZirkOscjuceAudioProcessor *ourProcessor = getProcessor();
    int selectedSource = ourProcessor->getSelectedSource();
    g.setColour(Colours::red);
    float HRAzim = PercentToHR(ourProcessor->getSources()[selectedSource].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    float HRElev = PercentToHR(ourProcessor->getSources()[selectedSource].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    //float HRAzim = (float) _AzimuthSlider.getValue();
    //float HRElev  = (float) _ElevationSlider.getValue();
    Point <float> screen = degreeToXy(Point<float>(HRAzim,HRElev));
    g.drawLine(_ZirkOSC_Center_X, _ZirkOSC_Center_Y, _ZirkOSC_Center_X + screen.getX(), _ZirkOSC_Center_Y + screen.getY() );
}

void ZirkOscjuceAudioProcessorEditor::paintZenithCircle (Graphics& g){
    ZirkOscjuceAudioProcessor *ourProcessor = getProcessor();
    int selectedSource = ourProcessor->getSelectedSource();
    float HRAzim = PercentToHR(ourProcessor->getSources()[selectedSource].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    float HRElev = PercentToHR(ourProcessor->getSources()[selectedSource].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    Point <float> screen = degreeToXy(Point<float>(HRAzim,HRElev));
    float radiusZenith = sqrtf(screen.getX()*screen.getX() + screen.getY()*screen.getY());
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
* \param p : Point <float> (Azimuth,Elevation) in degree
*/
Point <float> ZirkOscjuceAudioProcessorEditor::degreeToXy (Point <float> p){
    float x,y;
    x = -ZirkOscjuceAudioProcessor::s_iDomeRadius * sinf(degreeToRadian(p.getX())) * cosf(degreeToRadian(p.getY()));
    y = -ZirkOscjuceAudioProcessor::s_iDomeRadius * cosf(degreeToRadian(p.getX())) * cosf(degreeToRadian(p.getY()));
    return Point <float> (x, y);
}

/*!
 * \param p : Point <float> (Azimuth,Elevation) in degree
 */
Point <float> ZirkOscjuceAudioProcessorEditor::xyToDegree (Point <float> p){
    float x,y;
    x = -ZirkOscjuceAudioProcessor::s_iDomeRadius * sinf(degreeToRadian(p.getX())) * cosf(degreeToRadian(p.getY()));
    y = -ZirkOscjuceAudioProcessor::s_iDomeRadius * cosf(degreeToRadian(p.getX())) * cosf(degreeToRadian(p.getY()));
    return Point <float> (x, y);
}

/*!
 * \param degree : degree value.
 * \return radian value
 */

inline float ZirkOscjuceAudioProcessorEditor::degreeToRadian (float degree){
    return ((degree/360.0f)*2*3.1415);
}

/*!
 * \param radian : radian value.
 * \return degree value
 */
inline float ZirkOscjuceAudioProcessorEditor::radianToDegree(float radian){
    return (radian/(2*3.1415)*360.0f);
}

/*!
 * Function called every 50ms to refresh value from Host
 * repaint only if the user is not moving any source (_isSourceBeingDragged)
 */
void ZirkOscjuceAudioProcessorEditor::timerCallback(){
    
#if defined(DEBUG)
    clock_t begin = clock();
#endif
    
    //get ref to our processor, WHY DO THAT EVERYTIME?
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();

#if defined(DEBUG)
    clock_t proc = clock();
#endif
    //ref to currently selected source
    int selectedSource = ourProcessor->getSelectedSource();
    
    //based on selected source, update all sliders
    m_pGainSlider->setValue (ourProcessor->getSources()[selectedSource].getGain(), dontSendNotification);

    float HRValue = PercentToHR(ourProcessor->getSources()[selectedSource].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    m_pAzimuthSlider->setValue(HRValue,dontSendNotification);

    HRValue = PercentToHR(ourProcessor->getSources()[selectedSource].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    m_pElevationSlider->setValue(HRValue,dontSendNotification);

    HRValue = PercentToHR(ourProcessor->getSources()[selectedSource].getAzimuthSpan(), ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);
    m_pAzimuthSpanSlider->setValue(HRValue,dontSendNotification);

    HRValue = PercentToHR(ourProcessor->getSources()[selectedSource].getElevationSpan(), ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max);
    m_pElevationSpanSlider->setValue(HRValue,dontSendNotification);

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
    
#if defined(DEBUG)
    clock_t end = clock();
    cout << "processor:\t" << proc - begin <<"ms"<< endl;
    cout << "sliders:\t" << sliders - proc <<"ms"<< endl;
    cout << "ref gui:\t" << gui - sliders <<"ms"<< endl;
    cout << "repaint:\t" << end - gui <<"ms"<< endl;
    cout << "whole thing:\t" << end - begin <<"ms"<< endl;
#endif
    
}

void ZirkOscjuceAudioProcessorEditor::refreshGui(){
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    _ZkmOscPortTextEditor.setText(String(ourProcessor->getOscPortZirkonium()));
    _NbrSourceTextEditor.setText(String(ourProcessor->getNbrSources()));
    _FirstSourceIdTextEditor.setText(String(ourProcessor->getSources()[0].getChannel()));
    _MovementConstraintComboBox.setSelectedId(ourProcessor->getSelectedMovementConstraintAsInteger());
    _OscActiveButton.setToggleState(ourProcessor->getIsOscActive(), dontSendNotification);
    _LinkSpanButton.setToggleState(ourProcessor->getIsSpanLinked(), dontSendNotification);

//    _TrajectoryComboBox.setSelectedId(ourProcessor->getSelectedTrajectoryAsInteger());
//    _SyncWTempoButton.setToggleState(ourProcessor->getIsSyncWTempo(), dontSendNotification);
//    _WriteTrajectoryButton.setToggleState(ourProcessor->getIsWriteTrajectory(), dontSendNotification);
//    _TrajectoryCountTextEditor.setText(String(ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoryCount_ParamId)));
//    _TrajectoryDurationTextEditor.setText(String(ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoriesDuration_ParamId)));
    
    m_oTrajectoryComboBoxProperty->refresh();
    m_oTrajectoryCountTextProperty->refresh();
    m_oTrajectoryTempoButtonProperty->refresh();
    m_oTrajectoryDurationTextProperty->refresh();
    m_oTrajectoryWriteButtonProperty->refresh();
    m_oTrajectoryPreviewButtonProperty->refresh();

    
    _IpadIncomingOscPortTextEditor.setText(ourProcessor->getOscPortIpadIncoming());
    _IpadOutgoingOscPortTextEditor.setText(ourProcessor->getOscPortIpadOutgoing());
    _IpadIpAddressTextEditor.setText(ourProcessor->getOscAddressIpad());

}

void ZirkOscjuceAudioProcessorEditor::buttonClicked (Button* button){
    
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    
    if(button == &_LinkSpanButton){
        ourProcessor->setIsSpanLinked(_LinkSpanButton.getToggleState());
    }
    else if(button == &_OscActiveButton){
        ourProcessor->setIsOscActive(_OscActiveButton.getToggleState());
    }
//    else if(button == &_WriteTrajectoryButton){
//        
//        //set isWriteTrajectory property in processor, only if not currently playing
//        if (!ourProcessor->isCurrentlyPlaying()){
//            bool isWriteTrajectory = _WriteTrajectoryButton.getToggleState();
//            ourProcessor->setIsWriteTrajectory(isWriteTrajectory);
//        } else {
//            _WriteTrajectoryButton.setToggleState(ourProcessor->getIsWriteTrajectory(), dontSendNotification);
//        }
//
//    }
//    else if(button == &_SyncWTempoButton){
//        ourProcessor->setIsSyncWTempo(_SyncWTempoButton.getToggleState());
//    }
}



void ZirkOscjuceAudioProcessorEditor::sliderDragStarted (Slider* slider) {
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    int selectedSource = ourProcessor->getSelectedSource();                             //get selected source
    int selectedConstraint = ourProcessor->getSelectedMovementConstraintAsInteger();    //get selected movement constraint
    bool isSpanLinked = ourProcessor->getIsSpanLinked();
    
    if (slider == m_pGainSlider) {
        ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Gain_ParamId + (selectedSource*5) );
    }
    else if (slider == m_pAzimuthSlider) {
        if (selectedConstraint == Independant){
            ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + (selectedSource*5));
        } else {
            for(int i = 0;i<getProcessor()->getNbrSources(); ++i){
                ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + i*5);
            }
        }
    }
    else if (slider == m_pElevationSlider) {
        if (selectedConstraint == Independant){
            ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + (selectedSource*5));
        } else {
            for(int i = 0;i<getProcessor()->getNbrSources(); ++i){
                ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + i*5);
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
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    int selectedSource = ourProcessor->getSelectedSource();                             //get selected source
    int selectedConstraint = ourProcessor->getSelectedMovementConstraintAsInteger();    //get selected movement constraint
    bool isSpanLinked = ourProcessor->getIsSpanLinked();
    
    if (slider == m_pGainSlider) {
        ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Gain_ParamId + (selectedSource*5) );
    }
    else if (slider == m_pAzimuthSlider) {
        if (selectedConstraint == Independant){
            ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + (selectedSource*5));
        } else {
            for(int i = 0;i<getProcessor()->getNbrSources(); ++i){
                ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + i*5);
            }
        }
    }
    else if (slider == m_pElevationSlider) {
        if (selectedConstraint == Independant){
            ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + (selectedSource*5));
        } else {
            for(int i = 0;i<getProcessor()->getNbrSources(); ++i){
                ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + i*5);
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
    //get processor and selected source
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    int selectedSource = ourProcessor->getSelectedSource();
    bool isSpanLinked = ourProcessor->getIsSpanLinked();
    float percentValue=0;
    SoundSource curLocationSource = ourProcessor->getSources()[selectedSource];
    Point<float> newLocation;
    
    if (slider == m_pGainSlider) {
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Gain_ParamId + (selectedSource*5), (float) m_pGainSlider->getValue());
    }
    
    else if (slider == m_pAzimuthSlider || slider == m_pElevationSlider) {

        //get selected movement constraint
        int selectedConstraint = ourProcessor->getSelectedMovementConstraintAsInteger();
        
        if (slider == m_pAzimuthSlider ){
            //figure out where the slider should move the point
            percentValue = HRToPercent((float) m_pAzimuthSlider->getValue(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
            if (selectedConstraint == Independant){
                ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + (selectedSource*5), percentValue);
                return;
            }
            //if we get here, we're not in independent mode
            else {
                //calculate new location point using current location and new value of the slider
                SoundSource newLocationSource(percentValue, curLocationSource.getElevation());
                newLocation = newLocationSource.getPositionXY();
            }
        } else {
            percentValue = HRToPercent((float) m_pElevationSlider->getValue(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
            if (selectedConstraint == Independant){
                ourProcessor->setParameterNotifyingHost  (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + (selectedSource * 5), percentValue);
                return;
            }
            //if we get here, we're not in independent mode
            else {
                //calculate new location point using current location and new value of the slider
                SoundSource newLocationSource(curLocationSource.getAzimuth(), percentValue);
                newLocation = newLocationSource.getPositionXY();
            }
        }
        
        //if we get here, we're not in independent mode
        if (selectedConstraint == FixedAngles){
            moveFixedAngles(newLocation);
        }
        else if (selectedConstraint == FixedRadius){
            moveCircularWithFixedRadius(newLocation);
        }
        else if (selectedConstraint == FullyFixed){
            moveFullyFixed(newLocation);
        }
        else if (selectedConstraint == DeltaLocked){
            Point<float> DeltaMove = newLocation - ourProcessor->getSources()[selectedSource].getPositionXY();
            moveSourcesWithDelta(DeltaMove);
        }
        else if (selectedConstraint == Circular){
            moveCircular(newLocation);
        }
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
    int source=-1;

    //if event is within the wall circle, select source that is clicked on (if any)
    if (event.x>5 && event.x <20+ZirkOscjuceAudioProcessor::s_iDomeRadius*2 && event.y>5 && event.y< 40+ZirkOscjuceAudioProcessor::s_iDomeRadius*2) {
        source=getSourceFromPosition(Point<float>(event.x-_ZirkOSC_Center_X, event.y-_ZirkOSC_Center_Y));

    }
    
    //if a source is clicked on, flag _isSourceBeingDragged to true
    _isSourceBeingDragged = (source!=-1);
    if(_isSourceBeingDragged){

        //if sources are being dragged, tell host that their parameters are about to change (beginParameterChangeGesture). Logic needs this
        getProcessor()->setSelectedSource(source);
        int selectedConstraint = getProcessor()->getSelectedMovementConstraintAsInteger();
        if (selectedConstraint == Independant) {
            getProcessor()->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + source*5);
            getProcessor()->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + source*5);
        }
        else {
            for(int i = 0;i<getProcessor()->getNbrSources(); ++i){
                getProcessor()->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + i*5);
                getProcessor()->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + i*5);
                getProcessor()->getSources()[i].setAzimReverse(false);
            }
        }
        //repaint();
    }
    m_pGainSlider->grabKeyboardFocus();
}

int ZirkOscjuceAudioProcessorEditor::getSourceFromPosition(Point<float> p ){
    for (int i=0; i<getProcessor()->getNbrSources() ; ++i){
        if (getProcessor()->getSources()[i].contains(p)){
            return i;
        }
    }
    return -1;
}


void ZirkOscjuceAudioProcessorEditor::mouseDrag (const MouseEvent &event){
    if(_isSourceBeingDragged){
        
        //get processor and selected source
        ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
        int selectedSource = ourProcessor->getSelectedSource();
        
        //get point of current event
        Point<float> pointRelativeCenter = Point<float>(event.x-_ZirkOSC_Center_X, event.y-_ZirkOSC_Center_Y);
        
        //get current mouvement constraint
        int selectedConstraint = ourProcessor->getSelectedMovementConstraintAsInteger();
        if (selectedConstraint == Independant) {
            //set source position to current event point
            ourProcessor->getSources()[selectedSource].setPositionXY(pointRelativeCenter);
            //notify the host+processor of the source position
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + selectedSource*5, ourProcessor->getSources()[selectedSource].getAzimuth());
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + selectedSource*5, ourProcessor->getSources()[selectedSource].getElevation());
            //send source position by osc
            ourProcessor->sendOSCValues();
        }
        
        else if (selectedConstraint == FixedAngles){
             moveFixedAngles(pointRelativeCenter);
        }
        else if (selectedConstraint == FixedRadius){
            moveCircularWithFixedRadius(pointRelativeCenter);
        }
        else if (selectedConstraint == FullyFixed){
            moveFullyFixed(pointRelativeCenter);
        }
        else if (selectedConstraint == DeltaLocked){
            Point<float> DeltaMove = pointRelativeCenter - ourProcessor->getSources()[selectedSource].getPositionXY();
            moveSourcesWithDelta(DeltaMove);
        }
        else if (selectedConstraint == Circular){
            moveCircular(pointRelativeCenter);
        }
        //repaint();
    }
    getProcessor()->sendOSCValues();
    m_pGainSlider->grabKeyboardFocus();
}

void ZirkOscjuceAudioProcessorEditor::mouseUp (const MouseEvent &event){
    if(_isSourceBeingDragged){
        int selectedConstrain = getProcessor()->getSelectedMovementConstraintAsInteger();
        if(selectedConstrain == Independant){
            int selectedSource = getProcessor()->getSelectedSource();
            getProcessor()->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId+ selectedSource*5);
            getProcessor()->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + selectedSource*5);
            
        }
        else {
            for(int i = 0;i<getProcessor()->getNbrSources(); ++i){
                getProcessor()->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + i*5);
                getProcessor()->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + i*5);
            }
        }
        _isSourceBeingDragged=false;
    }
    m_pGainSlider->grabKeyboardFocus();
}

void ZirkOscjuceAudioProcessorEditor::moveFixedAngles(Point<float> p){
    if (_isNeedToSetFixedAngles){
        orderSourcesByAngle(getProcessor()->getSelectedSource(),getProcessor()->getSources());
        _isNeedToSetFixedAngles=false;
    }
    moveCircular(p);
}

void ZirkOscjuceAudioProcessorEditor::moveFullyFixed(Point<float> p){
    if (_isNeedToSetFixedAngles){
        orderSourcesByAngle(getProcessor()->getSelectedSource(),getProcessor()->getSources());
        _isNeedToSetFixedAngles=false;
    }
    moveCircularWithFixedRadius(p);
}

void ZirkOscjuceAudioProcessorEditor::orderSourcesByAngle (int selected, SoundSource tab[]){
    int nbrSources = getProcessor()->getNbrSources();
    int* order = getOrderSources(selected, tab, nbrSources);
    int count = 0;
    for(int i= 1; i < nbrSources ; ++i){ //for(int i= 1; i != nbrSources ; ++i){
        getProcessor()->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + (order[i]*5), tab[order[0]].getAzimuth()+ (float)(++count)/(float) nbrSources);
    }
}
//starting from the selected source, cycle through the other sources to find in which order they are
int* ZirkOscjuceAudioProcessorEditor::getOrderSources(int selected, SoundSource tab [], int nbrSources){
    int * order = new int [nbrSources];
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

void ZirkOscjuceAudioProcessorEditor::moveCircular(Point<float> pointRelativeCenter){
    moveCircular(pointRelativeCenter, false);
    
}

void ZirkOscjuceAudioProcessorEditor::moveCircularWithFixedRadius(Point<float> pointRelativeCenter){
    moveCircular(pointRelativeCenter, true);
}

void ZirkOscjuceAudioProcessorEditor::moveCircular(Point<float> pointRelativeCenter, bool isRadiusFixed){

    //get processor and selected source
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    int selectedSource = ourProcessor->getSelectedSource();
    
    //figure out new location that source should move to
    SoundSource sourceNewLocation = SoundSource();
    sourceNewLocation.setPositionXY(pointRelativeCenter);
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
        float curSourceElev = ourProcessor->getSources()[iCurSource].getElevationRawValue();
        
        //set new azimuth through host
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + iCurSource * 5, curSourceAzim + deltaX);

        //if radius is fixed, set all elevation to the same thing
        if (isRadiusFixed){
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + iCurSource * 5, (currentElevation + HRToPercent(deltaCircularMove.getY(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max)));
        }
        //if radius is not fixed, set all elevation to be currrent elevation +/- deltaY
        else {
            //if azimuth is NOT reversed, ie, NOT on the other side of the dome's middle point
            if(!ourProcessor->getSources()[iCurSource].isAzimReverse()){
                ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + iCurSource * 5, curSourceElev + deltaY);
            } else {
                ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + iCurSource * 5, curSourceElev - deltaY);
            }
        }
    }
}

void ZirkOscjuceAudioProcessorEditor::moveSourcesWithDelta(Point<float> DeltaMove){
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    int nbrSources = ourProcessor->getNbrSources();
    Point<float> currentPosition;
    bool inTheDome = true;
    for (int i =0; i<ourProcessor->getNbrSources()&&inTheDome; ++i) {
         inTheDome=ourProcessor->getSources()[i].isStillInTheDome(DeltaMove);
    }
    if (inTheDome){
        for(int i=0;i<nbrSources;++i){
            currentPosition = ourProcessor->getSources()[i].getPositionXY();
            ourProcessor->getSources()[i].setPositionXY(currentPosition + DeltaMove);
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + i*5,
                                                     ourProcessor->getSources()[i].getAzimuth());
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + i*5,
                                                     ourProcessor->getSources()[i].getElevationRawValue());
            //azimuthLabel.setText(String(ourProcessor->tabSource[i].getElevation()), false);
            ourProcessor->sendOSCValues();
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
    
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();

    if(&_NbrSourceTextEditor == &textEditor) {
        //if we have a valid number of sources, set it in processor
        if(intValue >=1 && intValue <= 8){
            ourProcessor->setNbrSources(intValue);
            
            //need to give those new sources IDs, so get first source ID
            int sourceId = ourProcessor->getSources()[0].getChannel();
            
            //then set all subsequent source IDs to subsequent numbers
            for (int iCurSource = 1; iCurSource < 8; ++iCurSource){
                ourProcessor->getSources()[iCurSource].setChannel(++sourceId);
            }
            
            //toggle fixed angle repositioning, if we need to
            int selectedConstraint = ourProcessor->getSelectedMovementConstraintAsInteger();
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
        
        int newChannel = intValue;
    
        //set the ID of the first source to intValue, then set all subsequent source IDs to subsequent numbers
        for (int iCurSource = 0; iCurSource < 8; ++iCurSource){
            ourProcessor->getSources()[iCurSource].setChannel(newChannel++);
        }
       ourProcessor->sendOSCValues();
    }
    
    else if(&_ZkmOscPortTextEditor == &textEditor ){
        int newPort = intValue;
        ourProcessor->changeZirkoniumOSCPort(newPort);
        _ZkmOscPortTextEditor.setText(String(ourProcessor->getOscPortZirkonium()));
    }
    
//    else if(&_TrajectoryCountTextEditor == &textEditor ){
//        double doubleValue = textEditor.getText().getDoubleValue();
//        if ((doubleValue > 0 && doubleValue < 10000) || (doubleValue < 0 && doubleValue > -10000)){
//            ourProcessor->setParameterNotifyingHost(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoryCount_ParamId, doubleValue);
//        }
//        _TrajectoryCountTextEditor.setText(String(ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoryCount_ParamId)));
//    }
    
//    else if(&_TrajectoryDurationTextEditor == &textEditor){
//        double doubleValue = textEditor.getText().getDoubleValue();
//        if (doubleValue > 0 && doubleValue < 10000){
//            ourProcessor->setParameterNotifyingHost(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoriesDuration_ParamId, doubleValue);
//        }
//        _TrajectoryDurationTextEditor.setText(String(ourProcessor->getParameter(ZirkOscjuceAudioProcessor::ZirkOSC_TrajectoriesDuration_ParamId)));
//    }
    
    else if (&_IpadOutgoingOscPortTextEditor == &textEditor) {
        int newIpadOutgoingPort = intValue;
        String curIpAddress = ourProcessor->getOscAddressIpad();
        
        ourProcessor->changeOSCSendIPad(newIpadOutgoingPort, curIpAddress);
        
        _IpadOutgoingOscPortTextEditor.setText(ourProcessor->getOscPortIpadOutgoing());
    }
    
    else if (&_IpadIpAddressTextEditor == &textEditor) {
        String newIpAddress = text;
        int curOscOutgoingPort = ourProcessor->getOscPortIpadOutgoing().getIntValue();
        
        ourProcessor->changeOSCSendIPad(curOscOutgoingPort, newIpAddress);
        
        _IpadIpAddressTextEditor.setText(ourProcessor->getOscAddressIpad());
    }
    
    else if (&_IpadIncomingOscPortTextEditor == &textEditor) {
        int newIpadIncomingPort = intValue;
        ourProcessor->changeOSCReceiveIpad(newIpadIncomingPort);
        _IpadIncomingOscPortTextEditor.setText(ourProcessor->getOscPortIpadIncoming());
        
    }
    ourProcessor->sendOSCConfig();
    ourProcessor->sendOSCValues();
    ourProcessor->sendOSCMovementType();
    if (!_isReturnKeyPressedCalledFromFocusLost){
        m_pGainSlider->grabKeyboardFocus();
    }
}



void ZirkOscjuceAudioProcessorEditor::comboBoxChanged (ComboBox* comboBoxThatHasChanged){
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    
//    if (comboBoxThatHasChanged == &_TrajectoryComboBox){
//        ourProcessor->setParameterNotifyingHost(ZirkOscjuceAudioProcessor::ZirkOSC_SelectedTrajectory_ParamId,
//                                                IntToPercent(comboBoxThatHasChanged->getSelectedId(), TotalNumberTrajectories));
//    } else
        if (comboBoxThatHasChanged == &_MovementConstraintComboBox){

            
        ourProcessor->setParameterNotifyingHost(ZirkOscjuceAudioProcessor::ZirkOSC_MovementConstraint_ParamId,
                                                IntToPercentStartsAtOne(comboBoxThatHasChanged->getSelectedId(), TotalNumberConstraints));

        int selectedConstraint = comboBoxThatHasChanged->getSelectedId();
        if( selectedConstraint == FixedAngles || selectedConstraint == FullyFixed){
            _isNeedToSetFixedAngles=true;
        }
        ourProcessor->sendOSCMovementType();
        //m_pGainSlider->grabKeyboardFocus();
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





