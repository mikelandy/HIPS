#include <hipl_format.h>
#include <stdio.h>
#include "hipsview.h"
#include <sunwindow/window_hs.h>
#include <suntool/canvas.h>
#include <suntool/sunview.h>
#include <sunwindow/win_input.h>
#include <sunwindow/cms.h>
#include <sunwindow/cms_rainbow.h>

#define GLOBAL extern
#include "viewglobal.h"

#define MAX_HLCNT 20
static int selHLcnt = MAX_HLCNT;

draw_picture(pic_type)
{
	pw_writebackground(canvas_pw, 0, 0, window_get(canvas, CANVAS_WIDTH),
			window_get(canvas, CANVAS_HEIGHT), PIX_CLR);
	if(imwin==0)
		return;

	if(!scaling_done)
		scale_image();

	switch(pic_type){
	case 'e':
		printf("drawing histogram Equalized image...");
		fflush(stdout);

		imewin = im_alloc(imewin, nu*nv);
		if(imewin==0){
			fprintf(stdout, "Can't malloc imewin\n");
			return;
		}

		ShowHistWinPlot();

		hist_eq(ihist, imewin, imwin, nu*nv);
		pic_magnified = ' ';
		pic_base_type = 'e';
		display_pic(imewin, 'e');
		break;
	case 'n':
	case '\0': case ' ':
		printf("drawing normal image...");
		fflush(stdout);

		pic_magnified = ' ';
		pic_base_type = 'n';
		display_pic(imwin, 'n');
		break;
	case 'm':
		if(mbuf[1]!=0)
			sscanf(&mbuf[1], "%d %d", &sm_wd_scale, &sm_ht_scale);
		if(sm_wd_scale<0) sm_wd_scale = 1;
		if(sm_ht_scale<0) sm_ht_scale = 1;
 
		immwin = im_alloc(immwin, m_pic_wd*m_pic_ht);
		if(immwin==0){
			fprintf(stdout, "Can't malloc immwin\n");
			return;
		}

		if(pic_base_type=='n')
			cur_imwin = imwin;
		else if(pic_base_type=='e')
			cur_imwin = imewin;
		else
			break;

		if(immwin && cur_imwin){
			magpic(cur_imwin, nu, nv,
				mh_xorg, mh_xorg+mh_wd,
				mh_yorg, mh_yorg+mh_ht,
				immwin,
				m_pic_wd, m_pic_ht,
				&m_wd_scale, &m_ht_scale,
				sm_wd_scale, sm_ht_scale);
			display_pic(immwin, 'm');
			pic_magnified = 'm';
		}

		break;
	case 't':
		if(mbuf[1]!=0)
			sscanf(&mbuf[1], "%d %d", &ht_lothresh, &ht_hithresh);
		if(ht_lothresh<0)   ht_lothresh = 0;
		if(ht_hithresh>255) ht_hithresh = 255;

		if(pic_magnified=='m')
			cur_imwin = immwin;
		else if(pic_base_type=='e')
			cur_imwin = imewin;
		else
			cur_imwin = imwin;

		if(cur_imwin)
			display_pic(cur_imwin, 't');
	}
}

static unsigned char grR[CMS_SIZE], grG[CMS_SIZE], grB[CMS_SIZE];

void updategraymap()
{
  grR[BG_COLORM]= grG[BG_COLORM]= grB[BG_COLORM]= viewBGcolor;
  grR[FG_COLORM]= grG[FG_COLORM]= grB[FG_COLORM]= viewFGcolor;
  grR[0]= grG[0]= grB[0]=viewBGcolor;
  grR[BG_COLORM-1]= grG[BG_COLORM-1]= grB[BG_COLORM-1]= viewFGcolor;
  pw_setcmsname(canvas_pw, CMS_NAME);
  pw_putcolormap(canvas_pw, 0, CMS_SIZE, grR, grG, grB);
}

void updaterasmap()
{
  rasR[BG_COLORM]= rasG[BG_COLORM]= rasB[BG_COLORM]= viewBGcolor;
  rasR[FG_COLORM]= rasG[FG_COLORM]= rasB[FG_COLORM]= viewFGcolor;
  rasR[0]= rasG[0]= rasB[0]= 0;
  grR[BG_COLORM-1]= grG[BG_COLORM-1]= grB[BG_COLORM-1]= 255;
  pw_setcmsname(canvas_pw, CMS_NAME);
  pw_putcolormap(canvas_pw, 0, CMS_SIZE, rasR, rasG, rasB);
}


