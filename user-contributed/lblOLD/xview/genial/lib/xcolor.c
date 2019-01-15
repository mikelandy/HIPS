
/*
 * xcolor.c - color allocation/sorting/freeing code
 *
 *  Author:    John Bradley, University of Pennsylvania
 *                (bradley@cis.upenn.edu)
 *
 *  modified for use at LBL by Brian Tierney,  3/92
 *
 *  Contains:
 *     void   SortColormap()
 *     void   AllocColors()
 *     Status xvAllocColor()
 *     void   AllocRWColors()
 *     void   xvFreeColors()
 *     void   FreeAllColors()
 */


/*
 * Copyright 1989, 1990, 1991, 1992 by John Bradley and
 *                       The University of Pennsylvania
 *
 * Permission to use, copy, and distribute for non-commercial purposes,
 * is hereby granted without fee, providing that the above copyright
 * notice appear in all copies and that both the copyright notice and this
 * permission notice appear in supporting documentation.
 *
 * The software may be modified for your own purposes, but modified versions
 * may not be distributed.
 *
 * This software is provided "as is" without any expressed or implied warranty.
 *
 * The author may be contacted via:
 *    US Mail:   John Bradley
 *               GRASP Lab, Room 301C
 *               3401 Walnut St.
 *               Philadelphia, PA  19104
 *
 *    Phone:     (215) 898-8813
 *    EMail:     bradley@cis.upenn.edu
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef unsigned char byte;
static char     *myname;		/* program name for irintf's */

/* MONO returns total intensity of r,g,b components */
#define MONO(rd,gn,bl) (((rd)*11 + (gn)*16 + (bl)*5) >> 5)	/* .33R+ .5G+ .17B */

/* X stuff */
static Display *display = NULL;
static Visual *winv = NULL;
static Window mainW;
static Colormap  default_cmap, LocalCmap;
static Colormap  cmap;			/* currently included colormap */

static int DEBUG = 0;		/* print debugging info */

static byte colAllocOrder[256];	/* order to allocate cols */
static unsigned long freecols[256];	/* list of pixel values to free */
static unsigned long cols[256];	/* maps pic pixel values to X pixel vals */
static byte r[256], g[256], b[256];	/* incoming colormap */
static byte      rdisp[256], gdisp[256], bdisp[256];	/* DISPLAYED colors */
static byte save_red[256], save_green[256], save_blue[256];
static int fc2pcol[256];	/* maps freecols into pic pixel values */
static byte rwpc2pc[256];	/* mapping of shared pixels in -rw mode */
static int numcolors;		/* # of desired colors in picture */
static int nfcols;		/* number of colors to free */
static int VERBOSE;		/* print status message flag */

static byte     *image_sorted;		/* original data look-up table */

int       SortColormap();
void      AllocColors(), AllocRWColors();
Status    xvAllocColor();
void      xvFreeColors();
void      FreeAllColors();


/********************************************************************
 * arguments:
 *  disp:        X-windows display id (Display)
 *  winvis:      pointer to X-windows visual structure (Visual *)
 *  mainw:       X-window ID of main window  (Window)
 *  image_map:   input: buffer containing colormap (red,green,blue array)
 *                      look-up values
 *               output: X-colormap lookup values
 *  image_size:  size of map_lut buffer
 *  red, green, blue:  red, green, and blue colormap arrays
 *  ncells:      size of r,g,b arrays
 *  noglob:      flag: dont share colors with other applications, only use
 *               colors this program successfully allocates
 *  perfect:     flag: create all colors requested (this will change the colormap
 *               for other applications, causing color 'flash' problem)
 *  ncols:       maximum number of colors to try to allocate (for example: if
 *                your window manager is using 6 colors, set this to 248 or less)
 *  mono:        flag: assume grayscale monitor and convert all colors
 *               to gray values
 *  use_all:     flag: use all colors in colormap, even if not in the image. This
 *               is useful in creating a colormap for a sequence of images, where
 *               all colors my not be in the 1st frame
 *  rwcolor:     flag: create read/write colormap: this means xv will not
 *               share colors with other applications, but colormap changes
 *               like gamma correction will happen a little faster
 *  verbose:     flag: 0: silient, 1: print important status messages,
 *               2: print all status messages
 *  my_name:     name of this program to use in status/error messages
 *************************************************************************
 * return value:  map of image pixel values to X pixel vals (cols)
 *    (most program will not need to use this: only if you need to
 *     write your own custom 'map_image' routine)
 */

