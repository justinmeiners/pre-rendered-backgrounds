#define MAX_JOINTS 48
#define MAX_WEIGHTS 3

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

uniform vec3 u_jointOrigins[MAX_JOINTS];
uniform vec4 u_jointRotations[MAX_JOINTS];

in vec2 a_uv0;
in vec3 a_weight_joints;
in vec4 a_weight0;
in vec4 a_weight1;
in vec4 a_weight2;


vec3 quatRotate(const vec4 quat, const vec3 vec)
{
    vec3 t = cross(quat.xyz, vec) * 2.0;
    return vec + t * quat.w + cross(quat.xyz, t);
}

void main()
{
    vec3 vert = vec3(0.0, 0.0, 0.0);
    
    vec4 weights[MAX_WEIGHTS];
    weights[0] = a_weight0;
    weights[1] = a_weight1;
    weights[2] = a_weight2;
    
    for (int i = 0; i < MAX_WEIGHTS; i++)
    {
        int joint = int(a_weight_joints[i]);
        vec3 transformed = u_jointOrigins[joint] + quatRotate(u_jointRotations[joint], weights[i].xyz);
        vert += transformed * weights[i].w;
    }
    
    
    gl_Position = u_projection * u_view * u_model * vec4(vert, 1.0);
}
