/* vector.c 
 * Max Rible
 * Line functions for suntools stuff.
 */

#include "hipstool.h"

static unsigned get_and_set(), 
    get_char_value(), get_short_value(), get_long_value(),
    set_char_value(), set_short_value(), set_long_value();

static void drawline(), drawtrace(), writeline();

static void fit();

void (*draw[3])() = { drawline, drawtrace, writeline };

unsigned (*readarr[9])() = { 
    get_and_set, 
    get_char_value, get_short_value, NULL, get_long_value, /* data size */
    set_char_value, set_short_value, NULL, set_long_value, /* data size + 4 */
};

/* doline2d
 * Implementation of Bresenham's algorithm.  If
 * |slope| <= 1, draws from left to right, if |slope| > 1,
 * draws from top to bottom.  Returns a vector of grayvalues and
 * x and y coordinates.
 * mode:
 * 0:  give vector the values from pw_get() and make the point stand out
 * 1:  give vector the values from the character array
 * 2:  give vector the values from the short array
 * 4:  give vector the values from the long array
 */

void
doline2d(x_1, y_1, x_2, y_2, vals, mode, set)
     int x_1, y_1, x_2, y_2;
     Trace *vals;
     unsigned mode;
     enum truth set;
{
    int dx, dy, sx, sy, ex, ey, di, dj, t, i1, i2;
    register int i, j;
    unsigned (*getval)();
	
    dx = abs(x_1 - x_2);
    dy = abs(y_1 - y_2);

    getval = readarr[mode];

    switch(!!dx | (!!dy << 1)) {
    case 0:			/* Point */
	vals->info[0] = Calloc(3, unsigned);
	vals->info[1] = vals->info[0]+1; 
	vals->info[2] = vals->info[0]+2;
	vals->info[0][0] = (*getval)(x_1, y_1, set);
	vals->info[1][0] = x_1; vals->info[2][0] = y_1;
	vals->length = 1;
	break;
    case 1:			/* Horizontal */
	vals->info[0] = Calloc(3*(dx+1), unsigned);
	vals->info[1] = vals->info[0]+dx+1;
	vals->info[2] = vals->info[0]+2*(dx+1);
	sx = MIN(x_1, x_2); ex = MAX(x_1,x_2);
	for(i = sx; i <= ex; i++) {
	    vals->info[0][i - sx] = (*getval)(i, y_1, set);
	    vals->info[1][i - sx] = i;
	    vals->info[2][i - sx] = y_1;
	}
	vals->length = dx+1;
	break;
    case 2:			/* Vertical */
	vals->info[0] = Calloc(3*(dy+1), unsigned);
	vals->info[1] = vals->info[0]+dy+1;
	vals->info[2] = vals->info[0]+2*(dy+1);
	sy = MIN(y_1, y_2); ey = MAX(y_1,y_2);
	for(i = sy; i <= ey; i++) {
	    vals->info[0][i - sy] = (*getval)(x_1, i, set);
	    vals->info[1][i - sy] = x_1;
	    vals->info[2][i - sy] = i;
	}
	vals->length = dy+1;
	break;
    case 3:			/* Diagonal */
	vals->info[0] = Calloc(3*(i = (MAX(dx,dy)+1)), unsigned);
	vals->info[1] = vals->info[0]+i;
	vals->info[2] = vals->info[0]+2*i;
	vals->length = i;
	if(dy > dx) {		/* Draw from top to bottom */
	    if(y_1 < y_2) { sx = x_1; sy = y_1; ex = x_2; ey = y_2; }
	    else { sx = x_2; sy = y_2; ex = x_1; ey = y_1; }
	    if(ex < sx) di = -1; else di = 1;
	    t = 2*dx - dy; i1 = 2*dx; i2 = 2*(dx-dy);
	    for(i = sx, j = sy; j <= ey; j++) {
		vals->info[0][j - sy] = (*getval)(i, j, set);
		vals->info[1][j - sy] = i;
		vals->info[2][j - sy] = j;
		if(t < 0) t += i1; else { i += di; t += i2; }
	    }
	} else {		/* Draw from left to right */
	    if(x_1 < x_2) { sx = x_1; sy = y_1; ex = x_2; ey = y_2; }
	    else { sx = x_2; sy = y_2; ex = x_1; ey = y_1; }
	    if(ey < sy) dj = -1; else dj = 1;
	    t = 2*dy - dx; i1 = 2*dy; i2 = 2*(dy-dx);
	    for(i = sx, j = sy; i <= ex; i++) {
		vals->info[0][i - sx] = (*getval)(i, j, set);
		vals->info[1][i - sx] = i;
		vals->info[2][i - sx] = j;
		if(t < 0) t += i1; else { j += dj; t += i2; }
	    }
	}
	break;
    }
}

