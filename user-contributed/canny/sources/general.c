#include <math.h>

/* A collection of useful utility functions. */

/* Max and min do the obvious thing on integers. */

int max(x,y) int x,y; { return((x > y) ? x : y); }
int min(x,y) int x,y; { return((x < y) ? x : y); }

/*  Nearestint takes a float argument and rounds to the nearest integer. */

int nearestint(x) 
    float x; 
{   
    if (x >= 0) 
        return((int) (x + .5));
    else
        return((int) (x - .5));
}

short nearestshort(x)
    float x;
{
    if (x >= 0) 
        return((short) (x + .5));
    else
        return((short) (x - .5));
}


/* This routines takes two integers in small and big and return them so that */
/* the smaller is in small and the larger is in big */

void order(small, big)
    int *small, *big;
{   
    int temp;

    if ((temp = *small) > *big)
    {
        *small = *big;
        *big = temp;
    }
}

/* Same thing for floats */
 
void fporder(big,small)
    float *big,*small;
{
    float b,s;

    if ((b = *big) < (s = *small))
    {
        *small = b;
        *big = s;
    }
}  

/* Clip a floating point number to bound1 and bound2 (they need not be ordered)  */

float fpclip(x,bound1,bound2)
    float x,bound1,bound2;
{ 
    fporder(&bound1,&bound2);
    
    if (x > bound1) 
       return(bound1);

    if (x < bound2)
        return(bound2);

    return(x);
}

