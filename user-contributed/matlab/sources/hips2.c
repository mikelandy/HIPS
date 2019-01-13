/*
 * Calls a hips program on an image from MATLAB
 *
 * an example of a call to hips2 is "hips2('genframe -s 4 4 -g 4',[],0,1)
 *
 * The arguments are:
 * 1) a string containing a single hips command, no pipes!
 * 2) an input "image", a matlab full matrix.
 * 3) nonzero if the command requires an input image
 * 4) nonzero if the command returns an output image
 *
 * not much checking is done. You're on your own in getting the right
 * flags with the right command.
 *
 * Ramin Samadani
 * HIPS 2 - msl - 10/19/92
 * updated for Matlab 7 - rld/msl - 1/14/09
 * For this last update:
 *	Matlab doesn't set PATH correctly, so give full path to HIPS program
 *	Added a pipe so stderr output from child process is captured
 */

#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <hipl_format.h>
#include "mex.h"
#include "hips2.h"

#define COMMAND prhs[0]
#define INIMAGE prhs[1]
#define INFLAG  prhs[2]
#define OUTFLAG prhs[3]
#define OUTIMAGE plhs[0]

char path[300] = DESTDIR;	/* Matlab screws up the PATH environment var */

int pipe(),fork(),close(),dup(),execvp();
void server2(),parse(),read_pipe_hips(),write_pipe_hips(),mexErrMsgTxt();

void mexFunction(nlhs,plhs,nrhs,prhs)

int nlhs,nrhs;
mxArray **plhs;
const mxArray **prhs;

{
	double  *invalr,*invali,*dinflag,*doutflag,*pim;
	double *outvalr,*outvali;
	char *commandbuf,*argvv[10],buf[200];
	int len,i,j,rows,cols,childpid,pipe1[2],pipe2[2],pipe3[2];
	struct header hdi,hdo;
	h_boolean cflag,iflag,oflag;
	FILE *fp;

	Progname = "hips2";
	hipserrlev = HEL_SEVERE+1;
	if (nrhs != 4)
		mexErrMsgTxt("Usage: hips2('command',im,inflag,outflag)");
	if (mxGetM(COMMAND) > 1)
		mexErrMsgTxt("hips2: command must have one row");
	if (mxIsChar(COMMAND) != 1)
		mexErrMsgTxt("hips2: command name must be a string");
	if (mxIsDouble(INFLAG) != 1)
		mexErrMsgTxt("hips2: inflag must be double-precision float");
	if (mxIsDouble(OUTFLAG) != 1)
		mexErrMsgTxt("hips2:outflag must be double-precision float");
	dinflag = mxGetPr(INFLAG);
	doutflag = mxGetPr(OUTFLAG);
	commandbuf = mxArrayToString(COMMAND);
	iflag = (dinflag[0] != 0);
	oflag = (doutflag[0] != 0);
	if (pipe(pipe1) < 0 || pipe(pipe2) < 0 || pipe(pipe3) < 0)
		mexErrMsgTxt("hips2: can't create pipes");
	if ((childpid = fork()) < 0)
		mexErrMsgTxt("hips2: can't fork");
	else if (childpid == 0) {
		close(pipe1[1]);
		close(pipe2[0]);
		close(pipe3[0]);
		server2(pipe1[0],pipe2[1],pipe3[1],commandbuf);
		close(pipe1[0]);
		close(pipe2[1]);
		close(pipe3[1]);
		exit(1);
	}
	close(pipe1[0]);
	close(pipe2[1]);
	close(pipe3[1]);
	if (iflag) {
		invalr = mxGetPr(INIMAGE);
		rows = (int) mxGetM(INIMAGE);
		cols = (int) mxGetN(INIMAGE);
		cflag = mxIsComplex(INIMAGE);
		if (cflag)
			invali = mxGetPi(INIMAGE);
		init_header(&hdi,"","",1,"",rows,cols,
			cflag ? PFDBLCOM : PFDOUBLE,1,"");
		if ((hdi.image = (byte *) mxMalloc(hdi.sizeimage)) == 0)
			mexErrMsgTxt("hips2: can't allocate input image");
		pim = (double *) hdi.image;
		if (cflag) {
			for (i=0;i<rows;i++) {
				for (j=0;j<cols;j++) {
					*pim++ = invalr[j*rows+i];
					*pim++ = invali[j*rows+i];
				}
			}
		}
		else {
			for (i=0;i<rows;i++)
				for (j=0;j<cols;j++)
					*pim++ = invalr[j*rows+i];
		}
		mexPrintf("hips2: running command %s, rows = %d, cols = %d\n",
			commandbuf,rows,cols);
		write_pipe_hips(pipe1[1],&hdi);
	}
	else
		mexPrintf("hips2: running command %s\n",commandbuf);
	if ((fp = fdopen(pipe3[0],"r")) == NULL)
		mexPrintf("can't fdopen stderr pipe\n");
	else {
		while(fgets(buf,200,fp) != NULL)
			mexPrintf("%s",buf);
	}
	fclose(fp);
        if (oflag) {
		read_pipe_hips(pipe2[0],&hdo);
		while (1) {
			i = wait((int *) 0);
			if (i == -1)
				break;
			else if (i == childpid) {
				break;
			}
		}
		rows = hdo.orows;
		cols = hdo.ocols;
		if (hdo.pixel_format == PFDOUBLE) {
			OUTIMAGE = mxCreateDoubleMatrix(rows,cols,mxREAL);
			outvalr = mxGetPr(OUTIMAGE);
			pim = (double *) hdo.image;
			for (i=0;i<rows;i++)
				for (j=0;j<cols;j++)
					outvalr[j*rows+i] = *pim++;
		}
		else {
			OUTIMAGE = mxCreateDoubleMatrix(rows,cols,mxCOMPLEX);
			outvalr = mxGetPr(OUTIMAGE);
			outvali = mxGetPi(OUTIMAGE);
			pim = (double *) hdo.image;
			for (i=0;i<rows;i++) {
				for (j=0;j<cols;j++) {
					outvalr[j*rows+i] = *pim++;
					outvali[j*rows+i] = *pim++;
				}
			}
		}
	}
}