u_long   *
build_colormap(disp, winvis, mainw, image_map, image_size,
	       red, green, blue, ncells, noglob, perfect,
	       ncols, mono, use_all, rwcolor, verbose, my_name)
    Display  *disp;
    Visual   *winvis;
    Window    mainw;
    byte     *image_map;
    byte     *red, *green, *blue;
    int       image_size, ncells, noglob, perfect, ncols;
    int       rwcolor, mono, use_all, verbose;
    char     *my_name;
{
    int       i;

    display = disp;
    winv = winvis;
    myname = my_name;
    mainW = mainw;
    VERBOSE = verbose;

    if (ncells <= 0 || ncells > 256) {
	fprintf(stderr, "build_colormap error: ncells not between 0 and 255 (%d)\n", ncells);
	return (NULL);
    }
    for (i = 0; i < 256; i++) {
	r[i] = red[i];
	g[i] = green[i];
	b[i] = blue[i];
    }

    image_sorted = (byte *) malloc(image_size);

    default_cmap = DefaultColormap(display, DefaultScreen(display));

    /* sort incoming colormap */
    numcolors = SortColormap(image_map, image_size, ncols, use_all);
    if (use_all)		/* in use_all mode numcolors returned not
				 * accurate */
	numcolors = ncols;

    if (VERBOSE > 0)
	fprintf(stderr, "Image contains %d different colors \n", numcolors);

    /* make copy of sorted colormap (for gamma correction routine) */
    for (i = 0; i < 256; i++) {
	save_red[i] = r[i];
	save_green[i] = g[i];
	save_blue[i] = b[i];
    }

    if (VERBOSE > 1 && perfect)
	fprintf(stderr, "Attempting to create 'perfect' colormap (FLASH warning!) \n");
    if (rwcolor) {
	if (VERBOSE > 1)
	    fprintf(stderr, "Creating read/write colormap... \n");
	AllocRWColors(noglob, ncells, perfect, mono, ncols);
    } else {
	if (VERBOSE > 1)
	    fprintf(stderr, "Creating read-only colormap... \n");
	AllocColors(noglob, perfect, mono, ncells, ncols);
    }

    /* make copy of this original sorted map (for gamma correction) */
    memcpy((char *) image_sorted, (char *) image_map, image_size);

    return (cols);
}

/************************************************/
/* map image to color indexes */
map_image_to_colors(mapped_image, image, size)	
    byte     *mapped_image, *image;
    int       size;
{
    register int i, j;
    byte     *mbuf;

    if (winv->class == TrueColor) {
	/*
	 * with 24-bit systems, make sure user is not trying to map to the
	 * same buffer
	 */
	if (mapped_image == image) {
	    mbuf = (byte *) malloc(size);
	    memcpy((char *) mbuf, (char *) image, size);
	    image = mbuf;
	}
    }
    for (i = 0, j = 0; i < size; i++) {
	if (winv->class == TrueColor) {	/* 24-bit systems */
	    mapped_image[j++] = 0;
	    mapped_image[j++] = (cols[image[i]] >> 16) & 0xff;
	    mapped_image[j++] = (cols[image[i]] >> 8) & 0xff;
	    mapped_image[j++] = cols[image[i]] & 0xff;
	} else {
	    mapped_image[i] = (byte) cols[image[i]];
	}
    }
    if (winv->class == TrueColor) {
	free(mbuf);
	image = mapped_image;
    }
}

