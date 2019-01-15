/*	FITSHD . C
#
%	FITS header handlers
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

This software is copyright (C) by the Lawrence Berkeley National Laboratory.
Permission is granted to reproduce this software for non-commercial
purposes provided that this notice is left intact.

It is acknowledged that the U.S. Government has rights to this software
under Contract DE-AC03-76SF00098 between the U.S.  Department of Energy
and the University of California.

This software is provided as a professional and academic contribution
for joint exchange. Thus, it is experimental, and is provided ``as is'',
with no warranties of any kind whatsoever, no support, no promise of
updates, or printed documentation. By using this software, you
acknowledge that the Lawrence Berkeley Laboratory and Regents of the
University of California shall have no liability with respect to the
infringement of other copyrights by any part of this software.

For further information about this notice, contact William Johnston,
Bld. 50B, Rm. 2239, Lawrence Berkeley National Laboratory, Berkeley, CA, 94720.
(wejohnston@lbl.gov)

For further information about this software, contact:
	Jin Guojun
	Bld. 50B, Rm. 2275
	Lawrence Berkeley National Laboratory, Berkeley, CA, 94720.
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% AUTHOR:	Jin Guojun - LBL	8/1/90
*/

#include "header.def"
#include "imagedef.h"
#include <math.h>

#define	write_line(str, fp)	fwrite(str, sizeof(char), 80, fp);

/*	global variables	*/

char	*fh_buf,
	DataType[32], Date[16], Form[80];
int	ByteShift=1,	/* default input format	(short)	*/
	oPBSize;	/* output Pix_Byte_Size */
bool	DCMP=False;
MType	F_Offset=0,	/* File descriptor offset	*/
	BUF_P=0;
float	MinShort=0, MaxShort=65535;
FITSType	ZLevel=0;


convts*
GetItem(char	opt, char*	buf)
{
register char*	i;
static	convts	cnvt;

i = (char *) index(buf, '=');
while (*(++i) == ' ');
switch(opt)
{	case 'i':	cnvt.h = atoi(i);
		break;
	case 'e':
	case 'f':	cnvt.f = atof(i);
		break;
	case 'c':
		i = (char *) index(i, '/');
		while (*--i == 0x20);
		while (*--i != 0x20);
		cnvt.c = *++i;
		break;
	case 's':
		if (*i == '\'')	i++;
		cnvt.s = i;
		while (i < buf+80 && *i != '\'')
			i++;
		*i = 0;
}
return	&cnvt;	/* not a good ideal but work with most machines */
}

