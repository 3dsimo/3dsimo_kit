# Material profiles
3Dsimo KIT already contains profiles for ABS and PLA of the box, but thanks to its nature, it can handle other materials easily as well. You can even add them yourself by changing our available code and adding your own material profiles.

See the [Firmware](Firmware.md) section on how to upload code. To add materials, look for the code *materials[]*, and add lines as you need them:

```C
  const profile_t materials[] PROGMEM = {  
    // {temperature (deg. C), motorSpeed (%), materialName}
       {0,                     0,             "OFF"},    /* BEGIN OFF - 3DPEN COOLS TO 153º */
       {210,                  40,             "PLA"},
       {210,                  60,             "PLA"},
       {210,                  80,             "PLA"},
       {230,                  30,             "ABS"},
       {230,                  50,             "ABS"},
       {230,                  70,             "ABS"},
       {235,                  30,             "PETG"},
       {235,                  50,             "PETG"},
       {235,                  70,             "PETG"}
  };
  /*
   *   define number of materials in list and variables
   */
  #define MATERIAL_COUNT  10
```

You can add lines to accomodate your plastics. **Carefull:** all lines, except the last, must end with a comma (,). Also, you need to give how many lines there are in the **MATERIAL_COUNT** variable. So if you add 2 lines, increase this variable by 2!

