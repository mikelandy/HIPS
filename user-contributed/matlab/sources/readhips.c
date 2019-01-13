/*
 * Connection to a hips file
 * Modified for version 4.0 of Matlab
 * Ramin Samadani - 10/7/92
 * HIPS 2 version Ramin Samadani/Mike Landy - 10/13/92
 * caught up to Matlab 7 - Mike Landy - 1/14/09
 *
 * [outval] = readhips(inval)
 * where inval is a hips file name and outval is a matrix
 * outval is either REAL or COMPLEX depending on format of the HIPS file
 * only uses the first frame of the input sequence
 */

#include <stdio.h>
#include <math.h>
#include <hipl_format.h>
#include "matrix.h"

#define	FLNAME	prhs[0]
#define	OUTMAT	plhs[0]

int types[] = {PFDOUBLE,PFDBLCOM,LASTTYPE};
int mexPrintf();
void mexErrMsgTxt();

void mexFunction(nlhs,plhs,nrhs,prhs)

int nlhs,nrhs;
mxArray **plhs;
const mxArray **prhs;

{
	double *outvalr,*outvali,*pin;
	char *fname;
	struct header hd,hdp;
        FILE *fp;
	int rows,cols,method,i,j;

	Progname = "readhips";
	hipserrlev = HEL_SEVERE+1;

	/* Check for proper number of arguments */

	if (nrhs != 1 || nlhs != 1)
		mexErrMsgTxt("Usage: matrix = readhips('filename')");
        if (mxGetM(FLNAME) > 1)
		mexErrMsgTxt("readhips: first argument must have one row");
	if (mxIsChar(FLNAME) != 1)
		mexErrMsgTxt("readhips: filename must be a string");
        fname = mxArrayToString(FLNAME);
        if ((fp = ffopen(fname,"r")) == (FILE *) HIPS_ERROR)
		mexErrMsgTxt(hipserr);
	if (fread_hdr_a(fp,&hd,fname) != HIPS_OK)
		mexErrMsgTxt(hipserr);
	if ((method = fset_conversion(&hd,&hdp,types,fname)) == HIPS_ERROR)
		mexErrMsgTxt(hipserr);
	rows = hd.orows;
	cols = hd.ocols;
        mexPrintf("readhips: reading from %s, rows = %d, cols = %d\n",
		fname,rows,cols);
	if (fread_imagec(fp,&hd,&hdp,method,0,fname) != HIPS_OK)
		mexErrMsgTxt(hipserr);

	/* Create a matrix for the return argument and fill it */

	if (hdp.pixel_format == PFDOUBLE) {
		OUTMAT = mxCreateDoubleMatrix(rows,cols,mxREAL);
		outvalr = mxGetPr(OUTMAT);
		pin = (double *) hdp.image;
		for (i=0;i<rows;i++)
			for (j=0;j<cols;j++)
				outvalr[j*rows+i] = *pin++;
	}
	else {
		OUTMAT = mxCreateDoubleMatrix(rows,cols,mxCOMPLEX);
		outvalr = mxGetPr(OUTMAT);
		outvali = mxGetPi(OUTMAT);
		pin = (double *) hdp.image;
		for (i=0;i<rows;i++) {
			for (j=0;j<cols;j++) {
				outvalr[j*rows+i] = *pin++;
				outvali[j*rows+i] = *pin++;
			}
		}
	}
}
