/*
 * display.h -- structures involving the display of images
 *
 */

/* palette for standout colors */

#include <hipl_format.h>
#include <X11/Xlib.h>
#include <xview/xview.h>
#include <xview/canvas.h>


#define NCOLORS 128     /* total number of colors to use */

#define PALSIZE 8 /* size of the palette */
#define RED                     0
#define YELLOW                  1
#define GREEN                   2
#define CYAN                    3
#define BLUE                    4
#define MAGENTA                 5
#define BLACK                   6
#define WHITE                   7

extern XColor    pallet[PALSIZE];
extern u_long    standout;
extern u_long    *colors;

/* structure for a single display window.  This is an alternative to the
   genial_imgwin_objects structure because genial_imgwin_objects can only
   contain what GUIDE puts into it.*/
struct disp_win {
  Xv_window d_win;      /* both of these derive from GUIDE */
  Xv_Window d_paintwin; /* the paint window of the canvas */
  Canvas d_canv;
  XID d_xid;            /* the XID of the paint window */
};

/* structure for an image dataset */
struct img_data {
  int hsize; /* header size in the file */
  int dsize; /* number of bytes / pixel */
  int width, height; /* the x and y size of the image */
  int nframes, cframe; /* number of frames in sequence, current
			  frame number */
  unsigned minv, maxv; /* min and max of the image */
  char *data;  /* address of actual image data */
  char *lut;  /* colormap lut values */
  char *comment;  /* comment for this image */
  struct header head; /* HIPS header */
  int  file_saved;    /* flag indicating file with log has been saved */
};
  
extern struct img_data *orig_img;
extern XImage *orig_ximg, *disp_ximg, *mk_x_img();

extern struct disp_win *img_win;


