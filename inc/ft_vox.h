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
# include "stb_image.h"
#
# include <SDL2/SDL.h>
# include <SDL2/SDL_video.h>
# include <SDL2/SDL_mutex.h>
# include <SDL2/SDL_thread.h>
# include <SDL2/SDL_events.h>
#
# if !defined (countof)
#  define countof(arr) ((int) (sizeof((arr)) / sizeof(*(arr))))
# endif

/* typedefs */

typedef uint8_t u8;

typedef int8_t i8;

typedef int16_t i16;

typedef uint16_t u16;

typedef uint32_t u32;

typedef int32_t i32;

typedef uint64_t u64;

typedef int64_t i64;


/* Forward-Declare */

struct state_s;

struct window_s;

struct keyboard_s;

struct mouse_s;

struct time_s;

struct entity_s;

struct ecs_s;

struct shader_s;

struct thpool_s;

struct chunk_s;

struct world_s;


/* ft_vox-state.c */

struct state_s {
    /* 'window' : global window object */
    struct
    window_s *window;

    /* 'time' : global timing object */
    struct
    time_s *time;

    /* 'input' : global input objects */
    struct {
        struct
        keyboard_s *keyboard;
        
        struct
        mouse_s *mouse;
    } input;

    /* 'gfx' : global graphics objects */
    struct {
        /* 'solid' : solid blocks shader */
        struct
        shader_s *solid;

        /* 'trans' : transparent blocks shader */
        struct
        shader_s *trans;

        /* 'water' : liquid blocks shader */
        struct
        shader_s *water;

        /* 'atlas1' : 2D texture atlas of blocks */
        struct
        sampler_s *atlas1;

        mat4 proj; /* 'proj' : projection matrix */
        mat4 view; /* 'view' : view matrix */
    } gfx;

    /* 'world' : current world */
    struct
    world_s *world;
};


extern struct state_s STATE;


/* ft_vox-window.c */

struct window_s {
    void *window;
    void *context;

    struct {
        u8 close : 1;
        u8 vsync : 1;
    } flags;
};


extern int ft_window_create(struct window_s **, const char *, const size_t, const size_t);

extern int ft_window_delete(struct window_s *);

extern int ft_window_poll_events(struct window_s *);

extern int ft_window_get_size(struct window_s *, int *, int *);

/* ft_vox-input.c */
    
/* create 'keyboard_s' struct */
struct keyboard_s {
    u8 current[SDL_NUM_SCANCODES];
    u8 previous[SDL_NUM_SCANCODES];
};


extern int ft_keyboard_create(struct keyboard_s **);

extern int ft_keyboard_delete(struct keyboard_s *);

extern int ft_keyboard_sync(struct keyboard_s *);


/* create 'mouse_s' struct */
struct mouse_s {
    struct {
        vec2 current;
        vec2 previous;
        vec2 delta;
    } position;
    struct {
        u8 current[8];
        u8 previous[8];
    } input;
    u8 locked;
};


extern int ft_mouse_create(struct mouse_s **);

extern int ft_mouse_delete(struct mouse_s *);

extern int ft_mouse_sync(struct mouse_s *);


/* create 'time_s' struct */
struct time_s {
    double current;
    double previous;
    double delta;
};


extern int ft_time_create(struct time_s **);

extern int ft_time_delete(struct time_s *);

extern int ft_time_sync(struct time_s *);


/* ft_vox-entity.c */

struct entity_s {
    /* 'parent' : ecs_s-parent object */
    struct
    ecs_s *parent;

    /* 'id' : ID of this entity, used for O(1) look-up into 'parent' */
    size_t id;

    /* methods used by this entity */
    int (*create) (struct entity_s *);
    int (*delete) (struct entity_s *);
    int (*update) (struct entity_s *);
    int (*render) (struct entity_s *);
};


struct ecs_s {
    /* 'parent' : parent reference */
    struct
    world_s *parent;

    /* 'entities' : vector of entities */
    struct {
        struct
        entity_s **data;
        size_t count;
        size_t capacity;
    } entities;
};


extern int ft_ecs_create(struct ecs_s **, struct world_s *);

extern int ft_ecs_delete(struct ecs_s *);

extern int ft_ecs_update(struct ecs_s *);

extern int ft_ecs_render(struct ecs_s *);

extern int ft_ecs_insert(struct ecs_s *, struct entity_s *);


/* ft_vox-shader.c */

enum shader_type_e {
    FT_SHADER_VERTEX = 0,

