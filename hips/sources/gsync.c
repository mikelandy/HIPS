/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * gsync.c - synchronize n plot files
 *
 * The size(in frames) of the output file is the size of the largest input file
 *
 * usage: gsync [file1 ... filen] > ofile
 *
 * To load:	cc -o gsync gsync.c -lhips
 *
 * Yoav Cohen 11/18/82
 * HIPS 2 - msl - 8/1/91
 */

#include <hipl_format.h>
#include <stdio.h>

char b[FBUFLIMIT];

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PLOT3D,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,thd,*inhd;
	int *nf,ib,nb,nfiles,i,j,k,file,frame,grnf,flags;
	double rot_m[3][3],sh_v[3];
	FILE **fp,*tfp;
	char *savedesc,*savehist;
	Filename *filelist;
	h_boolean foundone=FALSE;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFLIST,&nfiles,&filelist);
	fp = (FILE **) memalloc(nfiles,sizeof(FILE *));
	nf = (int *) memalloc(nfiles,sizeof(int));
	grnf=0;
	for (i=0;i<nfiles;i++) {
		fp[i] = hfopenr(filelist[i]);
		if (i==0)
			inhd = &hd;
		else
			inhd = &thd;
		fread_hdr_cpf(fp[i],inhd,types,filelist[i]);
		nf[i] = inhd->num_frame;
		if (nf[i]<=0)
			perr(HE_MSG,"number of frames must be positive");
		if (nf[i]>grnf)
			grnf=nf[i];
		if (hips_fullxpar && i>0)
			mergeparam(&hd,&thd);
		if (hips_fulldesc) {
			if (inhd->sizedesc > 1) {
				savedesc = inhd->seq_desc;
				if (foundone)
				  desc_append2(&hd,HEP_SDS,
				    "****%s: sequence %d, file: %s****\n",
					Progname,i,filelist[i]);
				else
				  desc_set2(&hd,HEP_SDS,
				    "****%s: sequence %d, file: %s****\n",
					Progname,i,filelist[i]);
				foundone = TRUE;
				desc_indentadd(&hd,savedesc);
			}
		}
		if (hips_fullhist) {
			savehist = inhd->seq_history;
			if (i == 0)
			    history_set(&hd,HEP_SDS,
				"****%s: sequence %d, file: %s****\n",Progname,
				i,filelist[i]);
			else
			    history_append(&hd,HEP_SDS,
				"****%s: sequence %d, file: %s****\n",Progname,
				i,filelist[i]);
			history_indentadd(&hd,savehist);
		}
	}
	hd.num_frame=grnf;
	if (hips_fullhist)
		write_headerun(&hd,argc,argv);
	else
		write_headeru(&hd,argc,argv);

/* sort files, shortest one first */

	for (i=0;i<nfiles-1;i++)
	    for (j=i+1;j<nfiles;j++) {
		if (nf[i]<=nf[j])
			continue;
		k=nf[i]; nf[i]=nf[j]; nf[j]=k;
		tfp=fp[i]; fp[i]=fp[j]; fp[j]=tfp;
	    }

/* read files frame by frame */

	for (frame=0;frame<grnf;frame++) {
		ib = 0;
		for (file=0;file<nfiles;file++) {
			if (frame>=nf[file])
				continue;
			nb = read_frame(fp[file],b+ib,
				FBUFLIMIT-ib,&flags,sh_v,rot_m,frame,
				filelist[file]);
			trans_frame(b+ib,nb,sh_v,rot_m,&flags);
			ib += nb;
		}
		write_frame(stdout,b,ib,sh_v,rot_m,frame);
	}
	return(0);
}
