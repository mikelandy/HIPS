/*
 * writehipsb.c - Connection to a hips file - byte output version
 *
 * writehipsb(filename,matrix)
 *
 * writes in pixel format PFBYTE using only the real part of the input
 * matrix with no scaling applied.  The input is rounded and is not
 * range-checked.
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
	double *invalr;
	byte *pout;
        char *fname;
        int i,j,rows,cols;
        FILE *fp;
	struct header hd;

	Progname = "writehipsb";
	hipserrlev = HEL_SEVERE+1;

	/* Check for proper number of arguments */

	if (nrhs != 2)
		mexErrMsgTxt("Usage: writehipsb('filename',matrix)");
        if (mxGetM(FLNAME) > 1)
		mexErrMsgTxt("writehipsb: first argument must have one row");
        if (mxIsChar(FLNAME) != 1)
		mexErrMsgTxt("writehipsb: filename must be a string");
        if (mxIsDouble(INMAT) != 1)
		mexErrMsgTxt("writehipsb: matrix must be double-precision float");
	if (mxIsComplex(INMAT) == 1)
		mexErrMsgTxt("writehipsb: matrix can not be complex");
	if (mxGetNumberOfDimensions(INMAT) != 2)
		mexErrMsgTxt("writehipsb: matrix must have only 2 dimensions");
        invalr = mxGetPr(INMAT);
	fname = mxArrayToString(FLNAME);
        if ((fp = ffopen(fname,"w")) == (FILE *) HIPS_ERROR)
		mexErrMsgTxt(hipserr);
        rows = (int) mxGetM(INMAT); /* m is the rows of the matrix */
        cols = (int) mxGetN(INMAT); /* n is the columns of the matrix */
        init_header(&hd,"","",1,"",rows,cols,PFBYTE,1,
		"Created from Matlab by writehipsb()");
	if ((hd.image = (byte *) mxMalloc(hd.sizeimage)) == 0)
		mexErrMsgTxt("writehipsb: can't allocate image memory");
	pout = hd.image;
	for (i=0;i<rows;i++)
		for (j=0;j<cols;j++)
			*pout++ = invalr[j*rows + i] + 0.5;
        mexPrintf("writehipsb: writing file %s, rows = %d, cols = %d\n",
		fname,rows,cols);
        if (fwrite_header(fp,&hd,fname) == HIPS_ERROR)
		mexErrMsgTxt(hipserr);
        if (fwrite_image(fp,&hd,0,fname) == HIPS_ERROR)
		mexErrMsgTxt(hipserr);
	mxFree(hd.image);
	fclose(fp);
}
