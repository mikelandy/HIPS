#include <stdio.h>
#include <suntool/sunview.h>
#include <suntool/walkmenu.h>
#include <suntool/canvas.h>

extern Menu viewmenu;
extern Canvas canvas;
extern op_I(), op_i(), op_C(), op_D();
extern help_slice(), help_file(), help_show(), help_hist(), help_draw();
extern help_reset(), help_render(), help_param(), help_mouse(), help_misc();

static menu_x, menu_y;
static R()
{
		/* updates arrow cursor before executing mouse cmd */
	update_cursor(menu_x, menu_y);
}

OP_sx() { R(); op_s('x', 0); }
OP_sy() { R(); op_s('y', 0); }
OP_sv() { R(); op_s('v', 0); }
OP_A()  { R(); op_pabox(stdout, 'A'); }
OP_P()  { R(); op_pabox(stdout, 'P'); }
OP_H()  { R(); op_HL('H'); }
OP_L()  { R(); op_HL('L'); }
OP_s0() { op_s('0', 0); }
OP_s1() { op_s('1', 0); }


OP_l()  { R(); op0_l(); menu1chcmd('l'); }
OP_w()  { R(); op0_w(); menu1chcmd('w'); }
OP_sw() { op_s('w', 0); }
OP_q()  { op0_q(); menu1chcmd('q'); }

OP_sa() { op_s('a', 0); }
OP_sm() { op2_s('m'); menu2chcmd("sm"); }
OP_sp() { op_s('p', 0); }
OP_st() { op_s('t', 0); }

OP_dn() { R(); op_d('n'); }
OP_de() { R(); op_d('e'); }
OP_dm() { op2_d('m'); menu2chcmd("dm"); }
OP_dt() { R(); op_d('t'); }

OP_rd() { R(); op_r('d'); }
OP_ri() { R(); op_r('i'); }
OP_rs() { R(); op_r('s'); }
OP_rw() { op2_r('w'); menu2chcmd("rw"); }

OP_D0() { op_D('0'); }
OP_D1() { op_D('1'); }
OP_D2() { op_D('2'); }

OP_u() { op0_u(); menu1chcmd('u'); }
OP_v() { op0_v(); menu1chcmd('v'); }
OP_x() { op0_x(); menu1chcmd('x'); }
OP_y() { op0_y(); menu1chcmd('y'); }
OP_du(){ op2_d('u'); menu2chcmd("du"); }
OP_dv(){ op2_d('v'); menu2chcmd("dv"); }
OP_c() { op0_c(); menu1chcmd('c'); }
OP_z() { op0_z(); menu1chcmd('z'); }
OP_s() { R(); op_s(0, 0); }
OP_percent()     { R(); op0_percent();     menu1chcmd('%'); }
OP_rt_arrow()    { R(); op0_rt_arrow();    menu1chcmd('>'); }
OP_exclamation() { R(); op0_exclamation(); menu1chcmd('!'); }

OP_shf() { op_s('h', 'f'); }
OP_shw() { op_s('h', 'w'); }
OP_h()   { op_h('n', 0); }
OP_he()  { op_h('e', 0); }
OP_hi()  { op_h('i', 0); }

static Menu view_menu;

