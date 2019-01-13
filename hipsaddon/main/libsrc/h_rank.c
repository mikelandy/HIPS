/*
 *
 *             Copyright (c) 1991 The Turing Institute
 *
 * Disclaimer:  No guarantees of performaroi_colse accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_rank.c - rank order filters an input image
 *
 * input pixel formats: byte, short, int, float, double
 * output pixel formats: same as input
 *
 * Author: Colin Urquhart, The Turing Institute, 11 September 1991
 *	   (converted from frank.c)
 *
 */

#include <hipl_format.h>
#include <math.h>

int expand_window_B(),expand_window_S(),expand_window_I(),expand_window_F(),
	expand_window_D(),shrink_window_B(),shrink_window_S(),
	shrink_window_I(),shrink_window_F(),shrink_window_D(),
	insert_fresh_B(),insert_fresh_S(),insert_fresh_I(),insert_fresh_F(),
	insert_fresh_D(),scan_down_B(),scan_down_S(),scan_down_I(),
	scan_down_F(),scan_down_D(),scan_up_B(),scan_up_S(),scan_up_I(),
	scan_up_F(),scan_up_D();

#define TOP 1000
#define BOTTOM -1

typedef struct
{
    int    	above; /* pointer to the pixel ranked above this pixel */
    int    	below; /* pointer to the pixel ranked below this pixel */
    int    	rank;  /* the rank position of this pixel */
    byte  	value; /* the pixel value of this pixel */
} Pixel_info_b;

typedef struct
{
    int    	above; /* pointer to the pixel ranked above this pixel */
    int    	below; /* pointer to the pixel ranked below this pixel */
    int    	rank;  /* the rank position of this pixel */
    short  	value; /* the pixel value of this pixel */
} Pixel_info_s;

typedef struct
{
    int    	above; /* pointer to the pixel ranked above this pixel */
    int    	below; /* pointer to the pixel ranked below this pixel */
    int    	rank;  /* the rank position of this pixel */
    int  	value; /* the pixel value of this pixel */
} Pixel_info_i;

typedef struct
{
    int    	above; /* pointer to the pixel ranked above this pixel */
    int    	below; /* pointer to the pixel ranked below this pixel */
    int    	rank;  /* the rank position of this pixel */
    float  	value; /* the pixel value of this pixel */
} Pixel_info_f;

typedef struct
{
    int    	above; /* pointer to the pixel ranked above this pixel */
    int    	below; /* pointer to the pixel ranked below this pixel */
    int    	rank;  /* the rank position of this pixel */
    double  	value; /* the pixel value of this pixel */
} Pixel_info_d;

/********************************* h_rank *************************************
*									      *
* Description:	rank order filters an HIPS2 header image.		      *
*									      *
* Parameters:	input_hdr	*header,				      *
*			pointer to the input image header.		      *
*		output_hdr	*header,				      *
*			pointer to the output image header.		      *
*		hor_size	int,	       				      *
*			the size of the horizontal filtering kernal.	      *
*		ver_size	int,	       				      *
*			the size of the vertical filtering kernal.	      *
*		hor_rank	int,	       			   	      *
*			the rank for filtering with the horizontal kernal.    *
*		ver_rank	int,	       			   	      *
*			the rank for filtering with the vertical kernal.      *
*									      *
* Return value: int,							      *
*			HIPS_ERROR - an error has occured.		      *
*			HIPS_OK    - successful execution.		      *
*									      *
* Functions Called:	h_rank_b,      					      *
*			h_rank_s,					      *
*			h_rank_i,					      *
*			h_rank_f,					      *
*			h_rank_d.					      *
*									      *
* External Functions:	perr.						      *
*									      *
* External Parameters:	none.						      *
*									      *
******************************************************************************/

int h_rank (input_hdr,
	output_hdr,
	hor_size,
	ver_size,
	hor_rank,
	ver_rank)

struct	header	*input_hdr;
struct	header	*output_hdr;
int	hor_size;
int	ver_size;
int	hor_rank;
int	ver_rank;
{
    /* Switch on the input pixel format */

    switch(input_hdr->pixel_format) 
    {
	case PFBYTE:	return (h_rank_b (input_hdr,
					  output_hdr,	
					  hor_size,
					  ver_size,
					  hor_rank,
					  ver_rank));

	case PFSHORT:	return (h_rank_s (input_hdr,
					  output_hdr,	
					  hor_size,
					  ver_size,
					  hor_rank,
					  ver_rank));

	case PFINT:	return (h_rank_i (input_hdr,
					  output_hdr,	
					  hor_size,
					  ver_size,
					  hor_rank,
					  ver_rank));

	case PFFLOAT:	return (h_rank_f (input_hdr,
					  output_hdr,	
					  hor_size,
					  ver_size,
					  hor_rank,
					  ver_rank));

	case PFDOUBLE:	return (h_rank_d (input_hdr,
					  output_hdr,	
					  hor_size,
					  ver_size,
					  hor_rank,
					  ver_rank));

	default:	return (perr (HE_FMTSUBR,
				      "h_rank",
				      hformatname (input_hdr->pixel_format)));
      }
}

/******************************** h_rank_b ************************************
*									      *
* Description:	1D rank order filters a PFBYTE format HIPS2 header image in   *
*		horizontal dimension.					      *
*									      *
* Parameters:	input_hdr	*header,				      *
*			pointer to the input image header.		      *
*		output_hdr	*header,				      *
*			pointer to the output image header.		      *
*		hor_size	int,	       				      *
*			the size of the horizontal filtering kernal.	      *
*		ver_size	int,	       				      *
*			the size of the vertical filtering kernal.	      *
*		hor_rank	int,	       			   	      *
*			the rank for filtering with the horizontal kernal.    *
*		ver_rank	int,	       			   	      *
*			the rank for filtering with the vertical kernal.      *
*									      *
* Return value: int,							      *
*			HIPS_ERROR - an error has occured.		      *
*			HIPS_OK    - successful execution.		      *
*									      *
* Functions Called:	h_rank_B,      					      *
*									      *
* External Functions:	none.						      *
*									      *
* External Parameters:	none.						      *
*									      *
******************************************************************************/

int h_rank_b (input_hdr,
	  output_hdr,	
	  hor_size,
	  ver_size,
	  hor_rank,
	  ver_rank)
     
struct 	header 	*input_hdr;
struct	header	*output_hdr;
int		hor_size;
int		ver_size;
int		hor_rank;
int		ver_rank;
{
    return (h_rank_B ((byte *) input_hdr->firstpix,
		      (byte *) output_hdr->firstpix,
		      input_hdr->rows,
		      input_hdr->cols,
		      input_hdr->ocols,
		      output_hdr->ocols,
		      hor_size,
		      ver_size,
		      hor_rank,
		      ver_rank));
}

/******************************** h_rank_B ************************************
*									      *
* Description:	rank order filters a byte format image. 		      *
*									      *
* Parameters:	input_roi	*byte,					      *
*			pointer to the start of the input region of interest. *
*		output_roi,	*byte,					      *
*			pointer to the start of the output roi.		      *
*	        roi_rows,	int,					      *
*			number of rows in the roi.			      *
*		roi_cols,	int,					      *
*			number of columns in the roi.			      *
*	  	input_cols,	int,					      *
*			number of columns in the entire input frame.	      *
*	  	output_cols,	int,					      *
*			number of columns in the entire output frame.	      *
*		hor_size	int,	       				      *
*			the size of the horizontal filtering kernal.	      *
*		ver_size	int,	       				      *
*			the size of the vertical filtering kernal.	      *
*		hor_rank	int,	       			   	      *
*			the rank for filtering with the horizontal kernal.    *
*		ver_rank	int,	       			   	      *
*			the rank for filtering with the vertical kernal.      *
*									      *
* Return value: int,							      *
*			HIPS_ERROR - an error has occured.		      *
*			HIPS_OK    - successful execution.		      *
*									      *
* Functions Called:	h_hor_rank_B,	      			       	      *
*			h_ver_rank_B.					      *
*									      *
* External Functions:	h_copy_B.		       			      *
*									      *
******************************************************************************/

int h_rank_B (input_roi,
	  output_roi,
	  roi_rows,
	  roi_cols,
	  input_cols,
	  output_cols,
	  hor_size,
	  ver_size,
	  hor_rank,
	  ver_rank)

byte 	*input_roi;
byte 	*output_roi;
int 	roi_rows;
int	roi_cols;
int	input_cols;
int	output_cols;
int	hor_size;
int	ver_size;
int	hor_rank;
int	ver_rank;
{
    if (hor_size > 1)
    {
	if ((h_hor_rank_B (input_roi,
			   output_roi,
			   roi_rows,
			   roi_cols,
			   input_cols,
			   output_cols,
			   hor_size,
			   hor_rank - 1)) == HIPS_ERROR)
	  return (HIPS_ERROR);
    }
    if (ver_size > 1)
    {
	if (hor_size > 1)
	{
	    /* Copy the output from horizontal filtering onto the input for *
	     * vertical filtering.					    */

	    if ((h_copy_B (output_roi,
			   input_roi,
			   roi_rows,
			   roi_cols,
			   input_cols,
			   output_cols)) == HIPS_ERROR)
	      return (HIPS_ERROR);
	}

	if ((h_ver_rank_B (input_roi,
			   output_roi,
			   roi_rows,
			   roi_cols,
			   input_cols,
			   output_cols,
			   ver_size,
			   ver_rank - 1)) == HIPS_ERROR)
	  return (HIPS_ERROR);
    }
    return (HIPS_OK);
}

/****************************** h_hor_rank_B **********************************
*									      *
* Description:	1D rank order byte format image in the horizontal dimension.  *
*									      *
* Parameters:	input_roi	*byte,					      *
*			pointer to the start of the input region of interest. *
*		output_roi,	*byte,					      *
*			pointer to the start of the output roi.		      *
*	        roi_rows,	int,					      *
*			number of rows in the roi.			      *
*		roi_cols,	int,					      *
*			number of columns in the roi.			      *
*	  	input_cols,	int,					      *
*			number of columns in the entire input frame.	      *
*	  	output_cols,	int,					      *
*			number of columns in the entire output frame.	      *
*		size		int,	       				      *
*			the size of the horizontal filtering kernal.	      *
*		rank		int,	       			   	      *
*			the rank for filtering with the horizontal kernal.    *
*									      *
* Return value: int,							      *
*			HIPS_ERROR - an error has occured.		      *
*			HIPS_OK    - successful execution.		      *
*									      *
* Functions Called:	expand_window_B,     			       	      *
*			shrink_window_B,       				      *
*			insert_fresh_B.					      *
*									      *
* External Functions:	perr.						      *
*									      *
* External Parameters:	none.						      *
*									      *
* Method: each row of the input frame is scanned with the filtering window.   *
*         PIXEL_INDEX and RANK_INDEX hold the required information for each   *
*         pixel in the window. As the window moves across the row, the stale  *
*         pixel (ie. the one just leaving the window) is removed from the     *
*         window and the fresh pixel (ie. the one just entering the window)   *
*         is inserted. The RANK_INDEX and PIXEL_INDEX are updated  	      *
*         appropriately and the value of the pixel with the required rank is  *
*         outputed. For regions near the beginning of the row, where the      *
*         entire window cannot fit in, the window size is grown from 1 pixel  *
*         upto the required size as the window moves along the row. The value *
*         of the pixel with the appropriate rank for the reduced window size  *
*         is outputed. In a similar manner the window size is shrunk down as  *
*         the window approaches the end of the row.			      *
*									      *
******************************************************************************/

int h_hor_rank_B (input_roi,
	      output_roi,
	      roi_rows,
	      roi_cols,
	      input_cols,
	      output_cols,
	      size,
	      rank)

byte 	*input_roi;
byte 	*output_roi;
int 	roi_rows;
int	roi_cols;
int	input_cols;
int	output_cols;
int	size;
int	rank;
{
    static	h_boolean		allocation = FALSE;
    static	int   		*rank_index;
    static	Pixel_info_b 	*pixel_index;

    int    col;             /* the roi column of the current pixel */
    int    row;             /* the roi row of the current pixel */
    int	   input_offset;    /* offset between end & start of input roi rows */
    int	   output_offset;   /* offset between end & start of output roi rows */
    int    current_size;    /* the current window size */
    int    current_rank;    /* the rank appropriate to the current size */
    int    half_size;       /* half the required window size */
    byte   *output_ptr;	    /* pointer to the current output pixel */
    byte   *fresh_ptr;      /* pointer to the "fresh" pixel in the i/p image */
    int    fresh_pixel;     /* pointer to the "fresh" pixel in pixel index */
    int    count;           /* counting variable for expanding/shrinking */

    if (allocation == FALSE)
    {
	/* allocate core for the rank index */
	
	if ((rank_index = (int *) calloc (size, sizeof(int))) == 0)
	  return (perr (HE_ALLOCSUBR, "h_hor_rank_B"));

	/* allocate core for the pixel index */
	
	if ((pixel_index = (Pixel_info_b *) 
	     calloc (size, sizeof(Pixel_info_b))) == 0)
	  return (perr (HE_ALLOCSUBR, "h_hor_rank_B"));

	allocation = TRUE;
    }

    half_size = (size + 1)/2;
    input_offset = input_cols - roi_cols;
    output_offset = output_cols - roi_cols;

    output_ptr = output_roi;
    fresh_ptr  = input_roi;

    for (row = 0; row < roi_rows; row++)
    {
	fresh_pixel = 0; /* points to the first pixel in the pixel index */

	for (col = 0; col < roi_cols; col++)
	{
	    if (col == 0) /* 1st pixel in the row */
	    {
	        current_size = 1;
		
		/* initialise the pixel index with this pixel */

		pixel_index[0].above = TOP;
		pixel_index[0].below = BOTTOM;
		pixel_index[0].value = *fresh_ptr;
		pixel_index[0].rank = 0;
		rank_index[0] = 0;

		/* output the input pixel unaltered */

		*output_ptr = *fresh_ptr++;
	    }
	    else if (col < half_size) /* pixel lies in near edge area */
	    {
		for (count = 0; count < 2; count++)
		{
		    /* expand window by appending new pixel */

		    current_size++;
		    expand_window_B (current_size,
				     *fresh_ptr,
				     rank_index,
				     pixel_index);
		    fresh_ptr++;
		}
		/* output pixel value dependant on current size */
		
		current_rank = (rank * current_size / size) + 0.5;

		*output_ptr = 
		  pixel_index[rank_index[current_rank]].value;
	    }
	    else if (col == roi_cols - 1) /* last pixel in the row */
	    {
		/* output the input pixel unaltered */

		*output_ptr = *(fresh_ptr - 1);
	    }
	    else if (col > roi_cols - half_size) /* pixel in far edge area */
	    {
		for (count = 0; count < 2; count++)
		{
		    /* shrink window by removing stale pixel */
		    /* Note: fresh pixel points to stale pixel also */

		    shrink_window_B (current_size,
				     fresh_pixel,
				     rank_index,
				     pixel_index);
		    current_size--;
		    fresh_pixel = (fresh_pixel + 1) % size;
		}
		/* output pixel value dependant on current size */
		
		current_rank = (rank * current_size / size) + 0.5;

		*output_ptr = 
		  pixel_index[rank_index[current_rank]].value;
	    }
	    else /* pixel lies in the body of the row */
	    {
		insert_fresh_B (size,
				fresh_pixel,
				*fresh_ptr,
				rank_index,
				pixel_index);		
		
		fresh_pixel = (fresh_pixel + 1) % size;
		fresh_ptr++;
		*output_ptr = pixel_index[rank_index[rank]].value;
	    }
	    output_ptr++;

	} /* for each col */

	/* Move to the start of the next row of the output and input roi */

	output_ptr += output_offset;
	fresh_ptr += input_offset;

    } /* for each row */

    return (HIPS_OK);
}

/****************************** h_ver_rank_B **********************************
*									      *
* Description:	1D rank order byte format image in the vertical dimension.    *
*									      *
* Parameters:	input_roi	*byte,					      *
*			pointer to the start of the input region of interest. *
*		output_roi,	*byte,					      *
*			pointer to the start of the output roi.		      *
*	        roi_rows,	int,					      *
*			number of rows in the roi.			      *
*		roi_cols,	int,					      *
*			number of columns in the roi.			      *
*	  	input_cols,	int,					      *
*			number of columns in the entire input frame.	      *
*	  	output_cols,	int,					      *
*			number of columns in the entire output frame.	      *
*		size		int,	       				      *
*			the size of the vertical filtering kernal.	      *
*		rank		int,	       			   	      *
*			the rank for filtering with the vertical kernal.      *
*									      *
* Return value: int,							      *
*			HIPS_ERROR - an error has occured.		      *
*			HIPS_OK    - successful execution.		      *
*									      *
* Functions Called:	expand_window_B,      			       	      *
*			shrink_window_B,	       			      *
*			insert_fresh_B.					      *
*									      *
* External Functions:	perr.						      *
*									      *
* External Parameters:	none.						      *
*									      *
* Method: each column of the input frame is scanned with the filtering window.*
*         PIXEL_INDEX and RANK_INDEX hold the required information for each   *
*         pixel in the window. As the window moves down the column, the stale *
*         pixel (ie. the one just leaving the window) is removed from the     *
*         window and the fresh pixel (ie. the one just entering the window)   *
*         is inserted. The RANK_INDEX and PIXEL_INDEX are updated  	      *
*         appropriately and the value of the pixel with the required rank is  *
*         outputed. For regions near the beginning of the column, where the   *
*         entire window cannot fit in, the window size is grown from 1 pixel  *
*         upto the required size as the window moves down the col. The value  *
*         of the pixel with the appropriate rank for the reduced window size  *
*         is outputed. In a similar manner the window size is shrunk down as  *
*         the window approaches the end of the column.			      *
*									      *
******************************************************************************/

int h_ver_rank_B (input_roi,
	      output_roi,
	      roi_rows,
	      roi_cols,
	      input_cols,
	      output_cols,
	      size,
	      rank)

byte 	*input_roi;
byte 	*output_roi;
int 	roi_rows;
int	roi_cols;
int	input_cols;
int	output_cols;
int	size;
int	rank;
{
    static	h_boolean		allocation = FALSE;
    static	int   		*rank_index;
    static	Pixel_info_b 	*pixel_index;

    int    col;             /* the roi column of the current pixel */
    int    row;             /* the roi row of the current pixel */
    int    current_size;    /* the current window size */
    int    current_rank;    /* the rank appropriate to the current size */
    int    half_size;       /* half the required window size */
    byte   *output_ptr;	    /* pointer to the current output pixel */
    byte   *fresh_ptr;      /* pointer to the "fresh" pixel in the i/p image */
    int    fresh_pixel;     /* pointer to the "fresh" pixel in pixel index */
    int    count;           /* counting variable for expanding/shrinking */

    if (allocation == FALSE)
    {
	/* allocate core for the rank index */
	
	if ((rank_index = (int *) calloc (size, sizeof(int))) == 0)
	  return (perr (HE_ALLOCSUBR, "h_ver_rank_B"));

	/* allocate core for the pixel index */
	
	if ((pixel_index = (Pixel_info_b *) 
	     calloc (size, sizeof(Pixel_info_b))) == 0)
	  return (perr (HE_ALLOCSUBR, "h_ver_rank_B"));

	allocation = TRUE;
    }

    half_size = (size + 1)/2;

    for (col = 0; col < roi_cols; col++)
    {
	/* Set positions to start of the current column */

	fresh_ptr = input_roi + col;
	output_ptr = output_roi + col;

	fresh_pixel = 0; /* points to the first pixel in the pixel index */

	for (row = 0; row < roi_rows; row++)
	{
	    if (row == 0) /* 1st pixel in the col */
	    {
	        current_size = 1;
		
		/* initialise the pixel index with this pixel */

		pixel_index[0].above = TOP;
		pixel_index[0].below = BOTTOM;
		pixel_index[0].value = *fresh_ptr;
		pixel_index[0].rank = 0;
		rank_index[0] = 0;

		/* output the input pixel unaltered */

		*output_ptr = *fresh_ptr;
	    }
	    else if (row < half_size) /* pixel lies in top edge area */
	    {
		/* move output_ptr down a row */
	    
		output_ptr += output_cols;

		/* Grow the window by two pixels */

		for (count = 0; count < 2; count++)
		{
		    /* expand window by appending new pixel */

		    /* move fresh_ptr to point to next pixel in the column */

		    fresh_ptr += input_cols;

		    current_size++;
		    expand_window_B (current_size,
				     *fresh_ptr,
				     rank_index,
				     pixel_index);
		}
		/* output pixel value dependant on current size */
		
		current_rank = (rank * current_size / size) + 0.5;

		*output_ptr = 
		  pixel_index[rank_index[current_rank]].value;
	    }
	    else if (row == roi_rows - 1) /* last pixel in the col */
	    {
		/* move output_ptr down a row */
	    
		output_ptr += output_cols;

		/* output the input pixel unaltered */

		*output_ptr = *fresh_ptr;
	    }
	    else if (row > roi_rows - half_size) /* pixel in bottom edge *
						  * area 		 */
	    {
		/* move output_ptr down a row */
	    
		output_ptr += output_cols;

		/* shrink window by 2 pixels */

		for (count = 0; count < 2; count++)
		{
		    /* shrink window by removing stale pixel */
		    /* Note: fresh pixel points to stale pixel also */

		    shrink_window_B (current_size,
				     fresh_pixel,
				     rank_index,
				     pixel_index);
		    current_size--;
		    fresh_pixel = (fresh_pixel + 1) % size;
		}
		/* output pixel value dependant on current size */
		
		current_rank = (rank * current_size / size) + 0.5;

		*output_ptr = 
		  pixel_index[rank_index[current_rank]].value;
	    }
	    else /* pixel lies in the body of the col */
	    {
		/* move fresh_ptr to point to next pixel in the column */

		fresh_ptr += input_cols;

		/* move output_ptr down a row */
	    
		output_ptr += output_cols;

		insert_fresh_B (size,
				fresh_pixel,
				*fresh_ptr,
				rank_index,
				pixel_index);		
		
		fresh_pixel = (fresh_pixel + 1) % size;

		*output_ptr = pixel_index[rank_index[rank]].value;
	    }

	} /* for each col */

    } /* for each row */

    return (HIPS_OK);
}
	   
