#!/bin/bash

rm -rf build/
mkdir build
cd build

if [ "$1" == "Arduino" ]; then
	cmake -DUseArduino=TRUE ..
else
	cmake ..
fi
