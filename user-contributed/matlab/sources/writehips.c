/*
 * writehips.c - Connection to a hips file
 *
 * writehips(filename,matrix)
 *
 * writes in pixel format PFDOUBLE or PFDBLCOM depending upon content of the
 * supplied matrix
 */

#include <sys/stat.h>
#include <sys/file.h>
#include <stdio.h>
#include <math.h>
#include <hipl_format.h>
#include "mex.h"

#define	INMAT	prhs[1]
#define FLNAME  prhs[0]

void mexFunction(nlhs,plhs,nrhs,prhs)

int nlhs,nrhs;
mxArray **plhs;
const mxArray **prhs;

{
	double *invalr,*invali,*pout;
	char *fname;
        int i,j,len,rows,cols;
        FILE *fp;
	h_boolean cflag;
	struct header hd;

	Progname = "writehips";
	hipserrlev = HEL_SEVERE+1;

	/* Check for proper number of arguments */

	if (nrhs != 2)
		mexErrMsgTxt("Usage: writehips('filename',matrix)");
        if (mxGetM(FLNAME) > 1)
		mexErrMsgTxt("writehips: first argument must have one row");
        if (mxIsChar(FLNAME) != 1)
		mexErrMsgTxt("writehips: filename must be a string");
        if (mxIsDouble(INMAT) != 1)
		mexErrMsgTxt("writehips: matrix must be double-precision float");
	if (mxGetNumberOfDimensions(INMAT) != 2)
		mexErrMsgTxt("writehips: matrix must have only 2 dimensions");
        invalr = mxGetPr(INMAT);
	cflag = mxIsComplex(INMAT);
	if (cflag)
		invali = mxGetPi(INMAT);
        fname = mxArrayToString(FLNAME);
        if ((fp = ffopen(fname,"w")) == (FILE *) HIPS_ERROR)
		mexErrMsgTxt(hipserr);
        rows = (int) mxGetM(INMAT); /* m is the rows of the matrix */
        cols = (int) mxGetN(INMAT); /* n is the columns of the matrix */
        init_header(&hd,"","",1,"",rows,cols,cflag ? PFDBLCOM : PFDOUBLE,1,
		"Created from Matlab by writehips()");
	if ((hd.image = (byte *) mxMalloc(hd.sizeimage)) == 0)
		mexErrMsgTxt("writehips: can't allocate image memory");
	pout = (double *) hd.image;
	if (cflag) {
		for (i=0;i<rows;i++)
			for (j=0;j<cols;j++) {
				*pout++ = invalr[j*rows + i];
				*pout++ = invali[j*rows + i];
			}
	}
	else {
		for (i=0;i<rows;i++)
			for (j=0;j<cols;j++)
				*pout++ = invalr[j*rows + i];
	}
        mexPrintf("writehips: writing file %s, rows = %d, cols = %d\n",
		fname,rows,cols);
        if (fwrite_header(fp,&hd,fname) == HIPS_ERROR)
		mexErrMsgTxt(hipserr);
        if (fwrite_image(fp,&hd,0,fname) == HIPS_ERROR)
		mexErrMsgTxt(hipserr);
	mxFree(hd.image);
	fclose(fp);
}
