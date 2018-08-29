# FREEMODE
### Alternative Control scheme for the 3Dsimo Kit pen

Freely set Temperature and Motor Speed. For right-handed use, see below how to change for left-hand.   
[How to change the firmware on the 3Dsimo Kit Pen.](../../documents/pdf/HW_FW_manualEN.pdf)  
⚠️ Be very careful when setting the temperature and speed. Wrong settings can damage the nozzle. Use the profile settings from the official firmware as a starting point (notice how the speed is around 40%). If the heat is too low and the speed is too high your nozzle can overflow! ⚠️

#### Buttons:
- Extrude Pressed: extrudes filament (when hot).
- Extrude Release: Filament will be pulled back slightly.
- Extrude AND Revert: Pauses the motor without pull back.
- Revert Pressed (you can release) ejects the filament.
- Up AND Down pressed: Will change the control mode. Release one or both buttons and push both again to change back.
- Up Released: Increase either Temperature or Motor Speed.
- Down Released: Decrease either Temperature or Motor Speed.

#### Screen:
- Screen is tiled in four parts.
- Upper line Left: Actual Temperature. Or "LOW" if it is below 153°C.
- Upper line Right: State of the nozzle: HEAT, COOL, DONE, OFF!. You can only extrude in done or cooling states.
- Lower line: User input settings. Selected mode is shown inverted.
- Lower Left: Target temperatures: "LOW", 155 - 255
- Lower Right: Motor Speed setting. (0 - 100 %)
- Lower Middle: Icon that indicates how to change mode.

#### OFF mode:
- ⚠️ WARNING: "LOW" or "OFF!" does not indicate that the nozzle IS cold. It needs some time to cool down, even if LOW is diplayed.
- [The temperature measurement can't go below 153°C on an unmodified 3Dsimo Kit](https://github.com/3dsimo/3dsimo_kit/issues/4).
- Freemode will start in OFF mode. Extruding is prevented and the pen tries to cool down to target 0°C.
- Increasing the Temperature one step with the UP button will directly set 155°C as target temperature and the pen heats up. If the pen reaches 155°C and more, it will display the actual temperature (might take a moment).
- In reverse: Going one step DOWN from 155°C target temperature will set 0°C as next target. The heater will turn OFF. As soon as the actual, measured temperature falls below 155°C it will diplay "LOW". ⚠️ TAKE CARE!! 154°C is still pretty hot to the touch! Cooling down takes a while, too.

#### Left-handed use:
- It is easy to change the code to left-hand mode. Just follow those lines:
- see line 24
- see line 462
- This will rotate the display and change the UP and DOWN buttons to fit the display reading direction.

### Work in Progress
- When using Up AND Down to switch between control modes: It can happen that you accidentally change the value due to single-button-presses. I will test it myself and if it is annoying I might try to prevent them.
- There might be bugs. 🐞

