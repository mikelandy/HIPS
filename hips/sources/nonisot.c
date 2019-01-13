/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * nonisot.c - nonisotropic convolution
 *
 * usage: nonisot chooser mask0 ... maskn imagefile > blurredfile
 *
 * Nonisot performs nonisotropic convolution.  Each mask file contains a
 * single frame convolution mask.  For each frame nonisot does the following.
 * After zeroing the output image, for every row and column, the value of the
 * chooser image is used as an index into the list of masks.  The appropriate
 * mask is scaled by the input and added into the output image.  The
 * chooser values are used modulo the number of masks (i.e. a chooser value
 * of n+1 uses mask0).  The mask is added to the output image starting from
 * its first pixels (thus, it is added to the right and downward from the
 * associated input pixel for a upper-left image origin, and to the right and
 * upward, otherwise).
 *
 * Load:	cc -o nonisot nonisot.c -lhipsh -lhips
 *
 * Michael Landy - 7/9/88
 * HIPS 2 - Michael Landy - 8/13/91
 */

#include <stdio.h>
#include <hipl_format.h>

int types[] = {PFINT,LASTTYPE};
static Flag_Format flagfmt[] = {LASTFLAG};
char fileusage[] = "chooser mask0 mask1 ... maskn inputfile";
void print_usage();

int main(argc,argv)

int argc;
char **argv;

{
	struct header **hdm,hdt,hdc,hd,hdp,hdo;
	FILE *fp,*fpt;
	int method,methodt,numfiles,nmasks,file,f,nf,savesize;
	char *savedesc,*savehist,msg[100];
	h_boolean foundone=FALSE,copyimage;
	struct hips_roi roi;
	Filename *filelist;
	Pixelval val;

	Progname = strsave(*argv);
	parseargsu(argc,argv,flagfmt,FFLIST,&numfiles,&filelist,fileusage);
	nmasks = numfiles - 2;
	if (nmasks < 1)
		print_usage();
	hdm = (struct header **) memalloc(nmasks,sizeof(struct header *));
	for (file=0;file<nmasks;file++)
		hdm[file] = (struct header *) memalloc(1,sizeof(struct header));

	fp = hfopenr(filelist[numfiles-1]);	/* input image */
	fread_hdr_a(fp,&hd,filelist[numfiles-1]);
	method = fset_conversion(&hd,&hdp,types,filelist[numfiles-1]);
	fpt = hfopenr(filelist[0]);		/* chooser image */
	fread_hdr_cca(fpt,&hdt,&hd,CM_ROWS|CM_COLS,filelist[0]);
	methodt = fset_conversion(&hdt,&hdc,types,filelist[0]);
	if (hdc.num_frame != 1)
		perr(HE_MSG,"chooser image must be a single frame");
	fread_imagec(fpt,&hdt,&hdc,methodt,0,filelist[0]);
	fclose(fpt);
	if (methodt != METH_IDENT)
		free(hdt.image);

	if (hips_fullxpar)
		mergeparam(&hdp,&hdc);
	if (hips_fulldesc) {
		savedesc = hdp.seq_desc;
		savesize = hdp.sizedesc;
		if (hdc.sizedesc > 1) {
			desc_set2(&hdp,HEP_SS,
			    "****%s: chooser image, file: %s****\n",
				Progname,filelist[0]);
			foundone = TRUE;
			desc_indentadd(&hdp,hdc.seq_desc);
		}
	}
	if (hips_fullhist) {
		savehist = hdp.seq_history;
		history_set(&hdp,HEP_SS,
			"****%s: chooser image, file: %s****\n",Progname,
			filelist[0]);
		history_indentadd(&hdp,hdc.seq_history);
	}
	for (file=0;file<nmasks;file++) {
		fpt = hfopenr(filelist[file+1]);	/* mask image */
		fread_hdr_a(fpt,&hdt,filelist[file+1]);
		methodt = fset_conversion(&hdt,hdm[file],types,
			filelist[file+1]);
		if (hdm[file]->num_frame != 1) {
			sprintf(msg,
				"mask image must be a single frame, file: %s",
				filelist[file+1]);
			perr(HE_MSG,msg);
		}
		fread_imagec(fpt,&hdt,hdm[file],methodt,0,filelist[file+1]);
		fclose(fpt);
		if (methodt != METH_IDENT)
			free(hdt.image);
		if (hips_fullxpar)
			mergeparam(&hdp,hdm[file]);
		if (hips_fulldesc) {
			if (hdm[file]->sizedesc > 1) {
				if (foundone)
				  desc_append2(&hdp,HEP_SDS,
				    "****%s: mask %d, file: %s****\n",
					Progname,file,filelist[file+1]);
				else
				  desc_set2(&hdp,HEP_SDS,
				    "****%s: mask %d, file: %s****\n",
					Progname,file,filelist[file+1]);
				foundone = TRUE;
				desc_indentadd(&hdp,hdm[file]->seq_desc);
			}
		}
		if (hips_fullhist) {
			history_append(&hdp,HEP_SDS,
				"****%s: mask %d, file: %s****\n",Progname,
				file,filelist[file+1]);
			history_indentadd(&hdp,hdm[file]->seq_history);
		}
	}
	if (hips_fulldesc) {
		if (savesize > 1) {
			if (foundone)
			  desc_append2(&hdp,HEP_SS,
			    "****%s: input sequence, file: %s****\n",
				Progname,filelist[numfiles-1]);
			else
			  desc_set2(&hdp,HEP_SS,
			    "****%s: input sequence, file: %s****\n",
				Progname,filelist[numfiles-1]);
			desc_indentadd(&hdp,savedesc);
		}
	}
	if (hips_fullhist) {
		history_append(&hdp,HEP_SS,
			"****%s: input sequence, file: %s****\n",Progname,
			filelist[numfiles-1]);
		history_indentadd(&hdp,savehist);
	}
	hd.seq_history = hdp.seq_history;
	hd.sizehist = hdp.sizehist;
	desc_set(&hd,hdp.seq_desc);
	dup_headern(&hdp,&hdo);
	alloc_image(&hdo);
	if (hips_convback) {
		if (hips_fullhist)
			write_headerun(&hd,argc,argv);
		else
			write_headeru(&hd,argc,argv);
	}
	else {
		if (hips_fullhist)
			write_headerun(&hdp,argc,argv);
		else
			write_headeru(&hdp,argc,argv);
	}
	copyimage = (hd.rows != hd.orows) || (hd.cols != hd.ocols);
	getroi(&hd,&roi);
	nf = hdp.num_frame;
	val.v_int = 0;

	for (f=0;f<nf;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filelist[numfiles-1]);
		if (copyimage) {
			clearroi(&hdp);
			clearroi(&hdo);
			h_copy(&hdp,&hdo);
			setroi2(&hdp,&roi);
			setroi2(&hdo,&roi);
		}
		h_setimage(&hdo,&val);
		h_nonisot(&hdp,&hdc,nmasks,hdm,&hdo);
		write_imagec(&hd,&hdo,method,hips_convback,f);
	}
	return(0);
}
