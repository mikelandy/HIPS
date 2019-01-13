/* image file header			*/
/* short hdtype, nx, ny, max, min;	*/

/*      hdtype          code		*/
/*#define RAS_MAGIC	0x59a66a95 *//*	See <rasterfile.h> */
/*#define UBYTE_HD	0x4500 */
/*#define BYTE_HD		0x4510 */
/*#define USHORT_HD	0x4502 */
/*#define SHORT_HD	0x4512 */
/*#define LONG_HD		0x4513 */
/*#define FLOAT_HD	0x4514 */
/*#define SIGNED		0x0010 */

#define NU	512	/* max X size */
#define NV	512	/* max Y size */

#define ZBASE	105
#define PLOT_NY	128

struct imstatistics{
        int     n;
        float   fsum;
        float   fsumsq;
        float   average;
        float   sigma;
        int     min_ix, min_iy;
        int     max_ix, max_iy;
        int     maxcnt, mincnt;
        int     imax, imin;
        float   fmax, fmin;
};

typedef unsigned char * IMAGEptr;
IMAGEptr *im_alloc();

#define PI 3.14159265359
#define NaN      0xFFFFFFFF	/* float NaN      */
#define INFINITY 0x7FFFFFFF	/* float infinity */
#define INITCLIP  99.e999       /*  0x7FFFFFFF;   */
#define AUTO_SCALE 0.

extern struct imstatistics ap_imst;
extern struct header hd;
extern int hdtype, nu, nv;	/* image file header */
extern int	nu_lim, nv_lim;
extern IMAGEptr	*imwin,  *imewin;
extern IMAGEptr	*image1, *image2, *image4;
extern int	showsw;
extern int	slx1, sly1, slx2, sly2;
extern int	mh_xorg, mh_yorg;
extern int	m_wd_scale,  m_ht_scale;
extern int	sm_wd_scale, sm_ht_scale;
extern int	pic_base_type;
extern int	cur_pic_type;
extern int	pic_magnified;

extern int	win_ht, win_wd;
extern int	frame_buf, page;
extern short	hilo;
extern int	picmode, pictype;
extern int	histplot_mode, histdone;
extern int	Imax, Imin;
extern float	Fmax, Fmin;
extern int	hist[256], ihist[256], ehist[256];
extern int	xdim, ydim;
extern int	wx, wy;
extern int	vmark_wx, vmark_wy;
extern int	plot_cursor_wx, plot_cursor_wy;
extern char	plottext1[80], plottext2[80];
extern int	argcc;
extern char	**argvv;

float rawtopix8scale(), rawtopix8offset();
