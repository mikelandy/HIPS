
/* block.c              -Brian Tierney, LBL        1/91  */

/*  This is part of isobuild program.
 *
 *  This routine divide the data into blocks, with a structure indicating
 *  the minimum, maximum, and location of each block of data.
 *
 *  Using this routines will usually result in a 30% speed up, because
 *  large sections of the data set can be skipped.  There is an additional
 *  use of memory however.
 */

 /* 
  *  This method used here is to create a 2D array of "BLOCK_INFO"
  *   structures which contain the following infomation:
  *     1) the location of the block in the 3D data set
  *     2) the size of the block
  *     3) the minimum and maximum values within the block
  *     4) pointers into the data and grid structures
  *
  *  Using this information, blocks of data can be skipped by just
  *  checking the minimum and maximum values within a block.
 */


/* $Id: block.c,v 1.3 1992/01/31 02:05:45 tierney Exp $ */

/* $Log: block.c,v $
 * Revision 1.3  1992/01/31  02:05:45  tierney
 * *** empty log message ***
 *
 * Revision 1.2  1991/12/19  01:41:16  davidr
 * added RCS identification markers
 * */

static char rcsid[] = "$Id: block.c,v 1.3 1992/01/31 02:05:45 tierney Exp $" ;

#include "isobuild.h"

/*********************************************************/
int
block_setup()
{
    int       nblocks;
    BLOCK_INFO **alloc_block_info_array();

    /* guess a good block size: this will depend on the type of data
       and the size of the structures relative to the size of the
       data set.  I've found that this seems to be a good value for
       medical data.  -BT
     */

    if (BLOCK_SIZE == 0)
	BLOCK_SIZE = MAX(xdim, ydim) / 12;
    if (BLOCK_SIZE < 3)
	BLOCK_SIZE = 3;

    fprintf(stderr, "Creating block min/max map with size %d blocks..\n",
	    BLOCK_SIZE);
    nblocks = MAX(xdim, ydim) / BLOCK_SIZE;
    if ((MAX(xdim, ydim) % BLOCK_SIZE) != 0)
	nblocks++;
    nblocks = nblocks * nblocks;
    if (block_info_array == NULL)
	block_info_array = alloc_block_info_array(zdim, nblocks);

    create_blocks(nblocks);

    get_block_min_max(nblocks);

    Status("Block map created.");

    return (nblocks);
}

/********************************************************************/
create_blocks(tot_num_blocks)  /* sets up block structures */
    int       tot_num_blocks;
{
    register int slice, block;
    BLOCK_INFO *binfo_ptr;
    Grid_type **alloc_2d_grid_array();
    int       xloc = 0, yloc = 0;
    int       nxsize, nysize;

    for (slice = 0; slice < zdim; slice++) {
	xloc = yloc = 0;
	for (block = 0; block < tot_num_blocks; block++) {

	    if (yloc >= ydim || xloc >= xdim) {
		Error("Error in block creation, try running with the -f option");
	    }
	    binfo_ptr = &block_info_array[slice][block];

	    binfo_ptr->xloc = xloc;
	    binfo_ptr->yloc = yloc;

	    binfo_ptr->dslice = data[slice];
	    binfo_ptr->grid = grid[slice];
	    
	    binfo_ptr->width = BLOCK_SIZE;
	    binfo_ptr->height = BLOCK_SIZE;

	    xloc += BLOCK_SIZE;
	    if (xloc >= xdim) {
		nxsize = xdim - xloc + BLOCK_SIZE;
		binfo_ptr->width = nxsize;
		xloc = 0;
		yloc += BLOCK_SIZE;
	    }

	    if (binfo_ptr->yloc + binfo_ptr->height >= ydim) {
		if (yloc >= ydim) 
		    nysize = ydim - yloc + BLOCK_SIZE;
		else
		    nysize = ydim - yloc;
		binfo_ptr->height = nysize;
	    }

	}
    }
}

/*******************************************************************/

get_block_min_max(nblocks)
    int       nblocks;
{
    register int i, j;
    register int slice, block;
    register Data_type **dptr;
    BLOCK_INFO *binfo_ptr;
    int       size, xloc, yloc;
    Data_type     dval;
    Data_type     min, max;		/* local min and max variable */

    for (slice = 0; slice < zdim; slice++) {
	for (block = 0; block < nblocks; block++) {
	    binfo_ptr = &block_info_array[slice][block];

	    size = binfo_ptr->width * binfo_ptr->height;
	    if (size > 0) {
		yloc = binfo_ptr->yloc;
		xloc = binfo_ptr->xloc;
		dptr = binfo_ptr->dslice;
		max = min = dptr[yloc][xloc];

		/* use '<=' because cube includes data[+1] */
		for (i = 0; i <= binfo_ptr->height; i++) {
		    xloc = binfo_ptr->xloc;
		    for (j = 0; j <= binfo_ptr->width; j++) {
			/* dont want `<=` for last block in each row/col */
			if (slice < zdim && yloc < ydim && xloc < xdim) {
			    dval = dptr[yloc][xloc];
			    if (dval > max)
				max = dval;
			    else if (dval < min)
				min = dval;
			}
			xloc++;
		    }
		    yloc++;
		}
		binfo_ptr->min = min;
		binfo_ptr->max = max;

		/* set global min/max */
		if (max > data_max)
		    data_max = max;
		if (min < data_min)
		    data_min = min;
	    } else {
		binfo_ptr->min = (Data_type)0;
		binfo_ptr->max = (Data_type)0;
	    }
	}
    }

    return;
}

/********************************************************************/
show_block_grid(tot_num_blocks)   /* for debugging */
    int       tot_num_blocks;
{
    register int slice, block, i,j;
    BLOCK_INFO *binfo;

    for (slice = 0; slice < zdim; slice++) {
	for (block = 0; block < tot_num_blocks; block++) {

	    fprintf(stderr, "\n\n slice: %d,  block: %d ", slice, block);

	    binfo = &block_info_array[slice][block];
	    for (j = 0; j <= binfo->height; j++) {
		fprintf(stderr, "\n");
		for (i = 0; i <= binfo->width; i++) 
		    fprintf(stderr, " %2d", binfo->grid[j][i]);
	    }
	}
    }
}

