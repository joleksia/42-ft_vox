#include "ft_vox.h"

/* Static API */

static GLint __ftReadFile(FILE *, char **, size_t *);

static GLint __ftShaderCompileGLSL(GLshader, const GLchar * const *);

static GLint __ftShaderCompileSPIRV(GLshader, const GLchar * const *);

static GLint __ftShaderLink(GLshader, GLshader);

/* Public API */

extern GLint ftLoadShaderGLSL(GLshader shader, GLenum type, FILE *pGlsl) {
    /* null-check */
    if (!shader) { return (GL_INVALID_VALUE); }
    if (!pGlsl)  { return (GL_INVALID_VALUE); }

    /* read 'pFile' */
    GLchar *code = 0;
    __ftReadFile(pGlsl, &code, 0);
    if (!code) { return (GL_INVALID_OPERATION); }

    /* try to compile shader 'object' */
    GLshader object = glCreateShader(type);
    if (__ftShaderCompileGLSL(object, (const GLchar * const *) &code) != GL_NO_ERROR) {
        free(code);
        return (0);
    }

    /* try to link 'shader' */
    if (__ftShaderLink(shader, object) != GL_NO_ERROR) {
        glDeleteShader(object);
        free(code);
        return (glGetError());
    }

    /* release resources */
    glDeleteShader(object);
    free(code);

    /* success */
    return (GL_NO_ERROR);
}


extern GLint ftLoadShaderSPIRV(GLshader shader, GLenum type, FILE *pSpirv) {
    /* null-check */
    if (!shader) { return (GL_INVALID_VALUE); }
    if (!pSpirv) { return (GL_INVALID_VALUE); }

    /* read 'pFile' */
    GLchar *code = 0;
    __ftReadFile(pSpirv, &code, 0);
    if (!code) { return (GL_INVALID_OPERATION); }

    /* try to compile shader 'object' */
    GLshader object = glCreateShader(type);
    if (__ftShaderCompileSPIRV(object, (const GLchar * const *) &code) != GL_NO_ERROR) {
        free(code);
        return (0);
    }

    /* try to link 'shader' */
    if (__ftShaderLink(shader, object) != GL_NO_ERROR) {
        glDeleteShader(object);
        free(code);
        return (glGetError());
    }

    /* release resources */
    glDeleteShader(object);
    free(code);

    /* success */
    return (GL_NO_ERROR);
}

/* Static API */

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

static GLint __ftShaderCompileGLSL(GLshader shader, const GLchar * const *code) {
    /* null-check */
    if (!shader) { return (GL_INVALID_VALUE); }
    if (!code)   { return (GL_INVALID_VALUE); }

    /* set 'shader's GLSL 'code' */
    glShaderSource(shader, 1, code, 0);

    /* compile 'shader' */
    glCompileShader(shader);

    /* compilation 'status' */
    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        /* log compilation error */
        GLchar log[1024] = { 0 };
        GLsizei length = 0;
        glGetShaderInfoLog(shader, sizeof(log), &length, log);

        /* print 'log' */
        printf("%s", log);
        return (GL_INVALID_OPERATION);
    }

    /* success */
    return (GL_NO_ERROR);
}

static GLint __ftShaderCompileSPIRV(GLshader shader, const GLchar * const *code) {
    /* null-check */
    if (!shader) { return (GL_INVALID_VALUE); }
    if (!code)   { return (GL_INVALID_VALUE); }

    /* set 'shader's GLSL 'code' */
    glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, code, 0);

    /* compile 'shader' */
    glCompileShader(shader);

    /* compilation 'status' */
    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        /* get error 'log' */
        GLchar log[1024] = { 0 };
        GLsizei length = 0;
        glGetShaderInfoLog(shader, sizeof(log), &length, log);

        /* print 'log' */
        printf("%s", log);
        return (GL_INVALID_OPERATION);
    }

    /* success */
    return (GL_NO_ERROR);
}


static GLint __ftShaderLink(GLshader shader, GLshader object) {
    /* null-check */
    if (!shader) { return (GL_INVALID_VALUE); }
    if (!object) { return (GL_INVALID_VALUE); }
    
    /* link 'object' to 'shader' program */
    glAttachShader(shader, object);

    /* link 'shader' */
    glLinkProgram(shader);
    
    /* link 'status' */
    GLint status = 0;
    glGetProgramiv(shader, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        /* get error 'log' */
        GLchar log[1024] = { 0 };
        GLsizei length = 0;
        glGetProgramInfoLog(shader, sizeof(log), &length, log);

        /* print 'log' */
        printf("%s", log);
        return (0);
    }

    /* success */
    return (GL_NO_ERROR);
}
