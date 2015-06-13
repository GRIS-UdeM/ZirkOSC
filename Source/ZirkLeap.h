/*!
 * ==============================================================================
 *
 *  ZirkLeap.h
 *  Created: 4 Aug 2014 1:23:01pm
 *  Author:  makira & Antoine L.
 *  Description :
 *  ZirkLeap allows you to create a Leap Listener through the help of Reference
 *  Counted Object which deletes it in a safe and proper way.
 *  The Leap Listener will be instanciate through the CreateLeapComponent method
 *  then it is added as a listener to a Leap Controller.
 *  ZirkLeap constructor needs two arguments which are the addresses of
 *  the main components of the plugin the Audio Processor and the Audio Processor
 *  Editor.
 * ==============================================================================
 */

#ifndef OCTOLEAP_H_INCLUDED
#define OCTOLEAP_H_INCLUDED

#include "Leap.h"
#include "PluginEditor.h"

class ZirkLeap : public ReferenceCountedObject , public Leap::Listener
{
public:
    typedef ReferenceCountedObjectPtr<ZirkLeap> Ptr;
    //! Interface to call the builder using the ReferenceCountedObject benefits
    static ZirkLeap::Ptr CreateLeapComponent(ZirkOscjuceAudioProcessor *filter, ZirkOscjuceAudioProcessorEditor *editor);
    //!Builder
    ZirkLeap(ZirkOscjuceAudioProcessor *filter, ZirkOscjuceAudioProcessorEditor *editor);
    //! Called when a Leap Motion is connected to the computer
    virtual void onConnect(const Leap::Controller& controller);
    //! Called when a Leap Motion is disconnected to the computer
    void onDisconnect(const Leap::Controller& controller);
    //!Called each time a frame is captured by the Leap Motion
    void onFrame(const Leap::Controller& controller);
    //! Called when a Leap Motion service is unreachable (Crashed)
    void onServiceDisconnect(const Leap::Controller& controller);
    //! Destroyer
    virtual ~ZirkLeap(){
    }
    
    
private:
    ZirkOscjuceAudioProcessor *mFilter;
    ZirkOscjuceAudioProcessorEditor *mEditor;
    
    ScopedPointer<Leap::Controller> mController;
    
    int32_t mPointableId;
    //* Boolean telling if the last frame had a valid position
    bool mLastPositionValid;
    //* Vector giving the last frame's position
    Leap::Vector mLastPosition;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZirkLeap)
    
};


void updateLeapComponent(Component * leapComponent);

#endif  // OCTOLEAP_H_INCLUDED
