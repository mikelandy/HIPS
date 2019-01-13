#include <stdio.h>
#include "hipsview.h"
#include <hipl_format.h>
#include <sunwindow/window_hs.h>
#include <suntool/canvas.h>
#include <suntool/sunview.h>
#include <sunwindow/win_input.h>
#include <sunwindow/cms.h>
#include <sunwindow/cms_rainbow.h>

#define GLOBAL extern
#include "viewglobal.h"

#define CMDMSK 0xF000
#define CMD1   0x4000
#define CMD2   0x8000
static int ccmd = 0;
static int ms_left, ms_middle, ms_right;
static int cursor_op = PIX_SRC^PIX_DST;
static int toggle_cursor_op = 0;

static char *mbp;

menu2chcmd(s)
char *s;
{
	if(*s!=0){
		ccmd = *s+CMD1;
		s++;
		mbp = &mbuf[0];
		while(*mbp = *s){
			mbp++; s++;
		}
	}
}

menu1chcmd(opch)
char opch;
{
	ccmd = opch+CMD1;
	overwritesw = OVERWRITESW_OFF;
	fflush(stdout);
	fflush(stderr);
}


void mouse_proc(dcanvas, event)
Canvas dcanvas;
Event *event;
{
	register int cc;

	cc = event_id(event);
	if(cc<=ASCII_LAST && event_is_up(event) )
		return;			/* prevents duplicate prints ?? */

	if(ccmd){
		if(cc!='\r'&&cc!='\n'){
			if(mbp>=&mbuf[80]){
				ccmd==0;
				return;
			}
			if(cc<=ASCII_LAST){
				if(cc=='\b'){
					mbp--;
					putchar('\b');
					putchar(' ');
				}else{
					*mbp++ = cc;
					if(cc==' '&&(ccmd&CMDMSK)==CMD2){
						cc = ccmd;
						*mbp = '\0';
						goto L_cmd2;
					}
				}
				putchar(cc);
			}
			flushcmd(cc);
			return;
		}else{
			if(ccmd&CMDMSK)
				ccmd = (ccmd & ~CMDMSK) | CMD1;
		}
			
		cc = ccmd;
		*mbp = '\0';
	}else
		mbp = mbuf;
 /*	ccmd = 0; */
   L_cmd2:
	if(Cur_Pic_Type=='m' || (Cur_Pic_Type=='t' && pic_magnified=='m') ){
		nu_lim = m_pic_wd;
		nv_lim = m_pic_ht;
	}else{
		nu_lim = nu;
		nv_lim = nv;
	}

	mouse_cmd(cc, event);
}

