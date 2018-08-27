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
