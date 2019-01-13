#define FRAME_BUF 0
#define XDIM	10
#define YDIM	10
#define	PBOXXLIM 32
#define	PBOXYLIM 64
#define	CMS_NAME "viewcolor"
#define	CMS_SIZE 256
#define FG_COLORM 255
#define BG_COLORM 254
#define TOM       BG_COLORM     /* Top Of (grayscale) Map */
#define FCOLOR  255
#define BCOLOR  0

GLOBAL int invertgraymap;
GLOBAL int viewFGcolor, viewBGcolor;
GLOBAL unsigned char rasR[CMS_SIZE+2], rasG[CMS_SIZE+2], rasB[CMS_SIZE+2]; 
GLOBAL colormap_t  ras_colormap;
GLOBAL int	isDwin;

GLOBAL struct imstatistics ap_imst;
GLOBAL IMAGEptr *cur_imwin;
GLOBAL IMAGEptr *imwin, *imewin, *immwin;
GLOBAL IMAGEptr *image1b, *image2b, *image4b;
GLOBAL unsigned char slice[1024];
GLOBAL int	showsw;		/*  show slice, plots, & print box switch */
GLOBAL int	show_ap_eqsw;
GLOBAL int	slx1, sly1, slx2, sly2;
GLOBAL int	ht_lothresh, ht_hithresh;
GLOBAL int	ht_select, ht_markx;
GLOBAL int	win_wd, win_ht;
GLOBAL int	pa_wd, pa_ht;
GLOBAL int	p_wd, p_ht;
GLOBAL int	v_wd, v_ht;
GLOBAL int	w_wd, w_ht;
GLOBAL int	mh_wd,   mh_ht;
GLOBAL int	mh_xorg, mh_yorg;
GLOBAL int	m_wd_scale,   m_ht_scale;
GLOBAL int	sm_wd_scale, sm_ht_scale;
GLOBAL int	m_pic_wd, m_pic_ht;
int	xScaleIFm(), yScaleIFm(), x_Scale(), y_Scale(), x_iScale(), y_iScale();
int	onvec_wx(), onvec_wy();

GLOBAL int	pic_in_mode;		/*  'n'  'e'		*/
GLOBAL int	pic_base_type;		/*  'n'  'e'		*/
GLOBAL int	last_pic_type;		/*  'n'  'e'  'm'  't'	*/
GLOBAL int	cur_pic_type;		/*  'n'  'e'  'm'  't'	*/
GLOBAL int	toggle_pic;		/*  'n'  'e'  'm'  't'	*/
GLOBAL int	pic_magnified;		/*  ' '  'm'		*/
GLOBAL int	hist_win_mode;		/*  'f'  'w'		*/
GLOBAL int	histplot_mode;		/*  ' '  'i'  'e'	*/
#define	Cur_Pic_Type  cur_pic_type 

GLOBAL int	Imax, Imin;
GLOBAL float	Fmax, Fmin;
GLOBAL float	pixClipHi, pixClipLo;
GLOBAL float	fpixZoneHi, fpixZoneLo;
GLOBAL float	fpixFillZval;
GLOBAL int	pixZoneHi, pixZoneLo;
GLOBAL int	pixFillZval;

GLOBAL int	pixZoneMid;
GLOBAL int	pixHalfDeadZ;
GLOBAL int	scaling_done;

GLOBAL int	histplot_mode;
GLOBAL int	hist[256], ihist[256], ehist[256];

#define CLRLINE_SZ 70  /* note! if to large causes extra parent window CR's */
GLOBAL char	clear_line[CLRLINE_SZ];
GLOBAL char	*Bfmt, *UBfmt, *Sfmt, *USfmt, *Lfmt, *Ffmt;

GLOBAL int	vmark_wx, vmark_wy;		/* set by slice.c */
GLOBAL int	plot_cursor_wx, plot_cursor_wy;
GLOBAL int	pic_cursor_wx,  pic_cursor_wy;
GLOBAL struct header hd;
GLOBAL int	hdtype, nu, nv;
GLOBAL int      nu_lim, nv_lim;

GLOBAL Cursor cursor;
GLOBAL int show_cursor;
void repaint_canvas();

GLOBAL char	picfname[80], wr_fname[80];
GLOBAL char	plottext1[80], plottext2[80];
GLOBAL int	frame_lable_sw;
GLOBAL char	frame_lable[80];
GLOBAL char	enable_fname_lable;
GLOBAL char	mbuf[80+1];
GLOBAL int	a_ulcsw;

GLOBAL Pixwin	*canvas_pw;
GLOBAL Frame	base_frame;
GLOBAL Canvas	canvas;
GLOBAL int	canvas_depth;
void mouse_proc();
void quit_proc();

#define MARGIN_X 10
#define MARGIN_Y 12
#define TEXT_HT	 17

#define OVERWRITESW_OFF 0
#define OVERWRITESW_ON  1
#define OVERWRITESW_ENABLE 2
GLOBAL int overwritesw;

GLOBAL int argcc;
GLOBAL char **argvv;