    FT_SHADER_TESS_CTRL = 1,

    FT_SHADER_TESS_EVAL = 2,
    
    FT_SHADER_GEOMETRY = 3,
    
    FT_SHADER_FRAGMENT = 4,
    
    FT_SHADER_COMPUTE = 5,

    /* ... */

    FT_SHADER_TYPE_COUNT
};


enum uniform_type_e {
    FT_UNIFORM_NONE = 0,

    FT_UNIFORM_INT1D,
    FT_UNIFORM_INT2D,
    FT_UNIFORM_INT3D,
    FT_UNIFORM_INT4D,

    FT_UNIFORM_UINT1D,
    FT_UNIFORM_UINT2D,
    FT_UNIFORM_UINT3D,
    FT_UNIFORM_UINT4D,

    FT_UNIFORM_FLOAT1D,
    FT_UNIFORM_FLOAT2D,
    FT_UNIFORM_FLOAT3D,
    FT_UNIFORM_FLOAT4D,
    
    FT_UNIFORM_MATRIX2,
    FT_UNIFORM_MATRIX3,
    FT_UNIFORM_MATRIX4,

    /* ... */

    FT_UNIFORM_TYPE_COUNT
};


struct shader_s {
    /* 'id' : shader program's ID */
    GLuint id;

    /* 'pipeline' : shader's exec. pipeline */
    GLuint pipeline[FT_SHADER_TYPE_COUNT];

    /* 'uniforms' : shader's uniform location cache */
    struct {
        const char *name;
        GLint       location;
    } uniforms[64];
};


extern int ft_shader_create(struct shader_s **);

extern int ft_shader_delete(struct shader_s *);

extern int ft_shader_bind(struct shader_s *);

extern int ft_shader_load(struct shader_s *, FILE *, enum shader_type_e);

extern int ft_shader_uniform(struct shader_s *, const char *, enum uniform_type_e, const void *);

extern int ft_shader_uniform_get(struct shader_s *, const char *);


/* ft_vox-sampler.c */

struct sampler_s {
    /* 'id' : sampler's ID */
    GLuint id;

    GLsizei width,   /* 'width' : width of a sampler */
            height,  /* 'height' : height of a sampler */
            channel; /* 'channel' : count of channels of a sampler */
};


extern int ft_sampler_create(struct sampler_s **, const char *);

extern int ft_sampler_delete(struct sampler_s *);

extern int ft_sampler_bind(struct sampler_s *, const u32);


/* ft_vox-camera.c */

struct camera_s {
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

extern int ft_camera_move(struct camera_s *, float);

extern int ft_camera_rotate(struct camera_s *, float);

extern int ft_camera_matrix(struct camera_s *, mat4 *, mat4 *);


/* ft_vox-thread.c */

typedef int (*thread_pool_func_t) (void *);


struct job_node_s {
    /* 'next' : pointer to next 'node' */
    struct
    job_node_s *next;

    /* 'data' : reference to data object of this job */
    void *data;

    /* 'func' : function pointer executed by thread pool */
    thread_pool_func_t func;
};


struct thpool_s {
    /* 'mutex' : main thread pool mutex */
    SDL_mutex *mutex;

    /* 'cond' : signals received to the threads */
    SDL_cond *cond;

    /* 'threads' : array of threads */
    struct {
        size_t size;
        SDL_Thread **data;
    } threads;

    /* 'jobs' : queue of jobs */
    struct {
        size_t size;
        struct
        job_node_s *head,
                   *tail;
    } jobs;

    /* 'alive' : if thread pool is alive */
    SDL_atomic_t alive;
};


extern int ft_thpool_create(struct thpool_s **);

extern int ft_thpool_delete(struct thpool_s *);

extern int ft_thpool_queue(struct thpool_s *, void *, int (*) (void *));

extern int ft_thpool_dequeue(struct thpool_s *, void *);


/* ft_vox-block.c */

# if !defined (FT_VOX_BLOCK_SIZE_X)
#  define FT_VOX_BLOCK_SIZE_X 16 /* block X-axis size in pixels */
# endif
# 
# if !defined (FT_VOX_BLOCK_SIZE_Y)
#  define FT_VOX_BLOCK_SIZE_Y 16 /* block Y-axis size in pixels */
# endif

enum face_e {
    FT_FACE_NORTH = 0,
    FT_FACE_SOUTH = 1,
    FT_FACE_WEST = 2,
    FT_FACE_EAST = 3,
    FT_FACE_DOWN = 4,
    FT_FACE_TOP = 5,

