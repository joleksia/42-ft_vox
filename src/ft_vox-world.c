#include "ft_vox.h"

/* static API */

static const int FT_VOX_CHUNK_NEIGHBORS[] = {  0.0,  0.0, -1.0, /* north (-z) */
                                               0.0,  0.0,  1.0, /* south (+z) */
                                              -1.0,  0.0,  0.0, /* west  (-x) */
                                               1.0,  0.0,  0.0, /* east  (+x) */
                                               0.0, -1.0,  0.0, /* down  (-y) */
                                               0.0,  1.0,  0.0, /* top   (+y) */ };

/* public API */

extern int ftWorldCreate(struct World *self, const int seed) {
    /* null-check */
    if (!self) { return (0); }
    memset(self, 0, sizeof(*self));

    /* alloc. 'ecs' */
    struct
    EntitySystem *ecs = (struct EntitySystem *) calloc(1, sizeof(struct EntitySystem));
    if (!ecs) { return (0); }
    else {
        ftEntitySystemCreate(ecs, self);
        self->ecs = ecs;
    }

    /* alloc. 'chunks' */
    struct
    Chunk **chunks = (struct Chunk **) calloc(FT_VOX_RENDER_DIAMETER_X *
                                              FT_VOX_RENDER_DIAMETER_Y *
                                              FT_VOX_RENDER_DIAMETER_Z, sizeof(struct Chunk *)); 
    if (!chunks) { return (0); }
    self->chunks = chunks;

    /* store 'seed' */
    self->seed = seed;

    /* re-center the world at [0, 0, 0] */
    return (ftWorldSetCenter(self, alg_vec3(0.0, 0.0, 0.0)));
}


extern int ftWorldDelete(struct World *self) {
    /* null-check */
    if (!self) { return (0); }

    /* release 'ecs' */
    struct
    EntitySystem *ecs = self->ecs;
    if (ecs) {
        ftEntitySystemDelete(ecs); free(ecs);
    }
    
    /* iterate on every 'chunk' */
    for (int y = 0; y < FT_VOX_RENDER_DIAMETER_X; y++) {
    for (int x = 0; x < FT_VOX_RENDER_DIAMETER_Y; x++) {
    for (int z = 0; z < FT_VOX_RENDER_DIAMETER_Z; z++) {
        struct Chunk *chunk = self->chunks[FT_VOX_WORLD_3D_1D(x, y, z)];
        
        /* release 'chunk' */
        ftChunkRelease(chunk);
        free(chunk);
    }}}

    /* release 'chunks' */
    free(self->chunks);

    /* success */
    return (1);
}


extern int ftWorldGenerate(struct World *self) {
    /* null-check */
    if (!self) { return (0); }

    /* iterate over every 'chunk' */
    for (int y = 0; y < FT_VOX_RENDER_DIAMETER_X; y++) {
    for (int x = 0; x < FT_VOX_RENDER_DIAMETER_Y; x++) {
    for (int z = 0; z < FT_VOX_RENDER_DIAMETER_Z; z++) {
        struct
        Chunk *chunk = self->chunks[FT_VOX_WORLD_3D_1D(x, y, z)];
        if (!chunk) { continue; }

        /* generate CPU-data of this 'chunk' */
        if (!ftChunkGenerateMesh(chunk)) {
            return (0);
        }
    }}}

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
        Chunk *chunk = self->chunks[FT_VOX_WORLD_3D_1D(x, y, z)];

        (void) chunk;
    }}}

    /* success */
    return (1);
}

