
#include "comp_hist.h"

comp_hist()
{				/* to compute histogram      */
    int       i;

    int       pv;		/* pixel value	 */

    if (pix_format == PFBYTE) {
	for (i = 0; i < 256; i++)
	    b_hist[i] = 0;	/* initial	 */
	val_min = barray[0];
	val_Max = barray[0];

	n_of_dv = 0;		/* number of different values	 */
	for (i = 0; i < size; i++) {
	    pv = barray[i];	/* pixel value		 */

	    if (b_hist[pv] == 0)
		n_of_dv++;

	    b_hist[pv]++;

	    if (pv < val_min)
		val_min = pv;
	    if (val_Max < pv)
		val_Max = pv;
	}

	free(barray);

	occ_Max = 0;		/* occurrence Maximum		 */
	for (i = val_min; i <= val_Max; i++)
	    if (occ_Max < b_hist[i]) {
		occ_Max = b_hist[i];
		occ_Max_i = i;
	    }
	set_str();		/* set hint strings	 */
	return;
    }				/* end of if byte		 */
    if (pix_format == PFSHORT) {
	s_hist = Calloc(65536, int);
	for (i = 0; i < 65536; i++)
	    s_hist[i] = 0;	/* initial	 */
	val_min = sarray[0];
	val_Max = sarray[0];

	n_of_dv = 0;		/* number of different values	 */
	for (i = 0; i < size; i++) {
	    pv = sarray[i] + 32768;	/* pixel value		 */

	    if (s_hist[pv] == 0)
		n_of_dv++;

	    s_hist[pv]++;

	    if (sarray[i] < val_min)
		val_min = sarray[i];
	    if (val_Max < sarray[i])
		val_Max = sarray[i];
	}

	free(sarray);

	occ_Max = 0;		/* occurrence Maximum		 */
	for (i = val_min + 32768; i <= val_Max + 32768; i++)
	    if (occ_Max < s_hist[i]) {
		occ_Max = s_hist[i];
		occ_Max_i = i - 32768;
	    }
	set_str();		/* set hint strings	 */
	return;
    }				/* end of if short		 */
    if (pix_format == PFINT) {
	int_hist();

#ifdef aaz
	printf("end of comp_hist, after int_hist, val_min=%d  val_Max=%d\n",
	       (int) val_min, (int) val_Max);
	printf("             n_of_dv=%d  occ_Max=%d  occ_Max_i=%d\n\n",
	       n_of_dv, occ_Max, occ_Max_i);
#endif

	return;
    }
    /* float	 */
    float_hist();		/* to compute float histogram f_hist[]		 */

#ifdef aaz
    printf("end of comp_hist, after float_hist, val_min=%.2f  val_Max=%.2f\n",
	   val_min, val_Max);
    printf("             n_of_dv=%d  occ_Max=%d  occ_Max_f=%.2f\n\n",
	   n_of_dv, occ_Max, occ_Max_f);
#endif

}				/* end of  comp_hist ()	 */

int_hist()
{
    int       i;
    int       pv;		/* pixel value     */

    Ihtype   *q, *r;		/* int histogram	 */

    val_min = iarray[0];
    val_Max = iarray[0];

    for (i = 0; i < size; i++) {
	pv = iarray[i];		/* int pixel value       */
	if (val_min > pv)
	    val_min = pv;
	if (val_Max < pv)
	    val_Max = pv;
    }

    cnt_size = val_Max - val_min + 1;	/* pv_cnt[] size   */
    pv_cnt = Calloc(cnt_size, int);
    if (pv_cnt != NULL) {
	i_bucket();
	return;
    }
    i0_hist = NULL;
    for (i = 0; i < size; i++)
	ins_i(iarray[i]);	/* insert int  */
    free(iarray);

    q = i0_hist;
    i_hist = Calloc(n_of_dv, Ihtype_a);

    occ_Max = 0;		/* occurrence Maximum		 */
    for (i = 0; i < n_of_dv; i++) {
	i_hist[i].pv = q->pv;
	i_hist[i].occ = q->occ;
	if (occ_Max < i_hist[i].occ) {
	    occ_Max = i_hist[i].occ;
	    occ_Max_i = i_hist[i].pv;
	}
	r = q->next;
	free(q);
	q = r;
    }

    set_str();			/* set hint strings	 */
}				/* end of int_hist ()		 */

