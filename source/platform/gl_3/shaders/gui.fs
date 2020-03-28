#define ATLAS_COUNT 4

uniform sampler2D u_atlas[ATLAS_COUNT];

in vec4 v_color;
in vec2 v_uv;
in float v_atlas_id;

out vec4 fragColor;

void main()
{
    vec4 color = vec4(0.0);
    
    for (int i = 0; i < ATLAS_COUNT; ++i)
    {
        vec4 atlasColor = texture(u_atlas[i], v_uv);
        float factor = max(v_atlas_id - float(i - 1), 0.0);
        color = mix(color, atlasColor, factor);
    }

    fragColor = color * v_color;
}
