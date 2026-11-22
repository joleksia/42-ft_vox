#include "ft_vox.h"

/* public API */

extern int ft_world_create(struct world_s **self, const int seed) {
    /* null-check */
    if (!self) { return (0); }

    /* alloc. 'world' */
    struct world_s *world = (struct world_s *) calloc(1, sizeof(struct world_s));
    if (!world) { return (0); }

    /* alloc. 'ecs' */
    struct
    ecs_s *ecs = 0; 
    if (!ft_ecs_create(&ecs, world)) {
        ft_world_delete(world);
        return (0);
    }
    world->ecs = ecs;

    /* alloc 'pool' */
    struct
    thpool_s *pool = 0; 
    if (!ft_thpool_create(&pool)) {
        ft_world_delete(world);
        return (0);
    }
    world->pool = pool;

    /* alloc. 'chunks' */
    struct
    chunk_s **chunks = (struct chunk_s **) calloc(FT_VOX_RENDER_DIAMETER, sizeof(struct chunk_s *)); 
    if (!chunks) {
        ft_world_delete(world);
        return (0);
    }
    world->chunks = chunks;

    /* store 'seed' */
    world->seed = seed;

    /* return 'self' */
    *self = world;

    /* re-center the world at [0, 0, 0] */
    return (ft_world_center(*self, alg_vec3(0.0, 0.0, 0.0)));
}


extern int ft_world_delete(struct world_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* release 'pool' */
    struct
    thpool_s *pool = self->pool;
    if (pool) {
        ft_thpool_delete(pool);
    }

    /* release 'ecs' */
    struct
    ecs_s *ecs = self->ecs;
    if (ecs) {
        ft_ecs_delete(ecs);
    }
   
    /* release 'chunks' */
    struct
    chunk_s **chunks = self->chunks;
    if (chunks) {
        /* iterate on every 'chunk' */
        for (int y = 0; y < FT_VOX_RENDER_DIAMETER_X; y++) {
        for (int x = 0; x < FT_VOX_RENDER_DIAMETER_Y; x++) {
        for (int z = 0; z < FT_VOX_RENDER_DIAMETER_Z; z++) {
            /* release 'chunk' */
            struct chunk_s *chunk = chunks[FT_VOX_WORLD_3D_1D(x, y, z)];
            if (chunk) {
                ft_chunk_delete(chunk);
            }
        }}}

        free(chunks);
    }

    /* release 'self' */
    free(self);

    /* success */
    return (1);
}


extern int ft_world_update(struct world_s *self) {
    /* null-check */
    if (!self) { return (0); }
    
    /* update 'ecs' */
    struct
    ecs_s *ecs = self->ecs;
    if (ecs) {
        ft_ecs_update(ecs);
    }
    
    /* iterate on every 'chunk' */
    for (int y = 0; y < FT_VOX_RENDER_DIAMETER_X; y++) {
    for (int x = 0; x < FT_VOX_RENDER_DIAMETER_Y; x++) {
    for (int z = 0; z < FT_VOX_RENDER_DIAMETER_Z; z++) {
        struct
        chunk_s *chunk = self->chunks[FT_VOX_WORLD_3D_1D(x, y, z)];
        if (!chunk) { continue; }

        ft_chunk_update(chunk);
    }}}

    /* success */
    return (1);
}

extern int ft_world_render(struct world_s *self, struct shader_s *shader) {
    /* null-check */
    if (!self) { return (0); }
    
    /* render 'ecs' */
    struct
    ecs_s *ecs = self->ecs;
    if (ecs) {
        ft_ecs_render(ecs, shader);
    }

    /* iterate on every 'chunk' */
    for (int y = 0; y < FT_VOX_RENDER_DIAMETER_X; y++) {
    for (int x = 0; x < FT_VOX_RENDER_DIAMETER_Y; x++) {
    for (int z = 0; z < FT_VOX_RENDER_DIAMETER_Z; z++) {
        struct
        chunk_s *chunk = self->chunks[FT_VOX_WORLD_3D_1D(x, y, z)];
        if (!chunk) { continue; }

        ft_chunk_render(chunk, shader);
    }}}

    /* success */
    return (1);
}