/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                              EXPAND_WINDOW                                **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: expand_window_B
**
** Purpose: to expand the size of the filtering window by appending a new pixel
**          to the pixel index and rank index.
**
** Inputs: the routine accepts the following inputs:
**	       SIZE        - the pixel length of the expanded window,
**             FRESH_VALUE - the value of the pixel to be appended,
**             RANK_INDEX  - array containing the pixel position associated
**                           with each rank,
**             PIXEL_INDEX - array containing rank information for each 
**                           pixel in the window.
**
** Outputs: the routine outputs PIXEL_INDEX and RANK_INDEX updated to include
**          the new pixel.
**
** Method: the fresh pixel is appended into the TOP position in the pixel 
**         index. If its value is less than the value of the pixel which was
**         previously the TOP pixel, the fresh pixel is scanned down the pixel
**         index until it is inserted into the right place.
*/

int expand_window_B (size,
		 fresh_value,
		 rank_index,
		 pixel_index)

int    		size;
byte  		fresh_value;
int    		*rank_index;
Pixel_info_b	*pixel_index;
{
    int    fresh_pixel;  /* pixel in the pixel index to be inserted */
    int    top_pixel;    /* the pixel which has the highest rank */

    fresh_pixel = size - 1;
    top_pixel = rank_index[size - 2];
    pixel_index[fresh_pixel].value = fresh_value;
	
    /* compare the value of fresh pixel with the value of the current */
    /* top pixel */

    if (fresh_value < pixel_index[top_pixel].value)
    {
	/* top pixel remains top pixel */
	/* increment rank of top pixel due to increase in size */
	/* scan down until the fresh pixel is inserted */
	
	rank_index[fresh_pixel] = top_pixel;
	pixel_index[top_pixel].rank = fresh_pixel;
	/* Note: fresh_pixel is also the current highest rank */
	
	scan_down_B (size,
		     fresh_pixel,
		     pixel_index[top_pixel].below,
		     top_pixel,
		     rank_index,
		     pixel_index,
		     fresh_pixel - 1);
    }
    else
    {
	/* fresh pixel becomes top pixel */

	rank_index[fresh_pixel] = fresh_pixel;
	pixel_index[fresh_pixel].rank = fresh_pixel;
	/* Note: fresh_pixel is also the current highest rank */
	
	pixel_index[top_pixel].above = fresh_pixel;
	pixel_index[fresh_pixel].below = top_pixel;
	pixel_index[fresh_pixel].above = TOP;
    }
    return (HIPS_OK);
}

/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                              SHRINK_WINDOW                                **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: shrink_window_B
**
** Purpose: to shrink the size of the filtering window by removing the stale
**          pixel.
**
** Inputs: the routine accepts the following inputs:
**	       SIZE        - the pixel length of the window before shrinking,
**             FRESH_PIXEL - pointer to the fresh/stale pixel in pixel index,
**             RANK_INDEX  - array containing the pixel position associated
**                           with each rank,
**             PIXEL_INDEX - array containing rank information for each 
**                           pixel in the window.
**
** Outputs: the routine outputs RANK_INDEX and PIXEL_INDEX, updated to exclude
**          the stale pixel.
**
** Method: the stale pixel is given a value one greater than the current
**         highest value. The stale pixel is then scanned up the pixel index
**         until it is inserted into the TOP position. The pixel below 
**         the stale pixel is then made the TOP pixel, removing the stale 
**         pixel from the pixel index. The window size must be decremented 
**         outside the routine after it is called.
*/

int shrink_window_B (size,
		 fresh_pixel,
		 rank_index,
		 pixel_index)

int    		size;
int    		fresh_pixel;
int    		*rank_index;
Pixel_info_b	*pixel_index;
{
    int   top_pixel;    /* the pixel which currently has the highest rank */
    int   above_pixel;  /* the pixel above the stale pixel */
    int   below_pixel;  /* the pixel below the stale pixel */
    int   current_rank; /* the current_rank of the fresh pixel */

    /* fresh_pixel holds the stale pixel information */

    current_rank = pixel_index[fresh_pixel].rank;
    below_pixel  = pixel_index[fresh_pixel].below;
    above_pixel  = pixel_index[fresh_pixel].above;

    if (above_pixel != TOP)
    {
	top_pixel = rank_index[size - 1];
        pixel_index[fresh_pixel].value = 
	  pixel_index[top_pixel].value + 1;

	/* move above pixel down a rank due to the removal of stale pixel */

	pixel_index[above_pixel].rank--;
	rank_index[current_rank] = above_pixel;
	pixel_index[below_pixel].above = above_pixel;
	pixel_index[above_pixel].below = below_pixel;

	/* scan up until the stale pixel reaches the TOP */

	scan_up_B (size,
		   fresh_pixel,
		   above_pixel,
		   pixel_index[above_pixel].above,
		   rank_index,
		   pixel_index,
		   ++current_rank);
    }
    /* pixel below fresh pixel is now made the top pixel */

    pixel_index[pixel_index[fresh_pixel].below].above = TOP;

    return (HIPS_OK);
}
	
/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                              INSERT_FRESH                                 **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: insert_fresh_B
**
** Purpose: overwrites the "stale" pixel information in the pixel index with
**          the information for the "fresh" pixel, updating the whole of the
**          pixel index and rank index.
**
** Inputs: the routine accepts the following inputs:
**             SIZE          - the size of the filtering window,
**             FRESH_PIXEL   - pointer to the "fresh" pixel in the pixel index,
**	       FRESH_VALUE   - the pixel value of the "fresh" pixel,
**	       RANK_INDEX    - array containing the pixel position associated
**                             with each rank,
**	       PIXEL_INDEX   - array containing rank information for each 
**                             pixel in the window.
**
** Outputs: the routine updates RANK_INDEX and PIXEL_INDEX to include the
**          fresh pixel and exclude the stale pixel.
**
** Method: the fresh pixel is inserted into the stale pixel's position in the
**         pixel index. If the value of the fresh pixel is greater than the
**         pixel above it, the fresh pixel is scanned up the pixel index until
**         it is inserted in the correct place. If the value of the fresh pixel
**         is less than the value of the pixel below it, the fresh pixel is
**         scanned down the pixel index until it is inserted in the correct
**         place.
*/

int insert_fresh_B (size,
		fresh_pixel,
		fresh_value,
		rank_index,
		pixel_index)

int    		size;
int    		fresh_pixel;
byte  		fresh_value;
int    		*rank_index;
Pixel_info_b	*pixel_index;
{
    int   above_pixel;  /* the pixel above the stale pixel */
    int   below_pixel;  /* the pixel below the stale pixel */
    int   current_rank; /* the current_rank of the fresh pixel */

    /* fresh_pixel currently holds the stale pixel information */

    current_rank = pixel_index[fresh_pixel].rank;
    below_pixel = pixel_index[fresh_pixel].below;
    above_pixel = pixel_index[fresh_pixel].above;
    pixel_index[fresh_pixel].value = fresh_value;

    if (below_pixel == BOTTOM)
    { 
	if (fresh_value > pixel_index[above_pixel].value) 
	{
	    /* above pixel becomes new bottom pixel and is moved down a rank */

	    pixel_index[above_pixel].rank--;
	    rank_index[current_rank] = above_pixel;
	    pixel_index[above_pixel].below = BOTTOM;

	    /* scan up until the correct position for fresh pixel is found */

	    scan_up_B (size,
		       fresh_pixel,
		       above_pixel,
		       pixel_index[above_pixel].above,
		       rank_index,
		       pixel_index,
		       ++current_rank);
	}
	/* else fresh pixel overwrites stale pixel directly */
    }
    else if (above_pixel == TOP) 
    {
	if (fresh_value < pixel_index[below_pixel].value)
	{
	    /* below pixel becomes new top pixel and is moved up a rank */

	    pixel_index[below_pixel].rank++;
	    rank_index[current_rank] = below_pixel;
	    pixel_index[below_pixel].above = TOP;

	    /* scan down until the correct position for fresh pixel is found */

	    scan_down_B (size,
			 fresh_pixel,
			 pixel_index[below_pixel].below,
			 below_pixel,
			 rank_index,
			 pixel_index,
			 --current_rank);
	}
	/* else fresh pixel overwrites stale pixel directly */
    }
    else if (fresh_value < pixel_index[below_pixel].value)
    {
	/* move below pixel up a rank due to the removal of stale pixel */

	pixel_index[below_pixel].rank++;
	rank_index[current_rank] = below_pixel;
	pixel_index[below_pixel].above = above_pixel;
	pixel_index[above_pixel].below = below_pixel;

	/* scan down until the correct position for fresh pixel is found */

	scan_down_B (size,
		     fresh_pixel,
		     pixel_index[below_pixel].below,
		     below_pixel,
		     rank_index,
		     pixel_index,
		     --current_rank);
    }
    else if (fresh_value > pixel_index[above_pixel].value)
    {
	/* move above pixel down a rank due to the removal of stale pixel */

	pixel_index[above_pixel].rank--;
	rank_index[current_rank] = above_pixel;
	pixel_index[below_pixel].above = above_pixel;
	pixel_index[above_pixel].below = below_pixel;

	/* scan up until the correct position for fresh pixel is found */

	scan_up_B (size,
		   fresh_pixel,
		   above_pixel,
		   pixel_index[above_pixel].above,
		   rank_index,
		   pixel_index,
		   ++current_rank);
    }
    /* else fresh pixel overwrites stale pixel directly */

    return (HIPS_OK);
}

/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                               SCAN_DOWN                                   **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: scan_down_B
**
** Purpose: to recursively scan down through the pixel index until the correct
**          position for the fresh pixel is found. The fresh pixel is then
**	    inserted. The pixel index is updated as it is scanned.
**
** Inputs: the routine accepts the following inputs:
**             SIZE          - the size of the filtering window,
**             FRESH_PIXEL   - pointer to the "fresh" pixel in the pixel index,
**             BELOW_PIXEL   - pointer to the pixel below the "fresh" pixel,
**             ABOVE_PIXEL   - pointer to the pixel above the "fresh" pixel,
**	       RANK_INDEX    - array containing the pixel position associated
**                             with each rank,
**	       PIXEL_INDEX   - array containing rank information for each 
**                             pixel in the window,
**             CURRENT_RANK  - the current rank of the "fresh" pixel.
**
** Outputs: the routine updates RANK_INDEX and PIXEL_INDEX to include the
**          fresh pixel and exclude the stale pixel.
*/

int scan_down_B (size,
	     fresh_pixel,
	     below_pixel,
	     above_pixel,
	     rank_index,
	     pixel_index,
	     current_rank)

int    		size;
int    		fresh_pixel;
int    		below_pixel;
int    		above_pixel;
int    		*rank_index;
Pixel_info_b	*pixel_index;
int    current_rank;
{
    if (below_pixel == BOTTOM)
    {
	/* fresh pixel becomes the new bottom pixel */

	rank_index[0] = fresh_pixel;
	pixel_index[fresh_pixel].rank = 0;
	pixel_index[fresh_pixel].below = BOTTOM;
	pixel_index[fresh_pixel].above = above_pixel;
	pixel_index[above_pixel].below = fresh_pixel;
    }
    else if (pixel_index[fresh_pixel].value < pixel_index[below_pixel].value)
    {
	/* scan down again */

	rank_index[current_rank] = below_pixel;
	pixel_index[below_pixel].rank = current_rank;

	scan_down_B (size,
		     fresh_pixel,
		     pixel_index[below_pixel].below,
		     below_pixel,
		     rank_index,
		     pixel_index,
		     --current_rank);
    }
    else
    {
	/* fresh pixel is inserted in the current position */

	rank_index[current_rank] = fresh_pixel;
	pixel_index[fresh_pixel].rank = current_rank;
	pixel_index[fresh_pixel].below = below_pixel;
	pixel_index[below_pixel].above = fresh_pixel;
	pixel_index[fresh_pixel].above = above_pixel;
	pixel_index[above_pixel].below = fresh_pixel;
    }
    return (HIPS_OK);
}

/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                                SCAN_UP                                    **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: scan_up_B
**
** Purpose: to recursively scan up through the pixel index until the correct
**          position for the fresh pixel is found. The fresh pixel is then
**	    inserted. The pixel index is updated as it is scanned.
**
** Inputs: the routine accepts the following inputs:
**             SIZE          - the size of the filtering window,
**             FRESH_PIXEL   - pointer to the "fresh" pixel in the pixel index,
**             BELOW_PIXEL   - pointer to the pixel below the "fresh" pixel,
**             ABOVE_PIXEL   - pointer to the pixel above the "fresh" pixel,
**	       RANK_INDEX    - array containing the pixel position associated
**                             with each rank,
**	       PIXEL_INDEX   - array containing rank information for each 
**                             pixel in the window,
**             CURRENT_RANK  - the current rank of the "fresh" pixel.
**
** Outputs: the routine updates RANK_INDEX and PIXEL_INDEX to include the
**          fresh pixel and exclude the stale pixel.
*/

int scan_up_B (size,
	   fresh_pixel,
	   below_pixel,
	   above_pixel,
	   rank_index,
	   pixel_index,
	   current_rank)

int    		size;
int    		fresh_pixel;
int    		below_pixel;
int    		above_pixel;
int    		*rank_index;
Pixel_info_b 	*pixel_index;
int    current_rank;
{
    if (above_pixel == TOP)
    {
	/* fresh pixel becomes the new top pixel */

	rank_index[current_rank] = fresh_pixel;
	pixel_index[fresh_pixel].rank = current_rank;
	pixel_index[fresh_pixel].below = below_pixel;
	pixel_index[fresh_pixel].above = TOP;
	pixel_index[below_pixel].above = fresh_pixel;
    }
    else if (pixel_index[fresh_pixel].value > pixel_index[above_pixel].value)
    {
	/* scan up again */

	rank_index[current_rank] = above_pixel;
	pixel_index[above_pixel].rank = current_rank;

	scan_up_B (size,
		   fresh_pixel,
		   above_pixel,
		   pixel_index[above_pixel].above,
		   rank_index,
		   pixel_index,
		   ++current_rank);
    }
    else
    {
	/* fresh pixel is inserted in the current position */

	rank_index[current_rank] = fresh_pixel;
	pixel_index[fresh_pixel].rank = current_rank;
	pixel_index[fresh_pixel].below = below_pixel;
	pixel_index[below_pixel].above = fresh_pixel;
	pixel_index[fresh_pixel].above = above_pixel;
	pixel_index[above_pixel].below = fresh_pixel;
    }
    return (HIPS_OK);
}

/******************************** h_rank_s ************************************
*									      *
* Description:	1D rank order filters a PFSHORT format HIPS2 header image in  *
*		horizontal dimension.					      *
*									      *
* Parameters:	input_hdr	*header,				      *
*			pointer to the input image header.		      *
*		output_hdr	*header,				      *
*			pointer to the output image header.		      *
*		hor_size	int,	       				      *
*			the size of the horizontal filtering kernal.	      *
*		ver_size	int,	       				      *
*			the size of the vertical filtering kernal.	      *
*		hor_rank	int,	       			   	      *
*			the rank for filtering with the horizontal kernal.    *
*		ver_rank	int,	       			   	      *
*			the rank for filtering with the vertical kernal.      *
*									      *
* Return value: int,							      *
*			HIPS_ERROR - an error has occured.		      *
*			HIPS_OK    - successful execution.		      *
*									      *
* Functions Called:	h_rank_S,      					      *
*									      *
* External Functions:	none.						      *
*									      *
* External Parameters:	none.						      *
*									      *
******************************************************************************/

int h_rank_s (input_hdr,
	  output_hdr,	
	  hor_size,
	  ver_size,
	  hor_rank,
	  ver_rank)
     
struct 	header 	*input_hdr;
struct	header	*output_hdr;
int		hor_size;
int		ver_size;
int		hor_rank;
int		ver_rank;
{
    return (h_rank_S ((short *) input_hdr->firstpix,
		      (short *) output_hdr->firstpix,
		      input_hdr->rows,
		      input_hdr->cols,
		      input_hdr->ocols,
		      output_hdr->ocols,
		      hor_size,
		      ver_size,
		      hor_rank,
		      ver_rank));
}

/******************************** h_rank_S ************************************
*									      *
* Description:	rank order filters a short format image. 		      *
*									      *
* Parameters:	input_roi	*short,					      *
*			pointer to the start of the input region of interest. *
*		output_roi,	*short,					      *
*			pointer to the start of the output roi.		      *
*	        roi_rows,	int,					      *
*			number of rows in the roi.			      *
*		roi_cols,	int,					      *
*			number of columns in the roi.			      *
*	  	input_cols,	int,					      *
*			number of columns in the entire input frame.	      *
*	  	output_cols,	int,					      *
*			number of columns in the entire output frame.	      *
*		hor_size	int,	       				      *
*			the size of the horizontal filtering kernal.	      *
*		ver_size	int,	       				      *
*			the size of the vertical filtering kernal.	      *
*		hor_rank	int,	       			   	      *
*			the rank for filtering with the horizontal kernal.    *
*		ver_rank	int,	       			   	      *
*			the rank for filtering with the vertical kernal.      *
*									      *
* Return value: int,							      *
*			HIPS_ERROR - an error has occured.		      *
*			HIPS_OK    - successful execution.		      *
*									      *
* Functions Called:	h_hor_rank_S,	      			       	      *
*			h_ver_rank_S.					      *
*									      *
* External Functions:	h_copy_S.		       			      *
*									      *
******************************************************************************/

int h_rank_S (input_roi,
	  output_roi,
	  roi_rows,
	  roi_cols,
	  input_cols,
	  output_cols,
	  hor_size,
	  ver_size,
	  hor_rank,
	  ver_rank)

short 	*input_roi;
short 	*output_roi;
int 	roi_rows;
int	roi_cols;
int	input_cols;
int	output_cols;
int	hor_size;
int	ver_size;
int	hor_rank;
int	ver_rank;
{
    if (hor_size > 1)
    {
	if ((h_hor_rank_S (input_roi,
			   output_roi,
			   roi_rows,
			   roi_cols,
			   input_cols,
			   output_cols,
			   hor_size,
			   hor_rank - 1)) == HIPS_ERROR)
	  return (HIPS_ERROR);
    }
    if (ver_size > 1)
    {
	if (hor_size > 1)
	{
	    /* Copy the output from horizontal filtering onto the input for *
	     * vertical filtering.					    */

	    if ((h_copy_S (output_roi,
			   input_roi,
			   roi_rows,
			   roi_cols,
			   input_cols,
			   output_cols)) == HIPS_ERROR)
	      return (HIPS_ERROR);
	}

	if ((h_ver_rank_S (input_roi,
			   output_roi,
			   roi_rows,
			   roi_cols,
			   input_cols,
			   output_cols,
			   ver_size,
			   ver_rank - 1)) == HIPS_ERROR)
	  return (HIPS_ERROR);
    }
    return (HIPS_OK);
}

