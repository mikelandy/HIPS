/*	POWERTOOL . C
#
%	Multi-Function Tool for Image Development
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
Bld. 50B, Rm. 2239, Lawrence Berkeley National Laboratory, Berkeley, CA, 94720
(wejohnston@lbl.gov)

For further information about this software, contact:
	Jin Guojun
	Bld. 50B, Rm. 2275
	Lawrence Berkeley National Laboratory, Berkeley, CA, 94720
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% AUTHOR:	Jin, Guojun - LBL	OCT. 1, 1990
*/

#include <math.h>
#include <errno.h>
#include "header.def"
#include "imagedef.h"
#include <sys/mman.h>

#ifndef	MaxWinWidth
# define	BufSize	2048L
#else
# define	BufSize	MaxWinWidth
#endif
#define	FILLER	10

char	TYPE_INFO[] = "input image format for pure images. Default is BYTE";
arg_fmt_list_string	arg_fmt[] =	{
	{"-TIFF", "%N", TiFF, 1, 0,
		"-image_type	TYPE in piping processes."},
/*	{"-[S][I][F[#]]", "%b", Flase, 1, 0, TYPE_INFO},	*/
	{"-F[#]", "%N %d", IFMT_FLOAT, 1, 0,
		"Float [force input # frames for a pure color image]"},
	{"-H", "%~ %d %d %d", 0, 4, 0,
		"\n\t[\"]row [column [frame]\"]	specify pure image input size",
		1},
	{"-I", "%N", IFMT_LONG, 1, 0, TYPE_INFO},
	{"-IL", "%N", IFMT_ILC, 1, 0, "input RGB (default SEPLANE)"},
	{"-S", "%N", IFMT_SHORT, 1, 0, (char*)TYPE_INFO},
	{"-P", "%b", False, 1, 1,
		"input not be Piped. It used for less memroy system"},
	{"-W", "%i", BufSize, 1, 1, "maximum width of output [%.0f]"},
	{"-a", "%b", True, 1, 0, "adjusting for factor not equal to 2^x\n\
		or generating HISTO data for gnuplot"},
	{"-f", "%d", True, 1, 0,
		"\tfilling the rest area when starting position is not 1"},
	{"-c", "%d", 1, 1, 1, "\tcolumn enlarge factor"},
	{"-r", "%d", 1, 1, 1, "\trow enlarge factor"},
	{"+", "%~ %d", 2, 2, 1, "\tenlarge factor for both directions"},
	{"-g", "%b", True, 1, 0, "gray scale only (default color)"},
	{"-o", "%s", NULL, 1, 1, "output file name"},
	{"-p", "%~ %d %d %d", 1, 4, 1,
		"[\"]row [column [frame]\"]	start position", 1},
	{"-s", "%# %d %d %d", 1, 4, 1,
		"[\"]row [column [frame]\"]	output window", 1},
	{"-slid", "%b", True, 1, 0, "message viewed in one line"},
#ifdef	_DEBUG_
	{"-d", "%b", True, 1, 0, "debug on"},
#endif
#ifdef	FITS_IMAGE
	{"-u", "%N", 'u', 1, 0, "unix FITS type"},
#endif
	{"-v", "%b", True, 1, 0, "verbose"},
	{"-w", "%b", False, 1, 1, "output pure image without header"},
	{"-x[x][X]", "%E %d %x %X", 0, 2, 1,
		"starting offset. skip first # [HEX] bytes.\n"},
	{"	[<] in_file_name [> [-o] out_file_name]\n\n\
 Notes:\n\
%	The regular mode is to allocate large memory for entire input data.\n\
%	The purpose is to speed up and able to pipe in for spreading	\n\
%	multi_frame images.\n\
%	\n\
%	The -P option -- file seek -- is a non piping input mode.	\n\
%	It requires no working memory. So it may be good for smaller	\n\
%	memory system or used in some special case.	\n\
%	\n\
%	The regular mode runs as twice fast as file seek mode for large	\n\
%	data file and about as four time fast as file seek mode for smaller\n\
%	data file in local file server.	\n\
%	If run on remote file server, for smaller data, pipe mode runs as\n\
%	twice fast as file seek mode. For large data, the difference is slight\n\
%	The file seek mode has 98% CPU utilization and memory mode has 49% CPU\n\
%	utilization. The memory mode is 17% - 30% faster than file mode.\n\
%	\n\
%	Input position is start from 1 rather than 0.	\n", "0", 0, 0, 0,
	"End of Notes"},	NULL	};
