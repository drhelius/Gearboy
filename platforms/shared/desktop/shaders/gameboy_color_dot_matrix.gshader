; Game Boy Color dot matrix preset adapted from libretro slang-shaders handheld/gameboy-color-dot-matrix.

[Preset]
Name=Game Boy Color Dot Matrix
Passes=5

[Pass0]
Path=gameboy_color_dot_matrix_pass0.glsl
ScaleType=Viewport
Filter=Nearest
History=true

[Pass1]
Path=gameboy_dot_matrix_pass1.glsl
ScaleType=Previous
Filter=Nearest

[Pass2]
Path=gameboy_dot_matrix_pass2.glsl
ScaleType=Previous
Filter=Nearest

[Pass3]
Path=gameboy_dot_matrix_pass3.glsl
ScaleType=Previous
Filter=Nearest

[Pass4]
Path=gameboy_color_dot_matrix_pass4.glsl
ScaleType=Previous
Filter=Nearest

[Parameters]
integer_mode=1.0
grey_balance=3.0
contrast=0.95
baseline_alpha=0.10
pixel_opacity=1.0
adjacent_texel_alpha_blending=0.1755
sharp_mode=1.0
pixel_softness=1.0
pixel_shape=1.0
sharpening_amount=1.0
pixel_size=0.80
response_time=0.33
screen_light=1.0
shadow_enable=1.0
shadow_opacity=0.55
shadow_offset_x=1.0
shadow_offset_y=1.0
shadow_scale=1.0
screen_offset_x=0.0
screen_offset_y=0.0

[Parameter.integer_mode]
Label=Square Pixels
Min=0.0
Max=1.0
Step=1.0

[Parameter.grey_balance]
Label=Grey Balance
Min=0.0
Max=4.0
Step=0.1

[Parameter.contrast]
Label=Contrast
Min=0.0
Max=1.0
Step=0.05

[Parameter.baseline_alpha]
Label=Base Pixel Alpha
Min=0.0
Max=1.0
Step=0.01

[Parameter.pixel_opacity]
Label=Pixel Opacity
Min=0.01
Max=1.0
Step=0.01

[Parameter.adjacent_texel_alpha_blending]
Label=Blend Amount
Min=0.0
Max=1.0
Step=0.05

[Parameter.sharp_mode]
Label=Sharp Mode
Min=0.0
Max=1.0
Step=1.0

[Parameter.pixel_softness]
Label=Pixel Softness
Min=0.20
Max=5.0
Step=0.05

[Parameter.pixel_shape]
Label=Pixel Shape
Min=0.0
Max=1.30
Step=0.05

[Parameter.sharpening_amount]
Label=Sharpening
Min=0.0
Max=1.0
Step=0.05

[Parameter.pixel_size]
Label=Pixel Size
Min=0.20
Max=1.10
Step=0.05

[Parameter.response_time]
Label=Response Time
Min=0.0
Max=0.777
Step=0.111

[Parameter.screen_light]
Label=Screen Light
Min=0.0
Max=2.0
Step=0.05

[Parameter.shadow_enable]
Label=Shadow
Min=0.0
Max=1.0
Step=1.0

[Parameter.shadow_opacity]
Label=Shadow Opacity
Min=0.0
Max=1.0
Step=0.05

[Parameter.shadow_offset_x]
Label=Shadow Offset X
Min=-8.0
Max=8.0
Step=0.25

[Parameter.shadow_offset_y]
Label=Shadow Offset Y
Min=-8.0
Max=8.0
Step=0.25

[Parameter.shadow_scale]
Label=Scale Shadow Offset
Min=0.0
Max=1.0
Step=1.0

[Parameter.screen_offset_x]
Label=Screen Offset X
Min=-8.0
Max=8.0
Step=0.25

[Parameter.screen_offset_y]
Label=Screen Offset Y
Min=-8.0
Max=8.0
Step=0.25