// Adapted from libretro slang-shaders handheld/gameboy pass0.
// Original shader copyright (C) 2013 Harlequin and 2024-2025 Matt Akins.

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform sampler2D SourceHistory0;
uniform sampler2D SourceHistory1;
uniform sampler2D SourceHistory2;
uniform sampler2D SourceHistory3;
uniform sampler2D SourceHistory4;
uniform sampler2D SourceHistory5;
uniform sampler2D SourceHistory6;
uniform vec4 SourceSize;
uniform vec4 OutputSize;
uniform int SourceHistoryCount;
uniform float pixel_size;
uniform float pixel_softness;
uniform float sharpening_amount;
uniform float integer_mode;
uniform float baseline_alpha;
uniform float grey_balance;
uniform float response_time;
uniform float sharp_mode;
uniform float pixel_shape;

const float GAMEBOY_INTEGER_SCALE_FACTOR = 5.0;

float intersect_line_segment(float pixel_start, float pixel_end, float dot_start, float dot_end)
{
    float overlap_start = max(pixel_start, dot_start);
    float overlap_end = min(pixel_end, dot_end);
    return max(overlap_end - overlap_start, 0.0);
}

float intersect_rect_area(vec4 pixel_square, vec4 rect)
{
    vec2 bottom_left = max(pixel_square.xy, rect.xy);
    vec2 top_right = min(pixel_square.zw, rect.zw);
    vec2 coverage = max(top_right - bottom_left, vec2(0.0));
    return coverage.x * coverage.y;
}

float calculate_coverage_at_point(vec2 texel_fraction, float sample_pixel_size, float sample_pixel_softness, float sample_sharpening_amount, float sample_pixel_shape, float sample_sharp_mode)
{
    vec2 pixel_center = texel_fraction - 0.5;
    vec4 pixel_rect = vec4(pixel_center - 0.5, pixel_center + 0.5);
    vec2 dot_size_in_pixels = vec2(sample_pixel_size);
    vec4 dot_rect = vec4(-dot_size_in_pixels * 0.5, dot_size_in_pixels * 0.5);

    float x_coverage = intersect_line_segment(pixel_rect.x, pixel_rect.z, dot_rect.x, dot_rect.z);
    float y_coverage = intersect_line_segment(pixel_rect.y, pixel_rect.w, dot_rect.y, dot_rect.w);
    float rect_linear = x_coverage * y_coverage;

    float rect_sharpened;
    if (sample_sharp_mode < 0.5)
    {
        float sharp_factor = 1.0 / max(sample_pixel_softness, 0.001);
        rect_sharpened = pow(x_coverage, sharp_factor) * pow(y_coverage, sharp_factor);
    }
    else
    {
        float sigmoid_strength = 10.0 / max(sample_pixel_softness, 0.001);
        float x_sharp = 1.0 / (1.0 + exp(-sigmoid_strength * (x_coverage - 0.5)));
        float y_sharp = 1.0 / (1.0 + exp(-sigmoid_strength * (y_coverage - 0.5)));
        rect_sharpened = x_sharp * y_sharp;
    }

    float rect_coverage = mix(rect_linear, rect_sharpened, sample_sharpening_amount);
    float circ_linear = intersect_rect_area(pixel_rect, dot_rect);

    float circ_sharpened;
    if (sample_sharp_mode < 0.5)
    {
        circ_sharpened = pow(circ_linear, 1.0 / max(sample_pixel_softness, 0.001));
    }
    else
    {
        float sigmoid_strength = 10.0 / max(sample_pixel_softness, 0.001);
        circ_sharpened = 1.0 / (1.0 + exp(-sigmoid_strength * (circ_linear - 0.5)));
    }

    float circ_coverage = mix(circ_linear, circ_sharpened, sample_sharpening_amount);
    return mix(circ_coverage, rect_coverage, sample_pixel_shape);
}

