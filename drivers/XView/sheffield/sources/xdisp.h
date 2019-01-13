#include <X11/Intrinsic.h>

#define	PROGRAM_NAME	"xdisp"
#define CONTROL_NAME	"xdisp control"
#define SCONTROL_NAME	"xdisp subsample"
#define PCONTROL_NAME	"xdisp properties"
#define GCONTROL_NAME	"xdisp gamma"
#define VGCONTROL_NAME	"xdisp overlay gamma"
#define ICONTROL_NAME	"xdisp info"
#define YCONTROL_NAME	"xdisp polygons"
#define FCONTROL_NAME	"xdisp files"
#define VCONTROL_NAME	"xdisp overlay"

#define	HIST_WIDTH	256
#define	HIST_HEIGHT	100
#define MAXCOLOR	256
#define VIEW_SIZE	512
#define SAMPLE_SIZE	256
#define COLORBAR_HEIGHT	16
#define MAXFRAME	16

#define RED		0
#define GREEN		1
#define BLUE		2

#define NREDS		6
#define NGREENS		6
#define NBLUES		6

#define	RECT		0
#define POLAR		1

#define POLY_GRAVITY_STRENGTH	5

/* macros to convert between image row/column and x/y taking into account 
   coordinate origin and magnification */

#define rctoxy(R,C,X,Y) X=(C-image_pix_data.origin.c)*image_pix_data.mag; \
			Y=(R-image_pix_data.origin.r)*image_pix_data.mag;
#define xytorc(X,Y,R,C) R=Y/image_pix_data.mag+image_pix_data.origin.r; \
			C=X/image_pix_data.mag+image_pix_data.origin.c;

typedef struct origin_t {
	int		r;
	int		c;
		} origin_t;

typedef struct lut_t {
	char		*name;
	XColor		*lut_colors;
	int		size;
	unsigned char	*ymap;
		} lut_t;

typedef struct gamma_limits_t {
	int		ll;
	int		ul;
	int		llx;
	int		ulx;
		} gamma_limits_t;

typedef struct pix_data_t {
	unsigned char	*im;
	int		width,height;
	int		max_width,max_height;
	int		depth;
	GC		gc;
	Pixmap		pix;
	Pixmap		*pix_cache;
	origin_t	origin;
	int		mag;
		} pix_data_t;

typedef struct pan_box_data_t {
	int		r;
	int		c;
	int		w;
	int		h;
	int		old_r;
	int		old_c;
	int		old_w;
	int		old_h;
		} pan_box_data_t;

typedef struct vertex_t {
	int		r;
	int		c;
	struct vertex_t	*next;
		} vertex_t;

typedef struct polygon_t {
	char		 *name;
	int		 nvertex;
	vertex_t	 *vertices;
	struct polygon_t *next;
		} polygon_t;

typedef struct poly_edit_data_t {
	polygon_t	*p;
	vertex_t	*v;
		} poly_edit_data_t;

extern int pixel_format;
extern int nrows;
extern int ncols;
extern int nframes;
extern int frame;

extern int srows;
extern int scols;

extern int maxrows;
extern int maxcols;

extern unsigned char **data,**images,**samples;
extern int **hists,**cdfs;
extern unsigned char **colorbar_images;

extern pix_data_t image_pix_data;
extern pix_data_t sample_pix_data;
extern pix_data_t hist_pix_data[];
extern pix_data_t colorbar_pix_data[];

extern Pixmap hist_pixmaps[];
extern Pixmap cdf_pixmaps[];

extern pan_box_data_t pan_box_data;

extern gamma_limits_t gamma_limits[];

extern int cdf_mode;

extern int pix_cache_on;

extern int changing_lut;

extern float sample_rate;

extern unsigned char *xmap[],*ymap;

extern struct header hdr,hdp;
extern int types[];

extern int cdconv;
extern int precision;
extern int option_e;
extern int extreme_type;
extern int cmy_state;

extern Colormap colormap,app_colormap;
extern Display  *dpy;
extern Visual   *visual;
extern Window	rws;
extern int	depth;

extern int ncolors;
extern int mentries;
extern int black_pixel,white_pixel;
extern XColor colors[];

extern polygon_t *polygon_list;
extern polygon_t *current_polygon;
extern int defining_poly;
extern int naming_polys;
extern int editing_polys;
extern int deleting_polys;
extern int showing_polys;
extern int in_poly_edit;
extern poly_edit_data_t poly_edit_data;

extern int hips_lut_exists;
extern char *hips_lut;
extern int hips_lut_n;

extern int red_frame;
extern int green_frame;
extern int blue_frame;

extern char def_file_name[];
extern char def_poly_name[];

extern Widget 	app_toplevel,
		app_control,
		app_sample,
		app_pcontrol,
		app_scontrol,
		app_gcontrol,
		app_ov_gcontrol,
		app_icontrol,
		app_ycontrol,
		app_fcontrol,
		app_vcontrol,
		rt,
		draw,
		sample,
		colorbar[],
		text,
		props,
		slider_low[],
		slider_high[],
		hist[],
		s_text[],
		info,
		a_popup,
		a_caption,
		a_text,
		a_accept,
		a_cancel;

extern GC gc;
extern GC lgc,llgc,rgc,dgc;
extern XGCValues lgcv,llgcv,rgcv,dgcv;
extern XtAppContext  app;

extern lut_t *current_lut;
extern int nluts;

extern Cursor current_cursor;
extern int ncursors;