mouse_cmd(opc, event)
int opc;
Event *event;
{
	register int lwd, lht;
	register int wx, wy, lwx, lwy, pwx, pwy;
	int ropc;

	if(opc > META_LAST){
		ms_left   = (int)window_get(canvas, WIN_EVENT_STATE, MS_LEFT);
		ms_middle = (int)window_get(canvas, WIN_EVENT_STATE, MS_MIDDLE);
		ms_right  = (int)window_get(canvas, WIN_EVENT_STATE, MS_RIGHT);
	}

	switch(opc){ 
	case MS_RIGHT:
		if(event_is_down(event)){
			if(ms_left)
				return;
			wx = event_x(event);
			wy = event_y(event);
			if(ms_middle){
				rtmouse_toggle(wx, wy);
		 	}else{
				display_menus(event);
				update_cursor(wx, wy);
				lable_pic();
				doShowSlicePlot(showsw);
			}
		}
		return;

	case MS_MIDDLE:
		if(event_is_down(event)){
			wx = event_x(event);
			wy = event_y(event);

			if(ms_left && !ms_right){
				/* set initial rubberband cursor position */
				clear_plot();
				switch(showsw){
				case 't':
				case 'h':
					if(Cur_Pic_Type=='t'){
						lwd = pa_wd; lht = pa_ht;
						break;
					}
					if(hist_win_mode=='f')
						return;
				case 'm':
					if(Cur_Pic_Type=='m'){
						lwd = pa_wd; lht = pa_ht;
					}else{
						lwd = mh_wd; lht = mh_ht;
						window_set(canvas, WIN_MOUSE_XY,
							   mh_xorg+lwd,
							   mh_yorg+lht,
							   0);
						return;
					}

				case 'a':
				case 'p':
					lwd = pa_wd; lht = pa_ht;
					break;
				case 'v':
					lwd = v_wd; lht = v_ht;
					break;
				case 'w':
					lwd = w_wd; lht = w_ht;
					break;
				default:
					return;
				}
				if(Cur_Pic_Type=='m'){
					lwd *= m_wd_scale; lht *= m_ht_scale;
				}
				window_set(canvas, WIN_MOUSE_XY,
					   pic_cursor_wx+lwd, 
					   pic_cursor_wy+lht,
					   0);
			}
			if(ms_left || ms_right)
				return;

			lwx = (wx>nu_lim) ? nu_lim-1 : wx;
			lwy = wy;

			switch(showsw){
			default:
			case 'a':
				pwx = xScaleIFm(pic_cursor_wx);
				pwy = yScaleIFm(pic_cursor_wy);
				printPaAvg(a_ulcsw, pwx, pwy);
				break;
			case 'p':
				pwx = xScaleIFm(pic_cursor_wx);
				pwy = yScaleIFm(pic_cursor_wy);
				fprint_box(stdout, pwx, pwy, pwx, pwy);
				break;
			case 'w':
				break;
			case 'h':
				ht_markx = lwx;
				ShowHistWinPlot();
				break;
			case 'm':
				if(Cur_Pic_Type!='m'){
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
				}
				break;
			case 't':
				if(wy>nv){
					reset_cursors();
					if(pic_magnified=='m')
						cur_imwin = immwin;
					else if(pic_base_type=='e')
						cur_imwin = imewin;
					else
						cur_imwin = imwin;

					if(ht_select==0){
						ht_lothresh = lwx;
						ht_hithresh = lwx;
						ht_select = 1;
						ShowHistWinPlot();
					}else{
						ht_hithresh = lwx;
						if(cur_imwin)
						    display_pic(cur_imwin, 't');
						ht_select = 0;
					}
				}
				break;
			case 'v':
				if(wy<nv_lim){
					lwx = onvec_wx(lwx);
					lwy = onvec_wy(lwy);
				}
				/* FALLTHROUGH*/
			case 'x':
			case 'y':
				ShowSlicePlot(showsw,
						pic_cursor_wx, pic_cursor_wy,
						lwx, lwy);
				update_slice_cursor_to_match_plot();
				display_cursor(pic_cursor_wx, pic_cursor_wy);
				break;
			}
		}else{
			if(ms_left && !ms_right){
			    if((showsw=='m'||showsw=='h'||showsw=='t') &&
					Cur_Pic_Type!='m' && Cur_Pic_Type!='t'){
				window_set(canvas, WIN_MOUSE_XY,
					   mh_xorg, mh_yorg, 0);
			    }else
				window_set(canvas, WIN_MOUSE_XY,
					   pic_cursor_wx, pic_cursor_wy, 0);
			}
		}
		return;

	case MS_LEFT:
		wx = event_x(event);
		wy = event_y(event);

		show_cursor = TRUE;
		if(event_is_down(event)){
			if(ms_middle || ms_right)
				return;

			if(wy>nv_lim){
				switch(showsw){
				case 'x':
					wx = plot_cursor_wx;
					wy = sly1;
					break;
				case 'y':
					wx = slx1;
					wy = plot_cursor_wx;
					break;
				case 'v':
					wx = pic_cursor_wx;
					wy = pic_cursor_wy;
					break;

				case 't':
				case 'h':
					if(hist_win_mode=='f')
						return;
				case 'm':
					if(Cur_Pic_Type=='m'){
						wx = mh_xorg;
						wy = mh_yorg;
					}else{
						wx = pic_cursor_wx;
						wy = pic_cursor_wy;
					}
					show_cursor = FALSE;
					break;

				case 'p': case 'a':
					show_cursor = FALSE;
					/*FALLTHROUGH to m*/
				case 'w':
					wx = pic_cursor_wx;
					wy = pic_cursor_wy;
					break;
				}
				lwx = (wx>nu_lim) ? nu_lim-1 : wx;
				lwy = (wy>nv_lim) ? nv_lim-1 : wy;
				display_cursor(lwx, lwy);
				pic_cursor_wx = lwx;
				pic_cursor_wy = lwy;
				window_set(canvas, WIN_MOUSE_XY, lwx, lwy, 0);
			}else
				window_set(canvas, WIN_MOUSE_XY, wx, wy, 0);

		}else{
			if(ms_middle && !ms_right){
			    if((showsw=='m'||showsw=='h'||showsw=='t') &&
					Cur_Pic_Type!='m' && Cur_Pic_Type!='t'){
				window_set(canvas, WIN_MOUSE_XY,
					   mh_xorg, mh_yorg, 0);
			    }else
				window_set(canvas, WIN_MOUSE_XY,
					   pic_cursor_wx, pic_cursor_wy, 0);
			}
		}

		update_win_cursor(show_cursor);

		goto L_leftsw_drag;

	case LOC_DRAG:
		if(ms_left && ms_middle && !ms_right){
					/* rubberband box or vector */
			wx = event_x(event);
			wy = event_y(event);
			switch(showsw){
			case 't':
				if(Cur_Pic_Type=='t')
					goto L_DRAGap;
				else
					goto L_DRAGh;
			case 'w':
			case 'v':
			case 'p': case 'a':
		L_DRAGap:
				lwd = wx - pic_cursor_wx;
				lht = wy - pic_cursor_wy;

				if(showsw=='v'){
					if(Cur_Pic_Type!='m'){
						v_wd  = lwd;
						v_ht  = lht;
					}else{
						v_wd  = lwd/m_wd_scale;
						v_ht  = lht/m_ht_scale;
					}
					break;
				}

				if(lwd<0){
					lwd = -lwd;
					pic_cursor_wx -= lwd;
				}
				if(lht<0){
					lht = -lht;
					pic_cursor_wy -= lht;
				}
			
				if(showsw=='p' || showsw=='a' || showsw=='t'){
					pa_wd = lwd;  pa_ht = lht;
				}else{			/* (showsw=='w') */
					w_wd = lwd;  w_ht = lht;
					if(w_wd&1) w_wd++;
				}
				break;
			case 'h':
		L_DRAGh:
				if(hist_win_mode=='f')
					return;
			case 'm':
				lwd = wx - mh_xorg;
				lht = wy - mh_yorg;
				if(lwd<0){
					lwd = -lwd;
					mh_xorg -= lwd;
				}
				if(lht<0){
					lht = -lht;
					mh_yorg -= lht;
				}

				if(Cur_Pic_Type=='m'){
					pa_wd  = lwd;  pa_ht = lht;
					pa_wd -= lwd>>2;
					pa_ht -= lht>>2;
				}else{
					mh_wd = lwd;  mh_ht = lht;
					display_cursor(mh_xorg, mh_yorg);
					return;
				}
				break;
			default:
				return;
			}

			display_cursor(pic_cursor_wx, pic_cursor_wy);
			return;
		}
		goto L_still;

	case LOC_RGNEXIT:
	case LOC_STILL:
	   L_still:
		wx = event_x(event);
		wy = event_y(event);

	   L_leftsw_drag:
		if(!ms_left || ms_middle || ms_right)
			return;

		switch(showsw){
		case '\0':
		case 'p': case 'a':
			lwd = pa_wd;  lht = pa_ht;
			break;
		case 'w':
			lwd = w_wd;  lht = w_ht;
			break;
		case 't':
		case 'h':
			if(Cur_Pic_Type=='t'){
				lwd = pa_wd;  lht = pa_ht;
				break;
			}
			if(hist_win_mode=='f')
				return;
		case 'm':
			if(Cur_Pic_Type=='m'||Cur_Pic_Type=='t'){
				lwd = pa_wd;  lht = pa_ht;
			}else{
				lwd = mh_wd;  lht = mh_ht;
			}
			break;
		default:
			lwd = lht = 1;
			break;
		}

		if(wx < 0)
			wx = 0;
		else if(wx > nu_lim - lwd)
			wx = nu_lim - lwd;

		if(wy < 0)
			wy = 0;
		else if(wy > nv_lim - lht)
			wy = nv_lim - lht;

		display_cursor(wx, wy);
		if( (showsw=='m' || showsw=='h' || showsw=='t') &&
				Cur_Pic_Type!='m' && Cur_Pic_Type!='t'){
			mh_xorg = wx;
			mh_yorg = wy;
		}else{
			pic_cursor_wx = wx; /*** get new cursor position ***/
			pic_cursor_wy = wy;
		}

		if(opc!=LOC_DRAG){
			switch(showsw){
			case 'x':
			case 'y':
			case 'v':
				ShowSlicePlot(showsw, wx, wy, wx, wy);
				break;
			case 't': 
			case 'h': 
				if(Cur_Pic_Type=='t')
					break;
				ht_markx = wx;
				ShowHistWinPlot();
				break;
			}
		}
	
		return;

	case LOC_MOVE:
		return;

		/****** single char op_cmd ASCII events ********/
	case 'I': op_I(); return;
	case 'i': op_i(); return;
	case 'C': op_C(); return;

	case '"':
		overwritesw = OVERWRITESW_ON;
		mbuf[0] = '"', mbuf[1] = '\0';
		op1_w(mbuf);
		break;
	case '=':
		op_pabox(stdout, show_ap_eqsw);
		break;
	case 'a':
	case 'p':
		show_ap_eqsw = opc;
		op_pabox(stdout, show_ap_eqsw);
		break;
	case 'H':
	case 'L':
		op_HL(opc);
		break;

	case '/':
		rtmouse_toggle(event_x(event), event_y(event));
		return;
	case 't':
	case '.':
		toggle_disp();
		return;
	case ',':
		if(toggle_cursor_op){
			cursor_op = PIX_SRC^PIX_DST;
			toggle_cursor_op = 0;
		}else{
			cursor_op = PIX_SRC|PIX_DST;
			toggle_cursor_op = 1;
		}
		update_win_cursor(show_cursor);
		return;

	default:
		if(opc<CMD1)
			ropc = op_cmd0(opc);
		else if(opc<CMD2)
			ropc = op_cmd1(opc);
		else
			ropc = op_cmd2(opc);
	}

/*	if(ropc== -1)
		printf(" No such cmd %c ? ", opc);	/* debug */

	if(ropc<=0){
		ccmd = 0;
		prompt();
	} else
		ccmd = ropc;

	flushcmd(opc);
}

