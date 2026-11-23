#include "ft_vox.h"

/* public API */

extern int ft_window_create(struct window_s **self, const char *title, const size_t width, const size_t height) {
    /* null-check */
    if (!self) { return (0); }

    /* alloc. 'window' */
    struct
    window_s *window = (struct window_s *) malloc(sizeof(struct window_s));
    if (!window) {
        return (0);
    }
    
    /* initialize 'SDL2' */
    if (SDL_Init(SDL_INIT_VIDEO)) { return (0); }
    
    /* configure OpenGL 'context' */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    /* create OpenGL 'window' */
    window->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                                           SDL_WINDOWPOS_CENTERED,
                                           width, height,
                                           SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    window->context = SDL_GL_CreateContext(window->window);

    SDL_GL_MakeCurrent(window->window,
                       window->context);

    /* initialize 'glld.h' */
    glld_loader(0, (glld_loader_t) SDL_GL_GetProcAddress);

    /* configure OpenGL */
    SDL_GL_SetSwapInterval(window->flags.vsync);
    
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    /* return 'self' */
    *self = window;

    /* success */
    return (1);
}


extern int ft_window_delete(struct window_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* release 'context' */
    void *context = self->context;
    if (context) { SDL_GL_DeleteContext(context); }

    /* release 'window' */
    void *window = self->window;
    if (window) { SDL_DestroyWindow(window); }
    
    /* unload 'glld.h' */
    glld_unload(0);
    
    /* unload 'SDL2' */
    SDL_Quit();

    /* release 'self' */
    free(self);

    /* success */
    return (1);
}


extern int ft_window_poll_events(struct window_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* swap window buffers */
    SDL_GL_SwapWindow(self->window);

    /* update 'window_s's configuration */
    SDL_GL_SetSwapInterval(self->flags.vsync);

    /* sync input objects */
    ft_keyboard_sync(STATE.input.keyboard);
    ft_mouse_sync(STATE.input.mouse);
    ft_time_sync(STATE.time);

    /* poll events */
    SDL_Event event = { 0 };
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case (SDL_QUIT): { self->flags.close = 1; } break;

            case (SDL_MOUSEMOTION): {
                STATE.input.mouse->position.previous = STATE.input.mouse->position.current;
                STATE.input.mouse->position.current = alg_vec2(event.motion.x,
                                                   event.motion.y);

                if (STATE.input.mouse->locked) {
                    vec2 relative = alg_vec2(event.motion.xrel,
                                             event.motion.yrel);
                    STATE.input.mouse->position.delta = alg_vec2add(STATE.input.mouse->position.delta, relative);
                }
            } break;

            case (SDL_MOUSEBUTTONUP):
            case (SDL_MOUSEBUTTONDOWN): {
                u8 button = event.button.button;
                STATE.input.mouse->input.current[button] = event.button.state;
            } break;

            case (SDL_KEYUP):
            case (SDL_KEYDOWN): {
                SDL_Scancode code = event.key.keysym.scancode;
                STATE.input.keyboard->current[code] = event.key.state;
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

    /* success */
    return (1);
}


extern int ft_window_get_size(struct window_s *self, int *pX, int *pY) {
    /* null-check */
    if (!self) { return (0); }

    /* get 'window_s's size */
    i32 x = 0,
        y = 0;
    SDL_GL_GetDrawableSize(self->window, &x, &y);

    /* return results */
    if (pX) { *pX = x; }
    if (pY) { *pY = y; }

    /* success */
    return (1);
}
