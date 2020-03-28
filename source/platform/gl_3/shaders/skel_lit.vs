#define MAX_JOINTS 48
#define MAX_WEIGHTS 3

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

uniform vec3 u_jointOrigins[MAX_JOINTS];
uniform vec4 u_jointRotations[MAX_JOINTS];

in vec3 a_normal;
in vec3 a_tangent;
in vec2 a_uv0;
in vec3 a_weight_joints;
in vec4 a_weight0;
in vec4 a_weight1;
in vec4 a_weight2;

out vec2 v_uvs[1];
out vec3 v_fragPosition;
out mat3 v_tbnMatrix;

vec3 quatRotate(const vec4 quat, const vec3 vec)
{
    vec3 t = cross(quat.xyz, vec) * 2.0;
    return vec + t * quat.w + cross(quat.xyz, t);
}

void main()
{
    vec3 vert = vec3(0.0, 0.0, 0.0);
    vec3 normal = vec3(0.0, 0.0, 0.0);
    vec3 tangent = vec3(0.0, 0.0, 0.0);

    vec4 weights[MAX_WEIGHTS];
    weights[0] = a_weight0;
    weights[1] = a_weight1;
    weights[2] = a_weight2;
    
    for (int i = 0; i < MAX_WEIGHTS; i++)
    {
        int joint = int(a_weight_joints[i]);
        vec3 transformed = u_jointOrigins[joint] + quatRotate(u_jointRotations[joint], weights[i].xyz);
        vert += transformed * weights[i].w;
        transformed = quatRotate(u_jointRotations[joint], a_normal);
        normal += transformed * weights[i].w;
        transformed = quatRotate(u_jointRotations[joint], a_tangent);
        tangent += transformed * weights[i].w;
    }
    
    v_uvs[0] = a_uv0;
    v_tbnMatrix = mat3(tangent, cross(tangent, normal), normal);
    v_fragPosition = (u_model * vec4(vert, 1.0)).xyz;
    
    gl_Position = u_projection * u_view * u_model * vec4(vert, 1.0);
}
