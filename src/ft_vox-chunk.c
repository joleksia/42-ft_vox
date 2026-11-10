#include "ft_vox.h"

/* static API */

static const vec3 FT_VOX_FACE_DIRECTION[] = { {  0.0,  0.0, -1.0, }, /* north (-z) */
                                              {  0.0,  0.0,  1.0, }, /* south (+z) */
                                              { -1.0,  0.0,  0.0, }, /* west  (-x) */
                                              {  1.0,  0.0,  0.0, }, /* east  (+x) */
                                              {  0.0, -1.0,  0.0, }, /* down  (-y) */
                                              {  0.0,  1.0,  0.0, }, /* top   (+y) */ };

static const GLuint FT_VOX_FACE_UNIQUE[] = { 1, 0, 5, 2 };
static const GLuint FT_VOX_FACE_INDICES[] = { 1, 0, 3, 1, 3, 2, };
static const GLuint FT_VOX_CUBE_INDICES[] = { 1, 0, 3, 1, 3, 2, /* north (-z) */
                                              4, 5, 6, 4, 6, 7, /* south (+z) */
                                              0, 4, 7, 0, 7, 3, /* west  (-x) */
                                              5, 1, 2, 5, 2, 6, /* east  (+x) */
                                              5, 4, 0, 5, 0, 1, /* down  (-y) */
                                              2, 3, 7, 2, 7, 6, /* top   (+y) */ };

static const vec3 FT_VOX_CUBE_POSITION[] = { { 0, 0, 0, },
                                             { 1, 0, 0, },
                                             { 1, 1, 0, },
                                             { 0, 1, 0, },
                                             { 0, 0, 1, },
                                             { 1, 0, 1, },
                                             { 1, 1, 1, },
                                             { 0, 1, 1, }, };

static const vec2 FT_VOX_CUBE_TEXCOORD[] = { { 1, 0, },
                                             { 0, 0, },
                                             { 0, 1, },
                                             { 1, 1, }, };


static int __ftChunkFaceCube(GLvertex *pVertices, GLsizei *pVsize, GLuint *pIndices, GLsizei *pIsize, int diridx, vec3 position, vec3 offset) {
    /* null-check */
    if (!pVertices) { return (0); }
    if (!pIndices)  { return (0); }
    if (!pVsize) { return (0); }
    if (!pIsize) { return (0); }

    /* 'pVertices' : send face */
    GLsizei base = *pVsize;
    for (size_t j = 0; j < 4; j++) {
        const vec3 vertex = FT_VOX_CUBE_POSITION[FT_VOX_CUBE_INDICES[diridx * 6 + FT_VOX_FACE_UNIQUE[j]]];
        pVertices[(*pVsize)++] = (GLvertex) {
            .position = { .x = vertex.x + position.x + offset.x,
                          .y = vertex.y + position.y + offset.y,
                          .z = vertex.z + position.z + offset.z, },

            .normal = { 0 },            

            .texcoord = FT_VOX_CUBE_TEXCOORD[j],
        };
    }

    /* 'pIndices' : send face */
    for (size_t j = 0; j < 6; j++) {
        pIndices[(*pIsize)++] = base + FT_VOX_FACE_INDICES[j];
    }

    /* success */
    return (1);
}

/* public API */

extern int ftChunkCreate(Chunk *pChunk, World *pWorld, const vec3 offset, const int seed) {
    /* null-check */
    if (!pChunk) { return (0); }

    /* reference 'world' */
    pChunk->world = pWorld;

    /* create 'data' */
    uint32_t *data = (uint32_t *) calloc(FT_VOX_CHUNK_BLOCKS_S, sizeof(uint32_t));
    if (data) {
        pChunk->data = data;
    }

    /* create 'mesh' */
    BlockMesh *mesh = (BlockMesh *) calloc(1, sizeof(BlockMesh));
    if (mesh) {
        glCreateBuffers(1, &mesh->vbo.id);
        glCreateBuffers(1, &mesh->ibo[0].id);
        glCreateBuffers(1, &mesh->ibo[1].id);
        pChunk->mesh = mesh;
    }

    /* set 'offset' */
    pChunk->offset = offset;

    /* set 'seed' */
    pChunk->seed = seed;

    /* set 'flags' */
    pChunk->flags.dirty = 0;
    pChunk->flags.empty = 0;

    /* success */
    return (1);
}


