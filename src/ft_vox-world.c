#include "ft_vox.h"

/* public API */

/* basic func. */

extern int ft_world_create(struct world_s **self, const int seed) {
    /* null-check */
    if (!self) { return (0); }

    /* alloc. 'world' */
    struct world_s *world = (struct world_s *) calloc(1, sizeof(struct world_s));
    if (!world) { return (0); }

    /* alloc. 'ecs' */
    if (!ft_ecs_create(&world->ecs, world)) { ft_world_delete(world); return (0); }

    /* alloc 'pool' */
    if (!ft_thpool_create(&world->pool)) { ft_world_delete(world); return (0); }

    /* alloc. 'chunks' */
    world->chunks = (struct chunk_s **) calloc(FT_RENDER_DIAMETER, sizeof(struct chunk_s *)); 
    if (!world->chunks) { ft_world_delete(world); return (0); }

    /* alloc. 'deleted' */
    world->delete = (struct chunk_s **) calloc(FT_RENDER_DIAMETER, sizeof(struct chunk_s *)); 
    if (!world->delete) { ft_world_delete(world); return (0); }

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
    if (pool) { ft_thpool_delete(pool); }

    /* release 'ecs' */
    struct
    ecs_s *ecs = self->ecs;
    if (ecs) { ft_ecs_delete(ecs); }
   
    /* release 'chunks' */
    struct
    chunk_s **chunks = self->chunks;
    if (chunks) {
        /* iterate on every 'chunk' */
        for (int i = 0; i < FT_RENDER_DIAMETER; i++) {
            /* get 'x', 'y' and 'z' */
            int x = 0,
                y = 0,
                z = 0;
            FT_WORLD_1D_3D(i, x, y, z);
            
            /* release 'chunk' */
            struct chunk_s *chunk = chunks[FT_WORLD_3D_1D(x, y, z)];
            if (chunk) {
                ft_chunk_delete(chunk);
            }
        }

        free(chunks);
    }

    /* release 'delete' */
    struct
    chunk_s **delete = self->delete;
    if (delete) {
        /* iterate on every 'delete' */
        for (int i = 0; i < FT_RENDER_DIAMETER; i++) {
            /* release 'chunk' */
            struct chunk_s *chunk = delete[i];
            if (chunk) {
                ft_chunk_delete(chunk);
            }
        }

        free(delete);
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
    for (int i = 0; i < FT_RENDER_DIAMETER; i++) {
        struct
        chunk_s *chunk = self->chunks[i];
        if (!chunk) { continue; }

        ft_chunk_update(chunk);
    }

    /* flush 'delete' array */
    return (ft_world_flush_delete(self));
}

extern int ft_world_render(struct world_s *self) {
    /* null-check */
    if (!self) { return (0); }
    
    /* render 'ecs' */
    struct
    ecs_s *ecs = self->ecs;
    if (ecs) {
        ft_ecs_render(ecs);
    }

    /* iterate on every 'chunk' */
    for (int i = 0; i < FT_RENDER_DIAMETER; i++) {
        struct
        chunk_s *chunk = self->chunks[i];
        if (!chunk) { continue; }

        ft_chunk_render(chunk);
    }

    /* success */
    return (1);
}

/* getter/setter func. */

extern int ft_world_get(struct world_s *self, int cx, int cy, int cz, struct chunk_s **result) {
    /* null-check */
    if (!self)  { return (0); }
    
    /* read 'origin' */
    vec3 origin = self->origin;
    cx -= (int) origin.x;
    cy -= (int) origin.y;
    cz -= (int) origin.z;
    
    /* check if 'chunk' is out-of-bounds */
    if ((u32) cx >= FT_RENDER_DIAMETER_X ||
        (u32) cy >= FT_RENDER_DIAMETER_Y ||
        (u32) cz >= FT_RENDER_DIAMETER_Z
    ) {
        *result = 0; return (0);
    }

    /* return 'result' */
    if (result) { *result = self->chunks[FT_WORLD_3D_1D(cx, cy, cz)]; }

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

/* other func. */

extern int ft_world_center(struct world_s *self, const vec3 block) {
    /* null-check */
    if (!self) { return (0); }

    vec3 center = FT_BLOCK_TO_CHUNK(block.x,
                                    block.y,
                                    block.z);
    vec3 origin = alg_vec3(center.x - FT_RENDER_DISTANCE_X,
                           center.y - FT_RENDER_DISTANCE_Y,
                           center.z - FT_RENDER_DISTANCE_Z);

    /* check if 'origin' changed */
    if (alg_vec3eq(origin, self->origin)) {
        /* success (early) */
        return (1);
    }

    /* 'tmp' chunks, 'self->chunks' back-up */
    struct
    chunk_s **tmp = (struct chunk_s **) calloc(FT_RENDER_DIAMETER, sizeof(struct chunk_s *));
    if (!tmp) {
        return (0);
    }

    memcpy(tmp, self->chunks, FT_RENDER_DIAMETER * sizeof(struct chunk_s *));
    memset(self->chunks, 0, FT_RENDER_DIAMETER * sizeof(struct chunk_s *));

    /* re-center 'self' */
    self->center = center;
    self->origin = origin;

    /* pass 1.: check out-of-bounds */
    for (int i = 0; i < FT_RENDER_DIAMETER; i++) {
        /* get 'chunk' from 'tmp' */
        struct
        chunk_s *chunk = tmp[i];
        if (!chunk) { continue; }

        /* get 'chunk's current position as integers */ 
        int cx = (int) chunk->origin.x,
            cy = (int) chunk->origin.y,
            cz = (int) chunk->origin.z;

        /* case : 'chunk' out-of-bounds */
        if (ft_world_out_of_bounds(self, cx, cy, cz)) {
            /* add 'chunk' to 'delete' array */
            for (int j = 0; j < FT_RENDER_DIAMETER; j++) {
                if (!self->delete[j]) {
                    SDL_AtomicSet(&chunk->atoms.delete, 1);
                    self->delete[j] = chunk;
                    break;
                }
            }
        }
        /* case : 'chunk' in-bound */
        else {
            self->chunks[FT_WORLD_3D_1D(cx - (int) origin.x,
                                        cy - (int) origin.y,
                                        cz - (int) origin.z)] = chunk;
        }
    }

    /* TODO: maybe sort chunks back-to-front or front-to-back? */

    /* pass 2.: create empty chunks */
    for (int i = 0; i < FT_RENDER_DIAMETER; i++) {
        /* get 'chunk' from 'chunks' */
        struct
        chunk_s *chunk = self->chunks[i];
        if (chunk) { continue; }
        
        /* get 'x', 'y' and 'z' */
        int x = 0,
            y = 0,
            z = 0;
        FT_WORLD_1D_3D(i, x, y, z);
        ft_chunk_create(&chunk, self, x + (int) origin.x,
                                      y + (int) origin.y,
                                      z + (int) origin.z);
        ft_chunk_alloc(chunk);

        /* store 'chunk' in 'self->loaded' */
        self->chunks[FT_WORLD_3D_1D(x, y, z)] = chunk;
    }

    /* release 'tmp' */
    free(tmp);

    /* success */
    return (1);
}


extern int ft_world_flush_delete(struct world_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* iterate on 'delete' array */
    struct
    chunk_s **delete = self->delete;
    if (!delete) { return (0); }

    for (int i = 0; i < FT_RENDER_DIAMETER; i++) {
        /* get 'chunk' */
        struct
        chunk_s *chunk = delete[i];
        if (!chunk) { continue; }
        
        /* drop all the 'chunk' jobs */
        int dropped = ft_thpool_dequeue(self->pool, chunk);
        SDL_AtomicAdd(&chunk->atoms.refer, -dropped);

        /* wait for all 'chunk's jobs to finish */
        if (SDL_AtomicGet(&chunk->atoms.refer) == 0) {
            /* release 'delete[i]' */
            ft_chunk_delete(delete[i]); delete[i] = 0;
        }
    }

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
    int status = (u32) cx >= FT_RENDER_DIAMETER_X ||
                 (u32) cy >= FT_RENDER_DIAMETER_Y ||
                 (u32) cz >= FT_RENDER_DIAMETER_Z;

    /* return 'status' */
    return (status);
}
