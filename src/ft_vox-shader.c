#include "ft_vox.h"

/* static API */

static const GLenum FT_VOX_SHADER_MAPPING[8] = { GL_VERTEX_SHADER,
                                                 GL_TESS_CONTROL_SHADER,
                                                 GL_TESS_EVALUATION_SHADER,
                                                 GL_GEOMETRY_SHADER,
                                                 GL_FRAGMENT_SHADER,
                                                 GL_COMPUTE_SHADER, };

static GLint __ftReadFile(FILE *, char **, size_t *);

/* public API */

extern int ftShaderCreate(struct Shader **self) {
    /* null-check */
    if (!self) { return (0); }

    /* alloc. 'shader' */
    struct Shader *shader = (struct Shader *) malloc(sizeof(struct Shader));
    if (!shader) { return (0); }

    /* create 'shader's 'program' */
    shader->program = glCreateProgram();

    /* create 'shader's 'pipeline' */
    for (size_t i = FT_SHADER_VERTEX; i < FT_SHADER_TYPE_COUNT; i++) {
        shader->pipeline[i] = glCreateShader(FT_VOX_SHADER_MAPPING[i]);
    }

    /* return 'self' */
    *self = shader;

    /* success */
    return (1);
}

extern int ftShaderDelete(struct Shader *self) {
    /* null-check */
    if (!self) { return (0); }

    /* clear 'pipeline' */
    for (size_t i = FT_SHADER_VERTEX; i < FT_SHADER_TYPE_COUNT; i++) {
        glDeleteShader(self->pipeline[i]), self->pipeline[i] = 0;
    }

    /* clear 'program' */
    glDeleteProgram(self->program), self->program = 0;

    /* release 'self' */
    free(self);

    /* success */
    return (1);
}

extern int ftShaderLoadGLSL(struct Shader *self, FILE *pFile, enum ShaderType type) {
    /* null-check */
    if (!self) { return (0); }
    if (!pFile)   { return (0); }

    /* read 'pFile' */
    GLchar *code = 0;
    __ftReadFile(pFile, &code, 0);
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
    glAttachShader(self->program, self->pipeline[type]);
    glLinkProgram(self->program);

    /* 'link' status */
    GLint link = 0;
    glGetProgramiv(self->program, GL_LINK_STATUS, &link);
    if (link != GL_TRUE) {
        /* get error 'log' */
        GLchar log[1024] = { 0 };
        GLsizei length = 0;
        glGetProgramInfoLog(self->program, sizeof(log), &length, log);

        /* print 'log' */
        printf("link: %s\n", log);
        return (0);
    }

    /* release 'code' */
    free(code);

    /* success */
    return (1);
}

extern int ftShaderUniformSet(struct Shader *self, const char *pName, enum ShaderUniformType type, const void *pData) {
    /* null-check */
    if (!self) { return (0); }
    if (!pName)   { return (0); }
    if (!pData)   { return (0); }

    /* set 'type' uniform */
    switch (type) {
        default: { return (0); }

        /* 'type' : 'FT_UNIFORM_MATRIX4X4' */
        case (FT_UNIFORM_MATRIX4X4): {
            /* get uniform 'location' */
            GLint location = glGetUniformLocation(self->program, pName);
            if (location == -1) {
                return (0);
            }

            /* set uniform 'pData' */
            glProgramUniformMatrix4fv(self->program, location, 1, GL_FALSE, pData);
        } break;
    }

    /* success */
    return (1);
}

/* static API */

static GLint __ftReadFile(FILE *pFile, char **pCstr, size_t *pSize) {
    /* null-check */
    if (!pFile) { return (GL_INVALID_VALUE); }

    /* the the size of the 'pFile' */
    fseek(pFile, 0, SEEK_END);
    size_t fsize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    /* create null-terminated c-string pointer of size of 'pFile' */
    char *fcstr = (char *) calloc(fsize + 1, sizeof(char));
    if (!fcstr) { return (0); }

    /* read 'pFile' to 'cstr' */
    if (fread(fcstr, sizeof(*fcstr), fsize, pFile) != fsize) {
        /* 'fread' didn't match the 'fsize'; something bad happened */
        free(fcstr); fcstr = 0;
        return (0);
    }

    /* return */
    if (pSize) { *pSize = fsize; }
    if (pCstr) { *pCstr = fcstr; }
    else {
        /* can't return 'pCstr'; release 'fcstr' */
        free(fcstr); fcstr = 0;
    }

    /* success */
    return (1);
}
