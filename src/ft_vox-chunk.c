#include "ft_vox.h"

/* static API */

static u32 __ft_vertex(u8, u8, u8, u8, u8, u8);

static int __ft_face_cube(u32, int, int, int, int, u32 *, u32 *, u32);

static int __ft_push(u32 **, u32 *, u32 *, u32 *, u32);

static int __ft_chunk_blocks_alloc(u32 **);

static int __ft_chunk_block_free(u32 *);

static int __ft_chunk_data_alloc(struct chunk_data_s *);

static int __ft_chunk_data_free(struct chunk_data_s *);

static int __ft_chunk_mesh_alloc(struct chunk_mesh_s *);

static int __ft_chunk_mesh_free(struct chunk_mesh_s *);

static int __ft_chunk_mesh_render_t(struct chunk_mesh_s *, struct chunk_s *);

static int __ft_chunk_mesh_render_s(struct chunk_mesh_s *, struct chunk_s *);

static int __ft_chunk_mesh_render_w(struct chunk_mesh_s *, struct chunk_s *);

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
    SDL_AtomicSet(&chunk->atoms.create, 1);

    /* return 'self' */
    *self = chunk;

    /* success */
    return (1);
}


extern int ft_chunk_delete(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); } 

    /* release 'mesh' */
    __ft_chunk_mesh_free(&self->mesh);

    /* release 'data' */
    __ft_chunk_data_free(&self->data);

    /* release 'blocks' */
    __ft_chunk_block_free(self->blocks);
    
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

    /* case : 'delete' */
    if (SDL_AtomicGet(&self->atoms.delete)) {
        /* skip 'update' */
        return (1);
    }
    
    /* get 'near' chunks */
    struct
    chunk_s *near[6] = { 0 };
    ft_world_get_near(self->parent, self->origin.x,
                                    self->origin.y,
                                    self->origin.z, near);

    /* case : 'create' */
    if (SDL_AtomicCAS(&self->atoms.create, 1, 0)) {
        SDL_AtomicAdd(&self->atoms.refer, 1);
        if (!ft_thpool_queue(pool, self, (thread_pool_func_t) ft_chunk_generate)) {
            SDL_AtomicAdd(&self->atoms.refer, -1);
            SDL_AtomicSet(&self->atoms.create, 1);
        }
    }

    /* case : 'dirty' */
    /* case : 'generate' */
    if (SDL_AtomicGet(&self->atoms.dirty) ||
        SDL_AtomicGet(&self->atoms.generate)
    ) {
        /* refer-check */
        if (SDL_AtomicGet(&self->atoms.refer) == 0) {
            /* 'mesh' start condition */
            u8 mesh = 1;
            for (int i = 0; i < countof(near); i++) {
                /* null-check */
                if (!near[i]) { mesh = 0; break; }

                /* atom-check */
                if (!SDL_AtomicGet(&near[i]->atoms.populated)) { mesh = 0; break; }
            }
            
            if (mesh) {
                SDL_AtomicSet(&self->atoms.dirty,    0);
                SDL_AtomicSet(&self->atoms.generate, 0);
                SDL_AtomicAdd(&self->atoms.refer, 1);
                if (!ft_thpool_queue(pool, self, (thread_pool_func_t) ft_chunk_mesh_bake)) {
                    SDL_AtomicAdd(&self->atoms.refer, -1);
                    SDL_AtomicSet(&self->atoms.generate, 1);
                }
            }
        }
    }

    /* case : 'meshed' */
    if (SDL_AtomicCAS(&self->atoms.meshed, 1, 0)) {
        SDL_AtomicAdd(&self->atoms.refer, 1);
        if (!ft_chunk_mesh_upload(self)) {
            SDL_AtomicAdd(&self->atoms.refer, -1);
            SDL_AtomicSet(&self->atoms.meshed, 1);
        }
    }

    /* success */
    return (1);
}


