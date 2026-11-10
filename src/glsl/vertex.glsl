#version 460 core

layout (location=0)
in vec3 a_position;

layout (location=1)
in vec3 a_normal;

layout (location=2)
in vec2 a_texcoord;

layout (location=0)
out vec3 v_normal;

layout (location=1)
out vec2 v_texcoord;

uniform mat4 u_model;
uniform mat4 u_proj;
uniform mat4 u_view;

void main() {
    gl_Position = u_proj * u_view * u_model * vec4(a_position, 1.0);
    v_normal = a_normal;
    v_texcoord = a_texcoord;
}