graycolormap(pw, sc, dc)
Pixwin *pw;
register int sc, dc;
{
	register i;

	/*  Initialize to fullgray cms  */
	for (i=0; i<TOM; i++) {
		grR[i] = grG[i] = grB[i] = sc;
		sc += dc;
		if(sc<0) sc=0;
	}
	updategraymap();
}

op_I()			/* interchange foreground & background */
{
	int t;

	if(canvas_depth==1) return;

	t = viewFGcolor;  viewFGcolor = viewBGcolor;  viewBGcolor = t;
	updategraymap();
}

op_i()
{
	if(invertgraymap){
		invertgraymap = 0;
		graycolormap(canvas_pw, 0, 1);
	}else{
		invertgraymap = 1;
		graycolormap(canvas_pw, 255, -1);
	}
}

op_C()
{
/*	if(hdtype==(RAS_MAGIC>>16) && ras_colormap.type!=RMT_NONE){
		fprintf(stdout, "hdtype=%x, type=%d len=%d\n",
			hdtype, ras_colormap.type, ras_colormap.length);
		updaterasmap();
	} */
}

op_h(ch0, ch1)			/* set hist plot mode & plot hist */
char ch0, ch1;
{
	printf(clear_line);
	reset_cursors();
	if(ch1=='f' || ch1=='w')
		hist_win_mode = ch1;

	switch(ch0){
	case 'e':
		printf("set hist plot mode to use Equalized image data\n");
		histplot_mode = 'e';
		break;
	case 'i':
		printf("set hist plot mode to Integrated\n");
		histplot_mode = 'i';
		break;
	case 'f':
	case 'w':
		hist_win_mode = ch0;
		break;
	default:
		printf("set hist plot mode to Non-Integrated\n");
		histplot_mode = ' ';
		break;
	}
	ht_markx = plot_cursor_wx; 	
	ShowHistWinPlot();
}

op0_c()				/* set clip&scale pix to (lo hi) points */
{
	printf(clear_line);
	reset_cursors();
	printf("Enter r to Reset clip&scale  points or\n");
	printf("Enter pix(lo=%f hi=%f) clip&scale points ? ",
						pixClipLo, pixClipHi);
}

op1_c(mbuf)
char *mbuf;
{
	int   n;
	float fn1, fn2;

	printf(clear_line);
	reset_cursors();
	if(mbuf[0]!=0){
		if(mbuf[0]=='r'){
			pixClipLo = -INITCLIP;
			pixClipHi =  INITCLIP;
			scaling_done = 0;
		}else{
			n = sscanf(mbuf, "%f %f", &fn1, &fn2);
			if(n==2){
				pixClipLo = fn1;
				pixClipHi = fn2;
				scaling_done = 0;
			}
		}
	}
}

op0_z()				/* set pix fill Zone(hi lo) with fill */
{
	printf(clear_line);
	reset_cursors();
	printf("Enter r to Reset Zone filling or\n");
	printf("Enter pix fill Zone & value (lo=%.3f hi=%.3f val=%.3f) ? ",
				fpixZoneLo, fpixZoneHi, fpixFillZval);
}

op1_z(mbuf)
char *mbuf;
{
	int   n;
	float fn1, fn2, fn3;

	printf(clear_line);
	reset_cursors();
	if(mbuf[0]!=0){
		if(mbuf[0]=='r'){
			fpixZoneLo = 0.0;	/* zFlo =  99.e999; */
			fpixZoneHi = 0.0;	/* zFhi = -99.e999; */
			fpixFillZval = 0.0;
			scaling_done = 0;
		}else{
			n = sscanf(mbuf, "%f %f %f", &fn1, &fn2, &fn3);
			if(n==3){
				if(fn2>fn1){
					fpixZoneLo = fn1;
					fpixZoneHi = fn2;
				}else{
					fpixZoneLo = fn2;
					fpixZoneHi = fn1;
				}
				fpixFillZval = fn3;
				scaling_done = 0;
			}
		}
	}
}

				/* set picture j in draw mode */
