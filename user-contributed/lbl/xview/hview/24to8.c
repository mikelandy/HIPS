#include <stdio.h>
#include <sys/types.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <hipl_format.h>

byte      r[256], g[256], b[256];     

extern char     *myname;		/* program name for printf's */

/***************************************************************/
/***************************************************************/

/*
 * xv24to8.c  -  contains the 24-to-8-bit Conv24to8 procedure
 *
 * The Conv24to8 procedure takes a pointer to a 24-bit image (loaded
 * previously).  The image will be a w * h * 3 byte array of
 * bytes.  The image will be arranged with 3 bytes per pixel (in order
 * R, G, and B), pixel 0 at the top left corner.  (As normal.)
 * The procedure also takes a maximum number of colors to use (numcols)
 *
 * The Conv24to8 procedure will set up the following:  it will allocate and
 * create 'pic', a pWIDE*pHIGH 8-bit picture.  it will set up pWIDE and pHIGH.
 * it will load up the r[], g[], and b[] colormap arrays.  it will NOT
 * calculate numcols, since the cmap sort procedure has to be called anyway
 *
 * Conv24to8 returns '0' if successful, '1' if it failed (presumably on a
 * malloc())
 *
 * contains:
 *   Cont24to8()
 *   Init24to8()
 */

#define	MAX_CMAP_SIZE	256
#define	COLOR_DEPTH	8
#define	MAX_COLOR	256
#define	B_DEPTH		5	/* # bits/pixel to use */
#define	B_LEN		(1<<B_DEPTH)
#define	C_DEPTH		2
#define	C_LEN		(1<<C_DEPTH)	/* # cells/color to use */
#define RANGE(a,b,c) { if (a < b) a = b;  if (a > c) a = c; }

typedef struct colorbox {
    struct colorbox *next, *prev;
    int       rmin, rmax, gmin, gmax, bmin, bmax;
    int       total;
}         CBOX;

typedef struct {
    int       num_ents;
    int       entries[MAX_CMAP_SIZE][2];
}         CCELL;

static byte *pic24, *pic;
static int num_colors, WIDE, HIGH, pWIDE, pHIGH;
static int histogram[B_LEN][B_LEN][B_LEN];

CBOX     *freeboxes, *usedboxes;
CCELL   **ColorCells;

static void get_histogram();
static CBOX *largest_box();
static void splitbox();
static void shrinkbox();
static void assign_color();
static CCELL *create_colorcell();
static void map_colortable();
static int quant_fsdither();
static int Quick24to8();
static int QuickCheck();

static int tbl1[512],		/* tables used in F-S Dithering */
          tbl3[512],		/* contain i/16, 3i/16, 5i/16, 7i/16, */
          tbl5[512],		/* (i=-256..255) respectively */
          tbl7[512];

int       slow24 = 0;

/****************************/
void
Init24to8()
/****************************/
{
    /* initialize Floyd-Steinberg division tables */
    /* make sure rounding is done correctly for negative values! */

    int       i;

    for (i = -256; i < 0; i++) {
	tbl1[i + 256] = -((8 - i) / 16);
	tbl3[i + 256] = -((8 - 3 * i) / 16);
	tbl5[i + 256] = -((8 - 5 * i) / 16);
	tbl7[i + 256] = -((8 - 7 * i) / 16);
    }

    for (i = 0; i < 256; i++) {
	tbl1[i + 256] = (i + 8) / 16;
	tbl3[i + 256] = (3 * i + 8) / 16;
	tbl5[i + 256] = (5 * i + 8) / 16;
	tbl7[i + 256] = (7 * i + 8) / 16;
    }
}



