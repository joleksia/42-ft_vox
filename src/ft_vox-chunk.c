#include "ft_vox.h"

/* static API */

static const byte FT_VOX_FACE_DIRECTION[] = {  0,  0, -1, /* north (-z) */
                                               0,  0,  1, /* south (+z) */
                                              -1,  0,  0, /* west  (-x) */
                                               1,  0,  0, /* east  (+x) */
                                               0, -1,  0, /* down  (-y) */
                                               0,  1,  0, /* top   (+y) */ };

static const ubyte FT_VOX_FACE_UNIQUE[] = { 1, 0, 5, 2 };
static const ubyte FT_VOX_FACE_INDICES[] = { 1, 0, 3, 1, 3, 2, };
static const ubyte FT_VOX_CUBE_INDICES[] = { 1, 0, 3, 1, 3, 2, /* north (-z) */
                                             4, 5, 6, 4, 6, 7, /* south (+z) */
                                             0, 4, 7, 0, 7, 3, /* west  (-x) */
                                             5, 1, 2, 5, 2, 6, /* east  (+x) */
                                             5, 4, 0, 5, 0, 1, /* down  (-y) */
                                             2, 3, 7, 2, 7, 6, /* top   (+y) */ };

static const ubyte FT_VOX_CUBE_POSITION[] = { 0, 0, 0,
                                              1, 0, 0,
                                              1, 1, 0,
                                              0, 1, 0,
                                              0, 0, 1,
                                              1, 0, 1,
                                              1, 1, 1,
                                              0, 1, 1, };

static const ubyte FT_VOX_CUBE_NORMAL[] = { 0, 0, 1, /* north (-z) */
                                            0, 0, 1, /* south (+z) */
                                            1, 0, 0, /* west  (-x) */
                                            1, 0, 0, /* east  (+x) */
                                            0, 1, 0, /* down  (-y) */
                                            0, 1, 0, /* top   (+y) */ };

static const ubyte FT_VOX_CUBE_TEXCOORD[] = { 1, 0,
                                              0, 0,
                                              0, 1,
                                              1, 1, };


static int __ftChunkFaceCube(uint *vb, size_t *vbs, uint *ib, size_t *ibs, int face, int x, int y, int z) {
    /* 'vb' : send face */
    GLsizei base = *vbs;
    for (size_t j = 0; j < 4; j++) {
        const ubyte *vp = &FT_VOX_CUBE_POSITION[FT_VOX_CUBE_INDICES[face * 6 + FT_VOX_FACE_UNIQUE[j]] * 3];
        const ubyte *vn = &FT_VOX_CUBE_NORMAL[face * 3];
        const ubyte *vt = &FT_VOX_CUBE_TEXCOORD[j * 2];
        uint vertex = /* pack: 'vp' */
                      (((vp[0] + (uint) x) & 0xffu) << 0)  |
                      (((vp[1] + (uint) y) & 0xffu) << 8)  |
                      (((vp[2] + (uint) z) & 0xffu) << 16) |

                      /* pack: 'vn' */
                      (((vn[0]) & 0x01u) << 24) |
                      (((vn[1]) & 0x01u) << 25) |
                      (((vn[2]) & 0x01u) << 26) |
                        
                      /* pack: 'vt' */
                      ((vt[0] & 0x01u) << 27) |
                      ((vt[1] & 0x01u) << 28);

        vb[(*vbs)++] = vertex;
    }

    /* 'ib' : send face */
    for (size_t j = 0; j < 6; j++) {
        uint index = base + FT_VOX_FACE_INDICES[j];
        ib[(*ibs)++] = index;
    }

    /* success */
    return (1);
}


static int __ftChunkGenCPU(struct Chunk *self) {
    /* null-check */
    if (!self) { return (0); }

    /* check 'state' */
    SDL_atomic_t *state = &self->atoms.state;
    if (SDL_AtomicGet(&self->atoms.state) != FT_CHUNK_CPU_QUEUED) {
        return (0);
    }

    /* get 'data' */
    uint *id = self->data.id;
    if (!id) {
        return (0);
    }

    /* get 'offset' */
    vec3 offset = self->offset;

    /* get 'seed' from 'parent' */
    int seed = self->parent->seed;
   
    /* fill 'data' with some procedural ID's */
    for (int y = 0; y < FT_VOX_CHUNK_SIZE_Y; y++) {
    for (int x = 0; x < FT_VOX_CHUNK_SIZE_X; x++) {
    for (int z = 0; z < FT_VOX_CHUNK_SIZE_Z; z++) {
        double noise = perlin3d((offset.x + x) * 0.01,
                                (offset.y + y) * 0.01,
                                (offset.z + z) * 0.01,
                                 seed);

        /* > 'noise' <= '0.0' : air
         * > 'noise' >  '0.0' : no-air
         * */
        if (noise > 0.0) {
            id[FT_VOX_CHUNK_3D_1D(x, y, z)] = 1;
        }

        /* set 'air' */
        else {
            id[FT_VOX_CHUNK_3D_1D(x, y, z)] = 0;
        }
    }}}

    /* set 'state' */
    SDL_AtomicSet(state, FT_CHUNK_CPU_FINISH);

    /* success */
    return (1);
}


