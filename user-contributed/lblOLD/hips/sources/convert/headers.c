/*	HEADERS . C
#
% Display image headers for HIPS, FIST, GIF, PNM, RLE, SUN-RASter, and TIFF
%
%	Copyright (c)	1991 - 1995	Jin, Guojun
%
%	Permission to use, copy, modify, and distribute this software and
% its documentation for non-commercial purpose is hereby granted; provided that
% the above copyright notice appear in all copies and that both the copyright
% notice and this permission notice appear in supporting documentation.
%
% compile:	cc -O -o headers headers.c -lscs5 -lccs -lhips -lrle -ltiff ...
%
% AUTHOR:	Jin Guojun - LBL	12/10/91
*/

#include <string.h>
#include "header.def"
#include "imagedef.h"
#include <math.h>

#ifndef	SBUF_LEN
#define	SBUF_LEN	256
#endif

U_IMAGE	uimg;
char	usage[]="headers  [ < ]  image_files ... ",
	*color_or_grayscale(),	/* returns	color format */
	*format_string();	/* returns	format desciption */


main(argc, argv)
int	argc;
char*	argv[];
{
int	f=0;

format_init(&uimg, IMAGE_INIT_TYPE, COLOR_PS, -1, *argv, "Aug1-5");

if (argc==1) {	/* for single file using `<' redirction input */
	f--;
	io_test(fileno(in_fp), usage_n_options(usage, 0, "input?"));
}
while (++f < argc) {
    if (argc != 1 && (in_fp=zreopen(uimg.name=argv[f], NULL, NULL)) == NULL) {
	prgmerr(0, "can not open %s for input", argv[f]);
	continue;
    }
    uimg.in_type = IMAGE_INIT_TYPE;	/* important for multi-handling	*/
    if ((*uimg.header_handle)(HEADER_READ, &uimg, 0,0) < 0) {
	msg("%s is not in type of HIPS, FITS, GIF, ICC, PNM, RLE, SUN, and TIFF\n",
		argv[f]);
	continue;
    }
    printf("\nNAME =>	%s\n", argv[f]);
    printf("Type :	%s\n", ITypeName[uimg.in_type]);
    printf("color :	%s\n", color_or_grayscale(uimg.in_color));
    printf("format :	%s\n", uimg.in_type==XWD ? (char*)get_xwd_type(0) :
			format_string(uimg.in_form));
    printf("frames :	%d\n", uimg.frames ? uimg.frames : 1);
    printf("size   :	%d(w) x %d(h)\n", uimg.width, uimg.height);
    printf("sub-image :	%d(w) x %d(h) <%d:%d>\n",
	uimg.sub_img_w, uimg.sub_img_h, uimg.sub_img_x, uimg.sub_img_y);
    fprintf(out_fp, "pixel bytes :	%d\n", uimg.pxl_in);
    if (print_string(out_fp, "History", uimg.history))
	CFREEnNULL(uimg.history);
    if (print_string(stdout, "Desciption", uimg.desc))
	CFREEnNULL(uimg.desc);
}
exit(0);
}

char*	color_or_grayscale(type)
{
	switch (type) {
	case CFM_SGF:
		return	"GrayScale";
	case CFM_SCF:
		sprintf(usage, "8-bit Color with Color_Map [%d]", uimg.cmaplen);
		return	usage;
	case CFM_ILL:
		return	"24-bit Color, scan lined";
	case CFM_ILC:
		return	"24-bit Color, interleaved";
	case CFM_BITMAP:
		return	"Bit-Map";
	case CFM_ALPHA:
		return	"32-bit Color (alpha channel)";
	case CFM_SEPLANE:
		return	"24-bit Color, separate planes";
	default:	return	"Unknown color format";
	}
}


#ifdef	HIPS_PUB
char	*ifmt_str[]={"BYTE", "SHORT", "LONG", "FLOAT", "COMPLEX", "ASCII",
		"DOUBLE", "long DB"};
char*	hformatname(form)
{
	if (form < 8)	return	ifmt_str[form];
	sprintf(usage, "%d", form);	return	usage;
}
#endif

char*	format_string(form)
{
	switch (form) {
	case IFMT_VFFT3D:	return	"3D VFFT";
	case IFMT_VFFT2D:	return	"2D VFFT";
	case IFMT_DVFFT3D:	return	"3D Double VFFT";
	case IFMT_DVFFT2D:	return	"2D Double VFFT";
	case IFMT_VVFFT3D:	return	"3D VFFT in separated plane";
	case IFMT_DVVFFT3D:	return	"3D Double VFFT in separated plane";
	case IFMT_SCF:	return	"1 byte color";
	case IFMT_SEPLANE:
	case IFMT_ILC:
	case IFMT_ILL:	return	"3 byte color";
	case IFMT_ALPHA:return	"4 byte color";
	case IFMT_BITMAP:	return	"bitmap";
	default	:	return	hformatname(form);
	}
}

print_string(fp, phd, s)
FILE	*fp;
char	*phd, *s;
{
char	buffer[SBUF_LEN];	/* 256 is save for most case */
register int	i;
    if (s) {
	fprintf(fp, "%s\n", phd);
	do {
		for (i=0; *s && *s != '\n' && i<SBUF_LEN-1; i++)
			if (*s == '|' && *(s+1) == '\\')
			{	s+=2;	i--;	}
			else	buffer[i] = *s++;
		if (!*s)
			buffer[i++] = '\n';
		buffer[i]=0;
		i ^= i;
		while(buffer[i] == ' ')	i++;
		if (strlen(buffer+i))
			fprintf(fp, "%s\n", buffer+i);
	} while(*++s);
	return	1;
    } else	fprintf(fp, "No %s\n", phd);
return	0;
}
