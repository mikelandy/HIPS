/*	VISUAL . C
#
%	extracted from x11_stuff.c for public usage.
*/

#include "panel.h"

int	log2_levels;


static	CONST_DECL char*
visual_class_to_string(visual_type)
int	visual_type;
{
CONST_DECL char *type_string;

switch (visual_type) {
	case DirectColor:	type_string = "DirectColor";	break;
	case PseudoColor:	type_string = "PseudoColor";	break;
	case TrueColor:		type_string = "TrueColor";	break;
	case StaticColor:	type_string = "StaticColor";	break;
	case GrayScale:		type_string = "GrayScale";	break;
	case StaticGray:	type_string = "StaticGray";	break;
	default: 		type_string = "any/unknown";	break;
    }
return type_string;
}

void
get_x_colormap(img)
image_information *img;
{
img->colormap = XCreateColormap(img->dpy, Root_window, img->dpy_visual, AllocNone);
if (img->colormap == NULL)
	mesg("Could not create color map for visual\n");

VPRINTF(stderr, "created colormap for visual type %s\n",
	visual_class_to_string(img->visual_class));
}

#define BINARY_TABLE_INDEX	0
#define MONOCHROME_TABLE_INDEX	1
#define COLOR_TABLE_INDEX	2

/* Here we ALWAYS want to take a PseudoColor over the next best visual... */
/* in Monochrome, we can do just as much with a pseudo color visual, and   */
/* we may be able to preserve some of the default colors from other windows*/
/* In color mode, some eight bit displays offer a Direct and TrueColor     */
/* visual..   This is quite confusing, Have you ever seen a 24bit display  */
/* offer a 24-bit PseudoColor visual?  They offer 8 bit PC visuals usually */

static int	desired_class_table[][6] = {
{ PseudoColor, StaticGray, GrayScale, DirectColor, TrueColor, StaticColor},
{ PseudoColor, GrayScale, StaticGray, DirectColor, TrueColor, StaticColor},
{ PseudoColor, DirectColor, TrueColor, StaticColor, GrayScale, StaticGray}
};