int types[] = {PFDOUBLE,PFDBLCOM,LASTTYPE};

void read_pipe_hips(pipein,hdp)

int pipein;
struct header *hdp;

{
	FILE *fp;
	struct header hd;
	int method;

	if ((fp = fdopen(pipein,"r")) == NULL)
		mexErrMsgTxt("hips2: can't fdopen input pipe");
	if (fread_hdr_a(fp,&hd,"<Matlab input pipe>") != HIPS_OK)
		mexErrMsgTxt(hipserr);
	if ((method = fset_conversion(&hd,hdp,types,"<Matlab input pipe>"))
		== HIPS_ERROR)
			mexErrMsgTxt(hipserr);
	if (fread_imagec(fp,&hd,hdp,method,0,"<Matlab input pipe>") != HIPS_OK)
		mexErrMsgTxt(hipserr);
	fclose(fp);
}

void write_pipe_hips(pipeout,hd)

int pipeout;
struct header *hd;

{
	FILE *fp;

	if ((fp = fdopen(pipeout,"w")) == NULL)
		mexErrMsgTxt("hips2: can't fdopen output pipe");
	if (fwrite_header(fp,hd,"<Matlab output pipe>") == HIPS_ERROR)
		mexErrMsgTxt(hipserr);
	if (fwrite_image(fp,hd,0,"<Matlab output pipe>") == HIPS_ERROR)
		mexErrMsgTxt(hipserr);
	fclose(fp);
}

#define MAXARG 100
#define MAXBUF 200

void server2(ipcreadfd,ipcwritefd,ipcerrfd,buf)

int ipcreadfd,ipcwritefd,ipcerrfd;
char *buf;

{
	char *args[MAXARG],argsbuf[MAXARG][MAXBUF];
	int i;
	FILE *fp;

	for (i=0;i<MAXARG;i++) 
		args[i] = &argsbuf[i][0];
	parse(buf,args);
	strcat(path,"/");
	strcat(path,args[0]);
	close(0);
	dup(ipcreadfd);
	close(1);
	dup(ipcwritefd);
	close(2);
	dup(ipcerrfd);
	execvp(path,args);
	if ((fp = fdopen(2,"w")) != NULL) {
		fprintf(fp,"hips2: exec of '%s' failed, error code %d\n",
			path,errno);
		fclose(fp);
	}
	else
		close(2);
	close(0);
	close(1);
	exit(1);
}

/*
 * parse--split the command in buf into
 *         individual arguments.
 */

void parse(buf,args)

char *buf;
char **args;

{
	while (*buf != 0) {
		while ((*buf == ' ') || (*buf == '\t'))
			*buf++ = 0;
		if (*buf != 0)
			*args++ = buf;
		while ((*buf != 0) && (*buf != ' ') && (*buf != '\t'))
			buf++;
	}
	*args = (char *) 0;
}

