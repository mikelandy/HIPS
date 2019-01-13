
/*  bthin.c                                      Brian Tierney,  LBL  3/90
 *
 *  usage: bthin [-v NN] < infile > outfile
 *    where -v NN set the value for 'on' pixels  (default = 255)
 *
 * Notes on this algorithm:
 *  this program should be run in conjunction with bclean and fill_holes.
 *  It often leave single pixel fragments that should be
 *  removed with bclean. On sharp bends in the object, it often
 *  leaves holes which should be filled in with fill_holes.
 *
 * sample use: bthin < in | bclean | fill_holes -e > out
 */

/*   This program is copyright (C) 1990, Regents  of  the
University  of  California.   Anyone may reproduce this software,
in whole or in part, provided that:
(1)  Any copy  or  redistribution  must  show  the
     Regents  of  the  University of California, through its
     Lawrence Berkeley Laboratory, as the source,  and  must
     include this notice;
(2)  Any use of this software must reference this  distribu-
     tion,  state that the software copyright is held by the
     Regents of the University of California, and  that  the
     software is used by their permission.

     It is acknowledged that the U.S. Government has  rights
to this software under  Contract DE-AC03-765F00098 between the U.S.
Department of Energy and the University of California.

     This software is provided as a professional  academic  contribu-
tion  for  joint exchange.  Thus it is experimental, is pro-
vided ``as is'', with no warranties of any kind  whatsoever,
no  support,  promise  of updates, or printed documentation.
Bug reports or fixes may be sent to the author, who may or may
not act on them as he desires.
*/

/*   Author:  Brian L. Tierney
 *            Lawrence Berkeley Laboratory
 *            Imaging and Distributed Computing Group
 *            email: bltierney@lbl.gov
*/

#include <stdio.h>
#include <sys/types.h>

#include <hipl_format.h>

#define HVAL 255

/*************************************************************/
main(argc, argv)
    int       argc;
    char     *argv[];
{
    register int f;
    struct header hd;
    int       pval;

    u_char  **pic, **newpic;
    u_char  **alloc_2d_byte_array();

    Progname = strsave(*argv);

    pval = HVAL;		/* default */
    if (argc > 1) {
	if (strcmp(argv[1], "-v") == 0)
	    pval = atoi(argv[2]);
	else
	    usageterm();
    }
    read_header(&hd);
    if (hd.pixel_format != PFBYTE)
	perr("image pixel format must be byte");
    update_header(&hd, argc, argv);
    write_header(&hd);

    pic = alloc_2d_byte_array(hd.orows, hd.ocols);
    newpic = alloc_2d_byte_array(hd.orows, hd.ocols);

    for (f = 0; f < hd.num_frame; f++) {
#ifdef OLD
	read_2d_byte_array(stdin, pic, hd.orows, hd.ocols);
#else
	hd.image = pic[0];
	if (read_image(hd, f) == HIPS_ERROR)
	    return (HIPS_ERROR);
#endif
	thin_binary_image(pic, newpic, hd.orows, hd.ocols, pval);

#ifdef OLD
	write_2d_byte_array(stdout, newpic, hd.orows, hd.ocols);
#else
	hd.image = newpic[0];
	if (write_image(hd, f) == HIPS_ERROR)
	    return (HIPS_ERROR);
#endif
	bzero(newpic[0], hd.orows * hd.ocols);
    }
    fprintf(stderr, " Done. \n\n");
    return (0);
}

/********************************************************************/

thin_binary_image(image, out_image, nrow, ncol, pval)
    u_char  **image, **out_image;
    int       ncol, nrow, pval;

{

    register int i, j, i1, j1, i2, j2, ni, nj, di, dj;

    for (i = 0; i < nrow; i++) {
	for (j = 0; j < ncol; j++) {

	    if (image[i][j] > 0) {	/* pixel is part of an object */
		i1 = i2 = i;
		j1 = j2 = j;

		/* find 4 points for edge of the circle  */

		while (i1 < nrow && image[i1][j] > 0)
		    i1++;

		while (i2 > 0 && image[i2][j] > 0)
		    i2--;

		while (j1 < ncol && image[i][j1] > 0)
		    j1++;

		while (j2 > 0 && image[i][j2] > 0)
		    j2--;


		di = i1 - i2;
		dj = j1 - j2;

		/* calc new center based on smaller diameter */
		if (di <= 1 || dj <= 1) {
		    ni = i;
		    nj = j;
		    out_image[ni][nj] = pval;
		} else if (di > dj) {
		    ni = i;
		    nj = (j1 + j2) / 2;
		    out_image[ni][nj] = pval;
		} else if (di < dj) {
		    ni = (i1 + i2) / 2;
		    nj = j;
		    out_image[ni][nj] = pval;
		} else {	/* d1 == dj, do both */
		    ni = i;
		    nj = (j1 + j2) / 2;
		    out_image[ni][nj] = pval;
		    ni = (i1 + i2) / 2;
		    nj = j;
		    out_image[ni][nj] = pval;
		}
	    }			/* if */
	}			/* for j */
    }				/* for i */
}

/*********************************************************************/

usageterm()
{
    fprintf(stderr, "Usage: bthin [-v nn] < inseq > outseq \n ");
    fprintf(stderr, " Options: [-v nn]  pixel value for thinned objects (default=255) \n\n");
    exit(0);
}
