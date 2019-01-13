
/* xhist.h  */

#include <hipl_format.h>

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <math.h>

#include <xview/xview.h>
#include <xview/panel.h>
#include <X11/Xlib.h>

#include <X11/X.h>
#include <xview/frame.h>
#include <xview/canvas.h>
#include <xview/font.h>
#include <xview/xv_xrect.h>
#include "xhist_ui.h"

#define Calloc(x,y) (y *)calloc((unsigned)(x), sizeof(y))
#define Fread(a,b,c,d) fread((char *)(a), b, (int)(c), d)
#define Cfree(x,y,z) cfree((char *)(x), (unsigned)(y), z)

/* global image stuff */

extern int       pix_format;           /* byte, short, int, float  */

extern int       size;             /* size of image */

extern int b_hist[256];  /*  byte histogram	*/

extern float val_min;  /*  for byte,  short      */
extern float val_Max;
extern int occ_Max;

extern int *s_hist;  /*  [-32768:32767]  short histogram       */

extern int n_of_dv;  /*  number of different values            */

extern int cur_nofdv;  /*  number of different values on the current graph  */

typedef struct ihtype {
	int pv;   /*  pixel value       */
	int occ;  /*  occurrence        */
	struct ihtype *next;
	} Ihtype;  /*  int hist type    */

typedef struct fhtype {
	float pv;   /*  pixel value     */
	int occ;  /*  occurrence        */
	struct fhtype *next;
	} Fhtype;  /*  float hist type  */

extern Ihtype *i_hist;  /*  int histogram    */

extern Fhtype *f_hist;  /*  float histogram  */


extern char hint_s[30][50];  /*  hint string	*/

extern int Top;

extern int Bottom;

extern int histo_graph[1000];  /*  histogram graph     */

extern int histo_va[1000];  /*  current histo_graph value  */

extern float fhisto_va[1000];  /*  current histo_graph value  */

extern int range;  /*  range of indices of histo_graph to show   */

#define color_White  WhitePixel(display, DefaultScreen(display))
#define color_Black  BlackPixel(display, DefaultScreen(display))

int FRM_WD; /*  frame width             */

int FRM_HT; /*  frame height            */

float HGT;  /*  height of histogram               */

int TXTHT = 16;		/* height of text in canvas */

int TXTWD = 18;  /*  left blank -FH     */

float y_unit;

int CV_HT = 415;  /*  canvas height          */

int move_j = -1;
int NewLeft = -1;
int NewRight = -1;
int NewTop = -1;
int NewBottom = -1;

extern char *cur_fname;  /*  current file name      */

extern int paint;  /*   0:      fisrt picture of this image
			1:  not fisrt picture of this image	*/

extern int y_label[10];

extern int end_y;  /*  end of y_label[]        */

extern int x_label[20];

extern int end_x;  /*  end of x_label[]        */

extern float fx_label[20];

extern float pv_diff;  /*  pixel value difference      */

extern int occ_diff; /*  occurrence difference      */

int x_cover_l[20];

int x_cover_r[20];

int end_xc;  /*  end of x_cover_l[] and x_cover_r[]	*/

int y_cover_s[20];

int y_cover_g[20];

int end_yc;  /*  end of y_cover_s[] and y_cover_g[]	*/

extern int x_pos[20];

extern int y_pos[20];

extern int cur_size;  /*  size of current graph        */

