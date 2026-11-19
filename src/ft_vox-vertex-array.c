#include "ft_vox.h"

extern int ft_vao_create(struct vao_s **self) {
    /* null-check */
    if (!self) { return (0); }

    /* alloc 'vao' */
    struct vao_s *vao = (struct vao_s *) malloc(sizeof(struct vao_s));
    if (!vao) { return (0); }

    /* create 'vao's 'id' */
    glCreateVertexArrays(1, &vao->id);
   
    /* return 'self' */
    *self = vao;

    /* success */
    return (1);
}


extern int ft_vao_delete(struct vao_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* release resources */
    glDeleteVertexArrays(1, &self->id);

    /* release 'self' */
    free(self);
    
    /* success */
    return (1);
}


extern int ft_vao_layoutf(struct vao_s *self, const size_t attrib, const size_t count, const size_t offset) {
    /* null-check */
    if (!self) { return (0); }

    /* set new 'layout' */
    glEnableVertexArrayAttrib(self->id, attrib);
    glVertexArrayAttribBinding(self->id, attrib, 0);
    glVertexArrayAttribFormat(self->id, attrib, count, GL_FLOAT, GL_FALSE, offset);
    
    /* success */
    return (1);
}


extern int ft_vao_layoutd(struct vao_s *self, const size_t attrib, const size_t count, const size_t offset) {
    /* null-check */
    if (!self) { return (0); }

    /* set new 'layout' */
    glEnableVertexArrayAttrib(self->id, attrib);
    glVertexArrayAttribBinding(self->id, attrib, 0);
    glVertexArrayAttribFormat(self->id, attrib, count, GL_DOUBLE, GL_FALSE, offset);
    
    /* success */
    return (1);
}


extern int ft_vao_layoutb(struct vao_s *self, const size_t attrib, const size_t count, const size_t offset) {
    /* null-check */
    if (!self) { return (0); }

    /* set new 'layout' */
    glEnableVertexArrayAttrib(self->id, attrib);
    glVertexArrayAttribBinding(self->id, attrib, 0);
    glVertexArrayAttribIFormat(self->id, attrib, count, GL_UNSIGNED_BYTE, offset);
    
    /* success */
    return (1);
}


extern int ft_vao_layouts(struct vao_s *self, const size_t attrib, const size_t count, const size_t offset) {
    /* null-check */
    if (!self) { return (0); }

    /* set new 'layout' */
    glEnableVertexArrayAttrib(self->id, attrib);
    glVertexArrayAttribBinding(self->id, attrib, 0);
    glVertexArrayAttribIFormat(self->id, attrib, count, GL_UNSIGNED_SHORT, offset);
    
    /* success */
    return (1);
}


extern int ft_vao_layouti(struct vao_s *self, const size_t attrib, const size_t count, const size_t offset) {
    /* null-check */
    if (!self) { return (0); }

    /* set new 'layout' */
    glEnableVertexArrayAttrib(self->id, attrib);
    glVertexArrayAttribBinding(self->id, attrib, 0);
    glVertexArrayAttribIFormat(self->id, attrib, count, GL_UNSIGNED_INT, offset);
    
    /* success */
    return (1);
}


extern int ft_vao_layoutl(struct vao_s *self, const size_t attrib, const size_t count, const size_t offset) {
    /* null-check */
    if (!self) { return (0); }

    /* set new 'layout' */
    glEnableVertexArrayAttrib(self->id, attrib);
    glVertexArrayAttribBinding(self->id, attrib, 0);
    glVertexArrayAttribLFormat(self->id, attrib, count, GL_UNSIGNED_INT, offset);
    
    /* success */
    return (1);
}


extern int ft_vao_vbo(struct vao_s *self, const GLuint vbo) {
    /* null-check */
    if (!self) { return (0); }

    /* set 'vbo' buffer */
    glVertexArrayVertexBuffer(self->id, 0, vbo, 0, sizeof(GLuint));
    
    /* success */
    return (1);
}


extern int ft_vao_ibo(struct vao_s *self, const GLuint ibo) {
    /* null-check */
    if (!self) { return (0); }

    /* set 'ibo' buffer */
    glVertexArrayElementBuffer(self->id, ibo);
    
    /* success */
    return (1);
}
