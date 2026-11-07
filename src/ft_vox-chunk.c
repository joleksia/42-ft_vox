#include "ft_vox.h"

/* 'FT_VOX_VERTEX_DATA' : standard cube vertex layout */
static const GLvertex FT_VOX_VERTEX_DATA[] = { [0] = { .position = { 0.0, 0.0, 0.0 }, .normal = { 0.0, 0.0, 0.0 }, .texcoord = { 0.0, 0.0 } },
                                               [1] = { .position = { 1.0, 0.0, 0.0 }, .normal = { 0.0, 0.0, 0.0 }, .texcoord = { 1.0, 0.0 } },
                                               [2] = { .position = { 1.0, 1.0, 0.0 }, .normal = { 0.0, 0.0, 0.0 }, .texcoord = { 1.0, 1.0 } },
                                               [3] = { .position = { 0.0, 1.0, 0.0 }, .normal = { 0.0, 0.0, 0.0 }, .texcoord = { 0.0, 1.0 } },
                                               [4] = { .position = { 0.0, 0.0, 1.0 }, .normal = { 0.0, 0.0, 0.0 }, .texcoord = { 0.0, 0.0 } },
                                               [5] = { .position = { 1.0, 0.0, 1.0 }, .normal = { 0.0, 0.0, 0.0 }, .texcoord = { 1.0, 0.0 } },
                                               [6] = { .position = { 1.0, 1.0, 1.0 }, .normal = { 0.0, 0.0, 0.0 }, .texcoord = { 1.0, 1.0 } },
                                               [7] = { .position = { 0.0, 1.0, 1.0 }, .normal = { 0.0, 0.0, 0.0 }, .texcoord = { 0.0, 1.0 } }, };


/* 'FT_VOX_CUBE_INDEX_DATA' : index layout for building standard cube mesh */
static const GLuint FT_VOX_CUBE_INDEX_DATA[] = { 1, 0, 3, 1, 3, 2, /* north (-z) */
                                                 4, 5, 6, 4, 6, 7, /* south (+z) */
                                                 0, 4, 7, 0, 7, 3, /* west  (-z) */
                                                 5, 1, 2, 5, 2, 6, /* east  (+x) */
                                                 5, 4, 0, 5, 0, 1, /* down  (-y) */
                                                 2, 3, 7, 2, 7, 6, /* top   (+y) */ };


/* 'FT_VOX_SPRITE_INDEX_DATA' : index layout for building X-shaped sprite mesh */
/*
static const GLuint FT_VOX_SPRITE_INDEX_DATA[] = { 3, 5, 0, 3, 6, 5,
                                                   2, 1, 4, 2, 4, 7,
                                                   3, 0, 5, 3, 5, 6,
                                                   2, 4, 1, 2, 7, 4, }; */


extern int ftChunkCreate(Chunk *pChunk) {
    /* null-check */
    if (!pChunk) { return (0); }

    /* create GPU mesh */
    if (!ftMeshCreate(&pChunk->mesh)) { return (0); }

    /* create CPU index 'data' array */
    uint64_t *data = (uint64_t *) calloc(FT_VOX_CHUNK_S, sizeof(uint64_t));
    if (!data) {
        ftMeshDelete(&pChunk->mesh);
        return (0);
    }

    /* fill 'data' with some procedural ID's */
    for (size_t y = 0; y < FT_VOX_CHUNK_H; y++) {
        for (size_t x = 0; x < FT_VOX_CHUNK_W; x++) {
            for (size_t z = 0; z < FT_VOX_CHUNK_D; z++) {
                double noise = perlin3D(x * 0.001, y * 0.001, z * 0.001);
                if (noise > 0.0) { ftChunkSetBlock(pChunk, alg_vec3(x, y, z), 1); }
            }
        }
    }

    /* set 'pChunk's 'data' */
    pChunk->data = data;

    /* set 'dirty' flag */
    pChunk->dirty = 1;

    /* success */
    return (1);
}


extern int ftChunkDelete(Chunk *pChunk) {
    /* null-check */
    if (!pChunk) { return (0); }

    /* release CPU resources */
    if (pChunk->data) { free(pChunk->data); }

    /* release GPU resources */
    ftMeshDelete(&pChunk->mesh);

    /* success */
    return (1);
}


