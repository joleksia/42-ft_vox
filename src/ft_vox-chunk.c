#include "ft_vox.h"

/* static API */

static u32 __ft_vertex(u8, u8, u8, u8, u8, u8);

static int __ft_face_cube(u32, int, int, int, int, u32 *, u32 *, u32 *);

static int __ft_push(u32 **, u32 *, u32 *, u32 *, u32);

static int __ft_chunk_data_alloc(struct chunk_data_s *);

static int __ft_chunk_data_free(struct chunk_data_s *);

static int __ft_chunk_mesh_alloc(struct chunk_mesh_s *);

static int __ft_chunk_mesh_free(struct chunk_mesh_s *);

static int __ft_chunk_mesh_render_s(struct chunk_mesh_s *, struct chunk_s *, struct shader_s *);

static int __ft_chunk_mesh_render_t(struct chunk_mesh_s *, struct chunk_s *, struct shader_s *);

static int __ft_chunk_generate(struct chunk_s *);

static int __ft_chunk_mesh_bake(struct chunk_data_s *, struct chunk_s *);

static int __ft_chunk_mesh_upload(struct chunk_mesh_s *, struct chunk_data_s *, struct chunk_s *);

/* public API */

extern int ft_chunk_create(struct chunk_s **self, struct world_s *parent, int x, int y, int z) {
    /* null-check */
    if (!self) { return (0); }

    /* alloc. 'chunk' */
    struct chunk_s *chunk = (struct chunk_s *) calloc(1, sizeof(struct chunk_s));
    if (!chunk) { return (0); }

    /* set 'chunk's 'parent' */
    chunk->parent = parent;

    /* set 'origin' */
    chunk->origin = alg_vec3(x, y, z);

    /* set 'offset' */
    chunk->offset = alg_vec3(x * FT_VOX_CHUNK_SIZE_X,
                             y * FT_VOX_CHUNK_SIZE_Y,
                             z * FT_VOX_CHUNK_SIZE_Z);

    /* set 'atoms' */
    SDL_AtomicSet(&chunk->atoms.is_created, 1);

    /* return 'self' */
    *self = chunk;

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
    
    /* get 'near' chunks */
    struct
    chunk_s *near[6] = { 0 };
    ft_world_get_near(self->parent, self->origin.x,
                                    self->origin.y,
                                    self->origin.z, near);

    /* case : 'is_created' */
    if (SDL_AtomicCAS(&self->atoms.is_created, 1, 0)) {
        SDL_AtomicAdd(&self->atoms.refer, 1);
        if (!ft_thpool_queue(pool, self, (thread_pool_func_t) ft_chunk_generate)) {
            SDL_AtomicAdd(&self->atoms.refer, -1);
            SDL_AtomicSet(&self->atoms.is_created, 1);
        }
    }

    /* case : 'is_modified' */
    /* case : 'is_generated' */
    if (SDL_AtomicGet(&self->atoms.is_modified) ||
        SDL_AtomicGet(&self->atoms.is_generated)
    ) {
        /* refer-check */
        if (SDL_AtomicGet(&self->atoms.refer) == 0) {
            /* 'ready' condition */
            u8 ready = 1;
            for (int i = 0; i < countof(near); i++) {
                /* null-check */
                if (!near[i]) { ready = 0; break; }

                /* atom-check */
                if (!SDL_AtomicGet(&near[i]->atoms.has_blocks)) { ready = 0; break; }
            }
            
            if (ready) {
                SDL_AtomicSet(&self->atoms.is_modified, 0);
                SDL_AtomicSet(&self->atoms.is_generated, 0);
                SDL_AtomicAdd(&self->atoms.refer, 1);
                if (!ft_thpool_queue(pool, self, (thread_pool_func_t) ft_chunk_mesh_bake)) {
                    SDL_AtomicAdd(&self->atoms.refer, -1);
                    SDL_AtomicSet(&self->atoms.is_generated, 1);
                }
            }
        }
    }

    /* case : 'is_meshed' */
    if (SDL_AtomicCAS(&self->atoms.is_meshed, 1, 0)) {
        SDL_AtomicAdd(&self->atoms.refer, 1);
        if (!ft_chunk_mesh_upload(self)) {
            SDL_AtomicAdd(&self->atoms.refer, -1);
            SDL_AtomicSet(&self->atoms.is_meshed, 1);
        }
    }

    /* success */
    return (1);
}


