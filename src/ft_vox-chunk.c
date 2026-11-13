#include "ft_vox.h"

/* static API */

static const int FT_VOX_FACE_DIRECTION[] = {  0,  0, -1, /* north (-z) */
                                              0,  0,  1, /* south (+z) */
                                             -1,  0,  0, /* west  (-x) */
                                              1,  0,  0, /* east  (+x) */
                                              0, -1,  0, /* down  (-y) */
                                              0,  1,  0, /* top   (+y) */ };

static const uint FT_VOX_FACE_UNIQUE[] = { 1, 0, 5, 2 };
static const uint FT_VOX_FACE_INDICES[] = { 1, 0, 3, 1, 3, 2, };
static const uint FT_VOX_CUBE_INDICES[] = { 1, 0, 3, 1, 3, 2, /* north (-z) */
                                            4, 5, 6, 4, 6, 7, /* south (+z) */
                                            0, 4, 7, 0, 7, 3, /* west  (-x) */
                                            5, 1, 2, 5, 2, 6, /* east  (+x) */
                                            5, 4, 0, 5, 0, 1, /* down  (-y) */
                                            2, 3, 7, 2, 7, 6, /* top   (+y) */ };

static const uint FT_VOX_CUBE_POSITION[] = { 0, 0, 0,
                                             1, 0, 0,
                                             1, 1, 0,
                                             0, 1, 0,
                                             0, 0, 1,
                                             1, 0, 1,
                                             1, 1, 1,
                                             0, 1, 1, };

static const uint FT_VOX_CUBE_NORMAL[] = { 0, 0, 1, /* north (-z) */
                                           0, 0, 1, /* south (+z) */
                                           1, 0, 0, /* west  (-x) */
                                           1, 0, 0, /* east  (+x) */
                                           0, 1, 0, /* down  (-y) */
                                           0, 1, 0, /* top   (+y) */ };

static const uint FT_VOX_CUBE_TEXCOORD[] = { 1, 0,
                                             0, 0,
                                             0, 1,
                                             1, 1, };


static int __ftChunkFaceCube(GLuint *pVertices, GLsizei *pVsize, GLuint *pIndices, GLsizei *pIsize, int face, int x, int y, int z) {
    /* 'pVertices' : send face */
    GLsizei base = *pVsize;
    for (size_t j = 0; j < 4; j++) {
        const uint *position = &FT_VOX_CUBE_POSITION[FT_VOX_CUBE_INDICES[face * 6 + FT_VOX_FACE_UNIQUE[j]] * 3];
        const uint   *normal = &FT_VOX_CUBE_NORMAL[face * 3];
        const uint *texcoord = &FT_VOX_CUBE_TEXCOORD[j * 2];
        GLuint vertex = /* pack: 'position */
                        (((position[0] + (GLuint) x) & 0xffu) << 0)  |
                        (((position[1] + (GLuint) y) & 0xffu) << 8)  |
                        (((position[2] + (GLuint) z) & 0xffu) << 16) |

                        /* pack: 'normal' */
                        (((normal[0]) & 0x01u) << 24) |
                        (((normal[1]) & 0x01u) << 25) |
                        (((normal[2]) & 0x01u) << 26) |
                        
                        /* pack: 'texoord' */
                        ((texcoord[0] & 0x01u) << 27) |
                        ((texcoord[1] & 0x01u) << 28);

        pVertices[(*pVsize)++] = vertex;
    }

    /* 'pIndices' : send face */
    for (size_t j = 0; j < 6; j++) {
        GLuint index = base + FT_VOX_FACE_INDICES[j];
        pIndices[(*pIsize)++] = index;
    }

    /* success */
    return (1);
}

/* public API */

extern int ftChunkCreate(struct Chunk *self, struct World *pWorld, int x, int y, int z) {
    /* null-check */
    if (!self)   { return (0); }
    if (!pWorld) { return (0); }

    /* set 'chunk's 'parent' */
    self->parent = pWorld;

    /* set 'offset' */
    self->offset = alg_vec3(x, y, z);

    /* set 'position' */
    self->position = alg_vec3(x * FT_VOX_CHUNK_SIZE_X,
                              y * FT_VOX_CHUNK_SIZE_Y,
                              z * FT_VOX_CHUNK_SIZE_Z);

    /* set 'flags' */
    self->flags.dirty = 0;
    self->flags.empty = 1;
    self->flags.ready = 0;

    /* success */
    return (1);
}


