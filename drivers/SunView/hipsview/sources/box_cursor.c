#include <sunwindow/window_hs.h>
#include "hipsview.h"

static int restore_flag = 0;
static int oxlo, oxhi, oylo, oyhi, owd, oht;
static struct pixrect *mpr1P, *mpr2P, *mpr3P, *mpr4P;

/* Restore, Draw BOX in pixwin *pw */

box_cursor(pw, x, y, wd, ht, mx, my)
struct pixwin *pw;
int	x, y, wd, ht, mx, my;
{
	register int xlo, xhi, ylo, yhi;
	register int m_xlo, m_ylo;

		/* restore data */
	if(restore_flag){
		pw_write(pw, oxlo-1, oylo-1, owd,  2, PIX_SRC, mpr1P, 0, 0);
		pw_write(pw, oxhi  , oylo-1, 2,  oht, PIX_SRC, mpr2P, 0, 0);
		pw_write(pw, oxlo+1, oyhi  , owd,  2, PIX_SRC, mpr3P, 0, 0);
		pw_write(pw, oxlo-1, oylo+1, 2,  oht, PIX_SRC, mpr4P, 0, 0);
		restore_flag = 0;
	}
	if(wd==0 && ht==0)
		return;

	xlo = x - 1;	ylo = y - 1;
	xhi = x + wd;	yhi = y + ht;
	wd += 2;	ht += 2;

		/* save data */
	pw_read(mpr1P, 0, 0, wd,  2, PIX_SRC, pw, xlo-1, ylo-1);
	pw_read(mpr2P, 0, 0,  2, ht, PIX_SRC, pw, xhi  , ylo-1);
	pw_read(mpr3P, 0, 0, wd,  2, PIX_SRC, pw, xlo+1, yhi  );
	pw_read(mpr4P, 0, 0,  2, ht, PIX_SRC, pw, xlo-1, ylo+1);

		/* draw new box */
	pw_vector(pw, xlo, ylo, xhi, ylo, PIX_SET, 1);
	pw_vector(pw, xhi, ylo, xhi, yhi, PIX_SET, 1);
	pw_vector(pw, xhi, yhi, xlo, yhi, PIX_SET, 1);
	pw_vector(pw, xlo, yhi, xlo, ylo, PIX_SET, 1);

	switch(showsw){
	case 'x':		/* mark mx position */
		m_xlo = xlo+mx;
		pw_vector(pw, m_xlo, ylo, m_xlo+2, ylo, PIX_CLR, 1);
		pw_vector(pw, m_xlo, yhi, m_xlo+2, yhi, PIX_CLR, 1);
		break;
	case 'y':		/* mark my position */
		m_ylo = ylo+my;
		pw_vector(pw, xlo, m_ylo, xlo, m_ylo+2, PIX_CLR, 1);
		pw_vector(pw, xhi, m_ylo, xhi, m_ylo+2, PIX_CLR, 1);
		break;
	}	

	oxlo = xlo;
	oxhi = xhi;
	oylo = ylo;
	oyhi = yhi;
	owd  = wd;
	oht  = ht;

	xlo--; ylo--;
	xhi++; yhi++;
	pw_vector(pw, xlo, ylo, xhi, ylo, PIX_CLR, 1);
	pw_vector(pw, xhi, ylo, xhi, yhi, PIX_CLR, 1);
	pw_vector(pw, xhi, yhi, xlo, yhi, PIX_CLR, 1);
	pw_vector(pw, xlo, yhi, xlo, ylo, PIX_CLR, 1);
	restore_flag = 1;

	switch(showsw){
	case 'x':		/* mark mx position */
		m_xlo = xlo+mx;
		pw_vector(pw, m_xlo, ylo, m_xlo+2, ylo, PIX_SET, 1);
		pw_vector(pw, m_xlo, yhi, m_xlo+2, yhi, PIX_SET, 1);
		break;
	case 'y':		/* mark my position */
		m_ylo = ylo+my;
		pw_vector(pw, xlo, m_ylo, xlo, m_ylo+2, PIX_SET, 1);
		pw_vector(pw, xhi, m_ylo, xhi, m_ylo+2, PIX_SET, 1);
		break;
	}	

}

reset_box_cursor(pw)
struct pixwin *pw;
{
	box_cursor(pw, 0, 0, 0, 0, 0, 0);
	restore_flag = 0;
}

create_box_cursor_mprs(depth)
{
	destory_box_cursor_mprs();
	mpr1P = mem_create(NU,  2, depth);
	mpr2P = mem_create( 2, NV, depth);
	mpr3P = mem_create(NU,  2, depth);
	mpr4P = mem_create( 2, NV, depth);

}

destory_box_cursor_mprs()
{
	if(mpr1P!=0) pr_destroy(mpr1P);
	if(mpr2P!=0) pr_destroy(mpr2P);
	if(mpr3P!=0) pr_destroy(mpr3P);
	if(mpr4P!=0) pr_destroy(mpr4P);
	mpr1P = mpr2P = mpr3P = mpr4P = 0;
}
