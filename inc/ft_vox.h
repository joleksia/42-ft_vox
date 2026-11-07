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

typedef struct GLvertex GLvertex;

struct GLvertex {
    vec3 position;
    vec3 normal;
    vec2 texcoord;
};


/* ft_vox-shader.c */

typedef GLuint GLshader;

extern GLint ftLoadShaderGLSL(GLshader, GLenum, FILE *);

extern GLint ftLoadShaderSPIRV(GLshader, GLenum, FILE *);


/* ft_vox-mesh.c */

typedef struct Mesh Mesh;

struct Mesh {
    GLuint vbo; /* 'vbo' - vertex data buffer */
    GLuint ibo; /* 'ibo' - index data buffer  */
};

extern int ftMeshCreate(Mesh *);

extern int ftMeshDelete(Mesh *);

extern int ftMeshSetData(Mesh *, const GLvoid *, const GLsizei, const GLvoid *, const GLsizei);

extern int ftMeshUpload(Mesh *, const GLuint);

extern int ftMeshGetSize(Mesh *, GLsizei *, GLsizei *);


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
};

extern int ftCamera3DMatrix(Camera3D *, mat4 *, mat4 *, vec2);


/* ft_vox-chunk.c */

# define FT_VOX_CHUNK_W 16 /* chunk width (size along X-axis)  */
# define FT_VOX_CHUNK_H 16 /* chunk height (size along Y-axis) */
# define FT_VOX_CHUNK_D 16 /* chunk depth (size along Z-axis)  */
# define FT_VOX_CHUNK_S (FT_VOX_CHUNK_W * FT_VOX_CHUNK_H * FT_VOX_CHUNK_D)

typedef struct Chunk Chunk;

struct Chunk {
    /* 'data' : CPU numeric representation of the chunk (width * height * depth) */
    uint64_t *data;

    /* 'mesh' : GPU representation of the chunk (baked once, based on 'data') */
    Mesh mesh;

    /* 'dirty' : dirty-flag representing if chunk was modified */
    uint8_t dirty;
};

extern int ftChunkCreate(Chunk *);

extern int ftChunkDelete(Chunk *);

extern int ftChunkUpload(Chunk *, const GLuint);

extern int ftChunkGetBlock(Chunk *, const vec3, uint64_t *);

extern int ftChunkSetBlock(Chunk *, const vec3, const uint64_t);

/* ... */

#endif /* _ft_vox_h_ */
