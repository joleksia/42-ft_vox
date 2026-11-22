#include "ft_vox.h"

extern int ft_sampler_create(struct sampler_s **self, const char *path) {
    /* null-check */
    if (!self) { return (0); }

    /* alloc. 'sampler' */
    struct sampler_s *sampler = (struct sampler_s *) calloc(1, sizeof(struct sampler_s));
    if (!sampler) { return (0); }

    /* load 'image' from 'path' */
    u8 *image = stbi_load(path, &sampler->width,
                                &sampler->height,
                                &sampler->channel, 4);
    if (!image) {
        free(sampler);
        return (0);
    }

    /* create 'sampler's 'id' */
    glCreateTextures(GL_TEXTURE_2D, 1, &sampler->id);
    glTextureParameteri(sampler->id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(sampler->id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(sampler->id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(sampler->id, GL_TEXTURE_WRAP_T, GL_REPEAT);

    /* store 'image' in 'sampler' */
    glTextureStorage2D(sampler->id, 1, GL_RGBA8, sampler->width, sampler->height);
    glTextureSubImage2D(sampler->id, 0, 0, 0, sampler->width, sampler->height, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateTextureMipmap(sampler->id);

    /* release 'image' */
    free(image);

    /* return 'self' */
    *self = sampler;

    /* success */
    return (1);
}


extern int ft_sampler_delete(struct sampler_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* clear 'id' */
    glDeleteTextures(1, &self->id);

    /* release 'self' */
    free(self);

    /* success */
    return (1);
}


extern int ft_sampler_bind(struct sampler_s *self, const u32 slot) {
    /* null-check */
    if (!self) { return (0); }

    /* bind 'self' at 'slot' */
    glBindTextureUnit(slot, self->id);

    /* success */
    return (1);
}
