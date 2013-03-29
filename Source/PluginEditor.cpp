/*
 ==============================================================================

 Developer : Ludovic LAFFINEUR (ludovic.laffineur@gmail.com)

 Lexic :
 - all parameter preceeded by HR are Human Readable
 - parameter without HR are in percent
 - Points in spheric coord  : x -> azimuth, y -> elevation

 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cstdlib>
#include <string>
#include <string.h>
#include <sstream>
#include <istream>
#include <math.h>

using namespace std;

void setSliderAndLabel(int x, int y, int width, int height, String labelTest, Slider* slider, Label* label, float min, float max);


//==============================================================================
ZirkOscjuceAudioProcessorEditor::ZirkOscjuceAudioProcessorEditor (ZirkOscjuceAudioProcessor* ownerFilter)
:   AudioProcessorEditor (ownerFilter),
mSourcePoint(0.0f,0.0f), //coords dome : azimuth, elev
gainSlider (ZirkOSC_Gain_name[0]),
azimuthSlider(ZirkOSC_AzimSpan_name[0]),
elevationSlider(ZirkOSC_ElevSpan_name[0]),
azimuthLabel(ZirkOSC_Azim_name[0]),
elevationSpanSlider(ZirkOSC_ElevSpan_name[0]),
azimuthSpanLabel(ZirkOSC_AzimSpan_name[0]),
elevationLabel(ZirkOSC_Elev_name[0]),
gainLabel(ZirkOSC_Gain_name[0]),
OSCPortTextEditor("OscPort"),
NbrSourceTextEditor("NbrSource"),
OSCPortLabel("OscPort"),
NbrSourceLabel("NbrSources"),
channelNumberTextEditor("channelNbr"),
channelNumberLabel("channelNbr"),
mouvementConstrain("MouvemntContrain"),
linkSpanButton("Link Span")

{
    // This is where our plugin's editor size is set.
    setSize (ZirkOSC_Window_Width, ZirkOSC_Window_Height);
    /*  button1 = new TextButton("Button1");*/

    // button1->setBounds (20, 70, 260, 20);*/
    mSourcePoint.setX(0.0f);
    mSourcePoint.setY (0.0f);


    setSliderAndLabel(20,340, 360, 20, "Gain", &gainSlider,&gainLabel, 0.0, 1.0);
    addAndMakeVisible(&gainSlider);
    addAndMakeVisible(&gainLabel);

    setSliderAndLabel(20,380, 360, 20, "Azimuth", &azimuthSlider ,&azimuthLabel, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    addAndMakeVisible(&azimuthSlider);
    addAndMakeVisible(&azimuthLabel);


    setSliderAndLabel(20, 420, 360, 20, "Elevation", &elevationSlider, &elevationLabel, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    addAndMakeVisible(&elevationSlider);
    addAndMakeVisible(&elevationLabel);


    setSliderAndLabel(20, 460, 360, 20, "Elev. Sp.", &elevationSpanSlider, &elevationSpanLabel, ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max);
    addAndMakeVisible(&elevationSpanSlider);
    addAndMakeVisible(&elevationSpanLabel);

    setSliderAndLabel(20, 500, 360, 20, "Azim. Sp.", &azimuthSpanSlider, &azimuthSpanLabel, ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);
    addAndMakeVisible(&azimuthSpanSlider);
    addAndMakeVisible(&azimuthSpanLabel);

    NbrSourceLabel.setText("Nbr Sources", false);
    NbrSourceLabel.setBounds(ZirkOSC_Window_Width-80 , 30, 80, 25);
    NbrSourceTextEditor.setBounds(ZirkOSC_Window_Width-75 , 50, 60, 25);
    NbrSourceTextEditor.setText(String(getProcessor()->nbrSources));
    addAndMakeVisible(&NbrSourceLabel);
    addAndMakeVisible(&NbrSourceTextEditor);


    OSCPortLabel.setText("Port OSC", false);
    OSCPortLabel.setBounds(ZirkOSC_Window_Width-80 , 80, 60, 25);
    OSCPortTextEditor.setBounds(ZirkOSC_Window_Width-75 , 100, 60, 25);
    OSCPortTextEditor.setText(String(getProcessor()->moscPort));
    addAndMakeVisible(&OSCPortLabel);
    addAndMakeVisible(&OSCPortTextEditor);

    channelNumberLabel.setText("Channel nbr", false);
    channelNumberLabel.setBounds(ZirkOSC_Window_Width-80 , 130, 60, 25);
    channelNumberTextEditor.setBounds(ZirkOSC_Window_Width-75 , 150, 60, 25);
    channelNumberTextEditor.setText(String(getProcessor()->tabSource[getProcessor()->selectedSource].getChannel()));
    addAndMakeVisible(&channelNumberLabel);
    addAndMakeVisible(&channelNumberTextEditor);

    mouvementConstrain.setSize(50, 20);
    mouvementConstrain.setBounds(80, 540, 220, 25);

    mouvementConstrain.addItem("Independant",   Independant);
    mouvementConstrain.addItem("Circular",      Circular);
    mouvementConstrain.addItem("Fixed Radius",  FixedRadius);
    mouvementConstrain.addItem("Fixed Angle",   FixedAngles);
    mouvementConstrain.addItem("Fully fixed",   FullyFixed);
    mouvementConstrain.addItem("Delta Lock",    DeltaLocked);

    mouvementConstrain.setSelectedId(Independant);
    selectedConstrain = Independant;
    mouvementConstrain.addListener(this);
    addAndMakeVisible(&mouvementConstrain);
    
    linkSpanButton.setBounds(300, 300, 80, 30);
    addAndMakeVisible(&linkSpanButton);
    linkSpanButton.addListener(this);

    channelNumberTextEditor.addListener(this);
    OSCPortTextEditor.addListener(this);
    NbrSourceTextEditor.addListener(this);
    elevationSlider.addListener(this);
    azimuthSlider.addListener(this);
    gainSlider.addListener(this);
    elevationSpanSlider.addListener(this);
    azimuthSpanSlider.addListener(this);
    this->setFocusContainer(true);
    startTimer (50);
}

ZirkOscjuceAudioProcessorEditor::~ZirkOscjuceAudioProcessorEditor()
{
    //deleteAllChildren ();
}


//Automatic function to set label and Slider

void setSliderAndLabel(int x, int y, int width, int height, String labelText, Slider* slider, Label* label, float min, float max){
    slider->setBounds (x+60, y, width-60, height);
    slider->setTextBoxStyle(Slider::TextBoxRight, false, 80, 20);
    label->setText(labelText, false);
    label->setBounds(x, y, width-300, height);
    slider->setRange (min, max, 0.01);
}

//==============================================================================
void ZirkOscjuceAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::lightgrey);
    paintWallCircle(g);
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
    int selectedSource = ourProcessor->selectedSource;
    float HRAzim = PercentToHR(ourProcessor->tabSource[selectedSource].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max),
    HRElev = PercentToHR(ourProcessor->tabSource[selectedSource].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max),
    HRElevSpan = PercentToHR(ourProcessor->tabSource[selectedSource].getElevation_span(), ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max),
    HRAzimSpan = PercentToHR(ourProcessor->tabSource[selectedSource].getAzimuth_span(), ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);

    Point<float> maxElev = {HRAzim, HRElev+HRElevSpan/2};
    Point<float> minElev = {HRAzim, HRElev-HRElevSpan/2};

    if(minElev.getY() < ZirkOSC_ElevSpan_Min){
        maxElev.setY(maxElev.getY()+ ZirkOSC_ElevSpan_Min-minElev.getY());
        minElev.setY(ZirkOSC_ElevSpan_Min);
    }

    Point<float> screenMaxElev = domeToScreen(maxElev);
    Point<float> screenMinElev = domeToScreen(minElev);
    float maxRadius = sqrtf(screenMaxElev.getX()*screenMaxElev.getX() + screenMaxElev.getY()*screenMaxElev.getY());
    float minRadius = sqrtf(screenMinElev.getX()*screenMinElev.getX() + screenMinElev.getY()*screenMinElev.getY());
    //drawing the path for spanning
    Path myPath;
    myPath.startNewSubPath(ZirkOSC_Center_X+screenMinElev.getX(),ZirkOSC_Center_Y+screenMinElev.getY());

    //half first arc center
    myPath.addCentredArc(ZirkOSC_Center_X, ZirkOSC_Center_Y, minRadius, minRadius, 0.0, degreeToRadian(-HRAzim), degreeToRadian(-HRAzim + HRAzimSpan/2 ));

    if (maxElev.getY()> ZirkOSC_ElevSpan_Max) { // if we are over the top of the dome we draw the adjacent angle

        myPath.addCentredArc(ZirkOSC_Center_X, ZirkOSC_Center_Y, maxRadius, maxRadius, 0.0, M_PI+degreeToRadian(-HRAzim + HRAzimSpan/2), M_PI+degreeToRadian(-HRAzim - HRAzimSpan/2 ));
    }
    else{
        myPath.addCentredArc(ZirkOSC_Center_X, ZirkOSC_Center_Y, maxRadius, maxRadius, 0.0, degreeToRadian(-HRAzim+HRAzimSpan/2), degreeToRadian(-HRAzim-HRAzimSpan/2 ));
    }
    myPath.addCentredArc(ZirkOSC_Center_X, ZirkOSC_Center_Y, minRadius, minRadius, 0.0, degreeToRadian(-HRAzim-HRAzimSpan/2), degreeToRadian(-HRAzim ));
    myPath.closeSubPath();
    g.setColour(Colours::lightgrey);
    g.fillPath(myPath);
    g.setColour(Colours::black);
    g.strokePath(myPath, PathStrokeType::curved);
}