/************************************************/
int
SortColormap(image, im_size, ncols, use_all)
    byte     *image;
    int       im_size, ncols, use_all;
 /*
  * this routines sets the globals r,b,g, colAllocOrder, and returns the
  * sorted image lut in image
  */
{
    typedef struct thing {
	byte      r, g, b;	/* actual value of color */
	int       oldindex;	/* its index in the old colormap */
	int       use;		/* # of pixels of this color */
	int       mindist;	/* min distance to a selected color */
    }         CMAPENT;

    byte     *p;
    int       numcols;		/* # of desired colors in picture */
    int       i, j, mdist, entry, d, hist[256], trans[256];
    static CMAPENT c[256], c1[256], *cp, *cj, *ck;

    /* init some stuff */
    for (i = 0; i < 256; i++)
	colAllocOrder[i] = i;

    /* initialize histogram and compute it */
    for (i = 0; i < 256; i++)
	hist[i] = 0;

    for (i = im_size, p = image; i; i--, p++)
	hist[*p]++;

    if (use_all) {		/* make sure all colors get mapped */
	for (i = 0; i < ncols; i++) {
	    if (hist[i] == 0)
		hist[i] = 10;
	}
    }

    if (DEBUG > 1) {
	fprintf(stderr, "%s: Desired colormap\n", myname);
	for (i = 0; i < 256; i++)
	    if (hist[i])
		fprintf(stderr, "(%3d  (%3d,%3d,%3d) cnt: %d)\n",
			i, r[i], g[i], b[i], hist[i]);
	fprintf(stderr, "\n\n");
    }
    /*
     * put the actually-used colors into the 'c' array in the order they
     * occur also, while we're at it, calculate numcols, and close up gaps in
     * colortable
     */

    for (i = numcols = 0; i < 256; i++) {
	if (hist[i]) {
	    r[numcols] = r[i];
	    g[numcols] = g[i];
	    b[numcols] = b[i];
	    trans[i] = numcols;

	    cp = &c[numcols];
	    cp->r = r[i];
	    cp->g = g[i];
	    cp->b = b[i];
	    cp->use = hist[i];
	    cp->oldindex = numcols;
	    cp->mindist = 100000;
	    numcols++;
	}
    }

    /*
     * modify 'pic' to reflect new (compressed, but not reordered) colormap
     */
    for (i = im_size, p = image; i; i--, p++) {
	j = trans[*p];
	*p = j;
    }

    /* find most-used color, put that in c1[0] */
    entry = -1;
    mdist = -1;
    for (i = 0; i < numcols; i++) {
	if (c[i].use > mdist) {
	    mdist = c[i].use;
	    entry = i;
	}
    }
    memcpy(&c1[0], &c[entry], sizeof(CMAPENT));
    c[entry].use = 0;		/* and mark it dealt with */


    /*
     * sort rest of colormap.  Half of the entries are allocated on the basis
     * of distance from already allocated colors, and half on the basis of
     * usage.  (NB: 'taxicab' distance is used throughout this file.)
     * 
     * Mod:  pick first 10 colors based on maximum distance.  pick remaining
     * colors half by distance and half by usage   -- JHB
     * 
     * To obtain O(n^2) performance, we keep each unselected color (in c[], with
     * use>0) marked with the minimum distance to any of the selected colors
     * (in c1[]).  Each time we select a color, we can update the minimum
     * distances in O(n) time.
     * 
     * mod by Tom Lane   Tom.Lane@g.gp.cs.cmu.edu
     */

    for (i = 1; i < numcols; i++) {
	/* First, update distances to the just-selected color */
	ck = &c1[i - 1];	/* point to just-selected color */
	for (j = 0, cj = c; j < numcols; j++, cj++) {
	    if (cj->use) {	/* can ignore already-selected colors */
		d = abs(cj->r - ck->r) + abs(cj->g - ck->g) + abs(cj->b - ck->b);
		if (cj->mindist > d)
		    cj->mindist = d;
	    }
	}

	if (i & 1 || i < 10) {
	    /* Now find the i'th most different color */
	    /*
	     * we want to select the unused color that has the greatest
	     * mindist
	     */
	    entry = -1;
	    mdist = -1;
	    for (j = 0, cj = c; j < numcols; j++, cj++) {
		if (cj->use) {	/* this color has not been marked already */
		    if (cj->mindist > mdist) {
			mdist = cj->mindist;
			entry = j;
		    }
		}
	    }
	} else {
	    /* Now find the i'th most different color */
	    /*
	     * we want to select the unused color that has the greatest usage
	     */
	    entry = -1;
	    mdist = -1;
	    for (j = 0, cj = c; j < numcols; j++, cj++) {
		if (cj->use) {	/* this color has not been marked already */
		    if (cj->use > mdist) {
			mdist = cj->use;
			entry = j;
		    }
		}
	    }
	}


	/* c[entry] is the next color to put in the map.  do so */
	memcpy(&c1[i], &c[entry], sizeof(CMAPENT));
	c[entry].use = 0;
    }


    for (i = 0; i < numcols; i++)
	colAllocOrder[i] = c1[i].oldindex;

    if (DEBUG > 1) {
	fprintf(stderr, "%s: result of sorting colormap\n", myname);
	for (i = 0; i < numcols; i++)
	    fprintf(stderr, "(%3d: %3d,%3d,%3d)     ", i, r[i], g[i], b[i]);
	fprintf(stderr, "\n\n");

	fprintf(stderr, "%s: allocation order table\n", myname);
	for (i = 0; i < numcols; i++)
	    fprintf(stderr, "colAllocOrder[%d] = -> %d\n", i, colAllocOrder[i]);
	fprintf(stderr, "\n");
    }
    return (numcols);
}