extern int ft_chunk_render(struct chunk_s *self, struct shader_s *shader) {
    /* null-check */
    if (!self) { return (0); }

    /* atom-check */
    if (SDL_AtomicGet(&self->atoms.is_uploaded)) {
        /* get 'mesh' */
        struct
        chunk_mesh_s *mesh = &self->mesh;
        
        /* render 'solid' */
        if (mesh->count[0] > 0) {
            __ft_chunk_mesh_render_s(mesh, self, shader);
        }

        /* render 'trans' */
        if (mesh->count[1] > 0) {
            __ft_chunk_mesh_render_t(mesh, self, shader);
        }
    }

    /* success */
    return (1);
}


extern int ft_chunk_alloc(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* create 'blocks' */
    u64 *blocks = (u64 *) calloc(FT_VOX_CHUNK_SIZE, sizeof(u64));
    if (!blocks) {
        ft_chunk_delete(self);
        return (0);
    }
    
    self->blocks = blocks;

    /* create 'data' */
    if (!__ft_chunk_data_alloc(&self->data)) {
        ft_chunk_delete(self);
        return (0);
    }
    
    /* create 'mesh' */
    if (!__ft_chunk_mesh_alloc(&self->mesh)) {
        ft_chunk_delete(self);
        return (0);
    }

    /* success */
    return (1);
}


extern int ft_chunk_clear(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* clear 'atoms' */
    SDL_AtomicSet(&self->atoms.has_blocks,   0);
    SDL_AtomicSet(&self->atoms.is_created,   0);
    SDL_AtomicSet(&self->atoms.is_generated, 0);
    SDL_AtomicSet(&self->atoms.is_meshed,    0);
    SDL_AtomicSet(&self->atoms.is_uploaded,  0);
    SDL_AtomicSet(&self->atoms.is_modified,  0);

    /* clear 'blocks' */
    u64 *blocks = self->blocks;
    if (blocks) { memset(blocks, 0, FT_VOX_CHUNK_SIZE * sizeof(*blocks)); }

    /* clear 'mesh's 'count's */
    struct
    chunk_mesh_s *mesh = &self->mesh;
    if (mesh) {
        mesh->count[0] = 0;
        mesh->count[1] = 0;
    }

    /* success */
    return (1);
}


extern int ft_chunk_free(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); }
    
    /* release 'mesh' */
    __ft_chunk_mesh_free(&self->mesh);

    /* release 'data' */
    __ft_chunk_data_free(&self->data);

    /* release 'blocks' */
    u64 *blocks = self->blocks;
    if (blocks) {
        free(blocks);
    }

    /* success */
    return (1);
}


extern int ft_chunk_get(struct chunk_s *self, int bx, int by, int bz, u32 *result) {
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
        ft_world_get(self->parent, cx, cy, cz, &self);
        if (!self) {
            if (result) {
                *result = FT_BLOCK_OUT_OF_BOUNDS;
            }

            /* failure */
            return (0);
        }
    }
   
    /* get 'block' */
    u32 block = 0;
    if (self->blocks) {
        block = self->blocks[FT_VOX_CHUNK_3D_1D(bx, by, bz)];
    }

    /* return 'result' */
    if (result) { *result = block; }

    /* success */
    return (1);
}


extern int ft_chunk_get_near(struct chunk_s *self, int x, int y, int z, u32 *result) {
    /* null-check */
    if (!self) { return (0); }

    /* get the list of 'blocks' */
    u32 blocks[6] = { 0 };
    ft_chunk_get(self, x,     y,     z - 1, &blocks[FT_FACE_NORTH]);
    ft_chunk_get(self, x,     y,     z + 1, &blocks[FT_FACE_SOUTH]);
    ft_chunk_get(self, x - 1, y,     z,     &blocks[FT_FACE_WEST]);
    ft_chunk_get(self, x + 1, y,     z,     &blocks[FT_FACE_EAST]);
    ft_chunk_get(self, x,     y - 1, z,     &blocks[FT_FACE_DOWN]);
    ft_chunk_get(self, x,     y + 1, z,     &blocks[FT_FACE_TOP]);

    /* copy 'blocks' to 'result' */
    if (result) { memcpy(result, blocks, sizeof(blocks)); }

    /* success */
    return (1);
}


