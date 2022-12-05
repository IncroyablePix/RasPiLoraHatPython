#!/bin/sh

export LDFLAGS=-L/usr/local/opt/LoRaDraginoDriver/lib
export CPPFLAGS=-I/usr/local/opt/LoRaDraginoDriver/include/LoRaDraginoDriver

mkdir -p /usr/local/opt/LoRaDraginoDriver/lib
mkdir -p /usr/local/opt/LoRaDraginoDriver/include/LoRaDraginoDriver

mkdir build
cmake build .
make install

cp /usr/local/opt/LoRaDraginoDriver/lib/libLoRaDraginoDriver.so /usr/lib/libLoRaDraginoDriver.so