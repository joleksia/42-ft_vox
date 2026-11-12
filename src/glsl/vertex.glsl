#version 460 core

layout (location=0)
in uint a_vertex;

layout (location=0)
out vec3 v_normal;

layout (location=1)
out vec2 v_texcoord;

uniform mat4 u_model;
uniform mat4 u_proj;
uniform mat4 u_view;

void main() {
    vec3 v_position = vec3(float((a_vertex >>  0) & 0xffu),
                           float((a_vertex >>  8) & 0xffu),
                           float((a_vertex >> 16) & 0xffu));
    
    v_normal = vec3(float((a_vertex >> 24) & 0x01u),
                    float((a_vertex >> 25) & 0x01u),
                    float((a_vertex >> 26) & 0x01u));

    v_texcoord = vec2(float((a_vertex >> 27) & 0x01u),
                      float((a_vertex >> 28) & 0x01u));
    
    gl_Position = u_proj * u_view * u_model * vec4(v_position, 1.0);
}
