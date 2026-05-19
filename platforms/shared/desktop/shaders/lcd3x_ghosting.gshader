[Preset]
Name=LCD 3x + Ghosting
Passes=2

[Pass0]
Path=ghosting.glsl
ScaleType=Viewport
Filter=Nearest
Feedback=true
History=true

[Pass1]
Path=lcd3x.glsl
ScaleType=Viewport
Filter=Nearest

[Parameters]
Trails=0.7
FrameBlend=0.7
brighten_scanlines=16.0
brighten_lcd=4.0

[Parameter.Trails]
Label=Trails
Min=0.0
Max=0.98
Step=0.01

[Parameter.FrameBlend]
Label=Frame Blend
Min=0.0
Max=1.0
Step=0.05

[Parameter.brighten_scanlines]
Label=Brighten Scanlines
Min=1.0
Max=32.0
Step=0.5

[Parameter.brighten_lcd]
Label=Brighten LCD
Min=1.0
Max=12.0
Step=0.1