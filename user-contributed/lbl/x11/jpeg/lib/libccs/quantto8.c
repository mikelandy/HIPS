/*	QUANT_TO_8 . C
#
%	quantizing to Pseudo color image
*/

#include "header.def"
#include "imagedef.h"

#define	SYS_DEPTH	8
#define	BitsPPix	5		/* # bits/per pixel */
#define	HC_SIZE		(1<<BitsPPix)	/* histo size for each color */
#define	C_DEPTH		2
#define	C_LEN		(1<<C_DEPTH)	/* # cells/color to use */
#define	TOP_VALUE	(1<<24)
#define	RED	0
#define	GREEN	1
#define	BLUE	2

typedef	struct colorbox {
	struct colorbox	*next, *prev;
	int	rmin,rmax, gmin,gmax, bmin,bmax,
		total;
} CBOX;

typedef	struct {
	int	num_ents,
		entries[MaxColors][2];
	} CCELL;

static	CBOX	*freeboxes, *usedboxes;
static	CCELL	**ColorCells;
static	int	histogram[HC_SIZE][HC_SIZE][HC_SIZE],
		height, width, num_colors;


static void
effectivebox(box)
CBOX	*box;
{
int	*histp,ir,ig,ib;
int	rmin,rmax,gmin,gmax,bmin,bmax;

	rmin = box->rmin;	rmax = box->rmax;
	gmin = box->gmin;	gmax = box->gmax;
	bmin = box->bmin;	bmax = box->bmax;

	if (rmax>rmin) {
	    for (ir=rmin; ir<=rmax; ir++)
		for (ig=gmin; ig<=gmax; ig++) {
		    histp = &histogram[ir][ig][bmin];
		    for (ib=bmin; ib<=bmax; ib++)
			if (*histp++) {
				box->rmin = rmin = ir;
				goto	got_rmin;
			}
		}
	got_rmin:
		if (rmax>rmin)
		    for (ir=rmax; ir>=rmin; ir--)
			for (ig=gmin; ig<=gmax; ig++) {
			    histp = &histogram[ir][ig][bmin];
			    for (ib=bmin; ib<=bmax; ib++)
				if (*histp++) {
					box->rmax = rmax = ir;
					goto	got_rmax;
				}
			}
	}
got_rmax:
	if (gmax>gmin) {
	    for (ig=gmin; ig<=gmax; ig++)
		for (ir=rmin; ir<=rmax; ir++) {
		    histp = &histogram[ir][ig][bmin];
		    for (ib=bmin; ib<=bmax; ib++)
			if (*histp++) {
				box->gmin = gmin = ig;
				goto	got_gmin;
			}
		}
	got_gmin:
		if (gmax>gmin)
		    for (ig=gmax; ig>=gmin; ig--)
			for (ir=rmin; ir<=rmax; ir++) {
			    histp = &histogram[ir][ig][bmin];
			    for (ib=bmin; ib<=bmax; ib++)
				if (*histp++) {
					box->gmax = gmax = ig;
					goto	got_gmax;
				}
			}
	}
got_gmax:
	if (bmax>bmin) {
	    for (ib=bmin; ib<=bmax; ib++)
		for (ir=rmin; ir<=rmax; ir++) {
		    histp = &histogram[ir][gmin][ib];
		    for (ig=gmin; ig<=gmax; ig++) {
			if (*histp != 0) {
				box->bmin = bmin = ib;
				goto	got_bmin;
			}
		    histp += HC_SIZE;
		    }
		}
	got_bmin:
		if (bmax>bmin)
		    for (ib=bmax; ib>=bmin; ib--)
			for (ir=rmin; ir<=rmax; ir++) {
			    histp = &histogram[ir][gmin][ib];
			    for (ig=gmin; ig<=gmax; ig++) {
				if (*histp) {
					bmax = ib;
					goto	got_bmax;
				}
				histp += HC_SIZE;
			    }
			}
	}
got_bmax:	return;
}


