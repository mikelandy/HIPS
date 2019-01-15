/*
 * cmap.c -- routines for manipulating colormap
 *
 * general steps involved in creating ximage with proper color:
 *   1) read data
 *   2) create buffer of gray-level lut indices (make_lut)
 *   3) create colormap of these colors (build_colormap)
 *   4) create buffer of colormap lut indices (map_image_to_colors)
 *   5) create ximage (mk_x_img)
 */

#include "common.h"

/* colors must go in order of the list in common.h */
char     *palnames[PALSIZE] =
{
	"red",
	"brown",
	"orange",
	"yellow",
	"medium forest green",
	"turquoise",
	"blue",
	"purple",
	"black",
	"white"
};

static XColor    pallet[PALSIZE];
u_long   *colors;

byte      red[256], green[256], blue[256];

u_long   *build_colormap();

/* global colormap creation modes */
/* see xcolor.c for a better explaination of these flags */
static int numcol = 256;	/* # desired colors in the picture */
static int noglob = FALSE;	/* allows color flashing */
static int perfect = FALSE;	/* create all colors asked for? */
static int ncols = 245;		/* max # colos to try to allocate */
static int rwcolor = FALSE;	/* can other prg realloc the colormap? */
static int mono = FALSE;	/* convert all colors to grayscale? */
static int use_all = TRUE;	/* use all colors in the cmap, even though
				 * not used in this frame? */
vb = TRUE;			/* Give important messages */

/*************************************************************/
void
cmap_init()
{
	LOGIC rgb_too_close();

	Colormap cmap;
	int i, j, k, val;
	double gammaval = 1.0;
	int r, g, b;
	int gh = 0, rh = 0;		/* HSV values */
	double gs = 1., gv = 1., rs = 1., rv = 1.;
	extern int overlay_hue;	/* command line arg - make 1 for rh later */

	printf(" initializing colormap \n");

	mainw = win[WIN_VZ].xid;

	cmap = XDefaultColormap(display, screen);

	/* set up standout colors (read-only colors) */
	for (i = 0; i < PALSIZE; i++) {
		if (XParseColor(display, cmap, palnames[i], &pallet[i]) == 0)
			fprintf(stderr, "%s: Error parsing color \n", Progname);
		else if (XAllocColor(display, cmap, &pallet[i]) == 0)
			fprintf(stderr, "%s: couldn't allocate color: %s.\n",
				Progname, palnames[i]);
	}

	gammaval = (float) xv_get(Preferences_pop_preferences_display->set_image_contrast, PANEL_VALUE, NULL) / 10.;

	for (i = 0; i < NUM_GRAY; i++) { /* mask-image blend colors*/
		/* hsv model seems to work better */
		gh = overlay_hue;	/* greenish */
		gs = .8;

		/* .5 to 1.0 */
		gv = .5 + (.5 * (float) i / NUM_GRAY.);

		HSV_to_RGB((short) gh, gs, gv, &r, &g, &b);

		red[i] = (u_char) r;
		green[i] = (u_char) g;
		blue[i] = (u_char) b;
	}

	for (i = 0; i < NUM_GRAY; i++) {/* pt_list-image blend colors */
		rh = 0;			/* redish */
		rs = .8;

		/* .5 to 1.0 */
		rv = .5 + (.5 * (float) i / NUM_GRAY.);

		HSV_to_RGB((short) rh, rs, rv, &r, &g, &b);

		k = i + NUM_GRAY;
		red[k] = (u_char) r;
		green[k] = (u_char) g;
		blue[k] = (u_char) b;
	}

	if(segal.color) {
		vprint"*** putting colors into red, green, and blue arrays\n");

		/* put the most frequent colors in ... */
		i = 0; /* number of colors pulled out of rgb.histo */
		j = 0; /* index to the rgb.histo[] */
		while(i < rgb.quant.num_from_histo && j < rgb.num_histo) {
			k = i + NUM_GRAY * 2;
			/* make sure colors are "distinct" enough */
			if(!rgb_too_close(k, rgb.histo[j].r,
			rgb.histo[j].g, rgb.histo[j].b)) {
				red[k] = (u_char) rgb.histo[j].r;
				green[k] = (u_char) rgb.histo[j].g;
				blue[k] = (u_char) rgb.histo[j].b;

				/* remember where we just put this triple */
				rgb.histo[j].col_i = k;
				i++;
			}
			j++;
		}

		/* put the automatic-assigment colors in ... */
		for(i = 0, r = 1; r <= rgb.quant.auto_r; r++)
		for(g = 1; g <= rgb.quant.auto_g; g++)
		for(b = 1; b <= rgb.quant.auto_b; b++, i++) {
			k = i + NUM_GRAY * 2 + rgb.quant.num_from_histo;
			red[k] = (u_char) ((float) r * 
				255. / (float) rgb.quant.auto_r);
			green[k] = (u_char) ((float) g *
				255. / (float) rgb.quant.auto_g);
			blue[k] = (u_char) ((float) b *
				255. / (float) rgb.quant.auto_b);
		}
	}
	else {
		vprint"*** putting grays into red, green, and blue arrays\n");
		for (i = 0; i < NUM_GRAY; i++) {
		/* straight gray values */
			if (gammaval == 1.0)
				j = (int) ((i * 257.) / NUM_GRAY.);
			else
				j = (int) (257. * pow((double) i / NUM_GRAY.,
					1.0 / gammaval));
			if (j > 255) j = 255;

			k = i + NUM_GRAY * 2;
			red[k] = blue[k] = green[k] = (u_char) j;
		}
	}
}