/*
op_jin(mbuf)
char *mbuf;
{
	printf(clear_line);
	reset_cursors();
	switch(mbuf[0]){
	case 'e':
		printf("set picture in draw Mode to histogram Equalize\n");
		pic_in_mode = 'e';
		break;
	case 'n':
	case '\0': case ' ':
		printf("set picture in draw Mode to normal\n");
		pic_in_mode = 'n';
		break;
	}
}
*/

op2_r(ch0)
char ch0;
{
	if(ch0=='w')
		printf(" Enter new canvas window size, wd ht<%d %d> ? ",
						m_pic_wd, m_pic_ht);
}

op_r(ch0)				/* reset cmd */
char ch0;
{
	printf(clear_line);
	reset_cursors();
	switch(ch0){
	case 'd':			/* redispaly image */
		repaint_canvas();
		break;

	case 'i':			/* reinit */
		printf("reinitialize hipsview\n");
		pic_in_mode   = 'n';
		pic_base_type = 'n';
		last_pic_type = 'n';
		pic_magnified = ' ';
		showsw = 'p';
		show_ap_eqsw = 'p';
		overwritesw = OVERWRITESW_OFF;
		resize(NU, NV);
		pixClipLo = -INITCLIP;
		pixClipHi =  INITCLIP;

		fpixZoneLo = fpixZoneHi = 0.0;
		fpixFillZval = 0.0;
		pixZoneLo = pixZoneHi = 0;
		pixZoneMid= pixFillZval= pixHalfDeadZ= 0;
		pixFillZval = 1;
		scaling_done = 0;
		break;

	case 's':		/* resize window to fit current image */
		resize(nu, nv);
		break;
	case 'w':
		if(mbuf[1]!=0){
			int   n, wd, ht;
			n = sscanf(&mbuf[1], "%d %d", &wd, &ht);
			if(n==2){
				if(wd > 1024) wd = 1024;
				if(ht > 1024) ht = 1024;
				resize(wd, ht);
			}
		}
		break;
	}
}

op2_s(ch)
char ch;
{
	if(ch=='m'){
		printf(" Enter mag scale(wd=%d ht=%d) ? ",
					sm_wd_scale, sm_ht_scale);
	}
}

op_s(ch0, ch1)				/* show cmds & status cmd */
char ch0, ch1;
{
	float fmax, fmin;

	reset_cursors();
	switch(ch0){
	case 'a':
		a_ulcsw = ch1;
		showsw = 'a';
		show_ap_eqsw = 'a';
		break;
	case 'p':			/* set show print to boxed pixel mode */
		showsw = 'p';
		show_ap_eqsw = 'p';
		break;
	case '0':
		selHLcnt = 1;
		break;
	case '1':
		selHLcnt = MAX_HLCNT;
		break;
	case 'w':
		showsw = 'w';
		break;
	case 't':
		showsw = 't';
		ht_markx = plot_cursor_wx; 	
		ShowHistWinPlot();
		printf("\nst mode: select lo & hi thresholds via mouse\n");
		break;
	case 'h':
		showsw = 'h';
		if(ch1=='f' || ch1=='w')
			hist_win_mode = ch1;
		if(ch1==0)
			hist_win_mode = 'f';
		ht_markx = plot_cursor_wx; 	
		ShowHistWinPlot();
		break;
	case 'm':
		if(mbuf[1]!=0)
			sscanf(&mbuf[1], "%d %d", &sm_wd_scale, &sm_ht_scale);
		if(sm_wd_scale<0) sm_wd_scale = 1;
		if(sm_ht_scale<0) sm_ht_scale = 1;

		showsw = 'm';
		immwin = im_alloc(immwin, m_pic_wd*m_pic_ht);
		if(immwin==0){
			fprintf(stdout, "Can't malloc immwin\n");
			return;
		}
		break;
	case 'x':
	case 'y':
	case 'v':
		showsw = ch0;
		doShowSlicePlot(showsw);
		break;

	case '\0': case ' ':		/* status printed */
		printf("\n\t***  STATUS ***\n");
		printf(clear_line);
/*
		printf("m_cursor_pos=(%d,%d) m_plot_cursor=(%d,%d), plot_cursor=(%d,%d)\n",
			xScaleIFm(pic_cursor_wx),  yScaleIFm(pic_cursor_wy),
			xScaleIFm(plot_cursor_wx), yScaleIFm(plot_cursor_wy),
			plot_cursor_wx, plot_cursor_wy);
*//*debug*/
		printf("cursor_pos=(%d,%d)\n", pic_cursor_wx, pic_cursor_wy);
		printf("(wd,ht) of pa_box=(%d,%d) p_box=(%d,%d) w_box(%d,%d)\n",
			pa_wd, pa_ht, p_wd, p_ht, w_wd, w_ht);
		printf("v_line from xy1(%d,%d) to xy2(%d,%d) : duv(%d,%d)\n",
				slx1, sly1, slx2, sly2, v_wd, v_ht);
		printf("mh_org=(%d,%d) mh_box(%d,%d) m_scale(%d %d)\n",
			mh_xorg, mh_yorg, mh_wd, mh_ht, m_wd_scale, m_ht_scale);
		printf("\n");
		printf("rawpix min max (%.3f %.3f)\n", Fmin, Fmax);
		printf("clip & scale points(%.3f %.3f)\n",
						 pixClipLo, pixClipHi);
		printf("fill Zone=(%d<%.3f> %d<%.3f>) fill val= %d<%.3f>\n",
			pixZoneLo, fpixZoneLo,  pixZoneHi, fpixZoneHi,
			pixFillZval, fpixFillZval);
		printf("\n");
		printf("image file name: %s\n", picfname);
		printf("image file size uXv=(%dX%d)", nu, nv);
		fprint_hdtype(stdout);
		printf("\n");
		printf("image state = <B%c.%c H%c C%c S%c>\n",
                        pic_base_type, pic_magnified,
                        hist_win_mode, cur_pic_type, showsw);

		printf(" pix8 = (rawpix - %.3f ) / %.3f - 1   /* scaling equation *\/\n",
				rawtopix8offset(), rawtopix8scale() );					
		printf("\n");
		break;
	}
}

