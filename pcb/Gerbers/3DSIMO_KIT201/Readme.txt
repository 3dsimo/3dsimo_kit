I order Rigid Board with these parameters and files:

Name:               3DSIMO_KIT201
Customer:           3DSimo

PCB Parameters:
Type:               Rigid FR4
Number of layers:   2 rigid 
Final thickness:    1.6 mm
Copper thickness:   35um
Coating:            HAL Free
Soldermask TOP:		Black
Soldermask BOT:		Black
Silkscreen TOP:		White
Silkscreen Bottom:  White
Dimensions:			17 x 85 mm

Input Files:
bot.gbr             Copper - BOTTOM side
top.gbr             Copper - TOP side   
mill.gbr            Milling of PCB
plt.gbr             Silkscreen - TOP
plb.gbr             Silkscreen - BOTTOM
smb.gbr             Solder mask - BOTTOM side
smt.gbr             Solder mask - TOP side
npth.exc            Non plated drills
pth.exc             Plated drills
paste_top.gbr       SMD stencil - TOP


Copper order:
TOP
BOTTOM

Output testing:
Electrical testing: YES
