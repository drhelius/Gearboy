in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;

void main()
{
    FragColor = texture(Source, vTexCoord);
}
