/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * dpcm_r.c -- DPCM decoding program.
 *
 * usage: dpcm_r [-f file-name] < inseq >oseq
 *
 * The exact DPCM method is determined by the parameters file: "file-name".
 * If the file is not specified, default parameters are supplied-
 * see the routine "init_param" and the description below.
 *
 * The exact format for the parameters file is detailed below.
 *
 * To load: cc -o dpcm_r dpcm_r.c -lhips -lm
 *
 * Yoav Cohen, 6/15/82
 * HIPS 2 - msl - 7/23/91
 *
 * Parameters file format
 * **********************
 *
 *    There are three sections to the file, Each section starts with a single
 * capital letter (the letters are M,P and Q).
 *    The first line of the file (the M-line) specifies the method for
 * computing the mean. The line consists of three values: the first is the
 * capital letter M, the second is a letter in small case that specifies
 * the method (right now only the 'c' method -- "constant mean" -- is
 * implemented), the third is an integer which is the value for the mean.
 *    The second line of the file (the P-line) gives information about the
 * prediction method. The first character is the capital letter P, the
 * second is a letter in small case which specifies the prediction method
 * (right now only the 'l' method -- for linear prediction -- is implemented),
 * the third value is an integer 'n', which tells how many predictors there
 * are. This line is followed by n lines, a line for each predictor.
 * Each of these lines starts with a real number followed by three integers.
 * The real number is the weight for the predictor, the other numbers
 * give the signed difference between the coordinates of the predicted
 * pixel and those of the predicting pixels. The order of the coordinates
 * is: frame, row, column. For example, if a weight of .3 is given to
 * the pixel which is one line below the predicted pixel, but is on the
 * same column and the same frame, the line for this predictor would read:
 *	.3 0 -1 0
 *    The third section of the file starts with the Q-line, a Q followed by
 * a letter in small case for the quantization method (right now only
 * the f method -- for fixed quantization -- is implemented), followed
 * by the number of the quantization cutoff-points (the number of quantization
 * levels is 1 more than the number of the quantization cuts).
 * The next lines specify the quantization cutoff-points and levels.
 * There are two integers per line. The first is the upper bound 
 * and the second is the level for each quantization interval.
 * The upper bound for the last interval is not specified; hence the
 * last line contains only one value -- that for the average level of the
 * last interval.
 *    As an example consider a file for the default values for the
 * program:
 *
 *	M c 128
 *	P l 3
 *	.3 0 0 -1
 *	.3 0 -1 0
 *	.25 -1 0 0
 *	Q f 3
 *	-13 -20
 *	0 -5
 *	13 5
 *	20
 */

#include <stdio.h>
#include <hipl_format.h>

#define CONST 10	/* mean_method: constant value for mean 	*/
#define FRAME 11	/* mean_method: mean computed on each frame	*/
#define RUN 12		/* mean_method: mean of last n elements		*/
#define LINEAR 20	/* prediction-method: linear combination	*/
#define FIXED 3		/* quantization method: non-dynamic		*/

#define MAX_N_CUTS 16	/* maximum number of cutoff points for quantizer*/
#define MAX_N_PRED 20	/* maximum number of predictors			*/

static Flag_Format flagfmt[] = {
	{"f",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTFILENAME,"","predfile"},
		LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};

short int **estore;	/* storage of predicted pixels			*/
int frames_saved,ncolumns,nrows,addrows,addcols;
int quant_cuts,quant_bits,q[MAX_N_CUTS],iq[MAX_N_CUTS+1];
int current,row,col,out_decoder;
int mean_param,mean_method,frame_mean;
int pred_method,quant_method;
int n_predictors,r_frame[MAX_N_PRED],r_line[MAX_N_PRED],r_col[MAX_N_PRED];
int r_frame_offset[MAX_N_PRED],r_offset[MAX_N_PRED];
double r[MAX_N_PRED];
struct header hd,hdp,hdo;
int init_param(),read_param(),dpcm_init(),dpcm(),predict(),quantizer();
int inv_quantizer();
void test_params();

int main(argc,argv)

int argc;
char **argv;

{
	short int *te;
	int i,j,frame,intoutput,method;
	byte *getpix,*putpix;
	Filename filename,predfile;
	h_boolean fileflag;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&fileflag,&predfile,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	alloc_image(&hdo);
	write_headeru(&hdo,argc,argv);

	init_param();
	if (fileflag)
		read_param(predfile);
	test_params();

/* allocate core */

	estore = (short int **) halloc(frames_saved,sizeof(short int *));

	for (i=0;i<frames_saved;i++)
		estore[i] = (short int *)
			halloc(nrows*ncolumns,sizeof(short int));

/* initialize */

	for (i=0;i<frames_saved;i++)
	    for (j=0;j<nrows*ncolumns;j++)
		estore[i][j]=0;
	dpcm_init();
	current=frames_saved-1;

	for (frame=0;frame<hdp.num_frame;frame++) {
		getpix=hdp.image; putpix=hdo.image;
		fread_imagec(fp,&hd,&hdp,method,frame,filename);
		for (i=0;i<n_predictors;i++)
			r_frame_offset[i]=current+r_frame[i];
		for (row=addrows;row<nrows;row++)
		    for (col=addcols;col<ncolumns;col++) {
			intoutput=dpcm((int) *getpix++);
			if (intoutput<0 || intoutput>255) {
				fprintf(stderr,"%s: warning: pixel \
 %d %d %d out of range\n",Progname,frame,row,col);
				if (intoutput<0)
					intoutput=0;
				else 
					intoutput=255;
			}
			*putpix++ = intoutput;
		}
		te=estore[0];
		for (i=0;i<frames_saved -1; i++)
			estore[i]=estore[i+1];
		estore[frames_saved-1]=te;
		write_image(&hdo,frame);
	}
	return(0);
}