extern int ft_chunk_render(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* atom-check */
    if (SDL_AtomicGet(&self->atoms.upload)) {
        /* get 'mesh' */
        struct
        chunk_mesh_s *mesh = &self->mesh;
        
        /* render 'trans' */
        if (mesh->count[FT_BLOCK_TYPE_TRANS] > 0) {
            __ft_chunk_mesh_render_t(mesh, self);
        }

        /* render 'solid' */
        if (mesh->count[FT_BLOCK_TYPE_SOLID] > 0) {
            __ft_chunk_mesh_render_s(mesh, self);
        }

        /* render 'water' */
        if (mesh->count[FT_BLOCK_TYPE_WATER] > 0) {
            __ft_chunk_mesh_render_w(mesh, self);
        }
    }

    /* success */
    return (1);
}


extern int ft_chunk_alloc(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* create 'blocks' */
    if (!__ft_chunk_blocks_alloc(&self->blocks)) {
        ft_chunk_delete(self);
        return (0);
    }

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
    if (self->blocks) { block = self->blocks[FT_VOX_CHUNK_3D_1D(bx, by, bz)]; }

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

    bx = alg_clamp(bx, 0, FT_VOX_CHUNK_SIZE_X - 1),
    by = alg_clamp(by, 0, FT_VOX_CHUNK_SIZE_Y - 1),
    bz = alg_clamp(bz, 0, FT_VOX_CHUNK_SIZE_Z - 1);

    /* set 'id' */
    if (self->blocks) { self->blocks[FT_VOX_CHUNK_3D_1D(bx, by, bz)] = id; }

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


static int __ft_face_cube(u32 id, int face, int x, int y, int z, u32 *v, u32 *i, u32 off) {
    /* get 'block' */
    struct block_s block = BLOCKS[id];

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
    i[0] = off + 0;
    i[1] = off + 1;
    i[2] = off + 2;
    i[3] = off + 0;
    i[4] = off + 2;
    i[5] = off + 3;

    /* success */
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


static int __ft_chunk_blocks_alloc(u32 **self) {
    if (!self) { return (0); }

    /* alloc. 'blocks' */
    u32 *blocks = (u32 *) calloc(FT_VOX_CHUNK_SIZE, sizeof(u32));
    if (!blocks) { return (0); }

    /* return 'self' */
    *self = blocks;

    /* success */
    return (1);
}


static int __ft_chunk_block_free(u32 *self) {
    if (!self) { return (0); }

    /* release 'self' */
    free(self);

    /* success */
    return (1);
}


static int __ft_chunk_data_alloc(struct chunk_data_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* clear 'self' */
    memset(self, 0, sizeof(struct chunk_data_s));

    /* alloc. 'self' */
    for (int i = 0; i < FT_BLOCK_TYPE_COUNT; i++) {
        self->v[i].cnt = self->i[i].cnt = 0;
        self->v[i].cap = self->i[i].cap = 1;
        self->v[i].arr = (u32 *) calloc(self->v[i].cap, sizeof(u32));
        self->i[i].arr = (u32 *) calloc(self->i[i].cap, sizeof(u32));
    }

    /* success */
    return (1);
}


static int __ft_chunk_data_free(struct chunk_data_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* release 'self' */
    for (int i = 0; i < FT_BLOCK_TYPE_COUNT; i++) {
        self->v[i].cnt = self->i[i].cnt = 0;
        self->v[i].cap = self->i[i].cap = 0;
    
        free(self->v[i].arr);
        self->v[i].arr = 0;
    
        free(self->i[i].arr);
        self->i[i].arr = 0; 
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
    glCreateBuffers(3, self->vbo);
    
    /* create 'ibo's */
    glCreateBuffers(3, self->ibo);

    /* create 'vao's */
    glCreateVertexArrays(3, self->vao);
    
    /* config. 'vao's */
    for (size_t i = 0; i < FT_BLOCK_TYPE_COUNT; i++) {
        /* 'vao' layout */
        glEnableVertexArrayAttrib(self->vao[i], 0);
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
    glDeleteVertexArrays(3, self->vao);
    glDeleteBuffers(3, self->vbo);
    glDeleteBuffers(3, self->ibo);
    
    /* clear 'self' */
    memset(self, 0, sizeof(struct chunk_mesh_s));

    /* success */
    return (1);
}


static int __ft_chunk_mesh_render_t(struct chunk_mesh_s *self, struct chunk_s *chunk) {
    /* null-check */
    if (!self)  { return (0); }
    if (!chunk) { return (0); }

    /* configure OpenGL */
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    
    /* 'u_model' matrix */
    mat4 u_model = alg_mat4translate(chunk->offset);

    /* configure shader */
    ft_shader_bind(STATE.gfx.trans);
    ft_shader_uniform(STATE.gfx.trans, "u_proj", FT_UNIFORM_MATRIX4, &STATE.gfx.proj);
    ft_shader_uniform(STATE.gfx.trans, "u_view", FT_UNIFORM_MATRIX4, &STATE.gfx.view);
    ft_shader_uniform(STATE.gfx.trans, "u_model", FT_UNIFORM_MATRIX4, &u_model);

    /* draw-call */
    glBindVertexArray(self->vao[FT_BLOCK_TYPE_TRANS]);
    glDrawElements(GL_TRIANGLES, self->count[FT_BLOCK_TYPE_TRANS], GL_UNSIGNED_INT, (GLvoid *) (0));

    /* success */
    return (1);
}


static int __ft_chunk_mesh_render_s(struct chunk_mesh_s *self, struct chunk_s *chunk) {
    /* null-check */
    if (!self)  { return (0); }
    if (!chunk) { return (0); }

    /* configure OpenGL */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    
    /* 'u_model' matrix */
    mat4 u_model = alg_mat4translate(chunk->offset);

    /* configure shader */
    ft_shader_bind(STATE.gfx.solid);
    ft_shader_uniform(STATE.gfx.solid, "u_proj", FT_UNIFORM_MATRIX4, &STATE.gfx.proj);
    ft_shader_uniform(STATE.gfx.solid, "u_view", FT_UNIFORM_MATRIX4, &STATE.gfx.view);
    ft_shader_uniform(STATE.gfx.solid, "u_model", FT_UNIFORM_MATRIX4, &u_model);

    /* draw-call */
    glBindVertexArray(self->vao[FT_BLOCK_TYPE_SOLID]);
    glDrawElements(GL_TRIANGLES, self->count[FT_BLOCK_TYPE_SOLID], GL_UNSIGNED_INT, (GLvoid *) (0));

    /* success */
    return (1);
}


static int __ft_chunk_mesh_render_w(struct chunk_mesh_s *self, struct chunk_s *chunk) {
    /* null-check */
    if (!self)  { return (0); }
    if (!chunk) { return (0); }

    /* configure OpenGL */
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    
    /* 'u_model' matrix */
    mat4 u_model = alg_mat4translate(chunk->offset);

    /* configure shader */
    ft_shader_bind(STATE.gfx.water);
    ft_shader_uniform(STATE.gfx.water, "u_proj", FT_UNIFORM_MATRIX4, &STATE.gfx.proj);
    ft_shader_uniform(STATE.gfx.water, "u_view", FT_UNIFORM_MATRIX4, &STATE.gfx.view);
    ft_shader_uniform(STATE.gfx.water, "u_model", FT_UNIFORM_MATRIX4, &u_model);

    /* draw-call */
    glBindVertexArray(self->vao[FT_BLOCK_TYPE_WATER]);
    glDrawElements(GL_TRIANGLES, self->count[FT_BLOCK_TYPE_WATER], GL_UNSIGNED_INT, (GLvoid *) (0));

    /* success */
    return (1);
}


static int __ft_chunk_generate(struct chunk_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* run 'generate' algorithm */
    if (!ft_worldgen_generate(self->parent, self, self->offset.x, self->offset.y, self->offset.z)) { return (0); }

    /* success */
    SDL_AtomicSet(&self->atoms.populated, 1);
    SDL_AtomicSet(&self->atoms.generate, 1);
    return (1);
}


static int __ft_chunk_mesh_bake(struct chunk_data_s *self, struct chunk_s *chunk) {
    /* null-check */
    if (!self)  { return (0); }
    if (!chunk) { return (0); }

    /* prepare 'self' vectors */
    self->v[0].cnt = self->i[0].cnt = 0;
    self->v[1].cnt = self->i[1].cnt = 0;
    self->v[2].cnt = self->i[2].cnt = 0;
    
    /* iterate on every 'block' */
    for (int i = 0; i < FT_VOX_CHUNK_SIZE; i++) {
        /* get 'x', 'y', and 'z' */
        int x = 0,
            y = 0,
            z = 0;
        FT_VOX_CHUNK_1D_3D(i, x, y, z);

        /* get block ID */
        u32 id = 0;
        ft_chunk_get(chunk, x, y, z, &id);
        if (!id || id >= FT_BLOCK_COUNT) { continue; }
        
        /* get near blocks ID's */
        u32 nids[6] = { 0 };
        ft_chunk_get_near(chunk, x, y, z, nids);

        /* handle 'type's */
        enum
        block_type_e type = BLOCKS[id].type;
        switch (type) {

            case (FT_BLOCK_TYPE_TRANS): {
                for (enum face_e face = 0; face < FT_FACE_COUNT; face++) {
                    u32 nid = nids[face];
                    if (nid == id) { continue; }
                    
                    /* get 'vertices' and 'indices' */
                    u32 vertices[4] = { 0 },
                         indices[6] = { 0 };
                    __ft_face_cube(id, face, x, y, z, vertices, indices, self->v[type].cnt);
                    
                    /* push 'vertices' */
                    __ft_push(&self->v[type].arr,
                              &self->v[type].cnt,
                              &self->v[type].cap,
                              vertices, countof(vertices));

                    /* push 'indices' */
                    __ft_push(&self->i[type].arr,
                              &self->i[type].cnt,
                              &self->i[type].cap,
                              indices, countof(indices));
                }

            } break;

            case (FT_BLOCK_TYPE_SOLID): {
                for (enum face_e face = 0; face < FT_FACE_COUNT; face++) {
                    u32 nid = nids[face];
                    if (BLOCKS[nid].type != FT_BLOCK_TYPE_TRANS) { continue; }
                    
                    /* get 'vertices' and 'indices' */
                    u32 vertices[4] = { 0 },
                         indices[6] = { 0 };
                    __ft_face_cube(id, face, x, y, z, vertices, indices, self->v[type].cnt);
                    
                    /* push 'vertices' */
                    __ft_push(&self->v[type].arr,
                              &self->v[type].cnt,
                              &self->v[type].cap,
                              vertices, countof(vertices));

                    /* push 'indices' */
                    __ft_push(&self->i[type].arr,
                              &self->i[type].cnt,
                              &self->i[type].cap,
                              indices, countof(indices));
                }

            } break;

            case (FT_BLOCK_TYPE_WATER): {
                for (enum face_e face = 0; face < FT_FACE_COUNT; face++) {
                    u32 nid = nids[face];
                    if (nid != FT_BLOCK_AIR) { continue; }
                    
                    /* get 'vertices' and 'indices' */
                    u32 vertices[4] = { 0 },
                         indices[6] = { 0 };
                    __ft_face_cube(id, face, x, y, z, vertices, indices, self->v[type].cnt);
                    
                    /* push 'vertices' */
                    __ft_push(&self->v[type].arr,
                              &self->v[type].cnt,
                              &self->v[type].cap,
                              vertices, countof(vertices));

                    /* push 'indices' */
                    __ft_push(&self->i[type].arr,
                              &self->i[type].cnt,
                              &self->i[type].cap,
                              indices, countof(indices));
                }

            } break;

            default: { return (0); }
        }
    }

    /* success */
    SDL_AtomicSet(&chunk->atoms.meshed, 1);
    return (1);
}


static int __ft_chunk_mesh_upload(struct chunk_mesh_s *self, struct chunk_data_s *data, struct chunk_s *chunk) {
    /* null-check */
    if (!self)  { return (0); }
    if (!data)  { return (0); }
    if (!chunk) { return (0); }

    /* CPU-to-GPU */
    for (int i = 0; i < countof(data->v); i++) {
        if (data->v[i].cnt) {
            glNamedBufferData(self->vbo[i], data->v[i].cnt * sizeof(u32), data->v[i].arr, GL_DYNAMIC_DRAW);
            glNamedBufferData(self->ibo[i], data->i[i].cnt * sizeof(u32), data->i[i].arr, GL_DYNAMIC_DRAW);
        }

        /* unconditional 'count's */
        self->count[i] = data->i[i].cnt;
    }

    /* release 'data' */
    __ft_chunk_data_free(data);

    /* success */
    SDL_AtomicSet(&chunk->atoms.upload, 1);
    return (1);
}