/*************************************************************/
LOGIC
rgb_too_close(num_cols, r, g, b)
int num_cols;
byte r, g, b;
{
/* Searches the red[]. green[], and blue[] alloc'd colors to make sure that
 * this candidate r,g,b is far apart from them ... so we don't waste colors
 * for allocating
 */
	int i;

	for(i = 0; i < num_cols; i++)
		/* too close to an existing color? */
		if(DISTANCE(r, g, b, red[i], green[i], blue[i])
			< rgb.quant.min_rgb_dist)
			return(TRUE);

	/* else found a successful candidate r,g,b */
	return(FALSE);
}

/*************************************************************/
void
build_cmap()
{
	void make_luts();
	void scale_image();
#ifdef DEBUG
	void show_colormap();
#endif
	printf("*** building colormap ...\n");

	vprint"*** making lut's\n");
	make_luts();

	/* creates image_map (the colormap for the image) */
	/* I'm not sure this needs to remain in the program - Bryan */
	/*
	printf("*** scaling image ...\n");
	scale_image(win[WIN_VZ].i_data[0], image_map, NUM_GRAY);
	*/

	printf("*** defining colors ... \n");
	colors = build_colormap(display, winv, mainw,
		(byte *) image_map, win[WIN_VZ].img_size,
		red, green, blue, numcol,
		noglob, perfect, ncols, mono, use_all, rwcolor,
		vb, Progname);

	printf(" colors defined ...\n");
#ifdef DEBUG
	show_colormap();
#endif

return;
}

/************************************************/
void
scale_image(image_data, image_map, ncols)
byte *image_data, *image_map;
int ncols;
{
/* This fn could probably be replaced by a loop of lookups to the lut's */
/* Creates an image of colormap lookup values (array indices) */
	register int i, j;
	float     delta = (float) (ncols - 1) / 256.0;

	if(segal.color) {
	}
	else {
		for (i = 0; i < win[WIN_VZ].img_size; i++) {
			j = (int) image_data[i];
			image_map[i] = (byte) (j * delta);
		}
	}
	return;
}

/************************************************/
void
make_luts()
{
	byte ***alloc_3d_byte_array();
	byte closest_rgb_used();

	register int i, j, k;
	float scale1, scale2;
 
	scale1 = (((float) xv_get(Preferences_pop_preferences_display->set_image_opacity, PANEL_VALUE, NULL) / 100.) * NUM_GRAY.) / 257.;
	scale2 = (((float) xv_get(Preferences_pop_preferences_display->set_mask_opacity, PANEL_VALUE, NULL) / 100.) * NUM_GRAY.) / 257.;

	/* CHANGE - figure out better ways to do all this */
	scale2 = .25;
vprint"scale1 = %f, scale2 = %f\n", scale1, scale2);

	if(segal.color) {
		vprint"*** allocating rgb.map\n");

		if(rgb.map != NULL) {
			free_3d_byte_array(rgb.map);
			rgb.map = NULL;
		}
		rgb.map = alloc_3d_byte_array(rgb.map_r, rgb.map_g, rgb.map_b);

		for(i = 0; i < rgb.map_r; i++)
		for(j = 0; j < rgb.map_g; j++)
		for(k = 0; k < rgb.map_b; k++)
			rgb.map[i][j][k] = UNDEFINED_BYTE;
	}

	/* make gray, points, and blend luts */
	vprint"*** building gray, points, and blend luts\n");
	for (i = 0; i < 256; i++) {
		k = i * scale1;
		if (k >= NUM_GRAY)
			k = NUM_GRAY - 1;
		gray_lut[i] = 2 * NUM_GRAY + k;

		k = i * scale2;
		if (k >= NUM_GRAY)
			k = NUM_GRAY - 1;
		blend_lut[i] = k;
		pt_lut[i] = NUM_GRAY + k;
	}
}

