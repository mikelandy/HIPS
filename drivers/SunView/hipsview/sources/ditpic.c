#include <sunwindow/window_hs.h>
#include <suntool/canvas.h>

ditpic(pw, dx, dy, invert, sbuf, swd, sht, smax, smin, sv_mpr)
struct	pixwin *pw;
unsigned char	sbuf[];
struct pixrect  *sv_mpr;
{
	register int  r;
	register int  sum, value, intensity;
	register int  i, j;
	register int  threshold;
	register unsigned char *sbufP;
	register int *pP, *tP;
	int *prevP, *thisP;
	int inv, not_inv;
	struct pixrect  *mpr = mem_create(swd+32, 1, 1);
        register u_char *mpr_dP;
        register u_char *strtmpr_dP;


	if(invert){
		inv = 1; not_inv = 0;
	}else{
		inv = 0; not_inv = 1;  
	}

	if(smax==0){
		smax = 256;  smin = 0;
	}
		
	threshold = (smax + smin) / 2;

	prevP = (int *)calloc(520, sizeof(int) );
	thisP = (int *)calloc(520, sizeof(int) );
	if(prevP==0 || thisP==0){
		perror("ditpic");
		return;
	}

	strtmpr_dP = mprd8_addr( mpr_d(mpr), 0, 0, mpr->pr_depth);

	for(j=0; j<=sht; j++){
		sbufP = &sbuf[j*swd];
		r = 1;
		mpr_dP = strtmpr_dP;
		setmpr(mpr, 0);

		pP = prevP;
		tP = thisP;
		for(i=0; i<=swd; i++){
	/********************************************************/
	/*	sum  = *pP + (*tP + *(pP+1)) * 3 + *(pP+2);	*/
	/********************************************************/
			sum  = *pP + *(pP+2);
			pP++;
			value= *tP + *pP;
			tP++;
			sum += value;
			sum += value<<1;

			value = *sbufP++;
			intensity = (sum>>3) + value;
			if(intensity >= threshold){
				*tP = intensity - smax;
				r = (r<<1) + inv;
			}else{
				*tP = intensity - smin;
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
		pP = prevP; prevP = thisP; thisP = pP;
	}
	cfree(prevP);
	cfree(thisP);
	pr_destroy(mpr);
}

setmpr(mpr, value)
struct pixrect *mpr;
register int value;
{
        register u_char *strP = mprd8_addr( mpr_d(mpr), 0, 0, mpr->pr_depth);
	register u_char *endP = mprd8_addr( mpr_d(mpr), mpr->pr_width-1,
					    mpr->pr_height-1, mpr->pr_depth);

	if(value==0){
		while (strP <= endP)
			*strP++ = 0;
	}else{
		while (strP <= endP)
			*strP++ = value;
	}
}
