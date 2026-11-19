#include "ft_vox.h"

/* static API */

static const i8 FT_VOX_FACE_DIRECTION[] = {  0,  0, -1, /* north (-z) */
                                             0,  0,  1, /* south (+z) */
                                            -1,  0,  0, /* west  (-x) */
                                             1,  0,  0, /* east  (+x) */
                                             0, -1,  0, /* down  (-y) */
                                             0,  1,  0, /* top   (+y) */ };

static const u8 FT_VOX_FACE_UNIQUE[]  = { 1, 0, 5, 2 };
static const u8 FT_VOX_FACE_INDICES[] = { 1, 0, 3, 1, 3, 2, };
static const u8 FT_VOX_CUBE_INDICES[] = { 1, 0, 3, 1, 3, 2, /* north (-z) */
                                          4, 5, 6, 4, 6, 7, /* south (+z) */
                                          0, 4, 7, 0, 7, 3, /* west  (-x) */
                                          5, 1, 2, 5, 2, 6, /* east  (+x) */
                                          5, 4, 0, 5, 0, 1, /* down  (-y) */
                                          2, 3, 7, 2, 7, 6, /* top   (+y) */ };

static const u8 FT_VOX_CUBE_POSITION[] = { 0, 0, 0,
                                           1, 0, 0,
                                           1, 1, 0,
                                           0, 1, 0,
                                           0, 0, 1,
                                           1, 0, 1,
                                           1, 1, 1,
                                           0, 1, 1, };

static const u8 FT_VOX_CUBE_NORMAL[] = { 0, 0, 1, /* north (-z) */
                                         0, 0, 1, /* south (+z) */
                                         1, 0, 0, /* west  (-x) */
                                         1, 0, 0, /* east  (+x) */
                                         0, 1, 0, /* down  (-y) */
                                         0, 1, 0, /* top   (+y) */ };

static const u8 FT_VOX_CUBE_TEXCOORD[] = { 1, 0,
                                           0, 0,
                                           0, 1,
                                           1, 1, };


static int __ft_face_cube(u32 *vb, size_t *vbs, u32 *ib, size_t *ibs, int face, int x, int y, int z) {
    /* 'vb' : send face */
    GLsizei base = *vbs;
    for (size_t j = 0; j < 4; j++) {
        const u8 *vp = &FT_VOX_CUBE_POSITION[FT_VOX_CUBE_INDICES[face * 6 + FT_VOX_FACE_UNIQUE[j]] * 3];
        const u8 *vn = &FT_VOX_CUBE_NORMAL[face * 3];
        const u8 *vt = &FT_VOX_CUBE_TEXCOORD[j * 2];
        u32 vertex = /* pack: 'vp' */
                      (((vp[0] + (u32) x) & 0xffu) << 0)  |
                      (((vp[1] + (u32) y) & 0xffu) << 8)  |
                      (((vp[2] + (u32) z) & 0xffu) << 16) |

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
        u32 index = base + FT_VOX_FACE_INDICES[j];
        ib[(*ibs)++] = index;
    }

    /* success */
    return (1);
}


static int __ft_chunk_generate(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* check 'state' */
    SDL_atomic_t *state = &self->atoms.state;
    if (SDL_AtomicGet(&self->atoms.state) != FT_CHUNK_CPU_QUEUED) {
        return (0);
    }

    /* get 'data' */
    u32 *id = self->data.id;
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
            ft_chunk_set(self, x, y, z, 1);
        }

        /* set 'air' */
        else {
            ft_chunk_set(self, x, y, z, 0);
        }
    }}}

    /* set 'state' */
    SDL_AtomicSet(state, FT_CHUNK_CPU_FINISH);

    /* success */
    return (1);
}


