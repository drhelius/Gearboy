[Preset]
Name=NTSC Composite
Passes=4

[Pass0]
Path=ntsc_composite_encode.glsl
ScaleType=Source
ScaleX=4.0
ScaleY=1.0
Filter=Nearest
FloatFramebuffer=true

[Pass1]
Path=ntsc_composite_decode.glsl
ScaleType=Previous
ScaleX=0.5
ScaleY=1.0
Filter=Nearest

[Pass2]
Path=ntsc_gauss.glsl
ScaleTypeX=Previous
ScaleTypeY=Viewport
Scale=1.0
Filter=Nearest

[Pass3]
Path=present.glsl
ScaleType=Viewport
Filter=Linear

[Parameters]
NtscCrtGamma=2.50
NtscDisplayGamma=2.10
NtscScanlineIntensity=1.00

[Parameter.NtscCrtGamma]
Label=CRT Gamma
Min=1.0
Max=4.0
Step=0.05

[Parameter.NtscDisplayGamma]
Label=Display Gamma
Min=1.0
Max=4.0
Step=0.05

[Parameter.NtscScanlineIntensity]
Label=Scanlines
Min=0.0
Max=1.0
Step=0.05