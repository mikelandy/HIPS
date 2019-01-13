/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * mask_utils.c - Convolution mask-set I/O and memory management
 *
 * These routines read, allocate and free sets of convolution masks (used by
 * h_mask and mask).  A convolution mask set consists of 1 or more masks
 * (arrays of floats or ints), each with associated size and row and column
 * offsets (to denote which mask pixel is centered on the image pixel before
 * cross-correlating (***important note***, h_mask cross-correlates with the
 * masks rather than convolves; in other words, the rows and columns are not
 * first reflected;  for the usual mirror symmetric masks this poses no
 * complications).  The masks are read into a mask structure, which gives the
 * mask name, format (PFFLOAT or PFINT), size, offsets, and mask values.
 *
 * read_num_mask is used to read a standard HIPS mask set (usually kept in
 * /usr/hips/masks/mask#).  All of these masks have integer mask values, and
 * hence may be read either as ints or as floats.  read_mask reads a mask set
 * from a file.
 *
 * The format of the filter definition file is as follows:
 *
 *	"filter name"
 *	number-of-masks function-number mask-format
 *	
 *	mask-1-rows mask-1-cols mask-1-rowoffset mask-1-coloffset
 *	mask-1-values
 *	  .
 *	  .
 *	  .
 *	mask-(number-of-masks)-rows cols rowoffset coloffset
 *	mask-(number-of-masks)-values
 *
 * mask-rows and mask-cols give the side lengths of the rectangular mask.
 * mask-rowoffset and mask-coloffset identify the pixel which overlaps a given
 * image position to produce the mask value corresponding to that image
 * position.  Note that the earlier mask values are applied to earlier image
 * values.  Thus, the interpretation of the mask orientation depends on the
 * definition of ULORIG.  If ULORIG is defined (images have their origin at
 * the upper-left), then the first mask row is topmost relative to the image.
 * Otherwise, the last mask row is topmost relative to the image.  Mask
 * values are given as a sequence of integers or floats in column-fastest
 * order.
 *
 * Mask sets can include 1 or more masks.  The mask program applies a
 * function to the set of mask output values which results in the single
 * pixel value placed in a given position in the output image.  The second
 * line of the mask definition identifies which function is to be used,
 * chosen from:
 *
 *	1	MASKFUN_MAXABS	- the maximum absolute value of all mask outputs
 *	2	MASKFUN_MEANSQ  - the square root of the sum of the squares of
 *				  all masks
 *	3	MASKFUN_SUMABS  - the sum of the absolute value of all mask
 *				  outputs
 *	4	MASKFUN_MAX	- the maximum mask output
 *	5	MASKFUN_MAXFLR	- the maximum mask output, floored at zero
 *	6	MASKFUN_MXASFLR	- the larger of |mask-1| and |mask-2|, minus
 *				  |mask-3|, floored at zero
 *	7	MASKFUN_MUL	- the product of the mask outputs, each floored
 *				  at zero
 *	8	MASKFUN_NORM	- the first mask output normalized by the sum
 *				  of the mask entries
 *	9	MASKFUN_DIFF	- the value of the pixel minus the normalized
 *				  mask output
 *	10	MASKFUN_ORIENT	- compute orientation:
 *				  360*atan(mask2/mask1)/2*PI
 *	11	MASKFUN_IDENT	- the value of the first mask output (simple
 *				  convolution)
 *
 * free_maskcon is used to free the arrays in a mask structure.
 * mask_itof converts an integer mask set to floating point.
 *
 * Based on HIPS-1 mask: Michael Landy - 4/21/82
 * HIPS-2 - msl - 7/12/91
 */

#include <hipl_format.h>
#include <stdio.h>

char masklib[] = MSKLIB;

int read_num_mask(mask,masknum)

struct hips_mask *mask;
int masknum;

{
	char maskfile[150];

	strcpy(maskfile,masklib);
	strcat(maskfile,"/mask");
	sprintf(maskfile+strlen(maskfile),"%d",masknum);
	return(read_mask(mask,maskfile));
}

int read_mask(mask,maskfile)

struct hips_mask *mask;
Filename maskfile;

{
	FILE *filep;
	int i,mr,mc,*ip;
	char name[100],*s;
	float *fp;

	if ((filep = ffopen(maskfile,"r")) == (FILE *) HIPS_ERROR)
		return(HIPS_ERROR);
	s = name;
	while ((*s++ = getc(filep)) != '\n');
	*s++ = '\0';
	mask->name = strsave(name);
	if (fscanf(filep,"%d %d %d",&(mask->nmasks),&(mask->func_num),
	    &(mask->pixel_format)) != 3)
		return(perr(HE_READFILE,maskfile));
	if (mask->func_num < 1 || mask->func_num > MASKFUN_MAXMASKS)
		return(perr(HE_MSKFUNFILE,mask->func_num,maskfile));
	if (mask->pixel_format != PFINT && mask->pixel_format != PFFLOAT)
		return(perr(HE_FMTSUBRFILE,
			"read_mask",hformatname(mask->pixel_format),maskfile));
	if (mask->pixel_format == PFINT) {
		if ((mask->vals.i_values =
			(int **) memalloc(mask->nmasks,sizeof(int *)))
			== (int **) HIPS_ERROR)
				return(HIPS_ERROR);
	}
	else {
		if ((mask->vals.f_values =
			(float **) memalloc(mask->nmasks,sizeof(float *)))
			== (float **) HIPS_ERROR)
				return(HIPS_ERROR);
	}
	if ((mask->mask_rows = (int *) memalloc(mask->nmasks,sizeof(int)))
		== (int *) HIPS_ERROR)
			return(HIPS_ERROR);
	if ((mask->mask_cols = (int *) memalloc(mask->nmasks,sizeof(int)))
		== (int *) HIPS_ERROR)
			return(HIPS_ERROR);
	if ((mask->row_offset = (int *) memalloc(mask->nmasks,sizeof(int)))
		== (int *) HIPS_ERROR)
			return(HIPS_ERROR);
	if ((mask->col_offset = (int *) memalloc(mask->nmasks,sizeof(int)))
		== (int *) HIPS_ERROR)
			return(HIPS_ERROR);
	for (i=0;i < mask->nmasks;i++) {
		if (fscanf(filep,"%d %d %d %d",&(mask->mask_rows[i]),
			&(mask->mask_cols[i]),
			&(mask->row_offset[i]),&(mask->col_offset[i])) != 4)
				return(perr(HE_READFILE,maskfile));
		if (mask->pixel_format == PFINT) {
			if ((ip = (int *) memalloc((mask->mask_rows[i]) *
				(mask->mask_cols[i]),
				sizeof (int))) == (int *) HIPS_ERROR)
					return(HIPS_ERROR);
			mask->vals.i_values[i] = ip;
			for (mr=0;mr < mask->mask_rows[i];mr++) {
			    for (mc=0;mc < mask->mask_cols[i];mc++) {
				if (fscanf(filep,"%d",ip++) != 1)
					return(perr(HE_READFILE,maskfile));
			    }
			}
		}
		else {
			if ((fp = (float *) memalloc((mask->mask_rows[i]) *
				(mask->mask_cols[i]),
				sizeof (float))) == (float *) HIPS_ERROR)
					return(HIPS_ERROR);
			mask->vals.f_values[i] = fp;
			for (mr=0;mr < mask->mask_rows[i];mr++) {
			    for (mc=0;mc < mask->mask_cols[i];mc++) {
				if (fscanf(filep,"%f",fp++) != 1)
					return(perr(HE_READFILE,maskfile));
			    }
			}
		}
	}
	fclose(filep);
	return(HIPS_OK);
}

int free_maskcon(mask)

struct hips_mask *mask;

{
	int i;

	if (mask->nmasks == 0)
		return(HIPS_OK);
	free(mask->mask_rows);
	free(mask->mask_cols);
	free(mask->row_offset);
	free(mask->col_offset);
	for (i=0;i < mask->nmasks;i++)
		free(mask->vals.f_values[i]);	/* this works for ints too */
	free(mask->vals.f_values);
	return(HIPS_OK);
}

int mask_itof(mask)

struct hips_mask *mask;

{
	int m,mr,mc,*ip;
	float *fp,*fp2;

	if (sizeof(float) == sizeof(int)) {
		for (m=0;m < mask->nmasks;m++) {
			ip = mask->vals.i_values[m];
			fp = (float *) ip;
			for (mr=0;mr < mask->mask_rows[m];mr++)
				for (mc=0;mc < mask->mask_cols[m];mc++)
					*fp++ = *ip++;
		}
	}
	else {
		for (m=0;m < mask->nmasks;m++) {
			if ((fp = (float *) memalloc(mask->mask_rows[m] *
				mask->mask_cols[m],sizeof(float))) ==
				(float *) HIPS_ERROR)
					return(HIPS_ERROR);
			ip = mask->vals.i_values[m];
			fp2 = fp;
			for (mr=0;mr < mask->mask_rows[m];mr++)
				for (mc=0;mc < mask->mask_cols[m];mc++)
					*fp++ = *ip++;
			free(mask->vals.i_values[m]);
			mask->vals.f_values[m] = fp2;	/* works because float
							   and int pointers
							   have the same
							   length */
		}
	}
	mask->pixel_format = PFFLOAT;
	return(HIPS_OK);
}
