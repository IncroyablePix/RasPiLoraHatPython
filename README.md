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
3. Run `sudo chmod +x ./install.sh && sudo ./install.sh`

This will give you access to the **lora** module in Python. Since the driver is written in C, you may also write your application in C/C++ as depicted in the Sample directory.

## Usage
The **lorapy** module has a few features that you can use to interact with the LoRa/GPS HAT. lorapy exports one class : LoRaCom.

### LoRaCom
This class is the main and only class of the lorapy module. It is used to interact with the LoRa/GPS HAT.

#### Constructor
The constructor takes 2 arguments:
* **frequency**: The frequency to use for the LoRa communication. This is an integer value in Hz. For Europe you can set 868000000.
* **sf**: The spreading factor to use for the LoRa communication. This is an integer value. You could simply use 7.

#### Methods
* **set_on_receive**: This method is used to set the callback function that will be called when a message is received. It thus takes one argument:
  * **on_receive**: The callback function that will be called when a message is received. This function takes 1 argument:
    * **payload**: The payload of the received message as a Python string.

* **listen**: This method is used to start listening. It takes 1 argument:
  * **multithread**: Whether or not the function fires a new thread to listen. If no value is provided, it defaults to True. 

* **send**: This method is used to send a message. It takes 1 argument:
  * **payload**: The payload to send as a Python string.

* **stop**: This method is used to stop listening. It takes no argument.

⛔ The send function is currently not stable and might not work as expected! ⛔

#### Single or multithreaded
You can run the **listen** method in a separate thread (**default**) or in blocking mode.
In multithreaded mode, the listen method will not return until the LoRaCom instance is "stopped" with the **stop** method.

Example of multithreaded mode:
```python
from lorapy import LoRaCom

running = True
com = LoRaCom(868_000_000, 7)

def on_message(msg: str):
  global running
  print(msg)
  com.stop()
  running = False
  
com.set_on_receive(on_message)
com.listen(True)

while running:
  pass # Do whatever you want to do...
```

Make sure that you call the stop method *before* you exit the program, otherwise the program will not exit properly and raise a SegFault.

Example of blocking mode:
```python
from lorapy import LoRaCom

com = LoRaCom(868_000_000, 7)

def on_message(msg: str):
  print(msg)
  com.stop()

com.set_on_receive(on_message)
com.listen(False)
```