extern int ftChunkDelete(Chunk *pChunk) {
    /* null-check */
    if (!pChunk) { return (0); }

    /* release CPU resources */
    uint32_t *data = pChunk->data;
    if (data) {
        free(data);
    }

    /* release GPU resources */
    BlockMesh *mesh = pChunk->mesh;
    if (mesh) {
        glDeleteBuffers(1, &mesh->vbo.id);
        glDeleteBuffers(1, &mesh->ibo[0].id);
        glDeleteBuffers(1, &mesh->ibo[1].id);
        free(mesh);
    }

    /* success */
    return (1);
}


extern int ftChunkGenCPU(Chunk *pChunk) {
    /* null-check */
    if (!pChunk) { return (0); }
   
    /* fill 'data' with some procedural ID's */
    uint8_t dirty = 0;
    uint8_t empty = 1;
    for (int y = 0; y < FT_VOX_CHUNK_BLOCKS_H; y++) {
        for (int x = 0; x < FT_VOX_CHUNK_BLOCKS_W; x++) {
            for (int z = 0; z < FT_VOX_CHUNK_BLOCKS_D; z++) {
                double noise = fbm3d((pChunk->offset.x + x) * 0.1,
                                     (pChunk->offset.y + y) * 0.1,
                                     (pChunk->offset.z + z) * 0.1,
                                      0.5, 2.0, 8,
                                      pChunk->seed);

                /* > 'noise' <= '0.0' : air
                 * > 'noise' >  '0.0' : no-air
                 * */
                if (noise > 0.0) {
                    pChunk->data[x +
                                 y * FT_VOX_CHUNK_BLOCKS_W +
                                 z * FT_VOX_CHUNK_BLOCKS_W * FT_VOX_CHUNK_BLOCKS_H] = 1;
                    dirty = 1;
                    empty = 0;
                }
            }
        }
    }

    /* set 'flags' */
    pChunk->flags.dirty = dirty;
    pChunk->flags.empty = empty;

    /* success */
    return (1);
}


extern int ftChunkGenGPU(Chunk *pChunk){
    /* null-check */
    if (!pChunk) { return (0); }
    if (!pChunk->data) { return (0); }
    if (!pChunk->mesh) { return (0); }

    /* 'dirty'-check */
    if (!pChunk->flags.dirty) { return (0); }
    
    /* CPU-to-GPU */
    GLvertex *vertex = (GLvertex *) calloc(FT_VOX_CHUNK_BLOCKS_S * 24, sizeof(GLvertex));
    GLuint *solid = (GLuint *) calloc(FT_VOX_CHUNK_BLOCKS_S * 36, sizeof(GLuint)),
           *trans = (GLuint *) calloc(FT_VOX_CHUNK_BLOCKS_S * 36, sizeof(GLuint));
    GLsizei nv = 0, /* 'nv' : 'vertex' buffer size/index */
            ns = 0, /* 'ns' : 'solid' buffer size/index  */
            nt = 0; /* 'nt' : 'trans' buffer size/index  */
    
    for (int y = 0; y < FT_VOX_CHUNK_BLOCKS_H; y++) {
    for (int x = 0; x < FT_VOX_CHUNK_BLOCKS_W; x++) {
    for (int z = 0; z < FT_VOX_CHUNK_BLOCKS_D; z++) {
        /* get the 'current' block */
        uint32_t current = 0;
        ftChunkGetBlock(pChunk, alg_vec3(x, y, z), &current);

        /* check if 'current' is 'air' */
        if (!current) { continue; }

        /* check if 'current' is 'solid' */
        else if (current == 1) {
            for (size_t i = 0; i < 6; i++) {
                /* get the 'neighbor' block */
                vec3 position = alg_vec3(x + FT_VOX_FACE_DIRECTION[i].x,
                                         y + FT_VOX_FACE_DIRECTION[i].y,
                                         z + FT_VOX_FACE_DIRECTION[i].z);
                uint32_t neighbor = 0;
                ftChunkGetBlock(pChunk, position, &neighbor);

                /*check if 'neighbor' is 'solid' */
                if (neighbor == 1) { continue; }
                else {
                    __ftChunkFaceCube(vertex, &nv, solid, &ns, i, alg_vec3(x, y, z), pChunk->offset);
                }
            }
        }
    }
    }
    }

    /* set 'mesh' buffers */
    BlockMesh *mesh = pChunk->mesh;

    mesh->vbo.sz = nv;
    glNamedBufferData(mesh->vbo.id, nv * sizeof(GLvertex), vertex, GL_STATIC_DRAW);

    mesh->ibo[0].sz = ns;
    glNamedBufferData(mesh->ibo[0].id, ns * sizeof(GLuint), solid, GL_STATIC_DRAW);

    mesh->ibo[1].sz = nt;
    glNamedBufferData(mesh->ibo[1].id, nt * sizeof(GLuint), trans, GL_STATIC_DRAW);

    /* release 'vertex', 'solid' and 'trans' buffers */
    free(vertex); free(solid); free(trans);

    /* set 'dirty' flag */
    pChunk->flags.dirty = 0;

    /* return 'status' */
    return (1);
}


