# FREEMODE
### Alternative Control scheme for the 3Dsimo Kit pen

Freely set Temperature and Motor Speed.
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
- Upper line Left: Actual Temperature. (there seems to be a bug, shown minimum is 153 atm)
- Upper line Right: State of the nozzle: HEAT, COOL, DONE. You can only extrude in done or cooling states.
- Lower line: User input settings. Selected mode is shown inverted.
- Lower Left: Target temperaturen. (0 to 254°C (placeholder))
- Lower Right: Motor Speed setting. (0 - 100 %)
- Lower Middle: Icon that indicates how to change mode.

### Work in Progress
- Default Settings, Min and Max Settings are Placeholders. e.g.: It does not make sense to keep 0°C as Minimum Temperature.
- There is an [issue that the measured temperature on the display never shows below 153°C](https://github.com/3dsimo/3dsimo_kit/issues/4). The actual temperature can be below 153 degrees. This value, however, also drives the heating wich means that the pen thinks that set values like 70°C are already exceeded with 153°C (while, in reality, the tip doesn't heat at all).
- When using Up AND Down to switch between control modes: It can happen that you accidentally change the value due to single-button-presses. I will test it myself and if it is annoying I might try to prevent them.
- There might be bugs.
