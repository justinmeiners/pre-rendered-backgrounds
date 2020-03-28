uniform mat4 u_projection;

in vec2 a_vertex;
in vec2 a_uv0;
in vec4 a_color;
in float a_atlas_id;

out vec4 v_color;
out vec2 v_uv;
out float v_atlas_id;

void main()
{ 
    v_color = a_color;
    v_uv = a_uv0;
    v_atlas_id = a_atlas_id;
    
    gl_Position = u_projection * vec4(a_vertex, 0.0, 1.0);
}