static void
calc_histogram(p, box)
byte	*p;
CBOX *box;
{
int	i,j,r,g,b,*ptr;

	box->rmin = box->gmin = box->bmin = 999;
	box->rmax = box->gmax = box->bmax = -1;
	box->total = width * height;

	/* zero out histogram */
	ptr = histogram;
	for (i=HC_SIZE*HC_SIZE*HC_SIZE; i>0; i--)	*ptr++ = 0;

	/* calculate histogram */
	for (i=0; i<height; i++)
	    for (j=0; j<width; j++) {
		r = (*p++) >> (SYS_DEPTH-BitsPPix);
		g = (*p++) >> (SYS_DEPTH-BitsPPix);
		b = (*p++) >> (SYS_DEPTH-BitsPPix);

		if (r < box->rmin) box->rmin = r;
		if (r > box->rmax) box->rmax = r;

		if (g < box->gmin) box->gmin = g;
		if (g > box->gmax) box->gmax = g;

		if (b < box->bmin) box->bmin = b;
		if (b > box->bmax) box->bmax = b;
		histogram[r][g][b]++;
	    }
}


static void
splitbox(ptr)
CBOX *ptr;
{
CBOX	*new;
int	hist2[HC_SIZE], first, last, i, rdel, gdel, bdel;
int	*iptr, *histp, ir, ig, ib,
	rmin,rmax,gmin,gmax,bmin,bmax,	which;

	/*
	* see which axis is the largest, do a histogram along that axis.
	* Split at median point. Contract both new boxes to fit points and return
	*/

	first = last = 0;	/* shut RT hcc compiler up */

	rmin = ptr->rmin;  rmax = ptr->rmax;
	gmin = ptr->gmin;  gmax = ptr->gmax;
	bmin = ptr->bmin;  bmax = ptr->bmax;

	rdel = rmax - rmin;
	gdel = gmax - gmin;
	bdel = bmax - bmin;

	if (rdel>=gdel && rdel>=bdel)
		which = RED;
	else	which = gdel >= bdel ? GREEN : BLUE;

	/* get histogram along longest axis */
	switch (which) {
	case RED:
		histp = &hist2[rmin];
		for (ir=rmin; ir<=rmax; ir++, histp++) {
		    *histp = 0;
		    for (ig=gmin; ig<=gmax; ig++) {
			iptr = &histogram[ir][ig][bmin];
			for (ib=bmin; ib<=bmax; ib++)
				*histp += *iptr++;
		    }
		}
		first = rmin;  last = rmax;
		break;
	case GREEN:
		histp = &hist2[gmin];
		for (ig=gmin; ig<=gmax; ig++, histp++) {
		    *histp = 0;
		    for (ir=rmin; ir<=rmax; ir++) {
			iptr = &histogram[ir][ig][bmin];
			for (ib=bmin; ib<=bmax; ib++)
			*histp += *iptr++;
		    }
		}
		first = gmin;  last = gmax;
		break;
	case BLUE:
		histp = &hist2[bmin];
		for (ib=bmin; ib<=bmax; ib++) {
		    *histp = 0;
		    for (ir=rmin; ir<=rmax; ir++) {
			iptr = &histogram[ir][gmin][ib];
			for (ig=gmin; ig<=gmax; ig++) {
				*histp += *iptr;
				iptr += HC_SIZE;
			}
		    }
		    ++histp;
		}
		first = bmin;  last = bmax;
		break;
	}

	{
	int	sum, sum2;	/* find median point */

	histp = &hist2[first];

	sum2 = ptr->total >> 1;
	histp = &hist2[first];
	sum = 0;

	for (i=first; i<=last && (sum += *histp++)<sum2; i++);
	if (i==first) i++;
	}

	/* Create new box, re-allocate points */

	new = freeboxes;
	freeboxes = new->next;
	if (freeboxes) freeboxes->prev = NULL;

	if (usedboxes) usedboxes->prev = new;
	new->next = usedboxes;
	usedboxes = new;

	{
	int	sum,j;

	histp = &hist2[j=first];

	for (sum=0; j < i; j++)	sum += *histp++;
	new->total = sum;
	for (sum=0; j <= last; j++)	sum += *histp++;
	ptr->total = sum;
	}

	new->rmin = rmin;  new->rmax = rmax;
	new->gmin = gmin;  new->gmax = gmax;
	new->bmin = bmin;  new->bmax = bmax;

	switch (which) {
	case RED:	new->rmax = i-1; ptr->rmin = i;	break;
	case GREEN:	new->gmax = i-1; ptr->gmin = i;	break;
	case BLUE:	new->bmax = i-1; ptr->bmin = i;	break;
	}

	effectivebox(new);
	effectivebox(ptr);
}