/****************************/
int
Conv24to8(p, op, w, h, nc, mono, rr, gg, bb)
/****************************/
    byte     *p, *op;
    int       w, h, nc, mono;
    byte     *rr, *gg, *bb;
{
    int       i, rval;
    CBOX     *box_list, *ptr;

    /* copy arguments to local-global variables */
    pic24 = p;
    pWIDE = WIDE = w;
    pHIGH = HIGH = h;
    num_colors = nc;


    /*
     * allocate pic immediately, so that if we can't allocate it, we don't
     * waste time running this algorithm
     */

    pic = op;
    if (pic == NULL) {
	fprintf(stderr, "%s: Conv24to8() - failed to allocate 'pic'\n", myname);
	return (0);
    }
    /*
     * quick check:  if we're going to display a greyscale or 1-bit image,
     * DON'T run this annoyingly time-consuming code.  Just convert the
     * 24-bit color image to an 8-bit greyscale.  This takes virtually no
     * time, by comparision, and it has the same effect.
     */

    if (mono /* || nc==0 */ ) {
	byte     *pp, *p24;

	for (i = 0; i < 256; i++)
	    r[i] = g[i] = b[i] = i;	/* greyscale colormap */
	pp = pic;
	p24 = pic24;
	for (i = WIDE * HIGH; i > 0; i--, pp++, p24 += 3)
	    /* pp=.33R+.5G+.17B */
	    *pp = (p24[0] * 11 + p24[1] * 16 + p24[2] * 5) >> 5;

	get_global_cmap(rr, gg, bb);
	return (num_colors);
    }
    if (QuickCheck(pic24, w, h, nc)) {
	/* see if it's a <256 color RGB pic */
	fprintf(stderr, "No color compression was necessary.\n");
	get_global_cmap(rr, gg, bb);
	return num_colors;
    } else if (!slow24) {
	fprintf(stderr, "Doing quick 24-bit to 8-bit conversion....\n");
	rval = Quick24to8(pic24, w, h);
	get_global_cmap(rr, gg, bb);
	return (num_colors);
    } else
	fprintf(stderr, "Doing full 24-bit to 8-bit conversion.....\n");

    /**** STEP 1:  create empty boxes ****/

    usedboxes = NULL;
    box_list = freeboxes = (CBOX *) malloc(num_colors * sizeof(CBOX));

    if (box_list == NULL) {
	fprintf(stderr, "%s: Conv24to8() - failed to allocate 'freeboxes'\n", myname);
	return (0);
    }
    for (i = 0; i < num_colors; i++) {
	freeboxes[i].next = &freeboxes[i + 1];
	freeboxes[i].prev = &freeboxes[i - 1];
    }
    freeboxes[0].prev = NULL;
    freeboxes[num_colors - 1].next = NULL;


    /**** STEP 2: get histogram, initialize first box ****/

    ptr = freeboxes;
    freeboxes = ptr->next;
    if (freeboxes)
	freeboxes->prev = NULL;

    ptr->next = usedboxes;
    usedboxes = ptr;
    if (ptr->next)
	ptr->next->prev = ptr;

    get_histogram(ptr);


    /**** STEP 3: continually subdivide boxes until no more free boxes remain */

    while (freeboxes) {
	ptr = largest_box();
	if (ptr)
	    splitbox(ptr);
	else
	    break;
    }


    /**** STEP 4: assign colors to all boxes ****/

    for (i = 0, ptr = usedboxes; i < num_colors && ptr; i++, ptr = ptr->next) {
	assign_color(ptr, &r[i], &g[i], &b[i]);
    }

    /* We're done with the boxes now */
    num_colors = i;
    free(box_list);
    box_list = freeboxes = usedboxes = NULL;


    /**** STEP 5: scan histogram and map all values to closest color */

    /* 5a: create cell list as described in Heckbert[2] */

    ColorCells = (CCELL **) calloc(C_LEN * C_LEN * C_LEN, sizeof(CCELL *));


    /* 5b: create mapping from truncated pixel space to color table entries */

    map_colortable();


    /**** STEP 6: scan image, match input values to table entries */

    i = quant_fsdither();

    /* free everything that can be freed */
    free(ColorCells);

    get_global_cmap(rr, gg, bb);
    return num_colors;
}


/****************************/
get_global_cmap(rr, gg, bb)
    byte     *rr, *gg, *bb;
{
    int       i;

    for (i = 0; i < 256; i++) {	/* pass back results */
	rr[i] = r[i];
	gg[i] = g[i];
	bb[i] = b[i];
    }
}

/****************************/
static void
get_histogram(box)
    CBOX     *box;
/****************************/
{
    int       i, j, r, g, b, *ptr;
    byte     *p;

    box->rmin = box->gmin = box->bmin = 999;
    box->rmax = box->gmax = box->bmax = -1;
    box->total = WIDE * HIGH;

    /* zero out histogram */
    ptr = &histogram[0][0][0];
    for (i = B_LEN * B_LEN * B_LEN; i > 0; i--)
	*ptr++ = 0;

    /* calculate histogram */
    p = pic24;
    for (i = 0; i < HIGH; i++)
	for (j = 0; j < WIDE; j++) {
	    r = (*p++) >> (COLOR_DEPTH - B_DEPTH);
	    g = (*p++) >> (COLOR_DEPTH - B_DEPTH);
	    b = (*p++) >> (COLOR_DEPTH - B_DEPTH);

	    if (r < box->rmin)
		box->rmin = r;
	    if (r > box->rmax)
		box->rmax = r;

	    if (g < box->gmin)
		box->gmin = g;
	    if (g > box->gmax)
		box->gmax = g;

	    if (b < box->bmin)
		box->bmin = b;
	    if (b > box->bmax)
		box->bmax = b;
	    histogram[r][g][b]++;
	}
}



/******************************/
static CBOX *
largest_box()
/******************************/
{
    CBOX     *tmp, *ptr;
    int       size = -1;

    tmp = usedboxes;
    ptr = NULL;

    while (tmp) {
	if ((tmp->rmax > tmp->rmin ||
	     tmp->gmax > tmp->gmin ||
	     tmp->bmax > tmp->bmin) && tmp->total > size) {
	    ptr = tmp;
	    size = tmp->total;
	}
	tmp = tmp->next;
    }
    return (ptr);
}



/******************************/
static void
splitbox(ptr)
    CBOX     *ptr;