#define NOPIX 0xffffffff

/***********************************/
void
AllocColors(noglob, perfect, mono, ncells, ncols)
    int       noglob, perfect, mono, ncells, ncols;
{
    int       i, j, c, unique, p2alloc, p3alloc;
    Window    rootW;
    XColor    defs[256];
    XColor    ctab[256];
    int       dc;

    nfcols = unique = p2alloc = p3alloc = 0;

    /*
     * FIRST PASS COLOR ALLOCATION:  for each color in the 'desired
     * colormap', try to get it via xvAllocColor().  If for any reason it
     * fails, mark that pixel 'unallocated' and worry about it later. Repeat.
     */

    /*
     * attempt to allocate first ncols entries in colormap note: On displays
     * with less than 8 bits per RGB gun, it's quite possible that different
     * colors in the original picture will be mapped to the same color on the
     * screen.  X does this for you silently. However, this is not-desirable
     * for this application, because when I say 'allocate me 32 colors' I
     * want it to allocate 32 different colors, not 32 instances of the same
     * 4 shades...
     */

    for (i = 0; i < 256; i++)
	cols[i] = NOPIX;

    cmap = default_cmap;
    for (i = 0; i < numcolors && unique < ncols; i++) {
	c = colAllocOrder[i];
	if (mono) {
	    int       intens = MONO(r[c], g[c], b[c]);
	    defs[c].red = defs[c].green = defs[c].blue = intens << 8;
	} else {
	    defs[c].red = r[c] << 8;
	    defs[c].green = g[c] << 8;
	    defs[c].blue = b[c] << 8;
	}

	defs[c].flags = DoRed | DoGreen | DoBlue;

	if (xvAllocColor(display, cmap, &defs[c])) {
	    unsigned long pixel, *fcptr;

	    pixel = cols[c] = defs[c].pixel;
	    rdisp[c] = defs[c].red >> 8;
	    gdisp[c] = defs[c].green >> 8;
	    bdisp[c] = defs[c].blue >> 8;

	    /* see if the newly allocated color is new and different */
	    for (j = 0, fcptr = freecols; j < nfcols && *fcptr != pixel;
		 j++, fcptr++);
	    if (j == nfcols)
		unique++;

	    fc2pcol[nfcols] = c;
	    freecols[nfcols++] = pixel;
	} else {
	    /*
	     * the allocation failed.  If we want 'perfect' color, and we
	     * haven't already created our own colormap, we'll want to do so
	     */
	    if (perfect && !LocalCmap) {
		rootW = RootWindow(display, DefaultScreen(display));
		LocalCmap = XCreateColormap(display, rootW, winv, AllocNone);

		/*
		 * free all colors that were allocated, and try again with
		 * the new colormap.  This is necessary because
		 * 'XCopyColormapAndFree()' has the unpleasant side effect of
		 * freeing up the various colors I need for the control
		 * panel, etc.
		 */

		for (i = 0; i < nfcols; i++)
		    xvFreeColors(display, default_cmap, &freecols[i], 1, 0L);

		XSetWindowColormap(display, mainW, LocalCmap);

		cmap = LocalCmap;

		/* redo ALL allocation requests */
		for (i = 0; i < 256; i++)
		    cols[i] = NOPIX;
		nfcols = unique = 0;
		i = -1;
	    } else {
		/*
		 * either we don't care about perfect color, or we do care,
		 * have allocated our own colormap, and have STILL run out of
		 * colors (possible, even on an 8 bit display), just mark
		 * pixel as unallocated.  We'll deal with it later
		 */
		cols[c] = NOPIX;
	    }
	}
    }				/* FIRST PASS */



    if (nfcols == numcolors) {
	if (VERBOSE > 0) {
	    if (numcolors != unique)
		fprintf(stderr, "Got all %d desired colors.  (%d unique)\n",
			numcolors, unique);
	    else
		fprintf(stderr, "Got all %d desired colors.\n", numcolors);
	}
	return;
    }
    /*
     * SECOND PASS COLOR ALLOCATION: Allocating 'exact' colors failed. Now
     * try to allocate 'closest' colors.
     * 
     * Read entire X colormap (or first 256 entries) in from display. for each
     * unallocated pixel, find the closest color that actually is in the X
     * colormap.  Try to allocate that color (read only). If that fails, the
     * THIRD PASS will deal with it
     */

    if (VERBOSE > 0)
	fprintf(stderr, "Allocated %d out of %d colors. \n", nfcols, numcolors);


    /* read entire colormap (or first 256 entries) into 'ctab' */
    dc = (ncells < 256) ? ncells : 256;
    for (i = 0; i < dc; i++)
	ctab[i].pixel = (unsigned long) i;
    XQueryColors(display, cmap, ctab, dc);

    for (i = 0; i < numcolors && unique < ncols; i++) {
	c = colAllocOrder[i];

	if (cols[c] == NOPIX) {	/* an unallocated pixel */
	    int       d, mdist, close;
	    unsigned long ri, gi, bi;

	    mdist = 100000;
	    close = -1;
	    ri = r[c];
	    gi = g[c];
	    bi = b[c];

	    for (j = 0; j < dc; j++) {
		d = abs(ri - (ctab[j].red >> 8)) +
		    abs(gi - (ctab[j].green >> 8)) +
		    abs(bi - (ctab[j].blue >> 8));
		if (d < mdist) {
		    mdist = d;
		    close = j;
		}
	    }

	    if (close < 0) {
		fprintf(stderr, "Error: This Can't Happen! (How reassuring.)\n");
		exit(-1);
	    }
	    if (xvAllocColor(display, cmap, &ctab[close])) {
		memcpy(&defs[c], &ctab[close], sizeof(XColor));
		cols[c] = ctab[close].pixel;
		rdisp[c] = ctab[close].red >> 8;
		gdisp[c] = ctab[close].green >> 8;
		bdisp[c] = ctab[close].blue >> 8;
		fc2pcol[nfcols] = c;
		freecols[nfcols++] = cols[c];
		p2alloc++;
		unique++;
	    }
	}
    }



    /*
     * THIRD PASS COLOR ALLOCATION: We've alloc'ed all the colors we can.
     * Now, we have to map any remaining unalloced pixels into either A) the
     * colors that we DID get (noglob), or B) the colors found in the X
     * colormap
     */

    for (i = 0; i < numcolors; i++) {
	c = colAllocOrder[i];

	if (cols[c] == NOPIX) {	/* an unallocated pixel */
	    int       d, k, mdist, close;
	    unsigned long ri, gi, bi;

	    mdist = 100000;
	    close = -1;
	    ri = r[c];
	    gi = g[c];
	    bi = b[c];

	    if (!noglob) {	/* search the entire X colormap */
		for (j = 0; j < dc; j++) {
		    d = abs(ri - (ctab[j].red >> 8)) +
			abs(gi - (ctab[j].green >> 8)) +
			abs(bi - (ctab[j].blue >> 8));
		    if (d < mdist) {
			mdist = d;
			close = j;
		    }
		}
		if (close < 0) {
		    fprintf(stderr, "Error: This Can't Happen! (How reassuring.)\n");
		    exit(-1);
		}
		memcpy(&defs[c], &ctab[close], sizeof(XColor));
		cols[c] = defs[c].pixel;
		rdisp[c] = defs[c].red >> 8;
		gdisp[c] = defs[c].green >> 8;
		bdisp[c] = defs[c].blue >> 8;
		p3alloc++;
	    } else {		/* only search the alloc'd colors */
		for (j = 0; j < nfcols; j++) {
		    k = fc2pcol[j];
		    d = abs(ri - (defs[k].red >> 8)) +
			abs(gi - (defs[k].green >> 8)) +
			abs(bi - (defs[k].blue >> 8));
		    if (d < mdist) {
			mdist = d;
			close = k;
		    }
		}

		if (close < 0) {
		    fprintf(stderr, "Error: This Can't Happen! (How reassuring.)\n");
		    exit(-1);
		}
		memcpy(&defs[c], &defs[close], sizeof(XColor));
		cols[c] = defs[c].pixel;
		rdisp[c] = defs[c].red >> 8;
		gdisp[c] = defs[c].green >> 8;
		bdisp[c] = defs[c].blue >> 8;
	    }
	}
    }				/* THIRD PASS */



    if (VERBOSE > 0) {
	if (p2alloc && p3alloc)
	    fprintf(stderr, "Got %d 'close' color%s.  'Borrowed' %d color%s.",
		    p2alloc, (p2alloc > 1) ? "s" : "",
		    p3alloc, (p3alloc > 1) ? "s" : "");

	else if (p2alloc && !p3alloc)
	    fprintf(stderr, "Got %d 'close' color%s.",
		    p2alloc, (p2alloc > 1) ? "s" : "");

	else if (!p2alloc && p3alloc)
	    fprintf(stderr, "'Borrowed' %d color%s.",
		    p3alloc, (p3alloc > 1) ? "s" : "");
	fprintf(stderr, "\n\n");
    }
    return;
}

