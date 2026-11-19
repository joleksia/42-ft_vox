#include "ft_vox.h"

/* static API */

static const GLenum FT_VOX_SHADER_MAPPING[8] = { GL_VERTEX_SHADER,
                                                 GL_TESS_CONTROL_SHADER,
                                                 GL_TESS_EVALUATION_SHADER,
                                                 GL_GEOMETRY_SHADER,
                                                 GL_FRAGMENT_SHADER,
                                                 GL_COMPUTE_SHADER, };


static GLint __ft_read_file(FILE *file, char **cstr, size_t *size) {
    /* null-check */
    if (!file) { return (GL_INVALID_VALUE); }

    /* the the size of the 'file' */
    fseek(file, 0, SEEK_END);
    size_t fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    /* create null-terminated c-string pointer of size of 'file' */
    char *fcstr = (char *) calloc(fsize + 1, sizeof(char));
    if (!fcstr) { return (0); }

    /* read 'file' to 'cstr' */
    if (fread(fcstr, sizeof(*fcstr), fsize, file) != fsize) {
        /* 'fread' didn't match the 'fsize'; something bad happened */
        free(fcstr); fcstr = 0;
        return (0);
    }

    /* return */
    if (size) { *size = fsize; }
    if (cstr) { *cstr = fcstr; }
    else {
        /* can't return 'cstr'; release 'fcstr' */
        free(fcstr); fcstr = 0;
    }

    /* success */
    return (1);
}

/* public API */

extern int ft_shader_create(struct shader_s **self) {
    /* null-check */
    if (!self) { return (0); }

    /* alloc. 'shader' */
    struct shader_s *shader = (struct shader_s *) malloc(sizeof(struct shader_s));
    if (!shader) { return (0); }

    /* create 'shader's 'id' */
    shader->id = glCreateProgram();

    /* create 'shader's 'pipeline' */
    for (size_t i = FT_SHADER_VERTEX; i < FT_SHADER_TYPE_COUNT; i++) {
        shader->pipeline[i] = glCreateShader(FT_VOX_SHADER_MAPPING[i]);
    }

    /* return 'self' */
    *self = shader;

    /* success */
    return (1);
}

extern int ft_shader_delete(struct shader_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* clear 'pipeline' */
    for (size_t i = FT_SHADER_VERTEX; i < FT_SHADER_TYPE_COUNT; i++) {
        glDeleteShader(self->pipeline[i]), self->pipeline[i] = 0;
    }

    /* clear 'id' */
    glDeleteProgram(self->id), self->id = 0;

    /* release 'self' */
    free(self);

    /* success */
    return (1);
}

extern int ft_shader_load(struct shader_s *self, FILE *file, enum shader_type_e type) {
    /* null-check */
    if (!self) { return (0); }
    if (!file)   { return (0); }

    /* read 'file' */
    GLchar *code = 0;
    __ft_read_file(file, &code, 0);
    if (!code) { return (0); }

    /* compile */
    glShaderSource(self->pipeline[type], 1, (const GLchar * const *) &code, 0);
    glCompileShader(self->pipeline[type]);

    /* 'compile' status */
    GLint compile;
    glGetShaderiv(self->pipeline[type], GL_COMPILE_STATUS, &compile);
    if (compile != GL_TRUE) {
        /* log compilation error */
        GLchar log[1024] = { 0 };
        GLsizei length = 0;
        glGetShaderInfoLog(self->pipeline[type], sizeof(log), &length, log);

        /* print 'log' */
        printf("compile: %s\n", log);
        return (0);
    }

    /* link */
    glAttachShader(self->id, self->pipeline[type]);
    glLinkProgram(self->id);

    /* 'link' status */
    GLint link = 0;
    glGetProgramiv(self->id, GL_LINK_STATUS, &link);
    if (link != GL_TRUE) {
        /* get error 'log' */
        GLchar log[1024] = { 0 };
        GLsizei length = 0;
        glGetProgramInfoLog(self->id, sizeof(log), &length, log);

        /* print 'log' */
        printf("link: %s\n", log);
        return (0);
    }

    /* release 'code' */
    free(code);

    /* success */
    return (1);
}

extern int ft_shader_uniform(struct shader_s *self, const char *name, enum uniform_type_e type, const void *data) {
    /* null-check */
    if (!self) { return (0); }
    if (!name) { return (0); }
    if (!data) { return (0); }

    /* set 'type' uniform */
    switch (type) {
        default: { return (0); }

        /* 'type' : 'FT_UNIFORM_MATRIX4X4' */
        case (FT_UNIFORM_MATRIX4X4): {
            /* get uniform 'location' */
            GLint location = glGetUniformLocation(self->id, name);
            if (location == -1) {
                return (0);
            }

            /* set uniform 'data' */
            glProgramUniformMatrix4fv(self->id, location, 1, GL_FALSE, data);
        } break;
    }

    /* success */
    return (1);
}
