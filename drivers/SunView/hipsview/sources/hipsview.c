/*
# cc -c  -f68881 -fsingle -O hipsview.c
  cc -o hipsview  -f68881 -fsingle -O \
	hipsview.o hipsview_1.o viewop.o viewmenu.o viewhelp.o \
	ditpic.o binpic.o magpic.o box_cursor.o vec_cursor.o \
	hist.o slice.o getvector.o readim_file.o \
	-lsuntool -lsunwindow -lpixrect -lm
  exit
*/

#include <hipl_format.h>
#include <stdio.h>
#include "hipsview.h"
#include <sunwindow/window_hs.h>
#include <suntool/canvas.h>
#include <suntool/sunview.h>
#include <suntool/walkmenu.h>
#include <sunwindow/win_input.h>
#include <sunwindow/cms.h>
#include <sunwindow/cms_rainbow.h>

#define GLOBAL
#include "viewglobal.h"
Menu viewmenu, make_menus();

initglobals()
{
	register int i;

	invertgraymap = 0;
	isDwin = (canvas_depth==8) ? 0 : 1;

	a_ulcsw = 'c';
	showsw = show_ap_eqsw = 'p';
	ht_lothresh = 0;
	ht_hithresh = 256;
	pa_wd = XDIM;
	pa_ht = YDIM;
	p_wd = XDIM;
	p_ht = YDIM;
	v_ht = v_wd = 40;
	w_ht = w_wd = 256;
	mh_wd = mh_ht = 40;
	m_wd_scale  = m_ht_scale = 2;
	sm_wd_scale = sm_ht_scale = 2;

	pic_in_mode   = 'n';
	pic_base_type = 'n'; /* set by draw_picture */
	last_pic_type = 'n'; /* set by display_picture */
	cur_pic_type  = 'n';
	toggle_pic    = 'n';
	pic_magnified = ' '; /* set by display_picture */
	hist_win_mode = 'f';
	histplot_mode = ' ';

	pixClipHi =  INITCLIP;
 	pixClipLo = -INITCLIP;
	fpixZoneHi = fpixZoneLo = fpixFillZval= 0.0;
	pixZoneHi  = pixZoneLo = pixFillZval = 0;
	pixZoneMid = pixHalfDeadZ = 0;
	scaling_done= 0;

	Bfmt  = "%4d \0     ";
	UBfmt = "%4u \0     ";
	Sfmt  = "%5d \0     ";
	USfmt = "%5u \0     ";
	Lfmt  = "%6d \0     ";
	Ffmt  = "%6.1f \0   ";

	frame_lable_sw = 'f';
	enable_fname_lable = 0;

	overwritesw = OVERWRITESW_OFF;

	strcpy(&frame_lable[0], " HIPSVIEW:\0");
	for(i=CLRLINE_SZ; --i>=0; )
		clear_line[i] = ' ';
	clear_line[0] = clear_line[CLRLINE_SZ-2] = '\r';
	clear_line[CLRLINE_SZ-1] = '\0';
}

/***********************************************************************/
static short Pic1mprdata[((NU+15)*NV)/16];		/* 33728 bytes */
static short Pic2mprdata[((NU+15)*NV)/16];
static short Plotmprdata[((NU+15)*PLOT_NY)/16];
mpr_static(pic1mpr, NU, NV, 1, Pic1mprdata);
mpr_static(pic2mpr, NU, NV, 1, Pic2mprdata);
mpr_static(plotmpr, NU, PLOT_NY, 1, Plotmprdata);
extern struct pixrect pic1mpr, pic2mpr, plotmpr;   /* mpr's must be extern */

struct pixrect *rawgraympr8P, *heqgraympr8P, *maggraympr8P;
struct pixrect *rawpicmprP,   *heqpicmprP,   *magpicmprP,   *thpicmprP;
struct pixrect *nextpicmprP = &pic1mpr;

main(argc, argv)
int argc;
char **argv;
{
	Progname = strsave(*argv);
	argcc = argc;
	argvv = argv;
	initglobals();
		/* create mouse window */

	base_frame = window_create(NULL, FRAME,
		FRAME_ARGS,		argc, argv,
		FRAME_LABEL,		frame_lable,
		FRAME_DONE_PROC,	quit_proc,
		FRAME_INHERIT_COLORS,	TRUE,
		0);

	win_wd = (int)window_get(base_frame, WIN_WIDTH);
	win_ht = (int)window_get(base_frame, WIN_HEIGHT);

	if(win_wd > NU) win_wd = NU;
	if(win_ht > NV) win_ht = NV;

	set_m_picsize(win_wd, win_ht);
	nu = m_pic_wd;
	nv = m_pic_ht;

	if(win_wd<=510)
		win_ht += TEXT_HT;  /* make room for 2 lines of plot text */
	win_ht += PLOT_NY;

	window_set(base_frame, WIN_WIDTH, win_wd+MARGIN_X, 0);
	window_set(base_frame, WIN_HEIGHT,win_ht+MARGIN_Y+TEXT_HT, 0);

	canvas = window_create(base_frame, CANVAS,
		CANVAS_WIDTH,		win_wd,
		CANVAS_HEIGHT,		win_ht,
		CANVAS_RETAINED,	FALSE,
		CANVAS_REPAINT_PROC,	repaint_canvas,
		WIN_CONSUME_KBD_EVENT,	WIN_ASCII_EVENTS,
		WIN_CONSUME_PICK_EVENT,	WIN_MOUSE_BUTTONS,
		WIN_CONSUME_PICK_EVENT,	WIN_IN_TRANSIT_EVENTS,
		WIN_CONSUME_PICK_EVENT,	LOC_DRAG,
		WIN_CONSUME_PICK_EVENT,	LOC_STILL,
		WIN_EVENT_PROC,		mouse_proc,
		0);

	canvas_pw = canvas_pixwin(canvas);
	canvas_depth = canvas_pw->pw_pixrect->pr_depth;
	isDwin = (canvas_depth==8) ? 0 : 1;

	viewBGcolor = FCOLOR;
	viewFGcolor = BCOLOR;

	graycolormap(canvas_pw, 0, 1);

	signal(SIGQUIT, quit_proc);

	sprintf(&frame_lable[10], " %s",
			"   Type ? for HELP message => to parent window");
	window_set(base_frame, FRAME_LABEL, frame_lable, 0);

	viewmenu = make_menus(base_frame);

	window_main_loop(base_frame);
}