/****************************** h_hor_rank_S **********************************
*									      *
* Description:	1D rank order short format image in the horizontal dimension. *
*									      *
* Parameters:	input_roi	*short,					      *
*			pointer to the start of the input region of interest. *
*		output_roi,	*short,					      *
*			pointer to the start of the output roi.		      *
*	        roi_rows,	int,					      *
*			number of rows in the roi.			      *
*		roi_cols,	int,					      *
*			number of columns in the roi.			      *
*	  	input_cols,	int,					      *
*			number of columns in the entire input frame.	      *
*	  	output_cols,	int,					      *
*			number of columns in the entire output frame.	      *
*		size		int,	       				      *
*			the size of the horizontal filtering kernal.	      *
*		rank		int,	       			   	      *
*			the rank for filtering with the horizontal kernal.    *
*									      *
* Return value: int,							      *
*			HIPS_ERROR - an error has occured.		      *
*			HIPS_OK    - successful execution.		      *
*									      *
* Functions Called:	expand_window_S,	     		       	      *
*			shrink_window_S,	       			      *
*			insert_fresh_S.
*									      *
* External Functions:	perr.						      *
*									      *
* External Parameters:	none.						      *
*									      *
* Method: each row of the input frame is scanned with the filtering window.   *
*         PIXEL_INDEX and RANK_INDEX hold the required information for each   *
*         pixel in the window. As the window moves across the row, the stale  *
*         pixel (ie. the one just leaving the window) is removed from the     *
*         window and the fresh pixel (ie. the one just entering the window)   *
*         is inserted. The RANK_INDEX and PIXEL_INDEX are updated  	      *
*         appropriately and the value of the pixel with the required rank is  *
*         outputed. For regions near the beginning of the row, where the      *
*         entire window cannot fit in, the window size is grown from 1 pixel  *
*         upto the required size as the window moves along the row. The value *
*         of the pixel with the appropriate rank for the reduced window size  *
*         is outputed. In a similar manner the window size is shrunk down as  *
*         the window approaches the end of the row.			      *
*									      *
******************************************************************************/

int h_hor_rank_S (input_roi,
	      output_roi,
	      roi_rows,
	      roi_cols,
	      input_cols,
	      output_cols,
	      size,
	      rank)

short 	*input_roi;
short 	*output_roi;
int 	roi_rows;
int	roi_cols;
int	input_cols;
int	output_cols;
int	size;
int	rank;
{
    static	h_boolean		allocation = FALSE;
    static	int   		*rank_index;
    static	Pixel_info_s 	*pixel_index;

    int    col;             /* the roi column of the current pixel */
    int    row;             /* the roi row of the current pixel */
    int	   input_offset;    /* offset between end & start of input roi rows */
    int	   output_offset;   /* offset between end & start of output roi rows */
    int    current_size;    /* the current window size */
    int    current_rank;    /* the rank appropriate to the current size */
    int    half_size;       /* half the required window size */
    short  *output_ptr;	    /* pointer to the current output pixel */
    short  *fresh_ptr;      /* pointer to the "fresh" pixel in the i/p image */
    int    fresh_pixel;     /* pointer to the "fresh" pixel in pixel index */
    int    count;           /* counting variable for expanding/shrinking */

    if (allocation == FALSE)
    {
	/* allocate core for the rank index */
	
	if ((rank_index = (int *) calloc (size, sizeof(int))) == 0)
	  return (perr (HE_ALLOCSUBR, "h_hor_rank_S"));

	/* allocate core for the pixel index */
	
	if ((pixel_index = (Pixel_info_s *) 
	     calloc (size, sizeof(Pixel_info_s))) == 0)
	  return (perr (HE_ALLOCSUBR, "h_hor_rank_S"));

	allocation = TRUE;
    }

    half_size = (size + 1)/2;
    input_offset = input_cols - roi_cols;
    output_offset = output_cols - roi_cols;

    output_ptr = output_roi;
    fresh_ptr  = input_roi;

    for (row = 0; row < roi_rows; row++)
    {
	fresh_pixel = 0; /* points to the first pixel in the pixel index */

	for (col = 0; col < roi_cols; col++)
	{
	    if (col == 0) /* 1st pixel in the row */
	    {
	        current_size = 1;
		
		/* initialise the pixel index with this pixel */

		pixel_index[0].above = TOP;
		pixel_index[0].below = BOTTOM;
		pixel_index[0].value = *fresh_ptr;
		pixel_index[0].rank = 0;
		rank_index[0] = 0;

		/* output the input pixel unaltered */

		*output_ptr = *fresh_ptr++;
	    }
	    else if (col < half_size) /* pixel lies in near edge area */
	    {
		for (count = 0; count < 2; count++)
		{
		    /* expand window by appending new pixel */

		    current_size++;
		    expand_window_S (current_size,
				     *fresh_ptr,
				     rank_index,
				     pixel_index);
		    fresh_ptr++;
		}
		/* output pixel value dependant on current size */
		
		current_rank = (rank * current_size / size) + 0.5;

		*output_ptr = 
		  pixel_index[rank_index[current_rank]].value;
	    }
	    else if (col == roi_cols - 1) /* last pixel in the row */
	    {
		/* output the input pixel unaltered */

		*output_ptr = *(fresh_ptr - 1);
	    }
	    else if (col > roi_cols - half_size) /* pixel in far edge area */
	    {
		for (count = 0; count < 2; count++)
		{
		    /* shrink window by removing stale pixel */
		    /* Note: fresh pixel points to stale pixel also */

		    shrink_window_S (current_size,
				     fresh_pixel,
				     rank_index,
				     pixel_index);
		    current_size--;
		    fresh_pixel = (fresh_pixel + 1) % size;
		}
		/* output pixel value dependant on current size */
		
		current_rank = (rank * current_size / size) + 0.5;

		*output_ptr = 
		  pixel_index[rank_index[current_rank]].value;
	    }
	    else /* pixel lies in the body of the row */
	    {
		insert_fresh_S (size,
				fresh_pixel,
				*fresh_ptr,
				rank_index,
				pixel_index);		
		
		fresh_pixel = (fresh_pixel + 1) % size;
		fresh_ptr++;
		*output_ptr = pixel_index[rank_index[rank]].value;
	    }
	    output_ptr++;

	} /* for each col */

	/* Move to the start of the next row of the output and input roi */

	output_ptr += output_offset;
	fresh_ptr += input_offset;

    } /* for each row */

    return (HIPS_OK);
}

/****************************** h_ver_rank_S **********************************
*									      *
* Description:	1D rank order short format image in the vertical dimension.   *
*									      *
* Parameters:	input_roi	*short,					      *
*			pointer to the start of the input region of interest. *
*		output_roi,	*short,					      *
*			pointer to the start of the output roi.		      *
*	        roi_rows,	int,					      *
*			number of rows in the roi.			      *
*		roi_cols,	int,					      *
*			number of columns in the roi.			      *
*	  	input_cols,	int,					      *
*			number of columns in the entire input frame.	      *
*	  	output_cols,	int,					      *
*			number of columns in the entire output frame.	      *
*		size		int,	       				      *
*			the size of the vertical filtering kernal.	      *
*		rank		int,	       			   	      *
*			the rank for filtering with the vertical kernal.      *
*									      *
* Return value: int,							      *
*			HIPS_ERROR - an error has occured.		      *
*			HIPS_OK    - successful execution.		      *
*									      *
* Functions Called:	expand_window_S,       			       	      *
*			shrink_window_S,	       			      *
*			insert_fresh_S.					      *
*									      *
* External Functions:	perr.						      *
*									      *
* External Parameters:	none.						      *
*									      *
* Method: each column of the input frame is scanned with the filtering window.*
*         PIXEL_INDEX and RANK_INDEX hold the required information for each   *
*         pixel in the window. As the window moves down the column, the stale *
*         pixel (ie. the one just leaving the window) is removed from the     *
*         window and the fresh pixel (ie. the one just entering the window)   *
*         is inserted. The RANK_INDEX and PIXEL_INDEX are updated  	      *
*         appropriately and the value of the pixel with the required rank is  *
*         outputed. For regions near the beginning of the column, where the   *
*         entire window cannot fit in, the window size is grown from 1 pixel  *
*         upto the required size as the window moves down the col. The value  *
*         of the pixel with the appropriate rank for the reduced window size  *
*         is outputed. In a similar manner the window size is shrunk down as  *
*         the window approaches the end of the column.			      *
*									      *
******************************************************************************/

int h_ver_rank_S (input_roi,
	      output_roi,
	      roi_rows,
	      roi_cols,
	      input_cols,
	      output_cols,
	      size,
	      rank)

short 	*input_roi;
short 	*output_roi;
int 	roi_rows;
int	roi_cols;
int	input_cols;
int	output_cols;
int	size;
int	rank;
{
    static	h_boolean		allocation = FALSE;
    static	int   		*rank_index;
    static	Pixel_info_s 	*pixel_index;

    int    col;             /* the roi column of the current pixel */
    int    row;             /* the roi row of the current pixel */
    int    current_size;    /* the current window size */
    int    current_rank;    /* the rank appropriate to the current size */
    int    half_size;       /* half the required window size */
    short  *output_ptr;	    /* pointer to the current output pixel */
    short  *fresh_ptr;      /* pointer to the "fresh" pixel in the i/p image */
    int    fresh_pixel;     /* pointer to the "fresh" pixel in pixel index */
    int    count;           /* counting variable for expanding/shrinking */

    if (allocation == FALSE)
    {
	/* allocate core for the rank index */
	
	if ((rank_index = (int *) calloc (size, sizeof(int))) == 0)
	  return (perr (HE_ALLOCSUBR, "h_ver_rank_S"));

	/* allocate core for the pixel index */
	
	if ((pixel_index = (Pixel_info_s *) 
	     calloc (size, sizeof(Pixel_info_s))) == 0)
	  return (perr (HE_ALLOCSUBR, "h_ver_rank_S"));

	allocation = TRUE;
    }

    half_size = (size + 1)/2;

    for (col = 0; col < roi_cols; col++)
    {
	/* Set positions to start of the current column */

	fresh_ptr = input_roi + col;
	output_ptr = output_roi + col;

	fresh_pixel = 0; /* points to the first pixel in the pixel index */

	for (row = 0; row < roi_rows; row++)
	{
	    if (row == 0) /* 1st pixel in the col */
	    {
	        current_size = 1;
		
		/* initialise the pixel index with this pixel */

		pixel_index[0].above = TOP;
		pixel_index[0].below = BOTTOM;
		pixel_index[0].value = *fresh_ptr;
		pixel_index[0].rank = 0;
		rank_index[0] = 0;

		/* output the input pixel unaltered */

		*output_ptr = *fresh_ptr;
	    }
	    else if (row < half_size) /* pixel lies in top edge area */
	    {
		/* move output_ptr down a row */
	    
		output_ptr += output_cols;

		/* Grow the window by two pixels */

		for (count = 0; count < 2; count++)
		{
		    /* expand window by appending new pixel */

		    /* move fresh_ptr to point to next pixel in the column */

		    fresh_ptr += input_cols;

		    current_size++;
		    expand_window_S (current_size,
				     *fresh_ptr,
				     rank_index,
				     pixel_index);
		}
		/* output pixel value dependant on current size */
		
		current_rank = (rank * current_size / size) + 0.5;

		*output_ptr = 
		  pixel_index[rank_index[current_rank]].value;
	    }
	    else if (row == roi_rows - 1) /* last pixel in the col */
	    {
		/* move output_ptr down a row */
	    
		output_ptr += output_cols;

		/* output the input pixel unaltered */

		*output_ptr = *fresh_ptr;
	    }
	    else if (row > roi_rows - half_size) /* pixel in bottom edge *
						  * area 		 */
	    {
		/* move output_ptr down a row */
	    
		output_ptr += output_cols;

		/* shrink window by 2 pixels */

		for (count = 0; count < 2; count++)
		{
		    /* shrink window by removing stale pixel */
		    /* Note: fresh pixel points to stale pixel also */

		    shrink_window_S (current_size,
				     fresh_pixel,
				     rank_index,
				     pixel_index);
		    current_size--;
		    fresh_pixel = (fresh_pixel + 1) % size;
		}
		/* output pixel value dependant on current size */
		
		current_rank = (rank * current_size / size) + 0.5;

		*output_ptr = 
		  pixel_index[rank_index[current_rank]].value;
	    }
	    else /* pixel lies in the body of the col */
	    {
		/* move fresh_ptr to point to next pixel in the column */

		fresh_ptr += input_cols;

		/* move output_ptr down a row */
	    
		output_ptr += output_cols;

		insert_fresh_S (size,
				fresh_pixel,
				*fresh_ptr,
				rank_index,
				pixel_index);		
		
		fresh_pixel = (fresh_pixel + 1) % size;

		*output_ptr = pixel_index[rank_index[rank]].value;
	    }

	} /* for each col */

    } /* for each row */

    return (HIPS_OK);
}
	   
/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                              EXPAND_WINDOW                                **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: expand_window_S
**
** Purpose: to expand the size of the filtering window by appending a new pixel
**          to the pixel index and rank index.
**
** Inputs: the routine accepts the following inputs:
**	       SIZE        - the pixel length of the expanded window,
**             FRESH_VALUE - the value of the pixel to be appended,
**             RANK_INDEX  - array containing the pixel position associated
**                           with each rank,
**             PIXEL_INDEX - array containing rank information for each 
**                           pixel in the window.
**
** Outputs: the routine outputs PIXEL_INDEX and RANK_INDEX updated to include
**          the new pixel.
**
** Method: the fresh pixel is appended into the TOP position in the pixel 
**         index. If its value is less than the value of the pixel which was
**         previously the TOP pixel, the fresh pixel is scanned down the pixel
**         index until it is inserted into the right place.
*/

int expand_window_S (size,
		 fresh_value,
		 rank_index,
		 pixel_index)

int    		size;
short  		fresh_value;
int    		*rank_index;
Pixel_info_s	*pixel_index;
{
    int    fresh_pixel;  /* pixel in the pixel index to be inserted */
    int    top_pixel;    /* the pixel which has the highest rank */

    fresh_pixel = size - 1;
    top_pixel = rank_index[size - 2];
    pixel_index[fresh_pixel].value = fresh_value;
	
    /* compare the value of fresh pixel with the value of the current */
    /* top pixel */

    if (fresh_value < pixel_index[top_pixel].value)
    {
	/* top pixel remains top pixel */
	/* increment rank of top pixel due to increase in size */
	/* scan down until the fresh pixel is inserted */
	
	rank_index[fresh_pixel] = top_pixel;
	pixel_index[top_pixel].rank = fresh_pixel;
	/* Note: fresh_pixel is also the current highest rank */
	
	scan_down_S (size,
		     fresh_pixel,
		     pixel_index[top_pixel].below,
		     top_pixel,
		     rank_index,
		     pixel_index,
		     fresh_pixel - 1);
    }
    else
    {
	/* fresh pixel becomes top pixel */

	rank_index[fresh_pixel] = fresh_pixel;
	pixel_index[fresh_pixel].rank = fresh_pixel;
	/* Note: fresh_pixel is also the current highest rank */
	
	pixel_index[top_pixel].above = fresh_pixel;
	pixel_index[fresh_pixel].below = top_pixel;
	pixel_index[fresh_pixel].above = TOP;
    }
    return (HIPS_OK);
}

/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                              SHRINK_WINDOW                                **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: shrink_window_S
**
** Purpose: to shrink the size of the filtering window by removing the stale
**          pixel.
**
** Inputs: the routine accepts the following inputs:
**	       SIZE        - the pixel length of the window before shrinking,
**             FRESH_PIXEL - pointer to the fresh/stale pixel in pixel index,
**             RANK_INDEX  - array containing the pixel position associated
**                           with each rank,
**             PIXEL_INDEX - array containing rank information for each 
**                           pixel in the window.
**
** Outputs: the routine outputs RANK_INDEX and PIXEL_INDEX, updated to exclude
**          the stale pixel.
**
** Method: the stale pixel is given a value one greater than the current
**         highest value. The stale pixel is then scanned up the pixel index
**         until it is inserted into the TOP position. The pixel below 
**         the stale pixel is then made the TOP pixel, removing the stale 
**         pixel from the pixel index. The window size must be decremented 
**         outside the routine after it is called.
*/

int shrink_window_S (size,
		 fresh_pixel,
		 rank_index,
		 pixel_index)

int    		size;
int    		fresh_pixel;
int    		*rank_index;
Pixel_info_s	*pixel_index;
{
    int   top_pixel;    /* the pixel which currently has the highest rank */
    int   above_pixel;  /* the pixel above the stale pixel */
    int   below_pixel;  /* the pixel below the stale pixel */
    int   current_rank; /* the current_rank of the fresh pixel */

    /* fresh_pixel holds the stale pixel information */

    current_rank = pixel_index[fresh_pixel].rank;
    below_pixel  = pixel_index[fresh_pixel].below;
    above_pixel  = pixel_index[fresh_pixel].above;

    if (above_pixel != TOP)
    {
	top_pixel = rank_index[size - 1];
        pixel_index[fresh_pixel].value = 
	  pixel_index[top_pixel].value + 1;

	/* move above pixel down a rank due to the removal of stale pixel */

	pixel_index[above_pixel].rank--;
	rank_index[current_rank] = above_pixel;
	pixel_index[below_pixel].above = above_pixel;
	pixel_index[above_pixel].below = below_pixel;

	/* scan up until the stale pixel reaches the TOP */

	scan_up_S (size,
		   fresh_pixel,
		   above_pixel,
		   pixel_index[above_pixel].above,
		   rank_index,
		   pixel_index,
		   ++current_rank);
    }
    /* pixel below fresh pixel is now made the top pixel */

    pixel_index[pixel_index[fresh_pixel].below].above = TOP;

    return (HIPS_OK);
}
	
/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                              INSERT_FRESH                                 **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: insert_fresh_S
**
** Purpose: overwrites the "stale" pixel information in the pixel index with
**          the information for the "fresh" pixel, updating the whole of the
**          pixel index and rank index.
**
** Inputs: the routine accepts the following inputs:
**             SIZE          - the size of the filtering window,
**             FRESH_PIXEL   - pointer to the "fresh" pixel in the pixel index,
**	       FRESH_VALUE   - the pixel value of the "fresh" pixel,
**	       RANK_INDEX    - array containing the pixel position associated
**                             with each rank,
**	       PIXEL_INDEX   - array containing rank information for each 
**                             pixel in the window.
**
** Outputs: the routine updates RANK_INDEX and PIXEL_INDEX to include the
**          fresh pixel and exclude the stale pixel.
**
** Method: the fresh pixel is inserted into the stale pixel's position in the
**         pixel index. If the value of the fresh pixel is greater than the
**         pixel above it, the fresh pixel is scanned up the pixel index until
**         it is inserted in the correct place. If the value of the fresh pixel
**         is less than the value of the pixel below it, the fresh pixel is
**         scanned down the pixel index until it is inserted in the correct
**         place.
*/

int insert_fresh_S (size,
		fresh_pixel,
		fresh_value,
		rank_index,
		pixel_index)

