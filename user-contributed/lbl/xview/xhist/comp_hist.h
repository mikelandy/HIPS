
#include <hipl_format.h>

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <math.h>

#define Calloc(x,y) (y *)calloc((unsigned)(x), sizeof(y))
#define Fread(a,b,c,d) pread(d, (char *)(a), b*(int)(c) )
#define Cfree(x,y,z) cfree((char *)(x), (unsigned)(y), z)

/* global image stuff */
byte   *barray;
short  *sarray;
int      *iarray;
float    *farray;

int       pix_format;           /* byte, short, int, float  */

int       size = 0;             /* size of image */

int b_hist[256];  /*  byte histogram	*/

float val_min;  /*  for byte,  short	*/
float val_Max;
int occ_Max;

int occ_Max_i;
float occ_Max_f;

int *s_hist;  /*  [-32768:32767]  short histogram	*/

int n_of_dv;  /*  number of different values		*/

int cur_nofdv;  /*  number of different values on the current graph  */

typedef struct ihtype {
	int pv;   /*  pixel value	*/
	int occ;  /*  occurrence	*/
	struct ihtype *next;
	} Ihtype;  /*  int hist type	*/

typedef struct ihtype_a {
	int pv;   /*  pixel value	*/
	int occ;  /*  occurrence	*/
	} Ihtype_a;  /*  int hist type	*/

typedef struct fhtype_a {
	float pv;   /*  pixel value	*/
	int occ;  /*  occurrence	*/
	} Fhtype_a;  /*  float hist type	*/

typedef struct fhtype {
	float pv;   /*  pixel value	*/
	int occ;  /*  occurrence	*/
	struct fhtype *next;
	} Fhtype;  /*  float hist type	*/

Ihtype *i0_hist;  /*  int histogram	*/
Ihtype_a *i_hist;  /*  int histogram	*/

Fhtype *f0_hist;  /*  float histogram	*/
Fhtype_a *f_hist;

int Base;  /*  base index of f_hist[]	*/

char hint_s[30][50];  /*  hint string	*/

int Top;

int Bottom;

int histo_graph[1000];  /*  histogram graph	*/

int histo_va[1000];  /*  current histo_graph value  */

int range;  /*  range of indices of histo_graph to show	  */

float fhisto_va[1000];  /*  current histo_graph value  */

extern int NewLeft;
extern int NewRight;
extern int NewTop;
extern int NewBottom;

char *cur_fname;  /*  current file name	*/

int paint;  /*  0:      fisrt picture of this image
		1:  not fisrt picture of this image     */

int y_label[10];

int end_y;  /*  end of y_label[]	*/

int x_label[20];

int end_x;  /*  end of x_label[]	*/

float fx_label[20];

float fy_label[20];

float pv_diff;  /*  pixel value difference	*/

int occ_diff; /*  occurrence difference      */

int x_pos[20];

int y_pos[20];

extern int TXTWD; /*  text width	*/

extern int CV_HT; /*canvas height */

extern float HGT;  /*  height of histogram               */

extern int TXTHT;   /* height of text in canvas */

int cur_size;  /*  size of current graph        */

int cnt_size;  /* pv_cnt[] size			*/

int *pv_cnt;  /*  pv-->count,  to compute i_hist[] and f_hist[]		*/

extern int nrow;
extern int ncol;
extern int nfr;