/* Writes the array of grayvalues back into the proper place.
 */
void
undoline2d(vals)
     Trace *vals;
{
    int i;

    if(vals->info[0] == NULL || vals->length == 0) return;

    for(i = 0; i <= vals->length; i++)
	put_pix(&base.winfo, (int) vals->info[1][i], (int) vals->info[2][i], 
		(int) vals->info[0][i]);
    Cfree(vals->info[0], vals->length*3, unsigned);
    vals->info[0] = vals->info[1] = vals->info[2] = NULL;
    vals->length = 0;
}

static unsigned
get_and_set(x, y, s)
     int x, y, s;
{
    unsigned tmp;

    tmp = get_pix(&base.winfo, x, y);
    if(s)
	put_pix(&base.winfo, x, y, STANDOUT);
    return(tmp);
}

static unsigned
get_char_value(x, y, s)
     int x, y, s;
{
    if(s)
	put_pix(&base.winfo, x, y, STANDOUT);
    return((unsigned)base.buf.chars[base.winfo.width*y + x]);
}

static unsigned
get_short_value(x, y, s)
     int x, y, s;
{
    if(s)
	put_pix(&base.winfo, x, y, STANDOUT);

    return((unsigned)base.buf.shorts[base.winfo.width*y + x]);
}

static unsigned
get_long_value(x, y, s)
     int x, y, s;
{
    if(s)
	put_pix(&base.winfo, x, y, STANDOUT);

    return((unsigned)base.buf.longs[base.winfo.width*y + x]);
}

static unsigned
set_char_value(x, y, s)
     int x, y, s;
{
    if(s)
	put_pix(&base.winfo, x, y, STANDOUT);

    if(y > 0)
/*	base.buf.chars[base.winfo.width*(y-1) + x] = 
	    (unsigned char) base.extremes[1];*/
	base.buf.chars[base.winfo.width*(y-1) + x] = 0xFF;
    base.buf.chars[base.winfo.width*y + x] = 0;
    return(0);
}

static unsigned
set_short_value(x, y, s)
     int x, y, s;
{
    if(s)
	put_pix(&base.winfo, x, y, STANDOUT);

    if(y > 0)
	base.buf.shorts[base.winfo.width*(y-1) + x] = 
	    (unsigned short) base.extremes[1];
    base.buf.shorts[base.winfo.width*y + x] = 0;
    return(0);
}

static unsigned
set_long_value(x, y, s)
     int x, y, s;
{
    if(s)
	put_pix(&base.winfo, x, y, STANDOUT);

    if(y > 0)
	base.buf.longs[base.winfo.width*(y-1) + x] = base.extremes[1];
    base.buf.longs[base.winfo.width*y + x] = 0;
    return(0);
}

static void
drawline(x_1, y_1, x_2, y_2)
     int x_1, y_1, x_2, y_2;
{
    line(&base.winfo, x_1, y_1, x_2, y_2, STANDOUT);
}

