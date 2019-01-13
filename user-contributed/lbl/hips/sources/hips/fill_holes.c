
/*   fill_holes.c
 *  fills holes in a binary images
 *
 *  usage: fill_holes [-s NN] < infile > outfile
 *    where -s NN the size of holes to look for ( default = 2)
 *
 *   The main purpose of this program is to fix flaws in the bthin
 *    algorithm.  The 'bthin' program uses a very simple method to
 *    thin binary objects to a single pixel line, and an artifact
 *    of this algorithm is that the thinned objects will often have
 *    holes in them.  This program will fill those holes.
 *
 *   The input image must be a binary (gray values of 0 and 255 only).
 */

/*   Author:  Brian L. Tierney
 *            Lawrence Berkeley Laboratory
 *            Imaging Technolies Group
 *            email: bltierney@lbl.gov
 *
 *    converted to HIPS2
 *   	      Felix K. Huang
 *            Lawrence Berkeley Laboratory
 *            email: fhuang@george.lbl.gov
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

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
    {"s", {LASTFLAG}, 1, {{PTINT, "3", "hole size"}, LASTPARAMETER}},
    {"e", {LASTFLAG}, 0, {{PTBOOLEAN, "FALSE", "ends_only"}, LASTPARAMETER}},
LASTFLAG};

int       types[] = {PFBYTE, LASTTYPE};

int       hole_size;
h_boolean   ends_only;

int       nrow;
int       ncol;
int       i_ocol;

/*************************************************************/
main(argc, argv)
    int       argc;
    char     *argv[];
{
    struct header hd, hdp, hdo;
    int       method, fr, f;
    Filename  filename;
    FILE     *fp;
    h_boolean   imagecopy;
    struct hips_roi roi;

    Progname = strsave(*argv);
    parseargs(argc, argv, flagfmt, &hole_size, &ends_only, FFONE, &filename);

    fp = hfopenr(filename);
    fread_hdr_a(fp, &hd, filename);
    method = fset_conversion(&hd, &hdp, types, filename);

    imagecopy = FALSE;
    if (hdp.rows != hdp.orows || hdp.cols != hdp.ocols)
	imagecopy = TRUE;

    dup_headern(&hdp, &hdo);
    alloc_image(&hdo);
    write_headeru(&hdo, argc, argv);
    getroi(&hdp, &roi);
    fr = hd.num_frame;
    nrow = hd.rows;
    ncol = hd.cols;
    i_ocol = hd.ocols;

    for (f = 0; f < fr; f++) {
	fprintf(stderr, "%s: starting frame #%d\n", Progname, f);
	fread_imagec(fp, &hd, &hdp, method, f, filename);
	if (imagecopy) {
	    clearroi(&hdp);
	    clearroi(&hdo);
	    h_copy(&hdp, &hdo);
	    setroi2(&hdp, &roi);
	    setroi2(&hdo, &roi);
	}
	h_fill_holes(&hdp, &hdo);
	write_imagec(&hdo, &hd, method, TRUE, f);
    }
    fprintf(stderr, "%s done.\n\n", argv[0]);
    return (0);

}				/* end of  main of fill_holes */
