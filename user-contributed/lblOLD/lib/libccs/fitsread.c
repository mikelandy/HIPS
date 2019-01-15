/*	FITSREAD . C
#
@	read FITS images
@
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

This software is copyright (C) by the Lawrence Berkeley Laboratory.
Permission is granted to reproduce this software for non-commercial
purposes provided that this notice is left intact.

It is acknowledged that the U.S. Government has rights to this software
under Contract DE-AC03-765F00098 between the U.S.  Department of Energy
and the University of California.

This software is provided as a professional and academic contribution
for joint exchange. Thus, it is experimental, and is provided ``as is'',
with no warranties of any kind whatsoever, no support, no promise of
updates, or printed documentation. By using this software, you
acknowledge that the Lawrence Berkeley Laboratory and Regents of the
University of California shall have no liability with respect to the
infringement of other copyrights by any part of this software.

For further information about this notice, contact William Johnston,
Bld. 50B, Rm. 2239, Lawrence Berkeley Laboratory, Berkeley, CA, 94720.
(wejohnston@lbl.gov)

For further information about this software, contact:
	Jin Guojun
	Bld. 50B, Rm. 2275, Lawrence Berkeley Laboratory, Berkeley, CA, 94720.
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
@
@ AUTHOR:	Jin Guojun - LBL	9/1/91
*/

#include "header.def"
#include "fits2hip.h"
#include <math.h>

#ifndef	BufSize
#define	BufSize	4096
#endif	114030 = 1BD6E for unix

/************************************************************************
*	uncompress is based on the previously difference algorithm.	*
*	If input is not equal to -128, which means the difference	*
*	is less than 255 (maximum byte value).	So, we just simple	*
*	add current  value with pre byte value	to get current value.	*
*	Otherwise, goto Auxiliary buffer to pick the long_32 value.	*
*	J is Auxiliary buffer pointer. I is input buffer pointer.	*
*	After decompression, if J is not equal to Naux, then there	*
*	must be some thing wrong. So, please check J value.		*
*	Warning:							*
*		This uncompress will not properly work on IBM/PC for	*
*	large file. Since it must store all date blocks in main memory.	*
************************************************************************/

char	fits_uncompress(img, FTy)
U_IMAGE	*img;
bool	FTy;
{
VType	*unc_ibuf;
float	*unc_cnvt=NULL;
long_32	prev, next, j=0, length, fsize=img->height*img->width,
	NP=fsize*img->frames;
register FITSType*	aux = ZALLOC(Naux, 2, No);
register char*	p;
register float*	fp;
register MType	i;
#ifdef	_DEBUG_
FITSType	maxval=MaxShort, minval = MinShort;
#endif

p = unc_ibuf = NZALLOC(img->frames, fsize, "unc_in");
if (img->pxl_out != sizeof(*unc_cnvt))
	unc_cnvt = NZALLOC(NP, sizeof(*unc_cnvt), "unc_cnvt");
else	unc_cnvt = img->src;

	if (FORTRAN)	{
	    for (length=0; length<NP;)	/* reading difference data */
		length += read_var(p+length, img->IN_FP, hostype, FTy);

	    for (length=0; length < Naux<<1L;)	/* reading auxiliary data */
		length += read_var(aux + length, img->IN_FP, hostype, FTy);
	}
	else	{
		message("attention: Suppose not to be a FORTRAN File\n");
		(*img->i_read)(p, img->frames, fsize, img->IN_FP);
		(*img->i_read)(aux, Naux, 2, img->IN_FP);
	}

if (((hostype==2 || hostype==5) && FTy != 'u') || (hostype != 5 && FTy == 'u'))
    for (i=0; i < Naux; i++)
	*(aux + i) = ShortSwap(*(aux + i));
else	msg("No swaping, FTy = %c\n", FTy);

	prev = *aux;
	DEBUGMESSAGE("First Aux is %d,	first value = %d\n", prev, prev + *p);

	for (i=NP, fp=unc_cnvt; i--; p++){
	    if (*p != -128)
		next = prev + *p;
	    else{
		if	(j >= Naux)
			return	NULL;
		next = aux[++j];
	    }
	    *fp++ = prev = next;

#ifdef	_DEBUG_
	    if (maxval < prev)	maxval = prev;
	    if (minval > prev)	minval = prev;
#endif
	}
	CFREE(aux);
	CFREE(unc_ibuf);
#ifdef	_DEBUG_
	msg("DINF: Min_value = %4ld,	Max_value = %ld\n", minval, maxval);
#endif

	if (Scale && Scale != 1)
	    for (i=0, fp=unc_cnvt; i < NP; i++, fp++)
		*fp = *fp * Scale + Bzero;
	fp = unc_cnvt;
	switch(img->o_form){
	case IFMT_BYTE:
		for (i=0, p=img->src; i < NP; i++, p++, fp++){
			*fp -= ZLevel;
			if (*fp < 256)
				*(byte*)p = *fp;
			else	*p = 255;
		}
		break;
	case IFMT_SHORT:
		for (i=0, aux=img->src; i < NP; i++)
			*aux++ = *fp++ - ZLevel;
		break;
	case IFMT_LONG:
		for (i=0; i < NP; i++)
		((long_32*)img->src)[i] = *fp++ - ZLevel;
	}
	if (img->pxl_out != sizeof(*unc_cnvt))
		CFREE(unc_cnvt);
	if (j != Naux - 1)
		msg("diff Naux(%d) = %d, F %d\n", Naux, j, feof(img->IN_FP));
return	(Naux - 1 - j);
}


/************************************************************************
*	This procedure only swap and convert data from fits format to	*
*	regular data format. You may need to use mainpeak to get	*
*	proper picture.	(called by transf_data)				*
************************************************************************/