set_m_picsize(wd, ht)
{
	win_wd = wd;
	win_ht = ht;
	if(win_wd <256) win_wd = 256;
	if(win_ht <64)  win_ht = 64;
	m_pic_wd = win_wd;
	m_pic_ht = win_ht;
}

resize(wd, ht)
{
	set_m_picsize(wd, ht);

	if(win_wd<=510)
		win_ht += TEXT_HT;  /* make room for 2 lines of plot text */
	win_ht += PLOT_NY;

	window_set(base_frame, WIN_WIDTH, win_wd+MARGIN_X, 0);
	window_set(base_frame, WIN_HEIGHT,win_ht+MARGIN_Y+TEXT_HT, 0);

	window_set(canvas, CANVAS_WIDTH,  win_wd, 0);
	window_set(canvas, CANVAS_HEIGHT, win_ht, 0);
}

load_file(fnm)
char fnm[];
{
	register unsigned char  *winP;	/* ubyteP */
	register short	*shrtP;
	register unsigned short *ushrtP;
	register long	*lngP;
	register float	*fltP;

	register int i, nuXnv;
	register int Idata, imin, imax;

	enable_fname_lable = 1;
	printf(clear_line);
	printf(" Loading file <%s>...", fnm);
	fflush(stdout);
	ras_colormap.type = RMT_EQUAL_RGB;
	ras_colormap.length = 256;
	ras_colormap.map[0] = &rasR[0];
	ras_colormap.map[1] = &rasG[0];
	ras_colormap.map[2] = &rasB[0];
	if((i=readim_file(fnm, &image1b, &image2b, &image4b))<=0)
		return;

	sscanf(fnm, "%s", picfname);
	nuXnv = nu * nv;
	nu_lim = nu; nv_lim = nv;

/*	if(hdtype==(RAS_MAGIC>>16) && ras_colormap.type!=RMT_NONE){
		fprintf(stdout, "hdtype=%x, type=%d len=%d\n",
			hdtype, ras_colormap.type, ras_colormap.length);
		updaterasmap();
	} */

	if(i!=1){
		image1b = im_alloc(image1b, nuXnv);
		if(image1b==0){
			fprintf(stdout, "Can't malloc image1b\n");
			return;
		}
	}
	if(image2b && i!=2){
		free(image2b);
		image2b = 0;
	}
	if(image4b && i!=4){
		free(image4b);
		image4b = 0;
	}
	if(imewin){
		free(imewin);
		imewin = 0;
	}
	if(immwin){
		free(immwin);
		immwin = 0;
	}

	if(image1b==0)
		return;
	winP  = (unsigned char *)image1b;
	imwin = image1b;

	imax = -INFINITY;
	imin =  INFINITY;

	scaling_done = 0;

	fprint_hdtype(stdout);
	fprintf(stdout,"\n");

	switch(hdtype) {		/* find Imax and Imin */
	case PFBYTE:
		/* image pointed to by winP was loaded by readim_file() */
		for(i=0; i<nuXnv; i++){
			Idata = *winP++;
			if(Idata > imax)
				imax = Idata;
			else if(Idata < imin)
				imin = Idata;
		}
		break;

/*	case BYTE_HD:
		** image pointed to by winP was loaded by readim_file() **
		for(i=0; i<nuXnv; i++){
			Idata = (*winP++ += 128);
			if(Idata > imax)
				imax = Idata;
			else if(Idata < imin)
				imin = Idata;
		}
		break; */

	case PFSHORT:
		shrtP = (short *)image2b;
		for(i=0; i<nuXnv; i++){
			Idata = *shrtP++;
			if(Idata > imax)
				imax = Idata;
			else if(Idata < imin)
				imin = Idata;
		}
		break;

/*	case USHORT_HD:
		ushrtP = (unsigned short *)image2b;
		for(i=0; i<nuXnv; i++){
			Idata = *ushrtP++;
			if(Idata > imax)
				imax = Idata;
			else if(Idata < imin)
				imin = Idata;
		}
		break; */

	case PFINT:
		lngP = (long *)image4b;
		for(i=0; i<nuXnv; i++){
			Idata = *lngP++;
			if(Idata > imax)
				imax = Idata;
			else if(Idata < imin)
				imin = Idata;
		}
		break;

	case PFFLOAT:
		{
			register float fmax, fmin;
			register union{
				float FF;
				int   FI;
			}Fdata;
						/* find fmax and fmin */
			fmax = -INFINITY;
			fmin =  INFINITY;

			fltP = (float *)image4b;
			for(i=0; i<nuXnv; i++){
				Fdata.FF = *fltP++;
				   /* ignore NaN & Infinity */ 
				if((Fdata.FI&INFINITY)==INFINITY)
					continue;
				if(Fdata.FF > fmax)
					fmax = Fdata.FF;
				else if(Fdata.FF < fmin)
					fmin = Fdata.FF;
			}
			Fmax = fmax; Fmin = fmin;
			Imax = fmax; Imin = fmin;
		}
		break;

	default:
		printf("Illegal header type\n");
		return;
	}

	if(hdtype!=PFFLOAT){
		Fmax = imax; Fmin = imin;
		Imax = imax; Imin = imin;
	}
	scale_image();

	hist_win_mode = 'f';

	draw_picture(pic_in_mode);

		/* set initial window position */
	pic_cursor_wx = nu/2;
	pic_cursor_wy = nv/2;
	display_cursor(pic_cursor_wx, pic_cursor_wy);
}