    /* ... */

    FT_FACE_COUNT
};



enum block_type_e {
    FT_BLOCK_TYPE_TRANS = 0,
    FT_BLOCK_TYPE_SOLID = 1,
    FT_BLOCK_TYPE_WATER = 2,

    /* ... */

    FT_BLOCK_TYPE_COUNT
};


enum block_e {
    FT_BLOCK_AIR = 0,

    FT_BLOCK_STONE,

    FT_BLOCK_GRASS,

    FT_BLOCK_DIRT,

    FT_BLOCK_WATER,

    /* ... */

    FT_BLOCK_COUNT,

    /* ancillary blocks */

    FT_BLOCK_DEBUG,

    FT_BLOCK_OUT_OF_BOUNDS

};


struct block_s {
    /* 'id' : 16-bit numerical representation of a block */
    enum
    block_e id : 16;

    /* 'type' : 8-bit block type ('solid', 'trans', 'sprite', etc.) */
    enum
    block_type_e type : 8;

    int (*texcoord) (struct block_s *, vec2 *, enum face_e);
};

extern struct block_s BLOCKS[INT16_MAX];


extern int ft_blocks_create(void);

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
# define FT_VOX_CHUNK_SIZE (FT_VOX_CHUNK_SIZE_X * \
                            FT_VOX_CHUNK_SIZE_Y * \
                            FT_VOX_CHUNK_SIZE_Z)
#
# define FT_VOX_CHUNK_3D_1D(x, y, z) ((x) + \
                                      (y) * FT_VOX_CHUNK_SIZE_X + \
                                      (z) * FT_VOX_CHUNK_SIZE_X * FT_VOX_CHUNK_SIZE_Y)
#
# define FT_VOX_CHUNK_1D_3D(i, x, y, z) ((z) = ((i)) / (FT_VOX_CHUNK_SIZE_X * FT_VOX_CHUNK_SIZE_Y),\
                                         (y) = ((i) / FT_VOX_CHUNK_SIZE_X) % (FT_VOX_CHUNK_SIZE_Y),\
                                         (x) = ((i)) % (FT_VOX_CHUNK_SIZE_X))

struct chunk_mesh_s {
    /* NOTE: must match 'enum block_type_e' */
    GLuint vao[3],
           vbo[3],
           ibo[3],
           count[3];
};


struct chunk_data_s {
    /* NOTE: must match 'enum block_type_e' */
    struct {
        u32 *arr;
        u32  cnt,
             cap;
    } v[3],
      i[3];
};


struct chunk_s {
    /* 'parent' : world reference */
    struct
    world_s *parent;

    /* 'blocks' : array of blocks */
    u32 *blocks;

    /* 'atoms' : chunk atoms */
    struct {
        /* 'is_created' : chunk is created */
        SDL_atomic_t create;

        /* 'delete' : chunk is queued for deletion */
        SDL_atomic_t delete;
        
        /* 'generate' : chunk is generated */
        SDL_atomic_t generate;
        
        /* 'meshed' : chunk is meshed */
        SDL_atomic_t meshed;

        /* 'upload' : chunk is uploaded */
        SDL_atomic_t upload;

        /* 'dirty' : chunk changed */
        SDL_atomic_t dirty;

        /* 'populated' : chunk is populated by blocks */
        SDL_atomic_t populated;

        /* 'refer' : chunk's reference count */
        SDL_atomic_t refer;
    } atoms;

    /* 'mesh' : GPU chunk data */
    struct
    chunk_mesh_s mesh;

    /* 'data' : CPU chunk data */
    struct
    chunk_data_s data;

    /* 'origin' : 'parent' position (in chunks) */
    vec3 origin;

    /* 'position' : 'parent' position (in blocks) */
    vec3 offset;
};

/* basic func. */

extern int ft_chunk_create(struct chunk_s **, struct world_s *, int, int, int);

extern int ft_chunk_delete(struct chunk_s *);

extern int ft_chunk_update(struct chunk_s *);

extern int ft_chunk_render(struct chunk_s *);

/* memory func. */

extern int ft_chunk_alloc(struct chunk_s *);

/* getters/setters func. */

extern int ft_chunk_get(struct chunk_s *, int, int, int, u32 *);

extern int ft_chunk_get_near(struct chunk_s *, int, int, int, u32 *);

extern int ft_chunk_set(struct chunk_s *, int, int, int, int);

/* pipeline func. */

extern int ft_chunk_generate(struct chunk_s *);

