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
# include <SDL2/SDL_mutex.h>
# include <SDL2/SDL_thread.h>
# include <SDL2/SDL_events.h>

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

struct window_s;

struct keyboard_s;

struct mouse_s;

struct time_s;

struct entity_s;

struct ecs_s;

struct shader_s;

struct vao_s;

struct thpool_s;

struct chunk_s;

struct world_s;


/* ft_vox-window.c */

struct window_s {
    void *window;
    void *context;

    struct {
        u8 close : 1;
        u8 vsync : 1;
    } flags;
};


extern struct window_s *WINDOW;

extern int ft_window_create(const char *, const size_t, const size_t);

extern int ft_window_delete(void);

extern int ft_window_poll_events(void);

extern int ft_window_title(const char *, ...);

extern int ft_window_size(int *, int *);

/* ft_vox-input.c */
    
/* create 'keyboard_s' struct */
struct keyboard_s {
    u8 current[SDL_NUM_SCANCODES];
    u8 previous[SDL_NUM_SCANCODES];
};

extern struct keyboard_s *KEYBOARD;


extern int ft_keyboard_create(void);

extern int ft_keyboard_delete(void);

extern int ft_keyboard_sync(void);


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

extern struct mouse_s *MOUSE;


extern int ft_mouse_create(void);

extern int ft_mouse_delete(void);

extern int ft_mouse_sync(void);


/* create 'time_s' struct */
struct time_s {
    double current;
    double previous;
    double delta;
};

extern struct time_s *TIME;


extern int ft_time_create(void);

extern int ft_time_delete(void);

extern int ft_time_sync(void);


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
    int (*render) (struct entity_s *, struct vao_s *, struct shader_s *);
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


extern int ft_ecs_create(struct ecs_s *, struct world_s *);

extern int ft_ecs_delete(struct ecs_s *);

extern int ft_ecs_update(struct ecs_s *);

extern int ft_ecs_render(struct ecs_s *, struct vao_s *, struct shader_s *);

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

    FT_UNIFORM_MATRIX4X4,

    /* ... */

    FT_UNIFORM_TYPE_COUNT
};


struct shader_s {
    /* 'id' : shader program's ID */
    GLuint id;

    /* 'pipeline' : shader's exec. pipeline:
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


extern int ft_shader_create(struct shader_s **);

extern int ft_shader_delete(struct shader_s *);

extern int ft_shader_load(struct shader_s *, FILE *, enum shader_type_e);

extern int ft_shader_uniform(struct shader_s *, const char *, enum uniform_type_e, const void *);


/* ft_vox-vertex-array.c */

struct vao_s {
    /* 'id' : vertex array object's ID */
    GLuint id;
};


extern int ft_vao_create(struct vao_s **);

extern int ft_vao_delete(struct vao_s *);

extern int ft_vao_layoutf(struct vao_s *, const size_t, const size_t, const size_t);

extern int ft_vao_layoutd(struct vao_s *, const size_t, const size_t, const size_t);

extern int ft_vao_layoutb(struct vao_s *, const size_t, const size_t, const size_t);

extern int ft_vao_layouts(struct vao_s *, const size_t, const size_t, const size_t);

extern int ft_vao_layouti(struct vao_s *, const size_t, const size_t, const size_t);

extern int ft_vao_layoutl(struct vao_s *, const size_t, const size_t, const size_t);

extern int ft_vao_vbo(struct vao_s *, const GLuint);

extern int ft_vao_ibo(struct vao_s *, const GLuint);


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


extern int ft_thpool_create(struct thpool_s *);

extern int ft_thpool_delete(struct thpool_s *);

extern int ft_thpool_queue(struct thpool_s *, void *, int (*) (void *));

extern int ft_thpool_dequeue(struct thpool_s *, void *);


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

struct chunk_mesh_s {
    struct {
        GLuint  id;
        GLsizei ids;
    } vbo, sbo, tbo;
};


struct chunk_data_s {
    u32 *id;
    size_t ids,
           idc;
    
    u32 *vb;
    size_t vbs,
           vbc;
    
    u32 *sb;
    size_t sbs,
           sbc;
    
    u32 *tb;
    size_t tbs,
           tbc;
};


enum chunk_state_e {
    FT_CHUNK_CREATE,
    FT_CHUNK_RENDER,

    FT_CHUNK_CPU_QUEUED,
    FT_CHUNK_CPU_FINISH,

    FT_CHUNK_GPU_QUEUED,
    FT_CHUNK_GPU_FINISH,
};


struct chunk_s {
    /* 'parent' : world reference */
    struct
    world_s *parent;

    /* 'atoms' : chunk atoms */
    struct {
        SDL_atomic_t state; /* 'state' : chunk's state atom */
        SDL_atomic_t dirty; /* 'dirty' : chunk's change atom */
        SDL_atomic_t refer; /* 'refer' : chunk's reference count */
    } atoms;

    struct
    chunk_data_s data;

    struct
    chunk_mesh_s mesh;

    /* 'origin' : 'parent' position (in chunks) */
    vec3 origin;

    /* 'position' : 'parent' position (in blocks) */
    vec3 offset;
};

extern int ft_chunk_create(struct chunk_s *, struct world_s *, int, int, int);

extern int ft_chunk_delete(struct chunk_s *);

extern int ft_chunk_update(struct chunk_s *);

extern int ft_chunk_render(struct chunk_s *, struct vao_s *, struct shader_s *);

extern int ft_chunk_alloc(struct chunk_s *);

extern int ft_chunk_clear(struct chunk_s *);

extern int ft_chunk_free(struct chunk_s *);

extern int ft_chunk_generate(struct chunk_s *);

extern int ft_chunk_mesh_cpu(struct chunk_s *);

extern int ft_chunk_mesh_gpu(struct chunk_s *);

extern int ft_chunk_get(struct chunk_s *, int, int, int);

extern int ft_chunk_set(struct chunk_s *, int, int, int, int);


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

    /* 'center' : current world's center position (in chunks) */
    vec3 center;

    /* 'origin' : current world's origin position (in chunks) */
    vec3 origin;

    /* 'seed' : 32-bit seed of the world */
    u32 seed;
};


extern int ft_world_create(struct world_s *, const int);

extern int ft_world_delete(struct world_s *);

extern int ft_world_update(struct world_s *);

extern int ft_world_render(struct world_s *, struct vao_s *, struct shader_s *);

extern int ft_world_center(struct world_s *, const vec3);

extern int ft_world_get(struct world_s *, struct chunk_s **, int, int, int);

extern int ft_world_out_of_bounds(struct world_s *, int, int, int);

/* ... */

#endif /* _ft_vox_h_ */