int init_param()

{
	quant_cuts=4;
	frame_mean=128;
	mean_param=128;
	mean_method=CONST;
	pred_method=LINEAR;
	quant_method=FIXED;
	
	quant_cuts=3;
		q[0]= -13; q[1]= 0; q[2]=13;
		iq[0] = -20; iq[1]= -5; iq[2]=5; iq[3]=20;
	n_predictors=3;
		r[0]=.3; r_frame[0]=0; r_line[0]=0; r_col[0]= -1;
		r[1]=.3; r_frame[1]=0; r_line[1]= -1; r_col[1]=0;
		r[2]=.25; r_frame[2]= -1; r_line[2]=0; r_col[2]=0;
	return(0);
}

int read_param(fname)

Filename fname;

{
	FILE *fp;
	char c1[2],c2[2],msg[100];
	int i;

	if ((fp=fopen(fname,"r"))==NULL) {
		sprintf(msg,"can't open parameters file %s",fname);
		perr(HE_MSG,msg);
	}
	fscanf(fp,"%1s %1s %d",c1,c2,&mean_param);
	if (c1[0] != 'M')
		perr(HE_MSG,"error in M-line of parameters file");
	switch (c2[0]) {
		case 'c':	mean_method=CONST; break;
		case 'f':	mean_method=FRAME; break;
		case 'r':	mean_method=RUN; break;
		default:	perr(HE_MSG,"error in parameters");
	}
	fscanf(fp,"%1s %1s %d",c1,c2,&n_predictors);
	if (c1[0] != 'P')
		perr(HE_MSG,"error in P-line of parameters file");
	switch (c2[0]) {
		case 'l':	pred_method=LINEAR; break;
		default:	perr(HE_MSG,"error in parameters");
	}
	if ((n_predictors<0)||(n_predictors>MAX_N_PRED))
		perr(HE_MSG,"number of predictors out of limits");
	for (i=0;i<n_predictors;i++)
		fscanf(fp,"%lf %d %d %d",
			&r[i],&r_frame[i],&r_line[i],&r_col[i]);
	fscanf(fp,"%1s %1s %d ",c1,c2,&quant_cuts);
	if (c1[0] != 'Q')
		perr(HE_MSG,"error in Q-line of parameters file");
	switch (c2[0]) {
		case 'f':	quant_method=FIXED; break;
		default:	perr(HE_MSG,"error in parameters");
	}
	if ((quant_cuts<1)||(quant_cuts>MAX_N_CUTS))
		perr(HE_MSG,"number of quantizer cuts is out of limits");
	for (i=0;i<quant_cuts;i++)
		if (fscanf(fp,"%d %d",&q[i],&iq[i])==EOF)
			perr(HE_MSG,"unexpected EOF in parameters file");
	if (fscanf(fp,"%d",&iq[quant_cuts])==EOF)
		perr(HE_MSG,"unexpected EOF in parameters file");
	return(0);
}

void test_params()

{
	int i;
	int last;

	frames_saved=1;
	addrows=0; addcols=0;

	for (i=0;i<n_predictors;i++) {
		if (abs(r_frame[i])>(frames_saved-1))
			frames_saved=abs(r_frame[i])+1;
		if (r_frame[i]*hd.ocols*hd.orows + r_line[i]*hd.ocols + r_col[i]
			>= 0)
			perr(HE_MSG,"error in predictor index");
		if (r_line[i]<addrows)addrows=r_line[i];
		if (r_col[i]<addcols)addcols=r_col[i];
	}
	addrows= -addrows; addcols= -addcols;
	ncolumns=hd.ocols+addcols;
	nrows=hd.orows+addrows;

	last=q[0];
	for (i=1;i<quant_cuts;i++) {
		if (q[i]<last)
			perr(HE_MSG,"quantizer cuts are out of order");
		if (q[i]==last)
			perr(HE_IMSG,"two quantizer-cuts of the same value");
		last=q[i];
	}

	for (i=0;i<n_predictors;i++)
		r_offset[i]=r_line[i]*ncolumns+r_col[i];
	if (mean_method==CONST)
		frame_mean=mean_param;
}

int dpcm_init() 

{
	out_decoder=0; 
	return(0);
}

int dpcm(in) 

int in;

{
	int output;

	output=inv_quantizer(in)+out_decoder;
	out_decoder=predict(output,estore);
	return(output+frame_mean);
}

/* predict - encoding & decoding functions for DPCM */

int predict(in,store)

int in;
short int **store;

{
	double sum;
	int i,offset,out;

	offset=row*ncolumns+col;
	store[current][offset]=in;
	offset++;

	sum=0;
	for (i=0;i<n_predictors;i++)
		sum += store[r_frame_offset[i]][r_offset[i]+offset] * r[i];
	out=sum;
	if (out<(-128))
		out=(-128);
	else if (out>127)
		out=127;
	return(out);
}

/* quantizer */

int quantizer(inq)

int inq;

{
	int *qi,i;

	qi=q;
	for (i=0;i<quant_cuts;i++)
		if (inq<*qi++) return(i);
	return(quant_cuts);
}

int inv_quantizer(in)

int in;

{
	if (in<0 || in>quant_cuts)
		perr(HE_MSG,"incorrect input to inv.quantizer");
	return(iq[in]);
}
