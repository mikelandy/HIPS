#include <sunwindow/window_hs.h>
#define XOR PIX_SRC^PIX_DST

static int restore_vec_flag = 0;
static int ox1, ox2, oy1, oy2;

/* Restore, Draw VECTOR in pixwin *pw */

vec_cursor(pw, x1, y1, x2, y2, color, mx, my)
struct pixwin *pw;
{
		/* restore data */
	if(restore_vec_flag){
		pw_vector(pw, ox1, oy1, ox2, oy2, XOR, color);
		restore_vec_flag = 0;
	}
	if(x1==0 && y1==0 && x2==0 && y2==0)
		return;

		/* draw new vector */
	pw_vector(pw, x1, y1, x2, y2, XOR, color);

	ox1 = x1; ox2 = x2;
	oy1 = y1; oy2 = y2;

	restore_vec_flag = 1;
}

reset_vec_cursor(pw, color)
struct pixwin *pw;
{
	vec_cursor(pw, 0, 0, 0, 0, color, 0, 0);
	restore_vec_flag = 0;
	ox1 = ox2 = oy1 = oy2 = 0;
}
