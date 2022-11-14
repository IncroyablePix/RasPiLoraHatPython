#!/bin/sh

export LDFLAGS=-L/usr/local/opt/LoRaDraginoDriver/lib
export CPPFLAGS=-I/usr/local/opt/LoRaDraginoDriver/include

mkdir build
cmake build .
make install