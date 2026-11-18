#include "ft_vox.h"

/* public API */

extern int ftWorldCreate(struct World *self, const int seed) {
    /* null-check */
    if (!self) { return (0); }
    memset(self, 0, sizeof(*self));

    /* alloc. 'ecs' */
    struct
    EntitySystem *ecs = (struct EntitySystem *) calloc(1, sizeof(struct EntitySystem));
    if (!ecs) {
        ftWorldDelete(self);
        return (0);
    }

    /* create 'ecs' */
    ftEntitySystemCreate(ecs, self);
    self->ecs = ecs;

    /* alloc 'pool' */
    struct
    ThreadPool *pool = (struct ThreadPool *) calloc(1, sizeof(struct ThreadPool));
    if (!pool) {
        ftWorldDelete(self);
        return (0);
    }

    /* create 'pool' */
    ftThreadPoolCreate(pool);
    self->pool = pool;
    
    size_t count = FT_VOX_RENDER_DIAMETER_X *
                   FT_VOX_RENDER_DIAMETER_Y *
                   FT_VOX_RENDER_DIAMETER_Z;

    /* alloc. 'loaded' */
    struct
    Chunk **loaded = (struct Chunk **) calloc(count, sizeof(struct Chunk *)); 
    if (!loaded) {
        ftWorldDelete(self);
        return (0);
    }
    
    self->loaded = loaded;

    /* store 'seed' */
    self->seed = seed;

    /* re-center the world at [0, 0, 0] */
    return (ftWorldSetCenter(self, alg_vec3(0.0, 0.0, 0.0)));
}


extern int ftWorldDelete(struct World *self) {
    /* null-check */
    if (!self) { return (0); }

    /* release 'pool' */
    struct
    ThreadPool *pool = self->pool;
    if (pool) {
        ftThreadPoolDelete(pool); free(pool);
    }

    /* release 'ecs' */
    struct
    EntitySystem *ecs = self->ecs;
    if (ecs) {
        ftEntitySystemDelete(ecs); free(ecs);
    }
   
    /* release 'loaded' */
    struct
    Chunk **loaded = self->loaded;
    if (loaded) {
        /* iterate on every 'chunk' */
        for (int y = 0; y < FT_VOX_RENDER_DIAMETER_X; y++) {
        for (int x = 0; x < FT_VOX_RENDER_DIAMETER_Y; x++) {
        for (int z = 0; z < FT_VOX_RENDER_DIAMETER_Z; z++) {
            /* release 'chunk' */
            struct Chunk *chunk = loaded[FT_VOX_WORLD_3D_1D(x, y, z)];
            if (chunk) {
                ftChunkDelete(chunk);
            }
        }}}

        free(loaded);
    }

    /* success */
    return (1);
}


extern int ftWorldUpdate(struct World *self) {
    /* null-check */
    if (!self) { return (0); }
    
    /* update 'ecs' */
    struct
    EntitySystem *ecs = self->ecs;
    if (ecs) {
        ftEntitySystemUpdate(ecs);
    }
    
    /* iterate on every 'chunk' */
    for (int y = 0; y < FT_VOX_RENDER_DIAMETER_X; y++) {
    for (int x = 0; x < FT_VOX_RENDER_DIAMETER_Y; x++) {
    for (int z = 0; z < FT_VOX_RENDER_DIAMETER_Z; z++) {
        struct
        Chunk *chunk = self->loaded[FT_VOX_WORLD_3D_1D(x, y, z)];
        if (!chunk) { continue; }

        ftChunkUpdate(chunk);
    }}}

    /* success */
    return (1);
}

extern int ftWorldRender(struct World *self, struct VertexArray *vao, struct Shader *shader) {
    /* null-check */
    if (!self) { return (0); }
    
    /* render 'ecs' */
    struct
    EntitySystem *ecs = self->ecs;
    if (ecs) {
        ftEntitySystemRender(ecs, vao, shader);
    }

    /* iterate on every 'chunk' */
    for (int y = 0; y < FT_VOX_RENDER_DIAMETER_X; y++) {
    for (int x = 0; x < FT_VOX_RENDER_DIAMETER_Y; x++) {
    for (int z = 0; z < FT_VOX_RENDER_DIAMETER_Z; z++) {
        struct
        Chunk *chunk = self->loaded[FT_VOX_WORLD_3D_1D(x, y, z)];
        if (!chunk) { continue; }

        ftChunkRender(chunk, vao, shader);
    }}}

    /* success */
    return (1);
}


