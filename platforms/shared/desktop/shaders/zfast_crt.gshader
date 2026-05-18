; Zfast CRT preset for Gearboy.
; This is a derivative work based on zfast CRT by Greg Hogan / SoltanGris42.
; Original shader licensed GPL v2 or later.

[Preset]
Name=Zfast CRT
Passes=1

[Pass0]
Path=zfast_crt.glsl
ScaleType=Viewport
Filter=Linear

[Parameters]
BlurAmountX=0.30
ScanlineDarknessLow=6.0
ScanlineDarknessHigh=8.0
DarkPixelBoost=1.25
MaskAmount=0.25
MaskFade=0.8

[Parameter.BlurAmountX]
Label=Blur X
Min=0.0
Max=1.0
Step=0.05

[Parameter.ScanlineDarknessLow]
Label=Dark Scanlines
Min=0.0
Max=10.0
Step=0.5

[Parameter.ScanlineDarknessHigh]
Label=Bright Scanlines
Min=0.0
Max=50.0
Step=1.0

[Parameter.DarkPixelBoost]
Label=Dark Boost
Min=0.5
Max=1.5
Step=0.05

[Parameter.MaskAmount]
Label=Mask
Min=0.0
Max=1.0
Step=0.05

[Parameter.MaskFade]
Label=Mask Fade
Min=0.0
Max=1.0
Step=0.05