static	void
fill_freeboxes()
{
while (freeboxes) {
CBOX	*tmp, *ptr=NULL;
int	size = -1;

	tmp = usedboxes;

	while (tmp) {
	   if ((tmp->rmax > tmp->rmin  ||
		tmp->gmax > tmp->gmin  ||
		tmp->bmax > tmp->bmin)  &&  tmp->total > size) {
		ptr = tmp;
		size = tmp->total;
	    }
	    tmp = tmp->next;
	}
	if (ptr)
		splitbox(ptr);
	else	break;
}
}

static void
assign_color(ptr, rp, gp, bp)
CBOX	*ptr;
byte	*rp, *gp, *bp;
{
	*rp = ((ptr->rmin + ptr->rmax) << (SYS_DEPTH - BitsPPix)) >> 1;
	*gp = ((ptr->gmin + ptr->gmax) << (SYS_DEPTH - BitsPPix)) >> 1;
	*bp = ((ptr->bmin + ptr->bmax) << (SYS_DEPTH - BitsPPix)) >> 1;
}


static CCELL*
create_colorcell(rg_cmap,r1,g1,b1)
cmap_t	*rg_cmap[];
int	r1,g1,b1;
{
register int	i,tmp, dist;
register CCELL	*ptr;
register byte	*rp,*gp,*bp;
int		ir,ig,ib, mindist;

	ir = r1 >> (SYS_DEPTH-C_DEPTH);
	ig = g1 >> (SYS_DEPTH-C_DEPTH);
	ib = b1 >> (SYS_DEPTH-C_DEPTH);

	r1 &= ~1 << (SYS_DEPTH-C_DEPTH);
	g1 &= ~1 << (SYS_DEPTH-C_DEPTH);
	b1 &= ~1 << (SYS_DEPTH-C_DEPTH);

	ptr = (CCELL *) NZALLOC(sizeof(CCELL), 1, No);
	*(ColorCells + ir*C_LEN*C_LEN + ig*C_LEN + ib) = ptr;
	ptr->num_ents = 0;

	/* step 1: find all colors inside this cell, while we're at
	it, find distance of centermost point to furthest corner */

	mindist = 99999999;

	rp=rg_cmap[RED];  gp=rg_cmap[GREEN];  bp=rg_cmap[BLUE];
	for (i=0; i<num_colors; i++,rp++,gp++,bp++)
	    if (*rp>>(SYS_DEPTH-C_DEPTH) == ir &&
		*gp>>(SYS_DEPTH-C_DEPTH) == ig &&
		*bp>>(SYS_DEPTH-C_DEPTH) == ib)	{

		ptr->entries[ptr->num_ents][0] = i;
		ptr->entries[ptr->num_ents][1] = 0;
		++ptr->num_ents;

		tmp = *rp - r1;
		if (tmp < (MaxColors/C_LEN/2))
			tmp = MaxColors/C_LEN-1 - tmp;
		dist = tmp*tmp;

		tmp = *gp - g1;
		if (tmp < (MaxColors/C_LEN/2))
			tmp = MaxColors/C_LEN-1 - tmp;
		dist += tmp*tmp;

		tmp = *bp - b1;
		if (tmp < (MaxColors/C_LEN/2))
			tmp = MaxColors/C_LEN-1 - tmp;
		dist += tmp*tmp;

		if (dist < mindist)	mindist = dist;
	    }

	/* step 3: find all points within that distance to box */

	rp=rg_cmap[RED];  gp=rg_cmap[GREEN];  bp=rg_cmap[BLUE];
	for (i=0; i<num_colors; i++,rp++,gp++,bp++)
	    if (*rp >> (SYS_DEPTH-C_DEPTH) != ir ||
		*gp >> (SYS_DEPTH-C_DEPTH) != ig ||
		*bp >> (SYS_DEPTH-C_DEPTH) != ib)	{

		dist = 0;

		if ((tmp = r1 - *rp)>0 ||
			(tmp = *rp - (r1 + MaxColors/C_LEN-1)) > 0 )
			dist += tmp*tmp;

		if( (tmp = g1 - *gp)>0 ||
			(tmp = *gp - (g1 + MaxColors/C_LEN-1)) > 0 )
			dist += tmp*tmp;

		if( (tmp = b1 - *bp)>0 ||
			(tmp = *bp - (b1 + MaxColors/C_LEN-1)) > 0 )
			dist += tmp*tmp;

		if( dist < mindist ) {
			ptr->entries[ptr->num_ents][0] = i;
			ptr->entries[ptr->num_ents][1] = dist;
			++ptr->num_ents;
		}
	    }

	/* sort color cells by distance, use cheap exchange sort */
	{
	int	n, next_n;

	n = ptr->num_ents - 1;
	while (n>0) {
	    next_n = 0;
	    for (i=0; i<n; ++i)
		if (ptr->entries[i][1] > ptr->entries[i+1][1]) {
			tmp = ptr->entries[i][0];
			ptr->entries[i][0] = ptr->entries[i+1][0];
			ptr->entries[i+1][0] = tmp;
			tmp = ptr->entries[i][1];
			ptr->entries[i][1] = ptr->entries[i+1][1];
			ptr->entries[i+1][1] = tmp;
			next_n = i;
		}
	    n = next_n;
	}
	}
return (ptr);
}