/****************************************************************
% subroutine get_fits_head - get FITS header from input file	%
% The whole header is keeped in fh_buf. When get_fits_head is	%
% called, this fh_buf will be free and the information is lost.	%
****************************************************************/
int
get_fits_head(FITS_BASE	*fhd, U_IMAGE	*img, int host, bool	FTy)
{
int	reloads=0, MASize=FITSBlock << (FTy=='u' && FORTRAN);
short	blocks, blocksize;
char	c;
bool	ReLoad = True, EndOFile;
register char*	ibuf;

if (BUF_P)
	CFREE(fh_buf),	BUF_P = DCMP = 0;
else
	fhd->date = Date,
	fhd->datatype = DataType,
	fhd->format = Form;

do {
   if (ReLoad)	{	/* load header buffer	*/
	if (!BUF_P)
		fh_buf = (char *) NZALLOC(MASize, 1, "FITS_H");
	else	fh_buf = (char *) realloc(fh_buf, MASize * (reloads+1));
	ibuf = fh_buf + BUF_P;
	if (FORTRAN)
	{
		blocksize = blocks = 0;
	    do	{
		blocksize += read_var(fh_buf+BUF_P+blocksize,img->IN_FP,host,FTy);
		blocks++;
#ifdef	BSD44
		if (blocksize <= 0 && blocks > 7)	return	EOF;
#endif
		}	while (blocksize < FITSBlock);
		BUF_P += blocksize;
		F_Offset += blocksize + blocks * FFCL;
		if (FTy != 'v')
			F_Offset += blocks * FFCL;
	}
	else	F_Offset = BUF_P += blocksize =
			fread(fh_buf + BUF_P, 1, FITSBlock, img->IN_FP);
	ReLoad = False;
	EndOFile = feof(img->IN_FP);
   }

	/* start heading processing	*/
   if (strncmp(ibuf, "SIMPLE", 6) == 0)
	fhd->sample = GetItem('c', ibuf)->c;	/* maybe at different position	*/

   else	if (strncmp(ibuf, "BITPIX", 6) == 0)
	{
		img->pxl_in =  (fhd->bits_pxl = GetItem('i', ibuf)->h) >> 3;
		img->in_form = ByteShift = img->pxl_in >> 1; /* get sequence # */
	}
   else	if (strncmp(ibuf, "DATATYPE", 8) == 0)
	{	strncpy(DataType, GetItem('s', ibuf)->s, 15);
		c = 7;
		if (strncmp(DataType, "INTEGER", c)==0)
		{
			while(!isdigit(DataType[c]))	c++;
			switch(DataType[c])	/* use in type as out type */
			{
			case '1':
				img->pxl_out = sizeof(char);
				break;
			case '2':
				img->pxl_out = sizeof(short);
				break;
			case '4':
				img->pxl_out = sizeof(long);
				break;
wdtf:			default:msg("undetermined format %d\n", DataType[c]);
				if (img->pxl_in == 4)	goto	ffmt;
			}
		}
		else	if(strncmp(DataType, "FLOAT", 5)==0)
ffmt:			img->pxl_out = sizeof(float);
		else	goto	wdtf;
		if (img->pxl_in != img->pxl_out)
			msg("warning: unmatching data type\n");
	}
   else	if (strncmp(ibuf, "NAXIS ", 6) == 0)
	fhd->naxis = GetItem('i', ibuf)->h;

   else	if (strncmp(ibuf, "NAXIS1 ", 7) ==  0)
	img->width = fhd->naxis1 = GetItem('i', ibuf)->h;

   else	if (strncmp(ibuf, "NAXIS2 ", 7) == 0)
	img->height = fhd->naxis2 = GetItem('i', ibuf)->h;

   else	if (strncmp(ibuf, "NAXIS3 ", 7) == 0)
	img->frames = fhd->naxis3 = GetItem('i', ibuf)->h;

   else	if (strncmp(ibuf, "FORMAT", 6) == 0)
	{	strcpy(Form, GetItem('s', ibuf)->s);
		if (!strncmp(Form, "COMPRESSED", 10) || toupper(Form[0])=='C'){
			DCMP = True;	/* no convert, goto decompress	*/
			if (!img->pxl_in)
				img->pxl_in = sizeof(float),
				img->in_form = IFMT_FLOAT;
		}
	}
   else	if (strncmp(ibuf, "NAUX ", 5) == 0)
	fhd->naux = GetItem('i', ibuf)->h;

   else	if (strncmp(ibuf, "DATE-", 5) == 0)
	strncpy(Date, GetItem('s', ibuf)->s, 15);

   else	if (strncmp(ibuf, "SCALE", 5) == 0)
	fhd->scale = GetItem('e', ibuf)->f;

   else	if (strncmp(ibuf, "BSCALE", 6) == 0)
	fhd->bscale = GetItem('e', ibuf)->f;

   else	if (strncmp(ibuf, "BZERO", 5) == 0)
	fhd->BZero = GetItem('e', ibuf)->f;

   else	if (strncmp(ibuf, "CRVAL1", 6) == 0)
	fhd->crval1 = GetItem('e', ibuf)->f;

   else	if (strncmp(ibuf, "CRVAL2", 6) == 0)
	fhd->crval2 = GetItem('e', ibuf)->f;

   else	if (strncmp(ibuf, "SEEING", 6) == 0)
	fhd->seeing = GetItem('e', ibuf)->f;
   else	/* not a FITS image */
	if (!(isalpha(*ibuf) | isspace(*ibuf)))	return	-1;
/*
   else	{
	for (c=0; c < 80; c++)
		putc(*(ibuf+c), stderr);
	mesg("\n");
	}
*/
   ibuf += 80;
   c = strncmp(ibuf, "END", 3);
   if (ibuf >= fh_buf + blocksize + reloads*MASize){
	ReLoad = True;
	reloads++;
	}
} while(c && !EndOFile);

if (!img->frames)	img->frames = 1;

if (F_Offset)	/*	point to data	*/
	fseek(img->IN_FP, F_Offset, SEEK_SET);

msg("end header (%xH)\n", F_Offset);

return	!F_Offset || EndOFile;
}



/*	return	number of lines write to img->OUT_FP	*/

put_fits_head(FITS_BASE	*fhd, U_IMAGE	*img, bool	complete)
{
int	i = 0;
char	line[81];

sprintf(line, "SIMPLE  =                    %c                                                  ",
	fhd->sample);
write_line(line, img->OUT_FP);	++i;
sprintf(line, "BITPIX  =           %10d                                                  ",
	fhd->bits_pxl);
write_line(line, img->OUT_FP);	++i;
sprintf(line, "NAXIS   =           %10d                                                  ",
	fhd->naxis);
write_line(line, img->OUT_FP);	++i;
sprintf(line, "NAXIS1  =           %10d                                                  ",
	fhd->naxis1);
write_line(line, img->OUT_FP);	++i;
sprintf(line, "NAXIS2  =           %10d                                                  ",
	fhd->naxis2);
write_line(line, img->OUT_FP);	++i;
sprintf(line, "NAUX    =           %10d                                                  ",
	fhd->naux);
write_line(line, img->OUT_FP);	++i;
sprintf(line, "DATAMIN =           %10d                                                  ", 0);
write_line(line, img->OUT_FP);	++i;
sprintf(line, "DATAMAX =           %10d                                                  ",
	fhd->maxval);
write_line(line, img->OUT_FP);	++i;
sprintf(line, "BZERO   =           %10f                                                  ",
	fhd->BZero);
write_line(line, img->OUT_FP);	++i;
sprintf(line, "BSCALE  =           %10f                                                  ",
	fhd->bscale?fhd->bscale:1.0);
write_line(line, img->OUT_FP);	++i;
sprintf(line, "SCALE   =           %10f                                                  ",
	fhd->scale?fhd->scale:1.0);
write_line(line, img->OUT_FP);	++i;
sprintf(line, "HISTORY Created by fitstopgm.                                                   ");
write_line(line, img->OUT_FP);	++i;
if (complete){
	sprintf(line, "END                                                                             ");
	write_line(line, img->OUT_FP);	++i;
	sprintf(line, "                                                                                ");
	while (i < 36)
		write_line(line, img->OUT_FP);	++i;
}
return	i;
}

#ifdef	FITS_WRITE_LINE
write_line(char	*str, FILE	*fp)
{
	if (fwrite(str, sizeof(char), 80, fp) != 80)
		syserr("write line");
}
#endif
