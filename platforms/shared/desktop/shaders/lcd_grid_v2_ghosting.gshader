[Preset]
Name=LCD Grid v2 + Ghosting
Passes=2

[Pass0]
Path=ghosting_history.glsl
ScaleType=Source
Filter=Nearest
History=true

[Pass1]
Path=lcd_grid_v2.glsl
ScaleType=Viewport
Filter=Nearest

[Parameters]
Trails=0.7
FrameBlend=0.7
RSUBPIX_R=0.75
RSUBPIX_G=0.0
RSUBPIX_B=0.0
GSUBPIX_R=0.0
GSUBPIX_G=0.75
GSUBPIX_B=0.0
BSUBPIX_R=0.0
BSUBPIX_G=0.0
BSUBPIX_B=0.75
gain=1.5
gamma=2.2
blacklevel=0.0
ambient=0.0
BGR=0.0

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

[Parameter.RSUBPIX_R]
Label=R subpixel red
Min=0.0
Max=1.0
Step=0.01

[Parameter.RSUBPIX_G]
Label=R subpixel green
Min=0.0
Max=1.0
Step=0.01

[Parameter.RSUBPIX_B]
Label=R subpixel blue
Min=0.0
Max=1.0
Step=0.01

[Parameter.GSUBPIX_R]
Label=G subpixel red
Min=0.0
Max=1.0
Step=0.01

[Parameter.GSUBPIX_G]
Label=G subpixel green
Min=0.0
Max=1.0
Step=0.01

[Parameter.GSUBPIX_B]
Label=G subpixel blue
Min=0.0
Max=1.0
Step=0.01

[Parameter.BSUBPIX_R]
Label=B subpixel red
Min=0.0
Max=1.0
Step=0.01

[Parameter.BSUBPIX_G]
Label=B subpixel green
Min=0.0
Max=1.0
Step=0.01

[Parameter.BSUBPIX_B]
Label=B subpixel blue
Min=0.0
Max=1.0
Step=0.01

[Parameter.gain]
Label=Gain
Min=0.5
Max=2.0
Step=0.05

[Parameter.gamma]
Label=LCD Gamma
Min=0.5
Max=5.0
Step=0.1

[Parameter.blacklevel]
Label=Black level
Min=0.0
Max=0.5
Step=0.01

[Parameter.ambient]
Label=Ambient
Min=0.0
Max=0.5
Step=0.01

[Parameter.BGR]
Label=BGR
Min=0.0
Max=1.0
Step=1.0