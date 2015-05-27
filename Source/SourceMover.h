/*!
  ==============================================================================

    SourceMover.h
    Created: 8 Aug 2014 1:04:53pm
    Author:  makira
    Description:
    The SourceMover class allow you to easily and safely change to position of the source in the cirular space given by the dome.

  ==============================================================================
*/

#ifndef SOURCEMOVER_H_INCLUDED
#define SOURCEMOVER_H_INCLUDED

#include "PluginProcessor.h"

typedef enum
{
	kVacant,
	kField,
	kOsc,
	kLeap
} MoverType;

class SourceMover
{
public:
	SourceMover(ZirkOscjuceAudioProcessor *filter);
    void updateNumberOfSources();
	
	void begin(int s, MoverType mt);
	void move(FPoint p, MoverType mt);
	void end(MoverType mt);
	
private:
	ZirkOscjuceAudioProcessor *mFilter;
	MoverType mMover;
	int mSelectedItem;
	
	Array<FPoint> mSourcesDownXY;
	Array<FPoint> mSourcesDownRT;
	Array<float> mSourcesAngularOrder;
};




#endif  // SOURCEMOVER_H_INCLUDED
