#include "ft_vox.h"

/* static API */

static int __ft_water_texcoord(struct block_s *, vec2 *, enum face_e);

/* public API */

int ft_block_water(void) {
    /* create new 'block' */
    struct
    block_s block = {
        
        /* members */
        
        .id = FT_BLOCK_WATER,
        .type = FT_BLOCK_TYPE_TRANS,

        /* methods */

        .texcoord = __ft_water_texcoord
    };

    /* return 'block' */
    BLOCKS[block.id] = block;

    /* success */
    return (1);
}

/* static API */

static int __ft_water_texcoord(struct block_s *self, vec2 *uv, enum face_e face) {
    /* null-check */
    if (!self) { return (0); }
    (void) face;

    /* return 'uv' */
    if (uv) {
        *uv = alg_vec2(31.0, 0.0);
    }

    /* success */
    return (1);
}
