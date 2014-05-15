/*
 ==============================================================================

 Developer : Ludovic LAFFINEUR (ludovic.laffineur@gmail.com)
 Copyright 2013 Ludovic LAFFINEUR ludovic.laffineur@gmail.com
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
#include <cstdlib>
#include <string>
#include <string.h>
#include <sstream>
#include <istream>
#include <math.h>
#include <ctime>

using namespace std;


/*!
*  \param ownerFilter : the processor processor
*/

ZirkOscjuceAudioProcessorEditor::ZirkOscjuceAudioProcessorEditor (ZirkOscjuceAudioProcessor* ownerFilter)
:   AudioProcessorEditor (ownerFilter),

_LinkSpanButton("Link Span"),
_AzimuthSlider(ZirkOSC_AzimSpan_name[0]),
_ElevationSlider(ZirkOSC_ElevSpan_name[0]),
_ElevationSpanSlider(ZirkOSC_ElevSpan_name[0]),
_GainSlider (ZirkOSC_Gain_name[0]),
_AzimuthLabel(ZirkOSC_Azim_name[0]),
_AzimuthSpanLabel(ZirkOSC_AzimSpan_name[0]),
_ElevationLabel(ZirkOSC_Elev_name[0]),
_GainLabel(ZirkOSC_Gain_name[0]),
//strings in parameters are all used as component names
_ChannelNumberLabel("channelNbr"),
_OscPortLabel("OscPort"),
_OscPortOutgoingIPadLabel("OSCPortOutgoingIPad"),
_OscPortIncomingIPadLabel("OSCIpadInco"),
_NbrSourceLabel("NbrSources"),
_OscAdressIPadTextLabel("ipadadressLabel"),
_OscPortTextEditor("OscPort"),
_NbrSourceTextEditor("NbrSource"),
_FirstSourceIdTextEditor("channelNbr"),
_OscPortOutgoingIPadTextEditor("OSCPortOutgoingIPadTE"),
_OscAdressIPadTextEditor("ipaddress"),
_OscPortIncomingIPadTextEditor("OSCIpadIncoTE"),
_MovementConstraintComboBox("MovementConstraint")
{
    
    //radius of the wall circle displaying the sources
    _ZirkOSC_Center_X = ZirkOSC_Window_Default_Width-240;
    _ZirkOSC_Center_Y = ZirkOSC_Window_Default_Height-420;
    
    //---------- SLIDERS ----------
    setSliderAndLabel(20, ZirkOSC_Window_Default_Height-260, ZirkOSC_Window_Default_Width-40, 20, "Gain", &_GainSlider,&_GainLabel, ZirkOSC_Gain_Min, ZirkOSC_Gain_Max);
    addAndMakeVisible(&_GainSlider);
    addAndMakeVisible(&_GainLabel);

    setSliderAndLabel(20, ZirkOSC_Window_Default_Height-220, ZirkOSC_Window_Default_Width-40, 20, "Azimuth", &_AzimuthSlider ,&_AzimuthLabel, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    addAndMakeVisible(&_AzimuthSlider);
    addAndMakeVisible(&_AzimuthLabel);

    setSliderAndLabel(20, ZirkOSC_Window_Default_Height-180, ZirkOSC_Window_Default_Width-40, 20, "Elevation", &_ElevationSlider, &_ElevationLabel, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    addAndMakeVisible(&_ElevationSlider);
    addAndMakeVisible(&_ElevationLabel);

    setSliderAndLabel(20, ZirkOSC_Window_Default_Height-140, ZirkOSC_Window_Default_Width-40, 20, "Elev. Sp.", &_ElevationSpanSlider, &_ElevationSpanLabel, ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max);
    addAndMakeVisible(&_ElevationSpanSlider);
    addAndMakeVisible(&_ElevationSpanLabel);

    setSliderAndLabel(20, ZirkOSC_Window_Default_Height-100, ZirkOSC_Window_Default_Width-40, 20, "Azim. Sp.", &_AzimuthSpanSlider, &_AzimuthSpanLabel, ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);
    addAndMakeVisible(&_AzimuthSpanSlider);
    addAndMakeVisible(&_AzimuthSpanLabel);
    

    //---------- LABELS ----------
    _NbrSourceLabel.setText("Nbr Sources",  dontSendNotification);
    _NbrSourceTextEditor.setText(String(getProcessor()->getNbrSources()));
    addAndMakeVisible(&_NbrSourceLabel);
    addAndMakeVisible(&_NbrSourceTextEditor);

    _ChannelNumberLabel.setText("1st source ID",  dontSendNotification);
    _FirstSourceIdTextEditor.setText(String(getProcessor()->getSources()[0].getChannel()));
    addAndMakeVisible(&_ChannelNumberLabel);
    addAndMakeVisible(&_FirstSourceIdTextEditor);
    
    _OscPortLabel.setText("ZKM OSC Port",  dontSendNotification);
    _OscPortTextEditor.setText(String(getProcessor()->getOscPortZirkonium()));
    addAndMakeVisible(&_OscPortLabel);
    addAndMakeVisible(&_OscPortTextEditor);

    _OscPortIncomingIPadLabel.setText("Inc. port",  dontSendNotification);
    _OscPortIncomingIPadTextEditor.setText(String(getProcessor()->getOscPortIpadIncoming()));
    addAndMakeVisible(&_OscPortIncomingIPadLabel);
    addAndMakeVisible(&_OscPortIncomingIPadTextEditor);

    _OscPortOutgoingIPadLabel.setText("Out. port",  dontSendNotification);
    _OscPortOutgoingIPadTextEditor.setText(String(getProcessor()->getOscPortIpadOutgoing()));
    addAndMakeVisible(&_OscPortOutgoingIPadLabel);
    addAndMakeVisible(&_OscPortOutgoingIPadTextEditor);

    _OscAdressIPadTextLabel.setText("iPad IP add.",  dontSendNotification);
    _OscAdressIPadTextEditor.setText(String(getProcessor()->getOscAddressIpad()));
    addAndMakeVisible(&_OscAdressIPadTextLabel);
    addAndMakeVisible(&_OscAdressIPadTextEditor);
    
    
    //---------- LINK BUTTON ----------
    addAndMakeVisible(&_LinkSpanButton);
    _LinkSpanButton.addListener(this);
    
    
    //---------- COMBO BOX ----------
    _MovementConstraintComboBox.setSize(50, 20);
    _MovementConstraintComboBox.addItem("Independant",   Independant);
    _MovementConstraintComboBox.addItem("Circular",      Circular);
    _MovementConstraintComboBox.addItem("Equal Elevation",  FixedRadius);
    _MovementConstraintComboBox.addItem("Equal Azimuth",   FixedAngles);
    _MovementConstraintComboBox.addItem("Equal Azim+Elev",   FullyFixed);
    _MovementConstraintComboBox.addItem("Delta Lock",    DeltaLocked);
    int selected_id = getProcessor()->getSelectedMovementConstraintAsInteger();
    _MovementConstraintComboBox.setSelectedId(selected_id);
    _MovementConstraintComboBox.addListener(this);
    addAndMakeVisible(&_MovementConstraintComboBox);


    //---------- RESIZABLE CORNER ----------
    // add the triangular resizer component for the bottom-right of the UI
    addAndMakeVisible (_Resizer = new ResizableCornerComponent (this, &_ResizeLimits));
    //min dimensions are wallCircle radius (300) + offset in display (10,30) + padding (10)
    //_ResizeLimits.setSizeLimits (320, 340, ZirkOSC_Window_Default_Width, ZirkOSC_Window_Default_Height);
    _ResizeLimits.setSizeLimits (ZirkOSC_Window_Default_Width, ZirkOSC_Window_Default_Height, 2*ZirkOSC_Window_Default_Width, 2*ZirkOSC_Window_Default_Height);

    
    // set our component's initial size to be the last one that was stored in the filter's settings
    setSize (ownerFilter->getLastUiWidth(), ownerFilter->getLastUiHeight());

    _FirstSourceIdTextEditor.addListener(this);
    _OscPortTextEditor.addListener(this);
    _NbrSourceTextEditor.addListener(this);
    _ElevationSlider.addListener(this);
    _AzimuthSlider.addListener(this);
    _GainSlider.addListener(this);
    _ElevationSpanSlider.addListener(this);
    _AzimuthSpanSlider.addListener(this);
    _OscPortOutgoingIPadTextEditor.addListener(this);
    _OscPortIncomingIPadTextEditor.addListener(this);
    _OscAdressIPadTextEditor.addListener(this);
    this->setFocusContainer(true);
    
    startTimer (100);
}

ZirkOscjuceAudioProcessorEditor::~ZirkOscjuceAudioProcessorEditor()
{
    //stopTimer();
}


//this is called upon opening the window so probably no need to do all that in constructor
void ZirkOscjuceAudioProcessorEditor::resized()
{
    int iCurWidth  = getWidth();
    int iCurHeight = getHeight();
    
    //------------ LABELS ON RIGHT SIDE ------------
    setLabelAndTextEditorPosition(iCurWidth-80 , 10, 80, 25, &_NbrSourceLabel, &_NbrSourceTextEditor);
    setLabelAndTextEditorPosition(iCurWidth-80 , 60, 80, 25, &_ChannelNumberLabel, &_FirstSourceIdTextEditor);
    setLabelAndTextEditorPosition(iCurWidth-80 , 110, 80, 25, &_OscPortLabel, &_OscPortTextEditor);
    setLabelAndTextEditorPosition(iCurWidth-80 , 160, 80, 25, &_OscPortIncomingIPadLabel, &_OscPortIncomingIPadTextEditor);
    setLabelAndTextEditorPosition(iCurWidth-80 , 210, 80, 25, &_OscPortOutgoingIPadLabel, &_OscPortOutgoingIPadTextEditor);
    setLabelAndTextEditorPosition(iCurWidth-80 , 260, 80, 25, &_OscAdressIPadTextLabel, &_OscAdressIPadTextEditor);

    // link button
    _LinkSpanButton.setBounds(iCurWidth-80, 310, 80, 30);
    
    //------------ WALLCIRCLE ------------
    _ZirkOSC_Center_X = (iCurWidth -80)/2;
    _ZirkOSC_Center_Y = (iCurHeight-240)/2;
    //assign smallest possible radius
    int iXRadius = (iCurWidth -100)/2;
    int iYRadius = (iCurHeight -300)/2;
    ZirkOscjuceAudioProcessor::s_iDomeRadius = iXRadius <= iYRadius ? iXRadius: iYRadius;

    //------------ LABELS AND SLIDERS ------------
    setSliderAndLabelPosition(20, iCurHeight-260, iCurWidth-40, 20, &_GainSlider, &_GainLabel);
    setSliderAndLabelPosition(20, iCurHeight-220, iCurWidth-40, 20, &_AzimuthSlider ,&_AzimuthLabel);
    setSliderAndLabelPosition(20, iCurHeight-180, iCurWidth-40, 20, &_ElevationSlider, &_ElevationLabel);
    setSliderAndLabelPosition(20, iCurHeight-140, iCurWidth-40, 20, &_ElevationSpanSlider, &_ElevationSpanLabel);
    setSliderAndLabelPosition(20, iCurHeight-100, iCurWidth-40, 20, &_AzimuthSpanSlider, &_AzimuthSpanLabel);

    //combo box
    _MovementConstraintComboBox.setBounds((iCurWidth-220)/2, iCurHeight-60, 220, 25);
    
    getProcessor()->setLastUiWidth(iCurWidth);
    getProcessor()->setLastUiHeight(iCurHeight);
    
    _Resizer->setBounds (iCurWidth - 16, iCurHeight - 16, 16, 16);
    
}




//Automatic function to set label and Slider

/*!
* \param x : x position top left
* \param y : y position top left
* \param width : slider's and label's width
* \param height : slider's and label's heigth
* \param labelText : label's text
* \param slider : slider
* \param label : label
* \param min : minimum value of the slider
* \param max : maximum value of the slider
*/

void ZirkOscjuceAudioProcessorEditor::setSliderAndLabel(int x, int y, int width, int height, String labelText, Slider* slider, Label* label, float min, float max){
    slider->setBounds (x+60, y, width-60, height);
    slider->setTextBoxStyle(Slider::TextBoxRight, false, 80, 20);
    label->setText(labelText,  dontSendNotification);
    label->setBounds(x, y, width-300, height);
    slider->setRange (min, max, 0.01);
}

void ZirkOscjuceAudioProcessorEditor::setSliderAndLabelPosition(int x, int y, int width, int height, Slider* slider, Label* label){
    label->setBounds (x,    y, 60, height);
    slider->setBounds(x+60, y, width-60,  height);
}

void ZirkOscjuceAudioProcessorEditor::setLabelAndTextEditorPosition(int x, int y, int width, int height, Label* p_label, TextEditor* p_textEditor){
    p_label->setBounds(x, y, width, height);
    p_textEditor->setBounds(x, y+20, width, height);
}


void ZirkOscjuceAudioProcessorEditor::paint (Graphics& g)
{
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
    
    PathStrokeType strokeType = PathStrokeType( 1.0, juce::PathStrokeType::JointStyle::curved);
    g.strokePath(myPath, strokeType);

    //g.strokePath(myPath, PathStrokeType::JointStyle::curved);

}

void ZirkOscjuceAudioProcessorEditor::paintSourcePoint (Graphics& g){
    Point<float> screen;
    float HRAzim, HRElev;
    for (int i=0; i<getProcessor()->getNbrSources(); i++) {
        HRAzim = PercentToHR(getProcessor()->getSources()[i].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
        HRElev = PercentToHR(getProcessor()->getSources()[i].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
        screen = degreeToXy(Point<float> (HRAzim, HRElev));
        g.drawEllipse(_ZirkOSC_Center_X + screen.getX()-4, _ZirkOSC_Center_Y + screen.getY()-4, 8, 8,2);
        if(!_isSourceBeingDragged){
             g.drawText(String(getProcessor()->getSources()[i].getChannel()), _ZirkOSC_Center_X + screen.getX()+6, _ZirkOSC_Center_Y + screen.getY()-2, 40, 10, Justification::centred, false);
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
    for (int i= 0; i<ZirkOSC_NumMarks; i++) {
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
    _GainSlider.setValue (ourProcessor->getSources()[selectedSource].getGain(), dontSendNotification);

    float HRValue = PercentToHR(ourProcessor->getSources()[selectedSource].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    _AzimuthSlider.setValue(HRValue,dontSendNotification);

    HRValue = PercentToHR(ourProcessor->getSources()[selectedSource].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    _ElevationSlider.setValue(HRValue,dontSendNotification);

    HRValue = PercentToHR(ourProcessor->getSources()[selectedSource].getAzimuthSpan(), ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);
    _AzimuthSpanSlider.setValue(HRValue,dontSendNotification);

    HRValue = PercentToHR(ourProcessor->getSources()[selectedSource].getElevationSpan(), ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max);
    _ElevationSpanSlider.setValue(HRValue,dontSendNotification);

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
    
    if(!_isSourceBeingDragged){
          repaint();
    }
    
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
    _OscPortTextEditor.setText(String(ourProcessor->getOscPortZirkonium()));
    _NbrSourceTextEditor.setText(String(ourProcessor->getNbrSources()));
    _FirstSourceIdTextEditor.setText(String(ourProcessor->getSources()[0].getChannel()));
    _OscPortIncomingIPadTextEditor.setText(ourProcessor->getOscPortIpadIncoming());
    _OscPortOutgoingIPadTextEditor.setText(ourProcessor->getOscPortIpadOutgoing());
    _OscAdressIPadTextEditor.setText(ourProcessor->getOscAddressIpad());
    _MovementConstraintComboBox.setSelectedId(ourProcessor->getSelectedMovementConstraintAsInteger());
}

void ZirkOscjuceAudioProcessorEditor::buttonClicked (Button* button)
{
    if(button == &_LinkSpanButton){
        _LinkSpan = _LinkSpanButton.getToggleState();
    }
}

float PercentToHR(float percent, float min, float max){
    return percent*(max-min)+min;
}

float HRToPercent(float HRValue, float min, float max){
    return (HRValue-min)/(max-min);
}

int PercentToInt(float percent, int max){
    return percent * (max-1) + 1;
}

//max here represent the total range of numbers
float IntToPercent(int integer, int max){
    return static_cast<float>((integer-1)) / (max - 1);
}



void ZirkOscjuceAudioProcessorEditor::sliderValueChanged (Slider* slider) {
    //get processor and selected source
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    int selectedSource = ourProcessor->getSelectedSource();
    
    float percentValue=0;
    if (slider == &_GainSlider) {
        ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Gain_ParamId+ (selectedSource*5) );
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Gain_ParamId + (selectedSource*5), (float) _GainSlider.getValue());
        ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Gain_ParamId + (selectedSource*5));
    }
    else if (slider == &_AzimuthSlider) {
        //figure out where the slider should move the point
        percentValue = HRToPercent((float) _AzimuthSlider.getValue(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);

        //get selected movement constraint
        int selectedConstraint = ourProcessor->getSelectedMovementConstraintAsInteger();

        //if no movement constraint
        if (selectedConstraint == Independant){
            ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId+ (selectedSource*5));
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + (selectedSource*5), percentValue);
            ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + (selectedSource*5));
        }
        //else, movement is constrained
        else {
            
            //calculate new location point using current location and new value of the slider
            SoundSource curLocationSource = ourProcessor->getSources()[selectedSource];
            SoundSource newLocationSource(percentValue, curLocationSource.getElevation());
            Point<float> newLocation = newLocationSource.getPositionXY();
            
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
            repaint();
        }
    }
    else if (slider == &_ElevationSlider) {
        
        percentValue = HRToPercent((float) _ElevationSlider.getValue(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
       
        //if no movement constraint
        int selectedConstraint = ourProcessor->getSelectedMovementConstraintAsInteger();
        if (selectedConstraint == Independant){
            ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + (selectedSource * 5));
            ourProcessor->setParameterNotifyingHost  (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + (selectedSource * 5), percentValue);
            ourProcessor->endParameterChangeGesture  (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + (selectedSource * 5));
        }
        //else, movement is constrained
        else {

            //calculate new location point using current location and new value of the slider
            SoundSource curLocationSource = ourProcessor->getSources()[selectedSource];
            SoundSource newLocationSource(curLocationSource.getAzimuth(), percentValue);
            Point<float> newLocation = newLocationSource.getPositionXY();
            
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
            repaint();
        }
    }
    
    
    
    else if (slider == &_ElevationSpanSlider) {
        percentValue = HRToPercent((float) _ElevationSpanSlider.getValue(), ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max);
        if(_LinkSpan){
            for(int i=0 ; i<ourProcessor->getNbrSources(); i++){
                ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + (i*5));
                ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + (i*5), percentValue);
                ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId +(i*5));
            }
        }
        else{
            ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + (selectedSource*5));
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + (selectedSource*5), percentValue);
            ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId +(selectedSource*5));
        }

    }
    else if (slider == &_AzimuthSpanSlider) {
        percentValue = HRToPercent((float) _AzimuthSpanSlider.getValue(), ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);
        if(_LinkSpan){
            for(int i=0 ; i<ourProcessor->getNbrSources(); i++){
                ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + (i*5));
                ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + (i*5), percentValue);
                ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId +(i*5));
            }
        }
        else{
            ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + (selectedSource*5));
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + (selectedSource*5), percentValue);
            ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + (selectedSource*5));
        }
    }else;
    //repaint();
}

