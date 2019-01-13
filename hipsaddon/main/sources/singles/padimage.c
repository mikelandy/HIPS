static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/* padimage -- insert each foreground image in the input sequence 1 into
 *             each background image in the input sequence 2
 *		 
 * usage: padimage [-s startrow startcol] sequence1 [<] sequence2
 *
 * where startrow and startcol specify the position of the background image
 * for the top left corner of the foreground image. It defaults to (0,0).
 * If the foreground image lies across the boundary of the background image,
 * the part that lies outside the background image is clipped.
 *
 * to load: cc -o padimage padimage.c -lhips
 *
 * Mowforth P.H. and Jin Z.P. 9/10/85 
 * Rewritten by Jin Zhengping - 31 August 1991
 *
 */

#include <hipl_format.h>
#include <stdio.h>
#define MSG1	"%s: foreground image is totally outside background image\n"
#define MSG2	"%s: part of foreground image is outside background image and will be clipped.\n"
int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] =
{
	{"s",
		{LASTFLAG},
		2,
		{{PTINT,"0","startrow"},{PTINT,"0","startcol"},LASTPARAMETER}},
	LASTFLAG
};

int main(argc,argv)

int	argc;
char	**argv;

{	
	struct          header hd1,hdp1,hd2,hdp2;
	int             method,f,fr;
	Filename        filename1,filename2;
	FILE            *fp1,*fp2;
	int		srow,scol ;

	int		sr,sc,i,j;
	byte		*bp,*sp;


	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&srow,&scol,FFTWO,&filename1,&filename2);
	fp1=hfopenr(filename1);
	fp2=hfopenr(filename2);
	fread_hdr_a(fp2,&hd2,filename2);
	fread_hdr_cca(fp1,&hd1,&hd2,CM_NUMCOLOR|CM_FRAMESC,filename1);
	method=fset_conversion(&hd1,&hdp1,types,filename1);
	method=fset_conversion(&hd2,&hdp2,types,filename2);
	write_headeru2(&hd2,&hdp2,argc,argv,hips_convback);

	fr = hdp2.num_frame;
	if((hd2.orows <= srow) ||
	   (hd2.ocols <= scol) ||
	   ((srow + hd1.orows) <= 0) ||
	   ((scol + hd1.ocols) <= 0))
	{
		fprintf(stderr,MSG1,Progname) ;
		for (f=0;f<fr;f++)
		{
			fread_image(fp2,&hd2,f,filename2);
			write_image(&hd2,f);
		}
	} else
	{
		int	nex=hdp2.ocols-hd1.ocols;
		if((0 > srow) ||
		   (0 > scol) ||
		   ((srow + hd1.orows) > hd2.orows) ||
		   ((scol + hd1.ocols) > hd2.ocols))
			fprintf(stderr,MSG2,Progname) ;
		for (f=0;f<fr;f++)
		{
			fread_imagec(fp2,&hd2,&hdp2,method,f,filename2);
			fread_imagec(fp1,&hd1,&hdp1,method,f,filename1);
			sp=hdp1.image; bp=hdp2.image+srow*hdp2.ocols+scol;
			for(sr=srow,i=0; i<hd1.orows; i++,sr++)
			{
				for(sc=scol,j=0; j<hd1.ocols; j++,bp++,sp++,sc++)
					if(sr>=0 && sr<hd2.orows && sc>=0 && sc<hd2.ocols)
						*bp = *sp;
				bp+=nex;
			}
			write_imagec(&hd2,&hdp2,method,hips_convback,f);
		}
	}
	return(0);
}
