// Adapted from libretro slang-shaders handheld/gameboy pass2.
// Original shader copyright (C) 2013 Harlequin and 2024-2025 Matt Akins.

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform vec4 SourceSize;
uniform vec4 OutputSize;

void main()
{
    vec2 tex_coord = vTexCoord * 1.0001;
    vec2 texel = SourceSize.zw;
    vec2 lower_bound = vec2(0.0);
    vec2 upper_bound = texel * (OutputSize.xy - vec2(1.0));

    const float weight0 = 0.13465834124289954;
    const float weight1 = 0.13051534237555914;
    const float weight2 = 0.11883557904592230;
    const float weight3 = 0.10164546793794160;
    const float weight4 = 0.08167444001912719;

    vec4 out_color = texture(Source, clamp(tex_coord, lower_bound, upper_bound)) * weight0;
    out_color.a += texture(Source, clamp(tex_coord + vec2(texel.x, 0.0), lower_bound, upper_bound)).a * weight1;
    out_color.a += texture(Source, clamp(tex_coord - vec2(texel.x, 0.0), lower_bound, upper_bound)).a * weight1;
    out_color.a += texture(Source, clamp(tex_coord + vec2(texel.x * 2.0, 0.0), lower_bound, upper_bound)).a * weight2;
    out_color.a += texture(Source, clamp(tex_coord - vec2(texel.x * 2.0, 0.0), lower_bound, upper_bound)).a * weight2;
    out_color.a += texture(Source, clamp(tex_coord + vec2(texel.x * 3.0, 0.0), lower_bound, upper_bound)).a * weight3;
    out_color.a += texture(Source, clamp(tex_coord - vec2(texel.x * 3.0, 0.0), lower_bound, upper_bound)).a * weight3;
    out_color.a += texture(Source, clamp(tex_coord + vec2(texel.x * 4.0, 0.0), lower_bound, upper_bound)).a * weight4;
    out_color.a += texture(Source, clamp(tex_coord - vec2(texel.x * 4.0, 0.0), lower_bound, upper_bound)).a * weight4;

    FragColor = out_color;
}
