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

typedef enum ShaderType ShaderType;

enum ShaderType {
    FT_SHADER_VERTEX = 0,

    FT_SHADER_TESS_CTRL = 1,

    FT_SHADER_TESS_EVAL = 2,
    
    FT_SHADER_GEOMETRY = 3,
    
    FT_SHADER_FRAGMENT = 4,
    
    FT_SHADER_COMPUTE = 5,

    /* ... */

    FT_SHADER_TYPE_COUNT
};


typedef enum ShaderUniformType ShaderUniformType;

enum ShaderUniformType {
    FT_UNIFORM_NONE = 0,

    FT_UNIFORM_MATRIX4X4,

    /* ... */

    FT_UNIFORM_TYPE_COUNT
};


typedef struct Shader Shader;

struct Shader {
    /* Shader 'program' */
    GLuint program;

    /* Shader 'pipeline':
     * - [0] : vertex
     * - [1] : tess. control
     * - [2] : tess. evaluation 
     * - [3] : geometry
     * - [4] : fragment
     * - [5] : compute
     * - ... : padding
     * */
    GLuint pipeline[8];
};


extern int ftShaderCreate(Shader **);

extern int ftShaderDelete(Shader *);

extern int ftShaderLoadGLSL(Shader *, FILE *, ShaderType);

extern int ftShaderUniformSet(Shader *, const char *, ShaderUniformType, const void *);


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

struct BlockMesh {
    struct {
        GLuint  id;
        GLsizei sz;
    } vbo, ibo[2];
};

/* ft_vox-chunk.c */

# if !defined (FT_VOX_CHUNK_BLOCKS_X)
#  define FT_VOX_CHUNK_BLOCKS_X 32 /* chunk X-axis size in blocks */
# endif
# 
# if !defined (FT_VOX_CHUNK_BLOCKS_Y)
#  define FT_VOX_CHUNK_BLOCKS_Y 32 /* chunk Y-axis size in blocks */
# endif
# 
# if !defined (FT_VOX_CHUNK_BLOCKS_Z)
#  define FT_VOX_CHUNK_BLOCKS_Z 32 /* chunk Z-axis size in blocks */
# endif
#
# define FT_VOX_CHUNK_BLOCKS_S (FT_VOX_CHUNK_BLOCKS_X * FT_VOX_CHUNK_BLOCKS_Y * FT_VOX_CHUNK_BLOCKS_Z)
#
# define FT_VOX_CHUNK_3D_1D(x, y, z) (x + y * FT_VOX_CHUNK_BLOCKS_X + z * FT_VOX_CHUNK_BLOCKS_X * FT_VOX_CHUNK_BLOCKS_Y)

struct Chunk {
    /* 'world' : reference to the 'World' struct */
    World *world;

    /* 'data' : CPU representation of the block in the chunk */
    uint32_t *data;

    /* 'mesh' : GPU representation of solid/trans. blocks */
    BlockMesh *mesh;

    /* 'position' : numerical position in world-space */
    vec3 position;

    /* indices into 'pWorld->chunks' */
    int cx, cy, cz;

    /* 'flags' : 1-bit chunk flags */
    struct {
        uint8_t dirty : 1; /* 'dirty' : dirty-flag, if '1' then re-run CPU-to-GPU process */
        uint8_t empty : 1; /* 'empty' : if '1' then chunk has no blocks */
        uint8_t modified : 1; /* 'modified' : if '1' then chunk was modified by the player */
    } flags;
};

extern int ftChunkDelete(Chunk *);

extern int ftChunkGenCPU(Chunk *, const int);

extern int ftChunkGenGPU(Chunk *);

extern int ftChunkGetBlock(Chunk *, const vec3, uint32_t *);

extern int ftChunkSetBlock(Chunk *, const vec3, const uint32_t);


/* ft_vox-world.c */

# if !defined (FT_VOX_WORLD_BLOCKS_X)
#  define FT_VOX_WORLD_BLOCKS_X 16384 /* world X-axis size in blocks */
# endif
#
# if !defined (FT_VOX_WORLD_BLOCKS_Y)
#  define FT_VOX_WORLD_BLOCKS_Y 16384 /* world Y-axis size in blocks */
# endif
#
# if !defined (FT_VOX_WORLD_BLOCKS_Z)
#  define FT_VOX_WORLD_BLOCKS_Z 16384 /* world Z-axis size in blocks */
# endif
#
# if !defined (FT_VOX_RENDER_DISTANCE_X)
#  define FT_VOX_RENDER_DISTANCE_X 2 /* world X-axis render distance */
#  define FT_VOX_RENDER_DIAMETER_X (FT_VOX_RENDER_DISTANCE_X * 2)
# endif
#
# if !defined (FT_VOX_RENDER_DISTANCE_Y)
#  define FT_VOX_RENDER_DISTANCE_Y 2 /* world Y-axis render distance */
#  define FT_VOX_RENDER_DIAMETER_Y (FT_VOX_RENDER_DISTANCE_Y * 2)
# endif
#
# if !defined (FT_VOX_RENDER_DISTANCE_Z)
#  define FT_VOX_RENDER_DISTANCE_Z 2 /* world Z-axis render distance */
#  define FT_VOX_RENDER_DIAMETER_Z (FT_VOX_RENDER_DISTANCE_Z * 2)
# endif
#
# define FT_VOX_WORLD_3D_1D(x, y, z) (x + y * FT_VOX_RENDER_DIAMETER_X + z * FT_VOX_RENDER_DIAMETER_X * FT_VOX_RENDER_DIAMETER_Y)

struct World {

    /* 'chunks' : array of chunk pointers */
    Chunk **chunks;

    /* 'seed' : 32-bit seed of the world */
    int seed;

};

extern int ftWorldCreate(World *, const int);

extern int ftWorldDelete(World *);

extern int ftWorldUpdate(World *);

extern int ftWorldRender(World *, const GLuint, Shader *);

extern int ftWorldChunkTo3D(World *, Chunk *, int *, int *, int *);

extern int ftWorldGetChunk(World *, Chunk **, int, int, int);

/* ... */

#endif /* _ft_vox_h_ */