float	fits_convertdata(img, ibuf, length, obuf)
U_IMAGE	*img;
long_32	*ibuf, length;
register VType	*obuf;
{
float	*buf=NZALLOC((MType)sizeof(float), (MType)BufSize, "fits_cnvt");
register FITSType*	sp;
register float	*fp=buf, mean=0;
register MType	i;
register float	coef = Bzero - Seeing * ZLevel, Fscale = Bscale;

if (((hostype==2 || hostype==5) && FTy != 'u') ||	/* u & v => t	*/
	(hostype != 2 && hostype != 5 && FTy == 'u'))
	switch(img->pxl_in)	{
	case 2:
		for (i=0, sp=PtrCAST ibuf; i < length; i++)	{
			*sp = ShortSwap(*sp);
			fp[i] = *sp++ * Fscale + coef;
		}
	break;
	case 4:
		for (i=0; i < length; i++)
		    fp[i] = (*(ibuf + i)=LongSwap(ibuf[i])) * Fscale + coef;
	}
else	switch(img->pxl_in) {
	case 2:	for (i=0, sp=PtrCAST ibuf; i < length; i++)
			fp[i] = *sp++ * Fscale + coef;
		break;
	case 4:	for (i=0; i < length; i++)
			fp[i] = *(ibuf + i) * Fscale + coef;
	}

switch (img->o_form)	{
case IFMT_BYTE:
	for (i=0; i < length; mean += fp[i++])
		if (fp[i] > 255)
			*((char*)obuf + i) = 255;
		else if (fp[i] < 0)
			*((char*)obuf + i) = 0;
		else	*((char*)obuf + i) = fp[i];
	break;
case IFMT_LONG:
	for (i=0; i < length; mean += fp[i++])
		((long_32*)obuf)[i] = fp[i];
	break;
case IFMT_FLOAT:
		for (i=0; i < length; mean += fp[i++]);
		memcpy(obuf, fp, length*sizeof(*fp));
	break;
case IFMT_DOUBLE:
	for (i=0; i < length; mean += *fp++, i++)
		((double*)obuf)[i] = *fp;
	break;
case IFMT_SHORT:
default:
	for (i=0, sp=obuf; i < length; mean += *fp++, i++, sp++)
		if (*fp > MaxShort)
			*sp = MaxShort;
		else if (*fp < MinShort)
			*sp = MinShort;
		else	*sp = *fp;
}
CFREE(buf);
return	mean;
}

/************************************************************************
*	There is no decompression. It only transform fits image data	*
*	to hips image data. It can be used on variant machines.		*
************************************************************************/
fits_transf_data(img, FTy, wflag)
U_IMAGE	*img;
bool	FTy, wflag;
{
MType	block, length, pixels,
	isize=img->pxl_in*img->frames*img->height*img->width;
float	sumean=0;
VType	*t_ibuf = NZALLOC(img->pxl_in, BufSize, "transf_in"),
	*t_cnvt = NZALLOC(img->pxl_out, BufSize, "transf_cnvt");

#ifdef	_DEBUG_
msg("SEEing = %f, CrVal1 = %f\n", Seeing, Crval1);
#endif

if (FORTRAN)
    for (length=block=0; length < isize;)	{
	pixels = read_var(t_ibuf, img->IN_FP, hostype, FTy);
	if (pixels < 0 || feof(img->IN_FP)) {
		mesg("Warning!	may have errors\n");
		break;
	}
	length += pixels;
	pixels >>= ByteShift;		/* get pixel number	*/
	sumean += fits_convertdata(img, t_ibuf, pixels, t_cnvt);
	if (wflag)	(*img->i_write)(t_cnvt, oPBSize, pixels, img->OUT_FP);
	else	memcpy((char*)img->src+length, t_cnvt, pixels<<ByteShift);
    }
else {	mesg("Attention: Suppose not to be a FORTRAN File\n");

	for (length=0, pixels=BufSize; length < isize && pixels==BufSize;
		length += pixels<<ByteShift)
	{
		pixels = (*img->i_read)(t_ibuf, 1<<ByteShift, BufSize, img->IN_FP);
		sumean += fits_convertdata(img, t_ibuf, pixels, t_cnvt);
		if (wflag)
			(*img->i_write)(t_cnvt, oPBSize, pixels, img->OUT_FP);
		else	memcpy((char*)img->src+img->pxl_out*(length>>ByteShift),
				t_cnvt, pixels*img->pxl_out);
	}
}
msg("Mean Value = %lf\n", sumean/isize);
CFREE(t_ibuf);
CFREE(t_cnvt);
return	(int) length - (block << (FTy != 'v')) * FFCL - isize;
}


read_fits_image(img, Fty, dcmp, wflag)
U_IMAGE	*img;
bool	Fty, dcmp, wflag;	/* write data to a file */
{
int	errcode;

if (dcmp)	{	/* decompress */
	verify_buffer_size(&img->src, img->frames * img->pxl_out,
			img->height * img->width, "unc_src");
	if (errcode=fits_uncompress(img, Fty))
		msg("%d error(s) in decompressing\n", errcode);
	else	mesg("end of uncompressing\n");
	if (wflag)
		update(img->src, img->pxl_out, (MType)img->height * img->width,
			img->OUT_FP);
}
else	{
	if (!wflag)
		verify_buffer_size(&img->src, img->frames * img->pxl_out,
			img->height * img->width, "transf_src");
	errcode = fits_transf_data(img, Fty, wflag);
	msg("error(%d)	end of FITS transfer\n", errcode);
}
return	errcode;
}
