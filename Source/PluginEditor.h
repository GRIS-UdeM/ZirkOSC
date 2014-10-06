/*
 ==============================================================================
 ZirkOSC2: VST and AU audio plug-in enabling spatial movement of sound sources in a dome of speakers.
 
 Copyright (C) 2014  GRIS-UdeM
 
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
 */

#ifndef __PLUGINEDITOR_H_4624BC76__
#define __PLUGINEDITOR_H_4624BC76__

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "TrajectoryProperties.hpp"

//==============================================================================
/**
 */


class SlidersTab;
class TrajectoryTab;


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
    
    int getDomeRadius();
    
    //! Function to set the combination of Slider and Label.
    static void setSliderAndLabel(String labelText, Slider* slider, Label* label, float min, float max);


private:
    
    //! Called when a comboBox's value has changed
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
    //! Called when a button is clicked
    void buttonClicked (Button* button);
    
    bool m_bUseIpad;

    
    //METHODS FOR DEALING WITH DIRECT WALLCIRCLE INTERACTIONS
    //! Called when a mouse is clicked, if mouse is clicked on source, make this source the selected source
    void mouseDown (const MouseEvent &event);
    //! Called when there is a draggin event
 	void mouseDrag (const MouseEvent &event);
    //! Called when the mouse is up
 	void mouseUp (const MouseEvent &event);
    
    void setTrajectorySource();
    
    //! Called when a value of a slider has changed
    void sliderValueChanged (Slider* slider);
    
    //! Called when slider drag initiated
    void sliderDragStarted (Slider* slider);
    
    //! Called when slider drag ended
    void sliderDragEnded (Slider* slider);
    
    //! Called every laps of time
    void timerCallback();
    //! Called when enter is pressed in a TextEditor
    void textEditorReturnKeyPressed (TextEditor &editor);
    //! Called when text is changed in a TextEditor
    void textEditorFocusLost (TextEditor &editor);
    //! Function to set the position of one Slider and associated Label.
    void setSliderAndLabelPosition(int x, int y, int width, int height, Slider* slider, Label* label);
    //! Function to set the position of one label and associated text editor.   
    void setLabelAndTextEditorPosition(int x, int y, int width, int height, Label* p_label, TextEditor* p_textEditor);
    //! Return the position of the source at the position p if no source returns -1
    int getSourceFromPosition(Point<float> p );

    //! Resizable corner to allow plugin window to be resized
    ScopedPointer<ResizableCornerComponent> _Resizer;
    
    //! Bounds of the resizable window
    ComponentBoundsConstrainer _ResizeLimits;
    
    //! Toggle Button to link the span
    ToggleButton _LinkSpanButton;
    
    //! Toggle Button to de/activate osc messages to Zirkonium
    ToggleButton _OscActiveButton;

    bool _isReturnKeyPressedCalledFromFocusLost;
    
    TabbedComponent _TabComponent;
    
    
    //-------------TRAJECTORIES------------------
    ComboBox* m_pTrajectoryComboBox;
    
    ComboBox* m_pSyncWTempoComboBox;
    //ToggleButton*   m_pSyncWTempoButton;
    
    TextButton*     m_pWriteTrajectoryButton;
    
    TextButton*     m_pTrajectoryPreviewButton;
    
    static bool _AlreadySetTrajectorySource;
    
    Label* m_pTrajectoryCountLabel;
    
    Label* m_pTrajectoryDurationLabel;
    
    TextEditor* m_pTrajectoryCountTextEditor;
    
    TextEditor* m_pTrajectoryDurationTextEditor;
    

    
    //---------------- SLIDERS ------------------
    
    //! Slider to change the Azimuth
    Slider* m_pAzimuthSlider;
    //! Slider to change the Azimuth Span
    Slider* m_pAzimuthSpanSlider;
    //! Slider to change the Elevation
    Slider* m_pElevationSlider;
    //! Slider to change the Elevation Span
    Slider* m_pElevationSpanSlider;
    //! Slider to change the gain
    Slider* m_pGainSlider;

    //---------------- LABELS ------------------
    
    //! Azimuth's Label
    Label* m_pAzimuthLabel;
    //! Azimuth span's Label
    Label* m_pAzimuthSpanLabel;
    //! Elevation's Label
    Label* m_pElevationLabel;
    //! Elevation span's Label
    Label* m_pElevationSpanLabel;
    //! Gain's Label
    Label* m_pGainLabel;
    
    //! Label of the channel number of the selectedSource
    Label _FirstSourceIdLabel;
    //! Label of the ZKM port
    Label _ZkmOscPortLabel;
    //! Label of the number of sources
    Label _NbrSourceLabel;
    //! Label of the outgoing port to the iPad
    Label _IpadOutgoingOscPortLabel;
    //! Label of the incoming port
    Label _IpadIncomingOscPortLabel;
    //! Label of the iPad address
    Label _IpadIpAddressLabel;
    



    //! TextEditor for the channel number of the selected source
    TextEditor _FirstSourceIdTextEditor;
    //! TextEditor for the ZKM port
    TextEditor _ZkmOscPortTextEditor;
    //! TextEditor for the Number of sources
    TextEditor _NbrSourceTextEditor;
    //! TextEditor for the iPad Port outgoing (from the plug-in to the iPad)
    TextEditor _IpadOutgoingOscPortTextEditor;
    //! TextEditor for the iPad incoming port  (from the ipad to the plugin)
    TextEditor _IpadIncomingOscPortTextEditor;
    //! TextEditor for the iPad ip address
    TextEditor _IpadIpAddressTextEditor;
    


    //! Combobox to choose constrain type
    ComboBox _MovementConstraintComboBox;
    
    //! If there is a source beeing drag
    bool _isSourceBeingDragged = false;
    //! Whether the angles between the sources need to all be set equal
    bool _isNeedToSetFixedAngles=false;



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
    Point <float> degreeToXy (Point <float>);
    //! projects screen coords to dome coords (circle to sphere)
    Point <float> xyToDegree (Point <float>);

    //! converts degree to radian
    inline float degreeToRadian (float);
    //! converts radian to Degree
    inline float radianToDegree (float);
    //! get the source order by the angle value
    int * getOrderSources(int, SoundSource[], int nbrSources);
    
    int _ZirkOSC_Center_X;
    int _ZirkOSC_Center_Y;
    
    SlidersTab* m_oSlidersTab;
    
    TrajectoryTab* m_oTrajectoryTab;

    
};


#endif  // __PLUGINEDITOR_H_4624BC76__

