
/* addmask.c                                 Brian Tierney, LBL   4/90
 *
 *   usage:   addmask [-n] mask_image < image > new_image
 *
 *   creates new image which is the input image where the mask
 *    value is > 0, and zero everywhere else.
 *
 *  Works with data types: Byte, short, int, float, and complex.
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
 *            Imaging Technologies Group
 *            email: bltierney@lbl.gov
 *
 *  converted to HIPS2:   Felix Huang
*/

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
    {"n", {LASTFLAG}, 0, {{PTBOOLEAN, "FALSE", "negative"}, LASTPARAMETER}},
LASTFLAG};

int       types[] = {PFBYTE, PFSHORT, PFINT, PFFLOAT, PFCOMPLEX, LASTTYPE};

h_boolean   neg;

int       m_type[] = {PFBYTE, LASTTYPE};	/* for mask image	 */

struct header m_hd2;		/* mask image header		 */

main(argc, argv)
    int       argc;
    char    **argv;
{
    struct header hd, hdp, hdo;
    /* struct hips_mask mask; */
    int       method, fr, f, m_method;
    Filename  filename, mask_file;
    FILE     *fp, *m_fp;
    h_boolean   imagecopy;
    struct hips_roi roi;
    struct header m_hd;

    Progname = strsave(*argv);
    parseargs(argc, argv, flagfmt, &neg, FFTWO, &mask_file, &filename);

    fp = hfopenr(filename);
    fread_hdr_a(fp, &hd, filename);
    method = fset_conversion(&hd, &hdp, types, filename);

    m_fp = hfopenr(mask_file);
    fread_hdr_a(m_fp, &m_hd, mask_file);
    m_method = fset_conversion(&m_hd, &m_hd2, m_type, mask_file);

    if (hdp.frow + hdp.rows > m_hd2.orows ||
	hdp.fcol + hdp.cols > m_hd2.ocols) {
	fprintf(stderr, "input image is beyond mask.\n");
	exit(-1);
    }
    imagecopy = FALSE;
    if (hdp.rows != hdp.orows || hdp.cols != hdp.ocols)
	imagecopy = TRUE;

    dup_headern(&hdp, &hdo);
    alloc_image(&hdo);
    write_headeru(&hdo, argc, argv);
    getroi(&hdp, &roi);
    fr = hd.num_frame;

    for (f = 0; f < fr; f++) {
	fprintf(stderr, "%s: starting frame #%d\n", Progname, f);
	fread_imagec(fp, &hd, &hdp, method, f, filename);
        fread_imagec(m_fp, &m_hd, &m_hd2, m_method, 0, mask_file);
	if (imagecopy) {
	    clearroi(&hdp);
	    clearroi(&hdo);
	    h_copy(&hdp, &hdo);
	    setroi2(&hdp, &roi);
	    setroi2(&hdo, &roi);
	}
	h_addmask(&hdp, &hdo);
	/* write_image(&hdo,f);  */
	write_imagec(&hdo, &hd, method, TRUE, f);
    }
    return (0);

}				/* end of  main of addmask		 */