i_bucket()
{				/* int bucket sort	 */
    int       i;
    int       k;
    int       y;
    int       pv_min;

    pv_min = val_min;
    for (i = 0; i < cnt_size; i++)
	pv_cnt[i] = 0;
    n_of_dv = 0;		/* number of different values	 */

    for (i = 0; i < size; i++) {
	k = iarray[i] - pv_min;	/* k corresponds to pixel value.  */
	if (pv_cnt[k] == 0)
	    n_of_dv++;
	pv_cnt[k]++;
    }
    free(iarray);

    i_hist = Calloc(n_of_dv, Ihtype_a);
    y = 0;
    for (k = 0; k < cnt_size; k++) {	/* k corresponds to pixel value.  */
	if (pv_cnt[k] > 0) {
	    i_hist[y].pv = pv_min + k;	/* int pixel value	 */
	    i_hist[y].occ = pv_cnt[k];	/* count	 */
	    y++;
	}
    }
    /* printf("in i_bucket, n_of_dv=%d  y=%d\n", n_of_dv, y ); */
    free(pv_cnt);

    occ_Max = 0;		/* occurrence Maximum           */
    for (i = 0; i < n_of_dv; i++)
	if (occ_Max < i_hist[i].occ) {
	    occ_Max = i_hist[i].occ;
	    occ_Max_i = i_hist[i].pv;
	}
    set_str();			/* set hint strings	 */
}				/* end of   i_bucket ()		 */

float_hist()
{				/* to compute float histogram          */
    int       i;
    float     pv;		/* pixel value	 */

    Fhtype   *s, *t;		/* float histogram	 */

    val_min = floor(farray[0] * 100.);
    val_Max = floor(farray[0] * 100.);

    for (i = 0; i < size; i++) {
	pv = farray[i];		/* float pixel value	 */
	pv = floor(pv * 100.);
	farray[i] = pv;

	if (val_min > pv)
	    val_min = pv;
	if (val_Max < pv)
	    val_Max = pv;
    }

    cnt_size = val_Max - val_min + 1;	/* pv_cnt[] size	 */
    pv_cnt = Calloc(cnt_size, int);
    if (pv_cnt != NULL) {
	f_bucket();
	return;
    }
    val_min = val_min / 100.;
    val_Max = val_Max / 100.;

    f0_hist = NULL;
    for (i = 0; i < size; i++)
	ins_f(i);		/* insert int  */
    free(farray);

    s = f0_hist;
    f_hist = Calloc(n_of_dv, Fhtype_a);

    occ_Max = 0;		/* occurrence Maximum		 */
    for (i = 0; i < n_of_dv; i++) {
	f_hist[i].pv = s->pv / 100.;
	f_hist[i].occ = s->occ;
	if (occ_Max < f_hist[i].occ) {
	    occ_Max = f_hist[i].occ;
	    occ_Max_f = f_hist[i].pv;
	}
	t = s->next;
	free(s);
	s = t;
    }

    set_str();			/* set hint strings	 */
}				/* end of float_hist ()	 */

f_bucket()
{				/* float bucket sort	 */
    int       i;
    int       k;
    int       y;

    for (i = 0; i < cnt_size; i++)
	pv_cnt[i] = 0;

    n_of_dv = 0;		/* number of different values	 */

    for (i = 0; i < size; i++) {
	k = farray[i] - val_min;
	if (pv_cnt[k] == 0)
	    n_of_dv++;
	pv_cnt[k]++;
    }
    free(farray);

    f_hist = Calloc(n_of_dv, Fhtype_a);
    y = 0;
    for (i = 0; i < cnt_size; i++) {
	if (pv_cnt[i] > 0) {
	    f_hist[y].pv = (val_min + i) / 100.;	/* float pixel value  */
	    f_hist[y].occ = pv_cnt[i];	/* count	 */
	    y++;
	}
    }
    /* printf("in f_bucket, n_of_dv=%d  y=%d\n", n_of_dv, y );  */
    free(pv_cnt);
    val_min = val_min / 100.;
    val_Max = val_Max / 100.;

    occ_Max = 0;		/* occurrence Maximum           */
    for (i = 0; i < n_of_dv; i++)
	if (occ_Max < f_hist[i].occ) {
	    occ_Max = f_hist[i].occ;
	    occ_Max_f = f_hist[i].pv;
	}
    set_str();			/* set hint strings	 */
}				/* end of  f_bucket ()		 */

