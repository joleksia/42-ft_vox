#include "ft_vox.h"

extern int ftWorldCreate(World *pWorld, const int seed) {
    /* null-check */
    if (!pWorld) { return (0); }

    /* create 'chunks' array */
    Chunk **chunks = (Chunk **) malloc(FT_VOX_RENDER_DIAMETER_X *
                                       FT_VOX_RENDER_DIAMETER_Y *
                                       FT_VOX_RENDER_DIAMETER_Z * sizeof(Chunk *)); 

    /* set-up 'chunks' array */
    for (int y = 0; y < FT_VOX_RENDER_DIAMETER_X; y++) {
    for (int x = 0; x < FT_VOX_RENDER_DIAMETER_Y; x++) {
    for (int z = 0; z < FT_VOX_RENDER_DIAMETER_Z; z++) {
        /* alloc. 'chunk' */
        Chunk *chunk = (Chunk *) malloc(sizeof(Chunk));

        /* 'chunk' 'pWorld' reference */
        chunk->world = pWorld;

        /* 'chunk' data members */
        chunk->cx = x; chunk->cy = y; chunk->cz = z;
        chunk->position = alg_vec3((x - FT_VOX_RENDER_DISTANCE_X) * FT_VOX_CHUNK_BLOCKS_X,
                                   (y - FT_VOX_RENDER_DISTANCE_Y) * FT_VOX_CHUNK_BLOCKS_Y,
                                   (z - FT_VOX_RENDER_DISTANCE_Z) * FT_VOX_CHUNK_BLOCKS_Z);

        /* create 'data' */
        uint32_t *data = (uint32_t *) malloc(FT_VOX_CHUNK_BLOCKS_S * sizeof(uint32_t));
        if (data) {
            chunk->data = data;
        }

        /* create 'mesh' */
        BlockMesh *mesh = (BlockMesh *) malloc(sizeof(BlockMesh));
        if (mesh) {
            glCreateBuffers(1, &mesh->vbo.id);
            glCreateBuffers(1, &mesh->ibo[0].id);
            glCreateBuffers(1, &mesh->ibo[1].id);
            chunk->mesh = mesh;
        }

        /* gen. 'chunk' */
        ftChunkGenCPU(chunk, seed);

        /* store 'chunk' into 'chunks' */
        chunks[FT_VOX_WORLD_3D_1D(x, y, z)] = chunk;
    }}}

    /* return 'chunks' */
    pWorld->chunks = chunks;

    /* success */
    return (1);
}


extern int ftWorldDelete(World *pWorld) {
    /* null-check */
    if (!pWorld) { return (0); }
    
    /* iterate on every 'chunk' */
    for (int y = 0; y < FT_VOX_RENDER_DIAMETER_X; y++) {
    for (int x = 0; x < FT_VOX_RENDER_DIAMETER_Y; x++) {
    for (int z = 0; z < FT_VOX_RENDER_DIAMETER_Z; z++) {
        Chunk *chunk = pWorld->chunks[FT_VOX_WORLD_3D_1D(x, y, z)];
        
        /* release CPU resources */
        uint32_t *data = chunk->data;
        if (data) {
            free(data);
        }

        /* release GPU resources */
        BlockMesh *mesh = chunk->mesh;
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
    free(pWorld->chunks);

    /* success */
    return (1);
}


extern int ftWorldUpdate(World *pWorld) {
    /* null-check */
    if (!pWorld) { return (0); }
    
    /* iterate on every 'chunk' */
    for (int y = 0; y < FT_VOX_RENDER_DIAMETER_X; y++) {
    for (int x = 0; x < FT_VOX_RENDER_DIAMETER_Y; x++) {
    for (int z = 0; z < FT_VOX_RENDER_DIAMETER_Z; z++) {
        Chunk *chunk = pWorld->chunks[FT_VOX_WORLD_3D_1D(x, y, z)];

        /* ... */ (void) chunk;
    }}}

    /* success */
    return (1);
}

extern int ftWorldRender(World *pWorld, const GLuint vao, Shader *shader) {
    /* null-check */
    if (!pWorld) { return (0); }

    /* iterate on every 'chunk' */
    for (int y = 0; y < FT_VOX_RENDER_DIAMETER_X; y++) {
    for (int x = 0; x < FT_VOX_RENDER_DIAMETER_Y; x++) {
    for (int z = 0; z < FT_VOX_RENDER_DIAMETER_Z; z++) {
        Chunk *chunk = pWorld->chunks[FT_VOX_WORLD_3D_1D(x, y, z)];

        /* check 'empty' flag */
        if (chunk->flags.empty) { continue; }

        /* check 'dirty' flag */
        if (chunk->flags.dirty) { ftChunkGenGPU(chunk); }
        
        /* 'u_model' matrix */
        mat4 u_model = alg_mat4translate(chunk->position);
        ftShaderUniformSet(shader, "u_model", FT_UNIFORM_MATRIX4X4, &u_model);
        
        /* 'mesh' reference */
        BlockMesh *mesh = chunk->mesh;
        glVertexArrayVertexBuffer(vao, 0, mesh->vbo.id, 0, sizeof(GLvertex));

        /* render 'solid' */
        if (1) {
            if (mesh->ibo[0].sz > 0) {
                glVertexArrayElementBuffer(vao, mesh->ibo[0].id);
                glDrawElements(GL_TRIANGLES, mesh->ibo[0].sz, GL_UNSIGNED_INT, (GLvoid *) (0));
            }
        }

        /* render 'transparent' */
        if (0) {
            if (mesh->ibo[1].sz > 0) {
                glVertexArrayElementBuffer(vao, chunk->mesh->ibo[1].id);
                glDrawElements(GL_TRIANGLES, mesh->ibo[1].sz, GL_UNSIGNED_INT, (GLvoid *) (0));
            }
        }
    }}}

    /* success */
    return (1);
}


extern int ftWorldGetChunk(World *pWorld, Chunk **pChunk, int cx, int cy, int cz) {
    /* null-check */
    if (!pWorld) { return (0); }

    Chunk *chunk = 0;
    if ((unsigned) cx >= FT_VOX_RENDER_DIAMETER_X ||
        (unsigned) cy >= FT_VOX_RENDER_DIAMETER_Y ||
        (unsigned) cz >= FT_VOX_RENDER_DIAMETER_Z
    ) {
        if (pChunk) { *pChunk = chunk; }
        return (0);
    }

    /* return 'chunk' */
    chunk = pWorld->chunks[FT_VOX_WORLD_3D_1D(cx, cy, cz)];
    if (pChunk) { *pChunk = chunk; }

    /* success */
    return (1);
}