void ZirkOscjuceAudioProcessorEditor::paintSourcePoint (Graphics& g){
    Point<float> screen;
    float HRAzim, HRElev;
    for (int i=0; i<getProcessor()->nbrSources; i++) {
        HRAzim = PercentToHR(getProcessor()->tabSource[i].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
        HRElev = PercentToHR(getProcessor()->tabSource[i].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
        screen = domeToScreen(Point<float> (HRAzim, HRElev));
        g.drawEllipse(ZirkOSC_Center_X + screen.getX()-4, ZirkOSC_Center_Y + screen.getY()-4, 8, 8,2);
        g.drawText(String(getProcessor()->tabSource[i].getChannel()), ZirkOSC_Center_X + screen.getX()+6, ZirkOSC_Center_Y + screen.getY()-2, 40, 10, Justification::left, false);
    }
    // screen = domeToScreen(mSourcePoint);
    //g.setColour(Colours::blue);
    //g.drawEllipse(ZirkOSC_Center_X + screen.getX()-4, ZirkOSC_Center_Y + screen.getY()-4, 8, 8,2);
}

void ZirkOscjuceAudioProcessorEditor::paintWallCircle (Graphics& g){
    g.setColour(Colours::white);
    g.fillEllipse(10.0f, 30.0f, ZirkOSC_DomeRadius * 2, ZirkOSC_DomeRadius * 2);
    g.setColour(Colours::black);
    g.drawEllipse(10.0f, 30.0f, ZirkOSC_DomeRadius * 2, ZirkOSC_DomeRadius * 2, 1.0f);
}

void ZirkOscjuceAudioProcessorEditor::paintCenterDot (Graphics& g){
    g.setColour(Colours::red);
    g.fillEllipse(ZirkOSC_Center_X - 3.0f, ZirkOSC_Center_Y - 3.0f, 6.0f,6.0f );
}

void ZirkOscjuceAudioProcessorEditor::paintAzimuthLine (Graphics& g){
    g.setColour(Colours::red);
    float HRAzim = (float) azimuthSlider.getValue();
    float HRElev  = (float) elevationSlider.getValue();
    Point <float> screen = domeToScreen(Point<float>(HRAzim,HRElev));
    g.drawLine(ZirkOSC_Center_X, ZirkOSC_Center_Y, ZirkOSC_Center_X + screen.getX(), ZirkOSC_Center_Y + screen.getY() );
}

void ZirkOscjuceAudioProcessorEditor::paintZenithCircle (Graphics& g){
    Point <float> screen = domeToScreen(Point<float>((float)azimuthSlider.getValue(),(float)elevationSlider.getValue()));
    float raduisZenith = sqrtf(screen.getX()*screen.getX() + screen.getY()*screen.getY());
    g.drawEllipse(ZirkOSC_Center_X-raduisZenith, ZirkOSC_Center_Y-raduisZenith, raduisZenith*2, raduisZenith*2, 1.0);
}

void ZirkOscjuceAudioProcessorEditor::paintCrosshairs (Graphics& g){
    g.setColour(Colours::grey);
    float radianAngle=0.0f;
    float fraction = 0.9f;
    Point<float> axis = Point<float>();
    for (int i= 0; i<ZirkOSC_NumMarks; i++) {
        radianAngle = degreeToRadian(ZirkOSC_MarksAngles[i]);
        axis = {cosf(radianAngle), sinf(radianAngle)};
        g.drawLine(ZirkOSC_Center_X+(ZirkOSC_DomeRadius*fraction)*axis.getX(), ZirkOSC_Center_Y+(ZirkOSC_DomeRadius*fraction)*axis.getY(),ZirkOSC_Center_X+(ZirkOSC_DomeRadius)*axis.getX(), ZirkOSC_Center_Y+(ZirkOSC_DomeRadius)*axis.getY(),1.0f);
    }
}

void ZirkOscjuceAudioProcessorEditor::paintCoordLabels (Graphics& g){
    g.setColour(Colours::black);
    g.drawLine(ZirkOSC_Center_X - ZirkOSC_DomeRadius, ZirkOSC_Center_Y, ZirkOSC_Center_X + ZirkOSC_DomeRadius, ZirkOSC_Center_Y ,0.5f);
    g.drawLine(ZirkOSC_Center_X , ZirkOSC_Center_Y - ZirkOSC_DomeRadius, ZirkOSC_Center_X , ZirkOSC_Center_Y + ZirkOSC_DomeRadius,0.5f);
}

/*Conversion function*/

Point <float> ZirkOscjuceAudioProcessorEditor::domeToScreen (Point <float> p){
    float x,y;
    x = -ZirkOSC_DomeRadius * sinf(degreeToRadian(p.getX())) * cosf(degreeToRadian(p.getY()));
    y = -ZirkOSC_DomeRadius * cosf(degreeToRadian(p.getX())) * cosf(degreeToRadian(p.getY()));
    return Point <float> (x, y);
}


Point <float> ZirkOscjuceAudioProcessorEditor::screenToDome (Point <float>){
    return Point <float> (0.0, 0.0);
}

inline float ZirkOscjuceAudioProcessorEditor::degreeToRadian (float degree){
    return ((degree/360.0f)*2*3.1415);
}

inline float ZirkOscjuceAudioProcessorEditor::radianToDegree(float radian){
    return (radian/(2*3.1415)*360.0f);
}

//refesh value from the host
void ZirkOscjuceAudioProcessorEditor::timerCallback(){
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    int selectedSource = ourProcessor->selectedSource;
    gainSlider.setValue (ourProcessor->tabSource[selectedSource].getGain(), dontSendNotification);

    float HRValue = PercentToHR(ourProcessor->tabSource[selectedSource].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    azimuthSlider.setValue(HRValue,dontSendNotification);

    HRValue = PercentToHR(ourProcessor->tabSource[selectedSource].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    elevationSlider.setValue(HRValue,dontSendNotification);

    HRValue = PercentToHR(ourProcessor->tabSource[selectedSource].getAzimuth_span(), ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);
    azimuthSpanSlider.setValue(HRValue,dontSendNotification);

    HRValue = PercentToHR(ourProcessor->tabSource[selectedSource].getElevation_span(), ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max);
    elevationSpanSlider.setValue(HRValue,dontSendNotification);
    if (ourProcessor->refreshGui){
        refreshGui();
        ourProcessor->refreshGui=false;
    }
    
    repaint();
    /*    if (mSourcePoint.getX() !=  PercentToHR(ourProcessor->tabSource[selectedSource].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max)){

     mSourcePoint.x =(PercentToHR(ourProcessor->tabSource[selectedSource].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max));
     mSourcePoint.y = (PercentToHR(ourProcessor->tabSource[selectedSource].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max));
     repaint();
     }*/
}


void ZirkOscjuceAudioProcessorEditor::refreshGui(){
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    int selectedSource = ourProcessor->selectedSource;
    OSCPortTextEditor.setText(String(ourProcessor->moscPort));
    NbrSourceTextEditor.setText(String(ourProcessor->nbrSources));
    channelNumberTextEditor.setText(String(ourProcessor->tabSource[selectedSource].getChannel()));
}
void ZirkOscjuceAudioProcessorEditor::buttonClicked (Button* button)
{
    if(button == &linkSpanButton){
        linkSpan = linkSpanButton.getToggleState();
    }
}

float PercentToHR(float percent, float min, float max){
    return percent*(max-min)+min;
}

float HRToPercent(float HRValue, float min, float max){
    return (HRValue-min)/(max-min);
}

void ZirkOscjuceAudioProcessorEditor::sliderValueChanged (Slider* slider)
{
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    int selectedSource = ourProcessor->selectedSource;
    float percentValue=0;
    if (slider == &gainSlider) {
        ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Gain_Param+ (selectedSource*7) );
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Gain_Param + (selectedSource*7),
                                                 (float) gainSlider.getValue());
        ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Gain_Param + (selectedSource*7));
    }
    else if (slider == &azimuthSlider) {
        percentValue = HRToPercent((float) azimuthSlider.getValue(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
        ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param+ (selectedSource*7));
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param + (selectedSource*7),
                                                 percentValue);
        mSourcePoint.setX(azimuthSlider.getValue());
        ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param + (selectedSource*7));
    }
    else if (slider == &elevationSlider) {
        percentValue = HRToPercent((float) elevationSlider.getValue(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
        ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + (selectedSource*7));
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + (selectedSource*7),
                                                 percentValue);
        mSourcePoint.setY(elevationSlider.getValue());
        ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + (selectedSource*7));
    }
    else if (slider == &elevationSpanSlider) {
        percentValue = HRToPercent((float) elevationSpanSlider.getValue(), ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max);
        if(linkSpan){
            for(int i=0 ; i<ourProcessor->nbrSources; i++){
                ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_Param + (i*7));
                ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_Param + (i*7),
                                                         percentValue);
                ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_Param +(i*7));
            }
        }
        else{
            ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_Param + (selectedSource*7));
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_Param + (selectedSource*7),
                                                 percentValue);
            ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_Param +(selectedSource*7));
        }

    }else if (slider == &azimuthSpanSlider) {
        percentValue = HRToPercent((float) azimuthSpanSlider.getValue(), ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);
        if(linkSpan){
            for(int i=0 ; i<ourProcessor->nbrSources; i++){
                ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_Param + (i*7));
                ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_Param + (i*7),
                                                         percentValue);
                ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_Param +(i*7));
            }
        }
        else{
            ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_Param + (selectedSource*7));
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_Param + (selectedSource*7),
                                                 percentValue);
            ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_Param + (selectedSource*7));
        }
    }else;
    repaint();

}

