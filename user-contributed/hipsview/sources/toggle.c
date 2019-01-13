#include <stdio.h>
#include "hipsview.h"
#include <hipl_format.h>
#include <sunwindow/window_hs.h>
#include <suntool/canvas.h>
#include <suntool/sunview.h>
#include <sunwindow/win_input.h>

#define GLOBAL extern
#include "viewglobal.h"

toggle_disp()
{
	int Cpic_wx, Cpic_wy, Mpic_wx, Mpic_wy;
	int lnu, lnv;
	struct pixrect	*toggle_mprP, *get_mprP();

	reset_cursors();

	if(toggle_pic!=last_pic_type){
		toggle_mprP = get_mprP(last_pic_type);
		toggle_pic  = cur_pic_type = last_pic_type;
	}else{
		if(pic_magnified=='m'
				&& (last_pic_type=='t'||last_pic_type=='e') ){
			toggle_mprP = get_mprP('m');
			cur_pic_type = 'm';
		}else{
			toggle_mprP = get_mprP(pic_base_type);
			cur_pic_type = pic_base_type;
		}
		toggle_pic = pic_base_type;
	}

	if(cur_pic_type=='m' || (cur_pic_type=='t'&&pic_magnified=='m')){
		lnu = m_pic_wd;	lnv = m_pic_ht; 
	}else{
		lnu = nu;	lnv = nv;
	}

	lable_pic();
	pw_write(canvas_pw, 0, 0, lnu, lnv, PIX_SRC, toggle_mprP, 0, 0);

	if(pic_magnified!='m' || (last_pic_type=='t' && pic_magnified=='m') ){
		Cpic_wx = pic_cursor_wx;
		Cpic_wy = pic_cursor_wy;
		if(showsw=='v'){
			Mpic_wx = vmark_wx;
			Mpic_wy = vmark_wy;
		}
	}else{
		if(cur_pic_type=='m'){
			Cpic_wx = x_iScale(pic_cursor_wx);
			Cpic_wy = y_iScale(pic_cursor_wy);
			if(showsw=='v'){
				Mpic_wx = x_iScale(vmark_wx);
				Mpic_wy = y_iScale(vmark_wy);
			}
		}else{
			Cpic_wx = x_Scale(pic_cursor_wx);
			Cpic_wy = y_Scale(pic_cursor_wy);
			if(showsw=='v'){
				Mpic_wx = x_Scale(vmark_wx);
				Mpic_wy = y_Scale(vmark_wy);
			}
		}
	}

/* printf("tog %c type=%c, pic_cursor=(%d %d) Cpic=(%d %d)\n",
		toggle_pic, cur_pic_type,
		pic_cursor_wx, pic_cursor_wy, Cpic_wx, Cpic_wy);
*/ /*debug*/

	pic_cursor_wx = Cpic_wx;
	pic_cursor_wy = Cpic_wy;
	if(showsw=='v'){
		vmark_wx = Mpic_wx;
		vmark_wy = Mpic_wy;
	}
	doShowSlicePlot(showsw);	/* updates nu_lim, nv_lim */

	if(showsw=='v'){
		if(Mpic_wx<0) Mpic_wx = 0;
		else if(Mpic_wx>lnu) Mpic_wx = lnu-4;
		if(Mpic_wy<0) Mpic_wy = 0;
		else if(Mpic_wy>lnv) Mpic_wy = lnv-1;
		window_set(canvas, WIN_MOUSE_XY, Mpic_wx, Mpic_wy, 0);
	}else{
		if(Cpic_wx<0) Cpic_wx = 0;
		else if(Cpic_wx>lnu) Cpic_wx = lnu-4;
		 	/* the -4 keeps part of cursor arrow in view */
		if(Cpic_wy<0) Cpic_wy = 0;
		else if(Cpic_wy>lnv) Cpic_wy = lnv-1;
		window_set(canvas, WIN_MOUSE_XY, Cpic_wx, Cpic_wy, 0);
	}

	update_win_cursor(TRUE);
}

