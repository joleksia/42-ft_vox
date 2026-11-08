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
                                                 0, 4, 7, 0, 7, 3, /* west  (-x) */
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
    uint32_t *data = (uint32_t *) calloc(FT_VOX_CHUNK_S, sizeof(uint32_t));
    if (!data) {
        ftMeshDelete(&pChunk->mesh);
        return (0);
    }

    /* set 'pChunk's 'data' */
    pChunk->data = data;

    /* fill 'data' with some procedural ID's */
    for (size_t y = 0; y < FT_VOX_CHUNK_H; y++) {
        for (size_t x = 0; x < FT_VOX_CHUNK_W; x++) {
            for (size_t z = 0; z < FT_VOX_CHUNK_D; z++) {
                double noise = perlin3D(x * 0.05,
                                        y * 0.05,
                                        z * 0.05);
                if (noise > 0.1) {
                    ftChunkSetBlock(pChunk, alg_vec3(x, y, z), 1);
                }
            }
        }
    }

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
    GLvertex *vertices = (GLvertex *) malloc(8 * sizeof(GLvertex) * FT_VOX_CHUNK_S);
    size_t v_cnt = 0;
    GLuint *indices = (GLuint *) malloc(36 * sizeof(GLuint) * FT_VOX_CHUNK_S);
    size_t i_cnt = 0;
    
    /* CPU-to-GPU */
    for (int y = 0; y < FT_VOX_CHUNK_H; y++) {
        for (int x = 0; x < FT_VOX_CHUNK_W; x++) {
            for (int z = 0; z < FT_VOX_CHUNK_D; z++) {
                /* get the 'block' */
                uint32_t block = 0;
                ftChunkGetBlock(pChunk, alg_vec3(x, y, z), &block);
                /* skip if 'air' ('block' == '0') */
                if (!block) { continue; }

                /* get the neightboring 'blocks' */
                uint32_t blocks[6] = { 0 };
                ftChunkGetBlock(pChunk, alg_vec3(x, y, z - 1), &blocks[0]); /* north (-z) */
                ftChunkGetBlock(pChunk, alg_vec3(x, y, z + 1), &blocks[1]); /* south (+z) */
                ftChunkGetBlock(pChunk, alg_vec3(x - 1, y, z), &blocks[2]); /* west  (-z) */
                ftChunkGetBlock(pChunk, alg_vec3(x + 1, y, z), &blocks[3]); /* east  (+x) */
                ftChunkGetBlock(pChunk, alg_vec3(x, y - 1, z), &blocks[4]); /* down  (-y) */
                ftChunkGetBlock(pChunk, alg_vec3(x, y + 1, z), &blocks[5]); /* top   (+y) */

                /* check each neighboring block */
                for (size_t i = 0; i < 6; i++) {
                    uint32_t block = blocks[i];
                    /* check if neighbor is "transparent" (for now only 'air') */
                    if (!block) {
                        /* copy 'tmp' to 'indices' */
                        const GLuint *tmp = &FT_VOX_CUBE_INDEX_DATA[6 * i];
                        for (size_t j = 0; j < 6; j++) {
                            indices[i_cnt++] = tmp[j] + v_cnt;
                        }
                    }
                }

                /* copy vertex to 'vertices' */
                for (size_t i = 0; i < 8; i++) {
                    GLvertex tmp = FT_VOX_VERTEX_DATA[i];
                    tmp.position.x += x;
                    tmp.position.y += y;
                    tmp.position.z += z;
                    vertices[v_cnt++] = tmp;
                }
            }
        }
    }

    /* upload 'vertices' and 'indices' to GPU */
    int status = ftMeshSetData(&pChunk->mesh, vertices, v_cnt * sizeof(GLvertex),
                                              indices,  i_cnt * sizeof(GLuint));

    /* exec. only is 'status' is "no-error" */
    if (status) {
        /* upload GPU mesh to 'vao' */
        if (!ftMeshUpload(&pChunk->mesh, vao)) { return (0); }

        /* toggle 'dirty' flag */
        pChunk->dirty = 0;
    }

    /* release 'vertices' and 'indices' */
    free(vertices);
    free(indices);

    /* return status */
    return (status);
}


extern int ftChunkGetBlock(Chunk *pChunk, const vec3 position, uint32_t *pID) {
    /* null-check */
    if (!pChunk)       { return (0); }
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
    if (x < 0 || x >= FT_VOX_CHUNK_W ||
        y < 0 || y >= FT_VOX_CHUNK_H ||
        z < 0 || z >= FT_VOX_CHUNK_D
    ) {
        x %= FT_VOX_CHUNK_W;
        y %= FT_VOX_CHUNK_H;
        z %= FT_VOX_CHUNK_D;
        return (0);
    }
    
    /* get the block's 'ID' */
    uint32_t ID = pChunk->data[x + y * FT_VOX_CHUNK_W + z * FT_VOX_CHUNK_W * FT_VOX_CHUNK_H];

    /* return 'ID' */
    if (pID) { *pID = ID; }

    /* success */
    return (1);
}


extern int ftChunkSetBlock(Chunk *pChunk, const vec3 position, const uint32_t ID) {
    /* null-check */
    if (!pChunk)       { return (0); }
    if (!pChunk->data) { return (0); }

    /* try to set 'ID' into CPU 'data' */
    ssize_t x = (ssize_t) position.x,
            y = (ssize_t) position.y,
            z = (ssize_t) position.z;
    /* NOTE:
     * In the final game the case of clamping 'x', 'y' and 'z' probably shouldn't happen.
     * But I'll keep it for now.
     * */
    x = alg_clamp(x, 0, FT_VOX_CHUNK_W - 1);
    y = alg_clamp(y, 0, FT_VOX_CHUNK_H - 1);
    z = alg_clamp(z, 0, FT_VOX_CHUNK_D - 1);
    pChunk->data[x + y * FT_VOX_CHUNK_W + z * FT_VOX_CHUNK_W * FT_VOX_CHUNK_H] = ID;

    /* success */
    return (1);
}