extern int ftWorldSetCenter(struct World *self, const vec3 block) {
    /* null-check */
    if (!self) { return (0); }

    vec3 center = FT_VOX_BLOCK_TO_CHUNK(block.x,
                                        block.y,
                                        block.z);
    vec3 origin = alg_vec3(center.x - FT_VOX_RENDER_DISTANCE_X,
                           center.y - FT_VOX_RENDER_DISTANCE_Y,
                           center.z - FT_VOX_RENDER_DISTANCE_Z);

    /* check if 'origin' changed */
    if (alg_vec3eq(origin, self->origin)) {
        /* success (early) */
        return (1);
    }

    /* 'tmp' chunks, 'self->loaded' back-up */
    size_t count = FT_VOX_RENDER_DIAMETER_X *
                   FT_VOX_RENDER_DIAMETER_Y *
                   FT_VOX_RENDER_DIAMETER_Z;

    struct
    Chunk **tmp = (struct Chunk **) calloc(count, sizeof(struct Chunk *));
    if (!tmp) {
        return (0);
    }

    memcpy(tmp, self->loaded, count * sizeof(struct Chunk *));

    /* back-up 'old' origin */
    vec3 old = self->origin;

    /* re-center 'self' */
    self->center = center;
    self->origin = origin;

    /* iterate over every 'chunk' */
    for (int z = 0; z < FT_VOX_RENDER_DIAMETER_Z; z++) {
    for (int y = 0; y < FT_VOX_RENDER_DIAMETER_Y; y++) {
    for (int x = 0; x < FT_VOX_RENDER_DIAMETER_X; x++) {
        size_t i = FT_VOX_WORLD_3D_1D(x, y, z);
        struct
        Chunk *chunk = tmp[i];

        /* 'chunk's current origin : 'ccx', 'ccy', 'ccz' */
        int cx = chunk ? chunk->origin.x : old.x + x,
            cy = chunk ? chunk->origin.y : old.y + y,
            cz = chunk ? chunk->origin.z : old.z + z;

        /* 'chunk's new slot : 'sx', 'sy', 'sz' */
        int sx = (cx - (int) origin.x) % FT_VOX_RENDER_DIAMETER_X;
        if (sx < 0) { sx += FT_VOX_RENDER_DIAMETER_X; }
        
        int sy = (cy - (int) origin.y) % FT_VOX_RENDER_DIAMETER_Y;
        if (sy < 0) { sy += FT_VOX_RENDER_DIAMETER_Y; }
        
        int sz = (cz - (int) origin.z) % FT_VOX_RENDER_DIAMETER_Z;
        if (sz < 0) { sz += FT_VOX_RENDER_DIAMETER_Z; }

        /* 'chunk's new origin : 'nx', 'ny', 'nz' */
        int nx = origin.x + sx,
            ny = origin.y + sy,
            nz = origin.z + sz;

        /* case : 'chunk' is null */
        if (!chunk) {
            /* alloc. 'chunk' */
            chunk = (struct Chunk *) calloc(1, sizeof(struct Chunk));
            if (!chunk) { continue; }

            ftChunkAlloc(chunk);
            ftChunkCreate(chunk, self, nx, ny, nz);
        }

        /* case : 'chunk' is not null */
        else {
            /* case : 'chunk' is out-of-bounds */
            if (nx != cx || ny != cy || nz != cz) {
                ftThreadPoolDropJobs(self->pool, chunk);
                ftChunkClear(chunk);
                ftChunkCreate(chunk, self, nx, ny, nz);
            }
        }

        /* store 'chunk' back in 'self->loaded' */
        self->loaded[FT_VOX_WORLD_3D_1D(sx, sy, sz)] = chunk;
    }}}

    /* release 'tmp' */
    free(tmp);

    /* success */
    return (1);
}


extern int ftWorldGetChunk(struct World *self, struct Chunk **chunk, int x, int y, int z) {
    /* null-check */
    if (!self)  { return (0); }
    if (!chunk) { return (0); }

    /* check if 'chunk' is out-of-bounds */
    if (ftWorldPointOutOfBounds(self, alg_vec3(x, y, z))) {
        *chunk = 0;
        return (0);
    }

    /* return 'chunk' */
    x -= (int) self->origin.x;
    y -= (int) self->origin.y;
    z -= (int) self->origin.z;
    *chunk = self->loaded[FT_VOX_WORLD_3D_1D(x, y, z)];

    /* success */
    return (1);
}


extern int ftWorldPointOutOfBounds(struct World *self, vec3 point) {
    /* null-check */
    if (!self) { return (0); }

    /* in-bounds 'status' */
    vec3 p = alg_vec3sub(point, self->origin);
    int status = (uint) p.x >= FT_VOX_RENDER_DIAMETER_X &&
                 (uint) p.y >= FT_VOX_RENDER_DIAMETER_Y &&
                 (uint) p.z >= FT_VOX_RENDER_DIAMETER_Z;

    /* return 'status' */
    return (status);
}