ins_i(pv)			/* insert int pixel value	 */
    int       pv;
{
    Ihtype   *prev, *q, *r;	/* int histogram	 */

    if (i0_hist == NULL) {
	i0_hist = Calloc(1, Ihtype);
	n_of_dv = 1;		/* number of different values		 */

	i0_hist->pv = pv;	/* int pixel value	 */
	i0_hist->occ = 1;	/* occurrence		 */
	i0_hist->next = NULL;
	return;
    }
    q = i0_hist;
    while (q != NULL) {
	if (pv < q->pv) {
	    r = Calloc(1, Ihtype);
	    n_of_dv++;
	    r->pv = pv;		/* int pixel value       */
	    r->occ = 1;		/* occurrence            */
	    r->next = q;
	    if (q == i0_hist)
		i0_hist = r;
	    else
		prev->next = r;
	    return;
	}
	if (pv == q->pv) {
	    q->occ++;
	    return;
	}
	prev = q;

	q = q->next;
    }				/* end of while	 */

    r = Calloc(1, Ihtype);
    n_of_dv++;
    r->pv = pv;			/* int pixel value       */
    r->occ = 1;			/* occurrence            */
    r->next = NULL;
    prev->next = r;
}				/* end of ins_i (pv)		 */

ins_f(idx)			/* insert int pixel value	 */
    int       idx;		/* index of farray[]		 */
{
    float     pv;
    Fhtype   *prev, *q, *r;	/* int histogram	 */

    pv = farray[idx];		/* float pixel value	 */
    /* pv = floor( pv * 100. ) / 100. ;  */

    if (f0_hist == NULL) {
	f0_hist = Calloc(1, Fhtype);
	n_of_dv = 1;		/* number of different values		 */

	f0_hist->pv = pv;	/* float pixel value	 */
	f0_hist->occ = 1;	/* occurrence		 */
	f0_hist->next = NULL;
	goto end_;
    }
    q = f0_hist;
    while (q != NULL) {
	if (pv < q->pv) {
	    r = Calloc(1, Fhtype);
	    n_of_dv++;
	    r->pv = pv;		/* float pixel value       */
	    r->occ = 1;		/* occurrence            */
	    r->next = q;
	    if (q == f0_hist)
		f0_hist = r;
	    else
		prev->next = r;
	    goto end_;
	}
	if (pv == q->pv) {
	    q->occ++;
	    goto end_;
	}
	prev = q;

	q = q->next;
    }				/* end of while	 */

    r = Calloc(1, Fhtype);
    n_of_dv++;
    r->pv = pv;			/* float pixel value       */
    r->occ = 1;			/* occurrence            */
    r->next = NULL;
    prev->next = r;
end_:
#ifdef aay
    if (idx % 1000 == 0)
	printf("end of ins_f, i=%d  pv=%.2f  n_of_dv=%d\n",
	       idx, pv / 100., n_of_dv);
#endif

    return;
}				/* end of ins_f (idx)		 */

