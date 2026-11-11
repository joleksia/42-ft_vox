#include "ft_vox.h"

extern int ftWorldCreate(struct World *self, const int seed) {
    /* null-check */
    if (!self) { return (0); }

    /* alloc. 'chunks' */
    struct
    Chunk **chunks = (struct Chunk **) malloc(FT_VOX_RENDER_DIAMETER_X *
                                              FT_VOX_RENDER_DIAMETER_Y *
                                              FT_VOX_RENDER_DIAMETER_Z * sizeof(struct Chunk *)); 
    if (!chunks) { return (0); }
    self->chunks = chunks;

    /* iterate on every 'chunk' */
    for (int y = 0; y < FT_VOX_RENDER_DIAMETER_X; y++) {
    for (int x = 0; x < FT_VOX_RENDER_DIAMETER_Y; x++) {
    for (int z = 0; z < FT_VOX_RENDER_DIAMETER_Z; z++) {
        /* alloc. 'chunk' */
        struct
        Chunk *chunk = (struct Chunk *) calloc(1, sizeof(struct Chunk));

        /* 'chunk' 'self' reference */
        chunk->world = self;

        /* 'chunk' 3D-indices */
        chunk->cx = x;
        chunk->cy = y;
        chunk->cz = z;

        /* 'chunk' 3D-position */
        chunk->position = alg_vec3((x - FT_VOX_RENDER_DISTANCE_X) * FT_VOX_CHUNK_SIZE_X,
                                   (y - FT_VOX_RENDER_DISTANCE_Y) * FT_VOX_CHUNK_SIZE_Y,
                                   (z - FT_VOX_RENDER_DISTANCE_Z) * FT_VOX_CHUNK_SIZE_Z);

        /* store 'chunk' into 'chunks' */
        chunks[FT_VOX_WORLD_3D_1D(x, y, z)] = chunk;
    }}}

    /* store 'seed' */
    self->seed = seed;

    /* re-center the world at [0, 0, 0] */
    ftWorldSetCenter(self, alg_vec3(0.0, 0.0, 0.0));

    /* success */
    return (1);
}


extern int ftWorldDelete(struct World *self) {
    /* null-check */
    if (!self) { return (0); }
    
    /* iterate on every 'chunk' */
    for (int y = 0; y < FT_VOX_RENDER_DIAMETER_X; y++) {
    for (int x = 0; x < FT_VOX_RENDER_DIAMETER_Y; x++) {
    for (int z = 0; z < FT_VOX_RENDER_DIAMETER_Z; z++) {
        struct Chunk *chunk = self->chunks[FT_VOX_WORLD_3D_1D(x, y, z)];
        
        /* release CPU resources */
        uint32_t *data = chunk->data;
        if (data) {
            free(data);
        }

        /* release GPU resources */
        struct BlockMesh *mesh = chunk->mesh;
        if (mesh) {
            glDeleteBuffers(1, &mesh->vbo.id);
            glDeleteBuffers(1, &mesh->ibo[0].id);
            glDeleteBuffers(1, &mesh->ibo[1].id);
            free(mesh);
        }
        
        /* release 'chunk' */
        free(chunk);
    }}}

    /* release 'chunks' */
    free(self->chunks);

    /* success */
    return (1);
}


/* TODO:
 * Add multi-threading or something?
 * */
extern int ftWorldGenerate(struct World *self) {
    /* null-check */
    if (!self) { return (0); }

    /* Pass 1.: CPU data */
    for (int y = 0; y < FT_VOX_RENDER_DIAMETER_X; y++) {
    for (int x = 0; x < FT_VOX_RENDER_DIAMETER_Y; x++) {
    for (int z = 0; z < FT_VOX_RENDER_DIAMETER_Z; z++) {
        /* generate CPU-data of this 'chunk' */
        if (!ftChunkGenerateData(self->chunks[FT_VOX_WORLD_3D_1D(x, y, z)], self->seed)) { return (0); }
    }}}

    /* Pass 2.: GPU data */
    for (int y = 0; y < FT_VOX_RENDER_DIAMETER_X; y++) {
    for (int x = 0; x < FT_VOX_RENDER_DIAMETER_Y; x++) {
    for (int z = 0; z < FT_VOX_RENDER_DIAMETER_Z; z++) {
        /* generate GPU-data of this 'chunk' */
        if (!ftChunkGenerateMesh(self->chunks[FT_VOX_WORLD_3D_1D(x, y, z)])) { return (0); }
    }}}

    /* success */
    return (1);
}


extern int ftWorldUpdate(struct World *self) {
    /* null-check */
    if (!self) { return (0); }
    
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

    /* iterate on every 'chunk' */
    for (int y = 0; y < FT_VOX_RENDER_DIAMETER_X; y++) {
    for (int x = 0; x < FT_VOX_RENDER_DIAMETER_Y; x++) {
    for (int z = 0; z < FT_VOX_RENDER_DIAMETER_Z; z++) {
        struct
        Chunk *chunk = self->chunks[FT_VOX_WORLD_3D_1D(x, y, z)];

        /* skip 'chunk' if needed */
        if (chunk->flags.empty ||
            chunk->flags.dirty
        ) { continue; }
        
        /* 'u_model' matrix */
        mat4 u_model = alg_mat4translate(chunk->position);
        ftShaderUniformSet(pShader, "u_model", FT_UNIFORM_MATRIX4X4, &u_model);
        
        /* 'mesh' reference */
        struct
        BlockMesh *mesh = chunk->mesh;
       
        /* set 'vbo' */
        ftVertexArrayVertexBuffer(pVao, mesh->vbo.id);

        /* render 'solid' */
        if (1) {
            if (mesh->ibo[0].sz > 0) {
                /* set 'ibo[0]' */
                ftVertexArrayIndexBuffer(pVao, mesh->ibo[0].id);

                /* draw-call */
                glDrawElements(GL_TRIANGLES, mesh->ibo[0].sz, GL_UNSIGNED_INT, (GLvoid *) (0));
            }
        }

        /* render 'transparent' */
        if (0) {
            if (mesh->ibo[1].sz > 0) {
                /* set 'ibo[1]' */
                ftVertexArrayIndexBuffer(pVao, mesh->ibo[1].id);

                /* draw-call */
                glDrawElements(GL_TRIANGLES, mesh->ibo[1].sz, GL_UNSIGNED_INT, (GLvoid *) (0));
            }
        }
    }}}

    /* success */
    return (1);
}


extern int ftWorldSetCenter(struct World *self, const vec3 center) {
    /* null-check */
    if (!self) { return (0); }

    (void) center;

    /* success */
    return (1);
}


extern int ftWorldGetChunk(struct World *self, struct Chunk **pChunk, int cx, int cy, int cz) {
    /* null-check */
    if (!self) { return (0); }

    struct
    Chunk *chunk = 0;
    if ((unsigned) cx >= FT_VOX_RENDER_DIAMETER_X ||
        (unsigned) cy >= FT_VOX_RENDER_DIAMETER_Y ||
        (unsigned) cz >= FT_VOX_RENDER_DIAMETER_Z
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