float
rawtopix8scale()
{
	float fmax, fmin;
	fmax = (pixClipHi < Fmax) ?  pixClipHi : Fmax;
	fmin = (pixClipLo > Fmin) ?  pixClipLo : Fmin;
	return((fmax-fmin)/255.);
}
float
rawtopix8offset()
{
	float fmin;
	fmin = (pixClipLo > Fmin) ?  pixClipLo : Fmin;
	return(fmin);
}

op0_l()					/* load picture cmd */
{
	printf(clear_line);
	printf("\rEnter name of file to be loaded<%s>: ? ", picfname);
}

op1_l(mbuf)				/* load picture cmd */
char *mbuf;
{
	if(mbuf[0]!=0){
		if(mbuf[0]=='"' && picfname[0]!=0)
			load_file(picfname);
		else
			load_file(mbuf);
		if(win_wd<nu || (win_ht-PLOT_NY)<nv)
			resize(nu, nv);
		clear_plot();
	}
}

op0_w()					/*  write picture to file */
{
	printf(clear_line);
	printf("\rEnter name of file to be written<%s>: ? ", wr_fname);
}

op1_w(mbuf)				/* write picture to file */
char *mbuf;
{
	IMAGEptr *imP;
	char *mP;

	mP = &mbuf[0];
	while(*mP==' ')
		mP++;
/*printf("\n??%s\n", mP); */
	if(strcmp(mP, "view.help")==0){
		write_helpfile();
		return;
	}

	if(mbuf[0]!=0){
		if(mbuf[0]=='"')
			overwritesw = OVERWRITESW_ON;
		else if(mbuf[0]!='!')
			sscanf(mbuf, "%s", &wr_fname[0]);

 		if(picfname[0]<=' ')
			return;

		switch(showsw){
		case 'w':
			write_box_to_file(wr_fname,
					pic_cursor_wx, pic_cursor_wy);
			break;
		case 'p':
			write_pbox_to_file(wr_fname);
			break;
		case 'x':
		case 'y':
		case 'v':
			switch(hdtype){
			case PFBYTE:	imP = image1b;	break;
			/* case BYTE_HD:	imP = image1b;  break; */
			/* case USHORT_HD:	imP = image2b;  break; */
			case PFSHORT:	imP = image2b;  break;
			case PFINT:	imP = image4b;  break;
			case PFFLOAT:	imP = image4b;  break;
			}
			if(imP==0){
				printf("illegal write hdtype\n");
				return;
			}
			write_slice_to_file(wr_fname,
				slx1, sly1, slx2, sly2, imP, nu, nv);
			break;
		default:
			printf("illegal write mode s%c\n", showsw);
			break;
		}

	}
}

