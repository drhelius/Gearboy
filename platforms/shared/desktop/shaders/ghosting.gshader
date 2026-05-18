[Preset]
Name=Ghosting
Passes=1

[Pass0]
Path=ghosting.glsl
ScaleType=Viewport
Filter=Nearest
Feedback=true
History=true

[Parameters]
Trails=0.5
FrameBlend=0.4

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
