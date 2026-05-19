in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;

void main()
{
    FragColor = vec4(pow(texture(Source, vTexCoord).rgb, vec3(2.2)), 1.0);
}