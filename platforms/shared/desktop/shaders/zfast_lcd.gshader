; Zfast LCD preset adapted from libretro slang-shaders handheld/zfast-lcd by Greg Hogan (SoltanGris42).
[Preset]
Name=Zfast LCD
Passes=1

[Pass0]
Path=zfast_lcd.glsl
ScaleType=Viewport
Filter=Linear

[Parameters]
BORDERMULT=14.0
GBAGAMMA=1.0

[Parameter.BORDERMULT]
Label=Border Multiplier
Min=-40.0
Max=40.0
Step=1.0

[Parameter.GBAGAMMA]
Label=GBA Gamma Hack
Min=0.0
Max=1.0
Step=1.0