void ZirkOscjuceAudioProcessorEditor::mouseDown (const MouseEvent &event){
    int source=-1;

    //if event is within the wall circle, select source that is clicked on (if any)
    if (event.x>10 && event.x <20+ZirkOscjuceAudioProcessor::s_iDomeRadius*2 && event.y>20 && event.y< 40+ZirkOscjuceAudioProcessor::s_iDomeRadius*2) {
        source=getSourceFromPosition(Point<float>(event.x-_ZirkOSC_Center_X, event.y-_ZirkOSC_Center_Y));

    }
    
    //if a source is clicked on, mark this source as being dragged
    _isSourceBeingDragged = (source!=-1);
    if(_isSourceBeingDragged){

        //if sources are being dragged, tell host that their parameters are about to change (beginParameterChangeGesture)
        getProcessor()->setSelectedSource(source);
        int selectedConstraint = getProcessor()->getSelectedMovementConstraintAsInteger();
        if (selectedConstraint == Independant) {
            getProcessor()->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + source*5);
            getProcessor()->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + source*5);
        }
        else if (selectedConstraint == DeltaLocked || selectedConstraint == Circular || selectedConstraint == FixedRadius || selectedConstraint == FixedAngles || selectedConstraint == FullyFixed){
            for(int i = 0;i<getProcessor()->getNbrSources(); i++){
                getProcessor()->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + i*5);
                getProcessor()->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + i*5);
                getProcessor()->getSources()[i].setAzimReverse(false);
            }
        }
        //repaint();
    }
    _GainSlider.grabKeyboardFocus();
}