/***********************************/
void
AllocRWColors(noglob, ncells, perfect, mono, ncols)
    int       noglob, ncells, perfect, mono, ncols;
{
    int       i, j, c;
    Window    rootW;
    XColor    defs[256];

    nfcols = 0;

    cmap = default_cmap;

    for (i = 0; i < numcolors; i++)
	cols[colAllocOrder[i]] = NOPIX;

    for (i = 0; i < numcolors && i < ncols; i++) {
	unsigned long pmr[1], pix[1];
	c = colAllocOrder[i];

	if (XAllocColorCells(display, cmap, False, pmr, 0, pix, 1)) {

	    defs[c].pixel = cols[c] = pix[0];
	    if (mono) {
		int       intens = MONO(r[c], g[c], b[c]);
		defs[c].red = defs[c].green = defs[c].blue = intens << 8;
	    } else {
		defs[c].red = r[c] << 8;
		defs[c].green = g[c] << 8;
		defs[c].blue = b[c] << 8;
	    }

	    defs[c].flags = DoRed | DoGreen | DoBlue;
	    rdisp[c] = r[c];
	    gdisp[c] = g[c];
	    bdisp[c] = b[c];

	    fc2pcol[nfcols] = c;
	    rwpc2pc[c] = c;
	    freecols[nfcols++] = pix[0];
	} else {
	    if (perfect && !LocalCmap) {
		rootW = RootWindow(display, DefaultScreen(display));
		LocalCmap = XCreateColormap(display, rootW, winv, AllocNone);

		/*
		 * free all colors that were allocated, and try again with
		 * the new colormap.  This is necessary because
		 * 'XCopyColormapAndFree()' has the unpleasant side effect of
		 * freeing up the various colors I need for the control
		 * panel, etc.
		 */

		for (i = 0; i < nfcols; i++)
		    xvFreeColors(display, default_cmap, &freecols[i], 1, 0L);

		XSetWindowColormap(display, mainW, LocalCmap);
		cmap = LocalCmap;

		/* redo ALL allocation requests */
		for (i = 0; i < numcolors; i++)
		    cols[colAllocOrder[i]] = NOPIX;
		nfcols = 0;
		i = -1;
	    } else
		cols[c] = NOPIX;
	}
    }				/* for (i=0; ... */


    if (nfcols == numcolors)
	if (VERBOSE > 0)
	    fprintf(stderr, "Got all %d desired colors.\n", numcolors);

	else {
	    if (nfcols == 0) {
		fprintf(stderr, "No r/w cells available.  Using r/o color.\n");
		AllocColors(noglob, perfect, mono, ncells, ncols);
		return;
	    }
	    /*
	     * Failed to allocate all colors in picture.  Map remaining
	     * desired colors into closest allocated desired colors
	     */
	    if (VERBOSE > 0)
		fprintf(stderr, "Got %d out of %d colors.\n", nfcols, numcolors);

	    for (i = 0; i < numcolors; i++) {
		c = colAllocOrder[i];
		if (cols[c] == NOPIX) {	/* an unallocated pixel */
		    int       k, d, mdist, close;
		    unsigned long ri, gi, bi;

		    mdist = 100000;
		    close = -1;
		    ri = r[c];
		    gi = g[c];
		    bi = b[c];

		    for (j = 0; j < nfcols; j++) {
			k = fc2pcol[j];
			d = abs(ri - (defs[k].red >> 8)) + abs(gi - (defs[k].green >> 8)) +
			    abs(bi - (defs[k].blue >> 8));
			if (d < mdist) {
			    mdist = d;
			    close = k;
			}
		    }

		    if (close < 0) {
			fprintf(stderr, "Error: This Can't Happen! (How reassuring.)\n");
			exit(-1);
		    }
		    memcpy(&defs[c], &defs[close], sizeof(XColor));
		    cols[c] = defs[c].pixel;
		    rdisp[c] = defs[c].red >> 8;
		    gdisp[c] = defs[c].green >> 8;
		    bdisp[c] = defs[c].blue >> 8;
		    rwpc2pc[c] = close;
		}
	    }
	}

    /* load up the allocated colorcells */
    for (i = 0; i < nfcols; i++) {
	j = fc2pcol[i];
	defs[j].pixel = freecols[i];
	defs[j].red = r[j] << 8;
	defs[j].green = g[j] << 8;
	defs[j].blue = b[j] << 8;
	defs[j].flags = DoRed | DoGreen | DoBlue;
	XStoreColor(display, cmap, &defs[j]);
    }
    return;
}