int
pixtransform(v)
register int v;
{
	if(v>=0xFF)
		v = 0xFE;
	else if(v<=0)
		v = 1;
	else if(pixZoneHi>0){
		if(v<pixZoneHi && v>pixZoneLo)
			v  = pixFillZval;
		else if(pixHalfDeadZ>0){
			if(v>pixZoneMid)
				v -= pixHalfDeadZ;
			else
				v += pixHalfDeadZ;
		}
	}
	return(v);
}

scale_image()				/* scale into image window */
{
	register unsigned char  *winP;	/* ubyteP */
	register short	*shrtP;
	register unsigned short *ushrtP;
	register long	*lngP;
	register float	*fltP;

	register int i, nuXnv, pixv, imin;
	float	 fmax, fmin;
	float	 frange;

	nuXnv = nu * nv;

	fmax = (pixClipHi < Fmax) ?  pixClipHi : Fmax;
	fmin = (pixClipLo > Fmin) ?  pixClipLo : Fmin;
	imin = fmin;
	if(fmax==fmin){
		printf("can't scale image fmax==fmin\n");
		return;
	}
	frange = 253. / (fmax - fmin);

	if(fpixZoneHi > fpixZoneLo){
		pixZoneHi   = (fpixZoneHi - fmin) * frange;
		pixZoneLo   = (fpixZoneLo - fmin) * frange;
		pixFillZval = (fpixFillZval - fmin) * frange;
		pixZoneMid  = (pixZoneHi + pixZoneLo)/2;
/* pixHalfDeadZ = 1; */
		if(pixHalfDeadZ > 0){  	/* not implemented */
			pixHalfDeadZ = (pixZoneHi - pixZoneLo)/2;
			fmin -= pixHalfDeadZ*frange;
			fmax += pixHalfDeadZ*frange;
			imin = fmin;
			frange = 253. / (fmax - fmin);
		}
	}else{
		pixZoneHi   = 0;  
		pixZoneLo   = 0; 
		pixFillZval = 0;
		pixZoneMid  = 0;
		pixHalfDeadZ= 0; 
	}

	if(pixFillZval>=0xFF)
		pixFillZval = 0xFE;
	if(pixFillZval<=0)
		pixFillZval = 1;

	printf("scaling image to (%.3f  %.3f)\n", fmin, fmax);
	if(fpixFillZval>0.0)
		printf("and filling pixZone(%d<%.3f> %d<%.3f>) with %d<%.3f>\n",
			pixZoneLo, fpixZoneLo,  pixZoneHi, fpixZoneHi,
			pixFillZval, fpixFillZval);
	else if(pixHalfDeadZ > 0)
		printf("with pix Dead Zone(%d<%f> %d<%f>)\n",
			pixZoneLo, fpixZoneLo,  pixZoneHi, fpixZoneHi);

	winP = (unsigned char *)image1b;

	switch(hdtype){
	case PFBYTE:
		if(pixClipHi < Fmax || pixClipLo > Fmin || pixHalfDeadZ > 0){
/* SHOULD! first copy to save original image */
			for(i=0; i<nuXnv; i++){
				pixv = (*winP - imin) * frange;
				*winP++ = pixtransform(pixv);
			}
		}
		break;

	case PFSHORT:
		shrtP = (short *)image2b;
		for(i=0; i<nuXnv; i++){
			pixv = (*shrtP++ - imin) * frange;
			*winP++ = pixtransform(pixv);
		}
		break;

/*	case USHORT_HD:
		ushrtP = (unsigned short *)image2b;
		for(i=0; i<nuXnv; i++){
			pixv = (*ushrtP++ - imin) * frange;
			*winP++ = pixtransform(pixv);
		}
		break; */

	case PFINT:
		lngP = (long *)image4b;
		for(i=0; i<nuXnv; i++){
			pixv = (*lngP++ - imin) * frange;
			*winP++ = pixtransform(pixv);
		}
		break;

	case PFFLOAT:
		fltP = (float *)image4b;
		for(i=0; i<nuXnv; i++){
			pixv = (*fltP++ - fmin) * frange;
			*winP++ = pixtransform(pixv);
		}
		break;
	}
	scaling_done = 1;
}