extern int ftChunkUpload(Chunk *pChunk, const GLuint vao) {
    /* null-check */
    if (!pChunk) { return (0); }

    /* dirty-check (if not set, return) */
    if (!pChunk->dirty) { return (1); }
    
    /* create temporary buffers */
    GLvertex *tmp0 = (GLvertex *) malloc(8 * sizeof(GLvertex) * FT_VOX_CHUNK_S);
    if (!tmp0) { return (0); }

    GLuint *tmp1 = (GLuint *) malloc(36 * sizeof(GLuint) * FT_VOX_CHUNK_S);
    if (!tmp1) { free(tmp0); return (0); }

    /* CPU-to-GPU */
    size_t v_offset = 0,
           i_offset = 0;
    for (size_t y = 0; y < FT_VOX_CHUNK_H; y++) {
        for (size_t x = 0; x < FT_VOX_CHUNK_W; x++) {
            for (size_t z = 0; z < FT_VOX_CHUNK_D; z++) {

                /* vertex data */
                GLvertex vertices[8];
                for (size_t i = 0; i < 8; i++) {
                    vertices[i] = FT_VOX_VERTEX_DATA[i];
                    vertices[i].position.x += x;
                    vertices[i].position.y += y;
                    vertices[i].position.z += z;
                }

                /* concat. 'vertices' to 'tmp0' */
                memmove(&tmp0[v_offset], vertices, sizeof(vertices));

                /* cube index data */
                GLuint indices[36];
                for (size_t i = 0; i < 36; i++) {
                    indices[i] = FT_VOX_CUBE_INDEX_DATA[i] + v_offset;
                }

                /* concat. 'indices' to 'tmp1' */
                memmove(&tmp1[i_offset], indices, sizeof(indices));

                /* move offsets */
                v_offset += 8;
                i_offset += 36;
            }
        }
    }

    /* upload 'tmp0' and 'tmp1' to GPU */
    if (!ftMeshSetData(&pChunk->mesh, tmp0,  8 * sizeof(GLvertex) * FT_VOX_CHUNK_S,
                                      tmp1, 36 * sizeof(GLuint)   * FT_VOX_CHUNK_S)) {
        free(tmp0);
        free(tmp1);
        return (0);
    }

    /* release 'tmp0' and 'tmp1' */
    free(tmp0);
    free(tmp1);

    /* upload GPU mesh to 'vao' */
    if (!ftMeshUpload(&pChunk->mesh, vao)) { return (0); }

    /* toggle 'dirty' flag */
    pChunk->dirty = 0;

    /* success */
    return (1);
}


extern int ftChunkGetBlock(Chunk *pChunk, const vec3 position, uint64_t *pID) {
    /* null-check */
    if (!pChunk)       { return (0); }
    if (!pChunk->data) { return (0); }

    /* try to get 'ID' from CPU 'data' */
    size_t x = (size_t) position.x,
           y = (size_t) position.y,
           z = (size_t) position.z;
    x = alg_clamp(x, 0, FT_VOX_CHUNK_W - 1);
    y = alg_clamp(y, 0, FT_VOX_CHUNK_H - 1);
    z = alg_clamp(z, 0, FT_VOX_CHUNK_D - 1);
    uint64_t ID = pChunk->data[x + y * FT_VOX_CHUNK_W + z * FT_VOX_CHUNK_W * FT_VOX_CHUNK_H];

    /* return */
    if (pID) { *pID = ID; }

    /* success */
    return (1);
}


extern int ftChunkSetBlock(Chunk *pChunk, const vec3 position, const uint64_t ID) {
    /* null-check */
    if (!pChunk)       { return (0); }
    if (!pChunk->data) { return (0); }

    /* try to set 'ID' into CPU 'data' */
    size_t x = (size_t) position.x,
           y = (size_t) position.y,
           z = (size_t) position.z;
    x = alg_clamp(x, 0, FT_VOX_CHUNK_W - 1);
    y = alg_clamp(y, 0, FT_VOX_CHUNK_H - 1);
    z = alg_clamp(z, 0, FT_VOX_CHUNK_D - 1);
    pChunk->data[x + y * FT_VOX_CHUNK_W + z * FT_VOX_CHUNK_W * FT_VOX_CHUNK_H] = ID;

    /* success */
    return (1);
}