/***********************************************/

void
xvFreeColors(dp, cm, pixels, npixels, planes)
    Display  *dp;
    Colormap  cm;
    unsigned long pixels[];
    int       npixels;
    unsigned long planes;
{
    if (winv->class != TrueColor)
	XFreeColors(dp, cm, pixels, npixels, planes);
}

/***********************************************/
void
FreeAllColors()
{
    /* Call this routine before exitting */
    int       i;

    if (display == NULL)
	return;

    if (LocalCmap) {
	XFreeColormap(display, LocalCmap);
	LocalCmap = 0;
    } else {
	for (i = 0; i < nfcols; i++)
	    xvFreeColors(display, default_cmap, &freecols[i], 1, 0L);

	XFlush(display);	/* just to make sure they're all freed right
				 * now... */
    }
}

/*********************************************************************/
static int
firstbit(ul)
    unsigned long ul;
{
    int       i;
    for (i = 0; ((ul & 1) == 0) && i < 32; i++, ul = ul >> 1);
    return i;
}

/*********************************************************************/
Status
xvAllocColor(dp, cm, cdef)
    Display  *dp;
    Colormap  cm;
    XColor   *cdef;
{
    if (winv->class == TrueColor) {	/* 24-bit systems */
	unsigned short bitmask;
	unsigned long r, g, b;
	int       rshift, gshift, bshift;

	/* set r,g,b = to relevant hi-order bits */
	bitmask = (1 << winv->bits_per_rgb) - 1;
	bitmask = bitmask << (16 - winv->bits_per_rgb);
	r = cdef->red & bitmask;
	g = cdef->green & bitmask;
	b = cdef->blue & bitmask;

	/* compute shifts */
	rshift = (16 - winv->bits_per_rgb) - firstbit(winv->red_mask);
	gshift = (16 - winv->bits_per_rgb) - firstbit(winv->green_mask);
	bshift = (16 - winv->bits_per_rgb) - firstbit(winv->blue_mask);
	/* shift the bits around */
	if (rshift < 0)
	    r = r << (-rshift);
	else
	    r = r >> rshift;

	if (gshift < 0)
	    g = g << (-gshift);
	else
	    g = g >> gshift;

	if (bshift < 0)
	    b = b << (-bshift);
	else
	    b = b >> bshift;

	cdef->pixel = r | g | b;
	return 1;
    } else {
	return (XAllocColor(dp, cm, cdef));
    }
}