/******************************/
{
    int       hist2[B_LEN], first, last, i, rdel, gdel, bdel;
    CBOX     *new;
    int      *iptr, *histp, ir, ig, ib;
    int       rmin, rmax, gmin, gmax, bmin, bmax;
    enum {
	RED, GREEN, BLUE
    }         which;

    /*
     * see which axis is the largest, do a histogram along that axis.  Split
     * at median point.  Contract both new boxes to fit points and return
     */

    first = last = 0;		/* shut RT hcc compiler up */

    rmin = ptr->rmin;
    rmax = ptr->rmax;
    gmin = ptr->gmin;
    gmax = ptr->gmax;
    bmin = ptr->bmin;
    bmax = ptr->bmax;

    rdel = rmax - rmin;
    gdel = gmax - gmin;
    bdel = bmax - bmin;

    if (rdel >= gdel && rdel >= bdel)
	which = RED;
    else if (gdel >= bdel)
	which = GREEN;
    else
	which = BLUE;

    /* get histogram along longest axis */
    switch (which) {

    case RED:
	histp = &hist2[rmin];
	for (ir = rmin; ir <= rmax; ir++) {
	    *histp = 0;
	    for (ig = gmin; ig <= gmax; ig++) {
		iptr = &histogram[ir][ig][bmin];
		for (ib = bmin; ib <= bmax; ib++) {
		    *histp += *iptr;
		    ++iptr;
		}
	    }
	    ++histp;
	}
	first = rmin;
	last = rmax;
	break;

    case GREEN:
	histp = &hist2[gmin];
	for (ig = gmin; ig <= gmax; ig++) {
	    *histp = 0;
	    for (ir = rmin; ir <= rmax; ir++) {
		iptr = &histogram[ir][ig][bmin];
		for (ib = bmin; ib <= bmax; ib++) {
		    *histp += *iptr;
		    ++iptr;
		}
	    }
	    ++histp;
	}
	first = gmin;
	last = gmax;
	break;

    case BLUE:
	histp = &hist2[bmin];
	for (ib = bmin; ib <= bmax; ib++) {
	    *histp = 0;
	    for (ir = rmin; ir <= rmax; ir++) {
		iptr = &histogram[ir][gmin][ib];
		for (ig = gmin; ig <= gmax; ig++) {
		    *histp += *iptr;
		    iptr += B_LEN;
		}
	    }
	    ++histp;
	}
	first = bmin;
	last = bmax;
	break;
    }


    /* find median point */
    {
	int       sum, sum2;

	histp = &hist2[first];

	sum2 = ptr->total / 2;
	histp = &hist2[first];
	sum = 0;

	for (i = first; i <= last && (sum += *histp++) < sum2; i++);
	if (i == first)
	    i++;
    }


    /* Create new box, re-allocate points */

    new = freeboxes;
    freeboxes = new->next;
    if (freeboxes)
	freeboxes->prev = NULL;

    if (usedboxes)
	usedboxes->prev = new;
    new->next = usedboxes;
    usedboxes = new;

    {
	int       sum1, sum2, j;

	histp = &hist2[first];
	sum1 = 0;
	for (j = first; j < i; ++j)
	    sum1 += *histp++;
	sum2 = 0;
	for (j = i; j <= last; ++j)
	    sum2 += *histp++;
	new->total = sum1;
	ptr->total = sum2;
    }


    new->rmin = rmin;
    new->rmax = rmax;
    new->gmin = gmin;
    new->gmax = gmax;
    new->bmin = bmin;
    new->bmax = bmax;

    switch (which) {
    case RED:
	new->rmax = i - 1;
	ptr->rmin = i;
	break;
    case GREEN:
	new->gmax = i - 1;
	ptr->gmin = i;
	break;
    case BLUE:
	new->bmax = i - 1;
	ptr->bmin = i;
	break;
    }

    shrinkbox(new);
    shrinkbox(ptr);
}


/****************************/
static void
shrinkbox(box)
    CBOX     *box;
/****************************/
{
    int      *histp, ir, ig, ib;
    int       rmin, rmax, gmin, gmax, bmin, bmax;

    rmin = box->rmin;
    rmax = box->rmax;
    gmin = box->gmin;
    gmax = box->gmax;
    bmin = box->bmin;
    bmax = box->bmax;

    if (rmax > rmin) {
	for (ir = rmin; ir <= rmax; ir++)
	    for (ig = gmin; ig <= gmax; ig++) {
		histp = &histogram[ir][ig][bmin];
		for (ib = bmin; ib <= bmax; ib++)
		    if (*histp++ != 0) {
			box->rmin = rmin = ir;
			goto have_rmin;
		    }
	    }

      have_rmin:
	if (rmax > rmin)
	    for (ir = rmax; ir >= rmin; --ir)
		for (ig = gmin; ig <= gmax; ig++) {
		    histp = &histogram[ir][ig][bmin];
		    for (ib = bmin; ib <= bmax; ib++)
			if (*histp++ != 0) {
			    box->rmax = rmax = ir;
			    goto have_rmax;
			}
		}
    }
  have_rmax:

    if (gmax > gmin) {
	for (ig = gmin; ig <= gmax; ig++)
	    for (ir = rmin; ir <= rmax; ir++) {
		histp = &histogram[ir][ig][bmin];
		for (ib = bmin; ib <= bmax; ib++)
		    if (*histp++ != 0) {
			box->gmin = gmin = ig;
			goto have_gmin;
		    }
	    }
      have_gmin:
	if (gmax > gmin)
	    for (ig = gmax; ig >= gmin; --ig)
		for (ir = rmin; ir <= rmax; ir++) {
		    histp = &histogram[ir][ig][bmin];
		    for (ib = bmin; ib <= bmax; ib++)
			if (*histp++ != 0) {
			    box->gmax = gmax = ig;
			    goto have_gmax;
			}
		}
    }
  have_gmax:

    if (bmax > bmin) {
	for (ib = bmin; ib <= bmax; ib++)
	    for (ir = rmin; ir <= rmax; ir++) {
		histp = &histogram[ir][gmin][ib];
		for (ig = gmin; ig <= gmax; ig++) {
		    if (*histp != 0) {
			box->bmin = bmin = ib;
			goto have_bmin;
		    }
		    histp += B_LEN;
		}
	    }
      have_bmin:
	if (bmax > bmin)
	    for (ib = bmax; ib >= bmin; --ib)
		for (ir = rmin; ir <= rmax; ir++) {
		    histp = &histogram[ir][gmin][ib];
		    for (ig = gmin; ig <= gmax; ig++) {
			if (*histp != 0) {
			    bmax = ib;
			    goto have_bmax;
			}
			histp += B_LEN;
		    }
		}
    }
  have_bmax:return;
}