int    		size;
int    		fresh_pixel;
short  		fresh_value;
int    		*rank_index;
Pixel_info_s	*pixel_index;
{
    int   above_pixel;  /* the pixel above the stale pixel */
    int   below_pixel;  /* the pixel below the stale pixel */
    int   current_rank; /* the current_rank of the fresh pixel */

    /* fresh_pixel currently holds the stale pixel information */

    current_rank = pixel_index[fresh_pixel].rank;
    below_pixel = pixel_index[fresh_pixel].below;
    above_pixel = pixel_index[fresh_pixel].above;
    pixel_index[fresh_pixel].value = fresh_value;

    if (below_pixel == BOTTOM)
    { 
	if (fresh_value > pixel_index[above_pixel].value) 
	{
	    /* above pixel becomes new bottom pixel and is moved down a rank */

	    pixel_index[above_pixel].rank--;
	    rank_index[current_rank] = above_pixel;
	    pixel_index[above_pixel].below = BOTTOM;

	    /* scan up until the correct position for fresh pixel is found */

	    scan_up_S (size,
		       fresh_pixel,
		       above_pixel,
		       pixel_index[above_pixel].above,
		       rank_index,
		       pixel_index,
		       ++current_rank);
	}
	/* else fresh pixel overwrites stale pixel directly */
    }
    else if (above_pixel == TOP) 
    {
	if (fresh_value < pixel_index[below_pixel].value)
	{
	    /* below pixel becomes new top pixel and is moved up a rank */

	    pixel_index[below_pixel].rank++;
	    rank_index[current_rank] = below_pixel;
	    pixel_index[below_pixel].above = TOP;

	    /* scan down until the correct position for fresh pixel is found */

	    scan_down_S (size,
			 fresh_pixel,
			 pixel_index[below_pixel].below,
			 below_pixel,
			 rank_index,
			 pixel_index,
			 --current_rank);
	}
	/* else fresh pixel overwrites stale pixel directly */
    }
    else if (fresh_value < pixel_index[below_pixel].value)
    {
	/* move below pixel up a rank due to the removal of stale pixel */

	pixel_index[below_pixel].rank++;
	rank_index[current_rank] = below_pixel;
	pixel_index[below_pixel].above = above_pixel;
	pixel_index[above_pixel].below = below_pixel;

	/* scan down until the correct position for fresh pixel is found */

	scan_down_S (size,
		     fresh_pixel,
		     pixel_index[below_pixel].below,
		     below_pixel,
		     rank_index,
		     pixel_index,
		     --current_rank);
    }
    else if (fresh_value > pixel_index[above_pixel].value)
    {
	/* move above pixel down a rank due to the removal of stale pixel */

	pixel_index[above_pixel].rank--;
	rank_index[current_rank] = above_pixel;
	pixel_index[below_pixel].above = above_pixel;
	pixel_index[above_pixel].below = below_pixel;

	/* scan up until the correct position for fresh pixel is found */

	scan_up_S (size,
		   fresh_pixel,
		   above_pixel,
		   pixel_index[above_pixel].above,
		   rank_index,
		   pixel_index,
		   ++current_rank);
    }
    /* else fresh pixel overwrites stale pixel directly */

    return (HIPS_OK);
}

/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                               SCAN_DOWN                                   **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: scan_down_S
**
** Purpose: to recursively scan down through the pixel index until the correct
**          position for the fresh pixel is found. The fresh pixel is then
**	    inserted. The pixel index is updated as it is scanned.
**
** Inputs: the routine accepts the following inputs:
**             SIZE          - the size of the filtering window,
**             FRESH_PIXEL   - pointer to the "fresh" pixel in the pixel index,
**             BELOW_PIXEL   - pointer to the pixel below the "fresh" pixel,
**             ABOVE_PIXEL   - pointer to the pixel above the "fresh" pixel,
**	       RANK_INDEX    - array containing the pixel position associated
**                             with each rank,
**	       PIXEL_INDEX   - array containing rank information for each 
**                             pixel in the window,
**             CURRENT_RANK  - the current rank of the "fresh" pixel.
**
** Outputs: the routine updates RANK_INDEX and PIXEL_INDEX to include the
**          fresh pixel and exclude the stale pixel.
*/

int scan_down_S (size,
	     fresh_pixel,
	     below_pixel,
	     above_pixel,
	     rank_index,
	     pixel_index,
	     current_rank)

int    		size;
int    		fresh_pixel;
int    		below_pixel;
int    		above_pixel;
int    		*rank_index;
Pixel_info_s	*pixel_index;
int    current_rank;
{
    if (below_pixel == BOTTOM)
    {
	/* fresh pixel becomes the new bottom pixel */

	rank_index[0] = fresh_pixel;
	pixel_index[fresh_pixel].rank = 0;
	pixel_index[fresh_pixel].below = BOTTOM;
	pixel_index[fresh_pixel].above = above_pixel;
	pixel_index[above_pixel].below = fresh_pixel;
    }
    else if (pixel_index[fresh_pixel].value < pixel_index[below_pixel].value)
    {
	/* scan down again */

	rank_index[current_rank] = below_pixel;
	pixel_index[below_pixel].rank = current_rank;

	scan_down_S (size,
		     fresh_pixel,
		     pixel_index[below_pixel].below,
		     below_pixel,
		     rank_index,
		     pixel_index,
		     --current_rank);
    }
    else
    {
	/* fresh pixel is inserted in the current position */

	rank_index[current_rank] = fresh_pixel;
	pixel_index[fresh_pixel].rank = current_rank;
	pixel_index[fresh_pixel].below = below_pixel;
	pixel_index[below_pixel].above = fresh_pixel;
	pixel_index[fresh_pixel].above = above_pixel;
	pixel_index[above_pixel].below = fresh_pixel;
    }
    return (HIPS_OK);
}

/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                                SCAN_UP                                    **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: scan_up_S
**
** Purpose: to recursively scan up through the pixel index until the correct
**          position for the fresh pixel is found. The fresh pixel is then
**	    inserted. The pixel index is updated as it is scanned.
**
** Inputs: the routine accepts the following inputs:
**             SIZE          - the size of the filtering window,
**             FRESH_PIXEL   - pointer to the "fresh" pixel in the pixel index,
**             BELOW_PIXEL   - pointer to the pixel below the "fresh" pixel,
**             ABOVE_PIXEL   - pointer to the pixel above the "fresh" pixel,
**	       RANK_INDEX    - array containing the pixel position associated
**                             with each rank,
**	       PIXEL_INDEX   - array containing rank information for each 
**                             pixel in the window,
**             CURRENT_RANK  - the current rank of the "fresh" pixel.
**
** Outputs: the routine updates RANK_INDEX and PIXEL_INDEX to include the
**          fresh pixel and exclude the stale pixel.
*/

int scan_up_S (size,
	   fresh_pixel,
	   below_pixel,
	   above_pixel,
	   rank_index,
	   pixel_index,
	   current_rank)

int    		size;
int    		fresh_pixel;
int    		below_pixel;
int    		above_pixel;
int    		*rank_index;
Pixel_info_s 	*pixel_index;
int    current_rank;
{
    if (above_pixel == TOP)
    {
	/* fresh pixel becomes the new top pixel */

	rank_index[current_rank] = fresh_pixel;
	pixel_index[fresh_pixel].rank = current_rank;
	pixel_index[fresh_pixel].below = below_pixel;
	pixel_index[fresh_pixel].above = TOP;
	pixel_index[below_pixel].above = fresh_pixel;
    }
    else if (pixel_index[fresh_pixel].value > pixel_index[above_pixel].value)
    {
	/* scan up again */

	rank_index[current_rank] = above_pixel;
	pixel_index[above_pixel].rank = current_rank;

	scan_up_S (size,
		   fresh_pixel,
		   above_pixel,
		   pixel_index[above_pixel].above,
		   rank_index,
		   pixel_index,
		   ++current_rank);
    }
    else
    {
	/* fresh pixel is inserted in the current position */

	rank_index[current_rank] = fresh_pixel;
	pixel_index[fresh_pixel].rank = current_rank;
	pixel_index[fresh_pixel].below = below_pixel;
	pixel_index[below_pixel].above = fresh_pixel;
	pixel_index[fresh_pixel].above = above_pixel;
	pixel_index[above_pixel].below = fresh_pixel;
    }
    return (HIPS_OK);
}

/******************************** h_rank_i ************************************
*									      *
* Description:	rank order filters a PFINT format HIPS2 header image.	      *
*									      *
* Parameters:	input_hdr	*header,				      *
*			pointer to the input image header.		      *
*		output_hdr	*header,				      *
*			pointer to the output image header.		      *
*		hor_size	int,	       				      *
*			the size of the horizontal filtering kernal.	      *
*		ver_size	int,	       				      *
*			the size of the vertical filtering kernal.	      *
*		hor_rank	int,	       			   	      *
*			the rank for filtering with the horizontal kernal.    *
*		ver_rank	int,	       			   	      *
*			the rank for filtering with the vertical kernal.      *
*									      *
* Return value: int,							      *
*			HIPS_ERROR - an error has occured.		      *
*			HIPS_OK    - successful execution.		      *
*									      *
* Functions Called:	h_rank_I,      					      *
*									      *
* External Functions:	none.						      *
*									      *
* External Parameters:	none.						      *
*									      *
******************************************************************************/

int h_rank_i (input_hdr,
	  output_hdr,	
	  hor_size,
	  ver_size,
	  hor_rank,
	  ver_rank)
     
struct 	header 	*input_hdr;
struct	header	*output_hdr;
int		hor_size;
int		ver_size;
int		hor_rank;
int		ver_rank;
{
    return (h_rank_I ((int *) input_hdr->firstpix,
		      (int *) output_hdr->firstpix,
		      input_hdr->rows,
		      input_hdr->cols,
		      input_hdr->ocols,
		      output_hdr->ocols,
		      hor_size,
		      ver_size,
		      hor_rank,
		      ver_rank));
}

/******************************** h_rank_I ************************************
*									      *
* Description:	1D rank order filters a int format image.		      *
*									      *
* Parameters:	input_roi	*int,			              	      *
*			pointer to the start of the input region of interest. *
*		output_roi,	*int,		       		      	      *
*			pointer to the start of the output roi.		      *
*	        roi_rows,	int,					      *
*			number of rows in the roi.			      *
*		roi_cols,	int,					      *
*			number of columns in the roi.			      *
*	  	input_cols,	int,					      *
*			number of columns in the entire input frame.	      *
*	  	output_cols,	int,					      *
*			number of columns in the entire output frame.	      *
*		hor_size	int,	       				      *
*			the size of the horizontal filtering kernal.	      *
*		ver_size	int,	       				      *
*			the size of the vertical filtering kernal.	      *
*		hor_rank	int,	       			   	      *
*			the rank for filtering with the horizontal kernal.    *
*		ver_rank	int,	       			   	      *
*			the rank for filtering with the vertical kernal.      *
*									      *
* Return value: int,							      *
*			HIPS_ERROR - an error has occured.		      *
*			HIPS_OK    - successful execution.		      *
*									      *
* Functions Called:	h_hor_rank_I,	      			       	      *
*			h_ver_rank_I.					      *
*									      *
* External Functions:	h_copy_I.		       			      *
*									      *
******************************************************************************/

int h_rank_I (input_roi,
	  output_roi,
	  roi_rows,
	  roi_cols,
	  input_cols,
	  output_cols,
	  hor_size,
	  ver_size,
	  hor_rank,
	  ver_rank)

int 	*input_roi;
int 	*output_roi;
int 	roi_rows;
int	roi_cols;
int	input_cols;
int	output_cols;
int	hor_size;
int	ver_size;
int	hor_rank;
int	ver_rank;
{
    if (hor_size > 1)
    {
	if ((h_hor_rank_I (input_roi,
			   output_roi,
			   roi_rows,
			   roi_cols,
			   input_cols,
			   output_cols,
			   hor_size,
			   hor_rank - 1)) == HIPS_ERROR)
	  return (HIPS_ERROR);
    }
    if (ver_size > 1)
    {
	if (hor_size > 1)
	{
	    /* Copy the output from horizontal filtering onto the input for *
	     * vertical filtering.					    */

	    if ((h_copy_I (output_roi,
			   input_roi,
			   roi_rows,
			   roi_cols,
			   input_cols,
			   output_cols)) == HIPS_ERROR)
	      return (HIPS_ERROR);
	}

	if ((h_ver_rank_I (input_roi,
			   output_roi,
			   roi_rows,
			   roi_cols,
			   input_cols,
			   output_cols,
			   ver_size,
			   ver_rank - 1)) == HIPS_ERROR)
	  return (HIPS_ERROR);
    }
    return (HIPS_OK);
}

/****************************** h_hor_rank_I **********************************
*									      *
* Description:	1D rank order filters a int format image in the horizontal *
*	        dimension.						      *
*						       			      *
* Parameters:	input_roi	*int,		      	 		      *
*			pointer to the start of the input region of interest. *
*		output_roi,	*int,		   	    		      *
*			pointer to the start of the output roi.		      *
*	        roi_rows,	int,					      *
*			number of rows in the roi.			      *
*		roi_cols,	int,					      *
*			number of columns in the roi.			      *
*	  	input_cols,	int,					      *
*			number of columns in the entire input frame.	      *
*	  	output_cols,	int,					      *
*			number of columns in the entire output frame.	      *
*		size		int,	       				      *
*			the size of the horizontal filtering kernal.	      *
*		rank		int,	       			   	      *
*			the rank for filtering with the horizontal kernal.    *
*									      *
* Return value: int,							      *
*			HIPS_ERROR - an error has occured.		      *
*			HIPS_OK    - successful execution.		      *
*									      *
* Functions Called:	expand_window_I,	      		       	      *
*			shrink_window_I,			              *
*			insert_fresh_I.					      *
*									      *
* External Functions:	perr.						      *
*									      *
* External Parameters:	none.						      *
*									      *
* Method: each row of the input frame is scanned with the filtering window.   *
*         PIXEL_INDEX and RANK_INDEX hold the required information for each   *
*         pixel in the window. As the window moves across the row, the stale  *
*         pixel (ie. the one just leaving the window) is removed from the     *
*         window and the fresh pixel (ie. the one just entering the window)   *
*         is inserted. The RANK_INDEX and PIXEL_INDEX are updated  	      *
*         appropriately and the value of the pixel with the required rank is  *
*         outputed. For regions near the beginning of the row, where the      *
*         entire window cannot fit in, the window size is grown from 1 pixel  *
*         upto the required size as the window moves along the row. The value *
*         of the pixel with the appropriate rank for the reduced window size  *
*         is outputed. In a similar manner the window size is shrunk down as  *
*         the window approaches the end of the row.			      *
*									      *
******************************************************************************/

int h_hor_rank_I (input_roi,
	      output_roi,
	      roi_rows,
	      roi_cols,
	      input_cols,
	      output_cols,
	      size,
	      rank)

int 	*input_roi;
int 	*output_roi;
int 	roi_rows;
int	roi_cols;
int	input_cols;
int	output_cols;
int	size;
int	rank;
{
    static	h_boolean		allocation = FALSE;
    static	int   		*rank_index;
    static	Pixel_info_i 	*pixel_index;

    int    col;             /* the roi column of the current pixel */
    int    row;             /* the roi row of the current pixel */
    int	   input_offset;    /* offset between end & start of input roi rows */
    int	   output_offset;   /* offset between end & start of output roi rows */
    int    current_size;    /* the current window size */
    int    current_rank;    /* the rank appropriate to the current size */
    int    half_size;       /* half the required window size */
    int	   *output_ptr;	    /* pointer to the current output pixel */
    int	   *fresh_ptr;      /* pointer to the "fresh" pixel in the i/p image */
    int    fresh_pixel;     /* pointer to the "fresh" pixel in pixel index */
    int    count;           /* counting variable for expanding/shrinking */

    if (allocation == FALSE)
    {
	/* allocate core for the rank index */
	
	if ((rank_index = (int *) calloc (size, sizeof(int))) == 0)
	  return (perr (HE_ALLOCSUBR, "h_hor_rank_I"));

	/* allocate core for the pixel index */
	
	if ((pixel_index = (Pixel_info_i *) 
	     calloc (size, sizeof(Pixel_info_i))) == 0)
	  return (perr (HE_ALLOCSUBR, "h_hor_rank_I"));

	allocation = TRUE;
    }

    half_size = (size + 1)/2;
    input_offset = input_cols - roi_cols;
    output_offset = output_cols - roi_cols;

    output_ptr = output_roi;
    fresh_ptr  = input_roi;

    for (row = 0; row < roi_rows; row++)
    {
	fresh_pixel = 0; /* points to the first pixel in the pixel index */

	for (col = 0; col < roi_cols; col++)
	{
	    if (col == 0) /* 1st pixel in the row */
	    {
	        current_size = 1;
		
		/* initialise the pixel index with this pixel */

		pixel_index[0].above = TOP;
		pixel_index[0].below = BOTTOM;
		pixel_index[0].value = *fresh_ptr;
		pixel_index[0].rank = 0;
		rank_index[0] = 0;

		/* output the input pixel unaltered */

		*output_ptr = *fresh_ptr++;
	    }
	    else if (col < half_size) /* pixel lies in near edge area */
	    {
		for (count = 0; count < 2; count++)
		{
		    /* expand window by appending new pixel */

		    current_size++;
		    expand_window_I (current_size,
				     *fresh_ptr,
				     rank_index,
				     pixel_index);
		    fresh_ptr++;
		}
		/* output pixel value dependant on current size */
		
		current_rank = (rank * current_size / size) + 0.5;

		*output_ptr = 
		  pixel_index[rank_index[current_rank]].value;
	    }
	    else if (col == roi_cols - 1) /* last pixel in the row */
	    {
		/* output the input pixel unaltered */

		*output_ptr = *(fresh_ptr - 1);
	    }
	    else if (col > roi_cols - half_size) /* pixel in far edge area */
	    {
		for (count = 0; count < 2; count++)
		{
		    /* shrink window by removing stale pixel */
		    /* Note: fresh pixel points to stale pixel also */

		    shrink_window_I (current_size,
				     fresh_pixel,
				     rank_index,
				     pixel_index);
		    current_size--;
		    fresh_pixel = (fresh_pixel + 1) % size;
		}
		/* output pixel value dependant on current size */
		
		current_rank = (rank * current_size / size) + 0.5;

		*output_ptr = 
		  pixel_index[rank_index[current_rank]].value;
	    }
	    else /* pixel lies in the body of the row */
	    {
		insert_fresh_I (size,
				fresh_pixel,
				*fresh_ptr,
				rank_index,
				pixel_index);		
		
		fresh_pixel = (fresh_pixel + 1) % size;
		fresh_ptr++;
		*output_ptr = pixel_index[rank_index[rank]].value;
	    }
	    output_ptr++;

	} /* for each col */

	/* Move to the start of the next row of the output and input roi */

	output_ptr += output_offset;
	fresh_ptr += input_offset;

    } /* for each row */

    return (HIPS_OK);
}

/****************************** h_ver_rank_I **********************************
*									      *
* Description:	1D rank order int format image in the vertical dimension.     *
*									      *
* Parameters:	input_roi	*int,					      *
*			pointer to the start of the input region of interest. *
*		output_roi,	*int,					      *
*			pointer to the start of the output roi.		      *
*	        roi_rows,	int,					      *
*			number of rows in the roi.			      *
*		roi_cols,	int,					      *
*			number of columns in the roi.			      *
*	  	input_cols,	int,					      *
*			number of columns in the entire input frame.	      *
*	  	output_cols,	int,					      *
*			number of columns in the entire output frame.	      *
*		size		int,	       				      *
*			the size of the vertical filtering kernal.	      *
*		rank		int,	       			   	      *
*			the rank for filtering with the vertical kernal.      *
*									      *
* Return value: int,							      *
*			HIPS_ERROR - an error has occured.		      *
*			HIPS_OK    - successful execution.		      *
*									      *
* Functions Called:	expand_window_I,       			       	      *
*			shrink_window_I,		      		      *
*			insert_fresh_I.					      *
*									      *
* External Functions:	perr.						      *
*									      *
* External Parameters:	none.						      *
*									      *
* Method: each column of the input frame is scanned with the filtering window.*
*         PIXEL_INDEX and RANK_INDEX hold the required information for each   *
*         pixel in the window. As the window moves down the column, the stale *
*         pixel (ie. the one just leaving the window) is removed from the     *
*         window and the fresh pixel (ie. the one just entering the window)   *
*         is inserted. The RANK_INDEX and PIXEL_INDEX are updated  	      *
*         appropriately and the value of the pixel with the required rank is  *
*         outputed. For regions near the beginning of the column, where the   *
*         entire window cannot fit in, the window size is grown from 1 pixel  *
*         upto the required size as the window moves down the col. The value  *
*         of the pixel with the appropriate rank for the reduced window size  *
*         is outputed. In a similar manner the window size is shrunk down as  *
*         the window approaches the end of the column.			      *
*									      *
******************************************************************************/

int h_ver_rank_I (input_roi,
	      output_roi,
	      roi_rows,
	      roi_cols,
	      input_cols,
	      output_cols,
	      size,
	      rank)

