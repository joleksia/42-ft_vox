#include "ft_vox.h"

extern int ftGPUMeshCreate(GPUMesh *pMesh) {
    /* null-check */
    if (!pMesh) { return (0); }

    /* create 'vbo' and 'ibo' */
    GLuint vbo = 0,
           ibo = 0;
    glCreateBuffers(1, &vbo);
    glCreateBuffers(1, &ibo);

    /* return 'vbo' and 'ibo' */
    memset(pMesh, 0, sizeof(GPUMesh));
    pMesh->vbo.id = vbo;
    pMesh->ibo.id = ibo;

    /* success */
    return (1);
}


extern int ftGPUMeshDelete(GPUMesh *pMesh) {
    /* null-check */
    if (!pMesh) { return (0); }

    /* delete 'pMesh's buffers */

    if (glIsBuffer(pMesh->vbo.id)) {
        glDeleteBuffers(0, &pMesh->vbo.id);
        pMesh->vbo.id = 0;
        pMesh->vbo.size = 0;
    }
    
    if (glIsBuffer(pMesh->ibo.id)) {
        glDeleteBuffers(0, &pMesh->ibo.id);
        pMesh->ibo.id = 0;
        pMesh->ibo.size = 0;
    }

    /* success */
    return (1);
}


extern int ftGPUMeshSetData(GPUMesh *pMesh, const GLvoid *pVertices, const GLsizei vsize, const GLvoid *pIndices, const GLsizei isize) {
    /* null-check */
    if (!pMesh) { return (0); }

    /* upload data to 'pMesh's buffers */

    if (glIsBuffer(pMesh->vbo.id)) {
        pMesh->vbo.size = vsize;
        glNamedBufferData(pMesh->vbo.id,
                          pMesh->vbo.size,
                          pVertices, GL_STATIC_DRAW);
    }
    
    if (glIsBuffer(pMesh->ibo.id)) {
        pMesh->ibo.size = isize;
        glNamedBufferData(pMesh->ibo.id,
                          pMesh->ibo.size,
                          pIndices, GL_STATIC_DRAW);
    }

    /* success */
    return (1);
}


extern int ftGPUMeshUpload(GPUMesh *pMesh, const GLuint vao) {
    /* null-check */
    if (!pMesh) { return (0); }

    /* 'vao'-check */
    if (!glIsVertexArray(vao)) { return (0); }

    /* upload 'pMesh's buffers to 'vao' */

    if (glIsBuffer(pMesh->vbo.id)) {
        glVertexArrayVertexBuffer(vao, 0, pMesh->vbo.id, 0, sizeof(GLvertex));
    }
    
    if (glIsBuffer(pMesh->ibo.id)) {
        glVertexArrayElementBuffer(vao, pMesh->ibo.id);
    }

    /* success */
    return (1);
}
