static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */


/*
 * seecontents.c - print the contents of a frame sequence
 *
 * Usage:       seecontents 
 *
 * Load:        cc -o seecontents seecontents.c -lhips
 *
 * Rewritten by Jin Zhengping - 31 August 1991
 */
#include <hipl_format.h>
#include <stdio.h>
static Flag_Format flagfmt[] = {LASTFLAG};
void do_polyline(),do_discont(),do_others();

int main(argc,argv)

int argc;
char **argv;

{
	struct header	hd;
	Filename	filename;
	FILE		*fp;
	int		f,fr;


	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);

	switch(hd.pixel_format)
	{
	case PFPOLYLINE:
		do_polyline(&hd, FALSE);
		break;
	case PFRGPLINE:
		do_polyline(&hd, FALSE);
		do_discont();
		break;
	case PFRGISPLINE:
		do_polyline(&hd, TRUE);
		do_discont();
		break;
	default:
		fr = hd.num_frame;
		for (f=0;f<fr;f++)
		{
			fread_image(fp,&hd,f,filename);
			do_others(&hd);
		}
	}
	return(0);
}

void do_others(hd)
struct header	*hd;
{
	int	i,j ;
	int	rows = hd->rows ;
	int	cols = hd->cols ;
	int	nexi = hd->ocols-cols;

	switch(hd->pixel_format)
	{
		case PFBYTE:
		{
			byte	*pp = hd->firstpix ;
			for(i=0;i<rows;i++)
			{
				for (j=0;j<cols;j++)
				{
					if(!(j%8))
						fprintf(stderr,"\n");
					fprintf(stderr,"%6d ",(int)(*pp++ & 0377));
				}
				pp += nexi;
			}
			fprintf(stderr,"\n");
			break;
		}
		case PFSHORT:
		{
			short	*pp = (short *)hd->firstpix ;
			for(i=0;i<rows;i++)
			{
				for (j=0;j<cols;j++)
				{
					if(!(j%8))
						fprintf(stderr,"\n");
					fprintf(stderr,"%6d ",(int)*pp++);
				}
				pp += nexi;
			}
			fprintf(stderr,"\n");
			break;
		}
		case PFINT:
		{
			int	*pp = (int *)hd->firstpix ;
			for(i=0;i<rows;i++)
			{
				for (j=0;j<cols;j++)
				{
					if(!(j%8))
						fprintf(stderr,"\n");
					fprintf(stderr,"%6d ",(int)*pp++);
				}
				pp += nexi;
			}
			fprintf(stderr,"\n");
			break;
		}
		case PFFLOAT:
		{
			float	*pp = (float *)hd->firstpix ;
			for(i=0;i<rows;i++)
			{
				for (j=0;j<cols;j++)
				{
					if(!(j%5))
						fprintf(stderr,"\n");
					fprintf(stderr,"%e ",*pp++);
				}
				pp += nexi;
			}
			fprintf(stderr,"\n");
			break;
		}
		case PFCOMPLEX:
		{
			float	*pp = (float *)hd->firstpix ;
			for(i=0;i<rows;i++)
			{
				for (j=0;j<cols;j++)
				{
					if(!(j%2))
						fprintf(stderr,"\n");
					fprintf(stderr,"(%e, ",*pp++);
					fprintf(stderr,"%e) ",*pp++);
				}
				pp += nexi;
			}
			fprintf(stderr,"\n");
			break;
		}
		case PFDOUBLE:
		{
			double	*pp = (double *)hd->firstpix ;
			for(i=0;i<rows;i++)
			{
				for (j=0;j<cols;j++)
				{
					if(!(j%5))
						fprintf(stderr,"\n");
					fprintf(stderr,"%e ",*pp++);
				}
				pp += nexi;
			}
			fprintf(stderr,"\n");
			break;
		}
		case PFDBLCOM:
		{
			double	*pp = (double *)hd->firstpix ;
			for(i=0;i<rows;i++)
			{
				for (j=0;j<cols;j++)
				{
					if(!(j%2))
						fprintf(stderr,"\n");
					fprintf(stderr,"(%e, ",*pp++);
					fprintf(stderr,"%e) ",*pp++);
				}
				pp += nexi;
			}
			fprintf(stderr,"\n");
			break;
		}
		default:
			fprintf(stderr,"Unknown format code:\n");
	}
}

void do_discont() {
	int i, howmany;
	long ltmp1, ltmp2;
	double dtmp1, dtmp2;

	ffread(&howmany, sizeof(int), 1, stdin);
	if(howmany == 1)
		fprintf(stderr, "With %d Region:\n\n", howmany);
	else
		fprintf(stderr, "With %d Regions:\n\n", howmany);

	for(i=0 ; i < howmany ; i++) {
		ffread(&ltmp1, sizeof(long), 1, stdin);
		ffread(&ltmp2, sizeof(long), 1, stdin);
		fprintf(stderr, "\tRegion %d:\n", i+1);
		fprintf(stderr, "\t\tstarts at\t(%ld, %ld)\n", ltmp1, ltmp2);
		ffread(&ltmp1, sizeof(long), 1, stdin);
		ffread(&ltmp2, sizeof(long), 1, stdin);
		fprintf(stderr, "\t\tstops at\t(%ld, %ld)\n", ltmp1, ltmp2);
		ffread(&dtmp1, sizeof(double), 1, stdin);
		ffread(&dtmp2, sizeof(double), 1, stdin);
		fprintf(stderr, "\t\tscale X with\t");
		fprintf(stderr, "%0.4lfX + %0.4lf\n", dtmp1, dtmp2);
		ffread(&dtmp1, sizeof(double), 1, stdin);
		ffread(&dtmp2, sizeof(double), 1, stdin);
		fprintf(stderr, "\t\tscale Y with\t");
		fprintf(stderr, "%0.4lfY + %0.4lf\n\n", dtmp1, dtmp2);
	}
}

void do_polyline(hd, extended)
	struct header *hd;
	int extended; {
	long i, j, *b, *buffer;
	long *info = (long *)calloc(6, sizeof(long));

	for(i=0 ; i < hd->num_frame ; i++) {
		ffread((char *)info, sizeof(long), 2, stdin);
		fprintf(stderr, "frame %ld: %ld points ", i, *info);
		fprintf(stderr, "(colour %ld)\n", *(info+1));
		if(extended) {
			/* read in, interface number, interface type,
			 * segment number and whether the interface
			 * is a fault
			 */
			ffread((char *)(info+2), sizeof(long), 4, stdin);
			fprintf(stderr, "[interface number (%ld) interface type (%ld) segment number (%ld)] ", *(info+2), *(info+3), *(info+4));
			if(*(info+5))
				fprintf(stderr, "(Fault)");
			fprintf(stderr, "\n");
		}
		else
			fprintf(stderr, "\n");
		buffer = (long *)calloc((*info)*2, sizeof(long));
		ffread((char *)buffer, sizeof(long), (*info)*2, stdin);
		for(j=0, b=buffer ; j < *info ; j++, b += 2)
			fprintf(stderr, "\t(%ld, %ld)\n", *b, *(b+1));
		free(buffer);
		fprintf(stderr, "\n");
	}
}	
