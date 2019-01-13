/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * hformatname - return the ascii name of a pixel format
 */

#include <hipl_format.h>

extern struct h_types h_typenames[];
extern struct h_convs h_ctors[];
extern struct h_convs h_dctors[];
extern struct h_convs h_rtocs[];
extern struct h_convs h_rtodcs[];

char *hformatname(pfmt)

int pfmt;

{
	int i;

	i = 0;
	while (h_typenames[i].h_pfmt != pfmt) {
		if (h_typenames[i].h_pfmt == -1)
			break;
		i++;
	}
	return(h_typenames[i].h_fmtname);
}

char *hformatname_f(pfmtf,pfmtt)

int pfmtf,pfmtt;

{
	int i;

	if (pfmtf == PFCOMPLEX) {
		if (pfmtt == PFDBLCOM)
			return(hformatname(pfmtf));
		else {
			i = 0;
			while (h_ctors[i].h_cnvtype != hips_cplxtor) {
				if (h_ctors[i].h_cnvtype == -1)
					break;
				i++;
			}
			return(h_ctors[i].h_cnvname);
		}
	}
	else if (pfmtf == PFDBLCOM) {
		if (pfmtt == PFCOMPLEX)
			return(hformatname(pfmtf));
		else {
			i = 0;
			while (h_dctors[i].h_cnvtype != hips_cplxtor) {
				if (h_dctors[i].h_cnvtype == -1)
					break;
				i++;
			}
			return(h_dctors[i].h_cnvname);
		}
	}
	else
		return(hformatname(pfmtf));
}

char *hformatname_t(pfmtf,pfmtt)

int pfmtf,pfmtt;

{
	int i;

	if (pfmtt == PFCOMPLEX) {
		if (pfmtf == PFDBLCOM)
			return(hformatname(pfmtt));
		else {
			i = 0;
			while (h_rtocs[i].h_cnvtype != hips_rtocplx) {
				if (h_rtocs[i].h_cnvtype == -1)
					break;
				i++;
			}
			return(h_rtocs[i].h_cnvname);
		}
	}
	else if (pfmtt == PFDBLCOM) {
		if (pfmtf == PFCOMPLEX)
			return(hformatname(pfmtt));
		else {
			i = 0;
			while (h_rtodcs[i].h_cnvtype != hips_rtocplx) {
				if (h_rtodcs[i].h_cnvtype == -1)
					break;
				i++;
			}
			return(h_rtodcs[i].h_cnvname);
		}
	}
	else
		return(hformatname(pfmtt));
}
