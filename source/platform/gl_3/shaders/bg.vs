in vec2 a_vertex;
out vec2 v_uv;

void main()
{
    v_uv = vec2(0.5 + a_vertex.x * 0.5, 0.5 - a_vertex.y * 0.5);
    gl_Position = vec4(a_vertex, 0.0, 1.0);
}
