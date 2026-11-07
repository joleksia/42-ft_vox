#include "ft_vox.h"

typedef struct GLvertex GLvertex;

struct GLvertex {
    vec3 position;
    vec3 normal;
    vec2 texcoord;
};


int main(void) {

    /* create window */

    if (SDL_Init(SDL_INIT_VIDEO)) { return (1); }
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4)) { return (1); }
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6)) { return (1); }
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE)) { return (1); }

    SDL_Window *window = SDL_CreateWindow("glld.h - SDL2 sample", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Quit();
        return (1);
    }

    /* create context */

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return (1);
    }

    if (SDL_GL_MakeCurrent(window, context)) {
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return (2);
    }

    /* load OpenGL */

    if (glld_loader(0, (glld_loader_t) SDL_GL_GetProcAddress) != GL_NO_ERROR) {
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return (3);
    }
    
    /* create shader program */

    GLshader shader = glCreateProgram();
    
    FILE *fp_vertex_glsl = fopen("src/glsl/vertex.glsl", "rb");
    ftLoadShaderGLSL(shader, GL_VERTEX_SHADER, fp_vertex_glsl);
    fclose(fp_vertex_glsl);
    
    FILE *fp_fragment_glsl = fopen("src/glsl/fragment.glsl", "rb");
    ftLoadShaderGLSL(shader, GL_FRAGMENT_SHADER, fp_fragment_glsl);
    fclose(fp_fragment_glsl);

    GLint u_proj_loc = glGetUniformLocation(shader, "u_proj");
    GLint u_view_loc = glGetUniformLocation(shader, "u_view");

    /* create vertex array */

    GLuint vao;
    glCreateVertexArrays(1, &vao);

    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribBinding(vao, 0, 0);
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(GLvertex, position));
   
    glEnableVertexArrayAttrib(vao, 1);
    glVertexArrayAttribBinding(vao, 1, 0);
    glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(GLvertex, normal));
    
    glEnableVertexArrayAttrib(vao, 2);
    glVertexArrayAttribBinding(vao, 2, 0);
    glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(GLvertex, texcoord));

    /* create GPU buffer */

    GLvertex vertices[] = { [0] = { .position = alg_vec3(0.0,  0.0,  0.0), .normal = alg_vec3(0.0, 0.0, 0.0), .texcoord = alg_vec2(0.0, 0.0) },
                            [1] = { .position = alg_vec3(10.0, 0.0,  0.0), .normal = alg_vec3(0.0, 0.0, 0.0), .texcoord = alg_vec2(1.0, 0.0) },
                            [2] = { .position = alg_vec3(0.0,  10.0, 0.0), .normal = alg_vec3(0.0, 0.0, 0.0), .texcoord = alg_vec2(0.0, 1.0) },
                            [3] = { .position = alg_vec3(10.0, 10.0, 0.0), .normal = alg_vec3(0.0, 0.0, 0.0), .texcoord = alg_vec2(1.0, 1.0) } };
                
    GLuint vbo;
    glCreateBuffers(1, &vbo);
    glNamedBufferData(vbo, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(GLvertex));

    /* create GPU buffer */

    GLuint indices[] = { 0, 1, 2,
                         1, 2, 3 };

    GLuint ibo;
    glCreateBuffers(1, &ibo);
    glNamedBufferData(ibo, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexArrayElementBuffer(vao, ibo);

    /* create 3D camera */

    Camera3D camera = {
        .position = alg_vec3(0.0, 0.0, -100.0),
        .rotation = alg_vec3(0.0, 0.0, 1.0),
        .direction = alg_vec3(0.0, 0.0, -90.0),
        .up = alg_vec3(0.0, 1.0, 0.0),
        .fieldOfView = 60.0,
        .sensitivity = 1.0
    };

    int exit = 0;
    do {
        
        /* update */

        /* get window size */
        vec2 size = { 0 };
        SDL_GL_GetDrawableSize(window, (int *) &size.x,
                                       (int *) &size.y);

        mat4 u_proj = alg_mat4(1.0),
             u_view = alg_mat4(1.0);
        ftCamera3DMatrix(&camera, &u_proj, &u_view, size);

        /* ... */

        /* render */

        /* clear */
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        /* update uniforms */
        glProgramUniformMatrix4fv(shader, u_proj_loc, 1, GL_FALSE, (GLfloat *) &u_proj);
        glProgramUniformMatrix4fv(shader, u_view_loc, 1, GL_FALSE, (GLfloat *) &u_view);

        /* render current vertex array */
        glUseProgram(shader);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glUseProgram(0);

        /* poll events */

        SDL_GL_SwapWindow(window);
        SDL_Event event = { 0 };
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case (SDL_QUIT): { exit = 1; } break;

                case (SDL_KEYDOWN): {
                    SDL_Scancode code = event.key.keysym.scancode;
                    uint32_t keysym = event.key.keysym.sym; /* virtual key  */
                    uint32_t keymod = event.key.keysym.mod; /* key modifier */

                    (void) keysym;
                    (void) keymod;
                    
                    /* right-left */
                    if (code == SDL_SCANCODE_A) { camera.position.x += 1.0; }
                    if (code == SDL_SCANCODE_D) { camera.position.x -= 1.0; }
                    
                    /* top-down */
                    if (code == SDL_SCANCODE_SPACE)  { camera.position.y += 1.0; }
                    if (code == SDL_SCANCODE_LSHIFT) { camera.position.y -= 1.0; }

                    /* forward-backward */
                    if (code == SDL_SCANCODE_W) { camera.position.z += 1.0; }
                    if (code == SDL_SCANCODE_S) { camera.position.z -= 1.0; }
                } break;

                case (SDL_WINDOWEVENT): {
                    switch (event.window.event) {
                        case (SDL_WINDOWEVENT_RESIZED):
                        case (SDL_WINDOWEVENT_SIZE_CHANGED): {
                            glViewport(0, 0, event.window.data1,
                                             event.window.data2);
                        } break;
                    }
                } break;
            }
        }
    } while (!exit);
  
    /* unload OpenGL */

    glDeleteBuffers(1, &vbo), vbo = 0;
    glDeleteBuffers(1, &ibo), ibo = 0;
    glDeleteVertexArrays(1, &vao), vao = 0;
    glDeleteProgram(shader), shader = 0;

    /* quit */

    glld_unload(0);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return (0);
}
