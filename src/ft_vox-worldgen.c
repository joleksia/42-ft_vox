#include "ft_vox.h"

/* public API */

extern int ft_worldgen_generate(struct world_s *world, struct chunk_s *chunk, int cx, int cy, int cz) {
    /* null-check */
    if (!world) { return (0); }
    if (!chunk) { return (0); }

    /* iterate on X-axis and Z-axis of the 'chunk' */
    for (int x = 0; x < FT_VOX_CHUNK_SIZE_X; x++) {
    for (int z = 0; z < FT_VOX_CHUNK_SIZE_Z; z++) {
        /* 'height' : value on Y-axis representing the height of the final solid block */
        double height = fbm2d((cx + x) * 0.64,
                              (cz + z) * 0.64,
                              0.5, 2.0, 8,
                              world->seed);

        /* iterate on Y-axis in the 'chunk' */
        for (int y = 0; y < FT_VOX_CHUNK_SIZE_Y; y++) {
            /* 'idx' : block ID to be placed */
            u32 idx = FT_BLOCK_AIR;

            /* if below 'height', then place 'STONE' */
            if (cy + y < height) {
                idx = FT_BLOCK_STONE;
            }

            /* otherwise... */
            else {
                /* ...if on water level */
                if (cy < 0) {
                    // idx = FT_BLOCK_WATER;
                }
            }
            
            /* place 'idx' in 'chunk */
            ft_chunk_set(chunk, x, y, z, idx);
        }
    }}

    /* success */
    return (1);
}
