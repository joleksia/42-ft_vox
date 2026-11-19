#include "ft_vox.h"

/* static API */ 

struct window_s *WINDOW = 0;

/* public API */

extern int ft_window_create(const char *title, const size_t width, const size_t height) {
    /* alloc. global 'WINDOW' object */
    if (!WINDOW) {
        WINDOW = (struct window_s *) malloc(sizeof(struct window_s));
        if (!WINDOW) {
            SDL_Quit();
            return (0);
        }
    }
    
    /* initialize 'SDL2' */
    if (SDL_Init(SDL_INIT_VIDEO)) { return (0); }
    
    /* configure OpenGL 'context' */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    /* create OpenGL 'window' */
    void *window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                                           SDL_WINDOWPOS_CENTERED,
                                           width, height,
                                           SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Quit();
        free(WINDOW);
        return (0);
    }

    /* create OpenGL 'context' */
    void *context = SDL_GL_CreateContext(window);
    if (!context) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        free(WINDOW);
        return (0);
    }

    if (SDL_GL_MakeCurrent(window, context)) {
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        free(WINDOW);
        return (0);
    }

    /* initialize 'glld.h' */
    if (glld_loader(0, (glld_loader_t) SDL_GL_GetProcAddress) != GL_NO_ERROR) {
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        free(WINDOW);
        return (0);
    }

    /* return results */
    WINDOW->window  = window;
    WINDOW->context = context;

    /* set-up configuration 'flags' */
    WINDOW->flags.close = 0;
    WINDOW->flags.vsync = 1;

    /* configure OpenGL */
    SDL_GL_SetSwapInterval(WINDOW->flags.vsync);
    
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    /* success */
    return (1);
}


extern int ft_window_delete(void) {
    /* null-check */
    if (!WINDOW) { return (0); }

    /* release 'window_s's resources */
    void *context = WINDOW->context;
    if (context) { SDL_GL_DeleteContext(context); }

    void *window = WINDOW->window;
    if (window) { SDL_DestroyWindow(window); }
    
    /* quit 'glld.h' */
    glld_unload(0);
    
    /* quit 'SDL2' */
    SDL_Quit();

    /* release 'WINDOW' */
    free(WINDOW); WINDOW = 0;

    /* success */
    return (1);
}


extern int ft_window_poll_events(void) {
    /* null-check */
    if (!WINDOW) { return (0); }

    /* swap window buffers */
    SDL_GL_SwapWindow(WINDOW->window);

    /* update 'window_s's configuration */
    SDL_GL_SetSwapInterval(WINDOW->flags.vsync);

    /* sync input objects */
    ft_keyboard_sync();
    ft_mouse_sync();
    ft_time_sync();

    /* poll events */
    SDL_Event event = { 0 };
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case (SDL_QUIT): { WINDOW->flags.close = 1; } break;

            case (SDL_MOUSEMOTION): {
                MOUSE->position.previous = MOUSE->position.current;
                MOUSE->position.current = alg_vec2(event.motion.x,
                                                   event.motion.y);

                if (MOUSE->locked) {
                    vec2 relative = alg_vec2(event.motion.xrel,
                                             event.motion.yrel);
                    MOUSE->position.delta = alg_vec2add(MOUSE->position.delta, relative);
                }
            } break;

            case (SDL_MOUSEBUTTONUP):
            case (SDL_MOUSEBUTTONDOWN): {
                u8 button = event.button.button;
                MOUSE->input.current[button] = event.button.state;
            } break;

            case (SDL_KEYUP):
            case (SDL_KEYDOWN): {
                SDL_Scancode code = event.key.keysym.scancode;
                KEYBOARD->current[code] = event.key.state;
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


extern int ft_window_title(const char *format, ...) {
    /* null-check */
    if (!WINDOW) { return (0); }

    /* fixed-size null-terminated c-string */
    char buffer[128] = { 0 };

    /* extract 'list' */
    va_list list;
    va_start(list, format);
    size_t nwrite = vsnprintf(buffer, sizeof(buffer), format, list);
    va_end(list);

    /* if 'nwrite' matches or exceeds 'buffer' size */
    if (nwrite >= sizeof(buffer)) {
        snprintf((buffer + sizeof(buffer)) - 4, 4, "...");
    }

    /* set 'buffer' as 'window_s's title */
    SDL_SetWindowTitle(WINDOW->window, buffer);

    /* success */
    return (1);
}


extern int ft_window_size(int *pX, int *pY) {
    /* null-check */
    if (!WINDOW) { return (0); }

    /* get 'window_s's size */
    i32 x = 0,
        y = 0;
    SDL_GL_GetDrawableSize(WINDOW->window, &x, &y);

    /* return results */
    if (pX) { *pX = x; }
    if (pY) { *pY = y; }

    /* success */
    return (1);
}
