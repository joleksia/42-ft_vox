#include "ft_vox.h"

extern int ftVertexArrayCreate(struct VertexArray **self) {
    /* null-check */
    if (!self) { return (0); }

    /* alloc 'vao' */
    struct VertexArray *vao = (struct VertexArray *) malloc(sizeof(struct VertexArray));
    if (!vao) { return (0); }

    /* create 'vao's 'ID' */
    glCreateVertexArrays(1, &vao->ID);
   
    /* return 'self' */
    *self = vao;

    /* success */
    return (1);
}


extern int ftVertexArrayDelete(struct VertexArray *self) {
    /* null-check */
    if (!self) { return (0); }

    /* release resources */
    glDeleteVertexArrays(1, &self->ID);

    /* release 'self' */
    free(self);
    
    /* success */
    return (1);
}


extern int ftVertexArrayLayoutFloat(struct VertexArray *self, const size_t attrib, const size_t count, const size_t offset) {
    /* null-check */
    if (!self) { return (0); }

    /* set new 'layout' */
    glEnableVertexArrayAttrib(self->ID, attrib);
    glVertexArrayAttribBinding(self->ID, attrib, 0);
    glVertexArrayAttribFormat(self->ID, attrib, count, GL_FLOAT, GL_FALSE, offset);
    
    /* success */
    return (1);
}


extern int ftVertexArrayLayoutDouble(struct VertexArray *self, const size_t attrib, const size_t count, const size_t offset) {
    /* null-check */
    if (!self) { return (0); }

    /* set new 'layout' */
    glEnableVertexArrayAttrib(self->ID, attrib);
    glVertexArrayAttribBinding(self->ID, attrib, 0);
    glVertexArrayAttribFormat(self->ID, attrib, count, GL_DOUBLE, GL_FALSE, offset);
    
    /* success */
    return (1);
}


extern int ftVertexArrayLayoutByte(struct VertexArray *self, const size_t attrib, const size_t count, const size_t offset) {
    /* null-check */
    if (!self) { return (0); }

    /* set new 'layout' */
    glEnableVertexArrayAttrib(self->ID, attrib);
    glVertexArrayAttribBinding(self->ID, attrib, 0);
    glVertexArrayAttribIFormat(self->ID, attrib, count, GL_UNSIGNED_BYTE, offset);
    
    /* success */
    return (1);
}


extern int ftVertexArrayLayoutShort(struct VertexArray *self, const size_t attrib, const size_t count, const size_t offset) {
    /* null-check */
    if (!self) { return (0); }

    /* set new 'layout' */
    glEnableVertexArrayAttrib(self->ID, attrib);
    glVertexArrayAttribBinding(self->ID, attrib, 0);
    glVertexArrayAttribIFormat(self->ID, attrib, count, GL_UNSIGNED_SHORT, offset);
    
    /* success */
    return (1);
}


extern int ftVertexArrayLayoutInt(struct VertexArray *self, const size_t attrib, const size_t count, const size_t offset) {
    /* null-check */
    if (!self) { return (0); }

    /* set new 'layout' */
    glEnableVertexArrayAttrib(self->ID, attrib);
    glVertexArrayAttribBinding(self->ID, attrib, 0);
    glVertexArrayAttribIFormat(self->ID, attrib, count, GL_UNSIGNED_INT, offset);
    
    /* success */
    return (1);
}


extern int ftVertexArrayLayoutLong(struct VertexArray *self, const size_t attrib, const size_t count, const size_t offset) {
    /* null-check */
    if (!self) { return (0); }

    /* set new 'layout' */
    glEnableVertexArrayAttrib(self->ID, attrib);
    glVertexArrayAttribBinding(self->ID, attrib, 0);
    glVertexArrayAttribLFormat(self->ID, attrib, count, GL_UNSIGNED_INT, offset);
    
    /* success */
    return (1);
}


extern int ftVertexArrayVertexBuffer(struct VertexArray *self, const GLuint vbo) {
    /* null-check */
    if (!self) { return (0); }

    /* set 'vbo' buffer */
    glVertexArrayVertexBuffer(self->ID, 0, vbo, 0, sizeof(GLuint));
    
    /* success */
    return (1);
}


extern int ftVertexArrayIndexBuffer(struct VertexArray *self, const GLuint ibo) {
    /* null-check */
    if (!self) { return (0); }

    /* set 'ibo' buffer */
    glVertexArrayElementBuffer(self->ID, ibo);
    
    /* success */
    return (1);
}
