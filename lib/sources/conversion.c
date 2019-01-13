/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * conversion.c - find and effect the most suitable pixel format conversion
 *
 * Michael Landy - 1/5/91
 * added rgb/rgbz/zrgb/bgr/bgrz/zbgr conversions - msl - 5/20/93
 */

#include <stdio.h>
#include <hipl_format.h>

/*
 * clist's:  the list of types to which to convert from a given pixel format,
 * if necessary, in order of preference.
 *
 * Note that the 3-color formats (RGB/RGBZ/ZRGB/BGR/BGRZ/ZBGR) are listed as
 * the last choices because:
 *	* if numcolor is not 3 this conversion is illegal
 *	* if the caller doesn't grab num_frame from hdp, things fall apart
 *	  with such conversions, since conversions from 1-color to 3-color
 *	  cause a change in the number of frames.
 * The latter is also the reason that the 3-color types prefer to convert to
 * other 3-color types before converting to byte, etc.
 */

int clist_b[] = {PFSHORT,PFINT,PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,
			PFMSBF,PFLSBF,PFUSHORT,PFUINT,PFSBYTE,PFRGB,
			PFRGBZ,PFZRGB,PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
int clist_c[] = {PFDBLCOM,PFFLOAT,PFDOUBLE,PFINT,PFSHORT,PFBYTE,PFUINT,
			PFUSHORT,PFSBYTE,PFMSBF,PFLSBF,PFRGB,
			PFRGBZ,PFZRGB,PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
int clist_d[] = {PFFLOAT,PFDBLCOM,PFCOMPLEX,PFINT,PFSHORT,PFBYTE,PFUINT,
			PFUSHORT,PFSBYTE,PFMSBF,PFLSBF,PFRGB,
			PFRGBZ,PFZRGB,PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
int clist_dc[] = {PFCOMPLEX,PFDOUBLE,PFFLOAT,PFINT,PFSHORT,PFBYTE,PFUINT,
			PFUSHORT,PFSBYTE,PFMSBF,PFLSBF,PFRGB,
			PFRGBZ,PFZRGB,PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
int clist_f[] = {PFDOUBLE,PFCOMPLEX,PFDBLCOM,PFINT,PFSHORT,PFBYTE,PFUINT,
			PFUSHORT,PFSBYTE,PFMSBF,PFLSBF,PFRGB,
			PFRGBZ,PFZRGB,PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
int clist_i[] = {PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,PFSHORT,PFBYTE,PFUINT,
			PFUSHORT,PFSBYTE,PFMSBF,PFLSBF,PFRGB,
			PFRGBZ,PFZRGB,PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
int clist_lp[] = {PFBYTE,PFSHORT,PFINT,PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,
			PFUINT,PFUSHORT,PFSBYTE,PFMSBF,PFRGB,
			PFRGBZ,PFZRGB,PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
int clist_mp[] = {PFBYTE,PFSHORT,PFINT,PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,
			PFUINT,PFUSHORT,PFSBYTE,PFLSBF,PFRGB,
			PFRGBZ,PFZRGB,PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
int clist_s[] = {PFINT,PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,PFBYTE,PFUINT,
			PFUSHORT,PFSBYTE,PFMSBF,PFLSBF,PFRGB,
			PFRGBZ,PFZRGB,PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
int clist_sb[] = {PFSHORT,PFINT,PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,PFBYTE,
			PFUSHORT,PFUINT,PFMSBF,PFLSBF,PFRGB,
			PFRGBZ,PFZRGB,PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
int clist_ui[] = {PFINT,PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,PFSHORT,PFBYTE,
			PFUSHORT,PFSBYTE,PFMSBF,PFLSBF,PFRGB,
			PFRGBZ,PFZRGB,PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
int clist_us[] = {PFINT,PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,PFSHORT,PFBYTE,
			PFUINT,PFSBYTE,PFMSBF,PFLSBF,PFRGB,
			PFRGBZ,PFZRGB,PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
int clist_rgb[] = {PFRGBZ,PFZRGB,PFBGR,PFBGRZ,PFZBGR,PFBYTE,PFSHORT,PFINT,
			PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,
			PFMSBF,PFLSBF,PFUSHORT,PFUINT,PFSBYTE,LASTTYPE};
int clist_rgbz[] = {PFRGB,PFZRGB,PFBGR,PFBGRZ,PFZBGR,PFBYTE,PFSHORT,PFINT,
			PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,
			PFMSBF,PFLSBF,PFUSHORT,PFUINT,PFSBYTE,LASTTYPE};
int clist_zrgb[] = {PFRGB,PFRGBZ,PFBGR,PFBGRZ,PFZBGR,PFBYTE,PFSHORT,PFINT,
			PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,
			PFMSBF,PFLSBF,PFUSHORT,PFUINT,PFSBYTE,LASTTYPE};
int clist_bgr[] = {PFRGB,PFRGBZ,PFZRGB,PFBGRZ,PFZBGR,PFBYTE,PFSHORT,PFINT,
			PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,
			PFMSBF,PFLSBF,PFUSHORT,PFUINT,PFSBYTE,LASTTYPE};
int clist_bgrz[] = {PFRGB,PFRGBZ,PFZRGB,PFBGR,PFZBGR,PFBYTE,PFSHORT,PFINT,
			PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,
			PFMSBF,PFLSBF,PFUSHORT,PFUINT,PFSBYTE,LASTTYPE};
int clist_zbgr[] = {PFRGB,PFRGBZ,PFZRGB,PFBGR,PFBGRZ,PFBYTE,PFSHORT,PFINT,
			PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,
			PFMSBF,PFLSBF,PFUSHORT,PFUINT,PFSBYTE,LASTTYPE};

int find_closest(hd,typeslist)

struct header *hd;
int *typeslist;

{
	return(pfind_closest(hd->pixel_format,typeslist,"<stdin>"));
}

int ffind_closest(hd,typeslist,fname)

struct header *hd;
int *typeslist;
Filename fname;

{
	return(pfind_closest(hd->pixel_format,typeslist,fname));
}

/*
 * pfind_closest chooses the closest type to which to convert.  It may
 * decide to convert from a single pixel type (e.g. byte) to a 3-color
 * type (e.g. RGB), but it cannot check whether the input image has
 * numcolor equal to 3, so this is the caller's responsibility.
 */

int pfind_closest(pfmt,typeslist,fname)

int pfmt,*typeslist;
Filename fname;

{
	int *list;

	if (in_typeslist(pfmt,typeslist))
		return(pfmt);
	switch(pfmt) {
	case PFBYTE:	list = clist_b; break;
	case PFCOMPLEX:	list = clist_c; break;
	case PFDOUBLE:	list = clist_d; break;
	case PFDBLCOM:	list = clist_dc; break;
	case PFFLOAT:	list = clist_f; break;
	case PFINT:	list = clist_i; break;
	case PFLSBF:	list = clist_lp; break;
	case PFMSBF:	list = clist_mp; break;
	case PFSHORT:	list = clist_s; break;
	case PFSBYTE:	list = clist_sb; break;
	case PFUINT:	list = clist_ui; break;
	case PFUSHORT:	list = clist_us; break;
	case PFRGB:	list = clist_rgb; break;
	case PFRGBZ:	list = clist_rgbz; break;
	case PFZRGB:	list = clist_zrgb; break;
	case PFBGR:	list = clist_bgr; break;
	case PFBGRZ:	list = clist_bgrz; break;
	case PFZBGR:	list = clist_zbgr; break;
	default:	return(perr(HE_FMTSUBRFILE,"pfind_closest",
				hformatname(pfmt),fname));
	}
	while (*list != LASTTYPE) {
		if (in_typeslist(*list,typeslist))
			return(*list);
		list++;
	}
	return(perr(HE_FMTSUBRFILE,"pfind_closest",hformatname(pfmt),fname));
}

int in_typeslist(type,typeslist)

int type,*typeslist;

{
	register int *t;

	t = typeslist;
	while (*t != LASTTYPE) {
		if (type == *t++)
			return(TRUE);
	}
	return(FALSE);
}

/*
 * mlist's: the list of types from which each conversion routine is capable
 * of converting to a given pixel format.
 *
 * *** IMPORTANT NOTE ***
 *
 * This table (matrix) must be symmetric.  In other words, if it is possible
 * to convert from a to b, it must also be possible to convert from b to a.
 * The routine for conversion back to the input format (hconvertback) depends
 * on this fact.
 */

int mlist_b[] = {PFMSBF,PFLSBF,PFSHORT,PFINT,PFFLOAT,PFRGB,PFRGBZ,PFZRGB,
			PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
int mlist_c[] = {PFINT,PFFLOAT,PFDOUBLE,PFDBLCOM,LASTTYPE};
int mlist_d[] = {PFINT,PFFLOAT,PFCOMPLEX,PFDBLCOM,LASTTYPE};
int mlist_dc[] = {PFINT,PFFLOAT,PFDOUBLE,PFCOMPLEX,LASTTYPE};
int mlist_f[] = {PFBYTE,PFSHORT,PFINT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,LASTTYPE};
int mlist_i[] = {PFMSBF,PFLSBF,PFBYTE,PFSBYTE,PFUSHORT,PFSHORT,PFUINT,PFFLOAT,
			PFDOUBLE,PFCOMPLEX,PFDBLCOM,PFRGB,PFRGBZ,PFZRGB,
			PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
int mlist_lp[] = {PFBYTE,PFINT,LASTTYPE};
int mlist_mp[] = {PFBYTE,PFINT,LASTTYPE};
int mlist_s[] = {PFBYTE,PFSBYTE,PFINT,PFFLOAT,LASTTYPE};
int mlist_sb[] = {PFSHORT,PFINT,LASTTYPE};
int mlist_ui[] = {PFINT,LASTTYPE};
int mlist_us[] = {PFINT,LASTTYPE};
int mlist_rgb[] = {PFBYTE,PFINT,PFRGBZ,PFZRGB,PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
int mlist_rgbz[] = {PFBYTE,PFINT,PFRGB,PFZRGB,PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
int mlist_zrgb[] = {PFBYTE,PFINT,PFRGB,PFRGBZ,PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
int mlist_bgr[] = {PFBYTE,PFINT,PFRGB,PFRGBZ,PFZRGB,PFBGRZ,PFZBGR,LASTTYPE};
int mlist_bgrz[] = {PFBYTE,PFINT,PFRGB,PFRGBZ,PFZRGB,PFBGR,PFZBGR,LASTTYPE};
int mlist_zbgr[] = {PFBYTE,PFINT,PFRGB,PFRGBZ,PFZRGB,PFBGR,PFBGRZ,LASTTYPE};

/*
 * find_method - return the method identifier for conversion
 *
 * Note: because find_method returns a method identifier, or METH_IDENT,
 * or the negative of a method identifier (for conversion via PFINT), it is
 * essential that none of these possible values be identical to HIPS_ERROR so
 * that it also can give a normal hips error return.
 */

int find_method(typein,typeout)

int typein,typeout;

{
	return(ffind_method(typein,typeout,"<stdin>"));
}

int ffind_method(typein,typeout,fname)

int typein,typeout;
Filename fname;

{
	int *list,method;

	if (typein == typeout)
		return(METH_IDENT);
	switch(typeout) {
	case PFBYTE:	method = METH_BYTE; list = mlist_b; break;
	case PFCOMPLEX:	method = METH_COMPLEX; list = mlist_c; break;
	case PFDOUBLE:	method = METH_DOUBLE; list = mlist_d; break;
	case PFDBLCOM:	method = METH_DBLCOM; list = mlist_dc; break;
	case PFFLOAT:	method = METH_FLOAT; list = mlist_f; break;
	case PFINT:	method = METH_INT; list = mlist_i; break;
	case PFLSBF:	method = METH_LSBF; list = mlist_lp; break;
	case PFMSBF:	method = METH_MSBF; list = mlist_mp; break;
	case PFSHORT:	method = METH_SHORT; list = mlist_s; break;
	case PFSBYTE:	method = METH_SBYTE; list = mlist_sb; break;
	case PFUINT:	method = METH_UINT; list = mlist_ui; break;
	case PFUSHORT:	method = METH_USHORT; list = mlist_us; break;
	case PFRGB:	method = METH_RGB; list = mlist_rgb; break;
	case PFRGBZ:	method = METH_RGBZ; list = mlist_rgbz; break;
	case PFZRGB:	method = METH_ZRGB; list = mlist_zrgb; break;
	case PFBGR:	method = METH_BGR; list = mlist_bgr; break;
	case PFBGRZ:	method = METH_BGRZ; list = mlist_bgrz; break;
	case PFZBGR:	method = METH_ZBGR; list = mlist_zbgr; break;
	default:	return(perr(HE_FMTSUBRFILE,"find_method",
				hformatname(typeout),fname));
	}
	while (*list != LASTTYPE) {
		if (*list++ == typein)
			return(method);
	}
	return(-method);
}

int set_conversion(hd,hdp,typeslist)

struct header *hd,*hdp;
int *typeslist;

{
	return(fset_conversion(hd,hdp,typeslist,"<stdin>"));
}

int fset_conversion(hd,hdp,typeslist,fname)

struct header *hd,*hdp;
int *typeslist;
Filename fname;

{
	int ptype;

	if ((ptype = ffind_closest(hd,typeslist,fname)) == HIPS_ERROR)
		return(HIPS_ERROR);
	return (pset_conversion(hd,hdp,ptype,fname));
}

int pset_conversion(hd,hdp,ptype,fname)

struct header *hd,*hdp;
int ptype;
Filename fname;

{
	int method;
	h_boolean incol3,outcol3;

	if ((method = ffind_method(hd->pixel_format,ptype,fname)) == HIPS_ERROR)
		return(HIPS_ERROR);
	dup_header(hd,hdp);
	if (method == METH_IDENT)
		return(method);
	hd->imdealloc = TRUE;
	setformat(hdp,ptype);
	if (alloc_image(hdp) == HIPS_ERROR)
		return(HIPS_ERROR);
	incol3 = type_is_col3(hd);
	outcol3 = type_is_col3(hdp);
	if (incol3 && !outcol3) {
		if (hd->numcolor != 1)
			return(perr(HE_COL1,fname));
		hdp->numcolor = 3;
		hdp->num_frame *= 3;
	}
	else if ((!incol3) && outcol3) {
		if (hd->numcolor != 3)
			return(perr(HE_COL3,fname));
		hdp->numcolor = 1;
		hdp->num_frame /= 3;
	}
	if (method < 0)
		perr(HE_CONVI,hformatname_f(hd->pixel_format,ptype),
			hformatname_t(hd->pixel_format,ptype),fname);
	else
		perr(HE_CONV,hformatname_f(hd->pixel_format,ptype),
			hformatname_t(hd->pixel_format,ptype),fname);
	return(method);
}

static struct header convhdr;
static int convalloc = 0;

int hconvert(hd,hdp,method,fr)

struct header *hd,*hdp;
int method,fr;

{
	return(fhconvert(hd,hdp,method,fr,"<stdin>"));
}

int fhconvert(hd,hdp,method,fr,fname)

struct header *hd,*hdp;
int method,fr;
Filename fname;

{
	struct header *ihd;
	h_boolean incol3;

	if (method == METH_IDENT)
		return(HIPS_OK);
	incol3 = type_is_col3(hd);
	ihd = hd;
	if (method < 0) {
		if (convalloc) {
			if ((hd->orows != convhdr.orows) ||
			    (hd->ocols != convhdr.ocols)) {
				free_image(&convhdr);
				setsize(&convhdr,hd->orows,hd->ocols);
				if (alloc_image(&convhdr) == HIPS_ERROR)
					return(HIPS_ERROR);
			}
		}
		else {
			if (init_hdr_alloc(&convhdr,"","",1,"",hd->orows,
				hd->ocols,PFINT,1,"") == HIPS_ERROR)
					return(HIPS_ERROR);
			convalloc++;
		}
		if (incol3) {
			if (h_col3toi(hd,&convhdr,fr) == HIPS_ERROR)
				return(HIPS_ERROR);
			incol3 = FALSE;
		}
		else {
			if (h_toi(hd,&convhdr) == HIPS_ERROR)
				return(HIPS_ERROR);
		}
		method = -method;
		ihd = &convhdr;
	}
	switch(method) {
	case METH_BYTE:		if (incol3)
					return(h_col3tob(ihd,hdp,fr));
				else
					return(h_tob(ihd,hdp));
	case METH_COMPLEX:	return(h_toc(ihd,hdp));
	case METH_DOUBLE:	return(h_tod(ihd,hdp));
	case METH_DBLCOM:	return(h_todc(ihd,hdp));
	case METH_FLOAT:	return(h_tof(ihd,hdp));
	case METH_INT:		if (incol3)
					return(h_col3toi(ihd,hdp,fr));
				else
					return(h_toi(ihd,hdp));
	case METH_LSBF:		return(h_tolp(ihd,hdp));
	case METH_MSBF:		return(h_tomp(ihd,hdp));
	case METH_SHORT:	return(h_tos(ihd,hdp));
	case METH_SBYTE:	return(h_tosb(ihd,hdp));
	case METH_UINT:		return(h_toui(ihd,hdp));
	case METH_USHORT:	return(h_tous(ihd,hdp));
	case METH_RGB:		if (incol3)
					return(h_torgb(ihd,hdp));
				else
					return(h_col1torgb(ihd,hdp,fr));
	case METH_RGBZ:		if (incol3)
					return(h_torgbz(ihd,hdp));
				else
					return(h_col1torgbz(ihd,hdp,fr));
	case METH_ZRGB:		if (incol3)
					return(h_tozrgb(ihd,hdp));
				else
					return(h_col1tozrgb(ihd,hdp,fr));
	case METH_BGR:		if (incol3)
					return(h_tobgr(ihd,hdp));
				else
					return(h_col1tobgr(ihd,hdp,fr));
	case METH_BGRZ:		if (incol3)
					return(h_tobgrz(ihd,hdp));
				else
					return(h_col1tobgrz(ihd,hdp,fr));
	case METH_ZBGR:		if (incol3)
					return(h_tozbgr(ihd,hdp));
				else
					return(h_col1tozbgr(ihd,hdp,fr));
	default: 		return(perr(HE_METH,"hconvert",method,fname));
	}
}

int hconvertback(hd,hdp,method,fr)

struct header *hd,*hdp;
int method,fr;

{
	struct header *ihd;
	h_boolean incol3;

	if (method == METH_IDENT)
		return(HIPS_OK);
	incol3 = type_is_col3(hdp);
	ihd = hdp;
	if (method < 0) {
		if (convalloc) {
			if ((hd->orows != convhdr.orows) ||
			    (hd->ocols != convhdr.ocols)) {
				free_image(&convhdr);
				setsize(&convhdr,hd->orows,hd->ocols);
				if (alloc_image(&convhdr) == HIPS_ERROR)
					return(HIPS_ERROR);
			}
		}
		else {
			if (init_hdr_alloc(&convhdr,"","",1,"",hd->orows,
				hd->ocols,PFINT,1,"") == HIPS_ERROR)
					return(HIPS_ERROR);
			convalloc++;
		}
		if (incol3) {
			if (h_col3toi(hdp,&convhdr,fr) == HIPS_ERROR)
				return(HIPS_ERROR);
			incol3 = FALSE;
		}
		else {
			if (h_toi(hdp,&convhdr) == HIPS_ERROR)
				return(HIPS_ERROR);
		}
		ihd = &convhdr;
	}
	switch(hd->pixel_format) {
	case PFBYTE:	if (incol3)
				return(h_col3tob(ihd,hd,fr));
			else
				return(h_tob(ihd,hd));
	case PFCOMPLEX:	return(h_toc(ihd,hd));
	case PFDOUBLE:	return(h_tod(ihd,hd));
	case PFDBLCOM:	return(h_todc(ihd,hd));
	case PFFLOAT:	return(h_tof(ihd,hd));
	case PFINT:	if (incol3)
				return(h_col3toi(ihd,hd,fr));
			else
				return(h_toi(ihd,hd));
	case PFLSBF:	return(h_tolp(ihd,hd));
	case PFMSBF:	return(h_tomp(ihd,hd));
	case PFSHORT:	return(h_tos(ihd,hd));
	case PFSBYTE:	return(h_tosb(ihd,hd));
	case PFUINT:	return(h_toui(ihd,hd));
	case PFUSHORT:	return(h_tous(ihd,hd));
	case PFRGB:	if (incol3)
				return(h_torgb(ihd,hd));
			else
				return(h_col1torgb(ihd,hd,fr));
	case PFRGBZ:	if (incol3)
				return(h_torgbz(ihd,hd));
			else
				return(h_col1torgbz(ihd,hd,fr));
	case PFZRGB:	if (incol3)
				return(h_tozrgb(ihd,hd));
			else
				return(h_col1tozrgb(ihd,hd,fr));
	case PFBGR:	if (incol3)
				return(h_tobgr(ihd,hd));
			else
				return(h_col1tobgr(ihd,hd,fr));
	case PFBGRZ:	if (incol3)
				return(h_tobgrz(ihd,hd));
			else
				return(h_col1tobgrz(ihd,hd,fr));
	case PFZBGR:	if (incol3)
				return(h_tozbgr(ihd,hd));
			else
				return(h_col1tozbgr(ihd,hd,fr));
	default: return(perr(HE_FMTSUBR,"hconvertback",hd->pixel_format));
	}
}

void setupconvback(hd,hdp,hdcb)

struct header *hd,*hdp,*hdcb;

{
	dup_headern(hdp,hdcb);
	if (hd->pixel_format == hdp->pixel_format)
		hdcb->image = hdp->image;
	else {
		setformat(hdcb,hd->pixel_format);
		alloc_image(hdcb);
	}
	hdcb->numcolor = hd->numcolor;
	hdcb->num_frame = hd->num_frame;
}

int read_imagec(hd,hdp,method,fr)

struct header *hd,*hdp;
int method,fr;

{
	return(fread_imagec(stdin,hd,hdp,method,fr,"<stdin>"));
}

int fread_imagec(fp,hd,hdp,method,fr,fname)

FILE *fp;
struct header *hd,*hdp;
int method,fr;
Filename fname;

{
	h_boolean incol3,outcol3;

	if (method == METH_IDENT)
		return(fread_image(fp,hdp,fr,fname));
	incol3 = type_is_col3(hd);
	outcol3 = type_is_col3(hdp);
	if (incol3 == outcol3) {
		if (fread_image(fp,hd,fr,fname) == HIPS_ERROR)
			return(HIPS_ERROR);
		return(fhconvert(hd,hdp,method,fr,fname));
	}
	else if (incol3) {	/* convert from RGB/etc. to single pixel */
		if ((fr % 3) == 0) {
			if (fread_image(fp,hd,fr/3,fname) == HIPS_ERROR)
				return(HIPS_ERROR);
		}
		return(fhconvert(hd,hdp,method,fr,fname));
	}
	else {			/* convert from single pixel to RGB/etc. */
		if (fread_image(fp,hd,fr,fname) == HIPS_ERROR)	/* 1st color */
			return(HIPS_ERROR);
		if (fhconvert(hd,hdp,method,3*fr,fname) == HIPS_ERROR)
			return(HIPS_ERROR);
		if (fread_image(fp,hd,fr,fname) == HIPS_ERROR)	/* 2nd color */
			return(HIPS_ERROR);
		if (fhconvert(hd,hdp,method,3*fr+1,fname) == HIPS_ERROR)
			return(HIPS_ERROR);
		if (fread_image(fp,hd,fr,fname) == HIPS_ERROR)	/* 3rd color */
			return(HIPS_ERROR);
		return(fhconvert(hd,hdp,method,3*fr+2,fname));
	}
}

int write_imagec(hd,hdp,method,flag,fr)

struct header *hd,*hdp;
int method,fr;
h_boolean flag;

{
	return(fwrite_imagec(stdout,hd,hdp,method,flag,fr,"<stdout>"));
}

int fwrite_imagec(fp,hd,hdp,method,flag,fr,fname)

FILE *fp;
struct header *hd,*hdp;
int method,fr;
h_boolean flag;
Filename fname;

{
	h_boolean incol3,outcol3;

	if (flag) {
		incol3 = type_is_col3(hdp);
		outcol3 = type_is_col3(hd);
		if (incol3 == outcol3) {
			if (hconvertback(hd,hdp,method,fr) == HIPS_ERROR)
				return(HIPS_ERROR);
			return(fwrite_image(fp,hd,fr,fname));
		}
		else if (incol3) {	/* convert from RGB/etc. back to
						single pixel */
			if (hconvertback(hd,hdp,method,3*fr) == HIPS_ERROR)
				return(HIPS_ERROR);	/* first color */
			if (fwrite_image(fp,hd,3*fr,fname) == HIPS_ERROR)
				return(HIPS_ERROR);
			if (hconvertback(hd,hdp,method,3*fr+1) == HIPS_ERROR)
				return(HIPS_ERROR);	/* second color */
			if (fwrite_image(fp,hd,3*fr+1,fname) == HIPS_ERROR)
				return(HIPS_ERROR);
			if (hconvertback(hd,hdp,method,3*fr+2) == HIPS_ERROR)
				return(HIPS_ERROR);	/* third color */
			return(fwrite_image(fp,hd,3*fr+2,fname));
		}
		else {			/* convert from single pixel back to
						RGB/etc. */
			if (hconvertback(hd,hdp,method,fr) == HIPS_ERROR)
				return(HIPS_ERROR);
			if ((fr % 3) == 2)
				return(fwrite_image(fp,hd,fr,fname));
			return(HIPS_OK);
		}
	}
	return(fwrite_image(fp,hdp,fr,fname));
}
