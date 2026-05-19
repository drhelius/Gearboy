// Adapted from libretro slang-shaders handheld/gameboy pass4.
// Original shader copyright (C) 2013 Harlequin and 2024-2025 Matt Akins.

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform sampler2D PassOutput1;
uniform vec4 SourceSize;
uniform vec4 OutputSize;
uniform vec4 PassOutputSize1;
uniform float contrast;
uniform float screen_light;
uniform float pixel_opacity;
uniform float shadow_opacity;
uniform float shadow_offset_x;
uniform float shadow_offset_y;
uniform float screen_offset_x;
uniform float screen_offset_y;
uniform float shadow_enable;
uniform float palette;
uniform float shadow_scale;

const vec4 GAMEBOY_ORIGINAL_BACKGROUND = vec4(160.0 / 255.0, 170.0 / 255.0, 6.0 / 255.0, 1.0);
const vec4 GAMEBOY_BACKGROUND_TEXTURE = vec4(vec3(122.0 / 255.0), 1.0);

vec4 background_palette_color(void)
{
    if (palette < 0.5)
        return GAMEBOY_ORIGINAL_BACKGROUND;
    if (palette < 1.5)
        return vec4(0.651, 0.675, 0.518, 1.0);
    if (palette < 2.5)
        return vec4(0.737, 0.737, 0.737, 1.0);
    if (palette < 3.5)
        return vec4(1.0, 1.0, 1.0, 1.0);
    if (palette < 4.5)
        return vec4(0.627, 0.667, 0.024, 1.0);
    if (palette < 5.5)
        return vec4(0.027, 0.729, 0.369, 1.0);

    return vec4(0.027, 0.729, 0.608, 1.0);
}

void main()
{
    vec2 tex_coord = vTexCoord * 1.0001;
    vec2 foreground_tex = floor(PassOutputSize1.xy * tex_coord);
    foreground_tex = (foreground_tex + 0.5) * PassOutputSize1.zw;

    float scale_x = OutputSize.x / 640.0;
    float scale_y = OutputSize.y / 480.0;
    float resolution_scale = sqrt(scale_x * scale_y);
    float shadow_scale_factor = (shadow_scale > 0.5) ? resolution_scale : 1.0;
    vec2 texel = SourceSize.zw;
    vec2 shadow_offset = vec2(shadow_offset_x * texel.x * shadow_scale_factor,
                              shadow_offset_y * texel.y * shadow_scale_factor);
    vec2 screen_offset = vec2((screen_offset_x - 1.0) * texel.x,
                              (screen_offset_y - 1.0) * texel.y);

    vec4 background_palette = background_palette_color();
    vec4 foreground = texture(PassOutput1, foreground_tex - screen_offset);
    vec4 background = GAMEBOY_BACKGROUND_TEXTURE;
    vec4 shadows = texture(Source, tex_coord - (shadow_offset + screen_offset));

    foreground *= background_palette;

    float bg_test = (foreground.a > 0.0) ? 1.0 : 0.0;
    background -= (background - vec4(0.5)) * bg_test;
    background.rgb = clamp(background_palette.rgb + mix(vec3(-1.0), vec3(1.0), background.rgb), 0.0, 1.0);

    float shadow_alpha = contrast * shadow_opacity * shadow_enable;
    vec4 out_color = (shadows * shadows.a * shadow_alpha) + (background * (1.0 - shadows.a * shadow_alpha));
    out_color = (foreground * foreground.a * contrast) +
                (out_color * (screen_light - foreground.a * contrast * pixel_opacity));

    FragColor = vec4(out_color.rgb, 1.0);
}
