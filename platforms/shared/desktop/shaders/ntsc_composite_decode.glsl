// Gearboy NTSC composite decode pass.
// Horizontally filters the modulated YIQ signal and converts it back to RGB.

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform vec4 SourceSize;

vec3 yiq_to_rgb(vec3 yiq)
{
    return vec3(
            yiq.x + 0.9560 * yiq.y + 0.6210 * yiq.z,
            yiq.x - 0.2720 * yiq.y - 0.6474 * yiq.z,
            yiq.x - 1.1060 * yiq.y + 1.7046 * yiq.z);
}

float gaussian(float offset, float sigma)
{
    return exp(-0.5 * offset * offset / (sigma * sigma));
}

bool source_coord_valid(vec2 coord)
{
    if (coord.x < 0.0 || coord.x > 1.0 || coord.y < 0.0 || coord.y > 1.0)
        return false;

    return true;
}

void main()
{
    vec2 tex = vTexCoord - vec2(0.5 * SourceSize.z, 0.0);
    vec3 signal = vec3(0.0);
    float luma_weight_sum = 0.0;
    float chroma_weight_sum = 0.0;

    for (int i = -12; i <= 12; i++)
    {
        float offset = float(i);
        vec2 sample_coord = tex + vec2(offset * SourceSize.z, 0.0);
        float luma_weight = gaussian(offset, 3.0);
        float chroma_weight = gaussian(offset, 7.0);

        if (source_coord_valid(sample_coord))
        {
            vec3 sample_yiq = texture(Source, sample_coord).rgb;
            signal.x += sample_yiq.x * luma_weight;
            signal.yz += sample_yiq.yz * chroma_weight;
            luma_weight_sum += luma_weight;
            chroma_weight_sum += chroma_weight;
        }
    }

    if (luma_weight_sum > 0.0)
        signal.x /= luma_weight_sum;
    if (chroma_weight_sum > 0.0)
        signal.yz /= chroma_weight_sum;

    FragColor = vec4(clamp(yiq_to_rgb(signal), vec3(0.0), vec3(1.0)), 1.0);
}