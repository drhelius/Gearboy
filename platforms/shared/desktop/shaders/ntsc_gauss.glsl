// Gearboy NTSC vertical gaussian and display gamma pass.

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform vec4 SourceSize;
uniform float NtscCrtGamma;
uniform float NtscDisplayGamma;
uniform float NtscScanlineIntensity;

bool source_coord_valid(vec2 coord)
{
    if (coord.x < 0.0 || coord.x > 1.0 || coord.y < 0.0 || coord.y > 1.0)
        return false;

    return true;
}

vec3 sample_gamma_coord(vec2 coord)
{
    vec3 color = max(texture(Source, coord).rgb, vec3(0.0));
    return pow(color, vec3(NtscCrtGamma));
}

void add_scanline_sample(inout vec3 scanline, inout float valid_weight_sum, float offset, float weight)
{
    vec2 coord = vTexCoord + vec2(0.0, offset * SourceSize.w);

    if (source_coord_valid(coord))
    {
        scanline += sample_gamma_coord(coord) * weight;
        valid_weight_sum += weight;
    }
}

float scanline_weight(float distance)
{
    return exp(-5.0 * distance * distance);
}

void main()
{
    float offset_dist = fract(vTexCoord.y * SourceSize.y) - 0.5;
    float dist0 = 2.0 + offset_dist;
    float dist1 = 1.0 + offset_dist;
    float dist2 = offset_dist;
    float dist3 = -1.0 + offset_dist;
    float dist4 = -2.0 + offset_dist;

    float weight0 = scanline_weight(dist0);
    float weight1 = scanline_weight(dist1);
    float weight2 = scanline_weight(dist2);
    float weight3 = scanline_weight(dist3);
    float weight4 = scanline_weight(dist4);
    float total_weight_sum = weight0 + weight1 + weight2 + weight3 + weight4;
    float valid_weight_sum = 0.0;
    vec3 scanline = vec3(0.0);

    add_scanline_sample(scanline, valid_weight_sum, -2.0, weight0);
    add_scanline_sample(scanline, valid_weight_sum, -1.0, weight1);
    add_scanline_sample(scanline, valid_weight_sum, 0.0, weight2);
    add_scanline_sample(scanline, valid_weight_sum, 1.0, weight3);
    add_scanline_sample(scanline, valid_weight_sum, 2.0, weight4);

    if (valid_weight_sum > 0.0)
        scanline *= total_weight_sum / valid_weight_sum;

    vec3 base = sample_gamma_coord(vTexCoord);
    float scanline_intensity = clamp(NtscScanlineIntensity, 0.0, 1.0);
    vec3 color = mix(base, scanline, scanline_intensity);

    vec3 display_gamma = vec3(1.0 / max(NtscDisplayGamma, 0.001));
    FragColor = vec4(pow(max(1.15 * color, vec3(0.0)), display_gamma), 1.0);
}