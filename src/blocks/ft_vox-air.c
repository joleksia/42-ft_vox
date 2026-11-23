#include "ft_vox.h"

/* public API */

int ft_block_air(void) {
    /* create new 'block' */
    struct
    block_s block = { 0 };

    /* return 'block' */
    BLOCKS[block.id] = block;

    /* success */
    return (1);
}