static int __ftChunkGenGPU(struct Chunk *self) {
    /* null-check */
    if (!self) { return (0); }

    /* check 'state' */
    SDL_atomic_t *state = &self->atoms.state;
    if (SDL_AtomicGet(state) != FT_CHUNK_GPU_QUEUED) {
        return (0);
    }

    /* get 'data' */
    uint *vb = self->data.vb; size_t vbs = 0;
    uint *sb = self->data.sb; size_t sbs = 0;

    for (int y = 0; y < FT_VOX_CHUNK_SIZE_Y; y++) {
    for (int x = 0; x < FT_VOX_CHUNK_SIZE_X; x++) {
    for (int z = 0; z < FT_VOX_CHUNK_SIZE_Z; z++) {
        /* get the 'block' */
        int id = ftChunkGetBlock(self, x, y, z);
        /* block type: 'air' */
        if (!id) { continue; }
        
        /* get the neighbors */
        int ids[6] = { 0 };
        for (size_t i = 0; i < 6; i++) {
            const byte *direction = &FT_VOX_FACE_DIRECTION[i * 3];
            ids[i] = ftChunkGetBlock(self, x + direction[0],
                                           y + direction[1],
                                           z + direction[2]);
        }

        /* assemble faces */
        for (size_t i = 0; i < 6; i++) {
            int id = ids[i];

            /* skip if not 'air' */
            if (id != 0) { continue; }

            __ftChunkFaceCube(vb, &vbs,
                              sb, &sbs,
                              i, x, y, z);
        }

    }}}

    /* set 'state' */
    SDL_AtomicSet(state, FT_CHUNK_GPU_FINISH);

    /* success */
    return (1);
}

/* public API */

extern int ftChunkCreate(struct Chunk *self, struct World *parent, int x, int y, int z) {
    /* null-check */
    if (!self)   { return (0); }
    if (!parent) { return (0); }

    /* set 'chunk's 'parent' */
    self->parent = parent;

    /* set 'atoms' */
    SDL_AtomicSet(&self->atoms.state, FT_CHUNK_CREATE);

    /* set 'origin' */
    self->origin = alg_vec3(x, y, z);

    /* set 'offset' */
    self->offset = alg_vec3(x * FT_VOX_CHUNK_SIZE_X,
                            y * FT_VOX_CHUNK_SIZE_Y,
                            z * FT_VOX_CHUNK_SIZE_Z);

    /* set 'state' */
    SDL_AtomicSet(&self->atoms.state, FT_CHUNK_CREATE);

    /* success */
    return (1);
}


extern int ftChunkDelete(struct Chunk *self) {
    /* null-check */
    if (!self) { return (0); } 

    /* release 'self' */
    ftChunkFree(self);
    free(self);

    /* success */
    return (1);
}


extern int ftChunkUpdate(struct Chunk *self) {
    /* null-check */
    if (!self) { return (0); }
    
    /* 'pool' reference  */    
    struct
    ThreadPool *pool = self->parent->pool;
    if (!pool) { return (0); }
    
    /* check 'state' */
    switch (SDL_AtomicGet(&self->atoms.state)) {
        case (FT_CHUNK_CREATE): {
            if (SDL_AtomicCAS(&self->atoms.state, FT_CHUNK_CREATE,
                                                  FT_CHUNK_CPU_QUEUED)
            ) {
                ftThreadPoolQueue(pool, self, (thread_pool_func_t) ftChunkGenCPU);
            }
        } break;
        
        case (FT_CHUNK_CPU_FINISH): {
            if (SDL_AtomicCAS(&self->atoms.state, FT_CHUNK_CPU_FINISH,
                                                  FT_CHUNK_GPU_QUEUED)
            ) {
                ftThreadPoolQueue(pool, self, (thread_pool_func_t) ftChunkGenGPU);
            }
        } break;
        
        case (FT_CHUNK_GPU_FINISH): {
            ftChunkBakeMesh(self);
        } break;

        case (FT_CHUNK_RENDER): { /* ... */ } break;
    }

    /* success */
    return (1);
}