op0_exclamation()			/* ! cmd  execute unix system cmd  */
{
	printf(clear_line);
	printf(" Enter system cmd ? ");
}

op1_exclamation(mbuf)			/* ! execute unix system cmd */
char *mbuf;
{
	printf("\n");
	fflush(stdout);
	if(mbuf[0]!=0)
		system(mbuf);
}

op0_rt_arrow()				/* > */
{
	printf("\n"); fflush(stdout);
	printf(clear_line);
	printf("pwd: "); fflush(stdout);
	system("pwd");
	printf(" chdir< Enter new dirname> ? ");
}

op1_rt_arrow(mbuf)			/* > */
char *mbuf;
{		/* change directory from hipsview */
	if(mbuf[0]!=0){
		chdir(mbuf);
		printf("\nnew pwd: "); fflush(stdout);
		system("pwd");
	}
}

op_d(ch)			/* dt du dv */
char ch;
{
	int ht = -INFINITY;

	switch(ch){
	case 'e':
	case 'n': case '\0': case ' ':
	case 'm':
	case 't':
		printf(clear_line);
		reset_cursors();
 
		draw_picture(ch);
		break;
	case 'u':
	case 'v':
		if(showsw=='v'){
			if(mbuf[1]!=0){
				if(ch=='u'){
					sscanf(&mbuf[1], "%d %d", &v_wd, &ht);
					if(v_wd >=  nu_lim) v_wd =  nu_lim;
					if(v_wd <= -nu_lim) v_wd = -nu_lim;
					if(v_wd == 0) v_wd = 1;
				} else
					sscanf(&mbuf[1], "%d", &ht);
				if(ht!= -INFINITY){
					if(ht >=  nv_lim) ht =  nv_lim;
					if(ht <= -nv_lim) ht = -nv_lim;
					if(ht == 0) v_ht = 1;
					v_ht = ht;
				}
				doShowSlicePlot(showsw);
			}
		}else
			printf(" Err: Must be in sv mode\n");
		break;
	}
}

op2_d(ch)
char ch;			/* dt, du, dv */
{
	switch(ch){
	case 'm':
		printf(" Enter mag scale(wd=%d ht=%d) ? ",
					sm_wd_scale, sm_ht_scale);
		break;
	case 't':
		printf(" Enter thresholds(lo=%d hi=%d) ? ",
					ht_lothresh, ht_hithresh);
		break;
	case 'u':
		printf(" Enter Vector  duv<%d %d> ? ", v_wd, v_ht);
		break;
	case 'v':
		printf(" Enter Vector  dv<%d> ? ", v_ht);
		break;
	}
}

op0_u()				/* enter u__wd v__ht */
{
	switch(showsw){
	case 'w':
		printf(" Enter Write window  wd ht<%d %d> ? ", w_wd, w_ht);
		break;
	case 't':
	case 'h':
	case 'm':
		if(cur_pic_type!='m' && cur_pic_type!='t'){
		    printf(" Enter Magnify/Histogram window  wd ht<%d %d> ? ",
					mh_wd, mh_ht);
		    break;
		}
		/* FALLTHROUGH to case 'p' */
	case 'a': case 'p':
	case 'x': case 'y': case 'v':
		printf(" Enter Print/Avg window  wd ht<%d %d> ? ", pa_wd, pa_ht);
		break;
	}
}

op1_u(mbuf)			/* enter u__wd v__ht */
char *mbuf;
{
	int wd, ht;

	wd = ht = -INFINITY;
	if(mbuf[0]!=0){
		sscanf(mbuf, "%d %d", &wd, &ht);
		if(wd== -INFINITY) return;
		if(wd >= nu_lim) wd = nu_lim-1;
		if(showsw!='v' && wd < 0) wd = 1;
		switch(showsw){
		case 'w':
			w_wd = wd;
			if(w_wd&1) w_wd++;
			break;
		case 't':
		case 'h':
		case 'm':
			if(cur_pic_type!='m' && cur_pic_type!='t'){
				mh_wd = wd;
				break;
			}
			/* FALLTHROUGH to case 'p' */
		case 'a': case 'p':
		case 'x': case 'y': case 'v':
			pa_wd = wd;
			break;
		}
		if(ht!= -INFINITY){
			sprintf(&mbuf[0], "%d", ht);
			op1_v(&mbuf[0]);
		}
	}
}

