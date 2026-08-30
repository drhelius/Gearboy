// Author: rsn8887 (based on TheMaister)
// License: Public domain
// Source: https://github.com/rsn8887/Sharp-Bilinear-Shaders
// Ported to Gearboy GLSL preset uniforms.

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform vec4 SourceSize;
uniform vec4 OutputSize;

vec2 sharp_bilinear_coord()
{
    vec2 texel = vTexCoord * SourceSize.xy;
    vec2 scale = max(floor(OutputSize.xy / SourceSize.xy), vec2(1.0));
    vec2 texel_floored = floor(texel);
    vec2 center_dist = fract(texel) - 0.5;
    vec2 region_range = 0.5 - 0.5 / scale;
    vec2 offset = (center_dist - clamp(center_dist, -region_range, region_range)) * scale + 0.5;

    return (texel_floored + offset) * SourceSize.zw;
}

void main()
{
    FragColor = vec4(texture(Source, sharp_bilinear_coord()).rgb, 1.0);
}