/************************************************/
byte
closest_rgb_used(r, g, b)
byte r, g, b;
{
/* search for the closest r,g,b in used, and return col_i */
	int i, min_i;
	double dist, min_dist, total_colors;

	min_i = 0;
	min_dist = DISTANCE(0, 0, 0, 256, 256, 256);

	if(segal.color) total_colors = 2 * NUM_GRAY + rgb.quant.num_from_histo
		+ (rgb.quant.auto_r * rgb.quant.auto_r * rgb.quant.auto_b);
	else total_colors = 3 * NUM_GRAY;

	for(i = 0; i < total_colors; i++) {
		dist = DISTANCE(r, g, b, red[i], green[i], blue[i]);
		if(dist < min_dist) {
			min_i = i;
			min_dist = dist;
		}
	}
	return(min_i);
}

/************************************************/
byte
map_rgb_to_xcolor(r, g, b)
byte r, g, b;
{
/* Given r, g, b, this uses the rgbmap to find the appropriate color in the
 * colors[].
 */
	byte closest_rgb_used();

	float rx, gx, bx;
	int rm, gm, bm;
	byte i;

	if(rgb.map == NULL) return(0);

	rx = (float) rgb.map_r / 256.;
	gx = (float) rgb.map_g / 256.;
	bx = (float) rgb.map_b / 256.;

	/* indexes to rgb.map */
	rm = (int) ((float) r * rx);
	gm = (int) ((float) g * gx);
	bm = (int) ((float) b * bx);

	i = rgb.map[rm][gm][bm];
	if(i == UNDEFINED_BYTE)
	/* find the closest allocated color and fill in this lattice point */
		i = rgb.map[rm][gm][bm] = closest_rgb_used(r, g, b);

	return(i);
}

/************************************************/
void
map_image_to_buf(image, buf, rows, cols)
byte ***image, **buf;
int rows, cols;
{
/* map image thru color indexes to buf data */
	byte map_rgb_to_xcolor();

	register int r, c;
 
	if(segal.color) for(r = 0; r < rows; r++)
		for(c = 0; c < cols; c++)
		buf[r][c] = (byte) colors[map_rgb_to_xcolor(image[RP][r][c],
			image[GP][r][c], image[BP][r][c])];
	else for(r = 0; r < rows; r++)
		for(c = 0; c < cols; c++)
		buf[r][c] = (byte) colors[gray_lut[image[GRAY][r][c]]];
	return;
}

/************************************************/
void
map_mask_to_buf(mask, buf, mb_key, size, draw_pts)
byte *mask, *buf;
int mb_key, size;
LOGIC draw_pts;
{
    register int i;

	if(draw_pts) for (i = 0; i < size; i++) {
		if(mask[i] & mb_key) {
			if(mask[i] & m[BUF_PTS].bit_key)
				buf[i] = (byte) pallet[ORANGE].pixel;
			else buf[i] = (byte) pallet[CWHITE].pixel;
		}
		else {
			if(mask[i] & m[BUF_PTS].bit_key)
				buf[i] = (byte) pallet[RED].pixel;
			else buf[i] = (byte) pallet[CBLACK].pixel;
		}
	}
	else for (i = 0; i < size; i++) {
		if (mask[i] & mb_key) 
			buf[i] = (byte) pallet[CWHITE].pixel;
		else
			buf[i] = (byte) pallet[CBLACK].pixel;
	}
}

