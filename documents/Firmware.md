# Firmware for 3Dsimo Kit
The firmware for the open source 3D pen, is available in [this repository](../FW/3DsimoKit). 

Download from there the *3DsimoKit.ino* file and open it with the [Arduino IDE](http://arduino.cc/software). The code is fully available and commented so you don't have trouble finding your way around.

## Installing necessary libraries
The code uses 2 libraries you need to install. You can do that from menu Sketch => Include Library => Manage Libraries. 

A dialog opens in which you can search libraries.

  1. search for ssd1306, and select the *ssd1306* library from *Alexey Dynda*. We installed version 1.7.12 in our tests
  ![ssd1306 lib](img/ssd_lib.png?raw=true "ssd1306 lib")
  2. do the same with library *EveryTimer* by *Alessio Leoncini*.   We installed version  1.1.1 in our tests

## Uploading the code
**WARNING:** the nano used is customized. If you use another nano, **you** need to customize it, see [Arduino nano](documents/ArduinoNano.md) Section

If you have the code open in the Arduino IDE, connect the Nano device via a usb cable to your PC. Select in Tools, Board *Arduino Nano*:
![Select Nano](img/ard_ide_01.png?raw=true "Select Nano")

The processor of our nano is the *ATmega168*, so select this in the processor part of Tools menu:
![Select Processor](img/ard_ide_02.png?raw=true "Select Processor")

Compile the code. All should work without errors. Then upload the code to the nano.

## Troubleshooting

1. Your PC does not connect to the nano, or uploading keeps failing.

This should not happen. Use google to search for your error and possible solutions. One possibility is a damaged Nano, a new one will need to be used then.

2. The Nano resets all the time

You can verify this by changing the profile. If the profile switches back to the start up profile, the nano has reset. The cause of this is too high current draw which the power source cannot provide, causing a voltage drop and a reset. 

**Solution:** use a usb power source which can provide 2A of current (eg from tablet, or dedicated usb power source).