int 	*input_roi;
int 	*output_roi;
int 	roi_rows;
int	roi_cols;
int	input_cols;
int	output_cols;
int	size;
int	rank;
{
    static	h_boolean		allocation = FALSE;
    static	int   		*rank_index;
    static	Pixel_info_i 	*pixel_index;

    int    col;             /* the roi column of the current pixel */
    int    row;             /* the roi row of the current pixel */
    int    current_size;    /* the current window size */
    int    current_rank;    /* the rank appropriate to the current size */
    int    half_size;       /* half the required window size */
    int	   *output_ptr;	    /* pointer to the current output pixel */
    int	   *fresh_ptr;      /* pointer to the "fresh" pixel in the i/p image */
    int    fresh_pixel;     /* pointer to the "fresh" pixel in pixel index */
    int    count;           /* counting variable for expanding/shrinking */

    if (allocation == FALSE)
    {
	/* allocate core for the rank index */
	
	if ((rank_index = (int *) calloc (size, sizeof(int))) == 0)
	  return (perr (HE_ALLOCSUBR, "h_ver_rank_I"));

	/* allocate core for the pixel index */
	
	if ((pixel_index = (Pixel_info_i *) 
	     calloc (size, sizeof(Pixel_info_i))) == 0)
	  return (perr (HE_ALLOCSUBR, "h_ver_rank_I"));

	allocation = TRUE;
    }

    half_size = (size + 1)/2;

    for (col = 0; col < roi_cols; col++)
    {
	/* Set positions to start of the current column */

	fresh_ptr = input_roi + col;
	output_ptr = output_roi + col;

	fresh_pixel = 0; /* points to the first pixel in the pixel index */

	for (row = 0; row < roi_rows; row++)
	{
	    if (row == 0) /* 1st pixel in the col */
	    {
	        current_size = 1;
		
		/* initialise the pixel index with this pixel */

		pixel_index[0].above = TOP;
		pixel_index[0].below = BOTTOM;
		pixel_index[0].value = *fresh_ptr;
		pixel_index[0].rank = 0;
		rank_index[0] = 0;

		/* output the input pixel unaltered */

		*output_ptr = *fresh_ptr;
	    }
	    else if (row < half_size) /* pixel lies in top edge area */
	    {
		/* move output_ptr down a row */
	    
		output_ptr += output_cols;

		/* Grow the window by two pixels */

		for (count = 0; count < 2; count++)
		{
		    /* expand window by appending new pixel */

		    /* move fresh_ptr to point to next pixel in the column */

		    fresh_ptr += input_cols;

		    current_size++;
		    expand_window_I (current_size,
				     *fresh_ptr,
				     rank_index,
				     pixel_index);
		}
		/* output pixel value dependant on current size */
		
		current_rank = (rank * current_size / size) + 0.5;

		*output_ptr = 
		  pixel_index[rank_index[current_rank]].value;
	    }
	    else if (row == roi_rows - 1) /* last pixel in the col */
	    {
		/* move output_ptr down a row */
	    
		output_ptr += output_cols;

		/* output the input pixel unaltered */

		*output_ptr = *fresh_ptr;
	    }
	    else if (row > roi_rows - half_size) /* pixel in bottom edge *
						  * area 		 */
	    {
		/* move output_ptr down a row */
	    
		output_ptr += output_cols;

		/* shrink window by 2 pixels */

		for (count = 0; count < 2; count++)
		{
		    /* shrink window by removing stale pixel */
		    /* Note: fresh pixel points to stale pixel also */

		    shrink_window_I (current_size,
				     fresh_pixel,
				     rank_index,
				     pixel_index);
		    current_size--;
		    fresh_pixel = (fresh_pixel + 1) % size;
		}
		/* output pixel value dependant on current size */
		
		current_rank = (rank * current_size / size) + 0.5;

		*output_ptr = 
		  pixel_index[rank_index[current_rank]].value;
	    }
	    else /* pixel lies in the body of the col */
	    {
		/* move fresh_ptr to point to next pixel in the column */

		fresh_ptr += input_cols;

		/* move output_ptr down a row */
	    
		output_ptr += output_cols;

		insert_fresh_I (size,
				fresh_pixel,
				*fresh_ptr,
				rank_index,
				pixel_index);		
		
		fresh_pixel = (fresh_pixel + 1) % size;

		*output_ptr = pixel_index[rank_index[rank]].value;
	    }

	} /* for each col */

    } /* for each row */

    return (HIPS_OK);
}
	   
/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                              EXPAND_WINDOW                                **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: expand_window_I
**
** Purpose: to expand the size of the filtering window by appending a new pixel
**          to the pixel index and rank index.
**
** Inputs: the routine accepts the following inputs:
**	       SIZE        - the pixel length of the expanded window,
**             FRESH_VALUE - the value of the pixel to be appended,
**             RANK_INDEX  - array containing the pixel position associated
**                           with each rank,
**             PIXEL_INDEX - array containing rank information for each 
**                           pixel in the window.
**
** Outputs: the routine outputs PIXEL_INDEX and RANK_INDEX updated to include
**          the new pixel.
**
** Method: the fresh pixel is appended into the TOP position in the pixel 
**         index. If its value is less than the value of the pixel which was
**         previously the TOP pixel, the fresh pixel is scanned down the pixel
**         index until it is inserted into the right place.
*/

int expand_window_I (size,
		 fresh_value,
		 rank_index,
		 pixel_index)

int    		size;
int  		fresh_value;
int    		*rank_index;
Pixel_info_i	*pixel_index;
{
    int    fresh_pixel;  /* pixel in the pixel index to be inserted */
    int    top_pixel;    /* the pixel which has the highest rank */

    fresh_pixel = size - 1;
    top_pixel = rank_index[size - 2];
    pixel_index[fresh_pixel].value = fresh_value;
	
    /* compare the value of fresh pixel with the value of the current */
    /* top pixel */

    if (fresh_value < pixel_index[top_pixel].value)
    {
	/* top pixel remains top pixel */
	/* increment rank of top pixel due to increase in size */
	/* scan down until the fresh pixel is inserted */
	
	rank_index[fresh_pixel] = top_pixel;
	pixel_index[top_pixel].rank = fresh_pixel;
	/* Note: fresh_pixel is also the current highest rank */
	
	scan_down_I (size,
		     fresh_pixel,
		     pixel_index[top_pixel].below,
		     top_pixel,
		     rank_index,
		     pixel_index,
		     fresh_pixel - 1);
    }
    else
    {
	/* fresh pixel becomes top pixel */

	rank_index[fresh_pixel] = fresh_pixel;
	pixel_index[fresh_pixel].rank = fresh_pixel;
	/* Note: fresh_pixel is also the current highest rank */
	
	pixel_index[top_pixel].above = fresh_pixel;
	pixel_index[fresh_pixel].below = top_pixel;
	pixel_index[fresh_pixel].above = TOP;
    }
    return (HIPS_OK);
}

/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                              SHRINK_WINDOW                                **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: shrink_window_I
**
** Purpose: to shrink the size of the filtering window by removing the stale
**          pixel.
**
** Inputs: the routine accepts the following inputs:
**	       SIZE        - the pixel length of the window before shrinking,
**             FRESH_PIXEL - pointer to the fresh/stale pixel in pixel index,
**             RANK_INDEX  - array containing the pixel position associated
**                           with each rank,
**             PIXEL_INDEX - array containing rank information for each 
**                           pixel in the window.
**
** Outputs: the routine outputs RANK_INDEX and PIXEL_INDEX, updated to exclude
**          the stale pixel.
**
** Method: the stale pixel is given a value one greater than the current
**         highest value. The stale pixel is then scanned up the pixel index
**         until it is inserted into the TOP position. The pixel below 
**         the stale pixel is then made the TOP pixel, removing the stale 
**         pixel from the pixel index. The window size must be decremented 
**         outside the routine after it is called.
*/

int shrink_window_I (size,
		 fresh_pixel,
		 rank_index,
		 pixel_index)

int    		size;
int    		fresh_pixel;
int    		*rank_index;
Pixel_info_i	*pixel_index;
{
    int   top_pixel;    /* the pixel which currently has the highest rank */
    int   above_pixel;  /* the pixel above the stale pixel */
    int   below_pixel;  /* the pixel below the stale pixel */
    int   current_rank; /* the current_rank of the fresh pixel */

    /* fresh_pixel holds the stale pixel information */

    current_rank = pixel_index[fresh_pixel].rank;
    below_pixel  = pixel_index[fresh_pixel].below;
    above_pixel  = pixel_index[fresh_pixel].above;

    if (above_pixel != TOP)
    {
	top_pixel = rank_index[size - 1];
        pixel_index[fresh_pixel].value = 
	  pixel_index[top_pixel].value + 1;

	/* move above pixel down a rank due to the removal of stale pixel */

	pixel_index[above_pixel].rank--;
	rank_index[current_rank] = above_pixel;
	pixel_index[below_pixel].above = above_pixel;
	pixel_index[above_pixel].below = below_pixel;

	/* scan up until the stale pixel reaches the TOP */

	scan_up_I (size,
		   fresh_pixel,
		   above_pixel,
		   pixel_index[above_pixel].above,
		   rank_index,
		   pixel_index,
		   ++current_rank);
    }
    /* pixel below fresh pixel is now made the top pixel */

    pixel_index[pixel_index[fresh_pixel].below].above = TOP;

    return (HIPS_OK);
}
	
/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                              INSERT_FRESH                                 **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: insert_fresh_I
**
** Purpose: overwrites the "stale" pixel information in the pixel index with
**          the information for the "fresh" pixel, updating the whole of the
**          pixel index and rank index.
**
** Inputs: the routine accepts the following inputs:
**             SIZE          - the size of the filtering window,
**             FRESH_PIXEL   - pointer to the "fresh" pixel in the pixel index,
**	       FRESH_VALUE   - the pixel value of the "fresh" pixel,
**	       RANK_INDEX    - array containing the pixel position associated
**                             with each rank,
**	       PIXEL_INDEX   - array containing rank information for each 
**                             pixel in the window.
**
** Outputs: the routine updates RANK_INDEX and PIXEL_INDEX to include the
**          fresh pixel and exclude the stale pixel.
**
** Method: the fresh pixel is inserted into the stale pixel's position in the
**         pixel index. If the value of the fresh pixel is greater than the
**         pixel above it, the fresh pixel is scanned up the pixel index until
**         it is inserted in the correct place. If the value of the fresh pixel
**         is less than the value of the pixel below it, the fresh pixel is
**         scanned down the pixel index until it is inserted in the correct
**         place.
*/

int insert_fresh_I (size,
		fresh_pixel,
		fresh_value,
		rank_index,
		pixel_index)

int    		size;
int    		fresh_pixel;
int  		fresh_value;
int    		*rank_index;
Pixel_info_i	*pixel_index;
{
    int   above_pixel;  /* the pixel above the stale pixel */
    int   below_pixel;  /* the pixel below the stale pixel */
    int   current_rank; /* the current_rank of the fresh pixel */

    /* fresh_pixel currently holds the stale pixel information */

    current_rank = pixel_index[fresh_pixel].rank;
    below_pixel = pixel_index[fresh_pixel].below;
    above_pixel = pixel_index[fresh_pixel].above;
    pixel_index[fresh_pixel].value = fresh_value;

    if (below_pixel == BOTTOM)
    { 
	if (fresh_value > pixel_index[above_pixel].value) 
	{
	    /* above pixel becomes new bottom pixel and is moved down a rank */

	    pixel_index[above_pixel].rank--;
	    rank_index[current_rank] = above_pixel;
	    pixel_index[above_pixel].below = BOTTOM;

	    /* scan up until the correct position for fresh pixel is found */

	    scan_up_I (size,
		       fresh_pixel,
		       above_pixel,
		       pixel_index[above_pixel].above,
		       rank_index,
		       pixel_index,
		       ++current_rank);
	}
	/* else fresh pixel overwrites stale pixel directly */
    }
    else if (above_pixel == TOP) 
    {
	if (fresh_value < pixel_index[below_pixel].value)
	{
	    /* below pixel becomes new top pixel and is moved up a rank */

	    pixel_index[below_pixel].rank++;
	    rank_index[current_rank] = below_pixel;
	    pixel_index[below_pixel].above = TOP;

	    /* scan down until the correct position for fresh pixel is found */

	    scan_down_I (size,
			 fresh_pixel,
			 pixel_index[below_pixel].below,
			 below_pixel,
			 rank_index,
			 pixel_index,
			 --current_rank);
	}
	/* else fresh pixel overwrites stale pixel directly */
    }
    else if (fresh_value < pixel_index[below_pixel].value)
    {
	/* move below pixel up a rank due to the removal of stale pixel */

	pixel_index[below_pixel].rank++;
	rank_index[current_rank] = below_pixel;
	pixel_index[below_pixel].above = above_pixel;
	pixel_index[above_pixel].below = below_pixel;

	/* scan down until the correct position for fresh pixel is found */

	scan_down_I (size,
		     fresh_pixel,
		     pixel_index[below_pixel].below,
		     below_pixel,
		     rank_index,
		     pixel_index,
		     --current_rank);
    }
    else if (fresh_value > pixel_index[above_pixel].value)
    {
	/* move above pixel down a rank due to the removal of stale pixel */

	pixel_index[above_pixel].rank--;
	rank_index[current_rank] = above_pixel;
	pixel_index[below_pixel].above = above_pixel;
	pixel_index[above_pixel].below = below_pixel;

	/* scan up until the correct position for fresh pixel is found */

	scan_up_I (size,
		   fresh_pixel,
		   above_pixel,
		   pixel_index[above_pixel].above,
		   rank_index,
		   pixel_index,
		   ++current_rank);
    }
    /* else fresh pixel overwrites stale pixel directly */

    return (HIPS_OK);
}

/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                               SCAN_DOWN                                   **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: scan_down_I
**
** Purpose: to recursively scan down through the pixel index until the correct
**          position for the fresh pixel is found. The fresh pixel is then
**	    inserted. The pixel index is updated as it is scanned.
**
** Inputs: the routine accepts the following inputs:
**             SIZE          - the size of the filtering window,
**             FRESH_PIXEL   - pointer to the "fresh" pixel in the pixel index,
**             BELOW_PIXEL   - pointer to the pixel below the "fresh" pixel,
**             ABOVE_PIXEL   - pointer to the pixel above the "fresh" pixel,
**	       RANK_INDEX    - array containing the pixel position associated
**                             with each rank,
**	       PIXEL_INDEX   - array containing rank information for each 
**                             pixel in the window,
**             CURRENT_RANK  - the current rank of the "fresh" pixel.
**
** Outputs: the routine updates RANK_INDEX and PIXEL_INDEX to include the
**          fresh pixel and exclude the stale pixel.
*/

int scan_down_I (size,
	     fresh_pixel,
	     below_pixel,
	     above_pixel,
	     rank_index,
	     pixel_index,
	     current_rank)

int    		size;
int    		fresh_pixel;
int    		below_pixel;
int    		above_pixel;
int    		*rank_index;
Pixel_info_i	*pixel_index;
int    current_rank;
{
    if (below_pixel == BOTTOM)
    {
	/* fresh pixel becomes the new bottom pixel */

	rank_index[0] = fresh_pixel;
	pixel_index[fresh_pixel].rank = 0;
	pixel_index[fresh_pixel].below = BOTTOM;
	pixel_index[fresh_pixel].above = above_pixel;
	pixel_index[above_pixel].below = fresh_pixel;
    }
    else if (pixel_index[fresh_pixel].value < pixel_index[below_pixel].value)
    {
	/* scan down again */

	rank_index[current_rank] = below_pixel;
	pixel_index[below_pixel].rank = current_rank;

	scan_down_I (size,
		     fresh_pixel,
		     pixel_index[below_pixel].below,
		     below_pixel,
		     rank_index,
		     pixel_index,
		     --current_rank);
    }
    else
    {
	/* fresh pixel is inserted in the current position */

	rank_index[current_rank] = fresh_pixel;
	pixel_index[fresh_pixel].rank = current_rank;
	pixel_index[fresh_pixel].below = below_pixel;
	pixel_index[below_pixel].above = fresh_pixel;
	pixel_index[fresh_pixel].above = above_pixel;
	pixel_index[above_pixel].below = fresh_pixel;
    }
    return (HIPS_OK);
}

/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                                SCAN_UP                                    **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: scan_up_I
**
** Purpose: to recursively scan up through the pixel index until the correct
**          position for the fresh pixel is found. The fresh pixel is then
**	    inserted. The pixel index is updated as it is scanned.
**
** Inputs: the routine accepts the following inputs:
**             SIZE          - the size of the filtering window,
**             FRESH_PIXEL   - pointer to the "fresh" pixel in the pixel index,
**             BELOW_PIXEL   - pointer to the pixel below the "fresh" pixel,
**             ABOVE_PIXEL   - pointer to the pixel above the "fresh" pixel,
**	       RANK_INDEX    - array containing the pixel position associated
**                             with each rank,
**	       PIXEL_INDEX   - array containing rank information for each 
**                             pixel in the window,
**             CURRENT_RANK  - the current rank of the "fresh" pixel.
**
** Outputs: the routine updates RANK_INDEX and PIXEL_INDEX to include the
**          fresh pixel and exclude the stale pixel.
*/

int scan_up_I (size,
	   fresh_pixel,
	   below_pixel,
	   above_pixel,
	   rank_index,
	   pixel_index,
	   current_rank)

int    		size;
int    		fresh_pixel;
int    		below_pixel;
int    		above_pixel;
int    		*rank_index;
Pixel_info_i 	*pixel_index;
int    current_rank;
{
    if (above_pixel == TOP)
    {
	/* fresh pixel becomes the new top pixel */

	rank_index[current_rank] = fresh_pixel;
	pixel_index[fresh_pixel].rank = current_rank;
	pixel_index[fresh_pixel].below = below_pixel;
	pixel_index[fresh_pixel].above = TOP;
	pixel_index[below_pixel].above = fresh_pixel;
    }
    else if (pixel_index[fresh_pixel].value > pixel_index[above_pixel].value)
    {
	/* scan up again */

	rank_index[current_rank] = above_pixel;
	pixel_index[above_pixel].rank = current_rank;

	scan_up_I (size,
		   fresh_pixel,
		   above_pixel,
		   pixel_index[above_pixel].above,
		   rank_index,
		   pixel_index,
		   ++current_rank);
    }
    else
    {
	/* fresh pixel is inserted in the current position */

	rank_index[current_rank] = fresh_pixel;
	pixel_index[fresh_pixel].rank = current_rank;
	pixel_index[fresh_pixel].below = below_pixel;
	pixel_index[below_pixel].above = fresh_pixel;
	pixel_index[fresh_pixel].above = above_pixel;
	pixel_index[above_pixel].below = fresh_pixel;
    }
    return (HIPS_OK);
}

/******************************** h_rank_f ************************************
*									      *
* Description:	1D rank order filters a PFLOAT format HIPS2 header image in   *
*		horizontal dimension.					      *
*									      *
* Parameters:	input_hdr	*header,				      *
*			pointer to the input image header.		      *
*		output_hdr	*header,				      *
*			pointer to the output image header.		      *
*		hor_size	int,	       				      *
*			the size of the horizontal filtering kernal.	      *
*		ver_size	int,	       				      *
*			the size of the vertical filtering kernal.	      *
*		hor_rank	int,	       			   	      *
*			the rank for filtering with the horizontal kernal.    *
*		ver_rank	int,	       			   	      *
*			the rank for filtering with the vertical kernal.      *
*									      *
* Return value: int,							      *
*			HIPS_ERROR - an error has occured.		      *
*			HIPS_OK    - successful execution.		      *
*									      *
* Functions Called:	h_rank_F,      					      *
*									      *
* External Functions:	none.						      *
*									      *
* External Parameters:	none.						      *
*									      *
******************************************************************************/

int h_rank_f (input_hdr,
	  output_hdr,	
	  hor_size,
	  ver_size,
	  hor_rank,
	  ver_rank)
     
struct 	header 	*input_hdr;
struct	header	*output_hdr;
int		hor_size;
int		ver_size;
int		hor_rank;
int		ver_rank;
{
    return (h_rank_F ((float *) input_hdr->firstpix,
		      (float *) output_hdr->firstpix,
		      input_hdr->rows,
		      input_hdr->cols,
		      input_hdr->ocols,
		      output_hdr->ocols,
		      hor_size,
		      ver_size,
		      hor_rank,
		      ver_rank));
}