set_str()
{				/* set hint string  hint_s		 */
    int       i;

    paint = 0;			/* fisrt picture of this image  */

    sprintf(hint_s[0], "image file : %s", cur_fname);

    switch (pix_format) {
    case PFBYTE:
	sprintf(hint_s[1], "image type : byte");
	break;
    case PFSHORT:
	sprintf(hint_s[1], "image type : short");
	break;
    case PFINT:
	sprintf(hint_s[1], "image type : int");
	break;
    case PFFLOAT:
	sprintf(hint_s[1], "image type : float");
	break;
    }

    sprintf(hint_s[2], "image size : %d x %d ,", nrow, ncol);
    sprintf(hint_s[3], "             %d frame(s)", nfr);

    sprintf(hint_s[4], "total pixels : %d", size);
    sprintf(hint_s[5], "# of different pixel values :");
    sprintf(hint_s[6], "   %d", n_of_dv);
    hint_s[7][0] = 0;

    sprintf(hint_s[8], "Maximum count = %d", occ_Max);

    if (pix_format == PFFLOAT)
	sprintf(hint_s[9], "   at pixel value %.2f", occ_Max_f);
    else
	sprintf(hint_s[9], "   at pixel value %d", occ_Max_i);

    if (pix_format == PFFLOAT) {
	sprintf(hint_s[10], "min pixel value = %.2f", val_min);
	sprintf(hint_s[11], "Max pixel value = %.2f", val_Max);
    } else {
	sprintf(hint_s[10], "min pixel value = %d", (int) val_min);
	sprintf(hint_s[11], "Max pixel value = %d", (int) val_Max);
    }
    hint_s[12][0] = 0;

#ifdef aay
    switch (pix_format) {
    case PFBYTE:
/*     for (i=val_min; i<=val_Max; i++)
       printf("b_hist[%d] = %d\n", i, b_hist[i]);  */
	break;
    case PFSHORT:
/*     for (i=val_min+32768; i<=val_Max+32768; i++)
       printf("s_hist[%d] = %d\n", i, s_hist[i]);  */
	break;
    case PFINT:
	for (i = 0; i < n_of_dv; i++)
	    printf("i_hist[%d].pv=%d  i_hist[%d].occ=%d\n",
		   i, i_hist[i].pv, i, i_hist[i].occ);
	break;
    case PFFLOAT:
	for (i = 0; i < n_of_dv; i++)
	    printf("f_hist[%d].pv=%.2f  f_hist[%d].occ=%d\n",
		   i, f_hist[i].pv, i, f_hist[i].occ);
	break;
    }
#endif

}				/* end of set_str ()		 */

comp_hg()
{				/* to compute histogram graph	 */
    int       i;

#ifdef aax
    printf("enter comp_hg, paint=%d  NewLeft=%d  NewRight=%d  NewTop=%d  NewBottom=%d",
	   paint, NewLeft, NewRight, NewTop, NewBottom);
#endif

    switch (pix_format) {
    case PFBYTE:
	comp_byte_hg();		/* compute byte histogram graph		 */
	break;
    case PFSHORT:
	comp_short_hg();	/* compute byte histogram graph		 */
	break;
    case PFINT:
	comp_int_hg();		/* compute int histogram graph		 */
	break;
    case PFFLOAT:
	comp_float_hg();	/* compute byte histogram graph		 */
	break;
    }

    cur_size = 0;		/* current size        */
    for (i = 0; i <= range; i++)
	cur_size = cur_size + histo_graph[i];

    if (range < 200)
	expand();

    if (pix_format != PFFLOAT) {/* byte, short, int	 */
#ifdef aax
	printf("in comp_hg, histo_va[0]=%d  histo_va[%d]=%d\n",
	       histo_va[0], range, histo_va[range]);
#endif

	build_xlab(histo_va[0], histo_va[range]);

#ifdef aax
	for (i = 0; i < end_x; i++)
	    printf("    x_label[%d]=%d\n", i, x_label[i]);
#endif

	pv_diff = histo_va[range] - histo_va[0];
    } else {
	build_fxlab(fhisto_va[0], fhisto_va[range]);

#ifdef aax
	for (i = 0; i < end_x; i++)
	    printf("    fx_label[%d]=%.2f\n", i, fx_label[i]);
#endif

	pv_diff = fhisto_va[range] - fhisto_va[0];
    }

#ifdef aax
    printf("end of comp_hg,  Top=%d  Bottom=%d\n", Top, Bottom);
#endif

    occ_diff = Top - Bottom;

    if (occ_diff > 30)
	build_label(Bottom, Top);
    else if (occ_diff > 1) {
	y_label[0] = Bottom;
	y_label[1] = (Bottom + Top) / 2;
	y_label[2] = Top;
	end_y = 3;
    } else {
	y_label[0] = Bottom;
	y_label[1] = Top;
	end_y = 2;
    }

#ifdef aax
    for (i = 0; i < end_y; i++)
	printf("    y_label[%d]=%d\n", i, y_label[i]);
#endif

    set_xy();

}				/* end of comp_hg ()		 */