/***************************************************************/
void
apply_gamma(gammaval, noglob, perfect, ncols, mono, rwcolor)
    float     gammaval;
    int       noglob, perfect, ncols, rwcolor, mono;
{
    int       i;
    void change_ro_colors();
    extern int get_gamma();

    /*
     * NOTE: you need to define your own 'get_gamma' routine somewhere. See
     * hview.c for an example
     */

    for (i = 0; i < 256; i++) {
	r[i] = get_gamma(save_red[i], gammaval);
	b[i] = get_gamma(save_green[i], gammaval);
	g[i] = get_gamma(save_blue[i], gammaval);
#ifdef DEBUG
	fprintf(stderr, "%d: (%d,%d,%d) mapped to (%d,%d,%d) \n",
	     i, save_red[i], save_green[i], save_blue[i], r[i], g[i], b[i]);
#endif
    }

    if (rwcolor) {
        change_ro_colors(mono);
	return;
    }
    FreeAllColors();
    AllocColors(noglob, perfect, mono, numcolors, ncols);

    return;
}

/***************************************************************/

void
change_ro_colors(mono)
    int       mono;
{
    int       i, j;
    XColor    ctab[256];

    for (i = 0; i < nfcols; i++) {
	j = fc2pcol[i];
	if (mono) {
	    int       intens = MONO(r[j], g[j], b[j]);
	    ctab[i].red = ctab[i].green = ctab[i].blue = intens << 8;
	} else {
	    ctab[i].red = r[j] << 8;
	    ctab[i].green = g[j] << 8;
	    ctab[i].blue = b[j] << 8;
	}

	ctab[i].pixel = freecols[i];
	ctab[i].flags = DoRed | DoGreen | DoBlue;
	XStoreColor(display, LocalCmap ? LocalCmap : default_cmap, &ctab[i]);
    }
    XStoreColor(display, LocalCmap ? LocalCmap : default_cmap, &ctab[0]);

    for (i = 0; i < 256; i++) {
	rdisp[colAllocOrder[i]] = r[rwpc2pc[i]];
	gdisp[colAllocOrder[i]] = g[rwpc2pc[i]];
	bdisp[colAllocOrder[i]] = b[rwpc2pc[i]];
    }

    return;
}

/***************************************************************/

show_colormap()
{				/* for debugging */
    XColor    color_table[256];
    int       i;

    for (i = 0; i < 256; i++) {
	color_table[i].pixel = (unsigned long) i;
    }

    XQueryColors(display, LocalCmap ? LocalCmap : default_cmap,
		 color_table, 256);	/* fills in color_table entries */

    for (i = 0; i < 256; i++) {
	fprintf(stderr, "%3d: (%3d, %3d, %3d) ", i,
		(int) (color_table[i].red >> 8),
		(int) (color_table[i].green >> 8),
		(int) (color_table[i].blue >> 8));
		i++;
	fprintf(stderr, "%3d: (%3d, %3d, %3d) ", i,
		(int) (color_table[i].red >> 8),
		(int) (color_table[i].green >> 8),
		(int) (color_table[i].blue >> 8));
		i++;
	fprintf(stderr, "%3d: (%3d, %3d, %3d) \n", i,
		(int) (color_table[i].red >> 8),
		(int) (color_table[i].green >> 8),
		(int) (color_table[i].blue >> 8));
    }

    fprintf(stderr, "\n\n");
}
