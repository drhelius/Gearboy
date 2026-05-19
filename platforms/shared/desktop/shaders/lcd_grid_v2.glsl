in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform vec4 SourceSize;
uniform vec4 OutputSize;
uniform float RSUBPIX_R;
uniform float RSUBPIX_G;
uniform float RSUBPIX_B;
uniform float GSUBPIX_R;
uniform float GSUBPIX_G;
uniform float GSUBPIX_B;
uniform float BSUBPIX_R;
uniform float BSUBPIX_G;
uniform float BSUBPIX_B;
uniform float gain;
uniform float gamma;
uniform float blacklevel;
uniform float ambient;
uniform float BGR;

const float OUT_GAMMA = 2.2;

vec3 fetch_pixel(ivec2 coord)
{
    return pow(vec3(gain) * texelFetch(Source, coord, 0).rgb + vec3(blacklevel), vec3(gamma)) + vec3(ambient);
}

float intsmear_func_x(float z)
{
    float z2 = z * z;
    float z3 = z * z2;
    float z5 = z3 * z2;
    float z7 = z5 * z2;
    float z9 = z7 * z2;
    float z11 = z9 * z2;
    float z13 = z11 * z2;

    return z - (2.0 / 3.0) * z3 - (1.0 / 5.0) * z5 + (4.0 / 7.0) * z7 -
        (1.0 / 9.0) * z9 - (2.0 / 11.0) * z11 + (1.0 / 13.0) * z13;
}

float intsmear_func_y(float z)
{
    float z2 = z * z;
    float z3 = z * z2;
    float z5 = z3 * z2;
    float z7 = z5 * z2;
    float z9 = z7 * z2;
    float z11 = z9 * z2;
    float z13 = z11 * z2;

    return z - (4.0 / 5.0) * z5 + (2.0 / 7.0) * z7 + (4.0 / 9.0) * z9 -
        (4.0 / 11.0) * z11 + (1.0 / 13.0) * z13;
}

float intsmear_x(float x, float dx, float d)
{
    float zl = clamp((x - dx * 0.5) / d, -1.0, 1.0);
    float zh = clamp((x + dx * 0.5) / d, -1.0, 1.0);
    return d * (intsmear_func_x(zh) - intsmear_func_x(zl)) / dx;
}

float intsmear_y(float x, float dx, float d)
{
    float zl = clamp((x - dx * 0.5) / d, -1.0, 1.0);
    float zh = clamp((x + dx * 0.5) / d, -1.0, 1.0);
    return d * (intsmear_func_y(zh) - intsmear_func_y(zl)) / dx;
}

void main()
{
    vec2 texel_size = SourceSize.zw;
    vec2 range = OutputSize.zw;

    vec3 cred = pow(vec3(RSUBPIX_R, RSUBPIX_G, RSUBPIX_B), vec3(OUT_GAMMA));
    vec3 cgreen = pow(vec3(GSUBPIX_R, GSUBPIX_G, GSUBPIX_B), vec3(OUT_GAMMA));
    vec3 cblue = pow(vec3(BSUBPIX_R, BSUBPIX_G, BSUBPIX_B), vec3(OUT_GAMMA));

    ivec2 tli = ivec2(floor(vTexCoord / texel_size - vec2(0.4999)));

    float subpix = (vTexCoord.x / texel_size.x - 0.4999 - float(tli.x)) * 3.0;
    float rsubpix = range.x / texel_size.x * 3.0;

    vec3 lcol = vec3(intsmear_x(subpix + 1.0, rsubpix, 1.5),
        intsmear_x(subpix, rsubpix, 1.5),
        intsmear_x(subpix - 1.0, rsubpix, 1.5));
    vec3 rcol = vec3(intsmear_x(subpix - 2.0, rsubpix, 1.5),
        intsmear_x(subpix - 3.0, rsubpix, 1.5),
        intsmear_x(subpix - 4.0, rsubpix, 1.5));

    if (BGR > 0.5)
    {
        lcol.rgb = lcol.bgr;
        rcol.rgb = rcol.bgr;
    }

    subpix = vTexCoord.y / texel_size.y - 0.4999 - float(tli.y);
    rsubpix = range.y / texel_size.y;
    float tcol = intsmear_y(subpix, rsubpix, 0.63);
    float bcol = intsmear_y(subpix - 1.0, rsubpix, 0.63);

    vec3 top_left_color = fetch_pixel(tli + ivec2(0, 0)) * lcol * vec3(tcol);
    vec3 bottom_right_color = fetch_pixel(tli + ivec2(1, 1)) * rcol * vec3(bcol);
    vec3 bottom_left_color = fetch_pixel(tli + ivec2(0, 1)) * lcol * vec3(bcol);
    vec3 top_right_color = fetch_pixel(tli + ivec2(1, 0)) * rcol * vec3(tcol);

    vec3 average_color = top_left_color + bottom_right_color + bottom_left_color + top_right_color;
    average_color = mat3(cred, cgreen, cblue) * average_color;

    FragColor = vec4(pow(average_color, vec3(1.0 / OUT_GAMMA)), 1.0);
}