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
 */

#ifndef __PLUGINEDITOR_H_4624BC76__
#define __PLUGINEDITOR_H_4624BC76__

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "Leap.h"

//==============================================================================
/**
 */

class MiniProgressBar;
class SlidersTab;
class TrajectoryTab;
class InterfaceTab;
class HIDDelegate;
class ZirkLeap;
class ZirkOscAudioProcessor;

class ZirkOscAudioProcessorEditor  : public AudioProcessorEditor,
public ButtonListener,
public SliderListener,
public Timer,
public TextEditorListener,
public ComboBoxListener
{
    
public:
    
    //! Constructor
    ZirkOscAudioProcessorEditor (ZirkOscAudioProcessor* ownerFilter);
    //! Destructor
    ~ZirkOscAudioProcessorEditor();

    //! This is just a standard Juce paint method...
    void paint (Graphics& g) override;
    
    //! when you want to refresh the TextEditors.
    void refreshGui();
    
    //! called when window is resized
    void resized() override;
    
    void updateWallCircleSize(int iCurWidth, int iCurHeight);
    void updateTrajectoryTabSize(int iCurw, int iCurHeight);
    
    void move (int, float, float);
    
    //! Function to set the combination of Slider and Label.
    static void setSliderAndLabel(String labelText, Slider* slider, Label* label, float min, float max);
    
    //Import from octogris for Leap and Joystick
    
    Label * getmStateLeap() {return m_pLBLeapState;}
    HIDDelegate * getHIDDel() {return mHIDDel;};
    void uncheckJoystickButton();
    JUCE_COMPILER_WARNING("these 2 methods were created because of static issues in hid_delegate and such")
    int getNbSources();
    int getCBSelectedSource();
    
    void startEditorTimer(int ms);
private:
    
    ZirkOscAudioProcessor* ourProcessor;
    
    void updateSliders();
    void updateEndLocationTextEditors();
    void updateTurnsTextEditor();
    void setDefaultPendulumEndpoint();
    
    //! Called when a comboBox's value has changed
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;
    //! Called when a button is clicked
    void buttonClicked (Button* button) override;

    int getNumSelectedTrajectoryDirections();
    
    int getNumSelectedTrajectoryReturns();
    
    //METHODS FOR DEALING WITH DIRECT WALLCIRCLE INTERACTIONS
    //! Called when a mouse is clicked, if mouse is clicked on source, make this source the selected source
    void mouseDown (const MouseEvent &event) override;
    //! Called when there is a draggin event
 	void mouseDrag (const MouseEvent &event) override;
    //! Called when the mouse is up
 	void mouseUp (const MouseEvent &event) override;
    
    void setTrajectorySource();

    void sliderValueChanged (Slider* slider) override;
    void sliderDragStarted (Slider* slider) override;
    void sliderDragEnded (Slider* slider) override;
    
    void timerCallback() override;
    void textEditorReturnKeyPressed (TextEditor &editor) override;
    void textEditorFocusLost (TextEditor &editor) override;
    void setSliderAndLabelPosition(int x, int y, int width, int height, Slider* slider, Label* label);
    void setLabelAndTextEditorPosition(int x, int y, int width, int height, Label* p_label, TextEditor* p_textEditor);
    int getSourceFromPosition(Point<float> p);
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
    ComboBox* m_pTrajectoryTypeComboBox;
    ComboBox* m_pTrajectoryDirectionComboBox;
    ComboBox* m_pTrajectoryReturnComboBox;
    ComboBox* m_pSyncWTempoComboBox;
    
    TextButton* m_pWriteTrajectoryButton;
    TextButton* m_pSetEndTrajectoryButton;
    TextEditor* m_pEndAzimTextEditor;
    TextEditor* m_pEndElevTextEditor;
    TextButton* m_pResetEndTrajectoryButton;
    
    Label* m_pTrajectoryCountLabel;
    Label* m_pTrajectoryTurnsLabel;
    Label* m_pTrajectoryDeviationLabel;
    Label* m_pTrajectoryDampeningLabel;
    Label* m_pTrajectoryDurationLabel;
    
    TextEditor* m_pTrajectoryCountTextEditor;
    TextEditor* m_pTrajectoryTurnsTextEditor;
    TextEditor* m_pTrajectoryDeviationTextEditor;
    TextEditor* m_pTrajectoryDurationTextEditor;
    TextEditor* m_pTrajectoryDampeningTextEditor;
    
  	MiniProgressBar *mTrProgressBar;
    
    void updateTrajectoryComponents();

    Slider* m_pAzimuthSlider;
    Slider* m_pAzimuthSpanSlider;
    Slider* m_pElevationSlider;
    Slider* m_pElevationSpanSlider;
    Slider* m_pGainSlider;

    Label* m_pAzimuthLabel;
    Label* m_pAzimuthSpanLabel;
    Label* m_pElevationLabel;
    Label* m_pElevationSpanLabel;
    Label* m_pGainLabel;
    Label m_oFirstSourceIdLabel;
    Label m_oZkmOscPortLabel;
    Label m_oNbrSourceLabel;
    Label m_VersionLabel;
    
    ImageComponent m_logoImage;
    


    TextEditor _FirstSourceIdTextEditor;
    TextEditor _ZkmOscPortTextEditor;
    TextEditor _NbrSourceTextEditor;

//    //! Label of the outgoing port to the iPad
//    Label _IpadOutgoingOscPortLabel;
//    //! Label of the incoming port
//    Label _IpadIncomingOscPortLabel;
//    //! Label of the iPad address
//    Label _IpadIpAddressLabel;
//    //! TextEditor for the iPad Port outgoing (from the plug-in to the iPad)
//    TextEditor _IpadOutgoingOscPortTextEditor;
//    //! TextEditor for the iPad incoming port  (from the ipad to the plugin)
//    TextEditor _IpadIncomingOscPortTextEditor;
//    //! TextEditor for the iPad ip address
//    TextEditor _IpadIpAddressTextEditor;
    

    ComboBox m_oMovementConstraintComboBox;

    bool m_bIsSourceBeingDragged = false;

    //-------------INTERFACES------------------
    
    //! Toggle Button to de/activate leap motion usage
    ToggleButton* m_pTBEnableLeap;
    Label* m_pLBLeapState;
    ComboBox* m_pCBLeapSource;
    
    
    //! Toggle Button to de/activate joystick usage
    ToggleButton* m_pTBEnableJoystick;
    
    Label* m_pLBJoystickState;
    
    //joystick
    ReferenceCountedObjectPtr<HIDDelegate> mHIDDel;

    //! Auto generated function, to get the processor
    ZirkOscAudioProcessor* getProcessor() const
    {
        return static_cast <ZirkOscAudioProcessor*> (getAudioProcessor());
    }
   
    /*Painting functions*/
    //! Paint the center dot
    void paintCenterDot (Graphics& g);
    //! Paint the cross coord labels
    void paintCoordLabels (Graphics& g);
    //! Paint sources points
    void paintSourcePoint (Graphics& g);
    //! Paint the span arc for iSrc
    void paintSpanArc (Graphics& g, int iSrc);
    //! Paint the wall circle, ie the main circle in the gui
    void paintWallCircle (Graphics& g);
    
    //! projects dome coords to screen coords (sphere to circle)
    Point <float> degreeToXy (Point <float>);

    int _ZirkOSC_Center_X;
    int _ZirkOSC_Center_Y;
    
    SlidersTab* m_oSlidersTab;
    
    TrajectoryTab* m_oTrajectoryTab;
    
    InterfaceTab* m_oInterfaceTab;
    
    ScopedPointer<Leap::Controller> mController;
    
    ReferenceCountedObjectPtr<ZirkLeap>  mleap;
    
    enum {
        kTrReady = 0,
        kTrWriting
    };
    int mTrState;
    
    float m_fHueOffset;
    Label m_oEndPointLabel;
    
};


#endif  // __PLUGINEDITOR_H_4624BC76__

