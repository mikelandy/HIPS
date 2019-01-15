/*	TABLE_IF . C
#
%	Copyright (c)	Jin Guojun -	All rights reserved
%
% AUTHOR:	Jin Guojun - LBL	04/01/93
*/

#include <stdlib.h>
#include <string.h>
#include "header.def"
#include "imagedef.h"

typedef	struct	{
	MType	key,	/* table index, or magic #	*/
		dtype;	/* input format, or type	*/
	int	dlen,
		val;	/* data value, or marks	shorter than 3 bytes	*/
	char*	symbol;	/* key string, or longer marks	*/
	} *TC;	/* table content	*/

typedef	struct	{	/* need to match QSCell	*/
	MType	pos;
	TC	tc;
	} ccstable;

static char*	table_key[] = {	"header", "width", "height", "frames", "color",
	"depth", "encode", "format", "magic", "mark", "separator"	};
static const int t_keys = sizeof(table_key) / sizeof(table_key[0]);
ccstable	ct[12];	/* first entry is for header/magic only	*/

#define	GETC(fp)	(*img->r_seek)(fp, 0, SEEK_GETB)
#define	Feof(fp)	(*img->eof)(fp)

#define	DEFAULT_CCSTABLES	t_keys + 1
#define	TABLE_STOP	"#STOP"
#define	iscstr(s)	!strncmp(cp, s, strlen(s))
#define	isccstr(s)	!strncmp(cp, s, sizeof(s)-1)
#define	isbcstr(s)	!strncmp(fetchbuf, s, strlen(s))

#define	Key_Header	0
#define	Key_Width	1
#define	Key_Height	2
#define	Key_Frames	3
#define	Key_Colormap	4
#define	Key_Depth	5
#define	Key_Encode	6
#define	Key_Format	7
#define	Key_Magic	8
#define	Key_Mark	9
#define	Key_Separator	10
#define	Mark_EOH	Key_Mark
#define	Mark_EOI	11	/* enf of item	*/
#define	Key_Compress	Key_Encode	/* second parameter of encoder	*/

/*	TABLE file formats	*/
#define	ADDR_HEADER	0	/* default	*/
#define	LINE_HEADER	1	/* item is terminated by LF	*/
#define	SKIP_HEADER	2	/* skip to end of header	*/

def_cmap(U_IMAGE *img)
{
}

build_ccs_table(register ccstable *ct, register int nt)
{
if (nt < 4)	nt = DEFAULT_CCSTABLES;
if (verify_buffer_size(&ct->tc, sizeof(*ct->tc), nt, No))	{
register int	i = nt;
	while (--i)	ct[i].tc = ct->tc + i;
}
if (!ct->tc)	return	prgmerr(-1, "No core for TC");
return	nt;
}

void
free_ccs_table(register ccstable *ct)
{
register int	i = pointer_buffer_size(ct->tc) / sizeof(*ct->tc);
	while (i--)	if (ct[i].tc->symbol)	CFREE(ct[i].tc->symbol);
	CFREE(ct->tc);	ct->tc = NULL;
}

static	void
setup_colormap(TC tc, U_IMAGE *img)
{
register char*	cp = (char*)&tc->dtype;

	if (isccstr("GIF"))
		ReadRGBMap(img, GifScreen.ColorMap, tc->dlen, No);
	else if (isccstr("RAS"))
		read_ras_cmap(img, tc->dlen, 3);
	else if (isccstr("HEX"))
		read_hex_rgbmap(img, GifScreen.ColorMap, tc->dlen);
	else if (isccstr("DEF"))
		def_cmap(img);
	else prgmerr(0, "UNKNOWN color map format: %s", cp);
	img->cmaplen = tc->dlen;
}

static
sscan_format(register char* cp, register int i)
{
    if (i == Key_Format)	{
	i = IFMT_SGF;
	if (isccstr("SCF"))	i = IFMT_SCF;
	else if (isccstr("ILC"))	i = IFMT_ILC;
	else if (isccstr("ILL"))	i = IFMT_ILL;
	else if (isccstr("ALPHA"))	i = IFMT_ALPHA;
	else if (isccstr("SEPLANE"))	i = IFMT_SEPLANE;
	else if (isccstr("BITMAP"))	i = IFMT_BITMAP;
    }	else	{	/* type / encoding	*/
	char	*sp = index(cp, Space);
	if (sp)	 *sp = 0;
	else if (cp[i = strlen(cp) - 1] == LF)	cp[i] = 0;
	i = available_type(cp);
	if (sp)	*sp = Space;
    }
return	i;
}