expand()
{				/* as range < 200 after comp_..._hg		 */
    int       i, k;
    int       hgr[220];
    int       hv[220];
    float     fhv[220];

    for (k = 0; k <= range; k++) {
	hgr[k] = histo_graph[k];
	if (pix_format != PFFLOAT)
	    hv[k] = histo_va[k];
	else
	    fhv[k] = fhisto_va[k];
    }

    histo_graph[201] = histo_graph[range + 1];

    if (pix_format != PFFLOAT)
	histo_va[201] = histo_va[range + 1];
    else
	fhisto_va[201] = fhisto_va[range + 1];

    k = 0;			/* index of hgr[], hv[], fhv[]		 */

    for (i = 0; i <= 200; i++) {
	if (k <= range && k * 200 / (range + 1) == i) {
	    histo_graph[i] = hgr[k];
	    if (pix_format != PFFLOAT)
		histo_va[i] = hv[k];
	    else
		fhisto_va[i] = fhv[k];

	    k++;
	} else {
	    histo_graph[i] = histo_graph[i - 1];
	    if (pix_format != PFFLOAT)
		histo_va[i] = histo_va[i - 1];
	    else
		fhisto_va[i] = fhisto_va[i - 1];
	}
    }

    range = 200;
}				/* end of  expand ()		 */

set_xy()
{				/* set x_pos[] and y_pos[]	 */
    int       i;

    for (i = 0; i < end_x; i++) {
	if (pix_format != PFFLOAT)
	    x_pos[i] = 2 * TXTWD + range * (x_label[i] - histo_va[0]) / pv_diff;
	else
	    x_pos[i] = 2 * TXTWD + range * (fx_label[i] - fhisto_va[0]) / pv_diff;
    }

    for (i = 0; i < end_y; i++)
	y_pos[i] = 3 * TXTHT + HGT - HGT * (y_label[i] - Bottom) / occ_diff;

}				/* end of set_xy ()		 */

comp_byte_hg()
{				/* compute byte histogram graph          */
    int       i, k;
    int       Left, Right;	/* index of b_hist[]       */

    if (paint == 0) {		/* fisrt picture of this image  */
	Left = val_min;		/* index of b_hist[]	 */
	Right = val_Max;	/* index of b_hist[]       */
	Top = occ_Max;
	Bottom = 0;
	paint = 1;
    } else {
	if (NewLeft != -1)
	    Left = histo_va[NewLeft];
	else
	    Left = histo_va[0];	/* index of b_hist[]       */

	if (NewRight != -1)
	    Right = histo_va[NewRight];
	else
	    Right = histo_va[range];	/* index of b_hist[]       */

	if (NewTop != -1)
	    Top = NewTop;
	if (NewBottom != -1)
	    Bottom = NewBottom;
    }

#ifdef aax
    printf("in comp_byte_hg, Left=%d  Right=%d  Top=%d  Bottom=%d\n",
	   Left, Right, Top, Bottom);
#endif

    range = Right - Left;	/* range of indices of histo_graph to show   */

    cur_nofdv = 0;		/* current # of different pixel values   */
    for (i = 0; i <= range; i++) {
	k = Left + i;

	if (b_hist[k] < Bottom)
	    histo_graph[i] = 0;
	else {
	    if (b_hist[k] > 0)
		cur_nofdv++;

	    if (Top < b_hist[k])
		histo_graph[i] = Top;
	    else
		histo_graph[i] = b_hist[k];
	}

	histo_va[i] = k;
    }

    histo_va[range + 1] = Right + 1;

}				/* end of comp_byte_hg () 	 */


sif_occ()
{				/* short, int, float occurrence change	 */
    int       i;

    if (NewTop == -1)
	NewTop = Top;

    if (NewBottom == -1)
	NewBottom = Bottom;

    Top = NewTop;
    Bottom = NewBottom;

    for (i = 0; i <= range; i++) {
	if (histo_graph[i] < Bottom)
	    histo_graph[i] = 0;
	else if (Top < histo_graph[i])
	    histo_graph[i] = Top;
    }

}

