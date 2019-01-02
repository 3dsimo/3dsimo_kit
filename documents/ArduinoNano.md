# Arduino nano
3Dsimo Kit is based on very popular Arduino Nano. However, to allow the current draw needed for heating, **customization is REQUIRED**

Specifically, to allow heating of the nozzle with enough power, the diode at the miniUSB input must be replaced
with interconnect (0 Ohm resistor). The placement of the diode is shown (red) in the following picture.
If this modification is not performed, the diode will overload and burn and the device would become
non-functional. The board, which is included in the 3Dsimo Kit, already has this modification.

**Pic: Placement of the input diode, shown already replaced with a 0 Ohm resistor**
![Customized Nano](img/customized_nano.png?raw=true "Customized Nano")

The Nano used in the kit has processor **ATmega 168**, which will need to be selected in the Arduino IDE.

To change and upload the code, see the [FW 3Dsimo Kit](Firmware.md) Section