float sample_average_coverage(float sample_pixel_size, float sample_pixel_softness, float sample_sharpening_amount, float sample_pixel_shape, float sample_sharp_mode)
{
    float total = 0.0;

    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            vec2 sample_pos = vec2(-0.375 + float(x) * 0.25, -0.375 + float(y) * 0.25);
            total += calculate_coverage_at_point(sample_pos, sample_pixel_size, sample_pixel_softness, sample_sharpening_amount, sample_pixel_shape, sample_sharp_mode);
        }
    }

    return total / 16.0;
}

float grey_compensation_factor(void)
{
    if (integer_mode > 0.5)
        return 1.0;

    float ref_brightness = sample_average_coverage(0.80, 0.80, 1.0, 1.0, sharp_mode);
    float cur_brightness = sample_average_coverage(pixel_size, pixel_softness, sharpening_amount, pixel_shape, sharp_mode);
    float base_comp = (sharp_mode < 0.5) ? 2.8 : mix(2.5, 1.60, pixel_shape);
    float brightness_ratio = ref_brightness / max(cur_brightness, 0.001);
    float size_comp = (pixel_size > 0.80) ? pow(pixel_size / 0.80, 0.62) : 1.0;
    float softness_comp = (pixel_softness > 1.0) ? pow(pixel_softness, 0.444) : 1.0;
    float sharpening_comp = (sharp_mode < 0.5) ? mix(0.857, 1.0, sharpening_amount) : mix(1.5, 1.0, sharpening_amount);
    float shape_comp = (sharp_mode >= 0.5 && pixel_shape > 1.0) ? 1.0 + (pixel_shape - 1.0) * 0.667 : 1.0;
    return base_comp * brightness_ratio * size_comp * softness_comp * sharpening_comp * shape_comp;
}

vec3 history_rgb(sampler2D history_texture, vec2 coord, int required_count, vec3 fallback)
{
    if (SourceHistoryCount < required_count)
        return fallback;

    return abs(vec3(1.0) - texture(history_texture, coord).rgb);
}

