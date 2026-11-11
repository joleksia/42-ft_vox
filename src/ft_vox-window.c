#include "ft_vox.h"

/* 'Window' global object */

struct Window *Window = { 0 };

/* public API */

extern int ftWindowCreate(const char *pTitle, const size_t width, const size_t height) {
    /* alloc. global 'Window' object */
    if (!Window) {
        Window = (struct Window *) malloc(sizeof(struct Window));
        if (!Window) {
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
    void *window = SDL_CreateWindow(pTitle, SDL_WINDOWPOS_CENTERED,
                                            SDL_WINDOWPOS_CENTERED,
                                            width, height,
                                            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Quit();
        free(Window);
        return (0);
    }

    /* create OpenGL 'context' */
    void *context = SDL_GL_CreateContext(window);
    if (!context) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        free(Window);
        return (0);
    }

    if (SDL_GL_MakeCurrent(window, context)) {
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        free(Window);
        return (0);
    }

    /* initialize 'glld.h' */
    if (glld_loader(0, (glld_loader_t) SDL_GL_GetProcAddress) != GL_NO_ERROR) {
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        free(Window);
        return (0);
    }

    /* return results */
    Window->window  = window;
    Window->context = context;

    /* set-up configuration 'flags' */
    Window->flags.close = 0;
    Window->flags.vsync = 0;

    /* configure OpenGL */
    SDL_GL_SetSwapInterval(Window->flags.vsync);
    
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    /* success */
    return (1);
}


extern int ftWindowDelete(void) {
    /* null-check */
    if (!Window) { return (0); }

    /* release 'Window's resources */
    void *context = Window->context;
    if (context) { SDL_GL_DeleteContext(context); }

    void *window = Window->window;
    if (window) { SDL_DestroyWindow(window); }
    
    /* quit 'glld.h' */
    glld_unload(0);
    
    /* quit 'SDL2' */
    SDL_Quit();

    /* release 'Window' */
    free(Window); Window = 0;

    /* success */
    return (1);
}



extern int ftWindowPollEvents(void) {
    /* null-check */
    if (!Window) { return (0); }

    /* swap window buffers */
    SDL_GL_SwapWindow(Window->window);

    /* update 'Window's configuration */
    SDL_GL_SetSwapInterval(Window->flags.vsync);

    /* sync input objects */
    ftKeyboardSync();
    ftMouseSync();
    ftTimeSync();

    /* poll events */
    SDL_Event event = { 0 };
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case (SDL_QUIT): { Window->flags.close = 1; } break;

            case (SDL_MOUSEMOTION): {
                Mouse->position.previous = Mouse->position.current;
                Mouse->position.current = alg_vec2(event.motion.x,
                                                  event.motion.y);

                if (Mouse->locked) {
                    vec2 relative = alg_vec2(event.motion.xrel,
                                             event.motion.yrel);
                    Mouse->position.delta = alg_vec2add(Mouse->position.delta, relative);
                }
            } break;

            case (SDL_MOUSEBUTTONUP):
            case (SDL_MOUSEBUTTONDOWN): {
                uint8_t button = event.button.button;
                Mouse->input.current[button] = event.button.state;
            } break;

            case (SDL_KEYUP):
            case (SDL_KEYDOWN): {
                SDL_Scancode code = event.key.keysym.scancode;
                Keyboard->current[code] = event.key.state;
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


extern int ftWindowSetTitle(const char *format, ...) {
    /* null-check */
    if (!Window) { return (0); }

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

    /* set 'buffer' as 'Window's title */
    SDL_SetWindowTitle(Window->window, buffer);

    /* success */
    return (1);
}


extern int ftWindowGetSize(int *pX, int *pY) {
    /* null-check */
    if (!Window) { return (0); }

    /* get 'Window's size */
    int x = 0,
        y = 0;
    SDL_GL_GetDrawableSize(Window->window, &x, &y);

    /* return results */
    if (pX) { *pX = x; }
    if (pY) { *pY = y; }

    /* success */
    return (1);
}
