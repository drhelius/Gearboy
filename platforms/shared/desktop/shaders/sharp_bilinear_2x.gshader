[Preset]
Name=Sharp Bilinear 2x
Passes=2

[Pass0]
Path=present.glsl
ScaleType=Source
Scale=2.0
Filter=Nearest

[Pass1]
Path=present.glsl
ScaleType=Viewport
Filter=Linear
