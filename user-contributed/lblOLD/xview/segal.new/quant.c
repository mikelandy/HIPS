/*
 *	quant.c - quantizing routines to handle color images in Segal3d
 *
 *	Bryan Skene, LBL
 *	November, 1992
 */
#include "common.h"

/*****************************************/
void
sort_frame_lex(buf, size)
COLOR_TYPE *buf; /* one frame of rgb */
int size;
{
/* Sorts the triples in buf in lexicographic order: r, g, b */
	int compare_colors();

	qsort((u_char *) buf, size, sizeof(COLOR_TYPE), compare_colors);
}

/*****************************************/
int
compare_colors(c1, c2)
COLOR_TYPE *c1, *c2;
{
	unsigned long x1, x2;

	/* build  24 bit words describing each color */
	x1 = (c1->r * 1000000) + (c1->g * 1000) + c1->b;
	x2 = (c2->r * 1000000) + (c2->g * 1000) + c2->b;

	if(x1 < x2) return(-1);
	else if(x1 > x2) return(1);
	else return(0); /* the two colors are equal */
}

/*****************************************/
void
build_rgb_histo(buf, size)
COLOR_TYPE *buf;
int size;
{
/* Main quantizing routine */
	int compare_count();

	COLOR_TYPE mark;
	int i, p, count;

	/* 1st pass - determine number of distinct colors */
	rgb.num_histo = 0;
	p = 0;
	while(p < size) {
		mark.r = buf[p].r;
		mark.g = buf[p].g;
		mark.b = buf[p].b;

		/* count the repeats */
		count = 1;
		while(p + count < size
			&& mark.r == buf[p + count].r
			&& mark.g == buf[p + count].g
			&& mark.b == buf[p + count].b)
			count++;

		p += count;
		rgb.num_histo++;
	}

	/* 2nd pass - fill in rgb.histo[] */

	rgb.histo = (RGB_HISTO *) calloc(rgb.num_histo, sizeof(RGB_HISTO));

	i = 0;
	p = 0;
	while(p < size) {
		mark.r = buf[p].r;
		mark.g = buf[p].g;
		mark.b = buf[p].b;

		/* count the repeats */
		count = 1;
		while(p + count < size
			&& mark.r == buf[p + count].r
			&& mark.g == buf[p + count].g
			&& mark.b == buf[p + count].b)
			count++;

		p += count;

		rgb.histo[i].r = mark.r;
		rgb.histo[i].g = mark.g;
		rgb.histo[i].b = mark.b;
		rgb.histo[i].count = count;
		i++;
	}

	/* now sort the rgb_histo by count: highest first */
	qsort((char *) rgb.histo, rgb.num_histo, sizeof(RGB_HISTO),
		compare_count);
}

/*****************************************/
int
compare_count(c1, c2)
RGB_HISTO *c1, *c2;
{
	/* sort by count in descending order */
	if(c1->count > c2->count) return(-1);
	else if(c1->count < c2->count) return(1);
	else return(0);
}

/*****************************************/
void
get_quant_colors(image, rows, cols)
u_char ***image;
int rows, cols;
{
	void set_quant_vals();
	void sort_frame_lex();
	void build_rgb_histo();

	COLOR_TYPE *buf;
	register int r, c, i;
	int size, step;

	set_watch_cursor();

	set_quant_vals();
	if(rgb.quant.num_from_histo == 0) return; /* why bother? */

	/* max number of samples from the image */
	step = rows * cols / rgb.quant.max_samples;

	buf = Calloc(rgb.quant.max_samples, COLOR_TYPE);
	
	/* build list of triples from RGB SEP-PLANE image */
	for(i = 0, r = 0; r < rows; r += step)
	for(c = 0; c < cols; c += step, i++) {
		buf[i].r = image[RP][c][r];
		buf[i].g = image[GP][c][r];
		buf[i].b = image[BP][c][r];
	}

	sort_frame_lex(buf, rgb.quant.max_samples);

	build_rgb_histo(buf, rgb.quant.max_samples);

#ifdef DEBUG
	for(i = 0; i < 25; i++)
		vprint"rgb.histo[%2d]: %3d, %3d, %3d  count = %d\n",
			i, rgb.histo[i].r, rgb.histo[i].g, rgb.histo[i].b,
			rgb.histo[i].count);
#endif

	free(buf);

	unset_watch_cursor();
}

/*****************************************/
void
set_quant_vals()
{
	switch(rgb.quant.quality) {
	case Q_BEST :
		rgb.quant.num_from_histo = 100;
		rgb.quant.auto_r = 0;
		rgb.quant.auto_g = 0;
		rgb.quant.auto_b = 0;
		rgb.quant.max_samples = win[rgb.quant.win].img_size / 16;
		rgb.map_r = 32;
		rgb.map_g = 32;
		rgb.map_b = 32;
		break;
	case Q_MEDIUM :
		rgb.quant.num_from_histo = 64;
		rgb.quant.auto_r = 4; /* 36 colors */
		rgb.quant.auto_g = 3;
		rgb.quant.auto_b = 3;
		rgb.quant.max_samples = win[rgb.quant.win].img_size / 32;
		rgb.map_r = 32;
		rgb.map_g = 32;
		rgb.map_b = 32;
		break;
	case Q_WORST :
		rgb.quant.num_from_histo = 0;
		rgb.quant.auto_r = 5; /* 100 colors */
		rgb.quant.auto_g = 5;
		rgb.quant.auto_b = 4;
		rgb.quant.max_samples = UNDEFINED;
		rgb.map_r = 32;
		rgb.map_g = 24;
		rgb.map_b = 20;
		break;
	default :
		break;

	}
	/* hopefully this makes the alloc'd colors spread out as far as the
	 * lattice of rgb.map does.  This cuts out never even using the colors
	 * that got allocated because the resolution of the lattice is much
	 * worse than the resolution of the rgb.map.
	 */
	rgb.quant.min_rgb_dist = 256 / rgb.map_r; 
}

/*****************************************/
void
quantize()
{
	void get_quant_colors();
	void cmap_init();
	void FreeAllColors();
	void build_cmap();
	void redisplay_all();

	vprint"ReQuant routine\n");
	get_quant_colors(win[rgb.quant.win].i_data, win[rgb.quant.win].img_r,
		win[rgb.quant.win].img_c);
	cmap_init();
	FreeAllColors();
	build_cmap();
	redisplay_all();
}
