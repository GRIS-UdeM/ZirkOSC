//
//  Osc.h
//  ZirkOSCJUCE
//
//  Created by Lud's on 3/05/13.
//
//

#ifndef __ZirkOSCJUCE__Osc__
#define __ZirkOSCJUCE__Osc__

#include <iostream>
#include "../JuceLibraryCode/JuceHeader.h"
#include "lo.h"
#include "SoundSource.h"

using namespace std;
class Osc{
public:
    Osc(String addressExternalController, String portExternalController, String portIncomming, String portZirkonium, SoundSource* s, int& numberSource);
    ~Osc();

    bool sendSourcePosition (SoundSource* s);
    bool sendBeginMovePosition (SoundSource* s);
    bool sendEndMovePosition (SoundSource* s);

    bool sendValueAzimuthSpan (SoundSource* s);
    bool sendBeginAzimuthSpan (SoundSource* s);
    bool sendEndAzimuthSpan (SoundSource* s);

    bool sendValueElevationSpan (SoundSource* s);
    bool sendBeginElevationSpan (SoundSource* s);
    bool sendEndElevationSpan (SoundSource* s);

    bool isListenning () { return _listenning; }
    void setListenning (bool l) { _listenning = l; }

    String getAddressExternalController (String s) { return _AddressExternalController; }
    bool setAddressExternalController (String s);

    String getPortIncoming (String s) { return _PortIncoming; }
    bool setPortIncoming (String s);

    String getPortZirkonium (String s) { return _PortZirkonium; }
    bool setPortZirkonium (String s);

private:
    String _AddressExternalController;
    String _PortIncoming;
    String _PortExternalController;
    String _PortZirkonium;
    bool _listenning;
};


#endif /* defined(__ZirkOSCJUCE__Osc__) */