int
write_box_to_file(filenm, mx, my)
char *filenm;
{
	register int j, mxlim, mylim, n, nn;
	short nx, ny;
	int nbytes;
	FILE *wfp;
	unsigned char *chP;
	short Lhdtype;

	mxlim = mx+w_wd-1;
	mylim = my+w_ht-1;
	if(mxlim>=nu)
		mx = nu - w_wd;
	if(mylim>=nv)
		my = nv - w_ht;

	if(mx<=0 || my<=0){
		fprintf(stdout, "write Err: box out of pixel area\n");
		overwritesw = OVERWRITESW_OFF;
		return(-1);
	}
	if(access(filenm, 0)==0 && overwritesw!=OVERWRITESW_ON){
		overwritesw = OVERWRITESW_ENABLE;
		fprintf(stdout,
"\nFile <%s> exists - type '\"' to overwrite ? \n ***  ",filenm);
		fflush(stdout);
		return(1);
	}

	if(filenm==0 || filenm[0]<='"'){
		fprintf(stdout,"Illegal filename\n");
		return(1);
	}

	fprintf(stdout, "\n");
	overwritesw = OVERWRITESW_OFF;

	if((wfp=fopen(filenm, "w")) == NULL) {
		perror("write_box_to_file, ");
		fprintf(stderr, "Can't creat file: %s\n", filenm);
		return(-1);
	}

	fprintf(stdout, "Writing boxed image ");
	fprint_hdtype(stdout);
	fprintf(stdout, " to file <%s>....", filenm);
	fflush(stdout);

	nx = w_wd; ny = w_ht;
	setsize(&hd,ny,nx);
	fwrite_header(wfp,&hd,filenm);

	nbytes = 0;
	switch(hdtype){
	case  PFBYTE:
		n = w_wd;
		for(j=my; j<=mylim; j++){
			chP = (unsigned char *)imwin + (mx+j*nu);
			fwrite(chP, n,1,wfp);
			nbytes += n;
		}
		nn = 1;
		break;

	case  PFSHORT:
		n = w_wd*2;
		for(j=my; j<=mylim; j++){
			chP = (unsigned char *)image2b + (mx+j*nu)*2;
			fwrite(chP, n,1,wfp);
			nbytes += n;
		}
		nn = 2;
		break;

	case  PFINT:
	case  PFFLOAT:
		n = w_wd*4;
		for(j=my; j<=mylim; j++){
			chP = (unsigned char *)image4b + (mx+j*nu)*4;
			fwrite(chP, n,1,wfp);
			nbytes += n;
		}
		nn = 4;
		break;
	}
	fprintf(stdout, "\nWROTE (wd=%d X ht=%d X %d) = %d bytes\n",
				w_wd, w_ht, nn, nbytes);
	fflush(stdout);
	fclose(wfp);
	return(0);
}

int
getrawfpix(mx, my, rawfpixP)
int	mx, my;
float	*rawfpixP;
{
	register unsigned char  *ubyteP;
	register short	*shrtP;
	register unsigned short *ushrtP;
	register long	*lngP;
	register float	*fltP;

	register int  index;
	register char pixtype;
	int	 itmp, pix8;
	float	 rawfpix, fmax, fmin;

	mx = xScaleIFm(mx),  my = yScaleIFm(my);

	index = mx + my*nu;
	switch(hdtype){
	case PFBYTE:
		ubyteP  = (unsigned char *)imwin+index;
		rawfpix = *ubyteP;
		break;
/*	case BYTE_HD:
		ubyteP  = (unsigned char *)imwin+index;
		itmp    = (int)*ubyteP - 128;
		rawfpix = itmp;
		break; */
	case PFSHORT:
		shrtP   = (short *)image2b+index;
		rawfpix = *shrtP;
		break;
/*	case USHORT_HD:
		ushrtP  = (unsigned short *)image2b+index;
		rawfpix = *ushrtP;
		break; */
	case PFINT:
		lngP    = (long *)image4b+index;
		rawfpix = *lngP;
		break;
	case PFFLOAT:
		fltP    = (float *)image4b+index;
		rawfpix = *fltP;
		break;
	}

	fmax = (pixClipHi < Fmax) ?  pixClipHi : Fmax;
	fmin = (pixClipLo > Fmin) ?  pixClipLo : Fmin;
	pix8 = (rawfpix - fmin) * 253. / (fmax - fmin) + 1;

	if(!scaling_done)
		pixtype = '?';
	else if(pix8<pixZoneHi && pix8>pixZoneLo)
		pixtype = 'z';
	else if(pixClipHi>pixClipLo && (rawfpix>pixClipHi || rawfpix<pixClipLo))
		pixtype = 'c';
	else if(pixClipHi<pixClipLo && (rawfpix<pixClipHi || rawfpix>pixClipLo))
		pixtype = 'c';
	else
		pixtype = ' ';

	*rawfpixP = rawfpix;
	return(pixtype);
}

int
xScaleIFm(xx)
{
	if(cur_pic_type=='m' || (cur_pic_type=='t'&&pic_magnified=='m'))
		xx = mh_xorg + xx / m_wd_scale;
	return(xx);
}