op0_v()				/* enter v__ht */
{
	switch(showsw){
	case 'w':
		printf(" Enter Write window  ht<%d> ? ", w_ht);
		break;
	case 't':
	case 'h':
	case 'm':
		if(cur_pic_type!='m' && cur_pic_type!='t'){
		    printf(" Enter Magnify/Histogram window  ht<%d> ? ", mh_ht);
		    break;
		}
		/* FALLTHROUGH to case 'p' */
	case 'a': case 'p':
	case 'x': case 'y': case 'v':
		printf(" Enter Print/Avg window  ht<%d> ? ", pa_ht);
		break;
	}
}

op1_v(mbuf)			/* enter v__ht */
char *mbuf;
{
	int n;

	if(mbuf[0]!=0){
		sscanf(mbuf, "%d", &n);
		if(n >= nv_lim) n = nv_lim;
		if(showsw!='v' && n < 0) n = 1;
		switch(showsw){
		case 'w':
			w_ht = n;
			break;
		case 't':
		case 'h':
		case 'm':
			if(cur_pic_type!='m' && cur_pic_type!='t'){
				mh_ht = n;
				break;
			}
			/* FALLTHROUGH to case 'p' */
		case 'a': case 'p':
		case 'x': case 'y': case 'v':
			pa_ht = n;
			break;
		}
	}
}

op0_x()				/* enter X  Y coord */
{
	int wx = pic_cursor_wx;
	int wy = pic_cursor_wy;

	switch(showsw){
	case 'x': case 'y': case 'v':
		printf(" Enter X Y coord<%d %d>: ? ",
				pic_cursor_wx, pic_cursor_wy);
		break;	
	case 't':
	case 'h':
		if(hist_win_mode=='f')
			return;
		/* FALLTHROUGH*/
	case 'm':
		if(cur_pic_type!='m' && cur_pic_type!='t')
			wx = mh_xorg, wy = mh_yorg;
	default:
		printf(" Enter upper left X Y coord<%d %d>: ? ", wx, wy);
		break;	
	}
}

op1_x(mbuf)			/* enter X  Y(optional) coord */
char *mbuf;
{
	int n, wx, wy, *xP;

	wx = wy = -INFINITY;
	if(mbuf[0]==0)
		return;

	xP = &pic_cursor_wx;
	n = pa_wd;

	switch(showsw){		/* clip if outside of limits */
	case 'p': case 'a':
	case 'x': case 'y':
		break;
	case 'v':
		n = 0;
		break;
	case 't':
	case 'h':
		if(hist_win_mode=='f')
			return;
		/* FALLTHROUGH*/
	case 'm':
		if(cur_pic_type!='m' && cur_pic_type!='t'){
			n  = mh_wd;
			xP = &mh_xorg;
		}
		break;
	case 'w':
		n = w_wd;
		break;
	default:
		return;
	}

	sscanf(mbuf, "%d %d", &wx, &wy);
	if(wx== -INFINITY)
		return;

	if(wx < 0)
		wx = 0;
	else if(wx > nu_lim - n)
		wx = nu_lim - n;

	*xP = wx;

	switch(showsw){
	case 'x': case 'y': case 'v':
		if(cur_pic_type=='m' || (cur_pic_type=='t' && pic_magnified))
			wx = x_iScale(wx);
		pic_cursor_wx = wx;
		doShowSlicePlot(showsw);
		break;
	}
	if(wy!= -INFINITY){
		sprintf(&mbuf[0], "%d", wy);
/*		printf("\nwy = %s\n", mbuf); */
		op1_y(&mbuf[0]);
	}
	printf(clear_line);
}

op0_y()				/* enter Y coord */
{
	int wy = pic_cursor_wy;

	switch(showsw){
	case 'x': case 'y': case 'v':
		printf(" Enter Y coord<%d>: ? ", pic_cursor_wy);
		break;	
	case 't': case 'h': case 'm':
		if(cur_pic_type!='m' && cur_pic_type!='t')
			wy = mh_yorg;
	default:
		printf(" Enter upper left Y coord<%d>: ? ", wy);
		break;	
	}
}