extern int ftChunkAlloc(struct Chunk *self) {
    /* null-check */
    if (!self) { return (0); }

    /* get 'data' */
    uint *data = self->data;
    if (!data) {
        /* alloc. 'data' */
        data = (uint *) calloc(FT_VOX_CHUNK_SIZE_X *
                               FT_VOX_CHUNK_SIZE_Y *
                               FT_VOX_CHUNK_SIZE_Z, sizeof(uint));
        if (!data) {
            return (0);
        }

        /* return 'data' */
        self->data = data;
    }
    
    /* get 'mesh' */
    struct
    BlockMesh *mesh = self->mesh;
    if (!mesh) {
        /* alloc. 'mesh' */
        mesh = (struct BlockMesh *) calloc(1, sizeof(struct BlockMesh));
        if (!mesh) {
            return (0);
        }

        /* return 'mesh' */
        glCreateBuffers(1, &mesh->vbo.id);
        glCreateBuffers(1, &mesh->ibo[0].id);
        glCreateBuffers(1, &mesh->ibo[1].id);
        self->mesh = mesh;
    }

    /* success */
    return (1);
}


extern int ftChunkRelease(struct Chunk *self) {
    /* null-check */
    if (!self) { return (0); }
    
    /* get 'mesh' */
    struct
    BlockMesh *mesh = self->mesh;
    if (mesh) {
        glDeleteBuffers(1, &mesh->vbo.id);
        glDeleteBuffers(1, &mesh->ibo[0].id);
        glDeleteBuffers(1, &mesh->ibo[1].id);
        free(mesh);
        self->mesh = 0;
    }
    
    /* get 'data' */
    uint *data = self->data;
    if (data) {
        free(data);
        self->data = 0;
    }

    /* success */
    return (1);
}


extern int ftChunkRender(struct Chunk *self, struct VertexArray *pVao, struct Shader *pShader) {
    /* null-check */
    if (!self)    { return (0); }
    if (!pVao)    { return (0); }
    if (!pShader) { return (0); }

    /* get 'mesh' */
    struct
    BlockMesh *mesh = self->mesh;
    if (!mesh) { return (0); }

    /* check 'flags' */
    if (self->flags.empty) { return (1); }
    if (self->flags.dirty) { ftChunkGenerateMesh(self); }
       
    /* 'u_model' matrix */
    mat4 u_model = alg_mat4translate(self->position);
    ftShaderUniformSet(pShader, "u_model", FT_UNIFORM_MATRIX4X4, &u_model);
        
    /* set 'vbo' */
    ftVertexArrayVertexBuffer(pVao, mesh->vbo.id);

    /* render 'solid' */
    if (mesh->ibo[0].sz > 0) {
        /* set 'ibo[0]' */
        ftVertexArrayIndexBuffer(pVao, mesh->ibo[0].id);

        /* draw-call */
        glDrawElements(GL_TRIANGLES, mesh->ibo[0].sz, GL_UNSIGNED_INT, (GLvoid *) (0));
    }

    /* success */
    return (1);
}


extern int ftChunkGenerateData(struct Chunk *self, int seed) {
    /* null-check */
    if (!self) { return (0); }

    /* get 'data' */
    uint *data = self->data;
    if (!data) {
        return (0);
    }
   
    /* fill 'data' with some procedural ID's */
    self->flags.ready = 0;
    for (int y = 0; y < FT_VOX_CHUNK_SIZE_Y; y++) {
    for (int x = 0; x < FT_VOX_CHUNK_SIZE_X; x++) {
    for (int z = 0; z < FT_VOX_CHUNK_SIZE_Z; z++) {
        double noise = perlin3d((self->position.x + x) * 0.01,
                                (self->position.y + y) * 0.01,
                                (self->position.z + z) * 0.01,
                                 seed);

        /* > 'noise' <= '0.0' : air
         * > 'noise' >  '0.0' : no-air
         * */
        if (noise > 0.0) {
            data[FT_VOX_CHUNK_3D_1D(x, y, z)] = 1;
            self->flags.dirty = 1;
            self->flags.empty = 0;
        }

        /* set 'air' */
        else {
            data[FT_VOX_CHUNK_3D_1D(x, y, z)] = 0;
        }
    }}}

    /* set 'ready' */
    self->flags.ready = 1;

    /* success */
    return (1);
}


