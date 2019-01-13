/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * maxformat.c - find the most appropriate pixel format for multiple images
 *
 * hformatlevel - return the `level' of a given pixel format
 *
 * Michael Landy - 1/14/91
 */

#include <hipl_format.h>

int hformatlevel(pfmt)

int pfmt;

{
	switch(pfmt) {
	case PFMSBF:	return(1);
	case PFLSBF:	return(1);
	case PFBYTE:	return(2);
	case PFSBYTE:	return(2);
	case PFUSHORT:	return(3);
	case PFSHORT:	return(3);
	case PFUINT:	return(4);
	case PFINT:	return(4);
	case PFINTPYR:	return(4);
	case PFFLOAT:	return(5);
	case PFFLOATPYR:return(5);
	case PFDOUBLE:	return(6);
	case PFCOMPLEX:	return(6);
	case PFDBLCOM:	return(7);
	default:	return(HIPS_ERROR);
	}
}

int maxformat(pfmt1,pfmt2,typeslist,fname1,fname2)

int pfmt1,pfmt2,*typeslist;
Filename fname1,fname2;

{
	int lev1,lev2,mfmt,i;
	char fnames[200];

	if (pfmt1 == pfmt2)
		mfmt = pfmt1;
	else {
	    if (ptype_is_col3(pfmt1))
		pfmt1 = PFBYTE;
	    if (ptype_is_col3(pfmt2))
		pfmt2 = PFBYTE;
	    if (pfmt1 == pfmt2)
		mfmt = pfmt1;
	    else {
		if ((lev1 = hformatlevel(pfmt1)) == HIPS_ERROR)
			return(perr(HE_FMTFILE,hformatname(pfmt1),fname1));
		if ((lev2 = hformatlevel(pfmt2)) == HIPS_ERROR)
			return(perr(HE_FMTFILE,hformatname(pfmt2),fname2));
		if (lev1 == lev2) {
			switch(pfmt1) {
			case PFMSBF:
			case PFLSBF:	mfmt = PFBYTE; break;
			case PFBYTE:
			case PFSBYTE:	mfmt = PFSHORT; break;
			case PFUINT:
			case PFINT:
			case PFINTPYR:	if (pfmt1 == PFINTPYR ||
					    pfmt2 == PFINTPYR) {
						if (pfmt1 == PFINT ||
						    pfmt2 == PFINT) {
							mfmt = PFINTPYR;
							break;
						}
						else {
							mfmt = PFFLOATPYR;
							break;
						}
					}
					mfmt = PFFLOAT; break;
			case PFFLOAT:
			case PFFLOATPYR:mfmt = PFFLOATPYR; break;
			case PFDOUBLE:
			case PFCOMPLEX:	mfmt = PFDBLCOM; break;
			}
		}
		else {
			if (lev1 > lev2) {
				i = lev1;
				lev1 = lev2;
				lev2 = i;
				i = pfmt1;
				pfmt1 = pfmt2;
				pfmt2 = i;
			}
			mfmt = pfmt2;
			switch(pfmt2) {
			case PFSBYTE:
			case PFBYTE:	break;
			case PFUSHORT:	if (pfmt1 == PFSBYTE)
						mfmt = PFINT;
					break;
			case PFSHORT:	break;
			case PFUINT:	if (pfmt1 == PFSBYTE ||
					    pfmt1 == PFSHORT)
						mfmt = PFFLOAT;
					break;
			case PFINT:	break;
			case PFINTPYR:	break;
			case PFFLOAT:	if (pfmt1 == PFINTPYR)
						mfmt = PFFLOATPYR;
					break;
			case PFFLOATPYR:break;
			case PFDOUBLE:	break;
			case PFCOMPLEX:	break;
			case PFDBLCOM:	break;
			}
		}
	    }
	}
	sprintf(fnames,"%s and %s",fname1,fname2);
	return(pfind_closest(mfmt,typeslist,fnames));
}