int
yScaleIFm(yy)
{
	if(cur_pic_type=='m' || (cur_pic_type=='t'&&pic_magnified=='m'))
		yy = mh_yorg + yy / m_ht_scale;
	return(yy);
}
int
x_Scale(xx)
{
	xx = mh_xorg + xx / m_wd_scale;
	return(xx);
}
int
y_Scale(yy)
{
	yy = mh_yorg + yy / m_ht_scale;
	return(yy);
}

int
x_iScale(xx)
{
	xx = (xx - mh_xorg) * m_wd_scale;
	return(xx);
}
int
y_iScale(yy)
{
	yy = (yy - mh_yorg) * m_ht_scale;
	return(yy);
}

setp_wdht()
{
	p_wd = pa_wd;	p_ht = pa_ht;
	if(p_wd > PBOXXLIM) p_wd = PBOXXLIM;
	if(p_ht > PBOXYLIM) p_ht = PBOXYLIM;
}

static int L_imax, L_imin;

fprint_box(fP, mx, my, markx, marky)
FILE *fP;
int mx, my, markx, marky;
{
	register int i, j, signed, mxlim, mylim;
	register char	*byteP;
	register unsigned char  *ubyteP;
	register short	*shrtP;
	register unsigned short *ushrtP;
	register long	*lngP;
	register float	*fltP;
	register float	ft;
	int   it, Lp_wd, Lp_ht, pix8;
	float xc, yc;
	float fmax, fmin;

	if(fP==stdout){
		p_wd = pa_wd;	p_ht = pa_ht;
		if(p_wd > PBOXXLIM) p_wd = PBOXXLIM;
		if(p_ht > PBOXYLIM) p_ht = PBOXYLIM;
		Lp_wd = p_wd; Lp_ht = p_ht;
	}else{
		Lp_wd = pa_wd; Lp_ht = pa_ht;
	}
	mxlim = mx + Lp_wd - 1;
	mylim = my + Lp_ht - 1;
	xc = mx + (Lp_wd/2.0 - 0.5);
	yc = my + (Lp_ht/2.0 - 0.5);

	signed = (hdtype == PFBYTE) ? 0 : 1;
	fprintf(fP, "\n(%dX%d) FILE: %s  ,", nu, nv, picfname);
	fprint_hdtype(fP);
	fprintf(fP, "\n");

	getimstatistics(&ap_imst, mx, my, Lp_wd, Lp_ht, nu, nv);
	get_avg_sigma(&ap_imst);
	L_imax = ap_imst.imax;
	L_imin = ap_imst.imin;
	fmax = ap_imst.fmax;
	fmin = ap_imst.fmin;

	switch(hdtype) {
	case PFBYTE:
		if(imwin==0) return;
		i = (signed) ? Bfmt[1] : UBfmt[1];
		fpr_box_Xhd(fP, i, mx, mxlim, markx);
		for(j=my; j<=mylim; j++) {
			if(j==marky)
				fprintf(fP, "%3d< ", j);
			else
				fprintf(fP, "%3d: ", j);
			ubyteP =  (unsigned char *)imwin+(mx+j*nu);
			for(i=mx; i<=mxlim; i++){
				if(signed){
					it = (int)*ubyteP++ - 128;
					fprintf(fP, Bfmt, it);
				}else{
					it = *ubyteP++;
					fprintf(fP, UBfmt, it);
				}
				fpr_box_pixtype_code(fP, it, it);
			}
			fprintf(fP, "\n");
		}
		break;

	case PFSHORT:
		if(image2b==0) return;
		i = (signed) ? Sfmt[1] : USfmt[1];
		fpr_box_Xhd(fP, i, mx, mxlim, markx);
		for(j=my; j<=mylim; j++) {
			if(j==marky)
				fprintf(fP, "%3d< ", j);
			else
				fprintf(fP, "%3d: ", j);
			if(signed)
				shrtP  =  (short *)image2b+(mx+j*nu);
			else
				ushrtP =  (unsigned short *)image2b+(mx+j*nu);
			ubyteP =  (unsigned char *)imwin+(mx+j*nu);
			for(i=mx; i<=mxlim; i++){
				if(signed){
					it = *shrtP++;
					fprintf(fP, Sfmt, it);
				}else{
					it = *ushrtP++;
					fprintf(fP, USfmt, it);
				}
				fpr_box_pixtype_code(fP, it, *ubyteP++);
			}
			fprintf(fP, "\n");
		}
		break;

	case PFINT:
		if(image4b==0) return;
		fpr_box_Xhd(fP, Lfmt[1], mx, mxlim, markx);
		for(j=my; j<=mylim; j++) {
			if(j==marky)
				fprintf(fP, "%3d< ", j);
			else
				fprintf(fP, "%3d: ", j);
			lngP =  (long *)image4b+(mx+j*nu);
			ubyteP =  (unsigned char *)imwin+(mx+j*nu);
			for(i=mx; i<=mxlim; i++){
				it = *lngP++;
				fprintf(fP, Lfmt, it);
				fpr_box_pixtype_code(fP, it, *ubyteP++);
			}
			fprintf(fP, "\n");
		}
		break;

	case PFFLOAT:
		if(image4b==0) return;
		fpr_box_Xhd(fP, Ffmt[1], mx, mxlim, markx);
		for(j=my; j<=mylim; j++) {
			if(j==marky)
				fprintf(fP, "%3d< ", j);
			else
				fprintf(fP, "%3d: ", j);
			fltP =  (float *)image4b+(mx+j*nu);
			ubyteP =  (unsigned char *)imwin+(mx+j*nu);
			for(i=mx; i<=mxlim; i++){
				ft = *fltP++;
				fprintf(fP, Ffmt, ft);
				if(fmax==ft)
					fprintf(fP, "\bH");
				else if(fmin==ft)
					fprintf(fP, "\bL");
				else if((pix8= *ubyteP++)<pixZoneHi && pix8>pixZoneLo)
					fprintf(fP, "\bz");
				else if(ft>pixClipHi || ft<pixClipLo)
					fprintf(fP, "\bc");
			}
			fprintf(fP, "\n");
		}
		break;
	}

	fprintf(fP, "xyUL(%3d,%3d) xyC(%.1f,%.1f) ", mx, my, xc, yc);
	fprintf(fP, "wd:ht(%3d,%3d)\n", Lp_wd, Lp_ht);

	get_avg_sigma(&ap_imst);
	fprintf(fP, "pix avg = %f  sigma = %f  range = %f\n",
			ap_imst.average, ap_imst.sigma,
			ap_imst.fmax-ap_imst.fmin);
	fprintf(stdout, " %d min @xy(%d %d)=%f",
			ap_imst.mincnt, ap_imst.min_ix,
			ap_imst.min_iy, ap_imst.fmin);
	fprintf(stdout, "  %d max @xy(%d %d)=%f\n",
			ap_imst.maxcnt, ap_imst.max_ix,
			ap_imst.max_iy, ap_imst.fmax);
	fprintf(fP, "\n");
}

