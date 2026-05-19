[Preset]
Name=Grid + Ghosting
Passes=2

[Pass0]
Path=ghosting.glsl
ScaleType=Viewport
Filter=Nearest
Feedback=true
History=true

[Pass1]
Path=grid.glsl
ScaleType=Viewport
Filter=Nearest

[Parameters]
Trails=0.7
FrameBlend=0.7
GridIntensity=0.10

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

[Parameter.GridIntensity]
Label=Grid
Min=0.0
Max=1.0
Step=0.01