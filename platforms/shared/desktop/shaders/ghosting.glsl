in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform sampler2D PassFeedback0;
uniform sampler2D SourceHistory0;
uniform int SourceHistoryCount;
uniform float Trails;
uniform float FrameBlend;

void main()
{
    vec3 current = texture(Source, vTexCoord).rgb;
    vec3 frame_sum = current;
    float frame_count = 1.0;
    float history_blend = clamp(FrameBlend, 0.0, 1.0);

    if (SourceHistoryCount == 0)
    {
        FragColor = vec4(current, 1.0);
        return;
    }

    if (SourceHistoryCount > 0)
    {
        vec3 history = texture(SourceHistory0, vTexCoord).rgb;
        frame_sum += history * history_blend;
        frame_count += history_blend;
    }

    vec3 combined = frame_sum / max(frame_count, 1.0);
    vec3 previous = texture(PassFeedback0, vTexCoord).rgb;
    float trails = clamp(Trails, 0.0, 0.98);
    vec3 color = previous * trails + combined * (1.0 - trails);
    FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}