static int __ft_chunk_mesh_cpu(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* check 'state' */
    SDL_atomic_t *state = &self->atoms.state;
    if (SDL_AtomicGet(state) != FT_CHUNK_GPU_QUEUED) {
        return (0);
    }

    /* get 'data' */
    u32 *vb = self->data.vb; size_t *vbc = &self->data.vbc;
    u32 *sb = self->data.sb; size_t *sbc = &self->data.sbc;
    u32 *tb = self->data.tb; size_t *tbc = &self->data.tbc;
    for (int y = 0; y < FT_VOX_CHUNK_SIZE_Y; y++) {
    for (int x = 0; x < FT_VOX_CHUNK_SIZE_X; x++) {
    for (int z = 0; z < FT_VOX_CHUNK_SIZE_Z; z++) {
        /* get the 'block' */
        int id = ft_chunk_get(self, x, y, z);

        /* case : 'air' */
        if (!id) { continue; }
            
        /* get the neighbors */
        int ids[6] = { 0 };
        for (size_t i = 0; i < 6; i++) {
            const i8 *direction = &FT_VOX_FACE_DIRECTION[i * 3];
            ids[i] = ft_chunk_get(self, x + direction[0],
                                        y + direction[1],
                                        z + direction[2]);
        }

        /* case : 'issolid' */
        if (id == 1) {
            /* assemble faces */
            for (size_t i = 0; i < 6; i++) {
                int id = ids[i];

                /* skip if not 'trans.' */
                if (id != 0) { continue; }

                __ft_face_cube(vb, vbc,
                               sb, sbc,
                               i, x, y, z);
            }
        }

        /* case : 'istrans' */
        else if (id == 2) {
            /* assemble faces */
            for (size_t i = 0; i < 6; i++) {
                int id = ids[i];

                /* skip if not 'solid', nor 'air' */
                if (id != 0 &&
                    id != 1
                ) {
                    continue;
                }

                __ft_face_cube(vb, vbc,
                               tb, tbc,
                               i, x, y, z);
            }
        }

    }}}

    /* set 'state' */
    SDL_AtomicSet(state, FT_CHUNK_GPU_FINISH);

    /* success */
    return (1);
}


static int __ft_chunk_mesh_gpu(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* check 'state' */
    SDL_atomic_t *state = &self->atoms.state;
    if (SDL_AtomicGet(state) != FT_CHUNK_GPU_FINISH) {
        return (0);
    }

    /* get 'data' */
    u32 *vb = self->data.vb; size_t vbc = self->data.vbc;
    u32 *sb = self->data.sb; size_t sbc = self->data.sbc;
    u32 *tb = self->data.tb; size_t tbc = self->data.tbc;

    /* get 'mesh' */
    struct
    chunk_mesh_s *mesh = &self->mesh;

    /* set 'vertex' data */
    if (vbc > 0) {
        self->mesh.vbo.ids = self->data.vbc;
        glNamedBufferData(mesh->vbo.id, vbc * sizeof(u32), vb, GL_DYNAMIC_DRAW);
    }

    /* set 'solid' data */
    if (sbc) {
        self->mesh.sbo.ids = self->data.sbc;
        glNamedBufferData(mesh->sbo.id, sbc * sizeof(u32), sb, GL_DYNAMIC_DRAW);
    }
    
    /* set 'trans' data */
    if (tbc) {
        self->mesh.tbo.ids = self->data.tbc;
        glNamedBufferData(mesh->tbo.id, tbc * sizeof(u32), tb, GL_DYNAMIC_DRAW);
    }

    /* set 'state' */
    SDL_AtomicSet(state, FT_CHUNK_RENDER);

    /* return 'status' */
    return (1);
}


static int __ft_chunk_queue_job(struct chunk_s *self, struct thpool_s *pool, thread_pool_func_t job) {
    /* null-check */
    if (!self) { return (0); }
    if (!pool) { return (0); }

    /* pin */
    SDL_AtomicAdd(&self->atoms.refer, 1);
    if (!ft_thpool_queue(pool, self, job)) {
        /* unpin on error */
        SDL_AtomicAdd(&self->atoms.refer, -1);

        /* failure */
        return (0);
    }

    /* success */
    return (1);
}

