#if defined (__cplusplus)
# pragma once
#endif
#
#if !defined (_ft_vox_h_)
# define _ft_vox_h_ 1
#
# include <time.h>
# include <stdio.h>
# include <stddef.h>
# include <stdint.h>
# include <stdlib.h>
# include <stdarg.h>
#
# include "glld.h"
# include "perlin.h"
# include "algebra.h"
#
# include <SDL2/SDL.h>
# include <SDL2/SDL_video.h>
# include <SDL2/SDL_events.h>

/* typedefs */

typedef signed char byte;

typedef unsigned char ubyte;

typedef unsigned int uint;


/* Forward-Declare */

struct GLvertex;

struct Window;

struct Keyboard;

struct Mouse;

struct Time;

struct Entity;

struct EntitySystem;

struct Shader;

struct VertexArray;

struct BlockMesh;

struct Chunk;

struct World;


/* ft_vox-window.c */

struct Window {
    void *window;
    void *context;

    struct {
        ubyte close : 1;
        ubyte vsync : 1;
    } flags;
};


extern struct Window *Window;

extern int ftWindowCreate(const char *, const size_t, const size_t);

extern int ftWindowDelete(void);

extern int ftWindowPollEvents(void);

extern int ftWindowSetTitle(const char *, ...);

extern int ftWindowGetSize(int *, int *);

/* ft_vox-input.c */
    
/* create 'Keyboard' struct */
struct Keyboard {
    ubyte current[SDL_NUM_SCANCODES];
    ubyte previous[SDL_NUM_SCANCODES];
};

extern struct Keyboard *Keyboard;


extern int ftKeyboardCreate(void);

extern int ftKeyboardDelete(void);

extern int ftKeyboardSync(void);


/* create 'Mouse' struct */
struct Mouse {
    struct {
        vec2 current;
        vec2 previous;
        vec2 delta;
    } position;
    struct {
        ubyte current[8];
        ubyte previous[8];
    } input;
    ubyte locked;
};

extern struct Mouse *Mouse;


extern int ftMouseCreate(void);

extern int ftMouseDelete(void);

extern int ftMouseSync(void);


/* create 'Time' struct */
struct Time {
    double current;
    double previous;
    double delta;
};

extern struct Time *Time;


extern int ftTimeCreate(void);

extern int ftTimeDelete(void);

extern int ftTimeSync(void);


/* ft_vox-entity.c */

struct Entity {
    /* 'parent' : EntitySystem-parent object */
    struct
    EntitySystem *parent;

    /* 'ID' : ID of this entity, used for O(1) look-up into 'parent' */
    size_t ID;

    /* methods used by this entity */
    int (*create) (struct Entity *);
    int (*delete) (struct Entity *);
    int (*update) (struct Entity *);
    int (*render) (struct Entity *, struct VertexArray *, struct Shader *);
};


struct EntitySystem {
    /* 'parent' : parent reference */
    struct
    World *parent;

    /* 'entities' : vector of entities */
    struct {
        struct
        Entity **data;
        size_t count;
        size_t capacity;
    } entities;
};


extern int ftEntitySystemCreate(struct EntitySystem *, struct World *);

extern int ftEntitySystemDelete(struct EntitySystem *);

extern int ftEntitySystemUpdate(struct EntitySystem *);

extern int ftEntitySystemRender(struct EntitySystem *, struct VertexArray *, struct Shader *);

extern int ftEntitySystemInsert(struct EntitySystem *, struct Entity *);


/* ft_vox-shader.c */

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


enum ShaderUniformType {
    FT_UNIFORM_NONE = 0,

    FT_UNIFORM_MATRIX4X4,

    /* ... */

    FT_UNIFORM_TYPE_COUNT
};


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


extern int ftShaderCreate(struct Shader **);

extern int ftShaderDelete(struct Shader *);

extern int ftShaderLoadGLSL(struct Shader *, FILE *, enum ShaderType);

extern int ftShaderUniformSet(struct Shader *, const char *, enum ShaderUniformType, const void *);


/* ft_vox-vertex-array.c */

struct VertexArray {
    /* vao' 'ID' */
    GLuint ID;
};


extern int ftVertexArrayCreate(struct VertexArray **);

extern int ftVertexArrayDelete(struct VertexArray *);

extern int ftVertexArrayLayoutFloat(struct VertexArray *, const size_t, const size_t, const size_t);

extern int ftVertexArrayLayoutDouble(struct VertexArray *, const size_t, const size_t, const size_t);

extern int ftVertexArrayLayoutByte(struct VertexArray *, const size_t, const size_t, const size_t);

extern int ftVertexArrayLayoutShort(struct VertexArray *, const size_t, const size_t, const size_t);

extern int ftVertexArrayLayoutInt(struct VertexArray *, const size_t, const size_t, const size_t);

