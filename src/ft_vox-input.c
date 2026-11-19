#include "ft_vox.h"

/* static API */

struct keyboard_s *KEYBOARD = 0;

struct mouse_s *MOUSE = 0;

struct time_s *TIME = 0;

/* public API */

extern int ft_keyboard_create(void) {
    /* alloc. 'keyboard' */
    if (!KEYBOARD) {
        struct keyboard_s *keyboard = (struct keyboard_s *) calloc(1, sizeof(struct keyboard_s));
        if (!keyboard) { return (0); }

        /* return 'keyboard' */
        KEYBOARD = keyboard;
    }

    /* run 'sync' */
    return (ft_keyboard_sync());
}


extern int ft_keyboard_delete(void) {
    /* null-check */
    if (!KEYBOARD) { return (0); }

    /* release 'KEYBOARD' */
    free(KEYBOARD); KEYBOARD = 0;

    /* success */
    return (1);
}


extern int ft_keyboard_sync(void) {
    /* null-check */
    if (!KEYBOARD) { return (0); }

    /* copy 'current' inputs to 'previous' inputs */
    memcpy(KEYBOARD->previous,
           KEYBOARD->current,
           sizeof(KEYBOARD->current));

    /* success */
    return (1);
}


/* 'MOUSE' struct */

extern int ft_mouse_create(void) {
    /* alloc. 'mouse' object */
    if (!MOUSE) {
        struct mouse_s *mouse = (struct mouse_s *) calloc(1, sizeof(struct mouse_s));
        if (!mouse) { return (0); }

        /* return 'mouse' */
        MOUSE = mouse;
    }

    /* run 'sync' */
    return (ft_mouse_sync());
}


extern int ft_mouse_delete(void) {
    /* null-check */
    if (!MOUSE) { return (0); }

    /* release 'MOUSE' */
    free(MOUSE); MOUSE = 0;

    /* success */
    return (1);
}


extern int ft_mouse_sync(void) {
    /* null-check */
    if (!MOUSE) { return (0); }

    /* copy 'current' inputs to 'previous' inputs */
    memcpy(MOUSE->input.previous,
           MOUSE->input.current,
           sizeof(MOUSE->input.current));

    /* success */
    return (1);
}


/* 'TIME' struct */

extern int ft_time_create(void) {
    /* null-check */
    if (TIME) { return (0); }

    /* alloc. 'time' object */
    if (!TIME) {
        struct time_s *time = (struct time_s *) calloc(1, sizeof(struct time_s));
        if (!time) { return (0); }

        /* return 'time' */
        TIME = time;
    }

    /* read 'time' */
    TIME->previous = SDL_GetPerformanceCounter();
    TIME->current = SDL_GetPerformanceCounter();
    TIME->delta = (TIME->current - TIME->previous) / (double) SDL_GetPerformanceFrequency();

    /* success */
    return (1);
}


extern int ft_time_delete(void) {
    /* null-check */
    if (!TIME) { return (0); }

    /* release 'TIME' */
    free(TIME); TIME = 0;

    /* success */
    return (1);
}


extern int ft_time_sync(void) {
    /* null-check */
    if (!TIME) { return (0); }

    /* update 'TIME' components */
    TIME->previous = TIME->current;
    TIME->current = SDL_GetPerformanceCounter();
    TIME->delta = (TIME->current - TIME->previous) / (double) SDL_GetPerformanceFrequency();

    /* success */
    return (1);
}
