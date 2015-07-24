/*
 * ==============================================================================
 *
 *  OctoLeap.cpp
 *  Created: 4 Aug 2014 1:23:01pm
 *  Authors:  makira & Antoine L.
 *
 * ==============================================================================
 */

#include <iostream>
#include "ZirkLeap.h"
#include "ZirkConstants.h"




#if WIN32
Component * CreateLeapComponent(OctogrisAudioProcessor *filter, OctogrisAudioProcessorEditor *editor)
{
    /** not implemented yet on windows*/
    return NULL;
}
#else

#include "Leap.h"

/** ZirkLeap constructor taking two arguments and initializing its others components by default */
ZirkLeap::ZirkLeap(ZirkOscAudioProcessor *filter, ZirkOscAudioProcessorEditor *editor):
ourProcessor(filter),
mEditor(editor),
mController(NULL),
mPointableId(-1),
mLastPositionValid(0)
{
    
    
}
/** onConnect is called when Leap is connected this method is only changing the interface because the Leap Controller is handling everything */
void ZirkLeap::onConnect(const Leap::Controller& controller)
{
    const MessageManagerLock mmLock;
    mEditor->getmStateLeap()->setText("Leap connected", dontSendNotification);
}
/** onServiceDisconnected is only called if the Leap application on the computer has gone very wrong */
void ZirkLeap::onServiceDisconnect(const Leap::Controller& controller )
{
    printf("Service Leap Disconnected");
}
//Not dispatched when running in a debugger

/** onDisconnect is called when Leap is disconnected this method is only changing the interface because the Leap Controller is handling everything */
void ZirkLeap::onDisconnect(const Leap::Controller& controller)
{
    const MessageManagerLock mmLock;
    mEditor->getmStateLeap()->setText("Leap disconnected", dontSendNotification);
}

/** onFrame is called for each frame that the Leap Motion capture even if nothing is detected. If a hand is detected
 then it process the coord and move the selected source */

void ZirkLeap::onFrame(const Leap::Controller& controller)
{
    if(controller.hasFocus())
    {
        
        Leap::Frame frame = controller.frame();
        if (mPointableId >= 0)
        {
            ourProcessor->setSelectedSource(mEditor->getCBSelectedSource()-1);
            Leap::Pointable p = frame.pointable(mPointableId);
            if (!p.isValid() || !p.isExtended())
            {
                mPointableId = -1;
                mLastPositionValid = false;
            }
            else
            {
                Leap::Vector pos = p.tipPosition();
                const float zPlane1 = 50;	// 5 cm
                const float zPlane2 = 100;	// 10 cm
                
                if (pos.z < zPlane2)
                {
                    if (mLastPositionValid)
                    {
                        
                        //Leap Motion mouvement are calculated from the last position in order to have something dynamic and ergonomic
                        Leap::Vector delta = pos- mLastPosition;
                        
                        float scale = 3;
                        if (pos.z > zPlane1)
                        {
                            float s = 1 - (pos.z - zPlane1) / (zPlane2 - zPlane1);
                            scale *= s;
                            
                        }
                        
                        int src = ourProcessor->getSelectedSource();
                        float fX, fY;
                        ourProcessor->getSources()[src].getXY(fX, fY);
                        fX += delta.x * scale;
                        fY -= delta.y * scale;
                        
                        mEditor->move(src, fX, fY);
                        
                    }
                    else
                    {
                        //std::cout << "pointable last pos not valid" << std::endl;
                        
                    }
                    
                    mLastPosition = pos;
                    mLastPositionValid = true;
                }
                else
                {
                    //std::cout << "pointable not touching plane" << std::endl;
                    mLastPositionValid = false;
                    
                }
            }
        }
        if (mPointableId < 0)
        {
            Leap::PointableList pl = frame.pointables().extended();
            if (pl.count() > 0)
            {
                mPointableId = pl[0].id();
                //std::cout << "got new pointable: " << mPointableId << std::endl;
            }
        }
    }
}


/** CreateLeapComponent is called to create a ZirkLeap instance through the ReferenceCountedObject so it is destroyed properly */
ZirkLeap::Ptr ZirkLeap::CreateLeapComponent(ZirkOscAudioProcessor *filter, ZirkOscAudioProcessorEditor *editor)
{
    return new ZirkLeap(filter, editor);
}


#endif