static void
map_colortable(rg_cmap)
cmap_t	*rg_cmap[];
{
CCELL	*cell;
int	ir,ig,ib, *histp = histogram;

    for (ir=0; ir<HC_SIZE; ir++)
	for (ig=0; ig<HC_SIZE; ig++)
	    for (ib=0; ib<HC_SIZE; ib++) {
		if (*histp==0) *histp = -1;
		else {
		int	i, j, tmp, d2, dist;

		    cell = *(ColorCells +
			( ((ir>>(BitsPPix-C_DEPTH)) << C_DEPTH*2)
			+ ((ig>>(BitsPPix-C_DEPTH)) << C_DEPTH)
			+ (ib>>(BitsPPix-C_DEPTH)) ) );

		    if (cell==NULL)
			cell = create_colorcell(rg_cmap,
				ir<<(SYS_DEPTH-BitsPPix),
				ig<<(SYS_DEPTH-BitsPPix),
				ib<<(SYS_DEPTH-BitsPPix));

		    dist = TOP_VALUE;
		    for (i=0; i<cell->num_ents && dist>cell->entries[i][1]; i++) {
			j = cell->entries[i][0];
			d2 = rg_cmap[RED][j] - (ir << (SYS_DEPTH-BitsPPix));
			d2 *= d2;
			tmp = rg_cmap[GREEN][j] - (ig << (SYS_DEPTH-BitsPPix));
			d2 += tmp*tmp;
			tmp = rg_cmap[BLUE][j] - (ib << (SYS_DEPTH-BitsPPix));
			d2 += tmp*tmp;
			if(d2 < dist)	{ dist = d2;	*histp = j; }
		    }
		}
		histp++;
	    }
}