extern int ftChunkRender(struct Chunk *self, struct VertexArray *vao, struct Shader *shader) {
    /* null-check */
    if (!self) { return (0); }

    /* atom-check */
    if (SDL_AtomicGet(&self->atoms.state) != FT_CHUNK_RENDER) { return (1); }
       
    /* 'u_model' matrix */
    mat4 u_model = alg_mat4translate(self->offset);
    ftShaderUniformSet(shader, "u_model", FT_UNIFORM_MATRIX4X4, &u_model);
        
    /* set 'mesh' */
    ftVertexArrayIndexBuffer(vao, self->mesh.ibo.id);
    ftVertexArrayVertexBuffer(vao, self->mesh.vbo.id);

    /* draw-call */
    glDrawElements(GL_TRIANGLES, self->mesh.ibo.ids, GL_UNSIGNED_INT, (GLvoid *) (0));
    
    /* un-set 'mesh' */
    ftVertexArrayVertexBuffer(vao, 0);
    ftVertexArrayIndexBuffer(vao, 0);

    /* success */
    return (1);
}


extern int ftChunkAlloc(struct Chunk *self) {
    /* null-check */
    if (!self) { return (0); }
        
    size_t size = FT_VOX_CHUNK_SIZE_X *
                  FT_VOX_CHUNK_SIZE_Y *
                  FT_VOX_CHUNK_SIZE_Z;

    /* create 'data' */
    memset(&self->data, 0, sizeof(struct chunk_data_s));

    /* alloc. 'id' */
    uint *id = (uint *) malloc(size * sizeof(uint));
    if (!id) { ftChunkDelete(self); return (0); }
    self->data.id = id; self->data.ids = size;

    /* alloc. 'vb' */
    uint vbs = size * 8;
    uint *vb = (uint *) calloc(vbs, sizeof(uint));
    if (!vb) { ftChunkDelete(self); return (0); }
    self->data.vb = vb; self->data.vbs = vbs;

    /* alloc. 'sb' */
    uint sbs = size * 6;
    uint *sb = (uint *) calloc(sbs, sizeof(uint));
    if (!sb) { ftChunkDelete(self); return (0); }
    self->data.sb = sb; self->data.sbs = sbs;

    /* alloc. 'tb' */
    uint tbs = size * 6;
    uint *tb = (uint *) calloc(tbs, sizeof(uint));
    if (!tb) { ftChunkDelete(self); return (0); }
    self->data.tb = tb; self->data.tbs = tbs;
    
    /* create 'mesh' */
    memset(&self->mesh, 0, sizeof(struct chunk_mesh_s));

    /* return 'mesh' */
    glCreateBuffers(1, &self->mesh.vbo.id);
    glCreateBuffers(1, &self->mesh.ibo.id);

    /* success */
    return (1);
}


extern int ftChunkClear(struct Chunk *self) {
    /* null-check */
    if (!self) { return (0); }
    
    size_t size = FT_VOX_CHUNK_SIZE_X *
                  FT_VOX_CHUNK_SIZE_Y *
                  FT_VOX_CHUNK_SIZE_Z;

    /* clear 'id' */
    uint *id = self->data.id;
    if (id) { memset(id, 0, size * sizeof(uint)); }
    
    /* clear 'vb' */
    uint *vb = self->data.vb;
    if (vb) { memset(vb, 0, self->data.vbs * sizeof(uint)); }
    
    /* clear 'sb' */
    uint *sb = self->data.sb;
    if (sb) { memset(sb, 0, self->data.sbs * sizeof(uint)); }
    
    /* clear 'tb' */
    uint *tb = self->data.tb;
    if (tb) { memset(tb, 0, self->data.tbs * sizeof(uint)); }

    /* success */
    return (1);
}


extern int ftChunkFree(struct Chunk *self) {
    /* null-check */
    if (!self) { return (0); }
    
    /* release 'mesh' */
    glDeleteBuffers(1, &self->mesh.vbo.id);
    glDeleteBuffers(1, &self->mesh.ibo.id);
    
    /* clear 'mesh' */
    memset(&self->mesh, 0, sizeof(struct chunk_mesh_s));

    /* release 'id' */
    uint *id = self->data.id;
    if (id) { free(id); }
    
    /* release 'vb' */
    uint *vb = self->data.vb;
    if (vb) { free(vb); }
    
    /* release 'sb' */
    uint *sb = self->data.sb;
    if (sb) { free(sb); }
    
    /* release 'tb' */
    uint *tb = self->data.tb;
    if (tb) { free(tb); }
    
    /* clear 'data' */
    memset(&self->data, 0, sizeof(struct chunk_data_s));

    /* success */
    return (1);
}


