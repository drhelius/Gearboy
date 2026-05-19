[Preset]
Name=Grid + Bilinear
Passes=1

[Pass0]
Path=grid.glsl
ScaleType=Viewport
Filter=Linear

[Parameters]
GridIntensity=0.10

[Parameter.GridIntensity]
Label=Intensity
Min=0.0
Max=1.0
Step=0.01