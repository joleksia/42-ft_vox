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

/* Forward-Declare */

typedef struct BlockMesh BlockMesh;

typedef struct Chunk Chunk;

typedef struct World World;

/* Misc. */

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




/* ft_vox-camera.c */

/* 'Camera' : perspective 3D projection structure.
 * */

typedef struct Camera Camera;

struct Camera {
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

extern int ftCameraMatrix(Camera *, mat4 *, mat4 *, vec2);


/* ft_vox-mesh.c */

/* TODO:
 * Create an index-buffer layout of:
 * [...solid indices...][...trans. indices...]
 * */

struct BlockMesh {
    struct {
        GLuint  id;
        GLsizei sz;
    } vbo, ibo[2];
};

/* ft_vox-chunk.c */

# if !defined (FT_VOX_CHUNK_BLOCKS_W)
#  define FT_VOX_CHUNK_BLOCKS_W 32 /* chunk X-axis size in blocks */
# endif
# 
# if !defined (FT_VOX_CHUNK_BLOCKS_H)
#  define FT_VOX_CHUNK_BLOCKS_H 32 /* chunk Y-axis size in blocks */
# endif
# 
# if !defined (FT_VOX_CHUNK_BLOCKS_D)
#  define FT_VOX_CHUNK_BLOCKS_D 32 /* chunk Z-axis size in blocks */
# endif
#
# define FT_VOX_CHUNK_BLOCKS_S (FT_VOX_CHUNK_BLOCKS_W * FT_VOX_CHUNK_BLOCKS_H * FT_VOX_CHUNK_BLOCKS_D)

struct Chunk {
    /* 'world' : reference to the 'World' struct */
    World *world;

    /* 'data' : CPU representation of the block in the chunk */
    uint32_t *data;

    /* 'mesh' : GPU representation of solid/trans. blocks */
    BlockMesh *mesh;

    /* 'offset' : numerical position offset in world-space */
    vec3 offset;

    /* 'seed' : pseudo-random number generation seed used for noise */
    int32_t seed;

    /* 'flags' : 1-bit chunk flags */
    struct {
        uint8_t dirty : 1; /* 'dirty' : dirty-flag, if '1' then re-run CPU-to-GPU process */
        uint8_t empty : 1; /* 'empty' : if '1' then chunk has no blocks */
        uint8_t modified : 1; /* 'modified' : if '1' then chunk was modified by the player */
    } flags;
};

extern int ftChunkCreate(Chunk *, World *, const vec3, const int);

extern int ftChunkDelete(Chunk *);

extern int ftChunkGenCPU(Chunk *);

extern int ftChunkGenGPU(Chunk *);

extern int ftChunkGetBlock(Chunk *, const vec3, uint32_t *);

extern int ftChunkSetBlock(Chunk *, const vec3, const uint32_t);


/* ft_vox-world.c */

# if !defined (FT_VOX_WORLD_BLOCKS_W)
#  define FT_VOX_WORLD_BLOCKS_W 16384 /* world X-axis size in blocks */
# endif
#
# if !defined (FT_VOX_WORLD_BLOCKS_H)
#  define FT_VOX_WORLD_BLOCKS_H 256   /* world Y-axis size in blocks */
# endif
#
# if !defined (FT_VOX_WORLD_BLOCKS_D)
#  define FT_VOX_WORLD_BLOCKS_D 16384 /* world Z-axis size in blocks */
# endif
#
# define FT_VOX_WORLD_CHUNKS_W (FT_VOX_WORLD_BLOCKS_W / FT_VOX_CHUNK_BLOCKS_W) /* world X-axis size in chunks  */
# define FT_VOX_WORLD_CHUNKS_H (FT_VOX_WORLD_BLOCKS_H / FT_VOX_CHUNK_BLOCKS_H) /* world Y-axis size in chunks  */
# define FT_VOX_WORLD_CHUNKS_D (FT_VOX_WORLD_BLOCKS_D / FT_VOX_CHUNK_BLOCKS_D) /* world Z-axis size in chunks  */
# define FT_VOX_WORLD_CHUNKS_S (FT_VOX_WORLD_CHUNKS_W * FT_VOX_WORLD_CHUNKS_H * FT_VOX_WORLD_CHUNKS_D)

struct World {

    /* 'chunks' : chunks manager */
    struct {
        Chunk **used;
        size_t count;
    } chunks;

    /* 'rndrad' : render radius */
    struct {
        size_t x;
        size_t y;
        size_t z;
    } rndrad;
        
};

extern int ftWorldCreate(World *, const int);

extern int ftWorldDelete(World *);

extern int ftWorldUpdate(World *);

extern int ftWorldRender(World *, const GLuint);

/* ... */

#endif /* _ft_vox_h_ */
