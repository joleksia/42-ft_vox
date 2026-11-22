#include "ft_vox.h"

/* static API */

struct block_s *BLOCKS = 0;

static int __ft_blocks_populate(void);

/* public API */

extern int ft_blocks_create(const size_t size) {
    /* alloc. 'blocks' */
    if (!BLOCKS) {
        struct block_s *blocks = (struct block_s *) calloc(size, sizeof(struct block_s));
        if (!blocks) { return (0); }

        /* return 'blocks' */
        BLOCKS = blocks;
    }

    /* populate 'BLOCKS' */
    return (__ft_blocks_populate());
}


extern int ft_blocks_delete(void) {
    /* null-check */
    if (!BLOCKS) { return (0); }

    /* release 'BLOCKS' */
    free(BLOCKS); BLOCKS = 0;

    /* success */
    return (1);
}

/* static API */

static int __ft_blocks_populate(void) {
    /* null-check */
    if (!BLOCKS) { return (0); }

    /* TODO:
     * Automate this process with, maybe, file serialization?
     * */
    /* block : 'AIR'*/
    BLOCKS[FT_BLOCK_AIR] = (struct block_s) {
        .id = FT_BLOCK_AIR,
        .metadata = {
            .type = FT_BLOCK_TYPE_TRANS,
            .tex_index = { 0 }
        }
    };

    /* block : 'STONE'*/
    BLOCKS[FT_BLOCK_STONE] = (struct block_s) {
        .id = FT_BLOCK_STONE,
        .metadata = {
            .type = FT_BLOCK_TYPE_SOLID,
            .tex_index = { [FT_FACE_NORTH] = 1,
                           [FT_FACE_SOUTH] = 1, 
                           [FT_FACE_WEST]  = 1,
                           [FT_FACE_EAST]  = 1,
                           [FT_FACE_DOWN]  = 1,
                           [FT_FACE_TOP]   = 1 },
        },
    };

    /* block : 'DIRT'*/
    BLOCKS[FT_BLOCK_DIRT] = (struct block_s) {
        .id = FT_BLOCK_DIRT,
        .metadata = {
            .type = FT_BLOCK_TYPE_SOLID,
            .tex_index = { [FT_FACE_NORTH] = 2,
                           [FT_FACE_SOUTH] = 2, 
                           [FT_FACE_WEST]  = 2,
                           [FT_FACE_EAST]  = 2,
                           [FT_FACE_DOWN]  = 2,
                           [FT_FACE_TOP]   = 2 },
        },
    };

    /* success */
    return (1);
}
