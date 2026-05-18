// Public domain CRT styled scan-line shader by Timothy Lottes.
// Ported to Gearboy GLSL preset uniforms.

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform vec4 SourceSize;
uniform vec4 OutputSize;
uniform vec4 BackgroundColor;
uniform float HardScan;
uniform float HardPix;
uniform float WarpX;
uniform float WarpY;
uniform float MaskDark;
uniform float MaskLight;
uniform float ScaleInLinearGamma;
uniform float ShadowMask;
uniform float BrightBoost;
uniform float HardBloomPix;
uniform float HardBloomScan;
uniform float BloomAmount;
uniform float Shape;

float to_linear_1(float color)
{
    if (ScaleInLinearGamma < 0.5)
        return color;

    return color <= 0.04045 ? color / 12.92 : pow((color + 0.055) / 1.055, 2.4);
}

vec3 to_linear(vec3 color)
{
    if (ScaleInLinearGamma < 0.5)
        return color;

    return vec3(to_linear_1(color.r), to_linear_1(color.g), to_linear_1(color.b));
}

float to_srgb_1(float color)
{
    if (ScaleInLinearGamma < 0.5)
        return color;

    return color < 0.0031308 ? color * 12.92 : 1.055 * pow(max(color, 0.0), 0.41666) - 0.055;
}

vec3 to_srgb(vec3 color)
{
    if (ScaleInLinearGamma < 0.5)
        return color;

    return vec3(to_srgb_1(color.r), to_srgb_1(color.g), to_srgb_1(color.b));
}

vec3 fetch_source(vec2 pos, vec2 offset)
{
    vec2 uv = (floor(pos * SourceSize.xy + offset) + vec2(0.5)) * SourceSize.zw;
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
        return to_linear(BackgroundColor.rgb);

    return to_linear(BrightBoost * texture(Source, uv).rgb);
}

vec2 dist_to_texel(vec2 pos)
{
    pos *= SourceSize.xy;
    return -((pos - floor(pos)) - vec2(0.5));
}

float gaussian(float pos, float scale)
{
    return exp2(scale * pow(abs(pos), Shape));
}

vec3 horz3(vec2 pos, float offset)
{
    vec3 sample_left = fetch_source(pos, vec2(-1.0, offset));
    vec3 sample_center = fetch_source(pos, vec2( 0.0, offset));
    vec3 sample_right = fetch_source(pos, vec2( 1.0, offset));

    float dst = dist_to_texel(pos).x;
    float wb = gaussian(dst - 1.0, HardPix);
    float wc = gaussian(dst, HardPix);
    float wd = gaussian(dst + 1.0, HardPix);

    return (sample_left * wb + sample_center * wc + sample_right * wd) / (wb + wc + wd);
}

vec3 horz5(vec2 pos, float offset)
{
    vec3 sample_left_far = fetch_source(pos, vec2(-2.0, offset));
    vec3 sample_left = fetch_source(pos, vec2(-1.0, offset));
    vec3 sample_center = fetch_source(pos, vec2( 0.0, offset));
    vec3 sample_right = fetch_source(pos, vec2( 1.0, offset));
    vec3 sample_right_far = fetch_source(pos, vec2( 2.0, offset));

    float dst = dist_to_texel(pos).x;
    float wa = gaussian(dst - 2.0, HardPix);
    float wb = gaussian(dst - 1.0, HardPix);
    float wc = gaussian(dst, HardPix);
    float wd = gaussian(dst + 1.0, HardPix);
    float we = gaussian(dst + 2.0, HardPix);

    return (sample_left_far * wa + sample_left * wb + sample_center * wc + sample_right * wd + sample_right_far * we) / (wa + wb + wc + wd + we);
}

vec3 horz7(vec2 pos, float offset)
{
    vec3 sample_left_outer = fetch_source(pos, vec2(-3.0, offset));
    vec3 sample_left_far = fetch_source(pos, vec2(-2.0, offset));
    vec3 sample_left = fetch_source(pos, vec2(-1.0, offset));
    vec3 sample_center = fetch_source(pos, vec2( 0.0, offset));
    vec3 sample_right = fetch_source(pos, vec2( 1.0, offset));
    vec3 sample_right_far = fetch_source(pos, vec2( 2.0, offset));
    vec3 sample_right_outer = fetch_source(pos, vec2( 3.0, offset));

    float dst = dist_to_texel(pos).x;
    float wa = gaussian(dst - 3.0, HardBloomPix);
    float wb = gaussian(dst - 2.0, HardBloomPix);
    float wc = gaussian(dst - 1.0, HardBloomPix);
    float wd = gaussian(dst, HardBloomPix);
    float we = gaussian(dst + 1.0, HardBloomPix);
    float wf = gaussian(dst + 2.0, HardBloomPix);
    float wg = gaussian(dst + 3.0, HardBloomPix);

    return (sample_left_outer * wa + sample_left_far * wb + sample_left * wc + sample_center * wd + sample_right * we + sample_right_far * wf + sample_right_outer * wg) / (wa + wb + wc + wd + we + wf + wg);
}

