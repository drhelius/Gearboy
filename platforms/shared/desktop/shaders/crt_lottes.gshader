; CRT Lottes preset for Gearboy.
; Public domain CRT styled scan-line shader by Timothy Lottes.

[Preset]
Name=CRT Lottes
Passes=1

[Pass0]
Path=crt_lottes.glsl
ScaleType=Viewport
Filter=Nearest

[Parameters]
HardScan=-8.0
HardPix=-3.0
WarpX=0.031
WarpY=0.041
MaskDark=0.5
MaskLight=1.5
ScaleInLinearGamma=1.0
ShadowMask=3.0
BrightBoost=1.0
HardBloomPix=-1.5
HardBloomScan=-2.0
BloomAmount=0.1
Shape=2.0

[Parameter.HardScan]
Label=Hard Scan
Min=-20.0
Max=0.0
Step=1.0

[Parameter.HardPix]
Label=Hard Pixel
Min=-20.0
Max=0.0
Step=1.0

[Parameter.WarpX]
Label=Warp X
Min=0.0
Max=0.125
Step=0.01

[Parameter.WarpY]
Label=Warp Y
Min=0.0
Max=0.125
Step=0.01

[Parameter.MaskDark]
Label=Mask Dark
Min=0.0
Max=2.0
Step=0.1

[Parameter.MaskLight]
Label=Mask Light
Min=0.0
Max=2.0
Step=0.1

[Parameter.ScaleInLinearGamma]
Label=Linear Gamma
Min=0.0
Max=1.0
Step=1.0

[Parameter.ShadowMask]
Label=Shadow Mask
Min=0.0
Max=4.0
Step=1.0

[Parameter.BrightBoost]
Label=Brightness
Min=0.0
Max=2.0
Step=0.05

[Parameter.HardBloomPix]
Label=Bloom X Soft
Min=-2.0
Max=-0.5
Step=0.1

[Parameter.HardBloomScan]
Label=Bloom Y Soft
Min=-4.0
Max=-1.0
Step=0.1

[Parameter.BloomAmount]
Label=Bloom Amount
Min=0.0
Max=1.0
Step=0.05

[Parameter.Shape]
Label=Kernel Shape
Min=0.0
Max=10.0
Step=0.05