static
ctcompare(register ccstable *ct1, register ccstable *ct2)
{
	return	ct1->pos > ct2->pos;
}

static
findmark(register ccstable* ct, register int n, register int key)
{
	while (--n)	if (ct->tc->key == key)
		return	ct->tc->val;
return	n;
}

static
setup_img(U_IMAGE *img, int	n)
{
register int	i = n;
int	fw=1, fh=1, fd=1, compress=0, encoder=RAS;
	img->frames = img->pxl_in = 1;

	while (--i)	{
	register int	val = ct[i].tc->val;
		switch (ct[i].tc->key)	{
		case Key_Width:
			img->width = val;	fw = False;	break;
		case Key_Height:
			img->height = val;	fh = False;	break;
		case Key_Frames:
			img->frames = val;	break;
		case Key_Depth:
			img->channels = val >> 3;	fd = False;	break;
/*		case Key_Colormap:	*/
		case Key_Encode:	{
		register char*	cp = ct[i].tc->symbol;
			encoder = val;
			if (cp)		{
			    val = strlen(cp) - 1;
			    if (cp[val] == LF)	cp[val] = 0;
			    if (isccstr("RT_STD"))
				compress = RT_STANDARD;
			    else if (isccstr("RT_BENC"))
				compress = RT_BYTE_ENCODED;
			}
		}	break;
		case Key_Format:
			img->in_form = val;
			img->in_color = imageform_to_colorform(val);
		default:	break;
		}
	}
	img->in_type = i = encoder;
	i ^= img->o_type;
	img->o_type ^= i;
	(*img->header_handle)(HEADER_TO, img, compress, 0);
	img->o_type ^= i;
free_ccs_table(ct);
return	-(fw | fh | fd);
}

