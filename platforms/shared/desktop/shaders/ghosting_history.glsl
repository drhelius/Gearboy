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
uniform sampler2D SourceHistory7;
uniform int SourceHistoryCount;
uniform float Trails;
uniform float FrameBlend;

void accumulate_history(vec3 sample_color, float sample_weight, inout vec3 color_sum, inout float weight_sum, inout vec3 darkest)
{
    color_sum += sample_color * sample_weight;
    weight_sum += sample_weight;
    darkest = min(darkest, sample_color);
}

void main()
{
    vec3 current = texture(Source, vTexCoord).rgb;
    vec3 color_sum = current;
    vec3 darkest = current;
    float weight_sum = 1.0;
    float trail_decay = clamp(Trails, 0.0, 0.98);
    float history_blend = clamp(FrameBlend, 0.0, 1.0);
    float sample_weight = 1.0;

    if (SourceHistoryCount > 0)
    {
        sample_weight *= trail_decay;
        accumulate_history(texture(SourceHistory0, vTexCoord).rgb, sample_weight, color_sum, weight_sum, darkest);
    }

    if (SourceHistoryCount > 1)
    {
        sample_weight *= trail_decay;
        accumulate_history(texture(SourceHistory1, vTexCoord).rgb, sample_weight, color_sum, weight_sum, darkest);
    }

    if (SourceHistoryCount > 2)
    {
        sample_weight *= trail_decay;
        accumulate_history(texture(SourceHistory2, vTexCoord).rgb, sample_weight, color_sum, weight_sum, darkest);
    }

    if (SourceHistoryCount > 3)
    {
        sample_weight *= trail_decay;
        accumulate_history(texture(SourceHistory3, vTexCoord).rgb, sample_weight, color_sum, weight_sum, darkest);
    }

    if (SourceHistoryCount > 4)
    {
        sample_weight *= trail_decay;
        accumulate_history(texture(SourceHistory4, vTexCoord).rgb, sample_weight, color_sum, weight_sum, darkest);
    }

    if (SourceHistoryCount > 5)
    {
        sample_weight *= trail_decay;
        accumulate_history(texture(SourceHistory5, vTexCoord).rgb, sample_weight, color_sum, weight_sum, darkest);
    }

    if (SourceHistoryCount > 6)
    {
        sample_weight *= trail_decay;
        accumulate_history(texture(SourceHistory6, vTexCoord).rgb, sample_weight, color_sum, weight_sum, darkest);
    }

    if (SourceHistoryCount > 7)
    {
        sample_weight *= trail_decay;
        accumulate_history(texture(SourceHistory7, vTexCoord).rgb, sample_weight, color_sum, weight_sum, darkest);
    }

    vec3 trail_color = color_sum / max(weight_sum, 1.0);
    vec3 combined = mix(current, trail_color, history_blend);

    float current_luma = dot(current, vec3(0.299, 0.587, 0.114));
    float darkest_luma = dot(darkest, vec3(0.299, 0.587, 0.114));
    float shadow_hold = clamp((current_luma - darkest_luma) * 3.0, 0.0, 1.0) * history_blend;
    combined = mix(combined, darkest, shadow_hold);

    FragColor = vec4(clamp(combined, 0.0, 1.0), 1.0);
}