void ZirkOscjuceAudioProcessorEditor::mouseDown (const MouseEvent &event){
    int source=-1;

    if (event.x>10 && event.x <20+ZirkOSC_DomeRadius*2 && event.y>20 && event.y< 40+ZirkOSC_DomeRadius*2) {
        source=getSourceFromPosition(Point<float>(event.x-ZirkOSC_Center_X, event.y-ZirkOSC_Center_Y));

    }
    draggableSource = (source!=-1);
    if(draggableSource){

        getProcessor()->selectedSource = source;

        channelNumberTextEditor.setText(String(getProcessor()->tabSource[source].getChannel()));
        if (selectedConstrain == Independant) {
            mSourcePoint.setX(getProcessor()->tabSource[source].getAzimuth());
            mSourcePoint.setY(getProcessor()->tabSource[source].getElevation());
            getProcessor()->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param + source*7);
            getProcessor()->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + source*7);
        }
        else if (selectedConstrain == DeltaLocked || selectedConstrain == Circular || selectedConstrain == FixedRadius || selectedConstrain == FixedAngles || selectedConstrain == FullyFixed){
            for(int i = 0;i<getProcessor()->nbrSources; i++){
                getProcessor()->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param + i*7);
                getProcessor()->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + i*7);
                getProcessor()->tabSource[i].azim_reverse=false;
            }
        }
        repaint();
    }
    azimuthSlider.grabKeyboardFocus();
}

