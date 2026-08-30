in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform vec4 OriginalSize;
uniform float brighten_scanlines;
uniform float brighten_lcd;

const float PI = 3.141592654;
const vec3 OFFSETS = vec3(PI) * vec3(1.0 / 2.0, 1.0 / 2.0 - 2.0 / 3.0, 1.0 / 2.0 - 4.0 / 3.0);

void main()
{
    vec3 source_color = texture(Source, vTexCoord).rgb;
    vec2 angle = vTexCoord * PI * 2.0 * OriginalSize.xy;

    float yfactor = (brighten_scanlines + sin(angle.y)) / (brighten_scanlines + 1.0);
    vec3 xfactors = (brighten_lcd + sin(angle.x + OFFSETS)) / (brighten_lcd + 1.0);
    vec3 color = yfactor * xfactors * source_color;

    FragColor = vec4(color.r, color.g, color.b, 1.0);
}