extern int ftVertexArrayLayoutLong(struct VertexArray *, const size_t, const size_t, const size_t);

extern int ftVertexArrayVertexBuffer(struct VertexArray *, const GLuint);

extern int ftVertexArrayIndexBuffer(struct VertexArray *, const GLuint);


/* ft_vox-camera.c */

/* 'Camera' : perspective 3D projection structure.
 * */

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

extern int ftCameraMove(struct Camera *, float);

extern int ftCameraRotate(struct Camera *, float);

extern int ftCameraMatrix(struct Camera *, mat4 *, mat4 *);


/* ft_vox-mesh.c */

struct BlockMesh {
    struct {
        GLuint  id;
        GLsizei sz;
    } vbo, ibo[2];
};


/* ft_vox-chunk.c */

# if !defined (FT_VOX_CHUNK_SIZE_X)
#  define FT_VOX_CHUNK_SIZE_X 32 /* chunk X-axis size in blocks */
# endif
# 
# if !defined (FT_VOX_CHUNK_SIZE_Y)
#  define FT_VOX_CHUNK_SIZE_Y 32 /* chunk Y-axis size in blocks */
# endif
# 
# if !defined (FT_VOX_CHUNK_SIZE_Z)
#  define FT_VOX_CHUNK_SIZE_Z 32 /* chunk Z-axis size in blocks */
# endif
#
# define FT_VOX_CHUNK_SIZE (FT_VOX_CHUNK_SIZE_X * FT_VOX_CHUNK_SIZE_Y * FT_VOX_CHUNK_SIZE_Z)
#
# define FT_VOX_CHUNK_3D_1D(x, y, z) (x + y * FT_VOX_CHUNK_SIZE_X + z * FT_VOX_CHUNK_SIZE_X * FT_VOX_CHUNK_SIZE_Y)

struct Chunk {
    /* 'parent' : world reference */
    struct
    World *parent;

    /* 'data' : CPU representation of the block in the chunk */
    uint *data;

    /* 'mesh' : GPU representation of solid/trans. blocks */
    struct
    BlockMesh *mesh;

    /* 'offset' : position (in chunks) */
    vec3 offset;

    /* 'position' : position (in blocks) */
    vec3 position;

    /* 'flags' : 1-bit chunk flags */
    struct {
        ubyte dirty : 1; /* 'dirty' : dirty-flag, if '1' then re-run GPU meshing */
        ubyte empty : 1; /* 'empty' : if '1' then chunk has no blocks */
        ubyte ready : 1; /* 'ready' : flag telling if this chunk is ready to use */
    } flags;
};

extern int ftChunkCreate(struct Chunk *, struct World *, int, int, int);

extern int ftChunkAlloc(struct Chunk *);

extern int ftChunkRelease(struct Chunk *);

extern int ftChunkRender(struct Chunk *, struct VertexArray *, struct Shader *);

extern int ftChunkGenerateData(struct Chunk *, const int);

extern int ftChunkGenerateMesh(struct Chunk *);

extern int ftChunkGetBlock(struct Chunk *, int, int, int);

extern int ftChunkSetBlock(struct Chunk *, int, int, int, uint);


/* ft_vox-world.c */

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
# define FT_VOX_WORLD_3D_1D(x, y, z) ((x) + \
                                      (y) * FT_VOX_RENDER_DIAMETER_X + \
                                      (z) * FT_VOX_RENDER_DIAMETER_X * FT_VOX_RENDER_DIAMETER_Y)
#
# define FT_VOX_BLOCK_TO_CHUNK(x, y, z) (alg_vec3(floorf((x) / FT_VOX_CHUNK_SIZE_X), \
                                                  floorf((y) / FT_VOX_CHUNK_SIZE_Y), \
                                                  floorf((z) / FT_VOX_CHUNK_SIZE_Z)))
#
# define FT_VOX_FLOAT_TO_BLOCK(x, y, z) (alg_vec3(floorf((x)), \
                                                  floorf((y)), \
                                                  floorf((z))))

struct World {

    /* 'ecs' : 'EntitySystem' object */
    struct
    EntitySystem *ecs;

    /* 'chunks' : array of chunk pointers */
    struct
    Chunk **chunks;

    /* 'position' : current world's center position */
    vec3 position;

    /* 'seed' : 32-bit seed of the world */
    int seed;

};


extern int ftWorldCreate(struct World *, const int);

extern int ftWorldDelete(struct World *);

extern int ftWorldGenerate(struct World *);

extern int ftWorldUpdate(struct World *);

extern int ftWorldRender(struct World *, struct VertexArray *, struct Shader *);

extern int ftWorldSetCenter(struct World *, const vec3);

extern int ftWorldGetChunk(struct World *, struct Chunk **, int, int, int);

extern int ftWorldPointInBounds(struct World *, vec3);

/* ... */

#endif /* _ft_vox_h_ */
