#!/bin/bash
dylibbundler -od -b -x ./Chauchera.app/Contents/MacOS/Chauchera -d ./Chauchera.app/Contents/libs/ -p @executable_path/../libs/
/usr/local/opt/qt/bin/macdeployqt Chauchera.app/
