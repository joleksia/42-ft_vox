#include "ft_vox.h"

/* static API */

static int __ft_worldgen_gen_volume(struct world_s *, struct chunk_s *);

static int __ft_worldgen_gen_shape(struct world_s *, struct chunk_s *);

/* public API */

extern int ft_worldgen_generate(struct world_s *world, struct chunk_s *chunk) {
    /* null-check */
    if (!world) { return (0); }
    if (!chunk) { return (0); }

    /* step 1.: generate volumes */
    __ft_worldgen_gen_volume(world, chunk);

    /* step 2.: generate 'world's shape */
    __ft_worldgen_gen_shape(world, chunk);

    /* success */
    return (1);
}

/* static API */

static int __ft_worldgen_gen_volume(struct world_s *world, struct chunk_s *chunk) {
    /* null-check */
    if (!world) { return (0); }
    if (!chunk) { return (0); }

    /* 'noise' buffer */
    f32 noise[FT_UPSAMPLE_X * FT_UPSAMPLE_Z] = { 0 };

    /* generate 'noise' */
    for (int i = 0; i < FT_UPSAMPLE_X * FT_UPSAMPLE_Z; i++) {
        /* get 'x' and 'z' indices */
        int nx = i % FT_UPSAMPLE_X,
            nz = i / FT_UPSAMPLE_X;
        
        /* get 'volume' */
        f32 volume = perlin2d((chunk->offset.x + nx * FT_UPSAMPLE_STEP) * 0.01,
                              (chunk->offset.z + nz * FT_UPSAMPLE_STEP) * 0.01, world->seed);

        /* return 'volume' */
        noise[nz * FT_UPSAMPLE_X + nx] = volume;
    }
    
    /* generate interpolated 'volume' */
    for (int i = 0; i < FT_CHUNK_SIZE_X * FT_CHUNK_SIZE_Z; i++) {
        /* get 'x' and 'z' indices */
        int x = i % FT_CHUNK_SIZE_X,
            z = i / FT_CHUNK_SIZE_X;

        /* get 'noise's 'x' and 'z' */
        int nx = x / FT_UPSAMPLE_STEP,
            nz = z / FT_UPSAMPLE_STEP;

        /* get in-between 'x' and 'y' */
        f32 t1 = (x % FT_UPSAMPLE_STEP) / (f32) FT_UPSAMPLE_STEP,
            t2 = (z % FT_UPSAMPLE_STEP) / (f32) FT_UPSAMPLE_STEP;

        /* get values for lerp */
        f32 a = noise[(nz)     * FT_UPSAMPLE_X + (nx)],
            b = noise[(nz)     * FT_UPSAMPLE_X + (nx + 1)],
            c = noise[(nz + 1) * FT_UPSAMPLE_X + (nx)],
            d = noise[(nz + 1) * FT_UPSAMPLE_X + (nx + 1)];

        /* return 'volume' */
        chunk->volume[z * FT_CHUNK_SIZE_X + x] = alg_lerp(alg_lerp(a, b, t1),
                                                          alg_lerp(c, d, t1), t2);
    }

    /* success */
    return (1);
}


static int __ft_worldgen_gen_shape(struct world_s *world, struct chunk_s *chunk) {
    /* null-check */
    if (!world) { return (0); }
    if (!chunk) { return (0); }
    
    /* iterate on X-axis and Z-axis of the 'chunk' */
    for (int i = 0; i < FT_CHUNK_SIZE_X * FT_CHUNK_SIZE_Z; i++) {
        /* get 'x' and 'z' indices */
        int x = i % FT_CHUNK_SIZE_X,
            z = i / FT_CHUNK_SIZE_X;

        /* get 'height' */
        int height = chunk->volume[i] * 32.0;
        
        /* iterate on Y-axis of the 'chunk' */
        for (int y = 0; y < FT_CHUNK_SIZE_Y; y++) {
            /* 'y' offset by 'cy' */
            int yoff = chunk->offset.y + y;

            /* case : 'yoff' is at 'height' */
            if (yoff <= height) {
                chunk->blocks[FT_CHUNK_3D_1D(x, y, z)] = FT_BLOCK_STONE;
            }

            /* case : 'yoff' is above 'height' */
            else {
                chunk->blocks[FT_CHUNK_3D_1D(x, y, z)] = FT_BLOCK_AIR;
            }
        }
    }

    /* success */
    return (1);
}
