uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

uniform vec3 u_lightPositions[2];

in vec3 a_vertex;
in vec3 a_normal;
in vec2 a_uv0;

out vec3 v_normal;
out vec2 v_uv;
out vec3 v_lightPosition[2];
out vec3 v_fragPosition;

void main()
{    
    v_uv = a_uv0;
    
    v_lightPosition[0] = u_lightPositions[0];
    v_lightPosition[1] = u_lightPositions[1];
    
    v_normal = (u_model * vec4(a_normal , 0.0)).xyz;
    v_fragPosition = (u_model * vec4(a_vertex, 1.0)).xyz;
    
    gl_Position = u_projection * u_view * u_model * vec4(a_vertex, 1.0);
}