extern int ft_world_center(struct world_s *self, const vec3 block) {
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

    /* 'tmp' chunks, 'self->chunks' back-up */
    struct
    chunk_s **tmp = (struct chunk_s **) calloc(FT_VOX_RENDER_DIAMETER, sizeof(struct chunk_s *));
    if (!tmp) {
        return (0);
    }

    memcpy(tmp, self->chunks, FT_VOX_RENDER_DIAMETER* sizeof(struct chunk_s *));

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
        chunk_s *chunk = tmp[i];

        /* 'chunk's current origin : 'cx', 'cy', 'cz' */
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
            ft_chunk_create(&chunk, self, nx, ny, nz);
            ft_chunk_alloc(chunk);
        }

        /* case : 'chunk' is not null */
        else {
            /* case : 'chunk' is out-of-bounds */
            if (ft_world_out_of_bounds(self, cx, cy, cz)) {
                /* drop all the 'chunk' jobs */
                int dropped = ft_thpool_dequeue(self->pool, chunk);
                SDL_AtomicAdd(&chunk->atoms.refer, -dropped);

                /* wait until all remaining 'chunk' jobs finish */
                while (SDL_AtomicGet(&chunk->atoms.refer) > 0) { SDL_Delay(0); }

                /* re-create 'chunk' */
                ft_chunk_clear(chunk);
                ft_chunk_set_origin(chunk, nx, ny, nz);
                SDL_AtomicSet(&chunk->atoms.is_created, 1);
            }
        }

        /* store 'chunk' back in 'self->chunks' */
        self->chunks[FT_VOX_WORLD_3D_1D(sx, sy, sz)] = chunk;
    }}}

    /* release 'tmp' */
    free(tmp);

    /* success */
    return (1);
}


extern int ft_world_get(struct world_s *self, int cx, int cy, int cz, struct chunk_s **result) {
    /* null-check */
    if (!self)  { return (0); }
    
    /* read 'origin' */
    vec3 origin = self->origin;
    cx -= (int) origin.x;
    cy -= (int) origin.y;
    cz -= (int) origin.z;
    
    /* check if 'chunk' is out-of-bounds */
    if ((u32) cx >= FT_VOX_RENDER_DIAMETER_X ||
        (u32) cy >= FT_VOX_RENDER_DIAMETER_Y ||
        (u32) cz >= FT_VOX_RENDER_DIAMETER_Z
    ) {
        *result = 0; return (0);
    }

    /* return 'result' */
    if (result) { *result = self->chunks[FT_VOX_WORLD_3D_1D(cx, cy, cz)]; }

    /* success */
    return (1);
}


extern int ft_world_get_near(struct world_s *self, int x, int y, int z, struct chunk_s **result) {
    /* null-check */
    if (!self) { return (0); }

    /* get the list of 'chunks' */
    struct
    chunk_s *chunks[6] = { 0 };
    ft_world_get(self, x,     y,     z - 1, &chunks[FT_FACE_NORTH]);
    ft_world_get(self, x,     y,     z + 1, &chunks[FT_FACE_SOUTH]);
    ft_world_get(self, x - 1, y,     z,     &chunks[FT_FACE_WEST]);
    ft_world_get(self, x + 1, y,     z,     &chunks[FT_FACE_EAST]);
    ft_world_get(self, x,     y - 1, z,     &chunks[FT_FACE_DOWN]);
    ft_world_get(self, x,     y + 1, z,     &chunks[FT_FACE_TOP]);

    /* copy 'chunks' to 'result' */
    if (result) { memcpy(result, chunks, sizeof(chunks)); }

    /* success */
    return (1);
}


extern int ft_world_out_of_bounds(struct world_s *self, int cx, int cy, int cz) {
    /* null-check */
    if (!self) { return (0); }

    /* in-bounds 'status' */
    cx -= self->origin.x;
    cy -= self->origin.y;
    cz -= self->origin.z;
    int status = (u32) cx >= FT_VOX_RENDER_DIAMETER_X ||
                 (u32) cy >= FT_VOX_RENDER_DIAMETER_Y ||
                 (u32) cz >= FT_VOX_RENDER_DIAMETER_Z;

    /* return 'status' */
    return (status);
}