extern int ft_chunk_set(struct chunk_s *self, int bx, int by, int bz, int id) {
    /* null-check */
    if (!self) { return (0); }

    /* try to set 'ID' into CPU 'data' */
    bx = alg_clamp(bx, 0, FT_VOX_CHUNK_SIZE_X - 1),
    by = alg_clamp(by, 0, FT_VOX_CHUNK_SIZE_Y - 1),
    bz = alg_clamp(bz, 0, FT_VOX_CHUNK_SIZE_Z - 1);

    /* set 'id' */
    if (self->blocks) {
        self->blocks[FT_VOX_CHUNK_3D_1D(bx, by, bz)] = id;
    }

    /* success */
    return (1);
}


extern int ft_chunk_set_origin(struct chunk_s *self, int cx, int cy, int cz) {
    /* null-check */
    if (!self) { return (0); }

    /* set 'origin' */
    self->origin = alg_vec3(cx, cy, cz);

    /* set 'offset' */
    self->offset = alg_vec3(cx * FT_VOX_CHUNK_SIZE_X,
                            cy * FT_VOX_CHUNK_SIZE_Y,
                            cz * FT_VOX_CHUNK_SIZE_Z);

    /* success */
    return (1);
}


extern int ft_chunk_generate(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* call func. */
    int status = __ft_chunk_generate(self);

    /* return 'status' */
    SDL_AtomicAdd(&self->atoms.refer, -1);
    return (status);
}


extern int ft_chunk_mesh_bake(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* call func. */
    int status = __ft_chunk_mesh_bake(&self->data, self);

    /* return 'status' */
    SDL_AtomicAdd(&self->atoms.refer, -1);
    return (status);
}


extern int ft_chunk_mesh_upload(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* call func. */
    int status = __ft_chunk_mesh_upload(&self->mesh, &self->data, self);

    /* return 'status' */
    SDL_AtomicAdd(&self->atoms.refer, -1);
    return (status);
}

/* static API */

static u32 __ft_vertex(u8 x, u8 y, u8 z, u8 tex_index, u8 tex_corner, u8 face) {
    /* 'vertex' : 32-bit GPU-verticy representation of a single block:
     *  - position (18-bit)
     *  - texture index (6-bit)
     *  - texture corner (2-bit)
     *  - face (3-bit)
     * */
    u32 vertex = 0;

    /* pack : position (18-bit) */
    vertex |= (x & 0x3fu) <<  0|
              (y & 0x3fu) <<  6|
              (z & 0x3fu) << 12;

    /* pack : tex-index (6-bit) */
    vertex |= (tex_index & 0x3fu) << 18;

    /* pack : 'tex-corner' (2-bit) */
    vertex |= (tex_corner & 0x3u) << 24;

    /* pack : 'face' (3-bit) */
    vertex |= (face & 0x7u) << 26;

    /* return 'vertex' */
    return (vertex);
}


