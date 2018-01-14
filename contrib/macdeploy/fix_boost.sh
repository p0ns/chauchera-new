#!/bin/bash
fullpath=$(pwd)/$1
echo $fullpath
cp /usr/local/opt/boost/lib/libboost_system-mt.dylib $fullpath/Contents/Frameworks/
install_name_tool -change @loader_path/libboost_system-mt.dylib @executable_path/../Frameworks/libboost_system-mt.dylib $fullpath/Contents/Frameworks/libboost_thread-mt.dylib
install_name_tool -change @loader_path/libboost_system-mt.dylib @executable_path/../Frameworks/libboost_system-mt.dylib $fullpath/Contents/Frameworks/libboost_chrono-mt.dylib
install_name_tool -change @loader_path/libboost_system.dylib @executable_path/../Frameworks/libboost_system.dylib $fullpath/Contents/Frameworks/libboost_filesystem.dylib
