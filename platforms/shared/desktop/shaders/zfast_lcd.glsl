in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform vec4 SourceSize;
uniform float BORDERMULT;
uniform float GBAGAMMA;

void main()
{
    vec2 texcoord_in_pixels = vTexCoord * SourceSize.xy;
    vec2 center_coord = floor(texcoord_in_pixels) + vec2(0.5);
    vec2 dist_from_center = abs(center_coord - texcoord_in_pixels);

    float y = max(dist_from_center.x, dist_from_center.y);
    y *= y;
    float yy = y * y;
    float yyy = yy * y;

    float line_weight = yy - 2.7 * yyy;
    line_weight = 1.0 - BORDERMULT * line_weight;

    vec3 color = texture(Source, SourceSize.zw * center_coord).rgb * line_weight;

    if (GBAGAMMA > 0.5)
        color *= 0.6 + 0.4 * color;

    FragColor = vec4(color.rgb, 1.0);
}