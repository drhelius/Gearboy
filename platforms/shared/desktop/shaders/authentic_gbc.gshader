; Authentic GBC preset adapted from libretro slang-shaders handheld/authentic_gbc by fishku.
[Preset]
Name=Authentic GBC
Passes=2

[Pass0]
Path=authentic_gbc_to_lin.glsl
ScaleType=Source
Filter=Nearest
FloatFramebuffer=true

[Pass1]
Path=authentic_gbc.glsl
ScaleType=Viewport
Filter=Nearest

[Parameters]
AUTH_GBC_BRIG=0.6
AUTH_GBC_BLUR=0.3
AUTH_GBC_SUBPX=0.0
AUTH_GBC_SUBPX_ORIENTATION=0.0

[Parameter.AUTH_GBC_BRIG]
Label=Add brightness
Min=0.0
Max=1.0
Step=0.05

[Parameter.AUTH_GBC_BLUR]
Label=Anti-banding smoothing
Min=0.0
Max=1.0
Step=0.05

[Parameter.AUTH_GBC_SUBPX]
Label=Enable subpixel rendering
Min=0.0
Max=1.0
Step=1.0

[Parameter.AUTH_GBC_SUBPX_ORIENTATION]
Label=Subpixel layout
Min=0.0
Max=3.0
Step=1.0