#include "ft_vox.h"

extern int ftMeshCreate(Mesh *pMesh) {
    /* null-check */
    if (!pMesh) { return (0); }

    /* create 'pMesh's buffers */
    
    glCreateBuffers(1, &pMesh->vbo);
    if (!glIsBuffer(pMesh->vbo)) { return (0); }

    glCreateBuffers(1, &pMesh->ibo);
    if (!glIsBuffer(pMesh->ibo)) {
        glDeleteBuffers(1, &pMesh->vbo);
        return (0);
    }

    /* success */
    return (1);
}


extern int ftMeshDelete(Mesh *pMesh) {
    /* null-check */
    if (!pMesh) { return (0); }

    /* delete 'pMesh's buffers */

    if (glIsBuffer(pMesh->vbo)) { glDeleteBuffers(1, &pMesh->vbo); }
    if (glIsBuffer(pMesh->ibo)) { glDeleteBuffers(1, &pMesh->ibo); }

    /* success */
    return (1);
}


extern int ftMeshSetData(Mesh *pMesh, const GLvoid *pVert, const GLsizei vsize, const GLvoid *pIdx, const GLsizei isize) {
    /* null-check */
    if (!pMesh) { return (0); }

    /* upload data to 'pMesh's buffers */

    if (glIsBuffer(pMesh->vbo)) {
        glNamedBufferData(pMesh->vbo, vsize, pVert, GL_STATIC_DRAW);
    }
    
    if (glIsBuffer(pMesh->ibo)) {
        glNamedBufferData(pMesh->ibo, isize, pIdx, GL_STATIC_DRAW);
    }

    /* success */
    return (1);
}


extern int ftMeshUpload(Mesh *pMesh, const GLuint vao) {
    /* null-check */
    if (!pMesh) { return (0); }

    /* 'vao'-check */
    if (!glIsVertexArray(vao)) { return (0); }

    /* upload 'pMesh's buffers to 'vao' */

    if (glIsBuffer(pMesh->vbo)) {
        glVertexArrayVertexBuffer(vao, 0, pMesh->vbo, 0, sizeof(GLvertex));
    }
    
    if (glIsBuffer(pMesh->ibo)) {
        glVertexArrayElementBuffer(vao, pMesh->ibo);
    }

    /* success */
    return (1);
}


extern int ftMeshGetSize(Mesh *pMesh, GLsizei *pVsize, GLsizei *pIsize) {
    /* null-check */
    if (!pMesh) { return (0); }

    /* get 'pMesh's buffers info */

    if (glIsBuffer(pMesh->vbo)) {
        if (pVsize) {
            glGetNamedBufferParameteriv(pMesh->vbo, GL_BUFFER_SIZE, pVsize);
        }
    }
    
    if (glIsBuffer(pMesh->ibo)) {
        if (pIsize) {
            glGetNamedBufferParameteriv(pMesh->ibo, GL_BUFFER_SIZE, pIsize);
        }
    }

    /* success */
    return (1);
}