/******************************** h_rank_F ************************************
*									      *
* Description:	rank order filters a float format image. 		      *
*									      *
* Parameters:	input_roi	*float,					      *
*			pointer to the start of the input region of interest. *
*		output_roi,	*float,					      *
*			pointer to the start of the output roi.		      *
*	        roi_rows,	int,					      *
*			number of rows in the roi.			      *
*		roi_cols,	int,					      *
*			number of columns in the roi.			      *
*	  	input_cols,	int,					      *
*			number of columns in the entire input frame.	      *
*	  	output_cols,	int,					      *
*			number of columns in the entire output frame.	      *
*		hor_size	int,	       				      *
*			the size of the horizontal filtering kernal.	      *
*		ver_size	int,	       				      *
*			the size of the vertical filtering kernal.	      *
*		hor_rank	int,	       			   	      *
*			the rank for filtering with the horizontal kernal.    *
*		ver_rank	int,	       			   	      *
*			the rank for filtering with the vertical kernal.      *
*									      *
* Return value: int,							      *
*			HIPS_ERROR - an error has occured.		      *
*			HIPS_OK    - successful execution.		      *
*									      *
* Functions Called:	h_hor_rank_F,	      			       	      *
*			h_ver_rank_F.					      *
*									      *
* External Functions:	h_copy_F.		       			      *
*									      *
******************************************************************************/

int h_rank_F (input_roi,
	  output_roi,
	  roi_rows,
	  roi_cols,
	  input_cols,
	  output_cols,
	  hor_size,
	  ver_size,
	  hor_rank,
	  ver_rank)

float 	*input_roi;
float 	*output_roi;
int 	roi_rows;
int	roi_cols;
int	input_cols;
int	output_cols;
int	hor_size;
int	ver_size;
int	hor_rank;
int	ver_rank;
{
    if (hor_size > 1)
    {
	if ((h_hor_rank_F (input_roi,
			   output_roi,
			   roi_rows,
			   roi_cols,
			   input_cols,
			   output_cols,
			   hor_size,
			   hor_rank - 1)) == HIPS_ERROR)
	  return (HIPS_ERROR);
    }
    if (ver_size > 1)
    {
	if (hor_size > 1)
	{
	    /* Copy the output from horizontal filtering onto the input for *
	     * vertical filtering.					    */

	    if ((h_copy_F (output_roi,
			   input_roi,
			   roi_rows,
			   roi_cols,
			   input_cols,
			   output_cols)) == HIPS_ERROR)
	      return (HIPS_ERROR);
	}

	if ((h_ver_rank_F (input_roi,
			   output_roi,
			   roi_rows,
			   roi_cols,
			   input_cols,
			   output_cols,
			   ver_size,
			   ver_rank - 1)) == HIPS_ERROR)
	  return (HIPS_ERROR);
    }
    return (HIPS_OK);
}

/****************************** h_hor_rank_F **********************************
*									      *
* Description:	1D rank order float format image in the horizontal dimension. *
*									      *
* Parameters:	input_roi	*float,					      *
*			pointer to the start of the input region of interest. *
*		output_roi,	*float,					      *
*			pointer to the start of the output roi.		      *
*	        roi_rows,	int,					      *
*			number of rows in the roi.			      *
*		roi_cols,	int,					      *
*			number of columns in the roi.			      *
*	  	input_cols,	int,					      *
*			number of columns in the entire input frame.	      *
*	  	output_cols,	int,					      *
*			number of columns in the entire output frame.	      *
*		size		int,	       				      *
*			the size of the horizontal filtering kernal.	      *
*		rank		int,	       			   	      *
*			the rank for filtering with the horizontal kernal.    *
*									      *
* Return value: int,							      *
*			HIPS_ERROR - an error has occured.		      *
*			HIPS_OK    - successful execution.		      *
*									      *
* Functions Called:	expand_window_F,	      	       	       	      *
*			shrink_window_F,		       		      *
*			insert_fresh_F.					      *
*									      *
* External Functions:	perr.						      *
*									      *
* External Parameters:	none.						      *
*									      *
* Method: each row of the input frame is scanned with the filtering window.   *
*         PIXEL_INDEX and RANK_INDEX hold the required information for each   *
*         pixel in the window. As the window moves across the row, the stale  *
*         pixel (ie. the one just leaving the window) is removed from the     *
*         window and the fresh pixel (ie. the one just entering the window)   *
*         is inserted. The RANK_INDEX and PIXEL_INDEX are updated  	      *
*         appropriately and the value of the pixel with the required rank is  *
*         outputed. For regions near the beginning of the row, where the      *
*         entire window cannot fit in, the window size is grown from 1 pixel  *
*         upto the required size as the window moves along the row. The value *
*         of the pixel with the appropriate rank for the reduced window size  *
*         is outputed. In a similar manner the window size is shrunk down as  *
*         the window approaches the end of the row.			      *
*									      *
******************************************************************************/

int h_hor_rank_F (input_roi,
	      output_roi,
	      roi_rows,
	      roi_cols,
	      input_cols,
	      output_cols,
	      size,
	      rank)

float 	*input_roi;
float 	*output_roi;
int 	roi_rows;
int	roi_cols;
int	input_cols;
int	output_cols;
int	size;
int	rank;
{
    static	h_boolean		allocation = FALSE;
    static	int   		*rank_index;
    static	Pixel_info_f 	*pixel_index;

    int    col;             /* the roi column of the current pixel */
    int    row;             /* the roi row of the current pixel */
    int	   input_offset;    /* offset between end & start of input roi rows */
    int	   output_offset;   /* offset between end & start of output roi rows */
    int    current_size;    /* the current window size */
    int    current_rank;    /* the rank appropriate to the current size */
    int    half_size;       /* half the required window size */
    float  *output_ptr;	    /* pointer to the current output pixel */
    float  *fresh_ptr;      /* pointer to the "fresh" pixel in the i/p image */
    int    fresh_pixel;     /* pointer to the "fresh" pixel in pixel index */
    int    count;           /* counting variable for expanding/shrinking */

    if (allocation == FALSE)
    {
	/* allocate core for the rank index */
	
	if ((rank_index = (int *) calloc (size, sizeof(int))) == 0)
	  return (perr (HE_ALLOCSUBR, "h_hor_rank_F"));

	/* allocate core for the pixel index */
	
	if ((pixel_index = (Pixel_info_f *) 
	     calloc (size, sizeof(Pixel_info_f))) == 0)
	  return (perr (HE_ALLOCSUBR, "h_hor_rank_F"));

	allocation = TRUE;
    }

    half_size = (size + 1)/2;
    input_offset = input_cols - roi_cols;
    output_offset = output_cols - roi_cols;

    output_ptr = output_roi;
    fresh_ptr  = input_roi;

    for (row = 0; row < roi_rows; row++)
    {
	fresh_pixel = 0; /* points to the first pixel in the pixel index */

	for (col = 0; col < roi_cols; col++)
	{
	    if (col == 0) /* 1st pixel in the row */
	    {
	        current_size = 1;
		
		/* initialise the pixel index with this pixel */

		pixel_index[0].above = TOP;
		pixel_index[0].below = BOTTOM;
		pixel_index[0].value = *fresh_ptr;
		pixel_index[0].rank = 0;
		rank_index[0] = 0;

		/* output the input pixel unaltered */

		*output_ptr = *fresh_ptr++;
	    }
	    else if (col < half_size) /* pixel lies in near edge area */
	    {
		for (count = 0; count < 2; count++)
		{
		    /* expand window by appending new pixel */

		    current_size++;
		    expand_window_F (current_size,
				     *fresh_ptr,
				     rank_index,
				     pixel_index);
		    fresh_ptr++;
		}
		/* output pixel value dependant on current size */
		
		current_rank = (rank * current_size / size) + 0.5;

		*output_ptr = 
		  pixel_index[rank_index[current_rank]].value;
	    }
	    else if (col == roi_cols - 1) /* last pixel in the row */
	    {
		/* output the input pixel unaltered */

		*output_ptr = *(fresh_ptr - 1);
	    }
	    else if (col > roi_cols - half_size) /* pixel in far edge area */
	    {
		for (count = 0; count < 2; count++)
		{
		    /* shrink window by removing stale pixel */
		    /* Note: fresh pixel points to stale pixel also */

		    shrink_window_F (current_size,
				     fresh_pixel,
				     rank_index,
				     pixel_index);
		    current_size--;
		    fresh_pixel = (fresh_pixel + 1) % size;
		}
		/* output pixel value dependant on current size */
		
		current_rank = (rank * current_size / size) + 0.5;

		*output_ptr = 
		  pixel_index[rank_index[current_rank]].value;
	    }
	    else /* pixel lies in the body of the row */
	    {
		insert_fresh_F (size,
				fresh_pixel,
				*fresh_ptr,
				rank_index,
				pixel_index);		
		
		fresh_pixel = (fresh_pixel + 1) % size;
		fresh_ptr++;
		*output_ptr = pixel_index[rank_index[rank]].value;
	    }
	    output_ptr++;

	} /* for each col */

	/* Move to the start of the next row of the output and input roi */

	output_ptr += output_offset;
	fresh_ptr += input_offset;

    } /* for each row */

    return (HIPS_OK);
}

/****************************** h_ver_rank_F **********************************
*									      *
* Description:	1D rank order float format image in the vertical dimension.   *
*									      *
* Parameters:	input_roi	*float,					      *
*			pointer to the start of the input region of interest. *
*		output_roi,	*float,					      *
*			pointer to the start of the output roi.		      *
*	        roi_rows,	int,					      *
*			number of rows in the roi.			      *
*		roi_cols,	int,					      *
*			number of columns in the roi.			      *
*	  	input_cols,	int,					      *
*			number of columns in the entire input frame.	      *
*	  	output_cols,	int,					      *
*			number of columns in the entire output frame.	      *
*		size		int,	       				      *
*			the size of the vertical filtering kernal.	      *
*		rank		int,	       			   	      *
*			the rank for filtering with the vertical kernal.      *
*									      *
* Return value: int,							      *
*			HIPS_ERROR - an error has occured.		      *
*			HIPS_OK    - successful execution.		      *
*									      *
* Functions Called:	expand_window_F,       			       	      *
*			shrink_window_F,	       			      *
*			insert_fresh_F.					      *
*									      *
* External Functions:	perr.						      *
*									      *
* External Parameters:	none.						      *
*									      *
* Method: each column of the input frame is scanned with the filtering window.*
*         PIXEL_INDEX and RANK_INDEX hold the required information for each   *
*         pixel in the window. As the window moves down the column, the stale *
*         pixel (ie. the one just leaving the window) is removed from the     *
*         window and the fresh pixel (ie. the one just entering the window)   *
*         is inserted. The RANK_INDEX and PIXEL_INDEX are updated  	      *
*         appropriately and the value of the pixel with the required rank is  *
*         outputed. For regions near the beginning of the column, where the   *
*         entire window cannot fit in, the window size is grown from 1 pixel  *
*         upto the required size as the window moves down the col. The value  *
*         of the pixel with the appropriate rank for the reduced window size  *
*         is outputed. In a similar manner the window size is shrunk down as  *
*         the window approaches the end of the column.			      *
*									      *
******************************************************************************/

int h_ver_rank_F (input_roi,
	      output_roi,
	      roi_rows,
	      roi_cols,
	      input_cols,
	      output_cols,
	      size,
	      rank)

float 	*input_roi;
float 	*output_roi;
int 	roi_rows;
int	roi_cols;
int	input_cols;
int	output_cols;
int	size;
int	rank;
{
    static	h_boolean		allocation = FALSE;
    static	int   		*rank_index;
    static	Pixel_info_f 	*pixel_index;

    int    col;             /* the roi column of the current pixel */
    int    row;             /* the roi row of the current pixel */
    int    current_size;    /* the current window size */
    int    current_rank;    /* the rank appropriate to the current size */
    int    half_size;       /* half the required window size */
    float  *output_ptr;	    /* pointer to the current output pixel */
    float  *fresh_ptr;      /* pointer to the "fresh" pixel in the i/p image */
    int    fresh_pixel;     /* pointer to the "fresh" pixel in pixel index */
    int    count;           /* counting variable for expanding/shrinking */

    if (allocation == FALSE)
    {
	/* allocate core for the rank index */
	
	if ((rank_index = (int *) calloc (size, sizeof(int))) == 0)
	  return (perr (HE_ALLOCSUBR, "h_ver_rank_F"));

	/* allocate core for the pixel index */
	
	if ((pixel_index = (Pixel_info_f *) 
	     calloc (size, sizeof(Pixel_info_f))) == 0)
	  return (perr (HE_ALLOCSUBR, "h_ver_rank_F"));

	allocation = TRUE;
    }

    half_size = (size + 1)/2;

    for (col = 0; col < roi_cols; col++)
    {
	/* Set positions to start of the current column */

	fresh_ptr = input_roi + col;
	output_ptr = output_roi + col;

	fresh_pixel = 0; /* points to the first pixel in the pixel index */

	for (row = 0; row < roi_rows; row++)
	{
	    if (row == 0) /* 1st pixel in the col */
	    {
	        current_size = 1;
		
		/* initialise the pixel index with this pixel */

		pixel_index[0].above = TOP;
		pixel_index[0].below = BOTTOM;
		pixel_index[0].value = *fresh_ptr;
		pixel_index[0].rank = 0;
		rank_index[0] = 0;

		/* output the input pixel unaltered */

		*output_ptr = *fresh_ptr;
	    }
	    else if (row < half_size) /* pixel lies in top edge area */
	    {
		/* move output_ptr down a row */
	    
		output_ptr += output_cols;

		/* Grow the window by two pixels */

		for (count = 0; count < 2; count++)
		{
		    /* expand window by appending new pixel */

		    /* move fresh_ptr to point to next pixel in the column */

		    fresh_ptr += input_cols;

		    current_size++;
		    expand_window_F (current_size,
				     *fresh_ptr,
				     rank_index,
				     pixel_index);
		}
		/* output pixel value dependant on current size */
		
		current_rank = (rank * current_size / size) + 0.5;

		*output_ptr = 
		  pixel_index[rank_index[current_rank]].value;
	    }
	    else if (row == roi_rows - 1) /* last pixel in the col */
	    {
		/* move output_ptr down a row */
	    
		output_ptr += output_cols;

		/* output the input pixel unaltered */

		*output_ptr = *fresh_ptr;
	    }
	    else if (row > roi_rows - half_size) /* pixel in bottom edge *
						  * area 		 */
	    {
		/* move output_ptr down a row */
	    
		output_ptr += output_cols;

		/* shrink window by 2 pixels */

		for (count = 0; count < 2; count++)
		{
		    /* shrink window by removing stale pixel */
		    /* Note: fresh pixel points to stale pixel also */

		    shrink_window_F (current_size,
				     fresh_pixel,
				     rank_index,
				     pixel_index);
		    current_size--;
		    fresh_pixel = (fresh_pixel + 1) % size;
		}
		/* output pixel value dependant on current size */
		
		current_rank = (rank * current_size / size) + 0.5;

		*output_ptr = 
		  pixel_index[rank_index[current_rank]].value;
	    }
	    else /* pixel lies in the body of the col */
	    {
		/* move fresh_ptr to point to next pixel in the column */

		fresh_ptr += input_cols;

		/* move output_ptr down a row */
	    
		output_ptr += output_cols;

		insert_fresh_F (size,
				fresh_pixel,
				*fresh_ptr,
				rank_index,
				pixel_index);		
		
		fresh_pixel = (fresh_pixel + 1) % size;

		*output_ptr = pixel_index[rank_index[rank]].value;
	    }

	} /* for each col */

    } /* for each row */

    return (HIPS_OK);
}
	   
/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                              EXPAND_WINDOW                                **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: expand_window_F
**
** Purpose: to expand the size of the filtering window by appending a new pixel
**          to the pixel index and rank index.
**
** Inputs: the routine accepts the following inputs:
**	       SIZE        - the pixel length of the expanded window,
**             FRESH_VALUE - the value of the pixel to be appended,
**             RANK_INDEX  - array containing the pixel position associated
**                           with each rank,
**             PIXEL_INDEX - array containing rank information for each 
**                           pixel in the window.
**
** Outputs: the routine outputs PIXEL_INDEX and RANK_INDEX updated to include
**          the new pixel.
**
** Method: the fresh pixel is appended into the TOP position in the pixel 
**         index. If its value is less than the value of the pixel which was
**         previously the TOP pixel, the fresh pixel is scanned down the pixel
**         index until it is inserted into the right place.
*/

int expand_window_F (size,
		 fresh_value,
		 rank_index,
		 pixel_index)

int    		size;
float  		fresh_value;
int    		*rank_index;
Pixel_info_f	*pixel_index;
{
    int    fresh_pixel;  /* pixel in the pixel index to be inserted */
    int    top_pixel;    /* the pixel which has the highest rank */

    fresh_pixel = size - 1;
    top_pixel = rank_index[size - 2];
    pixel_index[fresh_pixel].value = fresh_value;
	
    /* compare the value of fresh pixel with the value of the current */
    /* top pixel */

    if (fresh_value < pixel_index[top_pixel].value)
    {
	/* top pixel remains top pixel */
	/* increment rank of top pixel due to increase in size */
	/* scan down until the fresh pixel is inserted */
	
	rank_index[fresh_pixel] = top_pixel;
	pixel_index[top_pixel].rank = fresh_pixel;
	/* Note: fresh_pixel is also the current highest rank */
	
	scan_down_F (size,
		     fresh_pixel,
		     pixel_index[top_pixel].below,
		     top_pixel,
		     rank_index,
		     pixel_index,
		     fresh_pixel - 1);
    }
    else
    {
	/* fresh pixel becomes top pixel */

	rank_index[fresh_pixel] = fresh_pixel;
	pixel_index[fresh_pixel].rank = fresh_pixel;
	/* Note: fresh_pixel is also the current highest rank */
	
	pixel_index[top_pixel].above = fresh_pixel;
	pixel_index[fresh_pixel].below = top_pixel;
	pixel_index[fresh_pixel].above = TOP;
    }
    return (HIPS_OK);
}

/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                              SHRINK_WINDOW                                **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: shrink_window_F
**
** Purpose: to shrink the size of the filtering window by removing the stale
**          pixel.
**
** Inputs: the routine accepts the following inputs:
**	       SIZE        - the pixel length of the window before shrinking,
**             FRESH_PIXEL - pointer to the fresh/stale pixel in pixel index,
**             RANK_INDEX  - array containing the pixel position associated
**                           with each rank,
**             PIXEL_INDEX - array containing rank information for each 
**                           pixel in the window.
**
** Outputs: the routine outputs RANK_INDEX and PIXEL_INDEX, updated to exclude
**          the stale pixel.
**
** Method: the stale pixel is given a value one greater than the current
**         highest value. The stale pixel is then scanned up the pixel index
**         until it is inserted into the TOP position. The pixel below 
**         the stale pixel is then made the TOP pixel, removing the stale 
**         pixel from the pixel index. The window size must be decremented 
**         outside the routine after it is called.
*/

int shrink_window_F (size,
		 fresh_pixel,
		 rank_index,
		 pixel_index)

int    		size;
int    		fresh_pixel;
int    		*rank_index;
Pixel_info_f	*pixel_index;
{
    int   top_pixel;    /* the pixel which currently has the highest rank */
    int   above_pixel;  /* the pixel above the stale pixel */
    int   below_pixel;  /* the pixel below the stale pixel */
    int   current_rank; /* the current_rank of the fresh pixel */

    /* fresh_pixel holds the stale pixel information */

    current_rank = pixel_index[fresh_pixel].rank;
    below_pixel  = pixel_index[fresh_pixel].below;
    above_pixel  = pixel_index[fresh_pixel].above;

    if (above_pixel != TOP)
    {
	top_pixel = rank_index[size - 1];
        pixel_index[fresh_pixel].value = 
	  pixel_index[top_pixel].value + 1;

	/* move above pixel down a rank due to the removal of stale pixel */

	pixel_index[above_pixel].rank--;
	rank_index[current_rank] = above_pixel;
	pixel_index[below_pixel].above = above_pixel;
	pixel_index[above_pixel].below = below_pixel;

	/* scan up until the stale pixel reaches the TOP */

	scan_up_F (size,
		   fresh_pixel,
		   above_pixel,
		   pixel_index[above_pixel].above,
		   rank_index,
		   pixel_index,
		   ++current_rank);
    }
    /* pixel below fresh pixel is now made the top pixel */

    pixel_index[pixel_index[fresh_pixel].below].above = TOP;

    return (HIPS_OK);
}
	
