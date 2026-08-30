; LCD 3x preset adapted from libretro slang-shaders handheld/lcd3x by Gigaherz.
[Preset]
Name=LCD 3x
Passes=1

[Pass0]
Path=lcd3x.glsl
ScaleType=Viewport
Filter=Nearest

[Parameters]
brighten_scanlines=16.0
brighten_lcd=4.0

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