/* public API */

extern int ft_chunk_create(struct chunk_s *self, struct world_s *parent, int x, int y, int z) {
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


extern int ft_chunk_delete(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); } 

    /* release 'self's resources */
    ft_chunk_free(self);
    
    /* release 'self' */
    free(self);

    /* success */
    return (1);
}


extern int ft_chunk_update(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); }
    
    /* 'pool' reference  */    
    struct thpool_s *pool = self->parent->pool;
    if (!pool) { return (0); }
    
    /* check 'state' */
    switch (SDL_AtomicGet(&self->atoms.state)) {
        case (FT_CHUNK_CREATE): {
            if (SDL_AtomicCAS(&self->atoms.state, FT_CHUNK_CREATE,
                                                  FT_CHUNK_CPU_QUEUED)
            ) {
                __ft_chunk_queue_job(self, pool, (thread_pool_func_t) ft_chunk_generate);
            }
        } break;
        
        case (FT_CHUNK_CPU_FINISH): {
            if (SDL_AtomicCAS(&self->atoms.state, FT_CHUNK_CPU_FINISH,
                                                  FT_CHUNK_GPU_QUEUED)
            ) {
                __ft_chunk_queue_job(self, pool, (thread_pool_func_t) ft_chunk_mesh_cpu);
            }
        } break;
        
        case (FT_CHUNK_GPU_FINISH): {
            ft_chunk_mesh_gpu(self);
        } break;

        case (FT_CHUNK_RENDER): {
            /* check if chunk was modified ('dirty' flag set) */
            if (SDL_AtomicGet(&self->atoms.dirty)) {
                if (SDL_AtomicCAS(&self->atoms.state, FT_CHUNK_RENDER,
                                                      FT_CHUNK_GPU_QUEUED)
                ) {
                    /* re-mesh 'self' */
                    SDL_AtomicSet(&self->atoms.dirty, 0);
                    __ft_chunk_queue_job(self, pool, (thread_pool_func_t) ft_chunk_mesh_cpu);
                }
            }
        } break;
    }

    /* success */
    return (1);
}


extern int ft_chunk_render(struct chunk_s *self, struct vao_s *vao, struct shader_s *shader) {
    /* null-check */
    if (!self) { return (0); }

    /* atom-check */
    if (SDL_AtomicGet(&self->atoms.state) != FT_CHUNK_RENDER) { return (1); }
       
    /* 'u_model' matrix */
    mat4 u_model = alg_mat4translate(self->offset);
    ft_shader_uniform(shader, "u_model", FT_UNIFORM_MATRIX4X4, &u_model);

    /* render 'solid' */
    if (self->data.vbc > 0 &&
        self->data.sbc
    ) {
        /* set 'mesh' */
        ft_vao_vbo(vao, self->mesh.vbo.id);
        ft_vao_ibo(vao, self->mesh.sbo.id);

        /* draw-call */
        glDrawElements(GL_TRIANGLES, self->mesh.sbo.ids, GL_UNSIGNED_INT, (GLvoid *) (0));
    
        /* un-set 'mesh' */
        ft_vao_ibo(vao, 0);
        ft_vao_vbo(vao, 0);
    }

    /* success */
    return (1);
}


