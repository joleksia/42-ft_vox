#include "ft_vox.h"

/* static API */

static int __ft_worldgen_landscape(struct world_s *, struct chunk_s *, int, int, int);

/* public API */

extern int ft_worldgen_generate(struct world_s *world, struct chunk_s *chunk, int cx, int cy, int cz) {
    /* null-check */
    if (!world) { return (0); }
    if (!chunk) { return (0); }

    /* step 1.: generate landscape */
    __ft_worldgen_landscape(world, chunk, cx, cy, cz);

    /* success */
    return (1);
}

/* static API */

static int __ft_worldgen_landscape(struct world_s *world, struct chunk_s *chunk, int cx, int cy, int cz) {
    /* null-check */
    if (!world) { return (0); }
    if (!chunk) { return (0); }

    /* iterate on X-axis and Z-axis of the 'chunk' */
    for (int x = 0; x < FT_VOX_CHUNK_SIZE_X; x++) {
    for (int z = 0; z < FT_VOX_CHUNK_SIZE_Z; z++) {
        /* 'height' : value on Y-axis representing the height of the final solid block */
        int height = fbm2d((cx + x) * 0.08,
                           (cz + z) * 0.08,
                            0.64, 1.8, 8, world->seed);

        /* iterate on Y-axis in the 'chunk' */
        for (int y = 0; y < FT_VOX_CHUNK_SIZE_Y; y++) {
            /* 'y' offset by 'cy' */
            int yoff = cy + y;

            /* case : 'yoff' is at 'height' */
            if (yoff <= height) {
                ft_chunk_set(chunk, x, y, z, FT_BLOCK_STONE);
            }

            /* case : 'yoff' is above 'height' */
            else {
                ft_chunk_set(chunk, x, y, z, FT_BLOCK_AIR);
            }
        }
    }}

    /* success */
    return (1);
}