comp_short_hg()
{				/* compute byte histogram graph          */
    int       i, k;
    int       Left, Right;	/* index of s_hist[]       */
    float     m;

    if (paint == 0) {		/* fisrt picture of this image  */
	Left = val_min + 32768;	/* index of s_hist[]	 */
	Right = val_Max + 32768;/* index of s_hist[]       */
	paint = 1;
    } else {
	if (NewTop != -1 || NewBottom != -1) {
	    sif_occ();
	    return;
	}
	if (NewLeft != -1)
	    Left = histo_va[NewLeft] + 32768;
	else
	    Left = histo_va[0] + 32768;	/* index of s_hist[]       */

	if (NewRight != -1)
	    Right = histo_va[NewRight + 1] - 1 + 32768;
	else
	    Right = histo_va[range + 1] - 1 + 32768;	/* index of s_hist[]   */
    }

    range = Right - Left;	/* range of indices of histo_graph to show   */

    if (range > 800) {
	m = (range + 1) / 801.;
	range = 800;
    } else
	m = 1;

    histo_va[range + 1] = Right + 1 - 32768;
    for (i = 0; i <= range; i++)
	histo_va[i] = Left + i * m - 32768;

    for (i = 0; i <= range; i++)
	histo_graph[i] = 0;
    i = 0;
    cur_nofdv = 0;		/* current # of different pixel values   */
    for (k = Left; k <= Right; k++) {
	if (s_hist[k] > 0)
	    cur_nofdv++;
	if (k - 32768 >= histo_va[i + 1])
	    i++;
	histo_graph[i] = histo_graph[i] + s_hist[k];
    }

#ifdef aax
    printf("in comp_short_hg, range=%d  i=%d\n", range, i);
#endif

    Top = histo_graph[0];
    Bottom = histo_graph[0];
    for (i = 1; i <= range; i++) {
	if (Top < histo_graph[i])
	    Top = histo_graph[i];
	if (Bottom > histo_graph[i])
	    Bottom = histo_graph[i];
    }

#ifdef aax
    printf("end of comp_short_hg, Left=%d  Right=%d  Top=%d  Bottom=%d\n",
	   Left, Right, Top, Bottom);
#endif

}				/* end of  comp_short_hg ()		 */


comp_float_hg()
{				/* compute byte histogram graph          */
    int       i, k;
    int       Left, Right;	/* index of f_hist[]       */
    float     m;

    if (paint == 0) {		/* fisrt picture of this image  */
	Left = 0;		/* index of f_hist[]	 */
	Right = n_of_dv - 1;	/* index of f_hist[]       */
	paint = 1;
    } else {
	if (NewTop != -1 || NewBottom != -1) {
	    sif_occ();
	    return;
	}
	if (NewLeft != -1)
	    Left = fv_idx(Base, NewLeft);	/* index of f_hist[] */
	else
	    Left = Base;	/* index of f_hist[]       */

/*
   printf("in comp_float_hg, Left=%d  f_hist[%d].pv=%.2f  f_hist[%d].occ=%d\n",
	  Left, Left, f_hist[Left].pv, Left, f_hist[Left].occ );
*/

	if (NewRight != -1)
	    Right = fv_idx(Left, NewRight + 1);
	else
	    Right = fv_idx(Left, range + 1);	/* index of f_hist[]   */
    }

    cur_nofdv = Right - Left + 1;	/* current # of different pv	 */

    Base = Left;		/* base index of f_hist[]   */

    range = (f_hist[Right].pv - f_hist[Left].pv) * 100;

    if (range > 800)
	range = 800;

    m = (f_hist[Right].pv - f_hist[Left].pv) / range;

    fhisto_va[range + 1] = f_hist[Right].pv + .01;
    for (i = 0; i <= range; i++)
	fhisto_va[i] = f_hist[Left].pv + i * m;

    for (i = 0; i <= range; i++)
	histo_graph[i] = 0;
    i = 0;
    for (k = Left; k <= Right; k++) {
	while (f_hist[k].pv >= fhisto_va[i + 1])
	    i++;
	histo_graph[i] = histo_graph[i] + f_hist[k].occ;
    }

#ifdef aax
    printf("in comp_float_hg, Base=%d  range=%d  i=%d\n", Base, range, i);
#endif

    Top = histo_graph[0];
    Bottom = histo_graph[0];
    for (i = 1; i <= range; i++) {
	if (Top < histo_graph[i])
	    Top = histo_graph[i];
	if (Bottom > histo_graph[i])
	    Bottom = histo_graph[i];
    }

#ifdef aax
    printf("in comp_float_hg, Left=%d  Right=%d  Top=%d  Bottom=%d\n",
	   Left, Right, Top, Bottom);
#endif

}				/* end of  comp_float_hg ()		 */