/*******************************/
static void
assign_color(ptr, rp, gp, bp)
    CBOX     *ptr;
    byte     *rp, *gp, *bp;
/*******************************/
{
    /* +1 ensures that color represents the middle of the box */
    *rp = ((ptr->rmin + ptr->rmax + 1) << (COLOR_DEPTH - B_DEPTH)) / 2;
    *gp = ((ptr->gmin + ptr->gmax + 1) << (COLOR_DEPTH - B_DEPTH)) / 2;
    *bp = ((ptr->bmin + ptr->bmax + 1) << (COLOR_DEPTH - B_DEPTH)) / 2;
}



/*******************************/
static CCELL *
create_colorcell(r1, g1, b1)
    int       r1, g1, b1;
/*******************************/
{
    register int i, tmp, dist;
    register CCELL *ptr;
    register byte *rp, *gp, *bp;
    int       ir, ig, ib, mindist;

    ir = r1 >> (COLOR_DEPTH - C_DEPTH);
    ig = g1 >> (COLOR_DEPTH - C_DEPTH);
    ib = b1 >> (COLOR_DEPTH - C_DEPTH);

    r1 &= ~1 << (COLOR_DEPTH - C_DEPTH);
    g1 &= ~1 << (COLOR_DEPTH - C_DEPTH);
    b1 &= ~1 << (COLOR_DEPTH - C_DEPTH);

    ptr = (CCELL *) malloc(sizeof(CCELL));
    *(ColorCells + ir * C_LEN * C_LEN + ig * C_LEN + ib) = ptr;
    ptr->num_ents = 0;

    /*
     * step 1: find all colors inside this cell, while we're at it, find
     * distance of centermost point to furthest corner
     */

    mindist = 99999999;

    rp = r;
    gp = g;
    bp = b;
    for (i = 0; i < num_colors; i++, rp++, gp++, bp++)
	if (*rp >> (COLOR_DEPTH - C_DEPTH) == ir &&
	    *gp >> (COLOR_DEPTH - C_DEPTH) == ig &&
	    *bp >> (COLOR_DEPTH - C_DEPTH) == ib) {

	    ptr->entries[ptr->num_ents][0] = i;
	    ptr->entries[ptr->num_ents][1] = 0;
	    ++ptr->num_ents;

	    tmp = *rp - r1;
	    if (tmp < (MAX_COLOR / C_LEN / 2))
		tmp = MAX_COLOR / C_LEN - 1 - tmp;
	    dist = tmp * tmp;

	    tmp = *gp - g1;
	    if (tmp < (MAX_COLOR / C_LEN / 2))
		tmp = MAX_COLOR / C_LEN - 1 - tmp;
	    dist += tmp * tmp;

	    tmp = *bp - b1;
	    if (tmp < (MAX_COLOR / C_LEN / 2))
		tmp = MAX_COLOR / C_LEN - 1 - tmp;
	    dist += tmp * tmp;

	    if (dist < mindist)
		mindist = dist;
	}
    /* step 3: find all points within that distance to box */

    rp = r;
    gp = g;
    bp = b;
    for (i = 0; i < num_colors; i++, rp++, gp++, bp++)
	if (*rp >> (COLOR_DEPTH - C_DEPTH) != ir ||
	    *gp >> (COLOR_DEPTH - C_DEPTH) != ig ||
	    *bp >> (COLOR_DEPTH - C_DEPTH) != ib) {

	    dist = 0;

	    if ((tmp = r1 - *rp) > 0 || (tmp = *rp - (r1 + MAX_COLOR / C_LEN - 1)) > 0)
		dist += tmp * tmp;

	    if ((tmp = g1 - *gp) > 0 || (tmp = *gp - (g1 + MAX_COLOR / C_LEN - 1)) > 0)
		dist += tmp * tmp;

	    if ((tmp = b1 - *bp) > 0 || (tmp = *bp - (b1 + MAX_COLOR / C_LEN - 1)) > 0)
		dist += tmp * tmp;

	    if (dist < mindist) {
		ptr->entries[ptr->num_ents][0] = i;
		ptr->entries[ptr->num_ents][1] = dist;
		++ptr->num_ents;
	    }
	}
    /* sort color cells by distance, use cheap exchange sort */
    {
	int       n, next_n;

	n = ptr->num_ents - 1;
	while (n > 0) {
	    next_n = 0;
	    for (i = 0; i < n; ++i) {
		if (ptr->entries[i][1] > ptr->entries[i + 1][1]) {
		    tmp = ptr->entries[i][0];
		    ptr->entries[i][0] = ptr->entries[i + 1][0];
		    ptr->entries[i + 1][0] = tmp;
		    tmp = ptr->entries[i][1];
		    ptr->entries[i][1] = ptr->entries[i + 1][1];
		    ptr->entries[i + 1][1] = tmp;
		    next_n = i;
		}
	    }
	    n = next_n;
	}
    }
    return (ptr);
}




