
#include <stdio.h>

#include <math.h>

extern int y_label[10];

extern int end_y;		/* end of y_label[]        */

extern int x_label[20];

extern int end_x;		/* end of x_label[]        */

extern float fx_label[20];

extern float fy_label[20];

int       nece[20];		/* necessary label		 */

int       end_nece;		/* end of nece[]		 */

float     fnece[20];		/* necessary label		 */


build_xlab(Low, High)		/* build int x label	 */
    int       Low, High;
{
    int       i;

    if (Low >= 0 && High >= 0) {
	if (High - Low > 10) {
	    build_label(Low, High);
	    for (i = 0; i < end_y; i++)
		x_label[i] = y_label[i];
	    end_x = end_y;
	} else if (High - Low > 4) {
	    x_label[0] = Low;
	    x_label[1] = (High + Low) / 4;
	    x_label[2] = (High + Low) / 2;
	    x_label[3] = (High + Low) * 3 / 4;
	    x_label[4] = High;
	    end_x = 5;
	} else if (High - Low > 1) {
	    x_label[0] = Low;
	    x_label[1] = (High + Low) / 2;
	    x_label[2] = High;
	    end_x = 3;
	} else {
	    x_label[0] = Low;
	    x_label[1] = High;
	    end_x = 2;
	}

	return;
    }
    if (Low < 0 && High >= 0) {
	if (Low < -10) {
	    build_label(0, -Low);
	    for (i = 0; i < end_y - 1; i++)
		x_label[i] = -y_label[end_y - 1 - i];
	    x_label[end_y - 1] = 0;
	    end_x = end_y;
	} else {
	    x_label[0] = Low;
	    x_label[1] = 0;
	    end_x = 2;
	}

	if (10 < High) {
	    build_label(0, High);
	    for (i = 1; i < end_y; i++)
		x_label[end_x - 1 + i] = y_label[i];
	    end_x = end_x - 1 + end_y;
	} else {
	    x_label[end_x] = High;
	    end_x++;
	}

	return;
    }
    /* Low<0 && High<0   */
    if (High - Low > 10) {
	build_label(-High, -Low);
	for (i = 0; i < end_y; i++)
	    x_label[i] = -y_label[end_y - 1 - i];
	end_x = end_y;
    } else if (High - Low > 1) {
	x_label[0] = Low;
	x_label[1] = (High + Low) / 2;
	x_label[2] = High;
	end_x = 3;
    } else {
	x_label[0] = Low;
	x_label[1] = High;
	end_x = 2;
    }

}				/* end of build_xlab (Low, High)    */


build_label(Low, High)
    int       Low, High;	/* positive integers and High - Low > 10	 */
{
    int       p;
    int       q;
    int       InLab;
    int       interior[20];
    int       k;
    int       j;
    int       unit;		/* unit=1, 2, 3, 4		 */

#ifdef aax
    printf("enter build_label, Low=%d  High=%d\n", Low, High);
#endif

    y_label[0] = Low;

    if (High - Low <= 30) {
	end_y = 1;
	for (j = Low + 1; j < High; j++)
	    if (j % 10 == 0) {
		y_label[end_y] = j;
		end_y++;
	    }
	y_label[end_y] = High;
	end_y++;
	return;
    }
    p = log10((double) High);
    q = pow(10.,(double) p);

    end_nece = 0;		/* end of nece[]                */

next_:

    InLab = ((Low + q) / q) * q;

    k = 0;

    while (Low < InLab && InLab < High) {
	interior[k] = InLab;
	k++;
	InLab = InLab + q;
    }

    switch (k) {
    case 0:
	q = q / 10;
	goto next_;
    case 1:
	nece[end_nece] = interior[0];
	end_nece++;
	if (q == (q / 40) * 40)
	    q = q / 4;
	else
	    q = q / 10;
	goto next_;
    case 2:
	nece[end_nece] = interior[0];
	end_nece++;
	nece[end_nece] = interior[1];
	end_nece++;
	q = q / 2;
	goto next_;
    case 3:
    case 4:
    case 5:
	unit = 1;
	break;
    case 6:
    case 7:
    case 8:
    case 9:
	unit = 2;
	break;
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
	unit = 3;
	break;
    default:
	unit = 4;
	break;
    }

    end_y = 1;

    for (j = 0; j < end_nece; j++) {
	if (end_y > 6)
	    break;
	else
	    add_lab(nece[j]);
    }

    for (j = k / 2; j < k; j = j + unit) {
	if (end_y > 6)
	    break;
	else
	    add_lab(interior[j]);
    }

    for (j = k / 2 - unit; j >= 0; j = j - unit) {
	if (end_y > 6)
	    break;
	else
	    add_lab(interior[j]);
    }

    y_label[end_y] = High;

    end_y++;			/* outside		 */

}				/* end of build_label (Low, High)	 */

