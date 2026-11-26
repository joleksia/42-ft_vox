#include "ft_vox.h"

/* static API */

struct block_s BLOCKS[INT16_MAX] = { 0 };

/* public API */

extern int ft_blocks_create(void) {
    
    /* create : 'FT_BLOCK_AIR' */
    extern int ft_block_air();
    ft_block_air();

    /* create : 'FT_BLOCK_STONE' */
    extern int ft_block_stone();
    ft_block_stone();

    /* create : 'FT_BLOCK_GRASS' */
    extern int ft_block_grass();
    ft_block_grass();

    /* create : 'FT_BLOCK_DIRT' */
    extern int ft_block_dirt();
    ft_block_dirt();

    /* create : 'FT_BLOCK_SAND' */
    extern int ft_block_sand();
    ft_block_sand();

    /* create : 'FT_BLOCK_GRAVEL' */
    extern int ft_block_gravel();
    ft_block_gravel();

    /* create : 'FT_BLOCK_WATER' */
    extern int ft_block_water();
    ft_block_water();

    /* success */
    return (1);
}