/***************************/
static void
map_colortable()
/***************************/
{
    int       ir, ig, ib, *histp;
    CCELL    *cell;

    histp = &histogram[0][0][0];
    for (ir = 0; ir < B_LEN; ir++)
	for (ig = 0; ig < B_LEN; ig++)
	    for (ib = 0; ib < B_LEN; ib++) {
		if (*histp == 0)
		    *histp = -1;
		else {
		    int       i, j, tmp, d2, dist;

		    cell = *(ColorCells +
			     (((ir >> (B_DEPTH - C_DEPTH)) << C_DEPTH * 2)
			      + ((ig >> (B_DEPTH - C_DEPTH)) << C_DEPTH)
			      + (ib >> (B_DEPTH - C_DEPTH))));

		    if (cell == NULL)
			cell = create_colorcell(ir << (COLOR_DEPTH - B_DEPTH),
					      ig << (COLOR_DEPTH - B_DEPTH),
					     ib << (COLOR_DEPTH - B_DEPTH));

		    dist = 9999999;
		    for (i = 0; i < cell->num_ents && dist > cell->entries[i][1]; i++) {
			j = cell->entries[i][0];
			d2 = r[j] - (ir << (COLOR_DEPTH - B_DEPTH));
			d2 *= d2;
			tmp = g[j] - (ig << (COLOR_DEPTH - B_DEPTH));
			d2 += tmp * tmp;
			tmp = b[j] - (ib << (COLOR_DEPTH - B_DEPTH));
			d2 += tmp * tmp;
			if (d2 < dist) {
			    dist = d2;
			    *histp = j;
			}
		    }
		}
		histp++;
	    }
}



/*****************************/
static int
quant_fsdither()
/*****************************/
{
    register int *thisptr, *nextptr;
    int      *thisline, *nextline, *tmpptr;
    int       r1, g1, b1, r2, g2, b2;
    int       i, j, imax, jmax, oval;
    byte     *inptr, *outptr;
    int       lastline, lastpixel;

    imax = HIGH - 1;
    jmax = WIDE - 1;

    thisline = (int *) malloc(WIDE * 3 * sizeof(int));
    nextline = (int *) malloc(WIDE * 3 * sizeof(int));

    if (thisline == NULL || nextline == NULL) {
	fprintf(stderr, "%s: unable to allocate stuff for the 'dither' routine\n",
		myname);
	return 1;
    }
    inptr = (byte *) pic24;
    outptr = (byte *) pic;

    /* get first line of picture */
    for (j = WIDE * 3, tmpptr = nextline; j; j--)
	*tmpptr++ = (int) *inptr++;

    for (i = 0; i < HIGH; i++) {
	/* swap thisline and nextline */
	tmpptr = thisline;
	thisline = nextline;
	nextline = tmpptr;
	lastline = (i == imax);

	/* read in next line */
	if (!lastline)
	    for (j = WIDE * 3, tmpptr = nextline; j; j--)
		*tmpptr++ = (int) *inptr++;

	/* dither this line and put it into the output picture */
	thisptr = thisline;
	nextptr = nextline;

	for (j = 0; j < WIDE; j++) {
	    lastpixel = (j == jmax);

	    r2 = *thisptr++;
	    g2 = *thisptr++;
	    b2 = *thisptr++;

	    if (r2 < 0)
		r2 = 0;
	    else if (r2 >= MAX_COLOR)
		r2 = MAX_COLOR - 1;
	    if (g2 < 0)
		g2 = 0;
	    else if (g2 >= MAX_COLOR)
		g2 = MAX_COLOR - 1;
	    if (b2 < 0)
		b2 = 0;
	    else if (b2 >= MAX_COLOR)
		b2 = MAX_COLOR - 1;

	    r1 = r2;
	    g1 = g2;
	    b1 = b2;

	    r2 >> = (COLOR_DEPTH - B_DEPTH);
	    g2 >> = (COLOR_DEPTH - B_DEPTH);
	    b2 >> = (COLOR_DEPTH - B_DEPTH);

	    if ((oval = histogram[r2][g2][b2]) == -1) {
		int       ci, cj, dist, d2, tmp;
		CCELL    *cell;

		cell = *(ColorCells +
			 (((r2 >> (B_DEPTH - C_DEPTH)) << C_DEPTH * 2)
			  + ((g2 >> (B_DEPTH - C_DEPTH)) << C_DEPTH)
			  + (b2 >> (B_DEPTH - C_DEPTH))));

		if (cell == NULL)
		    cell = create_colorcell(r1, g1, b1);

		dist = 9999999;
		for (ci = 0; ci < cell->num_ents && dist > cell->entries[ci][1]; ci++) {
		    cj = cell->entries[ci][0];
		    d2 = (r[cj] >> (COLOR_DEPTH - B_DEPTH)) - r2;
		    d2 *= d2;
		    tmp = (g[cj] >> (COLOR_DEPTH - B_DEPTH)) - g2;
		    d2 += tmp * tmp;
		    tmp = (b[cj] >> (COLOR_DEPTH - B_DEPTH)) - b2;
		    d2 += tmp * tmp;
		    if (d2 < dist) {
			dist = d2;
			oval = cj;
		    }
		}
		histogram[r2][g2][b2] = oval;
	    }
	    *outptr++ = oval;

	    r1 -= r[oval];
	    g1 -= g[oval];
	    b1 -= b[oval];

	    r1 += 256;		/* make positive for table indexing */
	    g1 += 256;
	    b1 += 256;

	    if (!lastpixel) {
		thisptr[0] += tbl7[r1];
		thisptr[1] += tbl7[g1];
		thisptr[2] += tbl7[b1];
	    }
	    if (!lastline) {
		if (j) {
		    nextptr[-3] += tbl3[r1];
		    nextptr[-2] += tbl3[g1];
		    nextptr[-1] += tbl3[b1];
		}
		nextptr[0] += tbl5[r1];
		nextptr[1] += tbl5[g1];
		nextptr[2] += tbl5[b1];

		if (!lastpixel) {
		    nextptr[3] += tbl1[r1];
		    nextptr[4] += tbl1[g1];
		    nextptr[5] += tbl1[b1];
		}
		nextptr += 3;
	    }
	}
    }
    free(thisline);
    free(nextline);
    return 0;
}





