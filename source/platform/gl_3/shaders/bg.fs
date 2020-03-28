
uniform sampler2D u_albedo;
uniform sampler2D u_depth;

in vec2 v_uv;
out vec4 fragColor;

void main()
{
    vec4 depth = texture(u_depth, v_uv);
    
    float near = 0.5;
    float far = 25.0;
    float z_linear = depth.r;
    
    float z_non_linear = -((near + far) * z_linear - (2.0 * near)) / ((near - far) * z_linear);
    
    gl_FragDepth = z_linear;
    
    vec4 color = texture(u_albedo, v_uv);
    fragColor = color;
}
