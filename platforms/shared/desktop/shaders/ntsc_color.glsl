// Gearboy NTSC color artifact pass.
// Adds YIQ color response and phase-based rainbow artifacts without composite blur or scanlines.

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform vec4 SourceSize;
uniform int FrameCount;
uniform float NtscArtifacts;
uniform float NtscSaturation;
uniform float NtscContrast;
uniform float NtscShimmer;

const float PI = 3.14159265;
const float CHROMA_MOD_FREQ = PI / 3.0;

vec3 rgb_to_yiq(vec3 color)
{
    return vec3(
            dot(color, vec3(0.2989, 0.5870, 0.1140)),
            dot(color, vec3(0.5959, -0.2744, -0.3216)),
            dot(color, vec3(0.2115, -0.5229, 0.3114)));
}

vec3 yiq_to_rgb(vec3 yiq)
{
    return vec3(
            yiq.x + 0.9560 * yiq.y + 0.6210 * yiq.z,
            yiq.x - 0.2720 * yiq.y - 0.6474 * yiq.z,
            yiq.x - 1.1060 * yiq.y + 1.7046 * yiq.z);
}

void main()
{
    vec4 source = texture(Source, vTexCoord);
    vec3 left = texture(Source, vTexCoord - vec2(SourceSize.z, 0.0)).rgb;
    vec3 right = texture(Source, vTexCoord + vec2(SourceSize.z, 0.0)).rgb;

    vec3 yiq = rgb_to_yiq(source.rgb);
    vec3 yiq_left = rgb_to_yiq(left);
    vec3 yiq_right = rgb_to_yiq(right);

    vec2 pixel = vTexCoord * SourceSize.xy;
    float frame_phase = mod(float(FrameCount), 2.0);
    float chroma_phase = 0.6667 * PI * (mod(pixel.y, 3.0) + frame_phase);
    float mod_phase = chroma_phase + pixel.x * CHROMA_MOD_FREQ;
    vec2 carrier = vec2(cos(mod_phase), sin(mod_phase));
    vec2 carrier_quadrature = vec2(-carrier.y, carrier.x);

    float luma_high = yiq.x - 0.5 * (yiq_left.x + yiq_right.x);
    float luma_edge = 0.5 * (yiq_right.x - yiq_left.x);
    vec2 artifact_chroma = carrier * luma_high * 0.55 + carrier_quadrature * luma_edge * 0.35;
    vec3 phase_yiq = vec3(
            yiq.x + yiq.y * carrier.x + yiq.z * carrier.y,
            (yiq.x + 2.0 * yiq.y * carrier.x) * carrier.x,
            (yiq.x + 2.0 * yiq.z * carrier.y) * carrier.y);
    float shimmer_gate = clamp(abs(luma_high) * 6.0 + abs(luma_edge) * 4.0 + length(yiq.yz) * 0.25, 0.0, 1.0);
    vec3 shimmer_yiq = (phase_yiq - yiq) * shimmer_gate;
    float intensity = clamp(NtscArtifacts, 0.0, 1.0);
    float shimmer = clamp(NtscShimmer, 0.0, 1.0);

    yiq.x = (yiq.x - 0.5) * NtscContrast + 0.5;
    yiq.x += shimmer_yiq.x * shimmer * intensity * 0.035;
    yiq.yz *= NtscSaturation;
    yiq.yz += artifact_chroma * NtscArtifacts;
    yiq.yz += shimmer_yiq.yz * shimmer * intensity * 0.12;

    vec3 corrected = clamp(yiq_to_rgb(yiq), vec3(0.0), vec3(1.0));

    FragColor = vec4(mix(source.rgb, corrected, intensity), source.a);
}