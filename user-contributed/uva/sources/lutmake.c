/*	PROGRAM
 *		lutmake
 *
 *	PURPOSE
 *		to define a LUT based on a color file with the option of 
 *		defining both color and intensity.
 *
 *	SYNOPSIS
 *              lutmake <filename> > lut data file
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

int main(argc,argv)
	int argc;
	char *argv[];
{
	FILE *fd,*cfd;
	struct header hd;
	float inc, base;
	int  i, j, cnum, maxnum, done, blue, green, red;
	int  begin, end, low, high, val;
	short cbuf[3][20], temp;
	unsigned char obuf[3][256], *bp;
	char name[80],tmp[100];

	Progname = strsave(*argv);
	if (argc != 2)
		perr(HE_MSG,"usage: lutmake <infile>");

	if ((fd = fopen (argv[1], "r")) == NULL) {
		sprintf(tmp,"Wrcolor: cannot open color file %s",argv[1]);
		perr(HE_MSG,tmp);
	}

	/* read past the first line as it is a comment line */
	if ((done = fscanf(fd,"%[^\n]\n",name)) == 0) {
		sprintf(tmp,"cannot read color file %s",argv[1]);
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
			printf("lutmake: color number %d is too large\n",val);
			sprintf(tmp,"\tmax number of colors is %d\n",maxnum);
			perr(HE_MSG,tmp);
		}
		if (done != EOF) {
			for (i=0; i<3; i++)  {
				bp = &obuf[i][begin];
				temp = cbuf[i][val];
				if (begin == end) inc = 0.0;
				else inc = (high-low) / (256. * (end-begin));
				base = low / 256.;
				for (j=0; j<1+end-begin; j++) 
				   *bp++ = (short)((float)temp*(inc*j+base));
			}
		}
	} while (done != EOF);
	
	fclose(fd);

	init_header(&hd,"lutmake","",1,"",3,256,PFLUT,1,"");
	update_header(&hd,argc,argv);
	write_header(&hd);

	fwrite(obuf,768,1,stdout);
}