static int __ft_face_cube(u32 idx, int face, int x, int y, int z, u32 *v, u32 *i, u32 *ioff) {
    /* get 'block' */
    struct block_s block = BLOCKS[idx];

    /* get 'texcoord' */
    vec2 texcoord = { 0 };
    if (block.texcoord) { block.texcoord(&block, &texcoord, face); }

    /* get 'tex_index' */
    u8 tex_index = texcoord.y * 32.0 + texcoord.x;

    /* vertices... */
    if (!v) { return (0); }
    switch (face) {
        case (FT_FACE_NORTH): {
            v[0] = __ft_vertex(x + 1, y,     z,     tex_index, 0, face);
            v[1] = __ft_vertex(x,     y,     z,     tex_index, 2, face);
            v[2] = __ft_vertex(x,     y + 1, z,     tex_index, 3, face);
            v[3] = __ft_vertex(x + 1, y + 1, z,     tex_index, 1, face);
        } break;

        case (FT_FACE_SOUTH): {
            v[0] = __ft_vertex(x,     y,     z + 1, tex_index, 0, face);
            v[1] = __ft_vertex(x + 1, y,     z + 1, tex_index, 2, face);
            v[2] = __ft_vertex(x + 1, y + 1, z + 1, tex_index, 3, face);
            v[3] = __ft_vertex(x,     y + 1, z + 1, tex_index, 1, face);
        } break;

        case (FT_FACE_WEST): {
            v[0] = __ft_vertex(x,     y,     z,     tex_index, 0, face);
            v[1] = __ft_vertex(x,     y,     z + 1, tex_index, 2, face);
            v[2] = __ft_vertex(x,     y + 1, z + 1, tex_index, 3, face);
            v[3] = __ft_vertex(x,     y + 1, z,     tex_index, 1, face);
        } break;

        case (FT_FACE_EAST): {
            v[0] = __ft_vertex(x + 1, y,     z + 1, tex_index, 0, face);
            v[1] = __ft_vertex(x + 1, y,     z,     tex_index, 2, face);
            v[2] = __ft_vertex(x + 1, y + 1, z,     tex_index, 3, face);
            v[3] = __ft_vertex(x + 1, y + 1, z + 1, tex_index, 1, face);
        } break;

        case (FT_FACE_DOWN): {
            v[0] = __ft_vertex(x,     y,     z,     tex_index, 0, face);
            v[1] = __ft_vertex(x + 1, y,     z,     tex_index, 2, face);
            v[2] = __ft_vertex(x + 1, y,     z + 1, tex_index, 3, face);
            v[3] = __ft_vertex(x,     y,     z + 1, tex_index, 1, face);
        } break;

        case (FT_FACE_TOP): {
            v[0] = __ft_vertex(x,     y + 1, z + 1, tex_index, 0, face);
            v[1] = __ft_vertex(x + 1, y + 1, z + 1, tex_index, 2, face);
            v[2] = __ft_vertex(x + 1, y + 1, z,     tex_index, 3, face);
            v[3] = __ft_vertex(x,     y + 1, z,     tex_index, 1, face);
        } break;
    }

    /* indices... */
    if (!i) { return (0); }
    i[0] = *ioff + 0;
    i[1] = *ioff + 1;
    i[2] = *ioff + 2;
    i[3] = *ioff + 0;
    i[4] = *ioff + 2;
    i[5] = *ioff + 3;

    /* success */
    *ioff += 4;
    return (1);
}


static int __ft_push(u32 **arr, u32 *cnt, u32 *cap, u32 *dat, u32 siz) {
    /* 'cap' check */
    if (*cnt + siz > *cap) {
        /* create 'newcap' capacity */
        u32 newcap = *cap * 2;
        if (!newcap) { newcap = 1; }

        /* resize 'newcap' until it's greater than 'cnt' + 'siz' */
        while (newcap < *cnt + siz) {
            newcap *= 2;
        }

        /* alloc. 'tmp' */
        u32 *tmp = (u32 *) realloc(*arr, newcap * sizeof(u32));
        if (!tmp) {
            return (0);
        }

        /* return 'tmp' and 'newcap' */
        *arr = tmp;
        *cap = newcap;
    }

    /* concat. 'dat' to 'arr' */
    memcpy(*arr + *cnt, dat, siz * sizeof(u32));
    *cnt += siz;

    /* success */
    return (1);
}


static int __ft_chunk_data_alloc(struct chunk_data_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* clear 'self' */
    memset(self, 0, sizeof(struct chunk_data_s));

    /* alloc. 'self' */
    self->v_solid.cnt = 0;
    self->v_solid.cap = 1;
    self->v_solid.arr = (u32 *) calloc(self->v_solid.cap, sizeof(u32));

    self->i_solid.cnt = 0;
    self->i_solid.cap = 1;
    self->i_solid.arr = (u32 *) calloc(self->i_solid.cap, sizeof(u32));

    self->v_trans.cnt = 0;
    self->v_trans.cap = 1;
    self->v_trans.arr = (u32 *) calloc(self->v_trans.cap, sizeof(u32));

    self->i_trans.cnt = 0;
    self->i_trans.cap = 1;
    self->i_trans.arr = (u32 *) calloc(self->i_trans.cap, sizeof(u32));

    /* success */
    return (1);
}


static int __ft_chunk_data_free(struct chunk_data_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* release 'self' */
    if (self->v_solid.arr) {
        free(self->v_solid.arr);
        self->v_solid.arr = 0;
        self->v_solid.cnt = 0;
        self->v_solid.cap = 0;
    }

    if (self->i_solid.arr) {
        free(self->i_solid.arr);
        self->i_solid.arr = 0;
        self->i_solid.cnt = 0;
        self->i_solid.cap = 0;
    }

    if (self->v_trans.arr) {
        free(self->v_trans.arr);
        self->v_trans.arr = 0;
        self->v_trans.cnt = 0;
        self->v_trans.cap = 0;
    }

    if (self->i_trans.arr) {
        free(self->i_trans.arr);
        self->i_trans.arr = 0;
        self->i_trans.cnt = 0;
        self->i_trans.cap = 0;
    }

    /* success */
    return (1);
}