extern int ft_chunk_mesh_bake(struct chunk_s *);

extern int ft_chunk_mesh_upload(struct chunk_s *);


/* ft_vox-world.c */

# if !defined (FT_VOX_RENDER_DISTANCE_X)
#  define FT_VOX_RENDER_DISTANCE_X 8 /* world X-axis render distance */
#  define FT_VOX_RENDER_DIAMETER_X (FT_VOX_RENDER_DISTANCE_X * 2)
# endif
#
# if !defined (FT_VOX_RENDER_DISTANCE_Y)
#  define FT_VOX_RENDER_DISTANCE_Y 8 /* world Y-axis render distance */
#  define FT_VOX_RENDER_DIAMETER_Y (FT_VOX_RENDER_DISTANCE_Y * 2)
# endif
#
# if !defined (FT_VOX_RENDER_DISTANCE_Z)
#  define FT_VOX_RENDER_DISTANCE_Z 8 /* world Z-axis render distance */
#  define FT_VOX_RENDER_DIAMETER_Z (FT_VOX_RENDER_DISTANCE_Z * 2)
# endif
#
# define FT_VOX_RENDER_DISTANCE (FT_VOX_RENDER_DISTANCE_X * \
                                 FT_VOX_RENDER_DISTANCE_Y * \
                                 FT_VOX_RENDER_DISTANCE_Z)
#
# define FT_VOX_RENDER_DIAMETER (FT_VOX_RENDER_DIAMETER_X * \
                                 FT_VOX_RENDER_DIAMETER_Y * \
                                 FT_VOX_RENDER_DIAMETER_Z)
#
# define FT_VOX_WORLD_3D_1D(x, y, z) ((x) + \
                                      (y) * FT_VOX_RENDER_DIAMETER_X + \
                                      (z) * FT_VOX_RENDER_DIAMETER_X * FT_VOX_RENDER_DIAMETER_Y)
#
# define FT_VOX_WORLD_1D_3D(i, x, y, z) ((z) = ((i)) / (FT_VOX_RENDER_DIAMETER_X * FT_VOX_RENDER_DIAMETER_Y),\
                                         (y) = ((i) / FT_VOX_RENDER_DIAMETER_X) % (FT_VOX_RENDER_DIAMETER_Y),\
                                         (x) = ((i)) % (FT_VOX_RENDER_DIAMETER_X))
#
# define FT_VOX_BLOCK_TO_CHUNK(x, y, z) (alg_vec3(floorf((x) / FT_VOX_CHUNK_SIZE_X), \
                                                  floorf((y) / FT_VOX_CHUNK_SIZE_Y), \
                                                  floorf((z) / FT_VOX_CHUNK_SIZE_Z)))
#
# define FT_VOX_FLOAT_TO_BLOCK(x, y, z) (alg_vec3(floorf((x)), \
                                                  floorf((y)), \
                                                  floorf((z))))

struct world_s {
    /* 'ecs' : 'ecs_s' object */
    struct
    ecs_s *ecs;

    /* 'pool' : 'thpool_s' object */
    struct
    thpool_s *pool;

    /* 'chunks' : array of chunk pointers */
    struct
    chunk_s **chunks;

    /* 'delete' : array of chunks to be deleted */
    struct
    chunk_s **delete;

    /* 'center' : current world's center position (in chunks) */
    vec3 center;

    /* 'origin' : current world's origin position (in chunks) */
    vec3 origin;

    /* 'seed' : 32-bit seed of the world */
    u32 seed;
};

/* basic func. */

extern int ft_world_create(struct world_s **, const int);

extern int ft_world_delete(struct world_s *);

extern int ft_world_update(struct world_s *);

extern int ft_world_render(struct world_s *);

/* getter/setter func. */

extern int ft_world_get(struct world_s *, int, int, int, struct chunk_s **);

extern int ft_world_get_near(struct world_s *, int, int, int, struct chunk_s **);

/* other func. */

extern int ft_world_center(struct world_s *, const vec3);

extern int ft_world_flush_delete(struct world_s *);

extern int ft_world_out_of_bounds(struct world_s *, int, int, int);


/* ft_vox-worldgen.c */

# if !defined (FT_VOX_WORLDGEN_SEA_LEVEL)
#  define FT_VOX_WORLDGEN_SEA_LEVEL (0)
# endif

extern int ft_worldgen_generate(struct world_s *, struct chunk_s *, int, int, int);

/* ... */

#endif /* _ft_vox_h_ */
