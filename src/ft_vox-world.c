#include "ft_vox.h"

extern int ftWorldCreate(World *pWorld, const int seed) {
    /* null-check */
    if (!pWorld) { return (0); }

    /* configure default render radius */
    pWorld->rndrad.x = 8;
    pWorld->rndrad.y = 8;
    pWorld->rndrad.z = 8;

    /* create 'chunks' array */
    size_t count = pWorld->rndrad.x * 2 *
                   pWorld->rndrad.y * 2 *
                   pWorld->rndrad.z * 2;
    Chunk **chunks = (Chunk **) calloc(count, sizeof(Chunk *));
    
    /* pre-generate tge 'chunks', prepare 'offset's, etc. */
    size_t i = 0;
    for (int y = -pWorld->rndrad.y; y < (int) pWorld->rndrad.y; y++) {
        for (int x = -pWorld->rndrad.x; x < (int) pWorld->rndrad.x; x++) {
            for (int z = -pWorld->rndrad.z; z < (int) pWorld->rndrad.z; z++) {
                /* alloc. 'chunks[i]' */
                chunks[i] = (Chunk *) calloc(1, sizeof(Chunk));
                
                /* 'chunk's world 'offset' */
                vec3 offset = alg_vec3(x * FT_VOX_CHUNK_BLOCKS_W,
                                       y * FT_VOX_CHUNK_BLOCKS_H,
                                       z * FT_VOX_CHUNK_BLOCKS_D);
               
                /* try to create the current 'chunks[i]' */
                if (!ftChunkCreate(chunks[i], pWorld, offset, seed)) {
                    /* failure releases the resources under 'chunks[i]' */
                    free(chunks[i]);
                }
                i++;
            }
        }
    }

    /* run CPU-side procedural generation */
    for (size_t i = 0; i < count; i++) {
        /* 'chunk' reference */
        Chunk *chunk = chunks[i];
        if (!chunk) { continue; }

        /* CPU-gen. */
        if (!ftChunkGenCPU(chunk)) {
            /* failure releases the resources under 'chunk' */
            free(chunk);
        }
    }

    /* return 'chunks' */
    pWorld->chunks.used = chunks;
    pWorld->chunks.count = count;

    /* success */
    return (1);
}


extern int ftWorldDelete(World *pWorld) {
    /* null-check */
    if (!pWorld) { return (0); }
    
    /* iterate on every 'used' chunk */
    for (size_t i = 0; i < pWorld->chunks.count; i++) {
        Chunk *chunk = pWorld->chunks.used[i];

        /* release 'chunk' */
        ftChunkDelete(chunk);
        free(chunk);
    }

    /* release 'used' */
    free(pWorld->chunks.used);

    /* success */
    return (1);
}


extern int ftWorldUpdate(World *pWorld) {
    /* null-check */
    if (!pWorld) { return (0); }
    
    /* iterate on every 'chunk' */
    for (size_t i = 0; i < pWorld->chunks.count; i++) {
        Chunk *chunk = pWorld->chunks.used[i];

        /* ... */ (void) chunk;
    }

    /* success */
    return (1);
}

extern int ftWorldRender(World *pWorld, const GLuint vao) {
    /* null-check */
    if (!pWorld) { return (0); }

    /* iterate on every 'chunk' */
    for (size_t i = 0; i < pWorld->chunks.count; i++) {
        Chunk *chunk = pWorld->chunks.used[i];

        /* check 'empty' flag */
        if (chunk->flags.empty) { continue; }

        /* check 'dirty' flag */
        if (chunk->flags.dirty) {
            ftChunkGenGPU(chunk);
        }
        
        /* 'mesh' reference */
        BlockMesh *mesh = chunk->mesh;
        glVertexArrayVertexBuffer(vao, 0, mesh->vbo.id, 0, sizeof(GLvertex));

        /* render 'solid' */
        if (1) {
            if (mesh->ibo[0].sz > 0) {
                glVertexArrayElementBuffer(vao, mesh->ibo[0].id);
                glDrawElements(GL_TRIANGLES, mesh->ibo[0].sz, GL_UNSIGNED_INT, (GLvoid *) (0));
            }
        }

        /* render 'transparent' */
        if (0) {
            if (mesh->ibo[1].sz > 0) {
                glVertexArrayElementBuffer(vao, chunk->mesh->ibo[1].id);
                glDrawElements(GL_TRIANGLES, mesh->ibo[1].sz, GL_UNSIGNED_INT, (GLvoid *) (0));
            }
        }
    }

    /* success */
    return (1);
}
