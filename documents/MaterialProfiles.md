# Material profiles
3Dsimo KIT already contains profiles for ABS and PLA of the box, but thanks to its nature, it can handle other materials easily as well. You can even add them yourself by changing our available code and adding your own material profiles.

### Getting started
- [Here is a step by step guide to connect the Arduino IDE with your 3Dsimo KIT](pdf/HW_FW_manualEN.pdf)
- [Here you can download the Arduino IDE](https://www.arduino.cc/en/Main/Software)
- [And here you can find the code (Arduino "sketch") to upload to your 3Dsimo KIT pen.](https://github.com/AnTi-ArT/3dsimo_kit/tree/patch-1/FW) You can pick one (eg the newest) file from the subfolder 3DsimoKit. The place to add new material profiles is near the lines 50~60. Also, make sure to adjust #define MATERIAL_COUNT according to your edited materials array.
