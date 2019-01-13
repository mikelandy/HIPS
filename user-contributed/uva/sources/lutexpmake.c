/*	PROGRAM
 *		lutexpmake
 *
 *	PURPOSE
 *		to define a LUT based on a color file with the option of 
 *		defining both color and intensity.
 *		use exponential instead of linear interpolation
 *
 *	SYNOPSIS
 *              lutexpmake <filename> [-numTC] > lut data file
 *	
 *      -  the format of the color file is the following:
 *         +  the first line must be a comment line
 *         +  all following lines contain, in the following order:
 *		beginning value, ending value, low intensity value, 
 *			high intensity value, resistor color code
 *
 *      -  the program uses a similar file to map the color codes
 *		into the specified LUT
 *
 *	-  the optional argument is the number of time constants each segment
 *		will span.
 *
 *	AUTHOR
 *
 *		Chuck Carman
 *		for
 *		Merickel Imaging Lab
 *		Biomedical Engineering
 *		University of Virginia
 *		Charlottesville, VA 22903
 */
#include <stdio.h>
#include <hipl_format.h>

#define	NUM_TC	3.0		/* number of time constants */
#define	MAX_VAL	256.0		/* maximum value for 8 bit lut */

int main(argc,argv)
	int argc;
	char *argv[];
{
	FILE *fd,*cfd;
	struct header hd;
	double dtmp, exp(), atof();
	float base, dvsr, inc, temp, numtc;
	int  i, j, cnum, maxnum, done, blue, green, red;
	int  begin, end, low, high, val, inc_flg;
	short cbuf[3][20], color;
	unsigned char obuf[3][256], *bp;
	char name[80], infilnm[80],tmp[100];

	Progname = strsave(*argv);
	if (argc < 2 || argc > 3) {
		sprintf(tmp,"usage: %s <infile> [-numTC]",argv[0]);
		perr(HE_MSG,tmp);
	}

	numtc = NUM_TC;
	for (i=1; i<argc; i++) {
		switch (*argv[i]) {
		case '-':
			numtc = atof(&argv[i][1]);
			break;
		case '/':
		case '.':
			strcpy(infilnm,argv[i]);
			break;
		default:
			strcpy(infilnm,"/usr/spool/images/lut/");
			strcat(infilnm,argv[i]);
			break;
		}
	}

	if ((fd = fopen (infilnm, "r")) == NULL) {
		sprintf(tmp,"cannot open color file %s",infilnm);
		perr(HE_MSG,tmp);
	}

	/* read past the first line as it is a comment line */
	if ((done = fscanf(fd,"%[^\n]\n",name)) == 0) {
		sprintf(tmp,"cannot read color file %s",infilnm);
		perr(HE_MSG,tmp);
	}

	/* set up the color reference buffers */
	sprintf(name,"/usr/spool/images/lut/lutmake.map");
	if ((cfd = fopen(name,"r")) == NULL)
		perr(HE_MSG,"cannot open lutmake.map");
	/* read past first line */
	if ((done = fscanf(cfd,"%[^\n]\n",name)) == 0)
		perr(HE_MSG,"cannot read lutmake.map");
	/* read past second line and get number of colors defined */
	if ((done = fscanf(cfd,"%[^=]%*s%d\n",name,&maxnum)) == 0)
		perr(HE_MSG,"cannot read lutmake.map");
	if (maxnum >= 20)
		perr(HE_MSG,"too many colors defined");
	
	for (i=0; i<maxnum; i++)  {
		done = fscanf(cfd,"%d%d%d%d\n",&cnum,&blue,&green,&red);
		if (0<=done && done<3)
			perr(HE_MSG,"incorrect lutmake.map file");
		if (cnum >= maxnum) {
			sprintf(tmp,"color number %d is too large",cnum);
			perr(HE_MSG,tmp);
		}
		cbuf[0][cnum] = blue;
		cbuf[1][cnum] = green;
		cbuf[2][cnum] = red;
	}

	fclose(cfd);

	do  {
		done = fscanf(fd,"%d%d%d%d%d\n",&begin,&end,&low,&high,&val);
		if (0<=done && done<3)
			perr(HE_MSG,"color file is incorrect");
		if (val >= maxnum)  {
			fprintf(stderr,
			    "%s: color number %d is too large\n",argv[0],val);
			sprintf(tmp,"\tmax number of colors is %d",maxnum);
			perr(HE_MSG,tmp);
		}
		if (done != EOF) {
			for (i=0; i<3; i++)  {
				bp = &obuf[i][begin];
				color = cbuf[i][val];
				if (high > low) {
					inc_flg = 1;
					inc = (float) (high - low) / MAX_VAL;
					base = low / MAX_VAL;
				} else {
					inc_flg = 0;
					inc = (float) (low - high) / MAX_VAL;
					base = high / MAX_VAL;
				}
				if (begin == end) *bp = color * (inc + base);
				else {
					dvsr = (float)(end-begin)/numtc;
					for (j=0; j<1+end-begin; j++) {
						dtmp = (float)j / dvsr;
						temp = exp(-dtmp);
						if (inc_flg) temp = 1.0 - temp;
				   		*bp++ = color*(inc*temp+base);
					}
				}
			}
		}
	} while (done != EOF);
	
	fclose(fd);

	init_header(&hd,"lutexpmake","",1,"",3,256,PFLUT,1,"");
	update_header(&hd,argc,argv);
	write_header(&hd);

	fwrite(obuf,768,1,stdout);
}