fpr_box_pixtype_code(fP, it, pix8)
FILE *fP;
{
	if(it==L_imax)
		fprintf(fP, "\bH");
	else if(it==L_imin)
		fprintf(fP, "\bL");
	else if(pix8<pixZoneHi && pix8>pixZoneLo)
		fprintf(fP, "\bz");
	else if(it>pixClipHi || it<pixClipLo)
		fprintf(fP, "\bc");
}

fpr_box_Xhd(fP, fmt1, ib, ie, xmark)
FILE *fP;
char fmt1;
{
		static char fmt[] = "%3d:";

		if(fmt1 <'3' || fmt1 >'9')  fmt1 = '3';
		fmt[1] = fmt1;
		fprintf(fP, "   : ");
		for(; ib<=ie; ib++){
			fmt[3] = (ib==xmark) ? '<' : ':';
			fprintf(fP, fmt, ib);
		}
		fprintf(fP, "\n");
}

fprint_hdtype(fP)
FILE *fP;
{
	fprintf(fP," type ");
	switch(hdtype){
	case PFBYTE:	fprintf(fP,"BYTE");	break;
/*	case BYTE_HD:	fprintf(fP,"BYTE");	break; */
	case PFSHORT:	fprintf(fP,"SHORT");	break;
/*	case USHORT_HD:	fprintf(fP,"USHORT");	break; */
	case PFINT:	fprintf(fP,"INT");	break;
	case PFFLOAT:	fprintf(fP,"FLOAT");	break;
	default: printf("INVALID HEADER");	break;
	}
}

display_pic(buf, pic_type)
IMAGEptr *buf;
{
	int	lnu, lnv;

	reset_cursors();
	pw_writebackground(canvas_pw, 0, 0, window_get(canvas, CANVAS_WIDTH),
			window_get(canvas, CANVAS_HEIGHT), PIX_CLR);

	if(pic_magnified=='m' && immwin)
		buf = immwin;

	if(buf==0) return;
	if(buf==immwin){
		pic_magnified = 'm';
		lnu = m_pic_wd;	lnv = m_pic_ht; 
	}else{
		pic_magnified = ' ';
		lnu = nu;	lnv = nv;
	}

	if(last_pic_type!=pic_type)
		nextpicmprP = (nextpicmprP== &pic1mpr) ? &pic2mpr : &pic1mpr;

	switch(pic_type){
	case 't':
		if(histplot_mode=='e' && imewin){
		    if(hist_win_mode=='w')
			gethist(ehist, imewin, mh_xorg, mh_yorg,
				mh_wd, mh_ht, nu, nv);
		    else
	  		gethist(ehist, imewin, 0, 0, nu, nv, nu, nv);
		}
		MakeHist();
		makehistplot(histplot_mode, pic_type, &plotmpr);
		showplot(nu);

		thpicmprP = nextpicmprP;
		if(ht_lothresh > ht_hithresh){	 /* swap */
			 ht_select   = ht_hithresh;
			 ht_hithresh = ht_lothresh;
			 ht_lothresh = ht_select;
		}
		ht_select = 0;
		printf("drawing threshold mapped binary image, lowth=%d hith=%d\n",
			ht_lothresh, ht_hithresh);
		fflush(stdout);
		binpic(canvas_pw, 0,0,0, buf, lnu, lnv,
				ht_lothresh, ht_hithresh, thpicmprP);
		break;
	case 'e':
	case 'm':
	case 'n':
		write_pic_to_win(buf, lnu, lnv, pic_type);
		break;
	}
	last_pic_type = cur_pic_type = toggle_pic = pic_type;
	lable_pic();
}
	