/************************************/
static int
Quick24to8(p24, w, h)
    byte     *p24;
    int       w, h;
{

    /*
     * floyd-steinberg dithering.
     * 
     * ----   x    7/16 3/16  5/16  1/16
     * 
     */

    /*
     * called after 'pic' has been alloced, pWIDE,pHIGH set up, mono/1-bit
     * checked already
     */

    byte     *pp;
    int       r1, g1, b1;
    int      *thisline, *nextline, *thisptr, *nextptr, *tmpptr;
    int       i, j, val, pwide3;
    int       imax, jmax;

    pp = pic;
    pwide3 = w * 3;
    imax = h - 1;
    jmax = w - 1;

    /* up to 256 colors:     3 bits R, 3 bits G, 2 bits B  (RRRGGGBB) */
#define RMASK 0xe0
#define R_SHIFT        0
#define GMASK 0xe0
#define G_SHIFT        3
#define BMASK 0xc0
#define B_SHIFT        6

    /* load up colormap */
    /* note that 0 and 255 of each color are always in the map; */
    /* intermediate values are evenly spaced. */
    for (i = 0; i < 256; i++) {
	r[i] = (((i << R_SHIFT) & RMASK) * 255 + RMASK / 2) / RMASK;
	g[i] = (((i << G_SHIFT) & GMASK) * 255 + GMASK / 2) / GMASK;
	b[i] = (((i << B_SHIFT) & BMASK) * 255 + BMASK / 2) / BMASK;
    }


    thisline = (int *) malloc(pwide3 * sizeof(int));
    nextline = (int *) malloc(pwide3 * sizeof(int));
    if (!thisline || !nextline) {
	fprintf(stderr, "%s: unable to allocate memory in Quick24to8()\n", myname);
	return (1);
    }
    /* get first line of picture */
    for (j = pwide3, tmpptr = nextline; j; j--)
	*tmpptr++ = (int) *p24++;

    for (i = 0; i < h; i++) {
	tmpptr = thisline;
	thisline = nextline;
	nextline = tmpptr;	/* swap */

	if (i != imax)		/* get next line */
	    for (j = pwide3, tmpptr = nextline; j; j--)
		*tmpptr++ = (int) *p24++;

	for (j = 0, thisptr = thisline, nextptr = nextline; j < w; j++, pp++) {
	    r1 = *thisptr++;
	    g1 = *thisptr++;
	    b1 = *thisptr++;
	    RANGE(r1, 0, 255);
	    RANGE(g1, 0, 255);
	    RANGE(b1, 0, 255);

	    /* choose actual pixel value */
	    val = (((r1 & RMASK) >> R_SHIFT) | ((g1 & GMASK) >> G_SHIFT) |
		   ((b1 & BMASK) >> B_SHIFT));
	    *pp = val;

	    /* compute color errors */
	    r1 -= r[val];
	    g1 -= g[val];
	    b1 -= b[val];

	    /* Add fractions of errors to adjacent pixels */
	    r1 += 256;		/* make positive for table indexing */
	    g1 += 256;
	    b1 += 256;

	    if (j != jmax) {	/* adjust RIGHT pixel */
		thisptr[0] += tbl7[r1];
		thisptr[1] += tbl7[g1];
		thisptr[2] += tbl7[b1];
	    }
	    if (i != imax) {	/* do BOTTOM pixel */
		nextptr[0] += tbl5[r1];
		nextptr[1] += tbl5[g1];
		nextptr[2] += tbl5[b1];

		if (j > 0) {	/* do BOTTOM LEFT pixel */
		    nextptr[-3] += tbl3[r1];
		    nextptr[-2] += tbl3[g1];
		    nextptr[-1] += tbl3[b1];
		}
		if (j != jmax) {/* do BOTTOM RIGHT pixel */
		    nextptr[3] += tbl1[r1];
		    nextptr[4] += tbl1[g1];
		    nextptr[5] += tbl1[b1];
		}
		nextptr += 3;
	    }
	}
    }

    return 0;
}



