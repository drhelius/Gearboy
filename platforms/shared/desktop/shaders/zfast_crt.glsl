in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform vec4 SourceSize;
uniform vec4 OutputSize;
uniform float BlurAmountX;
uniform float ScanlineDarknessLow;
uniform float ScanlineDarknessHigh;
uniform float DarkPixelBoost;
uniform float MaskAmount;
uniform float MaskFade;

void main()
{
    vec2 pixel = vTexCoord * SourceSize.xy;
    vec2 center = floor(pixel) + vec2(0.5);
    vec2 phase = pixel - center;
    vec2 sample_uv = (center + 4.0 * phase * phase * phase) * SourceSize.zw;
    sample_uv.x = mix(sample_uv.x, vTexCoord.x, BlurAmountX);

    vec3 color = texture(Source, sample_uv).rgb;

    float y = phase.y * phase.y;
    float yy = y * y;

    float mask_phase = fract(gl_FragCoord.x * -0.4999);
    float mask = 1.0 - ((mask_phase < 0.5) ? MaskAmount : 0.0);

    float scanline_dark = DarkPixelBoost - ScanlineDarknessLow * (y - 2.05 * yy);
    float scanline_bright = 1.0 - ScanlineDarknessHigh * (yy - 2.8 * yy * y);
    float brightness = dot(color, vec3(0.3333333 * MaskFade));
    float weight = mix(scanline_dark * mask, scanline_bright, brightness);

    FragColor = vec4(color * weight, 1.0);
}