#!/bin/sh

rm -r build
cmake --build . --target clean
pip3 uninstall lorapy -y