Menu make_menus(frame)
Frame frame; 
{
	Menu sun_menu;

	sun_menu = (Menu)window_get(frame, WIN_MENU);

	view_menu = menu_create(
	    MENU_PULLRIGHT_ITEM,
	    "slice",
		menu_create(
		MENU_ITEM,
			MENU_STRING, "x                 sx",
			MENU_NOTIFY_PROC, OP_sx,
			0,
		MENU_ITEM,
			MENU_STRING, "y                 sy",
			MENU_NOTIFY_PROC, OP_sy,
			0,
		MENU_ITEM,
			MENU_STRING, "v                 sv",
			MENU_NOTIFY_PROC, OP_sv,
			0,
		MENU_ITEM,
			MENU_STRING, "pr Avg,std,hilo    a",
			MENU_NOTIFY_PROC, OP_A,
			0,
		MENU_ITEM,
			MENU_STRING, "pr boxed pixels    p",
			MENU_NOTIFY_PROC, OP_P,
			0,
		MENU_ITEM,
			MENU_STRING, "select Hi pixel    H",
			MENU_NOTIFY_PROC, OP_H,
			0,
		MENU_ITEM,
			MENU_STRING, "select Lo pixel    L",
			MENU_NOTIFY_PROC, OP_L,
			0,
		MENU_ITEM,
			MENU_STRING, "iterate SelHL off s0",
			MENU_NOTIFY_PROC, OP_s0,
			0,
		MENU_ITEM,
			MENU_STRING, "iterate SelHL on  s1",
			MENU_NOTIFY_PROC, OP_s1,
			0,
		MENU_ITEM,
			MENU_STRING, "help",
			MENU_NOTIFY_PROC, help_slice,
			0,
		0),
	    MENU_PULLRIGHT_ITEM,
	    "file",
		menu_create(
		MENU_ITEM,
			MENU_STRING, "load file           l #",
			MENU_NOTIFY_PROC, OP_l,
			0,
		MENU_ITEM,
			MENU_STRING, "write boxed pixels  w #",
			MENU_NOTIFY_PROC, OP_w,
			0,
		MENU_ITEM,
			MENU_STRING, "set&show write box sw",
			MENU_NOTIFY_PROC, OP_sw,
			0,
		MENU_ITEM,
			MENU_STRING, "quit                q #",
			MENU_NOTIFY_PROC, OP_q,
			0,
		MENU_ITEM,
			MENU_STRING, "help",
			MENU_NOTIFY_PROC, help_file,
			0,
		0),
	    MENU_PULLRIGHT_ITEM,
	    "set show mode",
		menu_create(
		MENU_ITEM,
			MENU_STRING, "average boxed pixels  sa",
			MENU_NOTIFY_PROC, OP_sa,
			0,
		MENU_ITEM,
			MENU_STRING, "magnify boxed pixels  sm # #",
			MENU_NOTIFY_PROC, OP_sm,
			0,
		MENU_ITEM,
			MENU_STRING, "print   boxed pixels  sp",
			MENU_NOTIFY_PROC, OP_sp,
			0,
		MENU_ITEM,
			MENU_STRING, "threshold map pixels  st",
			MENU_NOTIFY_PROC, OP_st,
			0,
		MENU_ITEM,
			MENU_STRING, "help",
			MENU_NOTIFY_PROC, help_show,
			0,
		0),
	    MENU_PULLRIGHT_ITEM,
	    "histogram",
		menu_create(
		MENU_ITEM,
			MENU_STRING, "show Full  window plot     sh|shf",
			MENU_NOTIFY_PROC, OP_shf,
			0,
		MENU_ITEM,
			MENU_STRING, "show boxed Window plot        shw",
			MENU_NOTIFY_PROC, OP_shw,
			0,
		MENU_ITEM,
			MENU_STRING, "raw image hist plot mode       h",
			MENU_NOTIFY_PROC, OP_h,
			0,
		MENU_ITEM,
			MENU_STRING, "raw integrated hist plot mode  hi",
			MENU_NOTIFY_PROC, OP_hi,
			0,
		MENU_ITEM,
			MENU_STRING, "equalized image hist plot mode he",
			MENU_NOTIFY_PROC, OP_he,
			0,
		MENU_ITEM,
			MENU_STRING, "help",
			MENU_NOTIFY_PROC, help_hist,
			0,
		0),
	    MENU_PULLRIGHT_ITEM,
	    "draw image",
		menu_create(
		MENU_ITEM,
			MENU_STRING, "normal/raw     d",
			MENU_NOTIFY_PROC, OP_dn,
			0,
		MENU_ITEM,
			MENU_STRING, "hist equalized de",
			MENU_NOTIFY_PROC, OP_de,
			0,
		MENU_ITEM,
			MENU_STRING, "magnified      dm # #",
			MENU_NOTIFY_PROC, OP_dm,
			0,
		MENU_ITEM,
			MENU_STRING, "threshold map  dt # #",
			MENU_NOTIFY_PROC, OP_dt,
			0,
		MENU_ITEM,
			MENU_STRING, "help",
			MENU_NOTIFY_PROC, help_draw,
			0,
		0),
	    MENU_PULLRIGHT_ITEM,
	    "reset",
		menu_create(
		MENU_ITEM,
			MENU_STRING, "ReDisplay      rd",
			MENU_NOTIFY_PROC, OP_rd,
			0,
		MENU_ITEM,
			MENU_STRING, "ReInit 512X512 ri",
			MENU_NOTIFY_PROC, OP_ri,
			0,
		MENU_ITEM,
			MENU_STRING, "ReSize to fit  rs",
			MENU_NOTIFY_PROC, OP_rs,
			0,
		MENU_ITEM,
			MENU_STRING, "Resize Window  rw # #",
			MENU_NOTIFY_PROC, OP_rw,
			0,
		MENU_ITEM,
			MENU_STRING, "help",
			MENU_NOTIFY_PROC, help_reset,
			0,
		0),
	    MENU_PULLRIGHT_ITEM,
	    "render",
		menu_create(
		MENU_ITEM,
			MENU_STRING, "Interchange F/Bground I",
			MENU_NOTIFY_PROC, op_I,
			0,
		MENU_ITEM,
			MENU_STRING, "invert gray map       i",
			MENU_NOTIFY_PROC, op_i,
			0,
		MENU_ITEM,
			MENU_STRING, "Dither mode OFF       D0",
			MENU_NOTIFY_PROC, OP_D0,
			0,
		MENU_ITEM,
			MENU_STRING, "Dither mode ON        D1",
			MENU_NOTIFY_PROC, OP_D1,
			0,
		MENU_ITEM,
			MENU_STRING, "invert Dither mode ON D2",
			MENU_NOTIFY_PROC, OP_D2,
			0,
		MENU_ITEM,
			MENU_STRING, "help",
			MENU_NOTIFY_PROC, help_render,
			0,
		0),
	    MENU_PULLRIGHT_ITEM,
	    "show&set params",
		menu_create(
		MENU_ITEM,
			MENU_STRING, "wd ht      u # #",
			MENU_NOTIFY_PROC, OP_u,
			0,
		MENU_ITEM,
			MENU_STRING, "ht         v #",
			MENU_NOTIFY_PROC, OP_v,
			0,
		MENU_ITEM,
			MENU_STRING, "xy coord   x # #",
			MENU_NOTIFY_PROC, OP_x,
			0,
		MENU_ITEM,
			MENU_STRING, "y  coord   y #",
			MENU_NOTIFY_PROC, OP_y,
			0,
		MENU_ITEM,
			MENU_STRING, "vec dxy   du # #",
			MENU_NOTIFY_PROC, OP_du,
			0,
		MENU_ITEM,
			MENU_STRING, "vec dy    dv #",
			MENU_NOTIFY_PROC, OP_dv,
			0,
		MENU_ITEM,
			MENU_STRING, "Clip pts   c #'s ",
			MENU_NOTIFY_PROC, OP_c,
			0,
		MENU_ITEM,
			MENU_STRING, "fill Zone  z #'s",
			MENU_NOTIFY_PROC, OP_z,
			0,
		MENU_ITEM,
			MENU_STRING, "status     s",
			MENU_NOTIFY_PROC, OP_s,
			0,
		MENU_ITEM,
			MENU_STRING, "sp format  % #",
			MENU_NOTIFY_PROC, OP_percent,
			0,
		MENU_ITEM,
			MENU_STRING, "pwd&chdir  > #",
			MENU_NOTIFY_PROC, OP_rt_arrow,
			0,
		MENU_ITEM,
			MENU_STRING, "ex syscmd  ! #",
			MENU_NOTIFY_PROC, OP_exclamation,
			0,
		MENU_ITEM,
			MENU_STRING, "help",
			MENU_NOTIFY_PROC, help_param,
			0,
		0),
	    MENU_PULLRIGHT_ITEM,
	    "help",
		menu_create(
		MENU_ITEM,
			MENU_STRING, "mouse & toggle",
			MENU_NOTIFY_PROC, help_mouse,
			0,
		MENU_ITEM,
			MENU_STRING, "misc",
			MENU_NOTIFY_PROC, help_misc,
			0,
		0),
	    MENU_PULLRIGHT_ITEM,
	        "sun menu",
		sun_menu,
		0,
	0);

	window_set(frame, WIN_MENU, view_menu, 0);
	return(view_menu);
}

display_menus(event)
Event *event;
{
	menu_x = event_x(event);  menu_y = event_y(event);
	menu_show(view_menu, canvas, event, 0);
}
