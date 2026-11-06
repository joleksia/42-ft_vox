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

/* ... */

#endif /* _ft_vox_h_ */
