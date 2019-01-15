/*	FITS_IO . C
%
%	Byte based I/O utilities.
%	It also contains host checking, long_32 (LongSwap) and Short Swap.
%	There are two file open system:
%		One is for regular file open.
%	The other one, which is for debugging, must use standard I/O.
%
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
	Bld. 50B, Rm. 2275, Lawrence Berkeley Laboratory, Berkeley, CA, 94720
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% AUTHOR:	Guojun Jin - LBL	8/1/90
*/

#include "header.def"
#include "imagedef.h"

Sonion	sonion;
int	hostype,
	flag,
	FORTRAN,	/* Fortran file flag	*/
	FTy = 'v',	/* Fortran file type	*/
	FFCL;		/* Fortran Ctrl word length:	UNIX = 4, PC = 1. */


short	ShortSwap(iu)
short	iu;
{
char	tmp;
/* if (hostype != 2 && hostype != 5)	return; */
sonion.ilen = iu;
tmp = sonion.ichar[0];
sonion.ichar[0] = sonion.ichar[1];
sonion.ichar[1] = tmp;
return	sonion.ilen;
}

long_32 LongSwap(inval)  /*	swap 4 byte integer	*/
long_32 inval;
{
SwapUnion	onion;
char	temp;

/*	byte swap the input field	*/
	onion.llen = inval;
	temp = onion.ichar[0];
	onion.ichar[0]=onion.ichar[3];
	onion.ichar[3]=temp;
	temp = onion.ichar[1];
	onion.ichar[1] = onion.ichar[2];
	onion.ichar[2] = temp;
return (onion.llen);
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%	subroutine check_host - find out what kind of machine	%
%	we are on. This subroutine checks the attributes of	%
%	the host computer and returns a host code number.	%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

check_host()
{
int	swap, host=0, bits;
char	hostname[80];

strcpy(hostname, "Host ");

bits = sizeof(bits) << 3;

if (bits == 8)
	strcat(hostname, "0 - 8-bit machine.");
else	{
	sonion.ichar[0] = 1;
	sonion.ichar[1] = 0;
	swap = (sonion.ilen != 1);

    if (bits==16)	{
	host = 1 << swap;		/* 1 = IBM PC host  */
	sprintf(hostname + strlen(hostname),
		"%d - 16 bit integers with%s swapping, no var len support.",
		host, host & 1 ? "out" : " ");
    } else if (bits==32)	{
	if (!swap) {
		host = 3;	/* VAX host with var length support */
		strcat(hostname, "3,4 - 32 bit integers without swapping.");
	} else	{
		host = 5;	/* OTHER 32-bit host  */
	}
    } else	{	/* 64 or more	 */
		host = 6 + swap;
    }
    if (host > 4)
	sprintf(hostname + strlen(hostname),
		"%d - %d-bit integers with%s swapping, no var len support.",
		host, 32 << (host > 5), host & 1 ? " " : "out");
}
message("%s : %s\n", Progname, hostname);
return	host;
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*	subroutine get_files - get input filenames and open.		*
*	If No input file name and No msp given, stdin will be used.	*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

FILE* get_infile(hostype, inname, msp)
int	hostype;
char	*inname, *msp;
{
char	YN[4];
short	shortint;
FILE*	in_file;

if (!inname[0] || inname[0] == '.')
{
	if (strlen(msp))
	{	message("host = %d\n\n%s:	", hostype, msp);
		gets(inname);
	}
	else	return	stdin;
}

if (hostype == 1 || hostype == 2)
	in_file = fopen(inname, "rb");

else if (hostype == 3 || hostype == 5)
	in_file = fopen(inname, "r");
if (in_file == NULL)
	syserr("\ncan't open input file: %s\n", inname);

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
* If we are on a vax see if the file is in var length format.  *
* This logic is in here in case the vax file has been stored   *
* in fixed or undefined format.  This might be necessary since *
* vax variable length files can't be moved to other computer   *
* systems with standard comm programs (kermit, for example).   *
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
if (hostype==3){
	fread(&shortint, 2, 1, in_file);
	if (shortint > 0 && shortint < 80){
		hostype = 4;	/* change host to 4	*/
		strcpy(YN, "not");
	}
	else	YN[0] = 0;	/* not extra message	*/
	msg("This is %s a VAX variable length file.\n", YN);
	fseek(in_file, 0, SEEK_SET);	/* reposition to beginning of file */
}
return	in_file;
}

FILE*	get_outfile(hostype, outname, msp)
int	hostype;
char	*outname, *msp;
{
FILE*	out_file;

if (!outname[0] || outname[0] == '.')
{
	msg("\n%s:	", msp);
	gets(outname);
}
if (hostype==1 || hostype==2 || hostype==5)
	out_file = fopen(outname, "wb");
else	out_file = fopen(outname, "w");
if (!out_file)
	syserr("\ncan't open output file: %s\n", outname);
return	out_file;
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
* subroutine read_var - read variable length records from input file	*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

long_32	read_var(ibuf, ifp, hostype, FTy)
byte	*ibuf;
FILE	*ifp;
int	hostype;
bool	FTy;
{
long_32	resultu, ulen, resultr;
short	vlen, result;
byte	pclen;

switch (hostype){
	case 1: /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		*	IBM PC host,	no swap needed		*
		%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

	case 3: /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		*	VAX host with variable length support	*
		%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
	switch (FTy)
	{
	    case 'u':
		result = fread(&ulen, 1, sizeof(ulen), ifp);
		ulen = LongSwap(ulen);
		if (ulen > MaxUNIXFortranBlock)	return	-1;
		break;
	    case 'v':
		result = fread(&vlen, 1, sizeof(vlen), ifp);
		fread(&flag, 1, sizeof(flag), ifp);
		ulen = (long_32)(vlen - 2);
		if (ulen > MaxVAXFortranBlock)	return	-1;
		break;
	    case 'p':
		result = fread(&pclen, 1, sizeof(pclen), ifp);
		ulen = (long_32)pclen;
		if (ulen > MaxPCFortranBlock)	return	-1;
		break;
	    default:	return	-1;
	}
	if (ulen < 0)	return	-1;
	resultr = fread(ibuf, 1, ulen, ifp);
	switch (FTy)
	{
	    case 'u':
		fread(&resultu, 1, sizeof(resultu), ifp);
		resultu = LongSwap(resultu);
		if (resultu != resultr)	return	-1;
		break;
	    case 'p':
		fread(&pclen, 1, sizeof(pclen), ifp);
		if (pclen != (byte)resultr)
			return	-1;
	}
	return (resultr);

	case 2: /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		*	Macintosh host		*
		%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
	switch (FTy)
	{
	    case 'u':
		result = fread(&ulen, 1, sizeof(ulen), ifp);
		if (ulen > MaxUNIXFortranBlock)	return	-1;
		break;
	    case 'v':
		result = fread(&vlen, 1, sizeof(vlen), ifp);
		fread(&flag, 1, sizeof(flag), ifp);
		ulen = LongSwap((long_32)(vlen - 2));
		if (ulen > MaxVAXFortranBlock)	return	-1;
		break;
	    case 'p':
		result = fread(&pclen, 1, sizeof(pclen), ifp);
		ulen = (long_32)pclen;
		if (ulen > MaxPCFortranBlock)	return	-1;
		break;
	    default:	return	-1;
	}
	if (ulen < 0)	return	-1;
	resultr = fread(ibuf, 1, ulen, ifp);
	switch (FTy)
	{
	    case 'u':
		fread(&resultu, 1, sizeof(resultu), ifp);
		if (resultu != resultr)
			return	-1;
		break;
	    case 'p':
		fread(&pclen, 1, sizeof(pclen), ifp);
		if (pclen != (byte)resultr)
			return	-1;
	}

/*msg("vlen=%04x, result=%d, get = %d\n", nlen, result, vlen);*/
	return (resultr);
	/*	byte swap the vlen field			*/
/*	nlen = LongSwap(nlen);	 left out of earlier versions	*/

	case 4: /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		*	VAX host, but not a variable vlen file	*
		%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

	switch (FTy)
	{
	    case 'u':
		resultu = fread(&ulen, 1, sizeof(ulen), ifp);
		ulen = LongSwap(ulen);
		if (ulen > MaxUNIXFortranBlock)	return	-1;
		break;
	    case 'v':
		result = fread(&vlen, 1, sizeof(vlen), ifp);
		fread(&flag, 1, sizeof(flag), ifp);
		ulen = (long_32)(vlen - 2);
		if (ulen > MaxVAXFortranBlock)	return	-1;
		break;
	    case 'p':
		result = fread(&pclen, 1, sizeof(pclen), ifp);
		ulen = (long_32)pclen;
		if (ulen > MaxPCFortranBlock)	return	-1;
		break;
	    default:	return	-1;
	}

	if (ulen < 0 || ulen > MaxVAXFortranBlock)
		return	-1;
	resultr = fread(ibuf, 1, ulen, ifp);
	/* check to see if we crossed a vax record boundary	  */
	while (ulen > vlen)
	resultr += fread(ibuf + vlen, ulen - vlen, 1, ifp);
	switch (FTy)
	{
	    case 'u':
		fread(&resultu, 1, sizeof(resultu), ifp);
		resultu = LongSwap(resultu);
		if (resultu != resultr)
			return	-1;
		break;
	    case 'p':
		fread(&pclen, 1, sizeof(pclen), ifp);
		if (pclen != (byte)resultr)
			return	-1;
	}
	return (resultr);

	case 5: /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		* Unix workstation host (non-byte-swapped 32 bit host)	*
		%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
	switch (FTy)
	{
	    case 'u':
		result = fread(&ulen, 1, sizeof(ulen), ifp);
		if (ulen > MaxUNIXFortranBlock)	return	-1;
		break;
	    case 'v':
		result = fread(&vlen, 1, sizeof(vlen), ifp);
		fread(&flag, 1, sizeof(flag), ifp);
		ulen = LongSwap((long_32)(vlen - 2));
		if (ulen > MaxVAXFortranBlock)	return	-1;
		break;
	    case 'p':
		result = fread(&pclen, 1, sizeof(pclen), ifp);
		ulen = (long_32)pclen;
		if (ulen > MaxPCFortranBlock)	return	-1;
		break;
	    default:	return	-1;
	}
	if (ulen < 0)	return	-1;
	resultr = fread(ibuf, 1, ulen, ifp);
	switch (FTy)
	{
	    case 'u':
		fread(&resultu, 1, sizeof(resultu), ifp);
		if (resultu != resultr)
			return	-1;
		break;
	    case 'p':
		fread(&pclen, 1, sizeof(pclen), ifp);
		if (pclen != (byte)resultr)
			return	-1;
	}

/*msg("vlen=%04x, result=%d, get = %d\n", nlen, result, vlen);*/
	return (resultr);
	}
}

/*	write data to a file. if successful, return 0	*/
update(buf, psize, length, ofp)
VType	*buf;
MType	length;
FILE	*ofp;
{
MType	comfirm = fwrite(buf, psize, length, ofp);

if (comfirm != length)
	message("update[%ld] %ld\n", length, comfirm);
return	length - comfirm;
}

bool	comfirm_host(img, hostype, FTy)
U_IMAGE	*img;
bool	FTy;
{
char	buf[10240];

	if (FTy == 'p')				/* a IBM/PC Version	*/
	   if (getc(img->IN_FP) != 'K')
		goto	findout;		/* not a PC Version	*/
	if (read_var(buf, img->IN_FP, hostype, FTy) > 0)
	{	fseek(img->IN_FP, 0, SEEK_SET);
		if (FTy == 'p')			/* if it's a PC Version	*/
			getc(img->IN_FP);	/*	pass 'K' sign	*/
		FORTRAN = True;
	}
	else
findout:while(FTy)			/* find out what version is	*/
	{
	    fseek(img->IN_FP, 0, SEEK_SET);	/* back  to begining	*/
	    if (getc(img->IN_FP) == 'K') {
		if (read_var(buf, img->IN_FP, hostype, 'p') > 0)
		{	mesg("PC Version\n");
			FTy = 'p';
			fseek(img->IN_FP, 1, SEEK_SET);
			FORTRAN = True;
			break;
		}
	    }
	    fseek(img->IN_FP, 0, SEEK_SET);
	    if (read_var(buf, img->IN_FP, hostype, 'u') > 0){
		mesg("UNIX Version\n");
		FTy = 'u';
		fseek(img->IN_FP, 0, SEEK_SET);
		FORTRAN = True;
		break;
	    }
	    fseek(img->IN_FP, 0, SEEK_SET);
	    if (read_var(buf, img->IN_FP, hostype, 'v') > 0) {
		mesg("VMS Version\n");
		FTy = 'v';
		fseek(img->IN_FP, 0, SEEK_SET);
		FORTRAN = True;
		break;
	    }
	    if (FORTRAN)
		mesg("Warning: Unknow Version\n"),	FTy = NULL;
	    break;
	}	/* end while */
	if (FORTRAN)
	switch(FTy)
	{
	case 'u':	FFCL = 4;	break;
	case 'p':	FFCL = 1;	break;
	case False:	FORTRAN = FTy;
		perror("File type error: change to nonFORTRAN mode\n");
	}
	fseek(img->IN_FP, 0, SEEK_SET);
return	FTy;
}
