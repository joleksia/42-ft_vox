#include "ft_vox.h"

/* static API */

static int __ft_grass_texcoord(struct block_s *, vec2 *, enum face_e);

/* public API */

int ft_block_grass(void) {
    /* create new 'block' */
    struct
    block_s block = {
        
        /* members */
        
        .id = FT_BLOCK_GRASS,
        .type = FT_BLOCK_TYPE_SOLID,

        /* methods */

        .texcoord = __ft_grass_texcoord
    };

    /* return 'block' */
    BLOCKS[block.id] = block;

    /* success */
    return (1);
}

/* static API */

static int __ft_grass_texcoord(struct block_s *self, vec2 *uv, enum face_e face) {
    /* null-check */
    if (!self) { return (0); }

    /* return 'uv' */
    if (uv) {
        switch (face) {
            case (FT_FACE_DOWN): { *uv = alg_vec2(1.0, 0.0); } break;
            case (FT_FACE_TOP):  { *uv = alg_vec2(3.0, 0.0); } break;
            default:             { *uv = alg_vec2(2.0, 0.0); } break;
        }
    }

    /* success */
    return (1);
}
