// Gearboy NTSC composite encode pass.
// Uses standard RGB/YIQ conversion and three-phase NTSC chroma modulation.

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform vec4 OutputSize;
uniform int FrameCount;

const float PI = 3.14159265;
const float CHROMA_MOD_FREQ = PI / 3.0;

vec3 rgb_to_yiq(vec3 color)
{
    return vec3(
            dot(color, vec3(0.2989, 0.5870, 0.1140)),
            dot(color, vec3(0.5959, -0.2744, -0.3216)),
            dot(color, vec3(0.2115, -0.5229, 0.3114)));
}

vec3 composite_mix(vec3 yiq)
{
    const float brightness = 1.0;
    const float saturation = 1.0;
    const float artifacting = 1.0;
    const float fringing = 1.0;

    return vec3(
            brightness * yiq.x + artifacting * yiq.y + artifacting * yiq.z,
            fringing * yiq.x + 2.0 * saturation * yiq.y,
            fringing * yiq.x + 2.0 * saturation * yiq.z);
}

void main()
{
    vec3 color = texture(Source, vTexCoord).rgb;
    vec3 yiq = rgb_to_yiq(color);
    vec2 pixel = vTexCoord * OutputSize.xy;
    float frame_phase = mod(float(FrameCount), 2.0);
    float chroma_phase = 0.6667 * PI * (mod(pixel.y, 3.0) + frame_phase);
    float mod_phase = chroma_phase + pixel.x * CHROMA_MOD_FREQ;
    vec2 modulation = vec2(cos(mod_phase), sin(mod_phase));

    yiq.yz *= modulation;
    yiq = composite_mix(yiq);
    yiq.yz *= modulation;

    FragColor = vec4(yiq, 1.0);
}