void
find_appropriate_visual(img)
register image_information *img;
{
static int		num_visuals = 0;
static XVisualInfo	*visual_info = NULL;
register XVisualInfo	*vi, *found_vi;
XVisualInfo	*XGetVisualInfo();
VisualID	def_visual_id;
int		def_visual_index,
		def_scrn = DefaultScreen(img->dpy),
		desired_class, desired_depth,
		deepest_visual, depth_delta;
register int	i;
    
    DPRINTF(stderr, "In find_appropriate_visual(%d)\n", img->img_channels);

    if (visual_info == NULL) {
	visual_info = XGetVisualInfo(img->dpy, VisualNoMask, NULL, &num_visuals);
	if (visual_info == NULL || num_visuals == 0)
		prgmerr(1, "XGetVisualInfo failed\n");
    }


    for (depth_delta=desired_depth=i=1; (i<<=1) < img->lvls; desired_depth++);

    if (img->mono_img && img->lvls == 2)
	img->binary_img = True;

    if (img->binary_img) {
	desired_class = BINARY_TABLE_INDEX;
	desired_depth = 1;
    }
    else if (img->mono_img && /* was a bug */ img->visual_class != TrueColor)
	desired_class = MONOCHROME_TABLE_INDEX;
    else {
	desired_class = COLOR_TABLE_INDEX;	/* for default also */
	desired_depth *= (depth_delta = 3);	/* needed if separate colors */
    }
    i = XDefaultDepth(img->dpy, def_scrn);	/* important modify */
    if (desired_depth > i)
	desired_depth = i;

    VPRINTF(stderr, "Searching for %s visual with desired depth >= %d\n",
	     visual_class_to_string(img->visual_class), desired_depth);

	/*
	* find visual such that:
	*
	* 1. depth is as large as possible (up to true desired depth)
	* 2. visual depth is the smallest of those supported >= depth
	* 3. visual class is the `most desired' for the image type
	*	Meaning! that if it is the DefaultVisual it IS the
	*	most desired.  We can't choose one visual class over
	*	another for all displays!  If the depth of the DefaultVisual
	*	is large enough for the image, or as large as all others, we
	*	use it!  This minimizes screw ups on our part.
	*/

    def_visual_id = DefaultVisual(img->dpy, def_scrn)->visualid;
    found_vi = 0;

    for (i=deepest_visual=0; i < num_visuals; i++) {
	if (deepest_visual < visual_info[i].depth)
		deepest_visual = visual_info[i].depth;

	if (def_visual_id == visual_info[i].visualid)
		def_visual_index = i;
    }
    
    /* Take the Default Visual if it's cool enough... */
    if (visual_info[def_visual_index].depth >= desired_depth ||
	visual_info[def_visual_index].depth == deepest_visual)
	found_vi = &visual_info[def_visual_index];

    /* if we were told to look for a specific type first, do it */
    if (img->visual_class >= 0) {
	int depth;
	for (depth = desired_depth; depth >= 1; depth -= depth_delta) {
	    for (vi = visual_info; vi < visual_info + num_visuals; vi++)
		if (vi->class == img->visual_class && vi->depth >= depth &&
		    vi->screen == def_scrn) {
			found_vi = vi;
			if (found_vi->depth == depth)	break;
		}
	    if (found_vi != NULL && found_vi->class == img->visual_class)
		break;
	}
    }
	/* it == i = XMatchVisual(img->dpy, Screen, desired_depth, found_vi); */
    if (img->visual_class < 0) {
	int depth;
	for (depth = desired_depth; depth >= 0; depth -= depth_delta) {
	    if (found_vi != NULL)
		break;
	    for (i = 0; i < 6; i++) {	/* search for class and depth */
		int	vt = desired_class_table[desired_class][i];

		for (vi = visual_info; vi < visual_info+num_visuals; vi++)
		    if (vi->class == vt && vi->depth >= depth &&
			vi->screen == def_scrn) {
			if (found_vi==NULL || found_vi->depth > vi->depth)
				found_vi = vi;
			if (found_vi->depth == depth)
				break;
		    }
	    }
	}
    }

    if (!found_vi)
	prgmerr(1, "Could not find appropriate visual type - %s\n",
		visual_class_to_string(img->visual_class));

	/* set program the_hdr */
    Screen = found_vi->screen;
    Root_window = RootWindow(img->dpy, Screen);

	/* set img variables */
    img->dpy_depth = found_vi->depth;
    img->dpy_visual = found_vi->visual;
    img->visual_class = found_vi->class;

	/* We want to give the DefaultColormap a shot first */
    if (found_vi->visualid == def_visual_id)
	img->colormap = DefaultColormap(img->dpy, Screen);
    else
	get_x_colormap(img);

    if (img->dpy_depth == 1 || img->binary_img) {
	img->binary_img = img->mono_img = img->dither_img = True;
	img->color_dpy = img->sep_colors = img->rw_cmap = False;
	img->dpy_channels = log2_levels = 1;

	img->lvls = 2;
	img->lvls_squared = 4;
    }	else	{
	register int	class = img->visual_class;

	if (class == GrayScale || class == StaticGray) {
	    img->mono_img = True;
	    img->color_dpy = False;
	    img->dpy_channels = depth_delta = 1;
	}

	img->rw_cmap = class == PseudoColor || class == GrayScale;

	if (img->dpy_visual != DefaultVisual(img->dpy, Screen) &&
		found_vi->visualid == def_visual_id)
	    img->colormap = XCreateColormap(img->dpy, Root_window,
					img->dpy_visual, AllocNone);

	if (class == DirectColor || class == TrueColor/* || class == StaticColor*/) {
	    img->sep_colors = True;
	    i = 1 << (img->dpy_depth / depth_delta);

	    if (img->lvls > i) {
		img->lvls = i;
		img->lvls_squared = i * i;
	    }
	} else {
	    img->sep_colors = False;

	    /* Image is monochrome ??? */
	    if (depth_delta == 1) {
		i = found_vi->colormap_size;
	    }
	    else { /* color */
		for (i=1; i * i * i < found_vi->colormap_size; i++) {}
		i--;
	    }

	    if (img->lvls > i) {
		img->lvls = i;
		img->lvls_squared = i * i;
	    }
	}
	for (log2_levels=1, i=2; i < img->lvls; i <<= 1)	log2_levels++;
    }

    if (verbose)
	message("Visual type %s, depth %d, screen %d\n",
	    visual_class_to_string(img->visual_class), img->dpy_depth, Screen),
	message("levels: %d, log(2) levels: %d\n", img->lvls, log2_levels);
}