/*
%	When output columns > input columns, it must be integer times of input
%	columns.
%	This design is for cross network file server by a line writer. For local
%	file server, store output in a buffer and write to disk in once is fast.
*/

U_IMAGE	uimg;

#define	inbuf	uimg.src
#define	obuf	uimg.dest
#define	pform	uimg.in_form
#define	showin_info	message("input window = %d %d [%d %d]	",	\
				i_row, i_cln, i_row-bgn_row, i_cln-bgn_cln);

static	bool	gray_scale, graph, header=1, init_type=IMAGE_INIT_TYPE,
	forceF, filling, orw, pipe, relocat, szchange, sliding, verbose;
static	char	i_name[96]={NULL}, *topbd, cmd[128];
MType	Fpos, in_size, o_size, wlen, offs, wk_sz, pxlog_bytes,
	i_cln, i_row, frames=1,	/*	input file window size	*/
	o_cln, o_row, o_frm;	/* output file window size. init=0	*/
int	frm=1, bgn_cln=1, bgn_row=1,	/*	starting position	*/
	enlg_cln, enlg_row,		/*	enlarge factors		*/
	mulf_rowofs,	/*	mul_frame end of row offset	*/
	ratio, adj,	/*	deviation adjustment		*/
	bufsize;



main(argc, argv)
int	argc;
char*	argv[];
{
int	c, i, nf, job=0; /* 58H, 78H = 'X' ,'x';	7H : Ct, Sp, Pt	*/
char*	*fl, *of_name=0;
#ifdef	FITS_IMAGE
extern	int	FTy;
#endif

	Progname = *argv;
	p_use_pound_sign(True);

    if ((nf=parse_argus(&fl, argc, argv, arg_fmt, &init_type,
	&pform, &forceF,
	&header, &i_row, &i_cln, &frames,
	&pform, &pform, &pform,
	&pipe, &bufsize, &adj, &filling,
	&enlg_cln, &enlg_row,
	&enlg_cln, &enlg_row,	/* + */
	&gray_scale, &of_name,
	&relocat, &bgn_row, &bgn_cln, &frm,
	&szchange, &o_row, &o_cln, &o_frm,
	&sliding,
#ifdef	_DEBUG_
	&debug,
#endif
#ifdef	FITS_IMAGE
	&FTy,
#endif
	&verbose, &graph,
	&job, &offs, &offs, &offs)) < 0)	exit(-1);
    else	{
	if (of_name && !(out_fp=freopen(of_name, "wb", stdout)))
			syserr("open output file %s\n", of_name);

	if (nf)	strcpy(i_name, uimg.name = fl[0]);
    }
if (!strlen(i_name))
#ifdef	TC_Need
{	mesg("input original file name ");	gets(i_name);	}
#else
;else
#endif
if (!(in_fp=zreopen(i_name, NULL, NULL)))
	syserr("input file error\n");

uimg.color_dpy = -(!gray_scale);	/* for color HIPS	*/
format_init(&uimg, init_type, HIPS, -1, *argv, "S20-1");
io_test(fileno(in_fp), {parse_usage(arg_fmt); exit(0);});

fseek(in_fp, 0L, 0);	/*	if piped, errno = 29	*/
if (errno == ESPIPE)	pipe += ESPIPE;	/* someOS has 25	*/

if (header)	{	/* 'p' count spelane color frame as 3 frames	*/
	if ((*uimg.header_handle)(HEADER_READ, &uimg, 'p', 0, Yes) < 0)
		syserr("Unknown image type");
	frames= uimg.frames;
	i_cln = uimg.width;	i_row = uimg.height;
	if (pform==IFMT_HIST) {
	MType	v[4];
		fread(v, sizeof(v[0]), 2, in_fp);
		if (!adj)
		    fprintf(out_fp, "%s %4d %4d	%d %d %d\n", HIST_TITLE_STR,
			v[0], v[1], uimg.height, uimg.width, uimg.frames);
		for (c=0, o_row = v[1]>>2; c<frames; c++)	{
		    fread(&offs, sizeof(offs), 1, in_fp);
		    if (!adj)
			    fprintf(out_fp, "maxcnt %d\n", offs);
		    for (i=0; i<o_row; i++)	{
			register int	n = i<<2;
			fread(v, sizeof(v[0]), 4, in_fp);
			if (adj) {
			    fprintf(out_fp, "%d %d\n%d %d\n%d %d\n%d %d\n",
				n, v[0], n+1, v[1], n+2, v[2], n+3, v[3]);
			}
			else fprintf(out_fp, "%08x %9d %9d %9d %9d\n",
				n, v[0], v[1], v[2], v[3]);
		    }
		}
		exit(0);
	}
}
else	{	/*	uimg.frames = frames;	only for loading file */
	uimg.width = i_cln;
	uimg.height = i_row;
	if (pform==IFMT_BYTE || pform==IFMT_ILC)
		uimg.pxl_in = sizeof(char);
	else if (pform==IFMT_SHORT)
		uimg.pxl_in = sizeof(short);
	else	uimg.pxl_in = sizeof(float);
	uimg.channels = uimg.color_dpy ? 3 : 1;
	(*uimg.std_swif)(FI_INIT_NAME, &uimg, i_name, 0);
}
uimg.pxl_out = MIN(3, uimg.pxl_in);	/* for XWD	*/
pxlog_bytes = uimg.pxl_out>>1;
if (!gray_scale | !header)
	uimg.o_form = uimg.in_form,
	uimg.dpy_channels = MIN(3, uimg.channels);	/* for line 384 only */

if (enlg_cln < 1)
	if (enlg_row > 1)
		enlg_cln = enlg_row;
	else	enlg_cln = 1;
if (enlg_row < 1)	enlg_row = 1;

if (relocat > 0)	{
	showin_info;
	getvsize("resize area begins at [row column [frame]] ",
		&bgn_row, &bgn_cln, &frm);
}
if (szchange)	{
	if (!o_row)	{
 rd:	showin_info;
	getvsize("The new window size is [row column [frame(s)]] ",
		&o_row, &o_cln, &o_frm);
	}
	else if (!o_cln)	o_cln = i_row * o_cln / i_row;
	if (bgn_cln + o_cln > i_cln && o_cln % i_cln)	goto	rd;
}
else	o_row = i_row,	o_cln = i_cln;
if (frames == 1)	{
	if (enlg_cln == 1 && o_cln+bgn_cln > i_cln+1)	o_cln = i_cln-bgn_cln+1;
	if (enlg_row == 1 && o_row+bgn_row > i_row+1)	o_row = i_row-bgn_row+1;
}

sprintf(cmd,"%s: e%d*%d_%d_%d", i_name, enlg_row, enlg_cln, bgn_row,bgn_cln);
(*uimg.header_handle)(ADD_DESC, &uimg, cmd);

if (o_cln * enlg_cln > bufsize)	/* maximum buffer size	*/
	o_cln = bufsize / enlg_cln;

c = ratio = o_cln / i_cln;	/* c is boolean */
mulf_rowofs = i_row - o_row;
uimg.sub_img_h = o_row * enlg_row;
uimg.sub_img_w = o_cln * enlg_cln;
uimg.sub_img = True;

if (adj)	/* get deviation	*/
{	adj = uimg.width - o_cln * enlg_cln;
	msg("adjust margin = %d\n", adj);
}
if (frm<1 || frm>frames)	frm = 1;
frames -= frm-1;	/* pure size (not bytes)	*/
in_size = i_cln * (frames * i_row - (bgn_row-1)) - (bgn_cln-1);
wk_sz = o_row * o_cln;	/* pxl_in is pxlog_bytes, pxl_out is applied later */
if (o_cln < i_cln || o_row < i_row)	{
	if ((frames<<1) <= o_frm)	{
		i = i_row / o_row * (i_cln / o_cln) * frames;
		if (o_frm > i)	o_frm = i;
		job |= 2;	/* split */
	} else	job |= 4;	/* part	*/
}
/*	Set up output frames:
	when output columns > input columns & output frames < 1
	| output frames > total input pixel : total output pixel
*/
i = o_frm > in_size/wk_sz;
if (c && o_frm<1 || i)	{
	o_frm = in_size/wk_sz;
	o_frm += o_frm*wk_sz != in_size && i;
}
else if (o_frm<1 || o_frm>frames && !(job&2))	{
	o_frm = frames;
	if (o_row>i_row)
	    if	(filling || o_frm==1)
		orw = o_row - i_row;
	    else	o_frm *= (float)i_row/o_row;
}
if (forceF)	o_frm = forceF;
job |=  !(job&6) && (o_cln > i_cln || o_row > i_row ||
	bgn_row>1 || bgn_cln>1 || enlg_row>1 || enlg_cln>1);
uimg.load_all = uimg.frames;	/* save orig frames for loading file */
uimg.frames = o_frm;
if (job && uimg.in_color==CFM_SEPLANE)	o_frm *= uimg.dpy_channels;
wk_sz *= o_frm * uimg.pxl_out;
o_size = uimg.sub_img_h * uimg.sub_img_w * o_frm;

if (job & 1)	pw_read(in_size);
topbd = (char*)inbuf + (in_size << pxlog_bytes);

/* don't update again while HEADER_WRITE */
uimg.update_header = (*uimg.header_handle)(HEADER_TO, &uimg, Yes, No);
(*uimg.header_handle)(graph ? HEADER_WRITE : HEADER_FWRITE, &uimg,
	argc, argv, stderr);

#ifdef	_DEBUG_
if(verbose)
   message("INBUF = %u, r_e=%ld, c_e=%ld, pxlog_bytes=%ld\n",
	inbuf, i_row-bgn_row, i_cln-bgn_cln, pxlog_bytes);
#endif

/********************************
*	BEGIN Enlarge		*
* Set column & row deviation	*
********************************/

if (job & 1)	{
	obuf = zalloc(bufsize, 1<<pxlog_bytes, "obuf");
    if (pipe)
	if (pform==IFMT_SHORT)	i = pspowertool(inbuf, obuf, c, orw);
	else if (pform&IFMT_FLOAT)	i = pfpowertool(inbuf, obuf, c, orw);
	else	i = ppowertool(inbuf, obuf, c, orw);
    else
	if (pform==IFMT_SHORT)	i = spowertool(obuf, c, orw);
	else if (pform&IFMT_FLOAT)	i = fpowertool(obuf, c, orw);
	else	i = powertool(obuf, c, orw);
    message("\n%s Ok ! line [%d]\n", *argv, i);
}
else	{
char	*mmbuf;
int	mmaped=0, mmr, offm, nw; /* # of frames derived from a column	*/
	if (job & 4)
		nw = frames = 1;
	else	nw = i_cln / o_cln,	frames = i_row / o_row;
#define	nc	adj
#define	nr	ratio
#ifndef	SMEM_LIMIT
#define	SMEM_LIMIT	4194304	/* 4 MB	*/
#endif
#ifndef	PAGE_SIZE
#define	PAGE_SIZE	8192
#endif
#define	PAGE_MASK	~(PAGE_SIZE-1)
#define	PAGE_OFF_MASK	(PAGE_SIZE-1)

	errno = o_size = uimg.load_all = 0;
	if (job && uimg.o_form == IFMT_SEPLANE)	/* kludge job	*/
		o_frm *= uimg.dpy_channels,	uimg.dpy_channels = 1;
	bufsize = o_frm * o_row * o_cln << pxlog_bytes;	/* total output	*/
	in_size /= frm;
	c = (job & 2 ? in_size : o_row*o_cln) << pxlog_bytes;
	if (!header)	{
		pipe &=	/* use mmap if possible	*/
#ifdef	BSD44		/* cannot mmap /dev/sd?	*/
			~(i_cln > 1);	/* for truncat function	*/
#else
			~1;
#endif
	    if (!pipe && c > SMEM_LIMIT)
		if (!job)	{
			/* break large frames to more smaller ones.	*/
			o_frm = (wk_sz + SMEM_LIMIT - 1) / SMEM_LIMIT;
			c = SMEM_LIMIT;	uimg.dpy_channels = 1;
		} else	{
			c = o_row * i_cln;	/* entire strip	*/
			offm = 0;	mmr = frames;
			wk_sz *= o_frm;
		}
	}
	for (i=0; i<o_frm; i++)	{
		nf = c * uimg.dpy_channels;
		if (header || pipe)	{
			if (pw_read(i ? 0 : nf) < 1)	break;
		} else	{	/* save memory and little time	*/
		int	len = nf < wk_sz ? nf : wk_sz,
			offf = nf * i, offb;
			if (job)	{
				if (!mmr)	mmr = frames, offm += in_size;
				offf = offm;
				offf += c * (frames - mmr--);
			}
			offb = offf & PAGE_OFF_MASK;
			if (!(wk_sz -= len))	c = len / uimg.dpy_channels;
			if (mmaped && munmap(mmbuf, mmaped))
				prgmerr(0, "munmap %X, %x", mmbuf, mmaped);
			len += offb;
			mmbuf = mmap((caddr_t)0, len, PROT_READ, MAP_SHARED,
					fileno(in_fp), offf & PAGE_MASK);
			if (mmbuf == (caddr_t)-1)	break;
			inbuf = mmbuf + offb;
			mmaped = len;
		}
fi:		if (job & 6)	{	/* split or particle	*/
		char	*cbp=inbuf, *bp1;
		    for (nf=job&&mmaped?1:frames; nf--; cbp=bp1-(nw-1)*o_cln)
			for (nc=nw; nc--; cbp+=o_cln, i++)
			    for (nr=o_cln, bp1=cbp; nr--; bp1+=i_cln)
				o_size += fwrite(bp1, uimg.dpy_channels,
					o_cln, out_fp);
			i--;	/* adjust i for (i < o_frm)	*/
		} else	o_size += fwrite(inbuf, uimg.dpy_channels, c, out_fp);
		if (job == 'X')	offs = 0;
#ifdef	RLE_IMAGE
		if (uimg.in_type == RLE && forceF)	{
			if (rle_get_setup(&rle_dflt_hdr) != RLE_SUCCESS ||
			rle_dflt_hdr.xmax - rle_dflt_hdr.xmin + 1
			!= uimg.width || uimg.height !=
			rle_dflt_hdr.ymax - rle_dflt_hdr.ymin + 1)
			break;
		}
#endif
	}
	prgmerr(0, "%s [%d]", o_size==bufsize ? "convert" : "failure", o_size);
	if (mmaped)	munmap(inbuf, mmaped);
}
fclose(in_fp);	fclose(out_fp);
}