add_lab(val)			/* add label		 */
    int       val;
{
    int       i;

    for (i = 1; i < end_y; i++)
	if (val == y_label[i])
	    return;

    y_label[end_y] = val;

    end_y++;			/* outside          */
}

/*  The following are for float.		*/

build_fxlab(Low, High)		/* build int x label	 */
    float     Low, High;
{
    int       i;

    if (Low >= 0. && High >= 0.) {
	if (High - Low > 10.) {
	    bd_fy(Low, High);
	    for (i = 0; i < end_y; i++)
		fx_label[i] = fy_label[i];
	    end_x = end_y;
	} else {
	    fx_label[0] = Low;
	    fx_label[1] = (High + Low) / 4;
	    fx_label[2] = (High + Low) / 2;
	    fx_label[3] = (High + Low) * 3 / 4;
	    fx_label[4] = High;
	    end_x = 5;
	}

	return;
    }
    if (Low < 0. && High >= 0.) {
	if (Low < -10.) {
	    bd_fy(0., -Low);
	    for (i = 0; i < end_y - 1; i++)
		fx_label[i] = -fy_label[end_y - 1 - i];
	    fx_label[end_y - 1] = 0.;
	    end_x = end_y;
	} else {
	    fx_label[0] = Low;
	    fx_label[1] = 0.;
	    end_x = 2;
	}

	if (10. < High) {
	    bd_fy(0., High);
	    for (i = 1; i < end_y; i++)
		fx_label[end_x - 1 + i] = fy_label[i];
	    end_x = end_x - 1 + end_y;
	} else {
	    fx_label[end_x] = High;
	    end_x++;
	}

	return;
    }
    /* Low<0 && High<0   */
    if (High - Low > 10.) {
	bd_fy(-High, -Low);
	for (i = 0; i < end_y; i++)
	    fx_label[i] = -fy_label[end_y - 1 - i];
	end_x = end_y;
    } else if (High - Low > 1.) {
	fx_label[0] = Low;
	fx_label[1] = (High + Low) / 2;
	fx_label[2] = High;
	end_x = 3;
    } else {
	fx_label[0] = Low;
	fx_label[1] = High;
	end_x = 2;
    }

}				/* end of build_fxlab (Low, High)    */


bd_fy(Low, High)
    float     Low, High;	/* positive integers and High - Low > 10	 */
{
    int       p;
    float     q;
    float     InLab;
    float     interior[20];
    int       k;
    int       j;
    int       unit;		/* unit=1, 2, 3, 4		 */

#ifdef aax
    printf("enter bd_fy, Low=%.2f  High=%.2f\n", Low, High);
#endif

    fy_label[0] = Low;

    if (High - Low <= 30.) {
	end_y = 1;
	for (j = Low + 1; j < High; j++)
	    if (j % 10 == 0) {
		fy_label[end_y] = j;
		end_y++;
	    }
	fy_label[end_y] = High;
	end_y++;
	return;
    }
    p = log10((double) High);
    q = pow(10.,(double) p);

    end_nece = 0;		/* end of fnece[]                */

next_:

    InLab = floor((Low + q) / q) * q;

    k = 0;

    while (Low < InLab && InLab < High) {
	interior[k] = InLab;
	k++;
	InLab = InLab + q;
    }

    switch (k) {
    case 0:
	q = q / 10;
	goto next_;
    case 1:
	fnece[end_nece] = interior[0];
	end_nece++;
	if (q == floor(q / 40.) * 40.)
	    q = q / 4;
	else
	    q = q / 10;
	goto next_;
    case 2:
	fnece[end_nece] = interior[0];
	end_nece++;
	fnece[end_nece] = interior[1];
	end_nece++;
	q = q / 2;
	goto next_;
    case 3:
    case 4:
    case 5:
	unit = 1;
	break;
    case 6:
    case 7:
    case 8:
    case 9:
	unit = 2;
	break;
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
	unit = 3;
	break;
    default:
	unit = 4;
	break;
    }

    end_y = 1;

    for (j = 0; j < end_nece; j++) {
	if (end_y > 6)
	    break;
	else
	    add_flab(fnece[j]);
    }

    for (j = k / 2; j < k; j = j + unit) {
	if (end_y > 6)
	    break;
	else
	    add_flab(interior[j]);
    }

    for (j = k / 2 - unit; j >= 0; j = j - unit) {
	if (end_y > 6)
	    break;
	else
	    add_flab(interior[j]);
    }

    fy_label[end_y] = High;

    end_y++;			/* outside		 */

}				/* end of bd_fy (Low, High)	 */

add_flab(val)			/* add label		 */
    float     val;
{
    int       i;

    for (i = 1; i < end_y; i++)
	if (val == fy_label[i])
	    return;

    fy_label[end_y] = val;

    end_y++;			/* outside          */
}