extern int ftWorldRender(struct World *self, struct VertexArray *pVao, struct Shader *pShader) {
    /* null-check */
    if (!self) { return (0); }
    
    /* render 'ecs' */
    struct
    EntitySystem *ecs = self->ecs;
    if (ecs) {
        ftEntitySystemRender(ecs, pVao, pShader);
    }

    /* iterate on every 'chunk' */
    for (int y = 0; y < FT_VOX_RENDER_DIAMETER_X; y++) {
    for (int x = 0; x < FT_VOX_RENDER_DIAMETER_Y; x++) {
    for (int z = 0; z < FT_VOX_RENDER_DIAMETER_Z; z++) {
        struct
        Chunk *chunk = self->chunks[FT_VOX_WORLD_3D_1D(x, y, z)];
        if (chunk) {
            ftChunkRender(chunk, pVao, pShader);
        }
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
    vec3 position = alg_vec3(center.x - FT_VOX_RENDER_DISTANCE_X,
                             center.y - FT_VOX_RENDER_DISTANCE_Y,
                             center.z - FT_VOX_RENDER_DISTANCE_Z);

    /* check if 'block' equals to 'position' */
    if (alg_vec3eq(position, self->position)) {
        return (0);
    }

    /* re-center 'self' */
    self->position = position;

    /* 'old' chunks, 'self->chunks' back-up */
    size_t count = FT_VOX_RENDER_DIAMETER_X *
                   FT_VOX_RENDER_DIAMETER_Y *
                   FT_VOX_RENDER_DIAMETER_Z;

    struct
    Chunk **old = (struct Chunk **) calloc(count, sizeof(struct Chunk *));
    if (!old) { return (0); }

    /* copy 'self->chunks' to 'old' */
    memcpy(old, self->chunks, count * sizeof(struct Chunk *));

    /* clear 'self->chunks' */
    memset(self->chunks, 0, count * sizeof(struct Chunk *));

    /* PASS 1.: copy 'old' chunks to 'self->chunks' if in-bounds */ 
    for (size_t i = 0; i < count; i++) {
        struct
        Chunk *chunk = old[i];
        if (!chunk) { continue; }

        if (ftWorldPointInBounds(self, chunk->offset)) {
            int x = chunk->offset.x - self->position.x,
                y = chunk->offset.y - self->position.y,
                z = chunk->offset.z - self->position.z;
            self->chunks[FT_VOX_WORLD_3D_1D(x, y, z)] = chunk;
        }
        /* out-of-bounds 'chunk' are released */
        else {
            ftChunkRelease(chunk);
            free(chunk);
        }
    }

    /* release 'old' */
    free(old);

    /* PASS 2.: realloc. freed chunks into new valid chunks */
    for (int z = 0; z < FT_VOX_RENDER_DIAMETER_Z; z++) {
    for (int y = 0; y < FT_VOX_RENDER_DIAMETER_Y; y++) {
    for (int x = 0; x < FT_VOX_RENDER_DIAMETER_X; x++) {
        size_t i = FT_VOX_WORLD_3D_1D(x, y, z);
        if (self->chunks[i] != 0) { continue; }

        /* alloc. 'chunk' */
        struct
        Chunk *chunk = (struct Chunk *) calloc(1, sizeof(struct Chunk));
        if (!chunk) { continue; } /* ? */

        /* CPU-generate this 'chunk' */
        ftChunkAlloc(chunk);
        ftChunkCreate(chunk, self, (int) position.x + x,
                                   (int) position.y + y,
                                   (int) position.z + z);
        ftChunkGenerateData(chunk);

        /* set 'chunk' back to 'self->chunks' */
        self->chunks[i] = chunk;

        /* set 'chunk's neighbors to 'dirty' */
        for (size_t j = 0; j < 6; j++) {
            int nx = x + FT_VOX_CHUNK_NEIGHBORS[j * 3 + 0],
                ny = y + FT_VOX_CHUNK_NEIGHBORS[j * 3 + 1],
                nz = z + FT_VOX_CHUNK_NEIGHBORS[j * 3 + 2];
            if ((uint) nx >= FT_VOX_RENDER_DIAMETER_X ||
                (uint) ny >= FT_VOX_RENDER_DIAMETER_Y ||
                (uint) nz >= FT_VOX_RENDER_DIAMETER_Z
            ) {
                continue;
            }

            struct
            Chunk *chunk = self->chunks[FT_VOX_WORLD_3D_1D(nx, ny, nz)];
            if (chunk) {
                chunk->flags.dirty = 1;
            }
        }
    }}}

    /* success */
    return (1);
}


extern int ftWorldGetChunk(struct World *self, struct Chunk **pChunk, int cx, int cy, int cz) {
    /* null-check */
    if (!self) { return (0); }

    cx -= (int) self->position.x;
    cy -= (int) self->position.y;
    cz -= (int) self->position.z;

    struct
    Chunk *chunk = 0;
    if ((uint) cx >= FT_VOX_RENDER_DIAMETER_X ||
        (uint) cy >= FT_VOX_RENDER_DIAMETER_Y ||
        (uint) cz >= FT_VOX_RENDER_DIAMETER_Z
    ) {
        if (pChunk) { *pChunk = chunk; }
        return (0);
    }

    /* return 'chunk' */
    chunk = self->chunks[FT_VOX_WORLD_3D_1D(cx, cy, cz)];
    if (pChunk) { *pChunk = chunk; }

    /* success */
    return (1);
}


extern int ftWorldPointInBounds(struct World *self, vec3 point) {
    /* null-check */
    if (!self) { return (0); }

    /* in-bounds 'status' */
    vec3 p = alg_vec3sub(point, self->position);
    int status = (uint) p.x < FT_VOX_RENDER_DIAMETER_X &&
                 (uint) p.y < FT_VOX_RENDER_DIAMETER_Y &&
                 (uint) p.z < FT_VOX_RENDER_DIAMETER_Z;

    /* return 'status' */
    return (status);
}