int
op_cmd0(opc)
int  opc;
{
	int cmd = CMD1;

	switch(opc){
	case 'f':
	case 'l': op0_l();	break;
	case 'w': op0_w();	break;
	case 'u': op0_u();	break;
	case 'v': op0_v();	break;
	case 'x': op0_x();	break;
	case 'y': op0_y();	break;
	case 'c': op0_c();	break;
	case 'z': op0_z();	break;

	case 'q':
	case 'Q': op0_q();		break;
	case '!': op0_exclamation();	break;
	case '>': op0_rt_arrow();	break;
	case '%': op0_percent();	break;

	case '?':
	case 'h':
	case 'D':
		putchar(opc);
		break;
	case 'd':
	case 's':
	case 'r':
		cmd = CMD2;
		putchar(opc);
		break;
	default:
		return(-1);
	}

	return(opc+cmd);
}

int
op_cmd1(opc)
int  opc;
{
	switch(opc){
	case 'f'+CMD1:
	case 'l'+CMD1: op1_l(mbuf);	break;
	case 'w'+CMD1: op1_w(mbuf);	break;
	case 'u'+CMD1: op1_u(mbuf);	break;
	case 'v'+CMD1: op1_v(mbuf);	break;
	case 'x'+CMD1: op1_x(mbuf);	break;
	case 'y'+CMD1: op1_y(mbuf);	break;
	case 'c'+CMD1: op1_c(mbuf);	break;
	case 'z'+CMD1: op1_z(mbuf);	break;

	case 'q'+CMD1:
	case 'Q'+CMD1: op1_q(mbuf);		break;
	case '!'+CMD1: op1_exclamation(mbuf);	break;
	case '>'+CMD1: op1_rt_arrow(mbuf);	break;
	case '%'+CMD1: op1_percent(mbuf);	break;

	case '?'+CMD1: op_help(mbuf);		break;
	case 'h'+CMD1: op_h(mbuf[0],mbuf[1]);	break;
	case 'D'+CMD1: op_D(mbuf[0]);		break;

	case 'd'+CMD1: op_d(mbuf[0]);		break;
	case 'r'+CMD1: op_r(mbuf[0]);		break;
	case 's'+CMD1: op_s(mbuf[0],mbuf[1]);	break;
	default:
		return(-1);
	}
	return(0);
}