int ZirkOscjuceAudioProcessorEditor::getSourceFromPosition(Point<float> p ){
    for (int i=0; i<getProcessor()->nbrSources ; i++){
        if (getProcessor()->tabSource[i].contains(p)){
            return i;
        }
    }
    return -1;
}

void ZirkOscjuceAudioProcessorEditor::orderSourcesByAngle (int selected, SoundSource tab[]){
    int nbrSources = getProcessor()->nbrSources;
    /* int tabOrder[nbrSources];
     tabOrder[0] = selected;
     int currentSource = selected;
     for (int j=1; j<nbrSources;j++){
     float currentdelta = 100;

     for(int i = 0 ;i< nbrSources;i++) {
     if(tab[i].getAzimuth() - tab[currentSource].getAzimuth()< currentdelta && tabOrder[j] != i && tab[i].getAzimuth() - tab[currentSource].getAzimuth()>0){
     currentSource = i;
     currentdelta = abs(tab[i].getAzimuth() - tab[currentSource].getAzimuth());
     }
     }
     tabOrder[j]=currentSource;
     } */
    int* order = getOrderSources(selected, tab);
    int count = 0;
    for(int i= 1; i != nbrSources ; i++){
        getProcessor()->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param + (order[i]*7),
                                                   tab[order[0]].getAzimuth()+ (float)(++count)/(float) nbrSources);
        //tab[order[i]].setAzimuth(tab[order[0]].getAzimuth()+ (float)(++count)/(float) nbrSources);
    }


}


