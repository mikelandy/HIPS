/***********************************************************************/
#include <stdio.h>
#include <sys/types.h>
#define KERNEL KERNEL
#include <sundev/ipfbreg.h>
#include "convolve.h"
extern struct ipfb_reg  *reg;

fb_maxmin(fb,row,col,irow,icol,pmax,pmin)
short int *pmax, *pmin;
int row,col,irow,icol,fb;

{   int     i,
            j;
    short int x,y,pixel;
    unsigned  int mx,mn, upixel;
    x = X (fb) ;
    y = Y (fb) ;
    X (fb)= icol;
    Y (fb)= irow;
    *pmin = *pmax = (short) DATA (fb);
    mx= mn = 0x0000ffff & *pmin  ;

    for (i = 0; i < row; i++)
    {  X (fb) = icol;  /* X is incremented automatically after each read */
       Y (fb) = irow+i;
       for (j = 0; j < col; j++)
       {  pixel = (short) DATA (fb);
          upixel = 0x0000ffff & pixel;
          *pmin = *pmin <= pixel ? *pmin : pixel;
          *pmax = *pmax >= pixel ? *pmax : pixel;
          mn = mn<=upixel ? mn : upixel;
          mx = mx>=upixel ? mx : upixel;
       }
    }
    X (fb) = x ;
    Y (fb) = y ;
    fprintf (stderr, "MIN-PIX (SIGNED DEC) =%d\n", *pmin);
    fprintf (stderr, "MAX-PIX (SIGNED HEX) =%x\n", *pmax);
    fprintf (stderr, "UNSIGNED HEX: MAX-PIX =%x,  MIN-PIX=%x\n", mx, mn);
}
