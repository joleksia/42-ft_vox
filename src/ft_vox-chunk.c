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


static int __ftChunkFaceCube(struct GLvertex *pVertices, GLsizei *pVsize, GLuint *pIndices, GLsizei *pIsize, int face, vec3 offset) {
    /* 'pVertices' : send face */
    GLsizei base = *pVsize;
    for (size_t j = 0; j < 4; j++) {
        struct GLvertex vertex = { 0 };
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
    uint32_t *data = self->data;
    if (!data) {
        /* alloc. 'data' */
        data = (uint32_t *) calloc(FT_VOX_CHUNK_SIZE_X *
                                   FT_VOX_CHUNK_SIZE_Y *
                                   FT_VOX_CHUNK_SIZE_Z, sizeof(uint32_t));
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
    uint32_t *data = self->data;
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
    if (1) {
        if (mesh->ibo[0].sz > 0) {
            /* set 'ibo[0]' */
            ftVertexArrayIndexBuffer(pVao, mesh->ibo[0].id);

            /* draw-call */
            glDrawElements(GL_TRIANGLES, mesh->ibo[0].sz, GL_UNSIGNED_INT, (GLvoid *) (0));
        }
    }

    /* success */
    return (1);
}


extern int ftChunkGenerateData(struct Chunk *self, int seed) {
    /* null-check */
    if (!self) { return (0); }

    /* get 'data' */
    uint32_t *data = self->data;
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


extern int ftChunkGenerateMesh(struct Chunk *self){
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
    struct GLvertex *v = (struct GLvertex *) malloc(size * 24 * sizeof(struct GLvertex));
    GLuint *s = (GLuint *) malloc(size * 36 * sizeof(GLuint)),
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
        /* get the 'current' block */
        uint32_t current = 0;
        ftChunkGetBlock(self, alg_vec3(x, y, z), &current);

        /* check if 'current' is 'air' */
        if (!current) { continue; }

        /* check if 'current' is 'solid' */
        else if (current == 1) {
            for (size_t i = 0; i < 6; i++) {
                /* get the 'neighbor' block */
                uint32_t neighbor = 0;
                ftChunkGetBlock(self, alg_vec3(x + FT_VOX_FACE_DIRECTION[i].x,
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

    /* set 'v', 's' and 't' to the 'mesh' */

    mesh->vbo.sz = nv;
    glNamedBufferData(mesh->vbo.id, nv * sizeof(struct GLvertex), v, GL_STATIC_DRAW);

    mesh->ibo[0].sz = ns;
    glNamedBufferData(mesh->ibo[0].id, ns * sizeof(GLuint), s, GL_STATIC_DRAW);

    mesh->ibo[1].sz = nt;
    glNamedBufferData(mesh->ibo[1].id, nt * sizeof(GLuint), t, GL_STATIC_DRAW);

    /* release 'v', 's' and 't' buffers */
    free(v);
    free(s);
    free(t);

    /* set 'dirty' flag */
    self->flags.dirty = 0;

    /* return 'status' */
    return (1);
}


extern int ftChunkGetBlock(struct Chunk *self, const vec3 position, uint32_t *pID) {
    /* null-check */
    if (!self) { return (0); }


    int cx = (int) self->offset.x,
        cy = (int) self->offset.y,
        cz = (int) self->offset.z;

    int x = (int) position.x;
    if (x < 0) {
        cx--; x += FT_VOX_CHUNK_SIZE_X;
    } else if (x >= FT_VOX_CHUNK_SIZE_X) {
        cx++; x -= FT_VOX_CHUNK_SIZE_X;
    }
    
    int y = (int) position.y;
    if (y < 0) {
        cy--; y += FT_VOX_CHUNK_SIZE_Y;
    } else if (y >= FT_VOX_CHUNK_SIZE_Y) {
        cy++; y -= FT_VOX_CHUNK_SIZE_Y;
    }
    
    int z = (int) position.z;
    if (z < 0) {
        cz--; z += FT_VOX_CHUNK_SIZE_Z;
    } else if (z >= FT_VOX_CHUNK_SIZE_Z) {
        cz++; z -= FT_VOX_CHUNK_SIZE_Z;
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
    uint32_t *data = self->data;
    if (!data) { return (0); }
    
    /* return 'ID' */
    uint32_t ID = self->data[FT_VOX_CHUNK_3D_1D(x, y, z)];
    if (pID) { *pID = ID; }

    /* success */
    return (1);
}


extern int ftChunkSetBlock(struct Chunk *self, const vec3 position, const uint32_t ID) {
    /* null-check */
    if (!self)       { return (0); }
    if (!self->data) { return (0); }

    /* try to set 'ID' into CPU 'data' */
    int x = alg_clamp((int) position.x, 0, FT_VOX_CHUNK_SIZE_X - 1),
        y = alg_clamp((int) position.y, 0, FT_VOX_CHUNK_SIZE_Y - 1),
        z = alg_clamp((int) position.z, 0, FT_VOX_CHUNK_SIZE_Z - 1);
    self->data[FT_VOX_CHUNK_3D_1D(x, y, z)] = ID;

    /* success */
    return (1);
}
