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


static int __ftChunkFaceCube(GLvertex *pVertices, GLsizei *pVsize, GLuint *pIndices, GLsizei *pIsize, int face, vec3 offset) {
    /* 'pVertices' : send face */
    GLsizei base = *pVsize;
    for (size_t j = 0; j < 4; j++) {
        GLvertex vertex = { 0 };
        vertex.position = FT_VOX_CUBE_POSITION[FT_VOX_CUBE_INDICES[face * 6 + FT_VOX_FACE_UNIQUE[j]]];
        vertex.normal   = FT_VOX_FACE_DIRECTION[face];
        vertex.texcoord = FT_VOX_CUBE_TEXCOORD[j];
        vertex.position.x += offset.x;
        vertex.position.y += offset.y;
        vertex.position.z += offset.z;
        pVertices[(*pVsize)++] = vertex;
    }

    /* 'pIndices' : send face */
    for (size_t j = 0; j < 6; j++) {
        pIndices[(*pIsize)++] = base + FT_VOX_FACE_INDICES[j];
    }

    /* success */
    return (1);
}

/* public API */

extern int ftChunkGenCPU(Chunk *pChunk, int seed) {
    /* null-check */
    if (!pChunk) { return (0); }
   
    /* fill 'data' with some procedural ID's */
    uint8_t dirty = 0;
    uint8_t empty = 1;
    for (int y = 0; y < FT_VOX_CHUNK_BLOCKS_Y; y++) {
    for (int x = 0; x < FT_VOX_CHUNK_BLOCKS_X; x++) {
    for (int z = 0; z < FT_VOX_CHUNK_BLOCKS_Z; z++) {
        double noise = perlin3d((pChunk->position.x + x) * 0.1,
                                (pChunk->position.y + y) * 0.1,
                                (pChunk->position.z + z) * 0.1,
                                 seed);

        /* > 'noise' <= '0.0' : air
         * > 'noise' >  '0.0' : no-air
         * */
        if (noise > 0.0) {
            pChunk->data[FT_VOX_CHUNK_3D_1D(x, y, z)] = 1;
            dirty = 1;
            empty = 0;
        }
    }}}

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

    /* 'v' : vertex buffer
     * 's' : solid index buffer
     * 't' : transparent index buffer
     * */
    GLvertex *v = (GLvertex *) malloc(FT_VOX_CHUNK_BLOCKS_S * 24 * sizeof(GLvertex));
    GLuint *s = (GLuint *) malloc(FT_VOX_CHUNK_BLOCKS_S * 36 * sizeof(GLuint)),
           *t = (GLuint *) malloc(FT_VOX_CHUNK_BLOCKS_S * 36 * sizeof(GLuint));

    /* 'nv' : 'v' buffer size/index
     * 'ns' : 's' buffer size/index
     * 'nt' : 't' buffer size/index
     * */
    GLsizei nv = 0,
            ns = 0,
            nt = 0;
    
    for (int y = 0; y < FT_VOX_CHUNK_BLOCKS_Y; y++) {
    for (int x = 0; x < FT_VOX_CHUNK_BLOCKS_X; x++) {
    for (int z = 0; z < FT_VOX_CHUNK_BLOCKS_Z; z++) {
        /* get the 'current' block */
        uint32_t current = 0;
        ftChunkGetBlock(pChunk, alg_vec3(x, y, z), &current);

        /* check if 'current' is 'air' */
        if (!current) { continue; }

        /* check if 'current' is 'solid' */
        else if (current == 1) {
            for (size_t i = 0; i < 6; i++) {
                /* get the 'neighbor' block */
                uint32_t neighbor = 0;
                ftChunkGetBlock(pChunk, alg_vec3(x + FT_VOX_FACE_DIRECTION[i].x,
                                                 y + FT_VOX_FACE_DIRECTION[i].y,
                                                 z + FT_VOX_FACE_DIRECTION[i].z), &neighbor);
                
                /*check if 'neighbor' is 'solid' */
                if (neighbor == 1) { continue; }
                else {
                    __ftChunkFaceCube(v, &nv, s, &ns, i, alg_vec3(x, y, z));
                }
            }
        }
    }}}

    /* set 'mesh' buffers */
    BlockMesh *mesh = pChunk->mesh;

    mesh->vbo.sz = nv;
    glNamedBufferData(mesh->vbo.id, nv * sizeof(GLvertex), v, GL_STATIC_DRAW);

    mesh->ibo[0].sz = ns;
    glNamedBufferData(mesh->ibo[0].id, ns * sizeof(GLuint), s, GL_STATIC_DRAW);

    mesh->ibo[1].sz = nt;
    glNamedBufferData(mesh->ibo[1].id, nt * sizeof(GLuint), t, GL_STATIC_DRAW);

    /* release 'v', 's' and 't' buffers */
    free(v);
    free(s);
    free(t);

    /* set 'dirty' flag */
    pChunk->flags.dirty = 0;

    /* return 'status' */
    return (1);
}


extern int ftChunkGetBlock(Chunk *pChunk, const vec3 position, uint32_t *pID) {
    /* null-check */
    if (!pChunk) { return (0); }

    int cx = pChunk->cx,
        cy = pChunk->cy,
        cz = pChunk->cz;
    int x = (int) position.x,
        y = (int) position.y,
        z = (int) position.z;

    if      (x < 0)                      { cx--; x += FT_VOX_CHUNK_BLOCKS_X; }
    else if (x >= FT_VOX_CHUNK_BLOCKS_X) { cx++; x -= FT_VOX_CHUNK_BLOCKS_X; }
    if      (y < 0)                      { cy--; y += FT_VOX_CHUNK_BLOCKS_Y; }
    else if (y >= FT_VOX_CHUNK_BLOCKS_Y) { cy++; y -= FT_VOX_CHUNK_BLOCKS_Y; }
    if      (z < 0)                      { cz--; z += FT_VOX_CHUNK_BLOCKS_Z; }
    else if (z >= FT_VOX_CHUNK_BLOCKS_Z) { cz++; z -= FT_VOX_CHUNK_BLOCKS_Z; }
   
    /* get actual 'pChunk' (bound-check) */
    if (cx != pChunk->cx ||
        cy != pChunk->cy ||
        cz != pChunk->cz
    ) {
        ftWorldGetChunk(pChunk->world, &pChunk, cx, cy, cz);
        if (!pChunk) {
            return (0);
        }
    }

    /* read 'ID' from 'data' */
    uint32_t *data = pChunk->data;
    if (!data) { return (0); }
    
    /* return 'ID' */
    uint32_t ID = pChunk->data[FT_VOX_CHUNK_3D_1D(x, y, z)];
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
    x = alg_clamp(x, 0, FT_VOX_CHUNK_BLOCKS_X - 1);
    y = alg_clamp(y, 0, FT_VOX_CHUNK_BLOCKS_Y - 1);
    z = alg_clamp(z, 0, FT_VOX_CHUNK_BLOCKS_Z - 1);
    pChunk->data[FT_VOX_CHUNK_3D_1D(x, y, z)] = ID;

/* success */
    return (1);
}
