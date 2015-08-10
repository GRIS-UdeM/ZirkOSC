#!/bin/bash

#VERSION="3.0.5"
VERSION="$1"

pushd ~/Library/Audio/Plug-Ins/Components/
zip -r ~/Desktop/ZirkOSC$VERSION.zip ./ZirkOSC3.component 
popd

pushd ~/Library/Audio/Plug-Ins/VST/
zip -r ~/Desktop/ZirkOSC$VERSION.zip ./ZirkOSC3.vst 
popd

#zip -rj ~/Desktop/ZirkOSC.zip ~/Library/Audio/Plug-Ins/Components/ZirkOSC3.component ~/Library/Audio/Plug-Ins/VST/ZirkOSC3.vst 

echo "vst and component copied to ~/Desktop"
