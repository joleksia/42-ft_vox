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

    /* create 'cpu_data' */
    uint32_t *cpu_data = (uint32_t *) calloc(FT_VOX_CHUNK_BLOCKS_S, sizeof(uint32_t));
    if (!cpu_data) { return (0); }

    /* create 'gpu_data' */
    GPUMesh gpu_data = { 0 };
    if (!ftGPUMeshCreate(&gpu_data)) {
        free(cpu_data); return (0);
    }

    /* return results */
    pChunk->cpu = cpu_data;
    pChunk->gpu = gpu_data;

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
    free(pChunk->cpu);

    /* release GPU resources */
    ftGPUMeshDelete(&pChunk->gpu);

    /* success */
    return (1);
}


extern int ftChunkRender(Chunk *pChunk) {
    /* null-check */
    if (!pChunk) { return (0); }

    /* issue a draw-call */
    glDrawElements(GL_TRIANGLES, pChunk->gpu.ibo.size / sizeof(GLuint), GL_UNSIGNED_INT, 0);

    /* success */
    return (1);
}


extern int ftChunkGenCPU(Chunk *pChunk, const vec3 position, const int seed) {
    /* null-check */
    if (!pChunk)      { return (0); }
    if (!pChunk->cpu) { return (0); }
   
    (void) seed;
    /* fill 'data' with some procedural ID's */
    uint8_t dirty = 0;
    uint8_t empty = 1;
    for (int y = 0; y < FT_VOX_CHUNK_BLOCKS_H; y++) {
        for (int x = 0; x < FT_VOX_CHUNK_BLOCKS_W; x++) {
            for (int z = 0; z < FT_VOX_CHUNK_BLOCKS_D; z++) {
                double perlin = perlin3d((position.x + x) * 0.01,
                                         (position.y + y) * 0.01,
                                         (position.z + z) * 0.01,
                                         seed);

                double fbm = fbm3d((position.x + x) * 0.1,
                                   (position.y + y) * 0.1,
                                   (position.z + z) * 0.1,
                                    0.5, 2.0, 8, seed);

                (void) perlin;
                (void) fbm;
                if (fbm > 0.1) {
                    ftChunkSetBlock(pChunk, alg_vec3(x, y, z), 1);
                    dirty = 1;
                    empty = 0;
                }
            }
        }
    }

    /* set 'pChunk's 'position' */
    pChunk->position = position;
    
    /* set 'flags' */
    pChunk->flags.dirty = dirty;
    pChunk->flags.empty = empty;

    /* success */
    return (1);
}


extern int ftChunkGenGPU(Chunk *pChunk){
    /* null-check */
    if (!pChunk)      { return (0); }
    if (!pChunk->cpu) { return (0); }
    
    /* create 'vertices' and 'indices' */
    GLvertex *vertices = (GLvertex *) malloc(8 * sizeof(GLvertex) * FT_VOX_CHUNK_BLOCKS_S);
    GLuint *indices = (GLuint *) malloc(36 * sizeof(GLuint) * FT_VOX_CHUNK_BLOCKS_S);
    size_t v_cnt = 0;
    size_t i_cnt = 0;
    
    /* CPU-to-GPU */
    for (int y = 0; y < FT_VOX_CHUNK_BLOCKS_H; y++) {
        for (int x = 0; x < FT_VOX_CHUNK_BLOCKS_W; x++) {
            for (int z = 0; z < FT_VOX_CHUNK_BLOCKS_D; z++) {
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
                    tmp.position.x += x + pChunk->position.x;
                    tmp.position.y += y + pChunk->position.y;
                    tmp.position.z += z + pChunk->position.z;
                    vertices[v_cnt++] = tmp;
                }
            }
        }
    }
    
    /* upload 'vertices' and 'indices' to GPU */
    int status = ftGPUMeshSetData(&pChunk->gpu, vertices, v_cnt * sizeof(GLvertex),
                                             indices,  i_cnt * sizeof(GLuint));

    /* release 'vertices' and 'indices' */
    free(vertices);
    free(indices);

    /* return 'status' */
    return (status);
}


extern int ftChunkUpload(Chunk *pChunk, const GLuint vao) {
    /* null-check */
    if (!pChunk) { return (0); }
    
    /* upload GPU mesh to 'vao' */
    if (!ftGPUMeshUpload(&pChunk->gpu, vao)) { return (0); }

    /* set 'dirty' flag */
    pChunk->flags.dirty = 0;

    /* return status */
    return (1);
}


extern int ftChunkGetBlock(Chunk *pChunk, const vec3 position, uint32_t *pID) {
    /* null-check */
    if (!pChunk)      { return (0); }
    if (!pChunk->cpu) { return (0); }

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
    uint32_t ID = pChunk->cpu[x + y * FT_VOX_CHUNK_BLOCKS_W + z * FT_VOX_CHUNK_BLOCKS_W * FT_VOX_CHUNK_BLOCKS_H];

    /* return 'ID' */
    if (pID) { *pID = ID; }

    /* success */
    return (1);
}


extern int ftChunkSetBlock(Chunk *pChunk, const vec3 position, const uint32_t ID) {
    /* null-check */
    if (!pChunk)      { return (0); }
    if (!pChunk->cpu) { return (0); }

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
    pChunk->cpu[x + y * FT_VOX_CHUNK_BLOCKS_W + z * FT_VOX_CHUNK_BLOCKS_W * FT_VOX_CHUNK_BLOCKS_H] = ID;

    /* success */
    return (1);
}
