/****************************************************************************
 * slant.c -- program written in sun's c spring 1990.                       *
 *--------------------------------------------------------------------------*
 * AUTHOR: Craig E. Thayer, for CMPS 508, Image Processing, at USL.         *
 *--------------------------------------------------------------------------*
 * OVERVIEW :                                                               *
 *    This program computes a slant transform of an input image then does   *
 * an inverse slant, and rewrites the image. The input should be available  *
 * through standard input in hips format.  The imagesize should be square,  *
 * and sides should be a power of two length.  The pixel cells should be    *
 * 8 bit grayscale.  The output is in the same format as the input, and is  *
 * sent to standard output.                                                 *
 *--------------------------------------------------------------------------*
 * STRUCTURE :                                                              *
 *    The actual transform performed is done in subroutines called "slant"  *
 * and islant.  These subroutines are quite lengthy, but have not been      *
 * divided in order to improve efficiency.                                  *
 * The main() function loads the hips file, calls the transform functions,  *
 * and restores a hips file.  
 *--------------------------------------------------------------------------*
 * INPUT/OUTPUT :                                                           *
 *    The input is a hips format image.  It must be square, with edges      *
 * being a power of two size.  There should be eight bits per pixel.        *
 * All input is recieved via the standard input stream.                     *
 *    The output is a hips image of the same format as the input.  It is    *
 * written to the standard output stream.  Redirection of this stream into  *
 * a file will save the image for later use.                                *
 ****************************************************************************/ 


#include <stdio.h>
#include <math.h>
#include <hipl_format.h>
float invsqrt2;


/**************************************************************************
 * Some of the functions access the image as a two dimensional array.  The*
 * details of the address calculation are handled with the following      *
 * definintion.  Here it is assumed that x is the unidimensional array,   *
 * and N is the length of a side of the two dimensional array.            *
 **************************************************************************/
#define W(i,j)	x[(i) + (j) * N]


/**************************************************************************
 * When image enhancement takes place, we must be careful to use only     *
 * positive numbers (negative ones have no meaning in the hips file).     *
 * The following function scans all pixels in an image.  Each pixel is    *
 * replaced by its absolute value.  The maximum and minimum over the      *
 * entire picture are returned.                                           *
 *------------------------------------------------------------------------*
 * mag() serves as a subroutine to enhance().  It is called from enhance()*
 **************************************************************************/
void mag(N, x, max, min, mean)
          int N;
          float x[];
          float *max, *min, *mean;
{
 float w,maximum,minimum,sum;
 int i,j;
   maximum = 0.0;
   minimum = 15000.0;
   sum = 0.0;
   for (i=0; i<N; i++) {
     for (j=0; j<N; j++) {
       w = fabs(W(i,j));
       sum = sum + w;
       W(i,j) = w;
       if (w > maximum) {
         maximum = w;
       } else {
         if (w < minimum) {
           minimum = w;
         }
       }
     }
   }
   *max = maximum;
   *min = minimum;
   *mean = sum / (N * N);
} 


/**************************************************************************
 * The resulting image may not have the correct contrast to be stored in  *
 * eight bit pixels.  This enhancement function examines the image for the*
 * maximum and minimum (using mag()).  A linear replacement of values then*
 * takes place so that the lowest value becomes black, and the highest    *
 * becomes white.                                                         *
 *------------------------------------------------------------------------*
 * enhance() is called directly from main()                               *
 **************************************************************************/
void enhance(N, x)
          int N;
          float x[];
{
 float max,min,mean,b,m,w;
 int i,j;
   mag(N, x, &max, &min, &mean);    /* abs value of image; find max/min */
   m = 255.0 / (max - min);         /* calculate slope for enhancement  */
   b = - min * m;                   /* calculate offset for dark black  */
   for (i=0; i<N; i++) {
     for (j=0; j<N; j++) {
       w = W(i,j) * m + b;          /* enhance each pixel for max contrast */
       if (w > 255.0) w = 255.0;
       W(i,j) = w;
     }
   }
}