int* ZirkOscjuceAudioProcessorEditor::getOrderSources(int selected, SoundSource tab []){
    int nbrSources = getProcessor()->nbrSources;
    int * order = new int [nbrSources];
    int firstItem = selected;
    order[0] = selected;
    int count  = 1;
    do{
        int current = (selected + 1)%nbrSources;

        int bestItem = current;
        float bestDelta = tab[current].getAzimuth() - tab[selected].getAzimuth();
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
    }
    while (selected != firstItem);
    return order;
}

void ZirkOscjuceAudioProcessorEditor::mouseDrag (const MouseEvent &event){
    if(draggableSource){
        ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
        int selectedSource = ourProcessor->selectedSource;
        Point<float> pointRelativeCenter = Point<float>(event.x-ZirkOSC_Center_X, event.y-ZirkOSC_Center_Y);
        if (selectedConstrain == Independant) {
            ourProcessor->tabSource[selectedSource].setPositionXY(pointRelativeCenter);
            float HRAzimuth = PercentToHR(ourProcessor->tabSource[selectedSource].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
            mSourcePoint.setX(HRAzimuth);

            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param + selectedSource*7,
                                                     ourProcessor->tabSource[selectedSource].getAzimuth());
            float HRElevation = PercentToHR(ourProcessor->tabSource[selectedSource].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Min);
            mSourcePoint.setY(HRElevation);
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + selectedSource*7,
                                                     ourProcessor->tabSource[selectedSource].getElevation());
            ourProcessor->sendOSCValues();

            repaint();
        }
        else if (selectedConstrain == FixedAngles){
            if (!isFixedAngle){
                orderSourcesByAngle(selectedSource,ourProcessor->tabSource);
                isFixedAngle=true;
            }
            moveCircular(pointRelativeCenter);
        }
        else if (selectedConstrain == FixedRadius){
            /*Moyen d'optimiser ici*/
            moveCircularWithFixedRadius(pointRelativeCenter);
            repaint();
        }
        else if (selectedConstrain == FullyFixed){
            if (!isFixedAngle){
                orderSourcesByAngle(selectedSource,ourProcessor->tabSource);
                isFixedAngle=true;
            }
            moveCircularWithFixedRadius(pointRelativeCenter);
        }
        else if (selectedConstrain == DeltaLocked){
            Point<float> DeltaMove = pointRelativeCenter - ourProcessor->tabSource[selectedSource].getPositionXY();
            moveSourcesWithDelta(DeltaMove);
            //movePointsDeltaLock(pointRelativeCenter);
        }
        else if (selectedConstrain == Circular){
            moveCircular(pointRelativeCenter);
            repaint();
        }
    }
    azimuthSlider.grabKeyboardFocus();
}

