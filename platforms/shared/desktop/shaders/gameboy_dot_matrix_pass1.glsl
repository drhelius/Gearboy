// Adapted from libretro slang-shaders handheld/gameboy pass1.
// Original shader copyright (C) 2013 Harlequin and 2024-2025 Matt Akins.

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform vec4 SourceSize;
uniform vec4 OutputSize;
uniform float adjacent_texel_alpha_blending;

float blending_modifier(float alpha)
{
    return (alpha <= 0.0) ? 1.0 : 0.0;
}

void main()
{
    vec2 tex_coord = vTexCoord * 1.0001;
    vec2 texel = SourceSize.zw;
    vec2 lower_bound = vec2(0.0);
    vec2 upper_bound = texel * (OutputSize.xy - vec2(2.0));
    vec4 out_color = texture(Source, tex_coord);

    vec2 blur_coords_up = clamp(tex_coord - vec2(0.0, texel.y), lower_bound, upper_bound);
    vec2 blur_coords_down = clamp(tex_coord + vec2(0.0, texel.y), lower_bound, upper_bound);
    vec2 blur_coords_right = clamp(tex_coord + vec2(texel.x, 0.0), lower_bound, upper_bound);
    vec2 blur_coords_left = clamp(tex_coord - vec2(texel.x, 0.0), lower_bound, upper_bound);

    vec4 adjacent_texel_1 = texture(Source, blur_coords_up);
    vec4 adjacent_texel_2 = texture(Source, blur_coords_down);
    vec4 adjacent_texel_3 = texture(Source, blur_coords_right);
    vec4 adjacent_texel_4 = texture(Source, blur_coords_left);

    out_color.a -= ((out_color.a - adjacent_texel_1.a) +
                    (out_color.a - adjacent_texel_2.a) +
                    (out_color.a - adjacent_texel_3.a) +
                    (out_color.a - adjacent_texel_4.a)) *
                   adjacent_texel_alpha_blending * blending_modifier(out_color.a);

    FragColor = out_color;
}
