#version 460 core

layout (location=0)
in vec2 v_texcoord;

layout (location=1)
in vec3 v_normal;

layout (location=2)
in float v_shading;

layout (location=0)
out vec4 f_color;

layout (binding=0)
uniform sampler2D u_atlas;

void main() {
    vec4 sampler = texture(u_atlas, v_texcoord);
    if (sampler.a < 0.1) {
        discard;
    }

    /* set 'f_color' */
    f_color = vec4(sampler.rgb * v_shading, sampler.a);
}