/************************************************/
void
blend_images_to_buf(image, mask, buf, mb_key, rows, cols, draw_pts)
u_char ***image, **mask, **buf;
int mb_key, rows, cols;
LOGIC draw_pts;
{
	byte map_rgb_to_xcolor();

	register int r, c;
	int intensity;

	if(draw_pts) for(r = 0; r < rows; r++)
		for(c = 0; c < cols; c++) {
		if(segal.color)
			intensity = MONO(image[RP][r][c], image[GP][r][c],
				image[BP][r][c]);
		else intensity = image[GRAY][r][c];

		if(mask[r][c] & m[BUF_PTS].bit_key) {
			if(mask[r][c] & mb_key)
				buf[r][c] = (byte) pallet[ORANGE].pixel;
			else buf[r][c] = (byte) colors[pt_lut[intensity]];
		}
		else {
			if(mask[r][c] & mb_key)
				buf[r][c] = (byte) colors[blend_lut[intensity]];
			else {
				if(segal.color)
					buf[r][c] = (byte) colors[map_rgb_to_xcolor(image[RP][r][c], image[GP][r][c], image[BP][r][c])];
				else buf[r][c] = (byte) colors[gray_lut[intensity]];
			}
		}
	}
	else for(r = 0; r < rows; r++)
		for(c = 0; c < cols; c++) {

		/* define intensity */
		if(segal.color)
			intensity = MONO(image[RP][r][c], image[GP][r][c],
				image[BP][r][c]);
		else intensity = image[GRAY][r][c];

		/* get the right xcolor */
		if(mask[r][c] & mb_key) {
			buf[r][c] = (byte) colors[blend_lut[intensity]];
		}
		else {
			if(segal.color)
				buf[r][c] = (byte) colors[map_rgb_to_xcolor(image[RP][r][c], image[GP][r][c], image[BP][r][c])];
			else buf[r][c] = (byte) colors[gray_lut[intensity]];
		}
	}
}

/************************************************/
void
map_nothing_to_buf(buf, size)
byte *buf;
int size;
{
	int i;

	for(i = 0; i < size; i++)
		buf[i] = (byte) colors[(byte) i % 255];
}

/*************************************************************/
u_long
standout(col)
int col;
{
   return ((byte)pallet[col].pixel);
}

/*************************************************************/
u_long
whitepixel()
{
   return ((byte)pallet[CWHITE].pixel);
}
/*************************************************************/
u_long
blackpixel()
{
   return ((byte)pallet[CBLACK].pixel);
}
/*************************************************************/
int
get_gamma(val, gam)
    byte      val;
    float     gam;
{
    float     newval;

    newval = 256. * pow((double) val / 256., (double) (1.0 / gam));
    return (MIN((int) (newval + .5), 255));	/* dont allow values > 255 */
}


/*************************************************************/
void
gamma_proc(item, value, event)
Panel_item item;
int value;
Event *event;
{
	void map_buffers();
	int i;
	double gammaval;
	static double old_gammaval = -1.0;
	int get_gamma();

	gammaval = (float) xv_get(Preferences_pop_preferences_display->set_image_contrast, PANEL_VALUE, NULL) / 10.;

	if (gammaval == old_gammaval)
		return;

	fprintf(stderr, "xmask: performing gamma with value %.2f \n",
		(float) gammaval);

	old_gammaval = gammaval;

	apply_gamma(gammaval, noglob, perfect, ncols, mono, rwcolor);

	For_all_windows
		map_image_to_colors((byte *) win[i].ximg->data, image_map, win[WIN_VZ].img_size);

	/*
	blend(himage.data[0], work_buf[0], (u_char *) blend_image->data, win[WIN_VZ].img_size);
	*/

	map_buffers();
}

/*********************************************************/

HSV_to_RGB(hue, sat, val, red, green, blue)
/* written by David Robertson, LBL
 *
 * HSV_to_RGB - converts a color specified by hue, saturation and intensity
 * to r,g,b values.
 * Hue should be in the range 0-360 (0 is red); saturation and intensity
 * should both be in [0,1].
 */

    short     hue;
    double    sat, val;
    int      *red, *green, *blue;

{
    register double r, g, b;
    int       i;
    register double f, nhue;
    double    p, q, t;

    val *= 255.0;
    if (hue == 360)
	nhue = 0.0;
    else
	nhue = (double) hue / 60.0;
    i = (int) nhue;		/* Get integer part of nhue */
    f = nhue - (double) i;	/* And fractional part */
    p = val * (1.0 - sat);
    q = val * (1.0 - sat * f);
    t = val * (1.0 - sat * (1.0 - f));
    switch (i) {
    case 0:
	r = val;
	g = t;
	b = p;
	break;
    case 1:
	r = q;
	g = val;
	b = p;
	break;
    case 2:
	r = p;
	g = val;
	b = t;
	break;
    case 3:
	r = p;
	g = q;
	b = val;
	break;
    case 4:
	r = t;
	g = p;
	b = val;
	break;
    case 5:
	r = val;
	g = p;
	b = q;
    }

    *red = (int) (r + 0.5);
    *green = (int) (g + 0.5);
    *blue = (int) (b + 0.5);

}
