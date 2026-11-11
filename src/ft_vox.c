#include "ft_vox.h"

int main(void) {

    /* create OpenGL window */
    
    if (!ftWindowCreate("42warsaw - ft_vox", 800, 600)) { return (1); }
   
    /* create input objects */
    
    ftKeyboardCreate();
    ftMouseCreate();
    ftTimeCreate();

    /* create 'shader' */

    Shader *shader = 0;
    ftShaderCreate(&shader);

    /* load 'vertex.glsl' */
    FILE *fp_vertex_glsl = fopen("src/glsl/vertex.glsl", "rb");
    if (fp_vertex_glsl) {
        ftShaderLoadGLSL(shader, fp_vertex_glsl, FT_SHADER_VERTEX);
        fclose(fp_vertex_glsl);
    }

    /* load 'fragment.glsl' */
    FILE *fp_fragment_glsl = fopen("src/glsl/fragment.glsl", "rb");
    if (fp_fragment_glsl) {
        ftShaderLoadGLSL(shader, fp_fragment_glsl, FT_SHADER_FRAGMENT);
        fclose(fp_fragment_glsl);
    }

    /* create vertex array */

    VertexArray *vao = 0;
    ftVertexArrayCreate(&vao);
    ftVertexArrayLayout(vao, 0, 3, offsetof(GLvertex, position));
    ftVertexArrayLayout(vao, 1, 3, offsetof(GLvertex, normal));
    ftVertexArrayLayout(vao, 2, 2, offsetof(GLvertex, texcoord));

    /* create 3D 'camera' */

    Camera camera = {
        .position = alg_vec3(0.0, 0.0, 0.0),
        .rotation = alg_vec3(0.0, 0.0, -1.0),
        .direction = alg_vec3(0.0, 0.0, -90.0),
        .up = alg_vec3(0.0, 1.0, 0.0),
        .fieldOfView = 60.0,
    };

    /* create 'world' */

    srand(time(0));
    World world = { 0 };
    ftWorldCreate(&world, rand());

    /* update-render loop */

    do {

        /* update */

        /* set new 'window' title */
        ftWindowSetTitle("42warsaw - ft_vox (FPS.: %d)", (int) (1.0 / Time->delta));
        
        /* enable 'locked' mode */
        if (Mouse->input.current[1]) {
            if (!Mouse->input.previous[1]) {
                Mouse->locked = 1;
            }
        }

        /* disable 'locked' mode */
        if (Keyboard->current[SDL_SCANCODE_ESCAPE]) {
            if (!Keyboard->previous[SDL_SCANCODE_ESCAPE]) {
                Mouse->locked = 0;
            }
        }

        /* update 'camera' */
        ftCameraMove(&camera, 8.0 * Time->delta);
        ftCameraRotate(&camera, 256.0 * Time->delta);

        /* render */

        /* clear */
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);

        /* update uniforms */
        mat4 u_proj = alg_mat4(1.0),
             u_view = alg_mat4(1.0);
        ftCameraMatrix(&camera, &u_proj, &u_view);
        ftShaderUniformSet(shader, "u_proj", FT_UNIFORM_MATRIX4X4, &u_proj);
        ftShaderUniformSet(shader, "u_view", FT_UNIFORM_MATRIX4X4, &u_view);

        /* render current vertex array */
        glUseProgram(shader->program);
        glBindVertexArray(vao->ID);

        /* render 'world' */
        ftWorldRender(&world, vao, shader);

        glBindVertexArray(0);
        glUseProgram(0);

        /* poll-events */
        ftWindowPollEvents();

    } while (!Window->flags.close);

    /* unload 'world' */
    ftWorldDelete(&world);

    /* unload OpenGL */

    ftShaderDelete(shader);
    ftVertexArrayDelete(vao);
    
    /* unload input objects */
    
    ftTimeDelete();
    ftMouseDelete();
    ftKeyboardDelete();

    /* quit */

    ftWindowDelete();

    return (0);
}