extern int ftChunkGenerateMesh(struct Chunk *self) {
    /* null-check */
    if (!self) { return (0); }
    if (!self->data) { return (0); }

    /* check if 'self' is 'ready' */
    if (!self->flags.ready) { return (1); }

    /* get 'mesh' */
    struct
    BlockMesh *mesh = self->mesh;
    if (!mesh) {
        return (0);
    }
    
    /* CPU-to-GPU */

    const size_t size = FT_VOX_CHUNK_SIZE_X *
                        FT_VOX_CHUNK_SIZE_Y *
                        FT_VOX_CHUNK_SIZE_Z;

    /* 'v' : vertex buffer
     * 's' : solid index buffer
     * 't' : transparent index buffer
     * */
    GLuint *v = (GLuint *) malloc(size * 24 * sizeof(GLuint)),
           *s = (GLuint *) malloc(size * 36 * sizeof(GLuint)),
           *t = (GLuint *) malloc(size * 36 * sizeof(GLuint));

    /* 'nv' : 'v' buffer size/index
     * 'ns' : 's' buffer size/index
     * 'nt' : 't' buffer size/index
     * */
    GLsizei nv = 0,
            ns = 0,
            nt = 0;
    
    for (int y = 0; y < FT_VOX_CHUNK_SIZE_Y; y++) {
    for (int x = 0; x < FT_VOX_CHUNK_SIZE_X; x++) {
    for (int z = 0; z < FT_VOX_CHUNK_SIZE_Z; z++) {
        /* get the 'block' */
        uint block = ftChunkGetBlock(self, x, y, z);

        /* block type: 'air' */
        if (!block) { continue; }

        /* block type: 'solid' */
        else if (block == 1) {
            for (size_t i = 0; i < 6; i++) {
                const int *direction = &FT_VOX_FACE_DIRECTION[i * 3];

                /* get the neighbor 'block' */
                uint block = ftChunkGetBlock(self, x + direction[0],
                                                   y + direction[1],
                                                   z + direction[2]);
                
                /* block type: 'solid' */
                if (block == 1) { continue; }

                /* block type: 'air' / 'trans' */
                else {
                    __ftChunkFaceCube(v, &nv, s, &ns, i, x, y, z);
                }
            }
        }
    }}}

    /* set 'v', 's' and 't' to the 'mesh' */

    mesh->vbo.sz = nv;
    glNamedBufferData(mesh->vbo.id, nv * sizeof(GLuint), v, GL_STATIC_DRAW);

    mesh->ibo[0].sz = ns;
    glNamedBufferData(mesh->ibo[0].id, ns * sizeof(GLuint), s, GL_STATIC_DRAW);

    mesh->ibo[1].sz = nt;
    glNamedBufferData(mesh->ibo[1].id, nt * sizeof(GLuint), t, GL_STATIC_DRAW);

    /* release 'v', 's' and 't' buffers */
    free(v); free(s); free(t);

    /* set 'dirty' flag */
    self->flags.dirty = 0;

    /* return 'status' */
    return (1);
}


extern int ftChunkGetBlock(struct Chunk *self, int ix, int iy, int iz) {
    /* null-check */
    if (!self) { return (0); }

    int cx = (int) self->offset.x;
    if (ix < 0) {
        cx--; ix += FT_VOX_CHUNK_SIZE_X;
    } else if (ix >= FT_VOX_CHUNK_SIZE_X) {
        cx++; ix -= FT_VOX_CHUNK_SIZE_X;
    }

    int cy = (int) self->offset.y;
    if (iy < 0) {
        cy--; iy += FT_VOX_CHUNK_SIZE_Y;
    } else if (iy >= FT_VOX_CHUNK_SIZE_Y) {
        cy++; iy -= FT_VOX_CHUNK_SIZE_Y;
    }

    int cz = (int) self->offset.z;
    if (iz < 0) {
        cz--; iz += FT_VOX_CHUNK_SIZE_Z;
    } else if (iz >= FT_VOX_CHUNK_SIZE_Z) {
        cz++; iz -= FT_VOX_CHUNK_SIZE_Z;
    }

    /* get actual 'self' (bound-check) */
    if (cx != (int) self->offset.x ||
        cy != (int) self->offset.y ||
        cz != (int) self->offset.z
    ) {
        ftWorldGetChunk(self->parent, &self, cx, cy, cz);
        if (!self) {
            return (0);
        }
    }

    /* read 'ID' from 'data' */
    uint *data = self->data;
    if (!data) { return (0); }
    
    /* return 'ID' */
    uint ID = self->data[FT_VOX_CHUNK_3D_1D(ix, iy, iz)];

    /* success */
    return (ID);
}


extern int ftChunkSetBlock(struct Chunk *self, int ix, int iy, int iz, uint ID) {
    /* null-check */
    if (!self) { return (0); }

    /* try to set 'ID' into CPU 'data' */
    ix = alg_clamp(ix, 0, FT_VOX_CHUNK_SIZE_X - 1),
    iy = alg_clamp(iy, 0, FT_VOX_CHUNK_SIZE_Y - 1),
    iz = alg_clamp(iz, 0, FT_VOX_CHUNK_SIZE_Z - 1);

    /* get 'data' */
    uint *data = self->data;
    if (data) {
        data[FT_VOX_CHUNK_3D_1D(ix, iy, iz)] = ID;
    }

    /* success */
    return (1);
}