int 
fv_idx(Begin, Target)		/* float value --> index of f_list[].pv */
    int       Begin;		/* the beginning index	 */
    int       Target;		/* target index of fhisto_va[]	 */
{
    float     pv;
    int       k;		/* index of f_list[].pv */

    pv = fhisto_va[Target];

    k = Begin;
    do {
	if (f_hist[k].pv >= pv)
	    return k;
	k++;
    } while (k < n_of_dv);

    return n_of_dv - 1;
}


comp_int_hg()
{				/* compute byte histogram graph          */
    int       i, k;
    int       Left, Right;	/* index of i_hist[]       */
    float     m;

    if (paint == 0) {		/* fisrt picture of this image  */
	Left = 0;		/* index of i_hist[]	 */
	Right = n_of_dv - 1;	/* index of i_hist[]       */
	paint = 1;
    } else {
	if (NewTop != -1 || NewBottom != -1) {
	    sif_occ();
	    return;
	}
	if (NewLeft != -1)
	    Left = iv_idx(Base, NewLeft);	/* index of i_hist[] */
	else
	    Left = Base;	/* index of i_hist[]       */

	if (NewRight != -1)
	    Right = iv_idx(Left, NewRight + 1);
	else
	    Right = iv_idx(Left, range + 1);	/* index of i_hist[]   */
    }

    cur_nofdv = Right - Left + 1;	/* current # of different pv	 */

    Base = Left;		/* base index of i_hist[]   */

    range = i_hist[Right].pv - i_hist[Left].pv;

    if (range > 800)
	range = 800;

    m = (float) (i_hist[Right].pv - i_hist[Left].pv + 1) / (range + 1);

    histo_va[range + 1] = i_hist[Right].pv + 1;
    for (i = 0; i <= range; i++)
	histo_va[i] = i_hist[Left].pv + i * m;

    for (i = 0; i <= range; i++)
	histo_graph[i] = 0;
    i = 0;
    for (k = Left; k <= Right; k++) {
	while (i_hist[k].pv >= histo_va[i + 1])
	    i++;
	histo_graph[i] = histo_graph[i] + i_hist[k].occ;
    }

#ifdef aax
    printf("in comp_int_hg, Base=%d  range=%d  i=%d\n", Base, range, i);
#endif

    Top = histo_graph[0];
    Bottom = histo_graph[0];
    for (i = 1; i <= range; i++) {
	if (Top < histo_graph[i])
	    Top = histo_graph[i];
	if (Bottom > histo_graph[i])
	    Bottom = histo_graph[i];
    }

#ifdef aax
    printf("end of comp_int_hg, Left=%d  Right=%d  Top=%d  Bottom=%d\n",
	   Left, Right, Top, Bottom);
#endif

}				/* end of  comp_int_hg ()		 */

int 
iv_idx(Begin, Target)		/* int value --> index of f_list[].pv */
    int       Begin;		/* the beginning index	 */
    int       Target;		/* target index of histo_va[]	 */
{
    int       pv;
    int       k;		/* index of f_list[].pv */

    pv = histo_va[Target];

    k = Begin;
    do {
	if (i_hist[k].pv >= pv)
	    return k;
	k++;
    } while (k < n_of_dv);

    return n_of_dv - 1;
}