op1_y(mbuf)			/* enter Y coord */
char *mbuf;
{
	int n, wy, *yP;

	if(mbuf[0]==0)
		return;

	yP = &pic_cursor_wy;
	n = pa_ht;

	switch(showsw){		/* clip if outside of limits */
	case 'p': case 'a':
	case 'x': case 'y':
		break;
	case 'v':
		n = 0;
		break;
	case 't':
	case 'h':
		if(hist_win_mode=='f')
			return;
		/* FALLTHROUGH*/
	case 'm':
		if(cur_pic_type!='m' && cur_pic_type!='t'){
			n  = mh_ht;
			yP = &mh_yorg;
		}
		break;
	case 'w':
		n = w_ht;
		break;
	default:
		return;
	}

	wy = *yP;
	sscanf(mbuf, "%d", &wy);

	if(wy < 0)
		wy = 0;
	else if(wy > nv - n)
		wy = nv - n;

	*yP = wy;

	switch(showsw){
	case 'x': case 'y': case 'v':
		if(cur_pic_type=='m' || (cur_pic_type=='t' && pic_magnified))
			wy = y_iScale(wy);
		pic_cursor_wy = wy;
		doShowSlicePlot(showsw);
		break;
	}
	printf(clear_line);
}

int
set_pic_cursor_to_minmax(code)
{
	int pwx, pwy;
	int done = 1;

	pwx = xScaleIFm(pic_cursor_wx);
	pwy = yScaleIFm(pic_cursor_wy);

	switch(code){
	case 'H':
		if(ap_imst.maxcnt==1){
			if(pwx==ap_imst.max_ix && pwy==ap_imst.max_iy){
				printf(" High peak FOUND\n");
			}else{
				pwx = ap_imst.max_ix;
				pwy = ap_imst.max_iy;
				done = 0;
			}
		} else
			printf("too many %d High's\n", ap_imst.maxcnt);
		break;
	case 'L':
		if(ap_imst.mincnt==1){
			if(pwx==ap_imst.min_ix && pwy==ap_imst.min_iy){
				printf(" Low valley FOUND\n");
			}else{
				pwx = ap_imst.min_ix;
				pwy = ap_imst.min_iy;
				done = 0;
			}
		} else
			printf("too many %d Low's\n", ap_imst.mincnt);
		break;
	default:
		printf("default Err set_pic_cursor_to_minmax(%c)\n", code);
		return(1);
	}

	if(cur_pic_type=='m'){
		pwx = x_iScale(pwx);
		pwy = y_iScale(pwy);
	}
	pic_cursor_wx = pwx;
	pic_cursor_wy = pwy;

	doShowSlicePlot(showsw);
	update_slice_cursor_to_match_plot();
	display_cursor(pic_cursor_wx, pic_cursor_wy);
	return(done);
}

op_HL(c)
{
	int mx, my;
	int cnt = selHLcnt;

	do{
		mx = xScaleIFm(pic_cursor_wx) - (pa_wd-1)/2;
		my = yScaleIFm(pic_cursor_wy) - (pa_ht-1)/2;
		if(mx<pa_wd || my<pa_ht || mx>nu-pa_wd || my>nv-pa_ht){
			printf("Too close to boarder\n");
			break;
		}
		if(--cnt<0){
			if(selHLcnt==1)
				printf("HL iterate off done\n");
			else
				printf("HL iterate on done\n");
			break;
		}
		getimstatistics(&ap_imst, mx, my, pa_wd, pa_ht, nu, nv);
	}while(set_pic_cursor_to_minmax(c)==0);
}

op0_percent()			/* %  enter new print pixel format */
{
	switch(hdtype){
	case PFBYTE:
		printf(" Enter new BYTE format<%s>: ", UBfmt);
		break;
/*	case BYTE_HD:
		printf(" Enter new BYTE format<%s>: ", Bfmt);
		break; */
	case PFSHORT:
		printf(" Enter new SHORT format<%s>: ", Sfmt);
		break;
/*	case USHORT_HD:
		printf(" Enter new USHORT format<%s>: ", USfmt);
		break; */
	case PFINT:
		printf(" Enter new INT format<%s>: ", Lfmt);
		break;
	case PFFLOAT:
		printf(" Enter new FLOAT format<%s>: ", Ffmt);
		break;
	default:
		printf(" Header type error\n");
	}
}