void main()
{
    vec2 shader_tex_coord = (integer_mode > 0.5) ? vTexCoord * 1.0001 : vTexCoord;
    vec2 final_tex_coord = shader_tex_coord;
    float video_scale_factor = 1.0;
    vec2 scale_bounds = vec2(1.0);

    if (integer_mode > 0.5)
    {
        video_scale_factor = (integer_mode > 1.5) ? GAMEBOY_INTEGER_SCALE_FACTOR : floor(OutputSize.y * SourceSize.w);
        video_scale_factor = max(video_scale_factor, 1.0);
        scale_bounds = (SourceSize.xy * video_scale_factor) / OutputSize.xy;

        vec2 centered_coord = abs(shader_tex_coord - 0.5);
        if (centered_coord.x > scale_bounds.x * 0.5 || centered_coord.y > scale_bounds.y * 0.5)
        {
            FragColor = vec4(0.0);
            return;
        }

        final_tex_coord = (shader_tex_coord - 0.5) / scale_bounds + 0.5;
    }

    vec3 foreground_source = texture(Source, final_tex_coord).rgb;
    vec3 curr_rgb = abs(vec3(1.0) - foreground_source);
    vec3 prev0_rgb = history_rgb(SourceHistory0, final_tex_coord, 1, curr_rgb);
    vec3 prev1_rgb = history_rgb(SourceHistory1, final_tex_coord, 2, curr_rgb);
    vec3 prev2_rgb = history_rgb(SourceHistory2, final_tex_coord, 3, curr_rgb);
    vec3 prev3_rgb = history_rgb(SourceHistory3, final_tex_coord, 4, curr_rgb);
    vec3 prev4_rgb = history_rgb(SourceHistory4, final_tex_coord, 5, curr_rgb);
    vec3 prev5_rgb = history_rgb(SourceHistory5, final_tex_coord, 6, curr_rgb);
    vec3 prev6_rgb = history_rgb(SourceHistory6, final_tex_coord, 7, curr_rgb);

    float is_on_dot = 0.0;

    if (integer_mode > 0.5)
    {
        vec2 dot_size = SourceSize.zw;
        vec2 one_texel = vec2(1.0) / (SourceSize.xy * video_scale_factor);
        if (mod(final_tex_coord.x, dot_size.x) > one_texel.x && mod(final_tex_coord.y, dot_size.y) > one_texel.y)
            is_on_dot = 1.0;
    }
    else
    {
        vec2 texel_coord = shader_tex_coord * SourceSize.xy;
        vec2 texel_fraction = fract(texel_coord);
        vec2 texel_to_pixels = OutputSize.xy / SourceSize.xy;
        vec2 dot_size_in_pixels = texel_to_pixels * pixel_size;
        vec2 pixel_center = (texel_fraction - 0.5) * texel_to_pixels;
        vec4 pixel_rect = vec4(pixel_center - texel_to_pixels * 0.5, pixel_center + texel_to_pixels * 0.5);
        vec4 dot_rect = vec4(-dot_size_in_pixels * 0.5, dot_size_in_pixels * 0.5);

        float x_coverage = intersect_line_segment(pixel_rect.x, pixel_rect.z, dot_rect.x, dot_rect.z) / texel_to_pixels.x;
        float y_coverage = intersect_line_segment(pixel_rect.y, pixel_rect.w, dot_rect.y, dot_rect.w) / texel_to_pixels.y;
        float rect_linear = x_coverage * y_coverage;

        float rect_sharpened;
        if (sharp_mode < 0.5)
        {
            float sharp_factor = 1.0 / max(pixel_softness, 0.001);
            rect_sharpened = pow(x_coverage, sharp_factor) * pow(y_coverage, sharp_factor);
        }
        else
        {
            float sigmoid_strength = 10.0 / max(pixel_softness, 0.001);
            float x_sharp = 1.0 / (1.0 + exp(-sigmoid_strength * (x_coverage - 0.5)));
            float y_sharp = 1.0 / (1.0 + exp(-sigmoid_strength * (y_coverage - 0.5)));
            rect_sharpened = x_sharp * y_sharp;
        }

        float rect_coverage = mix(rect_linear, rect_sharpened, sharpening_amount);
        float circ_linear = intersect_rect_area(pixel_rect, dot_rect) / (texel_to_pixels.x * texel_to_pixels.y);

        float circ_sharpened;
        if (sharp_mode < 0.5)
        {
            circ_sharpened = pow(circ_linear, 1.0 / max(pixel_softness, 0.001));
        }
        else
        {
            float sigmoid_strength = 10.0 / max(pixel_softness, 0.001);
            circ_sharpened = 1.0 / (1.0 + exp(-sigmoid_strength * (circ_linear - 0.5)));
        }

        float circ_coverage = mix(circ_linear, circ_sharpened, sharpening_amount);
        is_on_dot = mix(circ_coverage, rect_coverage, pixel_shape);
    }

    vec3 input_rgb = curr_rgb;
    input_rgb += (prev0_rgb - input_rgb) * response_time;
    input_rgb += (prev1_rgb - input_rgb) * pow(response_time, 2.0);
    input_rgb += (prev2_rgb - input_rgb) * pow(response_time, 3.0);
    input_rgb += (prev3_rgb - input_rgb) * pow(response_time, 4.0);
    input_rgb += (prev4_rgb - input_rgb) * pow(response_time, 5.0);
    input_rgb += (prev5_rgb - input_rgb) * pow(response_time, 6.0);
    input_rgb += (prev6_rgb - input_rgb) * pow(response_time, 7.0);

    float brightness = input_rgb.r + input_rgb.g + input_rgb.b;
    float grey_balance_adjusted = grey_balance / max(grey_compensation_factor(), 0.001);
    grey_balance_adjusted = max(grey_balance_adjusted, 0.001);

    float rgb_to_alpha = brightness / grey_balance_adjusted + baseline_alpha;
    vec4 out_color = vec4(foreground_source, rgb_to_alpha);
    out_color.a *= is_on_dot;

    FragColor = out_color;
}