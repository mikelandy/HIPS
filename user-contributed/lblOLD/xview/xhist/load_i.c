
#include <stdio.h>
#include <hipl_format.h>

int       types[] = {PFBYTE, PFSHORT, PFINT, PFFLOAT, LASTTYPE};

extern char *cur_fname;		/* current file name */

extern byte *barray;
extern short *sarray;
extern int *iarray;
extern float *farray;

extern int pix_format;		/* byte, short, int, float	 */

extern int size;		/* size of image		 */

#define Calloc(x,y) (y *)calloc((unsigned)(x), sizeof(y))

int       nrow;
int       ncol;
int       i_ocol;
int       nfr;

byte     *qb;
short    *qs;
int      *qi;
float    *qf;

/**********************************************************/
int
load_image()
{
    struct header hd, hdp;
    int       method, f;
    FILE     *fp = NULL;
    int       start, finish;	/* for report time	 */

    fprintf(stderr, " Loading image... \n");

    if((fp = hfopenr(cur_fname)) == (FILE *)HIPS_ERROR) 
	return(-1);
    if (fread_hdr_a(fp, &hd, cur_fname) == HIPS_ERROR) {
	fclose(fp);
	free_image(&hd);
	return (-1);
    }
    method = fset_conversion(&hd, &hdp, types, cur_fname);

    nrow = hdp.rows;
    ncol = hdp.cols;
    i_ocol = hdp.ocols;

    nfr = hdp.num_frame;

    size = nrow * ncol * nfr;

    pix_format = hdp.pixel_format;

    switch (pix_format) {
    case PFBYTE:
	if ((barray = Calloc(size, byte)) == NULL) {
	    perror("calloc error: barray ");
	    goto f_error;
	}
	qb = barray;
	break;
    case PFSHORT:
	if ((sarray = Calloc(size, short)) == NULL) {
	    perror("calloc error: sarray ");
	    goto f_error;
	}
	qs = sarray;
	break;
    case PFINT:
	if ((iarray = Calloc(size, int)) == NULL) {
	    perror("calloc error: iarray ");
	    goto f_error;
	}
	qi = iarray;
	break;
    case PFFLOAT:
	if ((farray = Calloc(size, float)) == NULL) {
	    perror("calloc error: farray ");
	    goto f_error;
	}
	qf = farray;
	break;
    }				/* end of switch( pixel_format )	 */

    for (f = 0; f < nfr; f++) {
	fprintf(stderr, "%s: starting frame #%d\n", cur_fname, f);
	if (fread_imagec(fp, &hd, &hdp, method, f, cur_fname) == HIPS_ERROR) {
	    fprintf(stderr,"%s\n",hipserr);
	    return (-1);
	}
	build_array(&hdp);
    }

    start = time(NULL);
    comp_hist();		/* to compute histogram	 */
    finish = time(NULL);
    printf("\n time spent computing histogram = %d sec\n\n", finish - start);

    return 0;

f_error:
    size = 0;			/* no image		 */
    return (-1);

}				/* end of load_image ()			 */

build_array(hdi)
    struct header *hdi;
{
    switch (pix_format) {
    case PFBYTE:
	bd_barray(hdi);
	break;
    case PFSHORT:
	bd_sarray(hdi);
	break;
    case PFINT:
	bd_iarray(hdi);
	break;
    case PFFLOAT:
	bd_farray(hdi);
	break;
    }
}				/* end of build_array (hdi)	  */

bd_barray(hdi)
    struct header *hdi;
{
    int       x, y;
    byte     *p;

    p = hdi->firstpix;

    for (y = 0; y < nrow; y++, p = p + i_ocol)
	for (x = 0; x < ncol; x++, qb++)
	    *qb = p[x];
}

bd_sarray(hdi)
    struct header *hdi;
{
    int       x, y;
    short    *p;

    p = (short *) hdi->firstpix;

    for (y = 0; y < nrow; y++, p = p + i_ocol)
	for (x = 0; x < ncol; x++, qs++)
	    *qs = p[x];
}

bd_iarray(hdi)
    struct header *hdi;
{
    int       x, y;
    int      *p;

    p = (int *) hdi->firstpix;

    for (y = 0; y < nrow; y++, p = p + i_ocol)
	for (x = 0; x < ncol; x++, qi++)
	    *qi = p[x];
}

bd_farray(hdi)
    struct header *hdi;
{
    int       x, y;
    float    *p;

    p = (float *) hdi->firstpix;

    for (y = 0; y < nrow; y++, p = p + i_ocol)
	for (x = 0; x < ncol; x++, qf++)
	    *qf = p[x];
}
