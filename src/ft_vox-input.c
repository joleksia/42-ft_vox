#include "ft_vox.h"

/* public API */

extern int ft_keyboard_create(struct keyboard_s **self) {
    /* null-check */
    if (!self) { return (0); }

    /* alloc. 'keyboard' */
    struct keyboard_s *keyboard = (struct keyboard_s *) calloc(1, sizeof(struct keyboard_s));
    if (!keyboard) { return (0); }

    /* return 'self' */
    *self = keyboard;

    /* run 'sync' */
    return (ft_keyboard_sync(keyboard));
}


extern int ft_keyboard_delete(struct keyboard_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* release 'self' */
    free(self);

    /* success */
    return (1);
}


extern int ft_keyboard_sync(struct keyboard_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* copy 'current' inputs to 'previous' inputs */
    memcpy(self->previous,
           self->current,
           sizeof(self->current));

    /* success */
    return (1);
}


extern int ft_mouse_create(struct mouse_s **self) {
    /* null-check */
    if (!self) { return (0); }

    /* alloc. 'mouse' */
    struct mouse_s *mouse = (struct mouse_s *) calloc(1, sizeof(struct mouse_s));
    if (!mouse) { return (0); }

    /* return 'self' */
    *self = mouse;

    /* run 'sync' */
    return (ft_mouse_sync(mouse));
}


extern int ft_mouse_delete(struct mouse_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* release 'self' */
    free(self);

    /* success */
    return (1);
}


extern int ft_mouse_sync(struct mouse_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* copy 'current' inputs to 'previous' inputs */
    memcpy(self->input.previous,
           self->input.current,
           sizeof(self->input.current));

    /* success */
    return (1);
}


extern int ft_time_create(struct time_s **self) {
    /* null-check */
    if (!self) { return (0); }

    /* alloc. 'time' */
    struct time_s *time = (struct time_s *) calloc(1, sizeof(struct time_s));
    if (!time) { return (0); }
    
    /* read 'time' */
    time->previous = SDL_GetPerformanceCounter();
    time->current = SDL_GetPerformanceCounter();
    time->delta = (time->current - time->previous) / (double) SDL_GetPerformanceFrequency();

    /* return 'self' */
    *self = time;

    /* run 'sync' */
    return (ft_time_sync(time));
}


extern int ft_time_delete(struct time_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* release 'self' */
    free(self);

    /* success */
    return (1);
}


extern int ft_time_sync(struct time_s *self) {
    /* null-check */
    if (!self) { return (0); }

    self->previous = self->current;
    self->current = SDL_GetPerformanceCounter();
    self->delta = (self->current - self->previous) / (double) SDL_GetPerformanceFrequency();

    /* success */
    return (1);
}
