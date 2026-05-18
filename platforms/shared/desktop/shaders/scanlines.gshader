[Preset]
Name=Scanlines
Passes=1

[Pass0]
Path=scanlines.glsl
ScaleType=Viewport
Filter=Nearest

[Parameters]
ScanlineIntensity=0.10

[Parameter.ScanlineIntensity]
Label=Intensity
Min=0.0
Max=1.0
Step=0.01