float scan(vec2 pos, float offset)
{
    float dst = dist_to_texel(pos).y;
    return gaussian(dst + offset, HardScan);
}

float bloom_scan(vec2 pos, float offset)
{
    float dst = dist_to_texel(pos).y;
    return gaussian(dst + offset, HardBloomScan);
}

vec3 tri(vec2 pos)
{
    vec3 sample_up = horz3(pos, -1.0);
    vec3 sample_center = horz5(pos,  0.0);
    vec3 sample_down = horz3(pos,  1.0);

    float wa = scan(pos, -1.0);
    float wb = scan(pos,  0.0);
    float wc = scan(pos,  1.0);

    return sample_up * wa + sample_center * wb + sample_down * wc;
}

vec3 bloom(vec2 pos)
{
    vec3 sample_up_far = horz5(pos, -2.0);
    vec3 sample_up = horz7(pos, -1.0);
    vec3 sample_center = horz7(pos,  0.0);
    vec3 sample_down = horz7(pos,  1.0);
    vec3 sample_down_far = horz5(pos,  2.0);

    float wa = bloom_scan(pos, -2.0);
    float wb = bloom_scan(pos, -1.0);
    float wc = bloom_scan(pos,  0.0);
    float wd = bloom_scan(pos,  1.0);
    float we = bloom_scan(pos,  2.0);

    return sample_up_far * wa + sample_up * wb + sample_center * wc + sample_down * wd + sample_down_far * we;
}

vec2 warp_position(vec2 pos)
{
    pos = pos * 2.0 - 1.0;
    pos *= vec2(1.0 + pos.y * pos.y * WarpX, 1.0 + pos.x * pos.x * WarpY);
    return pos * 0.5 + 0.5;
}

vec3 mask(vec2 pos)
{
    vec3 result = vec3(MaskDark);

    if (ShadowMask < 1.5)
    {
        float mask_line = MaskLight;
        float odd = fract(pos.x / 6.0) < 0.5 ? 1.0 : 0.0;
        if (fract((pos.y + odd) / 2.0) < 0.5)
            mask_line = MaskDark;

        pos.x = fract(pos.x / 3.0);
        if (pos.x < 0.333)
            result.r = MaskLight;
        else if (pos.x < 0.666)
            result.g = MaskLight;
        else
            result.b = MaskLight;

        result *= mask_line;
    }
    else if (ShadowMask < 2.5)
    {
        pos.x = fract(pos.x / 3.0);
        if (pos.x < 0.333)
            result.r = MaskLight;
        else if (pos.x < 0.666)
            result.g = MaskLight;
        else
            result.b = MaskLight;
    }
    else if (ShadowMask < 3.5)
    {
        pos.x += pos.y * 3.0;
        pos.x = fract(pos.x / 6.0);
        if (pos.x < 0.333)
            result.r = MaskLight;
        else if (pos.x < 0.666)
            result.g = MaskLight;
        else
            result.b = MaskLight;
    }
    else
    {
        pos = floor(pos * vec2(1.0, 0.5));
        pos.x += pos.y * 3.0;
        pos.x = fract(pos.x / 6.0);
        if (pos.x < 0.333)
            result.r = MaskLight;
        else if (pos.x < 0.666)
            result.g = MaskLight;
        else
            result.b = MaskLight;
    }

    return result;
}

void main()
{
    vec2 pos = warp_position(vTexCoord);
    if (pos.x < 0.0 || pos.x > 1.0 || pos.y < 0.0 || pos.y > 1.0)
    {
        FragColor = vec4(BackgroundColor.rgb, 1.0);
        return;
    }

    vec3 color = tri(pos);

    if (BloomAmount > 0.0001)
        color += bloom(pos) * BloomAmount;

    if (ShadowMask >= 0.5)
        color *= mask(floor(vTexCoord * OutputSize.xy) + vec2(0.5));

    FragColor = vec4(to_srgb(max(color, vec3(0.0))), 1.0);
}