void ZirkOscjuceAudioProcessorEditor::movePointsDeltaLock(Point <float> pointRelativeCenter){
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    int selectedSource = ourProcessor->selectedSource;
    SoundSource DomeNewPoint = SoundSource();
    DomeNewPoint.setPositionXY(pointRelativeCenter);
    int nbrSources = ourProcessor->nbrSources;
    for(int i =0; i<nbrSources;i++){
        if(i!=selectedSource){
            float deltaAzim = -ourProcessor->tabSource[selectedSource].getAzimuth() + ourProcessor->tabSource[i].getAzimuth();
            float deltaElev = -ourProcessor->tabSource[selectedSource].getElevation() + ourProcessor->tabSource[i].getElevationRawValue();
            ourProcessor->tabSource[i].setAzimuth(DomeNewPoint.getAzimuth() + deltaAzim);
            ourProcessor->tabSource[i].setElevation(DomeNewPoint.getElevation() + deltaElev);
        }
    }
    ourProcessor->tabSource[selectedSource].setPositionXY(pointRelativeCenter);
}

void ZirkOscjuceAudioProcessorEditor::moveCircularWithFixedRadius(Point<float> pointRelativeCenter){
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    int selectedSource = ourProcessor->selectedSource;
    SoundSource DomeNewPoint = SoundSource();
    DomeNewPoint.setPositionXY(pointRelativeCenter);
    float HRElevation = PercentToHR(ourProcessor->tabSource[selectedSource].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    float HRAzimuth = 180+PercentToHR(ourProcessor->tabSource[selectedSource].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);

    float HRNewElevation = PercentToHR(DomeNewPoint.getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    float HRNewAzimuth = PercentToHR(DomeNewPoint.getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);

    Point<float> deltaCircularMove = Point<float>(HRNewAzimuth-HRAzimuth,HRNewElevation-HRElevation);
    //azimuthLabel.setText(String(deltaCircularMove.getY()),false);

    ourProcessor->tabSource[selectedSource].setElevation(ourProcessor->tabSource[selectedSource].getElevation()+HRToPercent(deltaCircularMove.getY(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max));
    for (int i = 0; i<ourProcessor->nbrSources; i++) {
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param + i*7,
                                                 ourProcessor->tabSource[i].getAzimuth()+HRToPercent(deltaCircularMove.getX(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max));
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + i*7,
                                                 ourProcessor->tabSource[selectedSource].getElevation());
    }
}


void ZirkOscjuceAudioProcessorEditor::moveCircular(Point<float> pointRelativeCenter){
    SoundSource DomeNewPoint = SoundSource();
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    int selectedSource = ourProcessor->selectedSource;
    DomeNewPoint.setPositionXY(pointRelativeCenter);
    float HRElevation = PercentToHR(ourProcessor->tabSource[selectedSource].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    float HRAzimuth = 180+PercentToHR(ourProcessor->tabSource[selectedSource].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);

    float HRNewElevation = PercentToHR(DomeNewPoint.getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    float HRNewAzimuth = PercentToHR(DomeNewPoint.getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);

    Point<float> deltaCircularMove = Point<float>(HRNewAzimuth-HRAzimuth,HRNewElevation-HRElevation);
    azimuthLabel.setText(String(deltaCircularMove.getY()),false);
    for (int i = 0; i<ourProcessor->nbrSources; i++) {
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param + i*7,
                                                 ourProcessor->tabSource[i].getAzimuth()+HRToPercent(deltaCircularMove.getX(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max));
        if(!ourProcessor->tabSource[i].azim_reverse){
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + i*7,
                                                     ourProcessor->tabSource[i].getElevationRawValue()+HRToPercent(deltaCircularMove.getY(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max));
        }
        else{
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + i*7,
                                                     ourProcessor->tabSource[i].getElevationRawValue()-HRToPercent(deltaCircularMove.getY(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max));
        }
    }
}

void ZirkOscjuceAudioProcessorEditor::moveSourcesWithDelta(Point<float> DeltaMove){
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    int nbrSources = ourProcessor->nbrSources;
    Point<float> currentPosition;
    for(int i=0;i<nbrSources;i++){
        currentPosition = ourProcessor->tabSource[i].getPositionXY();
        ourProcessor->tabSource[i].setPositionXY(currentPosition + DeltaMove);
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param + i*7,
                                                 ourProcessor->tabSource[i].getAzimuth());
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + i*7,
                                                 ourProcessor->tabSource[i].getElevationRawValue());
        azimuthLabel.setText(String(ourProcessor->tabSource[i].getElevation()), false);
        ourProcessor->sendOSCValues();
    }
    repaint();
}

void ZirkOscjuceAudioProcessorEditor::mouseUp (const MouseEvent &event){
    if(selectedConstrain == Independant){
        int selectedSource = getProcessor()->selectedSource;
        getProcessor()->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param+ selectedSource*7);
        getProcessor()->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + selectedSource*7);
    }
    else if (selectedConstrain == DeltaLocked || selectedConstrain == Circular || selectedConstrain == FixedRadius || selectedConstrain == FixedAngles || selectedConstrain == FullyFixed){
        for(int i = 0;i<getProcessor()->nbrSources; i++){
            getProcessor()->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param + i*7);
            getProcessor()->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + i*7);
        }
        isFixedAngle=false;
    }
    draggableSource=false;
    azimuthSlider.grabKeyboardFocus();
}