int ZirkOscjuceAudioProcessorEditor::getSourceFromPosition(Point<float> p ){
    for (int i=0; i<getProcessor()->getNbrSources() ; i++){
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
        repaint();
    }
    getProcessor()->sendOSCValues();
    _GainSlider.grabKeyboardFocus();
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
    for(int i= 1; i < nbrSources ; i++){ //for(int i= 1; i != nbrSources ; i++){
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
            // gainLabel.setText(String(count++),false);
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
    for (int i =0; i<ourProcessor->getNbrSources()&&inTheDome; i++) {
         inTheDome=ourProcessor->getSources()[i].isStillInTheDome(DeltaMove);
    }
    if (inTheDome){
        for(int i=0;i<nbrSources;i++){
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

void ZirkOscjuceAudioProcessorEditor::mouseUp (const MouseEvent &event){
    if(_isSourceBeingDragged){
        int selectedConstrain = getProcessor()->getSelectedMovementConstraintAsInteger();
        if(selectedConstrain == Independant){
            int selectedSource = getProcessor()->getSelectedSource();
            getProcessor()->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId+ selectedSource*5);
            getProcessor()->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + selectedSource*5);

        }
        else if (selectedConstrain == DeltaLocked || selectedConstrain == Circular || selectedConstrain == FixedRadius || selectedConstrain == FixedAngles || selectedConstrain == FullyFixed){
            for(int i = 0;i<getProcessor()->getNbrSources(); i++){
                getProcessor()->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + i*5);
                getProcessor()->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + i*5);
            }
            //VB APRIL 16 2014
            //_isNeedToSetFixedAngles=true;
        }
        _isSourceBeingDragged=false;
    }
    _GainSlider.grabKeyboardFocus();
}