write_pic_to_win(buf, lnu, lnv, pic_type)
IMAGEptr *buf;
{
	int invert;
	struct pixrect *mem_point();
	register struct pixrect **mprPP;

	if(isDwin){
		switch(pic_type){
		case 'e': heqpicmprP = nextpicmprP; break;
		case 'm': magpicmprP = nextpicmprP; break;
		case 'n': rawpicmprP = nextpicmprP; break;
		}

		invert = (isDwin==2) ? 1: 0;
		ditpic(canvas_pw, 0,0,invert, buf, lnu, lnv, 0,0, nextpicmprP);
		if(canvas_depth==8)
			pw_blackonwhite(canvas_pw, 0, 255);

	}else{
		switch(pic_type){
		case 'e': mprPP  = &heqgraympr8P; break;
		case 'm': mprPP  = &maggraympr8P; break;
		case 'n': mprPP  = &rawgraympr8P; break;
		}

		if(*mprPP) pr_destroy(*mprPP);
		*mprPP = mem_point(lnu, lnv, 8, &buf[0]);
		if(nu&1){
			fprintf(stdout,
"\nWARNING nu=%d must be even for graycolormap : use D1 mode\n", lnu);
		}
		pw_write(canvas_pw, 0, 0, lnu, lnv, PIX_SRC, *mprPP, 0, 0);
	}
}

struct pixrect *
get_mprP(pic_type)
{
	struct pixrect *mprP;

	if(isDwin){
		switch(pic_type){
			case 'e': mprP = heqpicmprP;	break;
			case 'm': mprP = magpicmprP;	break;
			case 't': mprP = thpicmprP;	break;
			case 'n':
			default:  mprP = rawpicmprP;	break;
		}
	}else{
		switch(pic_type){
			case 'e': mprP = heqgraympr8P;	break;
			case 'm': mprP = maggraympr8P;	break;
			case 't': mprP = thpicmprP;	break;
			case 'n':
			default:  mprP = rawgraympr8P;	break;
		}
	}
	return(mprP);
}
	

void
repaint_canvas(canvas, pw, repaint_area)
Canvas	 canvas;
Pixwin	 *pw;
Rectlist *repaint_area;
{
	int	lnu, lnv;
	struct pixrect *repaint_mprP;

	reset_cursors();
	if(cur_pic_type=='m'){
		lnu = m_pic_wd;	lnv = m_pic_ht; 
	}else{
		lnu = nu;	lnv = nv;
	}

	repaint_mprP = get_mprP(cur_pic_type);
	lable_pic();
	
	pw_write(canvas_pw, 0, 0, lnu, lnv, PIX_SRC, repaint_mprP, 0, 0);
	showplot(lnu);
}

lable_pic()
{

	if(frame_lable_sw=='s'){
		sprintf(&frame_lable[10], "%3d,%3d<B%c.%c H%c C%c S%c>",
			nu, nv, pic_base_type, pic_magnified,
			hist_win_mode, cur_pic_type, showsw);
	}else if(enable_fname_lable){
		sprintf(&frame_lable[10], " %s", picfname);
	}
	window_set(base_frame, FRAME_LABEL, frame_lable, 0);
}

doShowSlicePlot(showsw)
{
	if(showsw=='v')
		ShowSlicePlot(showsw, pic_cursor_wx, pic_cursor_wy,
				vmark_wx, vmark_wy);
	else
		ShowSlicePlot(showsw, pic_cursor_wx, pic_cursor_wy,
				pic_cursor_wx, pic_cursor_wy);
}

ShowSlicePlot(showsw, x, y, mkx, mky)
{
	int n;
	IMAGEptr *imP;

	imP = imwin;
	nu_lim = nu;	nv_lim = nv;

	switch(cur_pic_type){
	case 't':
		if(pic_magnified!='m')
			break;
	case 'm':
		imP = immwin;
		nu_lim = m_pic_wd; nv_lim = m_pic_ht;
		break;
	case 'e':
		imP = imewin;
		break;
	}

	switch(showsw){
	case 'x':
		slx1 = 0;     sly1 = y;
		slx2 = nu_lim-1; sly2 = y;
		break;
	case 'y':
		slx1 = x; sly1 = 0;
		slx2 = x; sly2 = nv_lim-1;
		break;
	case 'v':
		slx1 = x;
		sly1 = y;
		if(cur_pic_type=='m'||(cur_pic_type=='t'&&pic_magnified=='m')){
			slx2 = x + v_wd*m_wd_scale;
			sly2 = y + v_ht*m_ht_scale;
		}else{
			slx2 = x + v_wd;
			sly2 = y + v_ht;
		}
		break;
	default:
		return;
	}

	if(imP!=0){
		n= getpixvector(slice,
				slx1, sly1, slx2, sly2, imP, nu_lim, nv_lim);
		makesliceplot(slice, n, mkx, mky, &plotmpr);
	 	showplot(nu_lim);
	}
}

update_slice_cursor_to_match_plot()
{
	switch(showsw){
	case 'x':
		pic_cursor_wx = plot_cursor_wx;
		pic_cursor_wy = sly1;
		break;
	case 'y':
		pic_cursor_wx = slx1;
		pic_cursor_wy = plot_cursor_wx;
		break;
	case 'v':
		pic_cursor_wx = slx1;
		pic_cursor_wy = sly1;
		break;
	}
}

