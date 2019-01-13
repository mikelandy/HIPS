/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * dump_plot3d.c - dump out a plot3d file in a readable form
 *
 * usage:	dump_plot3d < plot3d-file
 *
 * to compile:	cc -o dump_plot3d dump_plot3d.c -lhips
 *
 * Michael Landy - 7/20/85
 * HIPS 2 - msl - 8/1/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PLOT3D,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int vflag,mflag,nbytes,f;
	char b,msg[100];
	double sv[3],rm[3][3];
	float args[10];
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_cpf(fp,&hd,types,filename);
	for (f=0;f<hd.num_frame;f++) {
		if (fread(&b,1,1,fp) != 1)
			perr(HE_READ);
		printf("Flags: ");
		mflag=b&01;
		vflag=b>>1;
		if (b==0)
			printf("None");
		if (mflag)
			printf("Rotation ");
		if (vflag)
			printf("Shift ");
		printf("\n");
		if (vflag) {
			if (fread(&sv[0],sizeof(double),3,fp) != 3)
				perr(HE_MSG,"error in reading shift_vector");
			printf("shift = (%.2f,%.2f,%.2f)\n",sv[0],sv[1],sv[2]);
		}
		if (mflag) {
			if (fread(&(rm[0][0]),sizeof(double),9,fp) != 9)
				perr(HE_MSG,"error in reading rotation_matrix");
			printf("rot =	(%.2f,%.2f,%.2f)\n",rm[0][0],rm[0][1],
				rm[0][2]);
			printf("	(%.2f,%.2f,%.2f)\n",rm[1][0],rm[1][1],
				rm[1][2]);
			printf("	(%.2f,%.2f,%.2f)\n",rm[2][0],rm[2][1],
				rm[2][2]);
		}
		if (fread(&nbytes,sizeof(int),1,fp) != 1)
			perr(HE_MSG,"error in reading size of frame");
		printf("nbytes = %d\n",nbytes);
		while (fread(&b,1,1,fp) == 1) {
			if (b == 'p') {
			    if (fread(&args[0],sizeof(float),4,fp) != 4)
					perr(HE_READ);
			    printf("'p' - point - %.2f - (%.2f,%.2f,%.2f)\n",
				args[0],args[1],args[2],args[3]);
			}
			else if (b == 'v') {
				if (fread(&args[0],sizeof(float),7,fp) != 7)
					perr(HE_READ);
				printf("'v' - vector - %.2f - (%.2f,%.2f,%.2f)",
					args[0],args[1],args[2],args[3]);
				printf(" - (%.2f,%.2f,%.2f)\n",args[4],args[5],
					args[6]);
			}
			else if (b == 'n') {
				if (fread(&args[0],sizeof(float),3,fp) != 3)
					perr(HE_READ);
				printf("'n' - end-point - (%.2f,%.2f,%.2f)\n",
					args[0],args[1],args[2]);
			}
			else if (b == 'e') {
				printf("'e' - end\n");
				break;
			}
			else {
				sprintf(msg,"unknown code: %c, run aborted",b);
				perr(HE_MSG,msg);
			}
		}
	}
	return(0);
}
