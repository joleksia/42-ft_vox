#include "ft_vox.h"

extern int ftVertexArrayCreate(VertexArray **pVao) {
    /* null-check */
    if (!pVao) { return (0); }

    /* alloc 'vao' */
    VertexArray *vao = (VertexArray *) malloc(sizeof(VertexArray));
    if (!vao) { return (0); }

    /* create 'vao's 'ID' */
    glCreateVertexArrays(1, &vao->ID);
   
    /* return 'pVao' */
    *pVao = vao;

    /* success */
    return (1);
}


extern int ftVertexArrayDelete(VertexArray *pVao) {
    /* null-check */
    if (!pVao) { return (0); }

    /* 'ID'-check */
    if (glIsVertexArray(pVao->ID) != GL_TRUE) { return (0); }

    /* release resources */
    glDeleteVertexArrays(1, &pVao->ID);

    /* release 'pVao' */
    free(pVao);
    
    /* success */
    return (1);
}


extern int ftVertexArrayLayout(VertexArray *pVao, const size_t attrib, const size_t count, const size_t offset) {
    /* null-check */
    if (!pVao) { return (0); }

    /* 'ID'-check */
    if (glIsVertexArray(pVao->ID) != GL_TRUE) { return (0); }

    /* set new 'layout' */
    glEnableVertexArrayAttrib(pVao->ID, attrib);
    glVertexArrayAttribBinding(pVao->ID, attrib, 0);
    glVertexArrayAttribFormat(pVao->ID, attrib, count, GL_FLOAT, GL_FALSE, offset);
    
    /* success */
    return (1);
}


extern int ftVertexArrayVertexBuffer(VertexArray *pVao, const GLuint vbo) {
    /* null-check */
    if (!pVao) { return (0); }

    /* 'ID'-check */
    if (glIsVertexArray(pVao->ID) != GL_TRUE) { return (0); }

    /* 'vbo'-check */
    if (glIsBuffer(vbo) != GL_TRUE) { return (0); }

    /* set 'vbo' buffer */
    glVertexArrayVertexBuffer(pVao->ID, 0, vbo, 0, sizeof(GLvertex));
    
    /* success */
    return (1);
}


extern int ftVertexArrayIndexBuffer(VertexArray *pVao, const GLuint ibo) {
    /* null-check */
    if (!pVao) { return (0); }

    /* 'ID'-check */
    if (glIsVertexArray(pVao->ID) != GL_TRUE) { return (0); }

    /* 'ibo'-check */
    if (glIsBuffer(ibo) != GL_TRUE) { return (0); }

    /* set 'ibo' buffer */
    glVertexArrayElementBuffer(pVao->ID, ibo);
    
    /* success */
    return (1);
}