/****************************/
static int
QuickCheck(pic24, w, h, maxcol)
    byte     *pic24;
    int       w, h, maxcol;
{
    /*
     * scans picture until it finds more than 'maxcol' different colors.  If
     * it finds more than 'maxcol' colors, it returns '0'.  If it DOESN'T, it
     * does the 24-to-8 conversion by simply sticking the colors it found
     * into a colormap, and changing instances of a color in pic24 into
     * colormap indicies (in pic)
     */

    unsigned long colors[256], col;
    int       i, nc, low, high, mid;
    byte     *p, *pix;

    if (maxcol > 256)
	maxcol = 256;

    /* put the first color in the table by hand */
    nc = 0;
    mid = 0;

    for (i = w * h, p = pic24; i; i--) {
	col = (((u_long) * p++) << 16);
	col += (((u_long) * p++) << 8);
	col += *p++;

	/* binary search the 'colors' array to see if it's in there */
	low = 0;
	high = nc - 1;
	while (low <= high) {
	    mid = (low + high) / 2;
	    if (col < colors[mid])
		high = mid - 1;
	    else if (col > colors[mid])
		low = mid + 1;
	    else
		break;
	}

	if (high < low) {	/* didn't find color in list, add it. */
	    /*
	     * WARNING: this is an overlapped memory copy.  memcpy doesn't do
	     * it correctly, hence 'bcopy', which claims to
	     */
	    if (nc >= maxcol)
		return 0;
	    bcopy(&colors[low], &colors[low + 1], (nc - low) * sizeof(unsigned long));
	    colors[low] = col;
	    nc++;
	}
    }


    /*
     * run through the data a second time, this time mapping pixel values in
     * pic24 into colormap offsets into 'colors'
     */

    for (i = w * h, p = pic24, pix = pic; i; i--, pix++) {
	col = (((u_long) * p++) << 16);
	col += (((u_long) * p++) << 8);
	col += *p++;

	/* binary search the 'colors' array.  It *IS* in there */
	low = 0;
	high = nc - 1;
	while (low <= high) {
	    mid = (low + high) / 2;
	    if (col < colors[mid])
		high = mid - 1;
	    else if (col > colors[mid])
		low = mid + 1;
	    else
		break;
	}

	if (high < low) {
	    fprintf(stderr, "QuickCheck:  impossible!\n");
	    exit(1);
	}
	*pix = mid;
    }

    /* and load up the 'desired colormap' */
    for (i = 0; i < nc; i++) {
	r[i] = colors[i] >> 16;
	g[i] = (colors[i] >> 8) & 0xff;
	b[i] = colors[i] & 0xff;
    }

    return 1;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/

/*
 *   to_8.c   color quantizing routines
 *
 *   written by Guojun Jin, LBL
 */


#define      cmap_t  byte
#define      MaxColors       256
#define	RED	0
#define	GREEN	1
#define	BLUE	2
#define truncate(a, b, c)	{ if (a<b) a=b;  else if (a>c) a=c; }

cmap_t   *reg_cmap[3];

static byte fsb_table[4][MaxColors];

/*********************************************************/
int
To_8(image, outbuf, width, height, max_colors, r, g, b)
    byte     *image, *outbuf;
    int       width, height, max_colors;
    cmap_t   *r, *b, *g;
{
    int       quant, i;

    if (reg_cmap[0] && pointer_buffer_size(reg_cmap[0]) < MaxColors * 3)
	free(reg_cmap[0]), reg_cmap[0] = NULL;
    if (reg_cmap[0] == NULL)
	reg_cmap[0] = (cmap_t *) calloc(MaxColors * 3, sizeof(cmap_t));
    reg_cmap[1] = reg_cmap[0] + MaxColors,
	reg_cmap[2] = reg_cmap[1] + MaxColors;

    quant = TrueCheck(image, outbuf, width * height, max_colors);
    if (quant < 0) {
	quant = dither_to8(image, outbuf, width, height);
	fprintf(stderr, "dithering required, %d colors in resulting color map \n", quant);
    } else {
	fprintf(stderr, "no dithering required, %d colors in image \n", quant);
    }

    for (i = 0; i < MaxColors; i++) {
	r[i] = reg_cmap[RED][i];
	g[i] = reg_cmap[GREEN][i];
	b[i] = reg_cmap[BLUE][i];
    }

    return quant;
}

/*********************************************************/
void
init_FS_tables()
{				/* initialize Floyd-Steinberg tables */
    register int i = MaxColors;
    while (i--) {
	fsb_table[0][i] = i >> 4;
	fsb_table[1][i] = 3 * i >> 4;
	fsb_table[2][i] = 5 * i >> 4;
	fsb_table[3][i] = 7 * i >> 4;
    }
}


/*********************************************************/
static
dither_to8(rgbp, obuf, w, h)
    byte     *rgbp, *obuf;
    int       w, h;
{
    int       r1, g1, b1;
    int      *thisline, *nextline, *thisptr, *nextptr, *tmpptr;
    int       i, j, rerr, gerr, berr, rgb_width = w * 3;
    int       imax = h - 1, jmax = w - 1;
    byte     *pp = obuf;

    for (i = 0; i < MaxColors; i++) {	/* build up colormap (RRRGGGBB)	 */
	reg_cmap[RED][i] = ((i & 0xe0) * 255) / 0xe0;
	reg_cmap[GREEN][i] = ((i & 0x1c) * 255) / 0x1c;
	reg_cmap[BLUE][i] = ((i & 0x03) * 255) / 0x03;
    }
    /*
     * floyd-steinberg dithering.
     * 
     * ----   x    7/16 3/16  5/16  1/16
     */
    init_FS_tables();

    thisline = (int *) calloc(rgb_width, sizeof(int));
    nextline = (int *) calloc(rgb_width, sizeof(int));

    /* get first line of picture */
    for (j = rgb_width, tmpptr = nextline; j--;)
	*tmpptr++ = *rgbp++;

    for (i = 0; i < h; i++) {
	tmpptr = thisline;
	thisline = nextline;
	nextline = tmpptr;

	if (i != imax)		/* get next line */
	    for (j = rgb_width, tmpptr = nextline; j--;)
		*tmpptr++ = *rgbp++;

	for (j = 0, thisptr = thisline, nextptr = nextline; j < w; j++, pp++) {
	    r1 = *thisptr++;
	    g1 = *thisptr++;
	    b1 = *thisptr++;
	    truncate(r1, 0, 255);
	    truncate(g1, 0, 255);
	    truncate(b1, 0, 255);

	    rerr = r1 & 0x1F;
	    gerr = g1 & 0x1F;
	    berr = b1 & 0x3F;
	    *pp = r1 & 0xE0 | (g1 >> 3) & 0x1C | (b1 >> 6);

	    if (j != jmax) {	/* adjust RIGHT pixel */
		thisptr[0] += fsb_table[3][rerr];
		thisptr[1] += fsb_table[3][gerr];
		thisptr[2] += fsb_table[3][berr];
	    }
	    if (i != imax) {	/* do BOTTOM pixel */
		nextptr[0] += fsb_table[2][rerr];
		nextptr[1] += fsb_table[2][gerr];
		nextptr[2] += fsb_table[2][berr];

		if (j > 0) {	/* do BOTTOM LEFT pixel */
		    nextptr[-3] += fsb_table[1][rerr];
		    nextptr[-2] += fsb_table[1][gerr];
		    nextptr[-1] += fsb_table[1][berr];
		}
		if (j != jmax) {/* do BOTTOM RIGHT pixel */
		    nextptr[3] += fsb_table[0][rerr];
		    nextptr[4] += fsb_table[0][gerr];
		    nextptr[5] += fsb_table[0][berr];
		}
		nextptr += 3;
	    }
	}
    }
    return MaxColors;
}

/*********************************************************/
static
TrueCheck(rgbp, obuf, fsize, maxcol)
    byte     *rgbp, *obuf;
    int       fsize, maxcol;
{
    long      colors[MaxColors], col;
    int       i, nc, low, high, mid;
    byte     *p, *pix;

    if (maxcol > MaxColors)
	maxcol = MaxColors;

    for (nc = mid = 0, i = fsize, p = rgbp; i--;) {
	col = *p++ << 16;
	col += *p++ << 8;
	col += *p++;

	/* binary search the 'colors' array to see if it's in there */
	low = 0;
	high = nc - 1;
	while (low <= high) {
	    mid = (low + high) >> 1;
	    if (col < colors[mid])
		high = mid - 1;
	    else if (col > colors[mid])
		low = mid + 1;
	    else
		break;
	}

	if (high < low) {	/* if not in list, add it. */
	    if (nc >= maxcol)
		return -1;	/* quantizing will be required ! */

	    /*
	     * memcpy doesn't do overlapped copy correctly
	     */
	    bcopy(&colors[low], &colors[low + 1],
		  (nc - low) * sizeof(unsigned long));
	    colors[low] = col;
	    nc++;
	}
    }

    /* run through the data a 2nd time, & convert to 8	 */
    for (i = fsize, p = rgbp, pix = obuf; i--; pix++) {
	col = *p++ << 16;
	col += *p++ << 8;
	col += *p++;

	low = 0;
	high = nc - 1;
	while (low <= high) {
	    mid = (low + high) >> 1;
	    if (col < colors[mid])
		high = mid - 1;
	    else if (col > colors[mid])
		low = mid + 1;
	    else
		break;
	}
	if (high < low)
	    fprintf(stderr, "TrueColor Check: double entry\n");
	*pix = mid;		/* set output */
    }

    for (i = nc; i--;) {	/* grab colormap */
	reg_cmap[RED][i] = (colors[i] >> 16) & 0xFF;
	reg_cmap[GREEN][i] = (colors[i] >> 8) & 0xFF;
	reg_cmap[BLUE][i] = colors[i] & 0xFF;
    }
    return nc;			/* quantizing not needed */
}

/***********************************************************/
pointer_buffer_size(p)		/* return the block size allocated for
				 * pointer *p */
    byte     *p;
{
    return *((int *) p - 2) - (sizeof(int) << 1);
}