extern int ftChunkGenCPU(struct Chunk *self) {
    /* null-check */
    if (!self) { return (0); }

    /* inc. 'refer' counter */
    SDL_AtomicIncRef(&self->atoms.refer);

    /* call func. */
    int status = __ftChunkGenCPU(self);

    /* dec. 'refer' counter */
    SDL_AtomicDecRef(&self->atoms.refer);

    /* return 'status' */
    return (status);
}


extern int ftChunkGenGPU(struct Chunk *self) {
    /* null-check */
    if (!self) { return (0); }

    /* inc. 'refer' counter */
    SDL_AtomicIncRef(&self->atoms.refer);

    /* call func. */
    int status = __ftChunkGenGPU(self);

    /* dec. 'refer' counter */
    SDL_AtomicDecRef(&self->atoms.refer);

    /* return 'status' */
    return (status);
}


extern int ftChunkBakeMesh(struct Chunk *self) {
    /* null-check */
    if (!self) { return (0); }

    /* check 'state' */
    SDL_atomic_t *state = &self->atoms.state;
    if (SDL_AtomicGet(state) != FT_CHUNK_GPU_FINISH) {
        return (0);
    }

    /* get 'data' */
    uint *vb = self->data.vb; size_t vbs = self->data.vbs;
    uint *sb = self->data.sb; size_t sbs = self->data.sbs;

    /* gert 'mesh' */
    struct
    chunk_mesh_s *mesh = &self->mesh;

    self->mesh.vbo.ids = self->data.vbs;
    glNamedBufferData(mesh->vbo.id, vbs * sizeof(uint), vb, GL_DYNAMIC_DRAW);

    self->mesh.ibo.ids = self->data.sbs;
    glNamedBufferData(mesh->ibo.id, sbs * sizeof(uint), sb, GL_DYNAMIC_DRAW);

    /* set 'state' */
    SDL_AtomicSet(state, FT_CHUNK_RENDER);

    /* return 'status' */
    return (1);
}


extern int ftChunkGetBlock(struct Chunk *self, int bx, int by, int bz) {
    /* null-check */
    if (!self) { return (0); }

    int cx = (int) self->origin.x;
    if (bx < 0) {
        cx--; bx += FT_VOX_CHUNK_SIZE_X;
    } else if (bx >= FT_VOX_CHUNK_SIZE_X) {
        cx++; bx -= FT_VOX_CHUNK_SIZE_X;
    }

    int cy = (int) self->origin.y;
    if (by < 0) {
        cy--; by += FT_VOX_CHUNK_SIZE_Y;
    } else if (by >= FT_VOX_CHUNK_SIZE_Y) {
        cy++; by -= FT_VOX_CHUNK_SIZE_Y;
    }

    int cz = (int) self->origin.z;
    if (bz < 0) {
        cz--; bz += FT_VOX_CHUNK_SIZE_Z;
    } else if (bz >= FT_VOX_CHUNK_SIZE_Z) {
        cz++; bz -= FT_VOX_CHUNK_SIZE_Z;
    }

    /* get actual 'self' (bound-check) */
    if (cx != (int) self->origin.x ||
        cy != (int) self->origin.y ||
        cz != (int) self->origin.z
    ) {
        ftWorldGetChunk(self->parent, &self, cx, cy, cz);
        if (!self) {
            return (-1);
        }
    }
    
    /* return 'id' */
    int id = -1;
    if (self->data.id) {
        id = self->data.id[FT_VOX_CHUNK_3D_1D(bx, by, bz)];
    }

    /* success */
    return (id);
}


extern int ftChunkSetBlock(struct Chunk *self, int bx, int by, int bz, int id) {
    /* null-check */
    if (!self) { return (0); }

    /* try to set 'ID' into CPU 'data' */
    bx = alg_clamp(bx, 0, FT_VOX_CHUNK_SIZE_X - 1),
    by = alg_clamp(by, 0, FT_VOX_CHUNK_SIZE_Y - 1),
    bz = alg_clamp(bz, 0, FT_VOX_CHUNK_SIZE_Z - 1);

    /* set 'id' */
    if (self->data.id) {
        self->data.id[FT_VOX_CHUNK_3D_1D(bx, by, bz)] = id;
    }

    /* success */
    return (1);
}