ccs_table_if(int job, U_IMAGE*	img /* ... */)
{
char	fetchbuf[256], *table_list;
FILE	*name_f, *table_f;
int	magic, n, i;
MType	mask=check_host()==5 ? 0xFFFFFF00 : 0xFFFFFF;
#define	ep	table_list

    if (!(table_list=getenv("CCSTABLE_LIST")) ||
	!(name_f=fopen(table_list, "r")))	return	EOF;
    do	{
	register TC	tc;
ntry:	if (!fgets(fetchbuf, sizeof(fetchbuf), name_f))	continue;
	if (isbcstr(TABLE_STOP))	break;
	if (fetchbuf[i=strlen(fetchbuf)-1] == LF)
		fetchbuf[i] = 0;
	if (!(table_f=fopen(fetchbuf, "r")))	continue; /* ignore erroe */
	if (!build_ccs_table(ct, 0))	return	EOF;
	i = 1;	/* point after header type - magic field	*/
	do {
	    if (!fgets(fetchbuf, sizeof(fetchbuf), table_f))	continue;
	    for (n=t_keys; n--;)
		if (isbcstr(table_key[n]))	{
		register char*	cp = index(fetchbuf, Space);
		    if (!cp)	{
		wf:	prgmerr(0, "wrong format %s", fetchbuf);
			goto	ntry;
		    }
		    if (!n || n == Key_Magic)
			tc = ct->tc;
		    else	tc = ct[i].tc;
		    if (n < Key_Mark)	{	/* get 2nd param, data len */
			sscanf(++cp, "%d", &tc->dlen);
			cp = index(cp, Space);
		    }
		    tc->key = n;
		    if(n && n < Key_Magic) {	/* common formats	*/
		    register int	dtype;
			if (!sscanf(++cp, "%s", &tc->dtype) ||
				!(cp = index(cp, Space)))	goto	wf;
			tc->dtype &= mask;	ep = (char*)&tc->dtype;
			dtype = *ep == '#';	/* IM value	*/
			if (n > Key_Depth && dtype) {	/* check format	*/
				tc->val = sscan_format(++cp, n);
				if (n == Key_Encode)
					ct[i].pos = -1;
			}
			else sscanf(++cp, "%d", dtype ? &tc->val : &ct[i].pos);
			/* if not empty string and not comment, add symbols */
			if ((cp=index(cp, Space)) && *++cp != '#')	{
			    if (ep=index(cp, Space))
				n = ep - cp;
			    else n = strlen(cp);
			    if (tc->symbol=NZALLOC(n, 1, No))
				strncpy(tc->symbol, cp, n);
			}
		    } else {	/* key & marks	*/
			switch (n)	{
			case Key_Header:
			case Key_Magic:
			    if (!sscanf(++cp, "%d", &tc->dtype) ||
				(tc->key=tc->dlen) && (cp=index(cp, Space)) &&
				!sscanf(++cp, "%4c", &tc->key))	goto	wf;
			    if (cp = index(cp, Space))	{
				cp++;
				if (iscstr("line"))
					tc->val = LINE_HEADER;
				else if (iscstr("skip") || !tc->key)
					tc->val = SKIP_HEADER;
			    }	i--;	/* header is reserved	*/
			    break;
			case Key_Mark:
			    if (iscstr("end of"))	cp += 7;
			    if (iscstr("item") || iscstr("line"))
				cp += 5,	tc->key = Mark_EOI;
			    else	{
				if (iscstr("eoh"))	cp += 4;
				else if (iscstr("header"))	cp += 7;
				else	goto	wf;
			    }
			    if (!sscanf(cp, "%s", &tc->val))	goto	wf;
			    tc->val &= mask;
			    if (index((char*)&tc->val, LF))
				ct[0].tc->val = LINE_HEADER;
			    break;
			case Key_Separator:
			default:	goto	wf;
			}
		    }
		i++;	break;
		}
	    if (n < 0)	prgmerr(0, "non table key %s", fetchbuf);
	} while (!feof(table_f));
	fclose(table_f);

	tc = ct->tc;
	if (tc->key)	{	/* magic present (not skip)	*/
	(*img->r_seek)(img->IN_FP, 0, SEEK_SET);
	(*img->i_read)(&magic, tc->dlen, 1, img->IN_FP);
	    if (tc->key != magic)	continue;
	    else if (!tc->val)	{	/* file macth in address mode	*/
#ifdef	FAST_REVERSE_SORT
		QuickSort(1, i-1, i-1, ct);
#else
		qsort(ct+1, i-1, sizeof(ct[1]), ctcompare);
#endif
		for (n=0; ++n < i;)	{
		    tc = ct[n].tc;
		    if (tc->key == Key_Colormap)	{
			if (ftell(img->IN_FP) != ct[n].pos)
				(*img->r_seek)(img->IN_FP, ct[n].pos, SEEK_SET);
			setup_colormap(tc, img);
		    } else if (ct[n].pos > 0 && tc->dtype)	{
			(*img->r_seek)(img->IN_FP, ct[n].pos, 0);
			fscanf(img->IN_FP, (char*)&tc->dtype, &tc->val);
		    }
		}
		    goto	tif_skip;
	    } else {	/* in line mode	*/
		register char*	mark = (char*)findmark(ct, i, Mark_EOI);
		    if (n=(int)index(mark, LF))
			fgets(fetchbuf, sizeof(fetchbuf), img->IN_FP);
		    else for (n=0; fetchbuf[n] != *mark; n++)
			fetchbuf[n] = GETC(img->IN_FP);

		goto	tif_skip;
	    }
	} else	{	/* skipping mode	*/
tif_skip:	tc = ct->tc;
		if (tc->dtype)	/* skip to eoh	*/
		    (*img->r_seek)(img->IN_FP, tc->dtype, 0);
		else if (n=findmark(ct, i, Mark_EOH))	{
		register char*	mark = (char*)n;
	fag:		while (GETC(img->IN_FP) != *mark && !Feof(img->IN_FP));
			if (mark[1] && GETC(img->IN_FP) != mark[1])
			    if (!Feof(img->IN_FP))
				goto	fag;
			    else return	prgmerr(0, "? eoh mark %s", mark);
		} else	message("warning: not search for eoh\n");
		fclose(name_f);
		return	setup_img(img, i);
	}
    } while (!feof(name_f));
    fclose(name_f);
return	EOF;
}

#ifdef	PROGRAM_TEST
char	*Progname;
main()
{
U_IMAGE	img;
	ccs_table_if(&img);
}
#endif
