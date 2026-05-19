in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform vec4 OriginalSize;
uniform vec4 OutputSize;
uniform float AUTH_GBC_BRIG;
uniform float AUTH_GBC_BLUR;
uniform float AUTH_GBC_SUBPX;
uniform float AUTH_GBC_SUBPX_ORIENTATION;

vec4 lcd_subpx_rect1;
vec4 lcd_subpx_rect2;
vec2 subpx_offset_in_px;
vec2 tx_coord;
vec2 tx_to_px;
vec2 tx_orig_offs;
float eff_blur_in_px;

float intersect_blurred_rect_area(vec4 px_square, vec4 rect, float blur)
{
    vec4 range = (rect.zw - rect.xy).xyxy;
    vec4 linear = clamp(px_square - rect.xyxy, vec4(0.0), range);

    if (blur < 1.0e-6)
        return (linear.z - linear.x) * (linear.w - linear.y);

    vec4 center = (0.5 * (rect.xy + rect.zw)).xyxy;
    vec4 dist_to_center = abs(px_square - center);
    vec4 blur_vec = vec4(blur);

    vec4 x_n = max(0.5 * (max(range, blur_vec) + blur_vec) - dist_to_center, vec4(0.0)) / blur_vec;
    const vec3 c = vec3(-0.3635, 0.727, 0.1365);
    vec4 poly = ((c.xxxx * x_n + c.yyyy) * x_n + c.zzzz) * x_n * x_n * min(range, blur_vec);
    vec4 transition = mix(poly, range - poly, step(center, px_square));

    vec4 res = mix(linear, transition, step(0.5 * (range - blur_vec), dist_to_center));
    return (res.z - res.x) * (res.w - res.y);
}

void calculate_lcd_params(vec2 source_size, vec2 output_size, float use_subpx, int subpx_orientation, float brightness_boost, vec2 tex_coord)
{
    const vec4 rot_corr = vec4(1.0, 0.0, -1.0, 0.0);
    subpx_offset_in_px = use_subpx / 3.0 * vec2(rot_corr[subpx_orientation], rot_corr[(subpx_orientation + 3) % 4]);

    tx_coord = tex_coord * source_size;
    tx_to_px = output_size / source_size;

    const vec2 subpx_ratio = vec2(0.296, 0.910);
    const vec2 notch_ratio = vec2(0.115, 0.166);

    vec2 lcd_subpx_size_in_px = tx_to_px * mix(subpx_ratio, vec2(0.75, 0.93), brightness_boost);
    vec2 notch_size_in_px = tx_to_px * mix(notch_ratio, vec2(0.29, 0.17), brightness_boost);
    lcd_subpx_rect1 = vec4(vec2(0.0), lcd_subpx_size_in_px - vec2(0.0, notch_size_in_px.y));
    lcd_subpx_rect2 = vec4(notch_size_in_px.x, lcd_subpx_size_in_px.y - notch_size_in_px.y, lcd_subpx_size_in_px);

    tx_orig_offs = (tx_to_px - lcd_subpx_size_in_px) * 0.5;
}

float subpx_coverage(vec4 px_square, vec2 subpx_orig)
{
    return intersect_blurred_rect_area(px_square, subpx_orig.xyxy + lcd_subpx_rect1, eff_blur_in_px) +
        intersect_blurred_rect_area(px_square, subpx_orig.xyxy + lcd_subpx_rect2, eff_blur_in_px);
}

vec3 pixel_color(vec2 tx_orig)
{
    return vec3(subpx_coverage(vec4(-subpx_offset_in_px - 0.5, -subpx_offset_in_px + 0.5),
            tx_orig + vec2(tx_orig_offs.x - tx_to_px.x / 3.0, tx_orig_offs.y)),
        subpx_coverage(vec4(vec2(-0.5), vec2(0.5)), tx_orig + tx_orig_offs),
        subpx_coverage(vec4(subpx_offset_in_px - 0.5, subpx_offset_in_px + 0.5),
            tx_orig + vec2(tx_orig_offs.x + tx_to_px.x / 3.0, tx_orig_offs.y)));
}

vec3 sample_pixel(vec2 texel_coord)
{
    return texture(Source, (texel_coord + 0.5) * OriginalSize.zw).rgb;
}

void main()
{
    int subpx_orientation = int(clamp(floor(AUTH_GBC_SUBPX_ORIENTATION + 0.5), 0.0, 3.0));
    calculate_lcd_params(OriginalSize.xy, OutputSize.xy, AUTH_GBC_SUBPX, subpx_orientation, AUTH_GBC_BRIG, vTexCoord);
    eff_blur_in_px = AUTH_GBC_BLUR * min(tx_to_px.x, tx_to_px.y) * 0.5;

    vec2 tx_coord_i = floor(tx_coord);
    vec2 tx_coord_f = tx_coord - tx_coord_i;
    vec2 tx_coord_off = step(vec2(0.5), tx_coord_f) * 2.0 - 1.0;

    vec2 offset0 = vec2(0.0);
    vec2 offset1 = vec2(tx_coord_off.x, 0.0);
    vec2 offset2 = vec2(0.0, tx_coord_off.y);
    vec2 offset3 = tx_coord_off;

    vec3 sample0 = sample_pixel(tx_coord_i + offset0);
    vec3 sample1 = sample_pixel(tx_coord_i + offset1);
    vec3 sample2 = sample_pixel(tx_coord_i + offset2);
    vec3 sample3 = sample_pixel(tx_coord_i + offset3);

    vec3 res = pixel_color((offset0 - tx_coord_f) * tx_to_px) * sample0 +
        pixel_color((offset1 - tx_coord_f) * tx_to_px) * sample1 +
        pixel_color((offset2 - tx_coord_f) * tx_to_px) * sample2 +
        pixel_color((offset3 - tx_coord_f) * tx_to_px) * sample3;

    FragColor = vec4(pow(res, vec3(1.0 / 2.2)), 1.0);
}