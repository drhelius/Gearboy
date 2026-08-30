in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform vec4 OriginalSize;
uniform vec4 OutputSize;
uniform float GridIntensity;

const float GRID_LINE_START = 0.75;

void main()
{
    vec4 color = texture(Source, vTexCoord);
    vec2 source_pos = (gl_FragCoord.xy / OutputSize.xy) * OriginalSize.xy;
    vec2 grid_pos = step(vec2(GRID_LINE_START), fract(source_pos));
    float mask = max(grid_pos.x, grid_pos.y);

    color.rgb *= 1.0 - (GridIntensity * mask);
    FragColor = color;
}