bseek(p, len, dir)	/* buffer seek	*/
char**	p;
MType	len;
{
switch (dir) {
	case 0:	*p = (char*)len;	break;
	case 1:	*p += len;		break;
	case 2:	*p = topbd + len;
}
return	(*p < (char*)inbuf || *p >= topbd);
}

WriteLn(r)
{
register int	j, l, wl;
for (j=l=0; j < enlg_row; j++)	{
	wl = fwrite(obuf, 1 << pxlog_bytes, enlg_cln * o_cln, out_fp);
	if (wl != (enlg_cln * o_cln))
		syserr("WriteLn %d", r);
	l += wl;
	if (adj)	l += fwrite(obuf, 1 << pxlog_bytes, adj, out_fp);
}
return	l;
}

fill(size, ch)
register char	ch;
{
register char*	p=obuf;
register int	i, l, s;
if (size < 1)	return	0;
	for (i=MIN(bufsize, size)<<pxlog_bytes; i;)
		p[--i] = ch;
	l = size / bufsize;
	s = size - bufsize * l;
	while (l--)
		i += fwrite(p, 1<<pxlog_bytes, bufsize, out_fp);
	i += fwrite(p, 1<<pxlog_bytes, s, out_fp);
	if (i != size)	syserr("filling %d [%d]", i, size);
	else if (verbose)
		message("filling %5d value %d	", i, ch);
return	i;
}