extern int ftChunkGetBlock(Chunk *pChunk, const vec3 position, uint32_t *pID) {
    /* null-check */
    if (!pChunk)      { return (0); }
    if (!pChunk->data) { return (0); }

    /* try to get 'ID' from CPU 'data' */
    ssize_t x = (ssize_t) position.x,
            y = (ssize_t) position.y,
            z = (ssize_t) position.z;

    /* TODO:
     * When we get more chunks to render, we should probably ask for neighbor block
     * from the neighboring chunk.
     * Position of neighbor block from neighbor chunk can be obtained by using '%'
     * on 'x', 'y', 'z' positions.
     * */
    if (x < 0 || x >= FT_VOX_CHUNK_BLOCKS_W ||
        y < 0 || y >= FT_VOX_CHUNK_BLOCKS_H ||
        z < 0 || z >= FT_VOX_CHUNK_BLOCKS_D
    ) {
        x %= FT_VOX_CHUNK_BLOCKS_W;
        y %= FT_VOX_CHUNK_BLOCKS_H;
        z %= FT_VOX_CHUNK_BLOCKS_D;
        return (0);
    }
    
    /* get the block's 'ID' */
    uint32_t ID = pChunk->data[x +
                               y * FT_VOX_CHUNK_BLOCKS_W +
                               z * FT_VOX_CHUNK_BLOCKS_W * FT_VOX_CHUNK_BLOCKS_H];

    /* return 'ID' */
    if (pID) { *pID = ID; }

    /* success */
    return (1);
}


extern int ftChunkSetBlock(Chunk *pChunk, const vec3 position, const uint32_t ID) {
    /* null-check */
    if (!pChunk)      { return (0); }
    if (!pChunk->data) { return (0); }

    /* try to set 'ID' into CPU 'data' */
    ssize_t x = (ssize_t) position.x,
            y = (ssize_t) position.y,
            z = (ssize_t) position.z;
    /* NOTE:
     * In the final game the case of clamping 'x', 'y' and 'z' probably shouldn't happen.
     * But I'll keep it for now.
     * */
    x = alg_clamp(x, 0, FT_VOX_CHUNK_BLOCKS_W - 1);
    y = alg_clamp(y, 0, FT_VOX_CHUNK_BLOCKS_H - 1);
    z = alg_clamp(z, 0, FT_VOX_CHUNK_BLOCKS_D - 1);
    pChunk->data[x +
                 y * FT_VOX_CHUNK_BLOCKS_W +
                 z * FT_VOX_CHUNK_BLOCKS_W * FT_VOX_CHUNK_BLOCKS_H] = ID;

    /* success */
    return (1);
}