/**************************************************************************
 * Like all matrix transformation opertors for two dimensional images,    *
 * the slant transform has one operation applied to rows, and another to  *
 * collumns.  In order to reduce the amount of programming we can reuse   *
 * the collumn operator on a transposed image.  This has the effect of    *
 * making the collumn operator into a row operator.                       *
 *  The v parameter to transpose is a constant multiplier for the matrix. *
 * Slant is supposed to apply a one over the square root of 2 constant at *
 * each stage of calculation.  It is easier, however, if all of the       *
 * constants are brought together and applied a single time.              *
 *------------------------------------------------------------------------*
 * transpose() is used inside of slant2d() and islant2d()                 *
 **************************************************************************/
void transpose(N, x, v) 
          int N;
          float x[];
          float v;
{
 float temp;
 int i,j;
   for (i=1; i<N; i++) {
     for (j=0; j<i; j++) {
       temp = W(i,j);
       W(i,j) = v * W(j,i);
       W(j,i) = v * temp;
     }
   }
}


/**************************************************************************
 * This is the forward slant transform collumn operator.  It is applied   *
 * to all collumns at once by, in essence, stacking the collumns of the   *
 * image into a one dimensional array.  Viewing the transform as a single *
 * dimensional function, it is no longer necessary to do the multiply in  *
 * the W() definition for each array access.                              *
 * IMPORTANT NOTE: After each application of a slant transform matrix to  *
 * a group of pixels, there is a reordering of the pixels.  This reordering
 * is to ensure that the frequencies of the components are in order.  The *
 * published form (found in Pratt) does not behave in such a manner.      * 
 *------------------------------------------------------------------------*
 * called by slant2d()                                                    *
 **************************************************************************/
void slant(N, x, P)
          int N;
          float x[];
          int P;
{
 float a,b,olda,f1,f2,f3,f4,temp2,temp3;
 int n,halfn,m,i,j,k,c;
 int xlow,xmid,xtop,xbot;
   m = N * N / 2;
   n = 2;
   xlow = 0;
   for (j=m; j>0; j--) {
     xmid = xlow+1;
     f1 = x[xlow] + x[xmid];
     f2 = x[xlow] - x[xmid];
     x[xlow] = f1;
     x[xmid] = f2;
     xlow = xlow + n;
   }
   a = 1.0;
   for (i=2; i<=P; i++) {
     olda = a;
     b = 1.0/sqrt(1.0 + 4.0 * olda * olda);
     a = 2.0 * b * olda;
     halfn = n;
     n = n * 2;
     m = m / 2;
     xlow = 0;
     for (j=m; j>0; j--) {
       xmid = xlow + halfn;
       f1 = x[xlow] + x[xmid];
       f2 = x[xlow] - x[xmid];
       f3 = x[xlow+1] + x[xmid+1];
       f4 = x[xlow+1] - x[xmid+1];
       x[xlow] = f1;
       x[xlow+1] = a*f2 + b*f3;
       temp2 = f4;
       temp3 = a*f3 - b*f2;
       for (k=2; k<halfn; k++) {
         xtop = k+xlow;
         xbot = xtop+halfn;
         f1 = x[xtop] + x[xbot];
         f2 = x[xtop] - x[xbot];
         x[xtop] = f1;
         x[xbot] = f2;
       }
       for (k=halfn; k>2; k=c) {
         c = k/2;
         x[k+xlow] = x[c+xlow];
       }
       x[xlow+2] = temp2;
       for (k=halfn+1; k>3; k=c) {
         switch (k & 3) {
           case 0: c = k / 2; break;
           case 1: c = n - (n - k + 1) / 2; break;
           case 2: c = n - (n - k) / 2; break;
           case 3: c = (k + 1) / 2 - 1; break;
         }
         x[k+xlow] = x[c+xlow];
       }
       x[xlow+3] = temp3;
       xlow = xlow + n;
     }
   }
}


/*************************************************************************
 * islant() reverses the effect of the application of slant().  Just as  *
 * with slant(), only one half of the transform is coded (the collumn    *
 * part).  The row operation is performed by transposing the original.   *
 *-----------------------------------------------------------------------*
 * called by islant2d()                                                  *
 *************************************************************************/
