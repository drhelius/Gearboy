; Zfast CRT + NTSC + Ghosting preset for Gearboy.
; Combines the Gearboy NTSC color artifact pass, zfast CRT, and feedback ghosting.
; The zfast CRT pass is a derivative work based on zfast CRT by Greg Hogan / SoltanGris42.
; Original zfast CRT shader licensed GPL v2 or later.

[Preset]
Name=Zfast CRT + NTSC + Ghosting
Passes=3

[Pass0]
Path=ntsc_color.glsl
ScaleType=Source
Filter=Nearest

[Pass1]
Path=zfast_crt.glsl
ScaleType=Viewport
Filter=Linear

[Pass2]
Path=ghosting.glsl
ScaleType=Viewport
Filter=Nearest
Feedback=true
History=true

[Parameters]
NtscArtifacts=1.00
NtscSaturation=0.85
NtscContrast=1.05
NtscShimmer=0.75
BlurAmountX=0.30
ScanlineDarknessLow=6.0
ScanlineDarknessHigh=8.0
DarkPixelBoost=1.25
MaskAmount=0.25
MaskFade=0.8
Trails=0.5
FrameBlend=0.4

[Parameter.NtscArtifacts]
Label=NTSC Artifacts
Min=0.0
Max=1.0
Step=0.05

[Parameter.NtscSaturation]
Label=NTSC Saturation
Min=0.0
Max=2.0
Step=0.05

[Parameter.NtscContrast]
Label=NTSC Contrast
Min=0.5
Max=1.5
Step=0.05

[Parameter.NtscShimmer]
Label=NTSC Shimmer
Min=0.0
Max=1.0
Step=0.05

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