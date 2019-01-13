#include <sunwindow/window_hs.h>
#include <suntool/canvas.h>

binpic(pw, dx, dy, invert, sbuf, swd, sht, lo_th, hi_th, sv_mpr)
struct	pixwin *pw;
unsigned char	sbuf[];
struct pixrect  *sv_mpr;
{
	register int  r;
	register int  i, j;
	register int  t, Llo_th, Lhi_th;
	register unsigned char *sbufP;
	register int inv, not_inv;
	struct pixrect  *mpr = mem_create(swd+32, 1, 1);
        register u_char *mpr_dP;
        register u_char *strtmpr_dP;


	if(invert){
		inv = 1; not_inv = 0;
	}else{
		inv = 0; not_inv = 1;  
	}

	Llo_th = lo_th;  Lhi_th = hi_th;

	strtmpr_dP = mprd8_addr( mpr_d(mpr), 0, 0, mpr->pr_depth);

	for(j=0; j<=sht; j++){
		sbufP = &sbuf[j*swd];
		r = 1;
		mpr_dP = strtmpr_dP;
		setmpr(mpr, 0);

		for(i=0; i<=swd; i++){

			t = *sbufP++;
			if(t>=Llo_th && t<=Lhi_th){
				r = (r<<1) + inv;
			}else{
				r = (r<<1) + not_inv;
			}

			if(r&0x100){
				*mpr_dP++ = r;
				r = 1;
			}
		}

		*mpr_dP = r;

		pw_write(  pw, dx, j+dy, swd, 1, PIX_SRC, mpr, 0, 0);
		pr_rop(sv_mpr, dx, j+dy, swd, 1, PIX_SRC, mpr, 0, 0);
	}
	pr_destroy(mpr);
}