void	say(len, f)
{
	if (sliding)	mesg("\r");	else	mesg("\n");
	message("strip at frame %4d (%d frames [f%2d] => %d)	",
		frm, ratio, uimg.frames-f+1, len);
	frm += ratio;
}


/*=======================================
%	main procedure	POWERTOOL	%
=======================================*/
#define	main_pro_POWERTOOL(p_type, inp, shifts)	\
register int	c, r,	/*	control variables	*/	\
/*	start not at pos 1, 1;	x_err used as n - 1.	*/	\
	c_err = i_cln - bgn_cln,	\
	r_err = i_row - bgn_row;	\
\
for (; o_frm > 0; o_frm--)	{	\
    if (orw)				\
	wlen += fill((orw>>1)*o_cln*enlg_cln, filling);	\
    for (r=0; r < o_row-orw; r++)	{		\
	register p_type	*obp=olinebuf;			\
	Fpos = (MType)(int)inp;				\
	for (c=0; c < o_cln; c++, inp++)	{	\
	register int	j=enlg_cln;			\
	    while(j--)	/* duplicate (enlarge) column	*/	\
		*obp++ = *inp;				\
		/*	EOL in o_cln	*/		\
	    if (oc && !((c - c_err) % i_cln)){	/* for spread only */	\
		if (filling && bgn_cln>1) {		\
		register MType	i;	/* fill Big Column = Old Frame width */\
			for (i=bgn_cln; --i;)		\
			    for (c++, j=enlg_cln; j--;)	\
				*obp++ = filling;	\
			bseek(&inp, (bgn_cln-1) << shifts, 1);	\
		}					\
		if (bseek(&inp, i_cln * (i_row - 1) << shifts, 1)) {	\
/*	#ifdef	_DEBUG_	\
		    if (debug)	msg("inp=%d, r=%d, c=%d, ws=%ld, is=%ld\n", \
				inp, r+1, c+1, wlen, in_size);	\
	#endif	*/	\
		    if (!((r - ((r_err+1) & -!!filling)) % i_row) &&	\
			(filling || wlen >= in_size))	\
			goto	finish;			\
		    bseek(&inp, Fpos + (i_cln << shifts), 0);	\
		    wlen += WriteLn(r);			\
		    goto	nextrow;		\
		}	\
	    }		\
	}		\
	wlen += WriteLn(r);	\
	if (oc)			\
	    if ((r - r_err) % i_row)	/* not EOFrame	*/	\
		bseek (&inp, (-o_cln * i_row + i_cln) << shifts, 1);	\
	    else	{					\
		if (verbose)	say(wlen, o_frm);		\
		if (filling && bgn_row>1)	{		\
			wlen += fill((bgn_row-1)*o_cln*enlg_cln, filling); \
			r += bgn_row - 1;			\
			bseek(&inp, -r_err*i_cln << shifts, 1);		\
		}	\
		else	bseek(&inp, i_cln*(1-i_row) << shifts, 1);	\
	    }	\
	else	bseek(&inp, i_cln - o_cln << shifts, 1);		\
\
	nextrow:	continue;	\
   }	\
   if (mulf_rowofs>0)	bseek(&inp, mulf_rowofs*i_cln << shifts, 1);	\
}	\
finish:	fill(o_size-wlen, FILLER);	return	r+orw;