op1_percent(mbuf)		/* %  enter new print pixel format */
char *mbuf;
{
	if(mbuf[0]!=0){
		switch(hdtype){
		case PFBYTE:	strcpy(UBfmt, mbuf);	break;
		/* case BYTE_HD:	strcpy(Bfmt, mbuf);	break; */
		case PFSHORT:	strcpy(Sfmt, mbuf);	break;
		/* case USHORT_HD:	strcpy(USfmt, mbuf);	break; */
		case PFINT:	strcpy(Lfmt, mbuf);	break;
		case PFFLOAT:	strcpy(Ffmt, mbuf);	break;
		}
	}
}

op_pabox(fP, pasw)		/* print boxed pixels or bpoxed avg value */
FILE *fP;
{
	int pwx, pwy, L_showsw;

	L_showsw = (pasw=='A'||pasw=='P') ? pasw : showsw;

	if(cur_pic_type=='m'){
		printf("\nmagnify orgin(%d,%d), scale(%d,%d)\n",
			mh_xorg, mh_yorg, m_wd_scale, m_ht_scale);
	}

	switch(L_showsw){
	case 'a': case 'A':
		pwx = xScaleIFm(pic_cursor_wx);
		pwy = yScaleIFm(pic_cursor_wy);
		printPaAvg(a_ulcsw, pwx, pwy);
		break;
	case 'h':
		pwx = xScaleIFm(mh_xorg);
		pwy = yScaleIFm(mh_yorg);
		if(pasw=='p'){
			fprint_box(fP, pwx, pwy, pwx, pwy);
		}else{
			if(hist_win_mode=='w'){
				printHwAvg('u', pwx, pwy);
			}else
				printHfAvg(nu, nv);
		}
		break;
	case 'p': case 'P':
	case 't':
		pwx = xScaleIFm(pic_cursor_wx);
		pwy = yScaleIFm(pic_cursor_wy);
		if(pasw=='p'||pasw=='P'){
			fprint_box(fP, pwx, pwy, pwx, pwy);
		}else{
			printPaAvg('u', pwx, pwy);
		}
		break;
	case 'm':
		if(cur_pic_type=='m'){
			pwx = xScaleIFm(pic_cursor_wx);
			pwy = yScaleIFm(pic_cursor_wy);
		}else{
			pwx = xScaleIFm(mh_xorg);
			pwy = yScaleIFm(mh_yorg);
		}
		if(pasw=='p'){
			fprint_box(fP, pwx, pwy, pwx, pwy);
		}else{
			printPaAvg('u', pwx, pwy);
		}
		break;
	case 'x':
	case 'y':
		pwx = xScaleIFm(pic_cursor_wx);
		pwy = yScaleIFm(pic_cursor_wy);
		if(pasw=='p'){
		    fprint_box(fP, pwx-(pa_wd-1)/2, pwy-(pa_ht-1)/2, pwx, pwy);
		}else{
		    printPaAvg( 'u', pwx-(pa_wd-1)/2, pwy-(pa_ht-1)/2);
		}
		break;
	case 'v':
		pwx = xScaleIFm(vmark_wx);
		pwy = yScaleIFm(vmark_wy);
		if(pasw=='p'){
		    fprint_box(fP, pwx-(pa_wd-1)/2, pwy-(pa_ht-1)/2, pwx, pwy);
		}else{
		    printPaAvg('u', pwx-(pa_wd-1)/2, pwy-(pa_ht-1)/2);
		}
		break;
	}
}

			/* set frame Lable type  */
/*
op_L(mbuf)
char *mbuf;
{
	frame_lable_sw = (mbuf[0]=='s') ? 's' : 'f';
}
*/

op_D(ch0)			/* enable|disable displaying on sun */
char ch0;
{
	if(ch0=='0')	  isDwin = 0;
	else if(ch0=='1') isDwin = 1;
	else if(ch0=='2') isDwin = 2;
}

op0_q()				/* quit */
{
	fprintf(stdout, "Type 'y' return to Confirm quit: ");
}

op1_q(mbuf)			/* quit */
char *mbuf;
{
	if(mbuf[0]=='y'){
		putchar('\n');
		window_set(base_frame, FRAME_NO_CONFIRM, TRUE, 0);
		quit_proc();
	}
}