void ZirkOscjuceAudioProcessorEditor::textEditorReturnKeyPressed (TextEditor &editor){
    
    String text = editor.getText();
    int intValue = editor.getText().getIntValue();
    
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();

    //osc port was changed
    if(&_OscPortTextEditor == &editor )
    {
        int newPort = intValue;
        ourProcessor->changeOSCPort(newPort);
        _OscPortTextEditor.setText(String(ourProcessor->getOscPortZirkonium()));
    }
    if(&_NbrSourceTextEditor == &editor)
    {
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
    if(&_FirstSourceIdTextEditor == &editor ){
        int newChannel = intValue;
    
        //set the ID of the first source to intValue, then set all subsequent source IDs to subsequent numbers
        for (int iCurSource = 0; iCurSource < 8; ++iCurSource){
            ourProcessor->getSources()[iCurSource].setChannel(newChannel++);
        }
       ourProcessor->sendOSCValues();
    }
    if (&_OscPortOutgoingIPadTextEditor == &editor) {
        int newPort = intValue;
        ourProcessor->changeOSCSendIPad(newPort, ourProcessor->getOscAddressIpad());
    }
    if (&_OscAdressIPadTextEditor == &editor) {
        String oscAddress = text;
        ourProcessor->changeOSCSendIPad(ourProcessor->getOscPortIpadOutgoing().getIntValue(), oscAddress);
    }
    if (&_OscPortIncomingIPadTextEditor == &editor) {
        int newPort = intValue;
        ourProcessor->changeOSCPortReceive(newPort);
    }
    ourProcessor->sendOSCConfig();
    ourProcessor->sendOSCValues();
    ourProcessor->sendOSCMovementType();
    _GainSlider.grabKeyboardFocus();
}

void ZirkOscjuceAudioProcessorEditor::comboBoxChanged (ComboBox* comboBoxThatHasChanged){
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    ourProcessor->setParameterNotifyingHost(ZirkOscjuceAudioProcessor::ZirkOSC_MovementConstraint_ParamId,
                                            IntToPercent(comboBoxThatHasChanged->getSelectedId()));

//    VB APRIL 16 2014
    int selectedConstraint = comboBoxThatHasChanged->getSelectedId();
    if( selectedConstraint == FixedAngles || selectedConstraint == FullyFixed){
        _isNeedToSetFixedAngles=true;
    }
    ourProcessor->sendOSCMovementType();
    _GainSlider.grabKeyboardFocus();

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
