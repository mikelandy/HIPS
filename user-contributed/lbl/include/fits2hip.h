/*	FITS2HIPS . H
#
%	global variables
*/

#include "imagedef.h"
#include "fits_def.h"

#define	Naux	fhd.naux
#define	Scale	fhd.scale
#define	Bscale	fhd.bscale
#define	Bzero	fhd.BZero
#define	Seeing	fhd.seeing
#define	Crval1	fhd.crval1

extern  U_IMAGE img;
#define	iPix_form	img.in_form	/* Short integer	*/
#define	Pix_bytes	img.pxl_in
#define	Row	img.height
#define	Coln	img.width
#define	Frm	img.frames
#define infile  img.IN_FP
#define outfile img.OUT_FP

extern	char	*fh_buf,
		DataType[32], Date[16], Form[80],
		inname[80], outname[80];
extern	int	ByteShift,	/* default input format	(short)	*/
		oPBSize,	/* output Pix_Byte_Size */
		flag;
extern	MType	F_Offset,	/* File descriptor offset	*/
		BUF_P;
extern	float	MinShort, MaxShort;
extern	FITSType	ZLevel;

extern	long_32	read_var();
extern	short	ShortSwap();
extern	long_32	LongSwap();
extern	int	check_host();