ppowertool(ibp, olinebuf, oc, orw)
char*	ibp, *olinebuf;
register int	oc;	/*	spread : o_cln > i_cln	*/
{
main_pro_POWERTOOL(char, ibp, 0);
}

pspowertool(isp, olinebuf, oc, orw)/*	for short	*/
short	*isp, *olinebuf;
register int	oc;
{
main_pro_POWERTOOL(short, isp, pxlog_bytes);
}

pfpowertool(ifp, olinebuf, oc, orw)/*	for all four bytes	*/
long	*ifp, *olinebuf;
register int	oc;
{
main_pro_POWERTOOL(long, ifp, pxlog_bytes);
}

	/*=======================================
	%	for non piped input mode	%
	%    with less main memory available	%
	=======================================*/
#define	non_pipe_POWERTOOL(p_type, func, shifts)	\
register int	c, j, r,	/*	control variables	*/	\
/*	start not at pos 0,0	*/	\
	c_err = i_cln - bgn_cln,	\
	r_err = i_row - bgn_row;	\
MType	NewFpos, FF = ftell(in_fp);	\
	\
for (; o_frm > 0; o_frm--)	{	\
    if (orw)				\
	wlen += fill((orw>>1)*o_cln*enlg_cln<<shifts, filling);	\
    for (r=0; r < o_row-orw; r++)	{	\
	Fpos = ftell(in_fp);			\
	for (c=0; c < o_cln; c++)	{	\
	p_type	*obp=olinebuf + c*enlg_cln;	\
		{	/* duplicate (enlarge) column	*/\
		p_type	b;	func;		\
		for (j=0; j < enlg_cln; j++)	\
			obp[j] = b;		\
		}	\
		/*	o_c > i_c and EOL, goto next frame same line	*/\
	    if (oc && !((c - c_err) % i_cln)){	\
		if (filling && bgn_cln>1){	\
		register MType	i;	/* fill Big Column = Old Frame width */	\
			for (i=0; i<bgn_cln-1; i++)	\
			    for (c++, obp+=enlg_cln, j=0; j<enlg_cln; j++) \
				obp[j] = filling;	\
			fseek(in_fp, i << shifts, 1);	\
		}	\
		if (fseek(in_fp, i_cln * (i_row - 1) << shifts, 1))	\
			syserr("fsk=%d, r=%d, c=%d\n", errno, r, c);	\
		NewFpos = ftell(in_fp);		\
		/* no more frame and also this is last frame last row */\
		if (NewFpos > FF + in_size){	\
		    if (!((r - ((r_err+1) & -!!filling)) % i_row) &&	\
			(filling || wlen >= in_size))			\
				goto	finish;				\
			fseek(in_fp, Fpos + (i_cln << shifts), 0);	\
			wlen += WriteLn(r);	\
			goto	nextrow;	\
		}	\
	    }		\
	}		\
	wlen += WriteLn(r);	/* duplicate (enlarge) row	*/	\
	if (oc)	/* o_c > i_c without enlarge	*/			\
	   if ((r - r_err) % i_row) /* not EOFrame, back n frames next row */\
		fseek(in_fp, ((-o_cln * i_row) + i_cln) << shifts, 1);	\
	   else	{	/* EOFrm, already at br-bc of next frame */	\
		if (verbose)	say(wlen, o_frm);	\
		if (filling && bgn_row>1){		\
			wlen += fill((bgn_row-1)*o_cln*enlg_cln, filling);\
			r += bgn_row - 1;		\
			fseek(in_fp, -r_err*i_cln << shifts, 1);	\
		}	\
		else	fseek(in_fp, i_cln*(1-i_row) << shifts, 1);	\
	    }	\
	else	/* in regular, goto next line (row) */		\
		fseek(in_fp, (i_cln - o_cln) << shifts, 1);	\
	nextrow:	continue;	\
    }	\
    if (mulf_rowofs>0)	fseek(in_fp, mulf_rowofs*i_cln<<shifts, 1);	\
}	\
finish:	fill(o_size-wlen, FILLER);	return	r+orw;