static int __ft_chunk_mesh_alloc(struct chunk_mesh_s *self) {
    /* null-check */
    if (!self) { return (0); }
    
    /* clear 'self' */
    memset(self, 0, sizeof(struct chunk_mesh_s));

    /* create 'vbo's */
    glCreateBuffers(2, self->vbo);
    
    /* create 'ibo's */
    glCreateBuffers(2, self->ibo);

    /* create 'vao's */
    glCreateVertexArrays(2, self->vao);
    
    /* config. 'vao's */
    for (size_t i = 0; i < countof(self->vao); i++) {
        /* 'vao' layout */
        glEnableVertexArrayAttrib( self->vao[i], 0);
        glVertexArrayAttribBinding(self->vao[i], 0, 0);
        glVertexArrayAttribIFormat(self->vao[i], 0, 1, GL_UNSIGNED_INT, 0);

        /* bind 'vbo' and 'ibo' */
        glVertexArrayVertexBuffer(self->vao[i], 0,
                                  self->vbo[i], 0,
                                  sizeof(u32));

        glVertexArrayElementBuffer(self->vao[i],
                                   self->ibo[i]);
    }

    /* success */
    return (1);
}


static int __ft_chunk_mesh_free(struct chunk_mesh_s *self) {
    /* null-check */
    if (!self) { return (0); }
    
    /* release 'self' */
    glDeleteVertexArrays(2, self->vao);
    glDeleteBuffers(2, self->vbo);
    glDeleteBuffers(2, self->ibo);
    
    /* clear 'self' */
    memset(self, 0, sizeof(struct chunk_mesh_s));

    /* success */
    return (1);
}


static int __ft_chunk_mesh_render_s(struct chunk_mesh_s *self, struct chunk_s *chunk, struct shader_s *shader) {
    /* null-check */
    if (!self)   { return (0); }
    if (!chunk)  { return (0); }
    if (!shader) { return (0); }
    
    /* 'u_model' matrix */
    mat4 u_model = alg_mat4translate(chunk->offset);

    /* set projection matrices */
    ft_shader_uniform(shader, "u_proj", FT_UNIFORM_MATRIX4, &STATE.proj);
    ft_shader_uniform(shader, "u_view", FT_UNIFORM_MATRIX4, &STATE.view);
    ft_shader_uniform(shader, "u_model", FT_UNIFORM_MATRIX4, &u_model);

    /* bind 'vao' */
    glBindVertexArray(self->vao[0]);

    /* draw-call */
    glDrawElements(GL_TRIANGLES, self->count[0], GL_UNSIGNED_INT, (GLvoid *) (0));

    /* success */
    return (1);
}


static int __ft_chunk_mesh_render_t(struct chunk_mesh_s *self, struct chunk_s *chunk, struct shader_s *shader) {
    /* null-check */
    if (!self)   { return (0); }
    if (!chunk)  { return (0); }
    if (!shader) { return (0); }
    
    /* 'u_model' matrix */
    mat4 u_model = alg_mat4translate(chunk->offset);

    /* set projection matrices */
    ft_shader_uniform(shader, "u_proj", FT_UNIFORM_MATRIX4, &STATE.proj);
    ft_shader_uniform(shader, "u_view", FT_UNIFORM_MATRIX4, &STATE.view);
    ft_shader_uniform(shader, "u_model", FT_UNIFORM_MATRIX4, &u_model);

    /* bind 'vao' */
    glBindVertexArray(self->vao[1]);

    /* draw-call */
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawElements(GL_TRIANGLES, self->count[1], GL_UNSIGNED_INT, (GLvoid *) (0));
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

    /* success */
    return (1);
}


static int __ft_chunk_generate(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* run 'generate' algorithm */
    if (!ft_worldgen_generate(self->parent, self, self->offset.x, self->offset.y, self->offset.z)) { return (0); }

    /* success */
    SDL_AtomicSet(&self->atoms.has_blocks, 1);
    SDL_AtomicSet(&self->atoms.is_generated, 1);
    return (1);
}


