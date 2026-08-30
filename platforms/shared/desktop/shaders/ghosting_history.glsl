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

void accumulate_history(vec3 sample_color, float sample_weight, inout vec3 color_sum, inout float weight_sum)
{
    color_sum += sample_color * sample_weight;
    weight_sum += sample_weight;
}

void main()
{
    vec3 current = texture(Source, vTexCoord).rgb;
    vec3 color_sum = current;
    float weight_sum = 1.0;
    float trail_decay = clamp(Trails, 0.0, 0.98);
    float history_blend = clamp(FrameBlend, 0.0, 1.0);
    float sample_weight = 1.0;

    if (SourceHistoryCount > 0)
    {
        sample_weight *= trail_decay;
        accumulate_history(texture(SourceHistory0, vTexCoord).rgb, sample_weight, color_sum, weight_sum);
    }

    if (SourceHistoryCount > 1)
    {
        sample_weight *= trail_decay;
        accumulate_history(texture(SourceHistory1, vTexCoord).rgb, sample_weight, color_sum, weight_sum);
    }

    if (SourceHistoryCount > 2)
    {
        sample_weight *= trail_decay;
        accumulate_history(texture(SourceHistory2, vTexCoord).rgb, sample_weight, color_sum, weight_sum);
    }

    if (SourceHistoryCount > 3)
    {
        sample_weight *= trail_decay;
        accumulate_history(texture(SourceHistory3, vTexCoord).rgb, sample_weight, color_sum, weight_sum);
    }

    if (SourceHistoryCount > 4)
    {
        sample_weight *= trail_decay;
        accumulate_history(texture(SourceHistory4, vTexCoord).rgb, sample_weight, color_sum, weight_sum);
    }

    if (SourceHistoryCount > 5)
    {
        sample_weight *= trail_decay;
        accumulate_history(texture(SourceHistory5, vTexCoord).rgb, sample_weight, color_sum, weight_sum);
    }

    if (SourceHistoryCount > 6)
    {
        sample_weight *= trail_decay;
        accumulate_history(texture(SourceHistory6, vTexCoord).rgb, sample_weight, color_sum, weight_sum);
    }

    if (SourceHistoryCount > 7)
    {
        sample_weight *= trail_decay;
        accumulate_history(texture(SourceHistory7, vTexCoord).rgb, sample_weight, color_sum, weight_sum);
    }

    vec3 trail_color = color_sum / max(weight_sum, 1.0);
    vec3 combined = mix(current, trail_color, history_blend);

    FragColor = vec4(clamp(combined, 0.0, 1.0), 1.0);
}