void ZirkOscjuceAudioProcessorEditor::textEditorReturnKeyPressed (TextEditor &editor){
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();

    if(&OSCPortTextEditor == &editor )
    {
        int newPort = OSCPortTextEditor.getText().getIntValue();
        ourProcessor->changeOSCPort(newPort);
        OSCPortTextEditor.setText(String(ourProcessor->moscPort));
    }
    if(&NbrSourceTextEditor == &editor )
    {
        int newNbrSources = NbrSourceTextEditor.getText().getIntValue();
        if(newNbrSources >0 && newNbrSources < 9){
            ourProcessor->nbrSources = newNbrSources;
        }
        else{
            NbrSourceTextEditor.setText(String(ourProcessor->nbrSources));
        }
    }
    if(&channelNumberTextEditor == &editor ){
        int newChannel = channelNumberTextEditor.getText().getIntValue();
        ourProcessor->tabSource[ourProcessor->selectedSource].setChannel(newChannel);
        ourProcessor->sendOSCValues();
    }
    azimuthSlider.grabKeyboardFocus();
}

void ZirkOscjuceAudioProcessorEditor::textEditorFocusLost (TextEditor &editor){
    //textEditorReturnKeyPressed(editor);
}

void ZirkOscjuceAudioProcessorEditor::comboBoxChanged (ComboBox* comboBoxThatHasChanged){
    selectedConstrain = comboBoxThatHasChanged->getSelectedId();
    azimuthSlider.grabKeyboardFocus();

}
