#version 460 core

layout (location=0)
in vec2 v_texcoord;

layout (location=1)
in float v_shading;

layout (location=0)
out vec4 f_color;

layout (binding=0)
uniform sampler2D u_atlas;

void main() {
    vec4 sampler = texture(u_atlas, v_texcoord) * v_shading;
    if (sampler.a == 0.0) {
        discard;
    }

    /* set 'f_color' */
    f_color = sampler;
}