static
quant_fsdither(inptr, outptr, rg_cmap)
byte	*inptr, *outptr;
cmap_t	*rg_cmap[];
{
register int	*thisptr, *nextptr;
int	*thisline, *nextline, *tmpptr,
	r1, g1, b1, r2, g2, b2,
	i, j, imax, jmax, oval;
byte	*tmpbptr;
int	lastline, lastpixel;

	imax = height - 1;
	jmax = width - 1;

	thisline = (int *) NZALLOC(width, 3 * sizeof(int), "dither this");
	nextline = (int *) NZALLOC(width, 3 * sizeof(int), "dither next");

	/* get first line of picture */
	for (j=width * 3, tmpptr=nextline, tmpbptr=inptr; j; j--)
	*tmpptr++ = (int) *tmpbptr++;

	for (i=0; i<height; i++) {
	/* swap thisline and nextline */
		tmpptr = thisline; thisline = nextline; nextline = tmpptr;
		lastline = i==imax;

	/* read in next line */
	    for (j=width * 3, tmpptr=nextline; j; j--)
		*tmpptr++ = (int) *inptr++;

	/* dither this line and put it into the output picture */
	    thisptr = thisline;	nextptr = nextline;

	    for (j=0; j<width; j++) {
		lastpixel = (j==jmax);

		r2 = *thisptr++;  g2 = *thisptr++;  b2 = *thisptr++;

		if (r2<0)	r2=0;
		else if (r2>=MaxColors)	r2=MaxColors-1;
		if (g2<0)	g2=0;
		else if (g2>=MaxColors)	g2=MaxColors-1;
		if (b2<0)	b2=0;
		else if (b2>=MaxColors)	b2=MaxColors-1;

		r1 = r2;  g1 = g2;  b1 = b2;

		r2 >>= (SYS_DEPTH-BitsPPix);
		g2 >>= (SYS_DEPTH-BitsPPix);
		b2 >>= (SYS_DEPTH-BitsPPix);

		if ( (oval=histogram[r2][g2][b2]) == -1) {
		int	ci, cj, dist, d2, tmp;
		CCELL*	cell = *( ColorCells +
				( ((r2>>(BitsPPix-C_DEPTH)) << C_DEPTH*2)
				+ ((g2>>(BitsPPix-C_DEPTH)) << C_DEPTH )
				+  (b2>>(BitsPPix-C_DEPTH)) ) );

		    if (cell==NULL)	cell = create_colorcell(rg_cmap,r1,g1,b1);

		    dist = TOP_VALUE;
		    for (ci=0; ci<cell->num_ents && dist>cell->entries[ci][1]; ci++) {
			cj = cell->entries[ci][0];
			d2 = (rg_cmap[RED][cj] >> (SYS_DEPTH-BitsPPix)) - r2;
			d2 *= d2;
			tmp = (rg_cmap[GREEN][cj] >> (SYS_DEPTH-BitsPPix)) - g2;
			d2 += tmp*tmp;
			tmp = (rg_cmap[BLUE][cj] >> (SYS_DEPTH-BitsPPix)) - b2;
			d2 += tmp*tmp;
			if (d2<dist) { dist = d2;  oval = cj; }
		    }
		    histogram[r2][g2][b2] = oval;
		}

		*outptr++ = oval;
		r1 -= rg_cmap[RED][oval];
		g1 -= rg_cmap[GREEN][oval];
		b1 -= rg_cmap[BLUE][oval];
		/* can't use tables because r1,g1,b1 go negative */

		if (!lastpixel) {
			thisptr[0] += r1*7 >> 4;
			thisptr[1] += g1*7 >> 4;
			thisptr[2] += b1*7 >> 4;
		}

		if (!lastline) {
		    if (j) {
			nextptr[-3] += r1*3 >> 4;
			nextptr[-2] += g1*3 >> 4;
			nextptr[-1] += b1*3 >> 4;
		    }

		    nextptr[0] += r1*5 >> 4;
		    nextptr[1] += g1*5 >> 4;
		    nextptr[2] += b1*5 >> 4;

		    if (!lastpixel) {
			nextptr[3] += r1 >> 4;
			nextptr[4] += g1 >> 4;
			nextptr[5] += b1 >> 4;
		    }
		    nextptr += 3;
		}
	    }
	}
	CFREE(thisline);
	CFREE(nextline);
return	0;
}


quant_to_8(img, i, rg_cmap, obuf)
U_IMAGE	*img;
cmap_t	*rg_cmap[];
VType	*obuf;
{
CBOX	*box_list, *ptr;

	if (img->color_form == CFM_ILL)	{
	register char	*buffer = img->src;
		img->src = NZALLOC(img->width*img->height, 3, "l_2_c");
		line_to_cell_color(img->src, buffer, img->width, img->height);
		CFREE(buffer);
	}
	height = img->height;	width = img->width;	num_colors = i;
	usedboxes = NULL;
	box_list = freeboxes = (CBOX *) ZALLOC(num_colors, sizeof(CBOX),
					"to8() - 'freeboxes'\n");

	for (i=0; i<num_colors; i++) {
		freeboxes[i].next = &freeboxes[i+1];
		freeboxes[i].prev = &freeboxes[i-1];
	}
	freeboxes[0].prev = NULL;
	freeboxes[num_colors-1].next = NULL;

	ptr = freeboxes;
	freeboxes = ptr->next;
	if (freeboxes)
		freeboxes->prev = NULL;

	ptr->next = usedboxes;
	usedboxes = ptr;
	if (ptr->next)	ptr->next->prev = ptr;

	calc_histogram(img->src, ptr);

	fill_freeboxes();

	for (i=0, ptr=usedboxes; i<num_colors && ptr; i++, ptr=ptr->next)
	assign_color(ptr, rg_cmap[RED]+i, rg_cmap[GREEN]+i, rg_cmap[BLUE]+i);
	num_colors = i;
	CFREE(box_list);
	box_list = freeboxes = usedboxes = NULL;

	{	int	j = C_LEN*C_LEN*C_LEN;
	ColorCells = (CCELL **) ZALLOC(j, sizeof(CCELL *), "CCELL");
	map_colortable(rg_cmap);
	i = quant_fsdither(img->src, obuf, rg_cmap);
	while (j--)
		if (ColorCells[j])	CFREE(ColorCells[j]);
	CFREE(ColorCells);
	}
return	i;
}