void islant(N, x, P)
          int N;
          float x[];
          int P;
{
 float a,b,f1,f2,f3,f4,nsquared;
 int n,halfn,m,i,j,k,c;
 int xlow,xmid,xtop,xbot;
   m = N;
   n = N;
   for (i=P; i>=2; i--) {
     nsquared = (float)n * (float)n;
     a = sqrt(0.75*nsquared / (nsquared - 1.0));
     b = sqrt((nsquared / 4.0 - 1.0) / (nsquared - 1.0));
     halfn = n / 2;
     xlow = 0;
     for (j=m; j>0; j--) {
       xmid = xlow + halfn;
       f1 = x[xlow];
       f2 = x[xlow+1];
       f3 = x[xlow+2];
       f4 = x[xlow+3];
       for (k=2; k<halfn; k=c) {
         c = k * 2;
         x[k+xlow] = x[c+xlow];
       }
       for (k=3; k!=halfn+1; k=c) {
         if (k < halfn) {
           if (k & 1)
             c = (k +1) * 2 - 1;
           else
             c = k * 2;
         } else {
           if (k & 1)
             c = n - (n - k) * 2;
           else
             c = n - (n - (k + 1)) * 2 - 1;
         }
         x[k+xlow] = x[c+xlow];
       }
       x[xlow] = f1 + a*f2 - b*f4;
       x[xlow+1] = b*f2 + f3 + a*f4;
       x[xmid] = f1 - a*f2 + b*f4;
       x[xmid+1] = b*f2 - f3 + a*f4;
       for (k=2; k<halfn; k++) {
         xtop = k+xlow;
         xbot = xtop+halfn;
         f1 = x[xtop] + x[xbot];
         f2 = x[xtop] - x[xbot];
         x[xtop] = f1;
         x[xbot] = f2;
       }
       xlow = xlow + n;
     }
     m = m * 2;
     n = halfn;
   }
   xlow = 0;
   for (j=m; j>0; j--) {
     xmid = xlow+1;
     f1 = x[xlow] + x[xmid];
     f2 = x[xlow] - x[xmid];
     x[xlow] = f1;
     x[xmid] = f2;
     xlow = xlow + n;
   }
}


/**********************************************************************
 * The two dimensional forward and inverse slant transforms are coded *
 * as collumn operators, a transpose, and then the collumn operator   *
 * again.  Constants are applied at the time of the transpose         *
 **********************************************************************/
void slant2d(N, x)
          int N;
          float x[];
{
 int P;
 float v;
   P = (int)rint(log((float)N)/log(2.0));
   v = pow(0.5, P);
   slant(N, x, P);
   transpose(N, x, v);
   slant(N, x, P); 
}


void islant2d(N, x)
          int N;
          float x[];
{
 int P;
 float v;
   P = (int)rint(log((float)N)/log(2.0));
   v = pow(0.5, P);
   islant(N, x, P);
   transpose(N, x, v);
   islant(N, x, P);
}


int main(argc, argv)
	int argc;
	char **argv;
{
 int imagesize, pixelsize;
 float *imagedata;
 struct header hdr;

   Progname = strsave(*argv);
   invsqrt2 = 1.0 / sqrt(2.0);
   read_header(&hdr);
   pixelsize = 1;              /* assume hdr.pixel_format == PFBYTE */
   imagesize = hdr.num_frame * hdr.orows * hdr.ocols * pixelsize;


   /* now we malloc memory, and read the data into imagedata */
   imagedata = (float *)malloc(imagesize * sizeof(float));
   fread(imagedata,imagesize,1,stdin);
   
                                /*************************************/
   slant2d(hdr.orows,imagedata); /*** THIS IS THE FORWARD TRANSFORM ***/
                                /*************************************/

   /*** processing on the slant transformed image should be called here ***/

                                /*************************************/
   islant2d(hdr.orows,imagedata);/*** THIS IS THE INVERSE TRANSFORM ***/
                                /*************************************/

   /* we enhance the output before casting back to 8 bit values */
   enhance(hdr.orows, imagedata);

   /* a header for the output is written to the stream */
   write_header(&hdr);

   /* finally, the image is sent out and memory is freed */
   fwrite(imagedata,imagesize,1,stdout);
}