/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                              INSERT_FRESH                                 **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: insert_fresh_F
**
** Purpose: overwrites the "stale" pixel information in the pixel index with
**          the information for the "fresh" pixel, updating the whole of the
**          pixel index and rank index.
**
** Inputs: the routine accepts the following inputs:
**             SIZE          - the size of the filtering window,
**             FRESH_PIXEL   - pointer to the "fresh" pixel in the pixel index,
**	       FRESH_VALUE   - the pixel value of the "fresh" pixel,
**	       RANK_INDEX    - array containing the pixel position associated
**                             with each rank,
**	       PIXEL_INDEX   - array containing rank information for each 
**                             pixel in the window.
**
** Outputs: the routine updates RANK_INDEX and PIXEL_INDEX to include the
**          fresh pixel and exclude the stale pixel.
**
** Method: the fresh pixel is inserted into the stale pixel's position in the
**         pixel index. If the value of the fresh pixel is greater than the
**         pixel above it, the fresh pixel is scanned up the pixel index until
**         it is inserted in the correct place. If the value of the fresh pixel
**         is less than the value of the pixel below it, the fresh pixel is
**         scanned down the pixel index until it is inserted in the correct
**         place.
*/

int insert_fresh_F (size,
		fresh_pixel,
		fresh_value,
		rank_index,
		pixel_index)

int    		size;
int    		fresh_pixel;
float  		fresh_value;
int    		*rank_index;
Pixel_info_f	*pixel_index;
{
    int   above_pixel;  /* the pixel above the stale pixel */
    int   below_pixel;  /* the pixel below the stale pixel */
    int   current_rank; /* the current_rank of the fresh pixel */

    /* fresh_pixel currently holds the stale pixel information */

    current_rank = pixel_index[fresh_pixel].rank;
    below_pixel = pixel_index[fresh_pixel].below;
    above_pixel = pixel_index[fresh_pixel].above;
    pixel_index[fresh_pixel].value = fresh_value;

    if (below_pixel == BOTTOM)
    { 
	if (fresh_value > pixel_index[above_pixel].value) 
	{
	    /* above pixel becomes new bottom pixel and is moved down a rank */

	    pixel_index[above_pixel].rank--;
	    rank_index[current_rank] = above_pixel;
	    pixel_index[above_pixel].below = BOTTOM;

	    /* scan up until the correct position for fresh pixel is found */

	    scan_up_F (size,
		       fresh_pixel,
		       above_pixel,
		       pixel_index[above_pixel].above,
		       rank_index,
		       pixel_index,
		       ++current_rank);
	}
	/* else fresh pixel overwrites stale pixel directly */
    }
    else if (above_pixel == TOP) 
    {
	if (fresh_value < pixel_index[below_pixel].value)
	{
	    /* below pixel becomes new top pixel and is moved up a rank */

	    pixel_index[below_pixel].rank++;
	    rank_index[current_rank] = below_pixel;
	    pixel_index[below_pixel].above = TOP;

	    /* scan down until the correct position for fresh pixel is found */

	    scan_down_F (size,
			 fresh_pixel,
			 pixel_index[below_pixel].below,
			 below_pixel,
			 rank_index,
			 pixel_index,
			 --current_rank);
	}
	/* else fresh pixel overwrites stale pixel directly */
    }
    else if (fresh_value < pixel_index[below_pixel].value)
    {
	/* move below pixel up a rank due to the removal of stale pixel */

	pixel_index[below_pixel].rank++;
	rank_index[current_rank] = below_pixel;
	pixel_index[below_pixel].above = above_pixel;
	pixel_index[above_pixel].below = below_pixel;

	/* scan down until the correct position for fresh pixel is found */

	scan_down_F (size,
		     fresh_pixel,
		     pixel_index[below_pixel].below,
		     below_pixel,
		     rank_index,
		     pixel_index,
		     --current_rank);
    }
    else if (fresh_value > pixel_index[above_pixel].value)
    {
	/* move above pixel down a rank due to the removal of stale pixel */

	pixel_index[above_pixel].rank--;
	rank_index[current_rank] = above_pixel;
	pixel_index[below_pixel].above = above_pixel;
	pixel_index[above_pixel].below = below_pixel;

	/* scan up until the correct position for fresh pixel is found */

	scan_up_F (size,
		   fresh_pixel,
		   above_pixel,
		   pixel_index[above_pixel].above,
		   rank_index,
		   pixel_index,
		   ++current_rank);
    }
    /* else fresh pixel overwrites stale pixel directly */

    return (HIPS_OK);
}

/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                               SCAN_DOWN                                   **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: scan_down_F
**
** Purpose: to recursively scan down through the pixel index until the correct
**          position for the fresh pixel is found. The fresh pixel is then
**	    inserted. The pixel index is updated as it is scanned.
**
** Inputs: the routine accepts the following inputs:
**             SIZE          - the size of the filtering window,
**             FRESH_PIXEL   - pointer to the "fresh" pixel in the pixel index,
**             BELOW_PIXEL   - pointer to the pixel below the "fresh" pixel,
**             ABOVE_PIXEL   - pointer to the pixel above the "fresh" pixel,
**	       RANK_INDEX    - array containing the pixel position associated
**                             with each rank,
**	       PIXEL_INDEX   - array containing rank information for each 
**                             pixel in the window,
**             CURRENT_RANK  - the current rank of the "fresh" pixel.
**
** Outputs: the routine updates RANK_INDEX and PIXEL_INDEX to include the
**          fresh pixel and exclude the stale pixel.
*/

int scan_down_F (size,
	     fresh_pixel,
	     below_pixel,
	     above_pixel,
	     rank_index,
	     pixel_index,
	     current_rank)

int    		size;
int    		fresh_pixel;
int    		below_pixel;
int    		above_pixel;
int    		*rank_index;
Pixel_info_f	*pixel_index;
int    current_rank;
{
    if (below_pixel == BOTTOM)
    {
	/* fresh pixel becomes the new bottom pixel */

	rank_index[0] = fresh_pixel;
	pixel_index[fresh_pixel].rank = 0;
	pixel_index[fresh_pixel].below = BOTTOM;
	pixel_index[fresh_pixel].above = above_pixel;
	pixel_index[above_pixel].below = fresh_pixel;
    }
    else if (pixel_index[fresh_pixel].value < pixel_index[below_pixel].value)
    {
	/* scan down again */

	rank_index[current_rank] = below_pixel;
	pixel_index[below_pixel].rank = current_rank;

	scan_down_F (size,
		     fresh_pixel,
		     pixel_index[below_pixel].below,
		     below_pixel,
		     rank_index,
		     pixel_index,
		     --current_rank);
    }
    else
    {
	/* fresh pixel is inserted in the current position */

	rank_index[current_rank] = fresh_pixel;
	pixel_index[fresh_pixel].rank = current_rank;
	pixel_index[fresh_pixel].below = below_pixel;
	pixel_index[below_pixel].above = fresh_pixel;
	pixel_index[fresh_pixel].above = above_pixel;
	pixel_index[above_pixel].below = fresh_pixel;
    }
    return (HIPS_OK);
}

/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                                SCAN_UP                                    **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: scan_up_F
**
** Purpose: to recursively scan up through the pixel index until the correct
**          position for the fresh pixel is found. The fresh pixel is then
**	    inserted. The pixel index is updated as it is scanned.
**
** Inputs: the routine accepts the following inputs:
**             SIZE          - the size of the filtering window,
**             FRESH_PIXEL   - pointer to the "fresh" pixel in the pixel index,
**             BELOW_PIXEL   - pointer to the pixel below the "fresh" pixel,
**             ABOVE_PIXEL   - pointer to the pixel above the "fresh" pixel,
**	       RANK_INDEX    - array containing the pixel position associated
**                             with each rank,
**	       PIXEL_INDEX   - array containing rank information for each 
**                             pixel in the window,
**             CURRENT_RANK  - the current rank of the "fresh" pixel.
**
** Outputs: the routine updates RANK_INDEX and PIXEL_INDEX to include the
**          fresh pixel and exclude the stale pixel.
*/

int scan_up_F (size,
	   fresh_pixel,
	   below_pixel,
	   above_pixel,
	   rank_index,
	   pixel_index,
	   current_rank)

int    		size;
int    		fresh_pixel;
int    		below_pixel;
int    		above_pixel;
int    		*rank_index;
Pixel_info_f 	*pixel_index;
int    current_rank;
{
    if (above_pixel == TOP)
    {
	/* fresh pixel becomes the new top pixel */

	rank_index[current_rank] = fresh_pixel;
	pixel_index[fresh_pixel].rank = current_rank;
	pixel_index[fresh_pixel].below = below_pixel;
	pixel_index[fresh_pixel].above = TOP;
	pixel_index[below_pixel].above = fresh_pixel;
    }
    else if (pixel_index[fresh_pixel].value > pixel_index[above_pixel].value)
    {
	/* scan up again */

	rank_index[current_rank] = above_pixel;
	pixel_index[above_pixel].rank = current_rank;

	scan_up_F (size,
		   fresh_pixel,
		   above_pixel,
		   pixel_index[above_pixel].above,
		   rank_index,
		   pixel_index,
		   ++current_rank);
    }
    else
    {
	/* fresh pixel is inserted in the current position */

	rank_index[current_rank] = fresh_pixel;
	pixel_index[fresh_pixel].rank = current_rank;
	pixel_index[fresh_pixel].below = below_pixel;
	pixel_index[below_pixel].above = fresh_pixel;
	pixel_index[fresh_pixel].above = above_pixel;
	pixel_index[above_pixel].below = fresh_pixel;
    }
    return (HIPS_OK);
}

/******************************** h_rank_d ************************************
*									      *
* Description:	rank order filters a PFDOUBLE format HIPS2 header image.      *
*									      *
* Parameters:	input_hdr	*header,				      *
*			pointer to the input image header.		      *
*		output_hdr	*header,				      *
*			pointer to the output image header.		      *
*		hor_size	int,	       				      *
*			the size of the horizontal filtering kernal.	      *
*		ver_size	int,	       				      *
*			the size of the vertical filtering kernal.	      *
*		hor_rank	int,	       			   	      *
*			the rank for filtering with the horizontal kernal.    *
*		ver_rank	int,	       			   	      *
*			the rank for filtering with the vertical kernal.      *
*									      *
* Return value: int,							      *
*			HIPS_ERROR - an error has occured.		      *
*			HIPS_OK    - successful execution.		      *
*									      *
* Functions Called:	h_rank_D,      					      *
*									      *
* External Functions:	none.						      *
*									      *
* External Parameters:	none.						      *
*									      *
******************************************************************************/

int h_rank_d (input_hdr,
	  output_hdr,	
	  hor_size,
	  ver_size,
	  hor_rank,
	  ver_rank)
     
struct 	header 	*input_hdr;
struct	header	*output_hdr;
int		hor_size;
int		ver_size;
int		hor_rank;
int		ver_rank;
{
    return (h_rank_D ((double *) input_hdr->firstpix,
		      (double *) output_hdr->firstpix,
		      input_hdr->rows,
		      input_hdr->cols,
		      input_hdr->ocols,
		      output_hdr->ocols,
		      hor_size,
		      ver_size,
		      hor_rank,
		      ver_rank));
}

/******************************** h_rank_D ************************************
*									      *
* Description:	1D rank order filters a double format image.		      *
*									      *
* Parameters:	input_roi	*double,			              *
*			pointer to the start of the input region of interest. *
*		output_roi,	*double,		       		      *
*			pointer to the start of the output roi.		      *
*	        roi_rows,	int,					      *
*			number of rows in the roi.			      *
*		roi_cols,	int,					      *
*			number of columns in the roi.			      *
*	  	input_cols,	int,					      *
*			number of columns in the entire input frame.	      *
*	  	output_cols,	int,					      *
*			number of columns in the entire output frame.	      *
*		hor_size	int,	       				      *
*			the size of the horizontal filtering kernal.	      *
*		ver_size	int,	       				      *
*			the size of the vertical filtering kernal.	      *
*		hor_rank	int,	       			   	      *
*			the rank for filtering with the horizontal kernal.    *
*		ver_rank	int,	       			   	      *
*			the rank for filtering with the vertical kernal.      *
*									      *
* Return value: int,							      *
*			HIPS_ERROR - an error has occured.		      *
*			HIPS_OK    - successful execution.		      *
*									      *
* Functions Called:	h_hor_rank_D,	      			       	      *
*			h_ver_rank_D.					      *
*									      *
* External Functions:	h_copy_D.		       			      *
*									      *
******************************************************************************/

int h_rank_D (input_roi,
	  output_roi,
	  roi_rows,
	  roi_cols,
	  input_cols,
	  output_cols,
	  hor_size,
	  ver_size,
	  hor_rank,
	  ver_rank)

double 	*input_roi;
double 	*output_roi;
int 	roi_rows;
int	roi_cols;
int	input_cols;
int	output_cols;
int	hor_size;
int	ver_size;
int	hor_rank;
int	ver_rank;
{
    if (hor_size > 1)
    {
	if ((h_hor_rank_D (input_roi,
			   output_roi,
			   roi_rows,
			   roi_cols,
			   input_cols,
			   output_cols,
			   hor_size,
			   hor_rank - 1)) == HIPS_ERROR)
	  return (HIPS_ERROR);
    }
    if (ver_size > 1)
    {
	if (hor_size > 1)
	{
	    /* Copy the output from horizontal filtering onto the input for *
	     * vertical filtering.					    */

	    if ((h_copy_D (output_roi,
			   input_roi,
			   roi_rows,
			   roi_cols,
			   input_cols,
			   output_cols)) == HIPS_ERROR)
	      return (HIPS_ERROR);
	}

	if ((h_ver_rank_D (input_roi,
			   output_roi,
			   roi_rows,
			   roi_cols,
			   input_cols,
			   output_cols,
			   ver_size,
			   ver_rank - 1)) == HIPS_ERROR)
	  return (HIPS_ERROR);
    }
    return (HIPS_OK);
}

/****************************** h_hor_rank_D **********************************
*									      *
* Description:	1D rank order filters a double format image in the horizontal *
*	        dimension.						      *
*						       			      *
* Parameters:	input_roi	*double,		       		      *
*			pointer to the start of the input region of interest. *
*		output_roi,	*double,		       		      *
*			pointer to the start of the output roi.		      *
*	        roi_rows,	int,					      *
*			number of rows in the roi.			      *
*		roi_cols,	int,					      *
*			number of columns in the roi.			      *
*	  	input_cols,	int,					      *
*			number of columns in the entire input frame.	      *
*	  	output_cols,	int,					      *
*			number of columns in the entire output frame.	      *
*		size		int,	       				      *
*			the size of the horizontal filtering kernal.	      *
*		rank		int,	       			   	      *
*			the rank for filtering with the horizontal kernal.    *
*									      *
* Return value: int,							      *
*			HIPS_ERROR - an error has occured.		      *
*			HIPS_OK    - successful execution.		      *
*									      *
* Functions Called:	expand_window_D,	      	       	       	      *
*			shrink_window_D,		       		      *
*			insert_fresh_D.					      *
*									      *
* External Functions:	perr.						      *
*									      *
* External Parameters:	none.						      *
*									      *
* Method: each row of the input frame is scanned with the filtering window.   *
*         PIXEL_INDEX and RANK_INDEX hold the required information for each   *
*         pixel in the window. As the window moves across the row, the stale  *
*         pixel (ie. the one just leaving the window) is removed from the     *
*         window and the fresh pixel (ie. the one just entering the window)   *
*         is inserted. The RANK_INDEX and PIXEL_INDEX are updated  	      *
*         appropriately and the value of the pixel with the required rank is  *
*         outputed. For regions near the beginning of the row, where the      *
*         entire window cannot fit in, the window size is grown from 1 pixel  *
*         upto the required size as the window moves along the row. The value *
*         of the pixel with the appropriate rank for the reduced window size  *
*         is outputed. In a similar manner the window size is shrunk down as  *
*         the window approaches the end of the row.			      *
*									      *
******************************************************************************/

int h_hor_rank_D (input_roi,
	      output_roi,
	      roi_rows,
	      roi_cols,
	      input_cols,
	      output_cols,
	      size,
	      rank)

double 	*input_roi;
double 	*output_roi;
int 	roi_rows;
int	roi_cols;
int	input_cols;
int	output_cols;
int	size;
int	rank;
{
    static	h_boolean		allocation = FALSE;
    static	int   		*rank_index;
    static	Pixel_info_d 	*pixel_index;

    int    col;             /* the roi column of the current pixel */
    int    row;             /* the roi row of the current pixel */
    int	   input_offset;    /* offset between end & start of input roi rows */
    int	   output_offset;   /* offset between end & start of output roi rows */
    int    current_size;    /* the current window size */
    int    current_rank;    /* the rank appropriate to the current size */
    int    half_size;       /* half the required window size */
    double  *output_ptr;    /* pointer to the current output pixel */
    double  *fresh_ptr;     /* pointer to the "fresh" pixel in the i/p image */
    int    fresh_pixel;     /* pointer to the "fresh" pixel in pixel index */
    int    count;           /* counting variable for expanding/shrinking */

    if (allocation == FALSE)
    {
	/* allocate core for the rank index */
	
	if ((rank_index = (int *) calloc (size, sizeof(int))) == 0)
	  return (perr (HE_ALLOCSUBR, "h_hor_rank_D"));

	/* allocate core for the pixel index */
	
	if ((pixel_index = (Pixel_info_d *) 
	     calloc (size, sizeof(Pixel_info_d))) == 0)
	  return (perr (HE_ALLOCSUBR, "h_hor_rank_D"));

	allocation = TRUE;
    }

    half_size = (size + 1)/2;
    input_offset = input_cols - roi_cols;
    output_offset = output_cols - roi_cols;

    output_ptr = output_roi;
    fresh_ptr  = input_roi;

    for (row = 0; row < roi_rows; row++)
    {
	fresh_pixel = 0; /* points to the first pixel in the pixel index */

	for (col = 0; col < roi_cols; col++)
	{
	    if (col == 0) /* 1st pixel in the row */
	    {
	        current_size = 1;
		
		/* initialise the pixel index with this pixel */

		pixel_index[0].above = TOP;
		pixel_index[0].below = BOTTOM;
		pixel_index[0].value = *fresh_ptr;
		pixel_index[0].rank = 0;
		rank_index[0] = 0;

		/* output the input pixel unaltered */

		*output_ptr = *fresh_ptr++;
	    }
	    else if (col < half_size) /* pixel lies in near edge area */
	    {
		for (count = 0; count < 2; count++)
		{
		    /* expand window by appending new pixel */

		    current_size++;
		    expand_window_D (current_size,
				     *fresh_ptr,
				     rank_index,
				     pixel_index);
		    fresh_ptr++;
		}
		/* output pixel value dependant on current size */
		
		current_rank = (rank * current_size / size) + 0.5;

		*output_ptr = 
		  pixel_index[rank_index[current_rank]].value;
	    }
	    else if (col == roi_cols - 1) /* last pixel in the row */
	    {
		/* output the input pixel unaltered */

		*output_ptr = *(fresh_ptr - 1);
	    }
	    else if (col > roi_cols - half_size) /* pixel in far edge area */
	    {
		for (count = 0; count < 2; count++)
		{
		    /* shrink window by removing stale pixel */
		    /* Note: fresh pixel points to stale pixel also */

		    shrink_window_D (current_size,
				     fresh_pixel,
				     rank_index,
				     pixel_index);
		    current_size--;
		    fresh_pixel = (fresh_pixel + 1) % size;
		}
		/* output pixel value dependant on current size */
		
		current_rank = (rank * current_size / size) + 0.5;

		*output_ptr = 
		  pixel_index[rank_index[current_rank]].value;
	    }
	    else /* pixel lies in the body of the row */
	    {
		insert_fresh_D (size,
				fresh_pixel,
				*fresh_ptr,
				rank_index,
				pixel_index);		
		
		fresh_pixel = (fresh_pixel + 1) % size;
		fresh_ptr++;
		*output_ptr = pixel_index[rank_index[rank]].value;
	    }
	    output_ptr++;

	} /* for each col */

	/* Move to the start of the next row of the output and input roi */

	output_ptr += output_offset;
	fresh_ptr += input_offset;

    } /* for each row */

    return (HIPS_OK);
}

