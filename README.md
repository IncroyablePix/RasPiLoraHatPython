# 👨‍💻 Dragino LoRa/GPS HAT for Raspberry Pi Driver and Python module 👨‍💻
This driver/python module was based on [the official Dragino LoRa/GPS HAT example code](https://github.com/dragino/rpi-lora-tranceiver), for the [Dragino LoRa/GPS hat for Raspberry Pi](https://www.dragino.com/products/lora/item/106-lora-gps-hat.html).
This source code is provided as is and is granted with no warranty of any kind. Use at your own risk.

## Installation
The installation is pretty easy and has a few requirements.

### Requirements
* Raspberry Pi 2+ with Dragino LoRa/GPS HAT
* Python 3.6+
* CMake 3.16+
* WiringPi (Probably already installed on your Raspberry Pi.)

### Installation
1. Clone this repository
2. Move to the cloned directory
3. Run `mkdir build && cmake build . && make`

This will give you access to the **lora** module in Python. Since the driver is written in C, you may also write your application in C/C++ as depicted in the Sample directory.

## Usage
The **lora** module has a few functions that you can use to interact with the LoRa/GPS HAT.