static int __ft_chunk_mesh_bake(struct chunk_data_s *self, struct chunk_s *chunk) {
    /* null-check */
    if (!self)  { return (0); }
    if (!chunk) { return (0); }

    /* offsets */
    u32 s_off = 0;
    u32 t_off = 0;

    /* iterate on every 'block' */
    self->v_solid.cnt = self->i_solid.cnt = 0;
    self->v_trans.cnt = self->i_trans.cnt = 0;
    for (int y = 0; y < FT_VOX_CHUNK_SIZE_Y; y++) {
    for (int x = 0; x < FT_VOX_CHUNK_SIZE_X; x++) {
    for (int z = 0; z < FT_VOX_CHUNK_SIZE_Z; z++) {
        u32 idx = 0;
        ft_chunk_get(chunk, x, y, z, &idx);
        if (!idx || idx >= FT_BLOCK_COUNT) { continue; }
        
        /* get 'near' blocks */
        u32 near[6] = { 0 };
        ft_chunk_get_near(chunk, x, y, z, near);

        /* 'idx'-to-'block' match */
        struct block_s block = BLOCKS[idx];
        switch (block.type) {
            case (FT_BLOCK_TYPE_TRANS): {
                /* iterate on every 'near' */
                for (int face = 0; face < FT_FACE_COUNT; face++) {
                    /* 'near'-to-'block' match */
                    struct block_s block = BLOCKS[near[face]];
                    if (block.id != FT_BLOCK_AIR) {
                        continue;
                    }
                
                    /* get 'vertices' and 'indices' */
                    u32 vertices[4] = { 0 },
                         indices[6] = { 0 };
                    __ft_face_cube(idx, face, x, y, z, vertices, indices, &t_off);

                    /* push 'vertices' */
                    __ft_push(&self->v_trans.arr,
                              &self->v_trans.cnt,
                              &self->v_trans.cap,
                              vertices, countof(vertices));

                    /* push 'indices' */
                    __ft_push(&self->i_trans.arr,
                              &self->i_trans.cnt,
                              &self->i_trans.cap,
                              indices, countof(indices));
                }
            } break;

            case (FT_BLOCK_TYPE_SOLID): {
                /* iterate on every 'near' */
                for (int face = 0; face < FT_FACE_COUNT; face++) {
                    if (near[face] >= FT_BLOCK_COUNT) { continue; }

                    /* 'near'-to-'block' match */
                    struct block_s block = BLOCKS[near[face]];
                    if (block.type != FT_BLOCK_TYPE_TRANS) { continue; }
                
                    /* get 'vertices' and 'indices' */
                    u32 vertices[4] = { 0 },
                         indices[6] = { 0 };
                    __ft_face_cube(idx, face, x, y, z, vertices, indices, &s_off);

                    /* push 'vertices' */
                    __ft_push(&self->v_solid.arr,
                              &self->v_solid.cnt,
                              &self->v_solid.cap,
                              vertices, countof(vertices));

                    /* push 'indices' */
                    __ft_push(&self->i_solid.arr,
                              &self->i_solid.cnt,
                              &self->i_solid.cap,
                              indices, countof(indices));
                }
            } break;
            
            default: { }
        }
    }}}

    /* success */
    SDL_AtomicSet(&chunk->atoms.is_meshed, 1);
    return (1);
}


static int __ft_chunk_mesh_upload(struct chunk_mesh_s *self, struct chunk_data_s *data, struct chunk_s *chunk) {
    /* null-check */
    if (!self)  { return (0); }
    if (!data)  { return (0); }
    if (!chunk) { return (0); }

    /* solid */
    if (data->v_solid.cnt) {
        glNamedBufferData(self->vbo[0], data->v_solid.cnt * sizeof(u32), data->v_solid.arr, GL_DYNAMIC_DRAW);
        glNamedBufferData(self->ibo[0], data->i_solid.cnt * sizeof(u32), data->i_solid.arr, GL_DYNAMIC_DRAW);
    }

    /* unconditional 'count's */
    self->count[0] = data->i_solid.cnt;
    
    /* trans */
    if (data->v_trans.cnt) {
        glNamedBufferData(self->vbo[1], data->v_trans.cnt * sizeof(u32), data->v_trans.arr, GL_DYNAMIC_DRAW);
        glNamedBufferData(self->ibo[1], data->i_trans.cnt * sizeof(u32), data->i_trans.arr, GL_DYNAMIC_DRAW);
    }

    /* unconditional 'count's */
    self->count[1] = data->i_trans.cnt;

    /* release 'data' */
    __ft_chunk_data_free(data);

    /* success */
    SDL_AtomicSet(&chunk->atoms.is_uploaded, 1);
    return (1);
}
