in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform vec4 OriginalSize;
uniform vec4 OutputSize;
uniform float ScanlineIntensity;

void main()
{
    vec4 color = texture(Source, vTexCoord);
    float source_y = (gl_FragCoord.y / OutputSize.y) * OriginalSize.y;
    float mask = fract(source_y) >= 0.5 ? 1.0 : 0.0;
    color.rgb *= 1.0 - (ScanlineIntensity * mask);
    FragColor = color;
}