MakeHist()
{
	if(imwin==0) return;

	if(hist_win_mode=='w')
		gethist(hist, imwin, mh_xorg, mh_yorg, mh_wd, mh_ht, nu, nv);
	else
		gethist(hist, imwin, 0, 0, nu, nv, nu, nv);
	
	getihist(ihist, hist);
	if(histplot_mode=='e' && imewin){
		if(hist_win_mode=='w')
		    gethist(ehist, imewin, mh_xorg, mh_yorg, mh_wd, mh_ht, nu, nv);
		else
		    gethist(ehist, imewin, 0, 0, nu, nv, nu, nv);
	}
}

ShowHistWinPlot()
{
	MakeHist();
	makehistplot(histplot_mode, showsw, &plotmpr);
	showplot(nu);
}

showplot(lnu)
{
	clear_plot();
	win_wd = (int)window_get(canvas, CANVAS_WIDTH);
	win_ht = (int)window_get(canvas, CANVAS_HEIGHT);
	if(win_wd > 510){
		pw_write(canvas_pw,  0, win_ht-PLOT_NY, lnu, PLOT_NY,
						  PIX_SRC, &plotmpr, 0, 0);
		pw_text( canvas_pw,  0, win_ht-5, PIX_SRC, 0, clear_line);
		pw_text( canvas_pw,  5, win_ht-5, PIX_SRC, 0, plottext1);
		pw_text( canvas_pw,256, win_ht-5, PIX_SRC, 0, plottext2);
	}else{
		pw_write(canvas_pw,  0, win_ht-17-PLOT_NY, lnu, PLOT_NY,
						     PIX_SRC, &plotmpr, 0, 0);
		pw_text( canvas_pw,  0, win_ht-5,    PIX_SRC, 0, clear_line);
		pw_text( canvas_pw,  0, win_ht-5-17, PIX_SRC, 0, clear_line);
		pw_text( canvas_pw,  5, win_ht-5,    PIX_SRC, 0, plottext1);
		pw_text( canvas_pw,  5, win_ht-5-17, PIX_SRC, 0, plottext2);
	}
}

clear_plot()
{
	win_wd = (int)window_get(canvas, CANVAS_WIDTH);
	win_ht = (int)window_get(canvas, CANVAS_HEIGHT);
	if(win_wd > 256){
		pw_writebackground(canvas_pw, 0, win_ht-PLOT_NY,
				win_wd, PLOT_NY+MARGIN_Y, PIX_CLR);
	}else{
		pw_writebackground(canvas_pw, 0, win_ht-17-PLOT_NY,
				win_wd, PLOT_NY+MARGIN_Y+TEXT_HT, PIX_CLR);
	}
}

static int cursor_depth = NULL;
static int vec_color = 1;

display_cursor(mx, my)
int mx, my;
{
	int c_wd, c_ht; 

	if(canvas_depth==1){
		if(cursor_depth!=1){
			create_box_cursor_mprs(1);
			cursor_depth = 1;
			vec_color = 1;
		}
	}else{
		if(cursor_depth!=8){
			create_box_cursor_mprs(8);
			cursor_depth = 8;
			vec_color = 0xFF;
		}
	}

	switch(showsw){
	case 'x':
		box_cursor(canvas_pw, 0, my, nu_lim-2, 1, mx, my);
		break;
	case 'y':
		box_cursor(canvas_pw, mx, 0, 1, nv_lim-2, mx, my);
		break;
	case 'v':
		if(cur_pic_type=='m'||(cur_pic_type=='t'&&pic_magnified=='m')){
			c_wd = v_wd*m_wd_scale; c_ht = v_ht*m_ht_scale;
		}else{
			c_wd = v_wd; c_ht = v_ht;
		}
		vec_cursor(canvas_pw, mx, my, mx+c_wd, my+c_ht,
				   vec_color, vmark_wx, vmark_wy);
		break;
	case 'w':
		if(cur_pic_type=='m'){
			c_wd = w_wd*m_wd_scale; c_ht = w_ht*m_ht_scale;
		}else{
			c_wd = w_wd; c_ht = w_ht;
		}
		box_cursor(canvas_pw, mx, my, c_wd, c_ht, mx, my);
		break;
	case 't':
	case 'h':
		if(cur_pic_type=='t')
			goto L_case_ap;
		if(hist_win_mode=='f')
			break;
		/*FALLTHROUGH to m*/
	case 'm':
		if(cur_pic_type=='m'){
			c_wd = pa_wd*m_wd_scale; c_ht = pa_ht*m_ht_scale;
		}else{
			c_wd = mh_wd; c_ht = mh_ht;
		}
		box_cursor(canvas_pw, mx, my, c_wd, c_ht, mx, my);
		break;
	case 'a':
	case 'p':
		show_ap_eqsw = showsw;
    L_case_ap:
		if(cur_pic_type=='m'){
			c_wd = pa_wd*m_wd_scale; c_ht = pa_ht*m_ht_scale;
		}else{
			c_wd = pa_wd; c_ht = pa_ht;
		}
		box_cursor(canvas_pw, mx, my, c_wd, c_ht, mx, my);
		break;
	}
}

reset_cursors()
{
	reset_box_cursor(canvas_pw);
	reset_vec_cursor(canvas_pw, vec_color);
}

void
quit_proc()
{
	window_destroy(base_frame);
	destory_box_cursor_mprs();
	if(heqgraympr8P) pr_destroy(heqgraympr8P);
	if(maggraympr8P) pr_destroy(maggraympr8P);
	if(rawgraympr8P) pr_destroy(rawgraympr8P);
	exit(0);
}

