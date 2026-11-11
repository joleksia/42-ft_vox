#include "ft_vox.h"

/* 'Keyboard' struct */

struct Keyboard *Keyboard = 0;


extern int ftKeyboardCreate(void) {
    /* null-check */
    if (Keyboard) { return (0); }

    /* alloc. 'keyboard' */
    struct Keyboard *keyboard = (struct Keyboard *) malloc(sizeof(struct Keyboard));
    if (!keyboard) { return (0); }

    /* return 'keyboard' */
    Keyboard = keyboard;

    /* run 'sync' */
    return (ftKeyboardSync());
}


extern int ftKeyboardDelete(void) {
    /* null-check */
    if (!Keyboard) { return (0); }

    /* release 'Keyboard' */
    free(Keyboard); Keyboard = 0;

    /* success */
    return (1);
}


extern int ftKeyboardSync(void) {
    /* null-check */
    if (!Keyboard) { return (0); }

    /* copy 'current' inputs to 'previous' inputs */
    memcpy(Keyboard->previous,
           Keyboard->current,
           sizeof(Keyboard->current));

    /* success */
    return (1);
}


/* 'Mouse' struct */

struct Mouse *Mouse = 0;

extern int ftMouseCreate(void) {
    /* null-check */
    if (Mouse) { return (0); }

    /* alloc. 'mouse' object */
    struct Mouse *mouse = (struct Mouse *) malloc(sizeof(struct Mouse));
    if (!mouse) { return (0); }

    /* return 'mouse' */
    Mouse = mouse;

    /* run 'sync' */
    return (ftMouseSync());
}


extern int ftMouseDelete(void) {
    /* null-check */
    if (!Mouse) { return (0); }

    /* release 'Mouse' */
    free(Mouse); Mouse = 0;

    /* success */
    return (0);
}


extern int ftMouseSync(void) {
    /* null-check */
    if (!Mouse) { return (0); }

    /* copy 'current' inputs to 'previous' inputs */
    memcpy(Mouse->input.previous,
           Mouse->input.current,
           sizeof(Mouse->input.current));

    /* success */
    return (1);
}


/* 'Time' struct */

struct Time *Time = 0;

extern int ftTimeCreate(void) {
    /* null-check */
    if (Time) { return (0); }

    /* alloc. 'mouse' object */
    struct Time *mouse = (struct Time *) malloc(sizeof(struct Time));
    if (!mouse) { return (0); }

    /* return 'mouse' */
    Time = mouse;

    /* run 'sync' */
    return (ftTimeSync());
}


extern int ftTimeDelete(void) {
    /* null-check */
    if (!Time) { return (0); }

    /* release 'Time' */
    free(Time); Time = 0;

    /* success */
    return (0);
}


extern int ftTimeSync(void) {
    /* null-check */
    if (!Time) { return (0); }

    /* update 'Time' components */
    Time->previous = Time->current;
    Time->current = SDL_GetPerformanceCounter();
    Time->delta = (Time->current - Time->previous) / (double) SDL_GetPerformanceFrequency();

    /* success */
    return (1);
}
