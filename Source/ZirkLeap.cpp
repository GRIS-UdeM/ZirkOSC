/*
  ==============================================================================

    OctoLeap.cpp
    Created: 4 Aug 2014 1:23:01pm
    Author:  makira

  ==============================================================================
*/

#include <iostream>
#include "ZirkLeap.h"
#include "ZirkConstants.h"




#if JUCE_WINDOWS
Component * CreateLeapComponent(OctogrisAudioProcessor *filter, OctogrisAudioProcessorEditor *editor)
{
	// not implemented yet on windows
	return NULL;
}
#else

#include "Leap.h"

	ZirkLeap::ZirkLeap(ZirkOscjuceAudioProcessor *filter, ZirkOscjuceAudioProcessorEditor *editor):
		mFilter(filter),
		mEditor(editor),
		mController(NULL),
		mPointableId(-1),
		mLastPositionValid(0)
    {

		
	}

	void ZirkLeap::onConnect(const Leap::Controller& controller)
	{
		const MessageManagerLock mmLock;
		mEditor->getmStateLeap()->setText("Leap connected", dontSendNotification);
	}
    void ZirkLeap::onServiceDisconnect(const Leap::Controller& controller )
    {
        printf("Service Leap Disconnected");
    }
	//Not dispatched when running in a debugger
	void ZirkLeap::onDisconnect(const Leap::Controller& controller)
	{
		const MessageManagerLock mmLock;
        mEditor->getmStateLeap()->setText("Leap disconnected", dontSendNotification);	}

	void ZirkLeap::onFrame(const Leap::Controller& controller)
	{
		//std::cout << "New frame available" << std::endl;
        
        if(controller.hasFocus())
        {
            
            Leap::Frame frame = controller.frame();
            if (mPointableId >= 0)
            {
                mFilter->setSelectedSource(mEditor->getCBSelectedSource()-1);
                Leap::Pointable p = frame.pointable(mPointableId);
                if (!p.isValid() || !p.isExtended())
                {
                    mPointableId = -1;
                    mLastPositionValid = false;
                    //std::cout << "pointable not valid or not extended" << std::endl;
                    
                    mEditor->getMover()->end(kLeap);
                }
                else
                {
                    Leap::Vector pos = p.tipPosition();
                    //std::cout << "x: " << pos.x << " y: " << pos.y << " z: " << pos.z << std::endl;
                    
                    const float zPlane1 = 50;	// 5 cm
                    const float zPlane2 = 100;	// 10 cm
                    
                    if (pos.z < zPlane2)
                    {
                        if (mLastPositionValid)
                        {
                            Leap::Vector delta = pos- mLastPosition;
                            //std::cout << "dx: " << delta.x << " dy: " << delta.y << " dz: " << delta.z << std::endl;
                            
                            float scale = 2.5;
                            if (pos.z > zPlane1)
                            {
                                float s = 1 - (pos.z - zPlane1) / (zPlane2 - zPlane1);
                                scale *= s;
                                
                            }
                            //std::cout << "scale: " << scale << std::endl;
                            
                            int src = mEditor->getOscLeapSource()-1;
                            
                            Point<float> sp = mFilter->getSources()[src].getPositionXY();
                            sp.x += delta.x * scale;
                            sp.y -= delta.y * scale;
                            //sp.x = 0.5;
                            //sp.y = 0.5;
                            int selectedConstraint = mFilter->getSelectedMovementConstraintAsInteger();
                            //mEditor->getMover()->move(sp, kLeap);
                            if(selectedConstraint == Independant)
                            {
                                
                                mFilter->getSources()[src].setPositionXY(sp);
                                mFilter->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + src*5, mFilter->getSources()[src].getAzimuth());
                                mFilter->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + src*5, mFilter->getSources()[src].getElevation());
                                //send source position by osc
                                mFilter->sendOSCValues();
                                
                            }
                            else if (selectedConstraint == FixedAngles){
                                mEditor->moveFixedAngles(sp);
                            }
                            else if (selectedConstraint == FixedRadius){
                                mEditor->moveCircularWithFixedRadius(sp);
                            }
                            else if (selectedConstraint == FullyFixed){
                                mEditor->moveFullyFixed(sp);
                            }
                            else if (selectedConstraint == DeltaLocked){
                                Point<float> DeltaMove = sp - mFilter->getSources()[src].getPositionXY();
                                mEditor->moveSourcesWithDelta(DeltaMove);
                            }
                            else if (selectedConstraint == Circular){
                                mEditor->moveCircular(sp);
                            }

                            
                            mEditor->fieldChanged();
                        }
                        else
                        {
                            //std::cout << "pointable last pos not valid" << std::endl;
                            mEditor->getMover()->begin(mEditor->getOscLeapSource(), kLeap);
                            
                        }
                        
                        mLastPosition = pos;
                        mLastPositionValid = true;
                    }
                    else
                    {
                        //std::cout << "pointable not touching plane" << std::endl;
                        mLastPositionValid = false;
                        
                        mEditor->getMover()->end(kLeap);
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
    



ZirkLeap::Ptr ZirkLeap::CreateLeapComponent(ZirkOscjuceAudioProcessor *filter, ZirkOscjuceAudioProcessorEditor *editor)
{
	return new ZirkLeap(filter, editor);
}


#endif
