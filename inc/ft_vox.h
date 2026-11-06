#if defined (__cplusplus)
# pragma once
#endif
#
#if !defined (_ft_vox_h_)
# define _ft_vox_h_ 1
#
# include <stdio.h>
# include <stddef.h>
# include <stdint.h>
# include <stdlib.h>
#
# include "glld.h"
# include "perlin.h"
# include "algebra.h"
#
# include <SDL2/SDL.h>
# include <SDL2/SDL_video.h>
# include <SDL2/SDL_events.h>

/* ft_vox-shader.c */

typedef GLuint GLshader;

extern GLint ftLoadShaderGLSL(GLshader, GLenum, FILE *);

extern GLint ftLoadShaderSPIRV(GLshader, GLenum, FILE *);

/* ft_vox-camera.c */

/* 'Camera2D' : orthographic 2D projection structure.
 * */

typedef struct Camera2D Camera2D;

struct Camera2D {
    /* 'target' : world-space target point */
    vec2 target;

    /* 'offset' : screen-space offset */
    vec2 offset;

    /* 'rotation' : degrees rotation around target pivot point */
    float rotation;

    /* 'scale' : camera scale/zoom, must be non-zero value */
    float scale;
};

extern int ftCamera2DMatrix(Camera2D *, mat4 *, mat4 *, vec2);


/* 'Camera3D' : perspective 3D projection structure.
 * */

typedef struct Camera3D Camera3D;

struct Camera3D {
    /* 'position' : current camera position */
    vec3 position;

    /* 'rotation' : current camera rotation target */
    vec3 rotation;

    /* 'direction' : internal camera direction vector:
     *  > 'x' : roll, X-axis (default = 0.0f, unused)
     *  > 'y' : pitch, Y-axis (default = 0.0f)
     *  > 'z' : yaw, Z-axis (default = -90.0f)
     * */
    vec3 direction;

    /* 'up' : camera's "up" vector (preferably: [0.0, 1.0, 0.0]) */
    vec3 up;

    /* 'fieldOfView' : value in degrees of current field of view */
    float fieldOfView;

    /* 'sensitivity' : rotation sensitivity */
    float sensitivity;
};

extern int ftCamera3DMatrix(Camera3D *, mat4 *, mat4 *, vec2);


/* ... */

#endif /* _ft_vox_h_ */