int
op_cmd2(opc)
int  opc;
{
	register char ch = mbuf[0];

	switch(opc){
	case 'd'+CMD2:
		if(ch=='m' || ch=='t' || ch=='u' || ch=='v')
			op2_d(ch);
		return('d'+CMD1);
	case 'r'+CMD2:
		if(ch=='w')
			op2_r(ch);
		return('r'+CMD1);
	case 's'+CMD2:
		if(ch=='m')
			op2_s(ch);
		return('s'+CMD1);

	default:
		return(-1);
	}
}

prompt()
{
	printf(clear_line);
	printf("? ");
	if( !(showsw=='m' || showsw=='t' || showsw=='h') )
		display_cursor(pic_cursor_wx, pic_cursor_wy);
}

flushcmd(opc)
{
	int lnv = nv_lim;
	int lnu = nu_lim;

	pw_vector(canvas_pw,  0, lnv,   lnu,  lnv,   PIX_SET, 1);
	pw_vector(canvas_pw,  0, lnv+1, lnu,  lnv+1, PIX_CLR, 1);
	pw_vector(canvas_pw, lnu,  0,   lnu,  lnv,   PIX_SET, 1);
/*	pw_vector(canvas_pw,  0,   0,   lnu,   0,    PIX_SET, 1);
	pw_vector(canvas_pw,  0,   0,    0,   lnv,   PIX_SET, 1);
*/
	lable_pic();

	if(opc!='w'+CMD1)
		overwritesw = OVERWRITESW_OFF;
	fflush(stdout);
	fflush(stderr);
}

