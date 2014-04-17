/*
 ==============================================================================

 This file was auto-generated by the Introjucer!

 It contains the basic startup code for a Juce application.
 Copyright 2013 Ludovic LAFFINEUR ludovic.laffineur@gmail.com
 ==============================================================================
 */

#ifndef __PLUGINEDITOR_H_4624BC76__
#define __PLUGINEDITOR_H_4624BC76__

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
 */
class ZirkOscjuceAudioProcessor;
class ZirkOscjuceAudioProcessorEditor  : public AudioProcessorEditor,
public ButtonListener,
public SliderListener,
public Timer,
public MouseListener,
public TextEditorListener,
public ComboBoxListener

{
public:
    
    //! Constructor
    ZirkOscjuceAudioProcessorEditor (ZirkOscjuceAudioProcessor* ownerFilter);
    //! Destructor
    ~ZirkOscjuceAudioProcessorEditor();

    //! This is just a standard Juce paint method...
    void paint (Graphics& g);
    
    //! when you want to refresh the TextEditors.
    void refreshGui();
    
    //! called when window is resized
    void resized() override;
    
    //! Enum of the movement constraints
    enum AllConstraints
    {
        Independant = 1,/*!< Independant mode */
        Circular,        /*!< Circular */
        FixedRadius,    /*!< All sources' radius are fixed */
        FixedAngles,    /*!< Angle between sources are fixed */
        FullyFixed,     /*!< FixedRadius and fixedAngles */
        DeltaLocked,    /*!< Delta lock mode */
        TotalNumberConstraints
    };
    //! Move the sources circular with a radius fixed
    void moveCircularWithFixedRadius (Point<float>);
    //! Move sources with a delta x and delta y (delta lock)
    void moveSourcesWithDelta(Point<float> DeltaMove);
    //! Move sources around the center
    void moveCircular(Point<float> );
    
    
    
    void moveCircular(Point<float>, bool isFixedRadius );
    
    
    //! Move sources with fixed angle between each source
    void moveFixedAngles(Point<float>);
    //! Move sources with fixed angles and fixed radius
    void moveFullyFixed(Point<float>);
    //! Setter FixedAngle
    void setFixedAngle(bool fixedAngle);
    //! Getter FixedAngle
    bool isFixedAngle();
    //! Setter draggableSource
    void setDraggableSource(bool drag);
    //! Getter draggableSource
    bool isDraggableSource();

private:
    
    //! Called when a comboBox's value has changed
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
    //! Called when a button is clicked
    void buttonClicked (Button* button);

    
    //METHODS FOR DEALING WITH DIRECT WALLCIRCLE INTERACTIONS
    //! Called when a mouse is clicked
    void mouseDown (const MouseEvent &event);
    //! Called when there is a draggin event
 	void mouseDrag (const MouseEvent &event);
    //! Called when the mouse is up
 	void mouseUp (const MouseEvent &event);
    //! Called when a value of a slider has changed
    
    
    void sliderValueChanged (Slider* slider);
    //! Called every laps of time
    void timerCallback();
    //! Called when enter is pressed in a TextEditor
    void textEditorReturnKeyPressed (TextEditor &editor);
    //! Function to set the combination of Slider and Label.
    void setSliderAndLabel(int x, int y, int width, int height, String labelText, Slider* slider, Label* label, float min, float max);
    //! Return the position of the source at the position p if no source returns -1
    int getSourceFromPosition(Point<float> p );

    //! Resizable corner to allow plugin window to be resized
    ScopedPointer<ResizableCornerComponent> _Resizer;
    
    //! Bounds of the resizable window
    ComponentBoundsConstrainer _ResizeLimits;
    
    //! Toggle Button to link the span
    ToggleButton _LinkSpanButton;
    
    //! Slider to change the Azimuth
    Slider _AzimuthSlider;
    //! Slider to change the Azimuth Span
    Slider _AzimuthSpanSlider;
    //! Slider to change the Elevation
    Slider _ElevationSlider;
    //! Slider to change the Elevation Span
    Slider _ElevationSpanSlider;
    //! Slider to change the gain
    Slider _GainSlider;

    //! Azimuth's Label
    Label _AzimuthLabel;
    //! Azimuth span's Label
    Label _AzimuthSpanLabel;
    //! Elevation's Label
    Label _ElevationLabel;
    //! Elevation span's Label
    Label _ElevationSpanLabel;
    //! Gain's Label
    Label _GainLabel;
    
    //! Label of the channel number of the selectedSource
    Label _ChannelNumberLabel;
    //! Label of the ZKM port
    Label _OscPortLabel;
    //! Label of the outgoing port to the iPad
    Label _OscPortOutgoingIPadLabel;
    //! Label of the incoming port
    Label _OscPortIncomingIPadLabel;
    //! Label of the number of sources
    Label _NbrSourceLabel;
    //! Label of the iPad address
    Label _OscAdressIPadTextLabel;

    //! TextEditor for the ZKM port 
    TextEditor _OscPortTextEditor;
    //! TextEditor for the Number of sources
    TextEditor _NbrSourceTextEditor;
    //! TextEditor for the channel number of the selected source 
    TextEditor _FirstSourceIdTextEditor;
    //! TextEditor for the iPad Port outgoing (from the plug-in to the iPad)
    TextEditor _OscPortOutgoingIPadTextEditor;
    //! TextEditor for the iPad ip address
    TextEditor _OscAdressIPadTextEditor;
    //! TextEditor for the iPad incoming port  (from the ipad to the plugin)
    TextEditor _OscPortIncomingIPadTextEditor;

    //! Combobox to choose constrain type
    ComboBox _MovementConstraintComboBox;

    //! If there is a source beeing drag
    bool _isSourceBeingDragged = false;
    //! Whether the angles between the sources need to all be set equal
    bool _isNeedToSetFixedAngles=false;
    //! If the span are linked
    bool _LinkSpan = false;

    //! Auto generated function, to get the processor
    ZirkOscjuceAudioProcessor* getProcessor() const
    {
        return static_cast <ZirkOscjuceAudioProcessor*> (getAudioProcessor());
    }
    
    //! Order the sources by angle
    void orderSourcesByAngle(int begin, SoundSource tab [] );
    
    /*Painting functions*/
    //! Paint the Azimuth line
    void paintAzimuthLine (Graphics& g);
    //! Paint the center dot
    void paintCenterDot (Graphics& g);
    //! Paint the cross coord labels
    void paintCoordLabels (Graphics& g);
    //! Paint the crosshairs
    void paintCrosshairs (Graphics& g);
    //! Paint sources points
    void paintSourcePoint (Graphics& g);
    //! Paint the span arc
    void paintSpanArc (Graphics& g);
    //! Paint the wall circle, ie the main circle in the gui
    void paintWallCircle (Graphics& g);
    //! Paint the Zenith circle, circle on the selected source
    void paintZenithCircle (Graphics& g);
    
    //! projects dome coords to screen coords (sphere to circle)
    Point <float> domeToScreen (Point <float>);
    //! projects screen coords to dome coords (circle to sphere)
    Point <float> screenToDome (Point <float>);

    //! converts degree to radian
    inline float degreeToRadian (float);
    //! converts radian to Degree
    inline float radianToDegree (float);
    //! get the source order by the angle value
    int * getOrderSources(int, SoundSource[], int nbrSources);
};

//! Conversion of percent value to Human readeable value
float PercentToHR(float , float , float );
//! Conversion of human readable value to percent value
float HRToPercent(float , float , float );
//! Conversion of percent value to integer. The min and max values represent the bounds of the integer range, ie, those values would respectively be converted to 0 and 1
int PercentToInt(float percent, int max=ZirkOscjuceAudioProcessorEditor::TotalNumberConstraints);
//! Conversion of integer value to percent. The min and max values represent the bounds of the integer range, ie, those values would respectively be converted to 0 and 1
float IntToPercent(int integer, int max=ZirkOscjuceAudioProcessorEditor::TotalNumberConstraints);

#endif  // __PLUGINEDITOR_H_4624BC76__