/****************************** h_ver_rank_D **********************************
*									      *
* Description:	1D rank order double format image in the vertical dimension.  *
*									      *
* Parameters:	input_roi	*double,				      *
*			pointer to the start of the input region of interest. *
*		output_roi,	*double,			      	      *
*			pointer to the start of the output roi.		      *
*	        roi_rows,	int,					      *
*			number of rows in the roi.			      *
*		roi_cols,	int,					      *
*			number of columns in the roi.			      *
*	  	input_cols,	int,					      *
*			number of columns in the entire input frame.	      *
*	  	output_cols,	int,					      *
*			number of columns in the entire output frame.	      *
*		size		int,	       				      *
*			the size of the vertical filtering kernal.	      *
*		rank		int,	       			   	      *
*			the rank for filtering with the vertical kernal.      *
*									      *
* Return value: int,							      *
*			HIPS_ERROR - an error has occured.		      *
*			HIPS_OK    - successful execution.		      *
*									      *
* Functions Called:	expand_window_D,	      	       	       	      *
*			shrink_window_D,		       		      *
*			insert_fresh_D.					      *
*									      *
* External Functions:	perr.						      *
*									      *
* External Parameters:	none.						      *
*									      *
* Method: each column of the input frame is scanned with the filtering window.*
*         PIXEL_INDEX and RANK_INDEX hold the required information for each   *
*         pixel in the window. As the window moves down the column, the stale *
*         pixel (ie. the one just leaving the window) is removed from the     *
*         window and the fresh pixel (ie. the one just entering the window)   *
*         is inserted. The RANK_INDEX and PIXEL_INDEX are updated  	      *
*         appropriately and the value of the pixel with the required rank is  *
*         outputed. For regions near the beginning of the column, where the   *
*         entire window cannot fit in, the window size is grown from 1 pixel  *
*         upto the required size as the window moves down the col. The value  *
*         of the pixel with the appropriate rank for the reduced window size  *
*         is outputed. In a similar manner the window size is shrunk down as  *
*         the window approaches the end of the column.			      *
*									      *
******************************************************************************/

int h_ver_rank_D (input_roi,
	      output_roi,
	      roi_rows,
	      roi_cols,
	      input_cols,
	      output_cols,
	      size,
	      rank)

double 	*input_roi;
double 	*output_roi;
int 	roi_rows;
int	roi_cols;
int	input_cols;
int	output_cols;
int	size;
int	rank;
{
    static	h_boolean		allocation = FALSE;
    static	int   		*rank_index;
    static	Pixel_info_d 	*pixel_index;

    int    col;             /* the roi column of the current pixel */
    int    row;             /* the roi row of the current pixel */
    int    current_size;    /* the current window size */
    int    current_rank;    /* the rank appropriate to the current size */
    int    half_size;       /* half the required window size */
    double  *output_ptr;    /* pointer to the current output pixel */
    double  *fresh_ptr;     /* pointer to the "fresh" pixel in the i/p image */
    int    fresh_pixel;     /* pointer to the "fresh" pixel in pixel index */
    int    count;           /* counting variable for expanding/shrinking */

    if (allocation == FALSE)
    {
	/* allocate core for the rank index */
	
	if ((rank_index = (int *) calloc (size, sizeof(int))) == 0)
	  return (perr (HE_ALLOCSUBR, "h_ver_rank_D"));

	/* allocate core for the pixel index */
	
	if ((pixel_index = (Pixel_info_d *) 
	     calloc (size, sizeof(Pixel_info_d))) == 0)
	  return (perr (HE_ALLOCSUBR, "h_ver_rank_D"));

	allocation = TRUE;
    }

    half_size = (size + 1)/2;

    for (col = 0; col < roi_cols; col++)
    {
	/* Set positions to start of the current column */

	fresh_ptr = input_roi + col;
	output_ptr = output_roi + col;

	fresh_pixel = 0; /* points to the first pixel in the pixel index */

	for (row = 0; row < roi_rows; row++)
	{
	    if (row == 0) /* 1st pixel in the col */
	    {
	        current_size = 1;
		
		/* initialise the pixel index with this pixel */

		pixel_index[0].above = TOP;
		pixel_index[0].below = BOTTOM;
		pixel_index[0].value = *fresh_ptr;
		pixel_index[0].rank = 0;
		rank_index[0] = 0;

		/* output the input pixel unaltered */

		*output_ptr = *fresh_ptr;
	    }
	    else if (row < half_size) /* pixel lies in top edge area */
	    {
		/* move output_ptr down a row */
	    
		output_ptr += output_cols;

		/* Grow the window by two pixels */

		for (count = 0; count < 2; count++)
		{
		    /* expand window by appending new pixel */

		    /* move fresh_ptr to point to next pixel in the column */

		    fresh_ptr += input_cols;

		    current_size++;
		    expand_window_D (current_size,
				     *fresh_ptr,
				     rank_index,
				     pixel_index);
		}
		/* output pixel value dependant on current size */
		
		current_rank = (rank * current_size / size) + 0.5;

		*output_ptr = 
		  pixel_index[rank_index[current_rank]].value;
	    }
	    else if (row == roi_rows - 1) /* last pixel in the col */
	    {
		/* move output_ptr down a row */
	    
		output_ptr += output_cols;

		/* output the input pixel unaltered */

		*output_ptr = *fresh_ptr;
	    }
	    else if (row > roi_rows - half_size) /* pixel in bottom edge *
						  * area 		 */
	    {
		/* move output_ptr down a row */
	    
		output_ptr += output_cols;

		/* shrink window by 2 pixels */

		for (count = 0; count < 2; count++)
		{
		    /* shrink window by removing stale pixel */
		    /* Note: fresh pixel points to stale pixel also */

		    shrink_window_D (current_size,
				     fresh_pixel,
				     rank_index,
				     pixel_index);
		    current_size--;
		    fresh_pixel = (fresh_pixel + 1) % size;
		}
		/* output pixel value dependant on current size */
		
		current_rank = (rank * current_size / size) + 0.5;

		*output_ptr = 
		  pixel_index[rank_index[current_rank]].value;
	    }
	    else /* pixel lies in the body of the col */
	    {
		/* move fresh_ptr to point to next pixel in the column */

		fresh_ptr += input_cols;

		/* move output_ptr down a row */
	    
		output_ptr += output_cols;

		insert_fresh_D (size,
				fresh_pixel,
				*fresh_ptr,
				rank_index,
				pixel_index);		
		
		fresh_pixel = (fresh_pixel + 1) % size;

		*output_ptr = pixel_index[rank_index[rank]].value;
	    }

	} /* for each col */

    } /* for each row */

    return (HIPS_OK);
}
	   
/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                              EXPAND_WINDOW                                **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: expand_window_D
**
** Purpose: to expand the size of the filtering window by appending a new pixel
**          to the pixel index and rank index.
**
** Inputs: the routine accepts the following inputs:
**	       SIZE        - the pixel length of the expanded window,
**             FRESH_VALUE - the value of the pixel to be appended,
**             RANK_INDEX  - array containing the pixel position associated
**                           with each rank,
**             PIXEL_INDEX - array containing rank information for each 
**                           pixel in the window.
**
** Outputs: the routine outputs PIXEL_INDEX and RANK_INDEX updated to include
**          the new pixel.
**
** Method: the fresh pixel is appended into the TOP position in the pixel 
**         index. If its value is less than the value of the pixel which was
**         previously the TOP pixel, the fresh pixel is scanned down the pixel
**         index until it is inserted into the right place.
*/

int expand_window_D (size,
		 fresh_value,
		 rank_index,
		 pixel_index)

int    		size;
double  		fresh_value;
int    		*rank_index;
Pixel_info_d	*pixel_index;
{
    int    fresh_pixel;  /* pixel in the pixel index to be inserted */
    int    top_pixel;    /* the pixel which has the highest rank */

    fresh_pixel = size - 1;
    top_pixel = rank_index[size - 2];
    pixel_index[fresh_pixel].value = fresh_value;
	
    /* compare the value of fresh pixel with the value of the current */
    /* top pixel */

    if (fresh_value < pixel_index[top_pixel].value)
    {
	/* top pixel remains top pixel */
	/* increment rank of top pixel due to increase in size */
	/* scan down until the fresh pixel is inserted */
	
	rank_index[fresh_pixel] = top_pixel;
	pixel_index[top_pixel].rank = fresh_pixel;
	/* Note: fresh_pixel is also the current highest rank */
	
	scan_down_D (size,
		     fresh_pixel,
		     pixel_index[top_pixel].below,
		     top_pixel,
		     rank_index,
		     pixel_index,
		     fresh_pixel - 1);
    }
    else
    {
	/* fresh pixel becomes top pixel */

	rank_index[fresh_pixel] = fresh_pixel;
	pixel_index[fresh_pixel].rank = fresh_pixel;
	/* Note: fresh_pixel is also the current highest rank */
	
	pixel_index[top_pixel].above = fresh_pixel;
	pixel_index[fresh_pixel].below = top_pixel;
	pixel_index[fresh_pixel].above = TOP;
    }
    return (HIPS_OK);
}

/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                              SHRINK_WINDOW                                **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: shrink_window_D
**
** Purpose: to shrink the size of the filtering window by removing the stale
**          pixel.
**
** Inputs: the routine accepts the following inputs:
**	       SIZE        - the pixel length of the window before shrinking,
**             FRESH_PIXEL - pointer to the fresh/stale pixel in pixel index,
**             RANK_INDEX  - array containing the pixel position associated
**                           with each rank,
**             PIXEL_INDEX - array containing rank information for each 
**                           pixel in the window.
**
** Outputs: the routine outputs RANK_INDEX and PIXEL_INDEX, updated to exclude
**          the stale pixel.
**
** Method: the stale pixel is given a value one greater than the current
**         highest value. The stale pixel is then scanned up the pixel index
**         until it is inserted into the TOP position. The pixel below 
**         the stale pixel is then made the TOP pixel, removing the stale 
**         pixel from the pixel index. The window size must be decremented 
**         outside the routine after it is called.
*/

int shrink_window_D (size,
		 fresh_pixel,
		 rank_index,
		 pixel_index)

int    		size;
int    		fresh_pixel;
int    		*rank_index;
Pixel_info_d	*pixel_index;
{
    int   top_pixel;    /* the pixel which currently has the highest rank */
    int   above_pixel;  /* the pixel above the stale pixel */
    int   below_pixel;  /* the pixel below the stale pixel */
    int   current_rank; /* the current_rank of the fresh pixel */

    /* fresh_pixel holds the stale pixel information */

    current_rank = pixel_index[fresh_pixel].rank;
    below_pixel  = pixel_index[fresh_pixel].below;
    above_pixel  = pixel_index[fresh_pixel].above;

    if (above_pixel != TOP)
    {
	top_pixel = rank_index[size - 1];
        pixel_index[fresh_pixel].value = 
	  pixel_index[top_pixel].value + 1;

	/* move above pixel down a rank due to the removal of stale pixel */

	pixel_index[above_pixel].rank--;
	rank_index[current_rank] = above_pixel;
	pixel_index[below_pixel].above = above_pixel;
	pixel_index[above_pixel].below = below_pixel;

	/* scan up until the stale pixel reaches the TOP */

	scan_up_D (size,
		   fresh_pixel,
		   above_pixel,
		   pixel_index[above_pixel].above,
		   rank_index,
		   pixel_index,
		   ++current_rank);
    }
    /* pixel below fresh pixel is now made the top pixel */

    pixel_index[pixel_index[fresh_pixel].below].above = TOP;

    return (HIPS_OK);
}
	
/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                              INSERT_FRESH                                 **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: insert_fresh_D
**
** Purpose: overwrites the "stale" pixel information in the pixel index with
**          the information for the "fresh" pixel, updating the whole of the
**          pixel index and rank index.
**
** Inputs: the routine accepts the following inputs:
**             SIZE          - the size of the filtering window,
**             FRESH_PIXEL   - pointer to the "fresh" pixel in the pixel index,
**	       FRESH_VALUE   - the pixel value of the "fresh" pixel,
**	       RANK_INDEX    - array containing the pixel position associated
**                             with each rank,
**	       PIXEL_INDEX   - array containing rank information for each 
**                             pixel in the window.
**
** Outputs: the routine updates RANK_INDEX and PIXEL_INDEX to include the
**          fresh pixel and exclude the stale pixel.
**
** Method: the fresh pixel is inserted into the stale pixel's position in the
**         pixel index. If the value of the fresh pixel is greater than the
**         pixel above it, the fresh pixel is scanned up the pixel index until
**         it is inserted in the correct place. If the value of the fresh pixel
**         is less than the value of the pixel below it, the fresh pixel is
**         scanned down the pixel index until it is inserted in the correct
**         place.
*/

int insert_fresh_D (size,
		fresh_pixel,
		fresh_value,
		rank_index,
		pixel_index)

int    		size;
int    		fresh_pixel;
double  		fresh_value;
int    		*rank_index;
Pixel_info_d	*pixel_index;
{
    int   above_pixel;  /* the pixel above the stale pixel */
    int   below_pixel;  /* the pixel below the stale pixel */
    int   current_rank; /* the current_rank of the fresh pixel */

    /* fresh_pixel currently holds the stale pixel information */

    current_rank = pixel_index[fresh_pixel].rank;
    below_pixel = pixel_index[fresh_pixel].below;
    above_pixel = pixel_index[fresh_pixel].above;
    pixel_index[fresh_pixel].value = fresh_value;

    if (below_pixel == BOTTOM)
    { 
	if (fresh_value > pixel_index[above_pixel].value) 
	{
	    /* above pixel becomes new bottom pixel and is moved down a rank */

	    pixel_index[above_pixel].rank--;
	    rank_index[current_rank] = above_pixel;
	    pixel_index[above_pixel].below = BOTTOM;

	    /* scan up until the correct position for fresh pixel is found */

	    scan_up_D (size,
		       fresh_pixel,
		       above_pixel,
		       pixel_index[above_pixel].above,
		       rank_index,
		       pixel_index,
		       ++current_rank);
	}
	/* else fresh pixel overwrites stale pixel directly */
    }
    else if (above_pixel == TOP) 
    {
	if (fresh_value < pixel_index[below_pixel].value)
	{
	    /* below pixel becomes new top pixel and is moved up a rank */

	    pixel_index[below_pixel].rank++;
	    rank_index[current_rank] = below_pixel;
	    pixel_index[below_pixel].above = TOP;

	    /* scan down until the correct position for fresh pixel is found */

	    scan_down_D (size,
			 fresh_pixel,
			 pixel_index[below_pixel].below,
			 below_pixel,
			 rank_index,
			 pixel_index,
			 --current_rank);
	}
	/* else fresh pixel overwrites stale pixel directly */
    }
    else if (fresh_value < pixel_index[below_pixel].value)
    {
	/* move below pixel up a rank due to the removal of stale pixel */

	pixel_index[below_pixel].rank++;
	rank_index[current_rank] = below_pixel;
	pixel_index[below_pixel].above = above_pixel;
	pixel_index[above_pixel].below = below_pixel;

	/* scan down until the correct position for fresh pixel is found */

	scan_down_D (size,
		     fresh_pixel,
		     pixel_index[below_pixel].below,
		     below_pixel,
		     rank_index,
		     pixel_index,
		     --current_rank);
    }
    else if (fresh_value > pixel_index[above_pixel].value)
    {
	/* move above pixel down a rank due to the removal of stale pixel */

	pixel_index[above_pixel].rank--;
	rank_index[current_rank] = above_pixel;
	pixel_index[below_pixel].above = above_pixel;
	pixel_index[above_pixel].below = below_pixel;

	/* scan up until the correct position for fresh pixel is found */

	scan_up_D (size,
		   fresh_pixel,
		   above_pixel,
		   pixel_index[above_pixel].above,
		   rank_index,
		   pixel_index,
		   ++current_rank);
    }
    /* else fresh pixel overwrites stale pixel directly */

    return (HIPS_OK);
}

/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                               SCAN_DOWN                                   **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: scan_down_D
**
** Purpose: to recursively scan down through the pixel index until the correct
**          position for the fresh pixel is found. The fresh pixel is then
**	    inserted. The pixel index is updated as it is scanned.
**
** Inputs: the routine accepts the following inputs:
**             SIZE          - the size of the filtering window,
**             FRESH_PIXEL   - pointer to the "fresh" pixel in the pixel index,
**             BELOW_PIXEL   - pointer to the pixel below the "fresh" pixel,
**             ABOVE_PIXEL   - pointer to the pixel above the "fresh" pixel,
**	       RANK_INDEX    - array containing the pixel position associated
**                             with each rank,
**	       PIXEL_INDEX   - array containing rank information for each 
**                             pixel in the window,
**             CURRENT_RANK  - the current rank of the "fresh" pixel.
**
** Outputs: the routine updates RANK_INDEX and PIXEL_INDEX to include the
**          fresh pixel and exclude the stale pixel.
*/

int scan_down_D (size,
	     fresh_pixel,
	     below_pixel,
	     above_pixel,
	     rank_index,
	     pixel_index,
	     current_rank)

int    		size;
int    		fresh_pixel;
int    		below_pixel;
int    		above_pixel;
int    		*rank_index;
Pixel_info_d	*pixel_index;
int    current_rank;
{
    if (below_pixel == BOTTOM)
    {
	/* fresh pixel becomes the new bottom pixel */

	rank_index[0] = fresh_pixel;
	pixel_index[fresh_pixel].rank = 0;
	pixel_index[fresh_pixel].below = BOTTOM;
	pixel_index[fresh_pixel].above = above_pixel;
	pixel_index[above_pixel].below = fresh_pixel;
    }
    else if (pixel_index[fresh_pixel].value < pixel_index[below_pixel].value)
    {
	/* scan down again */

	rank_index[current_rank] = below_pixel;
	pixel_index[below_pixel].rank = current_rank;

	scan_down_D (size,
		     fresh_pixel,
		     pixel_index[below_pixel].below,
		     below_pixel,
		     rank_index,
		     pixel_index,
		     --current_rank);
    }
    else
    {
	/* fresh pixel is inserted in the current position */

	rank_index[current_rank] = fresh_pixel;
	pixel_index[fresh_pixel].rank = current_rank;
	pixel_index[fresh_pixel].below = below_pixel;
	pixel_index[below_pixel].above = fresh_pixel;
	pixel_index[fresh_pixel].above = above_pixel;
	pixel_index[above_pixel].below = fresh_pixel;
    }
    return (HIPS_OK);
}

/*
*******************************************************************************
*******************************************************************************
**                                                                           **
**                                SCAN_UP                                    **
**                                                                           **
*******************************************************************************
*******************************************************************************
**
** Name: scan_up_D
**
** Purpose: to recursively scan up through the pixel index until the correct
**          position for the fresh pixel is found. The fresh pixel is then
**	    inserted. The pixel index is updated as it is scanned.
**
** Inputs: the routine accepts the following inputs:
**             SIZE          - the size of the filtering window,
**             FRESH_PIXEL   - pointer to the "fresh" pixel in the pixel index,
**             BELOW_PIXEL   - pointer to the pixel below the "fresh" pixel,
**             ABOVE_PIXEL   - pointer to the pixel above the "fresh" pixel,
**	       RANK_INDEX    - array containing the pixel position associated
**                             with each rank,
**	       PIXEL_INDEX   - array containing rank information for each 
**                             pixel in the window,
**             CURRENT_RANK  - the current rank of the "fresh" pixel.
**
** Outputs: the routine updates RANK_INDEX and PIXEL_INDEX to include the
**          fresh pixel and exclude the stale pixel.
*/

int scan_up_D (size,
	   fresh_pixel,
	   below_pixel,
	   above_pixel,
	   rank_index,
	   pixel_index,
	   current_rank)

int    		size;
int    		fresh_pixel;
int    		below_pixel;
int    		above_pixel;
int    		*rank_index;
Pixel_info_d 	*pixel_index;
int    current_rank;
{
    if (above_pixel == TOP)
    {
	/* fresh pixel becomes the new top pixel */

	rank_index[current_rank] = fresh_pixel;
	pixel_index[fresh_pixel].rank = current_rank;
	pixel_index[fresh_pixel].below = below_pixel;
	pixel_index[fresh_pixel].above = TOP;
	pixel_index[below_pixel].above = fresh_pixel;
    }
    else if (pixel_index[fresh_pixel].value > pixel_index[above_pixel].value)
    {
	/* scan up again */

	rank_index[current_rank] = above_pixel;
	pixel_index[above_pixel].rank = current_rank;

	scan_up_D (size,
		   fresh_pixel,
		   above_pixel,
		   pixel_index[above_pixel].above,
		   rank_index,
		   pixel_index,
		   ++current_rank);
    }
    else
    {
	/* fresh pixel is inserted in the current position */

	rank_index[current_rank] = fresh_pixel;
	pixel_index[fresh_pixel].rank = current_rank;
	pixel_index[fresh_pixel].below = below_pixel;
	pixel_index[below_pixel].above = fresh_pixel;
	pixel_index[fresh_pixel].above = above_pixel;
	pixel_index[above_pixel].below = fresh_pixel;
    }
    return (HIPS_OK);
}