powertool(olinebuf, oc, orw)
char	*olinebuf;
register int	oc;	/*	o_cln > i_cln	*/
{
	non_pipe_POWERTOOL(register char, b=fgetc(in_fp) & 0xFF, 0);
}

spowertool(olinebuf, oc, orw)
register short*	olinebuf;
register int	oc;
{
	non_pipe_POWERTOOL(short, fread(&b, 1, sizeof(b), in_fp), 0);
}

fpowertool(olinebuf, oc, orw)
long	*olinebuf;
register int	oc;
{
	non_pipe_POWERTOOL(register long, b=getw(in_fp), pxlog_bytes);
}

/*	routine for reading window size when resize or scale error	*/
#ifndef	TC_Need
getvsize(s,a,b,c)
char*	s;
int*	a,*b,*c;
{
	mesg(s);	read(2, cmd, sizeof(cmd));
	return	sscanf(cmd,"%d %d %d",a,b,c);
}
#else
getvsize(char* s, ...)
{
	mesg(s);	read(2, cmd,sizeof(cmd));
	return	vsscanf(cmd, "%d %d %d", ...);
}
#endif

skip_read(sn, bs)
{
if (pipe == 1)	{
	while (sn > 0x7FFFFFFF)
		fseek(in_fp, 0x7FFFFFFF, SEEK_CUR),
		sn -= 0x7FFFFFFF;
	fseek(in_fp, sn, SEEK_CUR);
} else	{
register int	pbytes = 1<<pxlog_bytes;
	while (sn > bs)	/* fit in line buffer	*/
		sn -= upread(inbuf, pbytes, bs, in_fp);
	upread(inbuf, pbytes, sn, in_fp);
}
}

pw_read(b_size)
{
/*	pass first begining rows and N frames. See top for note	*/
static	int	pb_size;
int	s_where = (((frm-1) * i_row + (bgn_row-1)) * i_cln + (bgn_cln-1))
		* uimg.channels;
if (pipe) {
	if (b_size)	pb_size = b_size,
		inbuf = nzalloc(b_size*uimg.channels, 1<<pxlog_bytes, "inbuf");
	if (offs)	skip_read(offs, b_size);
	if (b_size=s_where)
		skip_read(b_size, pb_size);
return	(*uimg.std_swif)(FI_LOAD_FILE, &uimg, No, uimg.color_dpy);
}
else	{
	msg("%s in file mode\n", Progname);
	if (fseek(in_fp, offs + s_where, 1))
		syserr("passing 2 (%d)", b_size);
}
return	1;
}