rtmouse_toggle(wx, wy)
{
	register int cur_wx, cur_wy;

	if(wy<nv_lim){
			/* toggle mouse position cursor	or */
			/*  arrow between picture & plot.  */
		switch(showsw){
		case 'x':
		case 'y':
		case 'v':
			cur_wx = plot_cursor_wx;
			cur_wy = plot_cursor_wy;
			goto L_right;
		}
	}

	switch(showsw){
	case 'm':
		toggle_disp();
		if(Cur_Pic_Type!='m'){
			cur_wx = x_Scale(pic_cursor_wx);
			cur_wy = y_Scale(pic_cursor_wy);
		}else{
			cur_wx = pic_cursor_wx;
			cur_wy = pic_cursor_wy;
		}
		break;

	case 't':
	case 'h':
		if(Cur_Pic_Type=='t'){
			cur_wx = pic_cursor_wx;
			cur_wy = pic_cursor_wy;
			break;
		}
		cur_wx = mh_xorg;
		cur_wy = mh_yorg;
		break;
	case 'p': case 'a':
	case 'w':
		cur_wx = pic_cursor_wx;
		cur_wy = pic_cursor_wy;
		break;
	case 'x':
		cur_wx = plot_cursor_wx;
		cur_wy = sly1;
		break;
	case 'y':
		cur_wx = slx1;
		cur_wy = plot_cursor_wx;
		break;
	case 'v':
		cur_wx = vmark_wx;
		cur_wy = vmark_wy;
		break;
	default:
		return;
	}

  L_right:
	if(cur_wx<0) cur_wx = 0;
	else if(cur_wx>=nu_lim) cur_wx = nu_lim-1;  /*win_wd*/
	if(cur_wy<0) cur_wy = 0;
	else if(cur_wy>=win_ht) cur_wy = win_ht-1;
	update_cursor(cur_wx, cur_wy);
}

update_cursor(wx, wy)
int wx, wy;
{
	if( ((showsw=='m' || showsw=='h') && Cur_Pic_Type!='m')
		    || (showsw=='t' && hist_win_mode=='w') )
		display_cursor(mh_xorg, mh_yorg);
	else
		display_cursor(pic_cursor_wx, pic_cursor_wy);

	window_set(canvas, WIN_MOUSE_XY, wx, wy, 0);
	update_win_cursor(TRUE);
}

update_win_cursor(show_cursor)
{
	cursor = window_get(canvas, WIN_CURSOR);
	cursor_set(cursor, CURSOR_SHOW_CURSOR, show_cursor, 0);
	cursor_set(cursor, CURSOR_OP, cursor_op, 0);
	window_set(canvas, WIN_CURSOR, cursor, 0);
}