static void
drawtrace(x_1, y_1, x_2, y_2)
     int x_1, y_1, x_2, y_2;
{
    Trace tmpline, *trace;
    unsigned *tmpvec[3], size;
    int i;

    doline2d(x_1, y_1, x_2, y_2, &tmpline, base.datasize, ON);

    if(tmpline.length <= 1) return;

    trace = &(cur_func->func.primit->data.trace);
    tmpvec[0] = trace->info[0];
    tmpvec[1] = trace->info[1];
    tmpvec[2] = trace->info[2];
    size = trace->length;
    size += tmpline.length-1;
    trace->info[0] = Calloc(3*size, unsigned);
    trace->info[1] = trace->info[0] + size;
    trace->info[2] = trace->info[0] + 2*size;
    if(trace->length > 0) {
	for(i = 0; i < trace->length; i++) {
	    trace->info[0][i] = tmpvec[0][i];
	    trace->info[1][i] = tmpvec[1][i];
	    trace->info[2][i] = tmpvec[2][i];
	}
    }
    /* There should be a 1-pixel overlap of the lines. */
    for(i = 1; i < tmpline.length; i++) {
	trace->info[0][trace->length+i-1] = tmpline.info[0][i];
	trace->info[1][trace->length+i-1] = tmpline.info[1][i];
	trace->info[2][trace->length+i-1] = tmpline.info[2][i];
    }
    Cfree(tmpline.info[0], 3*tmpline.length, unsigned);
    Cfree(tmpvec[0], 3*trace->length, unsigned);
    trace->length = size;
}

static void
writeline(x_1, y_1, x_2, y_2)
     int x_1, y_1, x_2, y_2;
{
    Trace tmpline;

    doline2d(x_1, y_1, x_2, y_2, &tmpline, base.datasize + 4, 1);
    Cfree(tmpline.info[0], 3*tmpline.length, unsigned);
}

/* This is just a front end to the obnoxiousness of fit() and
 * its cohorts.
 */
void 
fit_line(list, trace, mode, set, coords)
     Point list;		/* list of points */
     Trace *trace;		/* Trace to write to */
     unsigned mode;		/* readarr[] arg */
     enum truth set;		/* set or just read */
     int (*coords)[2];
{
    Point tmp;
    int length, i;
    float *x, *y, a, b;

    length = depth(list);

    x = Calloc(length, float);
    y = Calloc(length, float);

    for(i = 0, tmp = list; i < length; i++, tmp = tmp->next) {
	x[i] = (float) tmp->i.x; y[i] = (float) tmp->i.y;
    }

    fit(x, y, length, &a, &b, coords);

    doline2d(coords[0][0], coords[0][1], coords[1][0], coords[1][1], 
	     trace, mode, set);

    Cfree(x, length, float);
    Cfree(y, length, float);
}

/* This routine was taken from the Numerical Recipes file
 * "fit.c".  I stripped out such things as significance and
 * so on.
 */
static void 
fit(x, y, ndata, a, b, coords)
     float x[], y[], *a, *b;
     int ndata, (*coords)[2];
{
    int i;
    float t, sxoss, sx = 0.0, sy = 0.0, st2 = 0.0, ss;
    float minx = 65535.0, maxx = 0.0;

    *b = 0.0;
    for(i = 0; i < ndata; i++) {
	sx += x[i];
	sy += y[i];
	if(x[i] < minx) minx = x[i];
	if(x[i] > maxx) maxx = x[i];
    }
    ss = ndata;
    sxoss = sx/ss;
    for(i = 0; i < ndata; i++) {
	t = x[i] - sxoss;
	st2 += t*t;
	*b += t*y[i];
    }
    /* *b /= st2; */
    *b = *b / st2;
    *a = (sy - sx*(*b))/ss;

    coords[0][0] = (int) minx; 
    coords[0][1] = (int) (*a + *b * minx); 
    coords[1][0] = (int) maxx;
    coords[1][1] = (int) (*a + *b * maxx);
}
