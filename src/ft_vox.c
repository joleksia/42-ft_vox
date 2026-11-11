#include "ft_vox.h"

int main(void) {

    /* create window */

    if (SDL_Init(SDL_INIT_VIDEO)) { return (1); }
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4)) { return (1); }
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6)) { return (1); }
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE)) { return (1); }

    SDL_Window *window = SDL_CreateWindow("42warsaw - ft_vox", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Quit();
        return (1);
    }

    /* create 'keyboard' struct */
    struct {
        uint8_t current[SDL_NUM_SCANCODES];
        uint8_t previous[SDL_NUM_SCANCODES];
    } keyboard = { 0 };

    /* create 'mouse' struct */
    struct {
        struct {
            vec2 current;
            vec2 previous;
            vec2 delta;
        } position;
        struct {
            uint8_t current[8];
            uint8_t previous[8];
        } input;
        uint8_t locked;
    } mouse = { 0 };
    mouse.locked = 0;

    /* create 'time' struct */
    struct {
        double current;
        double previous;
        double delta;
    } time = { 0 };

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

    /* configure OpenGL */
    SDL_GL_SetSwapInterval(1);
    
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
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
        .position = alg_vec3(0.0, 0.0, 32.0),
        .rotation = alg_vec3(0.0, 0.0, -1.0),
        .direction = alg_vec3(0.0, 0.0, -90.0),
        .up = alg_vec3(0.0, 1.0, 0.0),
        .fieldOfView = 60.0,
    };

    /* create 'world' */
    World world = { 0 };
    ftWorldCreate(&world, rand());


    int exit = 0;
    do {
        
        /* update */

        /* set new 'window' title */
        char title[128] = { 0 };
        snprintf(title, 127, "42warsaw - ft_vox (FPS.: %d)", (int) (1.0 / time.delta));
        SDL_SetWindowTitle(window, title);

        /* enable 'locked' mode */
        if (mouse.input.current[1]) {
            if (!mouse.input.previous[1]) {
                mouse.locked = 1;
            }
        }

        /* disable 'locked' mode */
        if (keyboard.current[SDL_SCANCODE_ESCAPE]) {
            if (!keyboard.previous[SDL_SCANCODE_ESCAPE]) {
                mouse.locked = 0;
            }
        }

        /* update 'camera' position */
        float velocity = 8.0f * time.delta;
        if (keyboard.current[SDL_SCANCODE_D] ||
            keyboard.current[SDL_SCANCODE_A]
        ) {
            int8_t input = keyboard.current[SDL_SCANCODE_D] -
                           keyboard.current[SDL_SCANCODE_A];
            vec3 norm = alg_vec3norm(alg_vec3cross(camera.rotation, camera.up));
            camera.position = alg_vec3add(camera.position, alg_vec3mulf(norm, input * velocity));
        }
        
        if (keyboard.current[SDL_SCANCODE_SPACE] ||
            keyboard.current[SDL_SCANCODE_LSHIFT]
        ) {
            int8_t input = keyboard.current[SDL_SCANCODE_SPACE] -
                           keyboard.current[SDL_SCANCODE_LSHIFT];
            camera.position = alg_vec3add(camera.position, alg_vec3mulf(camera.up, input * velocity));
        }
        
        if (keyboard.current[SDL_SCANCODE_W] ||
            keyboard.current[SDL_SCANCODE_S]
        ) {
            int8_t input = keyboard.current[SDL_SCANCODE_W] -
                           keyboard.current[SDL_SCANCODE_S];

            double sy = sin(alg_deg2rad(camera.direction.z)),
                   cy = cos(alg_deg2rad(camera.direction.z));
            vec3 forward = alg_vec3(cy, 0.0f, sy);
            camera.position = alg_vec3add(camera.position, alg_vec3mulf(forward, input * velocity));
        }

        /* update 'camera' rotation */
        float sensitivity = 0.1f;
        SDL_SetRelativeMouseMode(mouse.locked);
        if (mouse.locked) {
            /* apply 'offset' to 'direction' */
            camera.direction.z += mouse.position.delta.x * sensitivity; /* yaw */
            camera.direction.y -= mouse.position.delta.y * sensitivity; /* pitch */
            
            /* clear 'mouse' 'delta' */
            mouse.position.delta = alg_vec2(0.0, 0.0);

            /* clamp 'pitch' in [-89.0, 89.0] */
            camera.direction.y = alg_clamp(camera.direction.y, -89.0, 89.0);

            /* apply 'direction' to 'rotation' */
            double sp = sin(alg_deg2rad(camera.direction.y)),
                   cp = cos(alg_deg2rad(camera.direction.y)),
                   sy = sin(alg_deg2rad(camera.direction.z)),
                   cy = cos(alg_deg2rad(camera.direction.z));
            camera.rotation = alg_vec3norm(alg_vec3(cy * cp, sp, sy * cp));
        }

        /* ... */

        /* render */

        /* clear */
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* update uniforms */
        vec2 size = { 0 };
        SDL_GL_GetDrawableSize(window, (int *) &size.x,
                                       (int *) &size.y);
        mat4 u_proj = alg_mat4(1.0),
             u_view = alg_mat4(1.0);
        ftCameraMatrix(&camera, &u_proj, &u_view, size);
        ftShaderUniformSet(shader, "u_proj", FT_UNIFORM_MATRIX4X4, &u_proj);
        ftShaderUniformSet(shader, "u_view", FT_UNIFORM_MATRIX4X4, &u_view);

        /* render current vertex array */
        glUseProgram(shader->program);
        glBindVertexArray(vao->ID);

        /* render 'world' */
        ftWorldRender(&world, vao, shader);

        glBindVertexArray(0);
        glUseProgram(0);

        /* copy 'current' inputs to 'previous' inputs */
        memcpy(keyboard.previous,
               keyboard.current,
               sizeof(keyboard.current));

        memcpy(mouse.input.previous,
               mouse.input.current,
               sizeof(mouse.input.current));

        /* poll events */

        SDL_GL_SwapWindow(window);
        SDL_Event event = { 0 };
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case (SDL_QUIT): { exit = 1; } break;

                case (SDL_MOUSEMOTION): {
                    mouse.position.previous = mouse.position.current;
                    mouse.position.current = alg_vec2(event.motion.x,
                                                      event.motion.y);

                    if (mouse.locked) {
                        vec2 relative = alg_vec2(event.motion.xrel,
                                                 event.motion.yrel);
                        mouse.position.delta = alg_vec2add(mouse.position.delta, relative);
                    }
                } break;

                case (SDL_MOUSEBUTTONUP):
                case (SDL_MOUSEBUTTONDOWN): {
                    uint8_t button = event.button.button;
                    mouse.input.current[button] = event.button.state;
                } break;

                case (SDL_KEYUP):
                case (SDL_KEYDOWN): {
                    SDL_Scancode code = event.key.keysym.scancode;
                    keyboard.current[code] = event.key.state;
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

        /* update 'time' */
        time.previous = time.current;
        time.current = SDL_GetPerformanceCounter();
        time.delta = (time.current - time.previous) / (double) SDL_GetPerformanceFrequency();
    
    } while (!exit);

    /* unload 'world' */
    ftWorldDelete(&world);

    /* unload OpenGL */

    ftShaderDelete(shader);
    ftVertexArrayDelete(vao);

    /* quit */

    glld_unload(0);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return (0);
}