extern int ft_chunk_alloc(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); }
        
    size_t size = FT_VOX_CHUNK_SIZE_X *
                  FT_VOX_CHUNK_SIZE_Y *
                  FT_VOX_CHUNK_SIZE_Z;

    /* create 'data' */
    memset(&self->data, 0, sizeof(struct chunk_data_s));

    /* alloc. 'id' */
    u32 *id = (u32 *) malloc(size * sizeof(u32));
    if (!id) { ft_chunk_delete(self); return (0); }
    self->data.id = id; self->data.ids = size; self->data.idc = 0;

    /* alloc. 'vb' */
    u32 vbs = size * 8;
    u32 *vb = (u32 *) calloc(vbs, sizeof(u32));
    if (!vb) { ft_chunk_delete(self); return (0); }
    self->data.vb = vb; self->data.vbs = size; self->data.vbc = 0;

    /* alloc. 'sb' */
    u32 sbs = size * 6;
    u32 *sb = (u32 *) calloc(sbs, sizeof(u32));
    if (!sb) { ft_chunk_delete(self); return (0); }
    self->data.sb = sb; self->data.sbs = size; self->data.sbc = 0;

    /* alloc. 'tb' */
    u32 tbs = size * 6;
    u32 *tb = (u32 *) calloc(tbs, sizeof(u32));
    if (!tb) { ft_chunk_delete(self); return (0); }
    self->data.tb = tb; self->data.tbs = size; self->data.tbc = 0;
    
    /* create 'mesh' */
    memset(&self->mesh, 0, sizeof(struct chunk_mesh_s));

    /* return 'mesh' */
    glCreateBuffers(1, &self->mesh.vbo.id);
    glCreateBuffers(1, &self->mesh.sbo.id);
    glCreateBuffers(1, &self->mesh.tbo.id);

    /* success */
    return (1);
}


extern int ft_chunk_clear(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); }
    
    /* clear 'id' */
    u32 *id = self->data.id;
    if (id) {
        memset(id, 0, self->data.ids * sizeof(u32));
        self->data.idc = 0;
    }
    
    /* clear 'vb' */
    u32 *vb = self->data.vb;
    if (vb) {
        memset(vb, 0, self->data.vbs * sizeof(u32));
        self->data.vbc = 0;
    }
    
    /* clear 'sb' */
    u32 *sb = self->data.sb;
    if (sb) {
        memset(sb, 0, self->data.sbs * sizeof(u32));
        self->data.sbc = 0;
    }
    
    /* clear 'tb' */
    u32 *tb = self->data.tb;
    if (tb) {
        memset(tb, 0, self->data.tbs * sizeof(u32));
        self->data.tbc = 0;
    }

    /* success */
    return (1);
}


extern int ft_chunk_free(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); }
    
    /* release 'mesh' */
    glDeleteBuffers(1, &self->mesh.vbo.id);
    glDeleteBuffers(1, &self->mesh.sbo.id);
    glDeleteBuffers(1, &self->mesh.tbo.id);
    
    /* clear 'mesh' */
    memset(&self->mesh, 0, sizeof(struct chunk_mesh_s));

    /* release 'id' */
    u32 *id = self->data.id;
    if (id) { free(id); }
    
    /* release 'vb' */
    u32 *vb = self->data.vb;
    if (vb) { free(vb); }
    
    /* release 'sb' */
    u32 *sb = self->data.sb;
    if (sb) { free(sb); }
    
    /* release 'tb' */
    u32 *tb = self->data.tb;
    if (tb) { free(tb); }
    
    /* clear 'data' */
    memset(&self->data, 0, sizeof(struct chunk_data_s));

    /* success */
    return (1);
}


extern int ft_chunk_generate(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* call func. */
    int status = __ft_chunk_generate(self);
    SDL_AtomicAdd(&self->atoms.refer, -1);

    /* return 'status' */
    return (status);
}


extern int ft_chunk_mesh_cpu(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* call func. */
    int status = __ft_chunk_mesh_cpu(self);
    SDL_AtomicAdd(&self->atoms.refer, -1);

    /* return 'status' */
    return (status);
}


extern int ft_chunk_mesh_gpu(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* call func. */
    int status = __ft_chunk_mesh_gpu(self);

    /* return 'status' */
    return (status);
}


extern int ft_chunk_get(struct chunk_s *self, int bx, int by, int bz) {
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
        ft_world_get(self->parent, &self, cx, cy, cz);
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


extern int ft_chunk_set(struct chunk_s *self, int bx, int by, int bz, int id) {
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
