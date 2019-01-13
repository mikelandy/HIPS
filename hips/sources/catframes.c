/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * catframes.c - concatenate HIPS files
 *
 * Usage:	catframes [-m] file1 file2 ...
 *
 * The region of interest is taken from file1.
 *
 * If -m is specified, the output will be in PFMIXED format and the input
 * files may be of any raster type (including PFMIXED).  Otherwise only
 * simple raster types are accepted, and the maximum common type is used on
 * output.
 *
 * Load:	cc -o catframes catframes.c -lhips
 *
 * HIPS 2 - Michael Landy - 7/6/91
 * PFMIXED - msl - 11/12/92
 * bug fixes/deal with depths - msl - 3/4/94
 */

#include <stdio.h>
#include <hipl_format.h>

int types[] = {PFMSBF,PFLSBF,PFBYTE,PFSBYTE,PFSHORT,PFUSHORT,PFINT,PFUINT,
	PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,PFINTPYR,PFFLOATPYR,PFRGB,
	PFRGBZ,PFZRGB,PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
int typesm[] = {PFMSBF,PFLSBF,PFBYTE,PFSBYTE,PFSHORT,PFUSHORT,PFINT,PFUINT,
	PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,PFINTPYR,PFFLOATPYR,PFMIXED,
	PFRGB,PFRGBZ,PFZRGB,PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
static Flag_Format flagfmt[] = {
	{"m",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,thd,stdinhd,hdp;
	FILE *fp;
	int numfiles,f,i,j,ofmt,method,savesize,nf,stdinfile = -1;
	int *fmts,fmtssize,*cfmts,cfmtssize;
	hsize_t currsize;
	char *savedesc,*savehist;
	Filename *filelist;
	h_boolean foundone,flagm;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&flagm,FFLIST,&numfiles,&filelist);
	if (numfiles < 2)
		perr(HE_MSG,"number of files must be at least two");

/***** first pass - count frames, check compatibility, merge headers *****/

	foundone = FALSE;
	for (i=0;i<numfiles;i++) {
		if (strcmp(filelist[i],"<stdin>")==0)
			stdinfile = i;
		fp = hfopenr(filelist[i]);
		if (i==0) {
			fread_hdr_cpf(fp,&hd,flagm ? typesm : types,
				filelist[0]);
			nf = hd.num_frame;
			if (!flagm && type_is_col3(hd))
				nf *= 3;
			if (i == stdinfile)
				dup_header(&hd,&stdinhd);
			if (flagm) {
				fmtssize = 1000 + nf;
				fmts = (int *) hmalloc(fmtssize*sizeof(int));
				if (hd.pixel_format == PFMIXED) {
					cfmtssize = hd.num_frame;
					getparam(&hd,"formats",PFINT,
						&cfmtssize,&cfmts);
					if (cfmtssize != hd.num_frame)
						perr(HE_FMTSLEN,filelist[i]);
					for (j=0;j<nf;j++)
						fmts[j] = cfmts[j];
				}
				else {
					for (j=0;j<nf;j++)
						fmts[j] = hd.pixel_format;
				}
			}
		}
		else {
			if (flagm)
				fread_hdr_cc(fp,&thd,&hd,
					CM_OROWS|CM_OCOLS|CM_NUMCOLOR|CM_DEPTH,
					filelist[i]);
			else
				fread_hdr_cc(fp,&thd,&hd,
					CM_OROWS|CM_OCOLS|CM_NUMCOLOR3|CM_DEPTH,
					filelist[i]);
			if (i == stdinfile)
				dup_header(&thd,&stdinhd);
			if (flagm) {
				if (nf+thd.num_frame > fmtssize) {
					cfmts = fmts;
					fmtssize = 1000 + nf + thd.num_frame;
					fmts = (int *)
						hmalloc(fmtssize*sizeof(int));
					for (j=0;j<nf;j++)
						fmts[j] = cfmts[j];
					free(cfmts);
				}
				if (thd.pixel_format == PFMIXED) {
					cfmtssize = thd.num_frame;
					getparam(&thd,"formats",PFINT,
						&cfmtssize,&cfmts);
					if (cfmtssize != thd.num_frame)
						perr(HE_FMTSLEN,filelist[i]);
					for (j=0;j<thd.num_frame;j++)
						fmts[nf+j] = cfmts[j];
				}
				else {
					for (j=0;j<thd.num_frame;j++)
						fmts[nf+j] = thd.pixel_format;
				}
			}
			else {
			    if (i==1)
				ofmt = maxformat(hd.pixel_format,
					thd.pixel_format,types,filelist[0],
					filelist[1]);
			    else
				ofmt = maxformat(ofmt,thd.pixel_format,types,
					filelist[0],filelist[i]);
			}
			if (!flagm && type_is_col3(thd))
				nf += 3*(thd.num_frame);
			else
				nf += thd.num_frame;
		}
		if (i != stdinfile)
			fclose(fp);
		if (hips_fullxpar && i>0)
			mergeparam(&hd,&thd);
		if (hips_fulldesc) {
			if ((i==0 ? hd.sizedesc : thd.sizedesc) > 1) {
				savedesc = (i==0) ? hd.seq_desc : thd.seq_desc;
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
			savehist = (i==0) ? hd.seq_history : thd.seq_history;
			if (i==0)
			    history_set(&hd,HEP_SDS,
				"****%s: sequence %d, file: %s****\n",Progname,
				i,filelist[i]);
			else
			    history_append(&hd,HEP_SDS,
				"****%s: sequence %d, file: %s****\n",Progname,
				i,filelist[i]);
			history_indentadd(&hd,savehist);
		}
		if (i>0)
			free_hdrcon(&thd);
	}
	if (!flagm && ptype_is_col3(ofmt))
		nf /= 3;	/* we left the files in an RGB-like format,
					so correct the number of frames */
	if (type_is_col3(&hd) && !ptype_is_col3(ofmt))
		hd.numcolor = 3;
	hd.num_frame = nf;
	if (flagm) {
		setformat(&hd,PFMIXED);
		setparam(&hd,"formats",PFINT,nf,fmts);
	}
	else
		setformat(&hd,ofmt);
	if (hips_fullhist)
		write_headerun(&hd,argc,argv);
	else
		write_headeru(&hd,argc,argv);
	free_hdrcon(&hd);

/***** second pass - get method, read/write images *****/

	f = 0;
	for (i=0;i<numfiles;i++) {
		if (i == stdinfile) {
			fp = stdin;
			dup_header(&stdinhd,&hd);
				alloc_image(&hd);
		}
		else {
			fp = hfopenr(filelist[i]);
			fread_hdr_a(fp,&hd,filelist[i]);
		}
		if (flagm) {
			if (hd.pixel_format == PFMIXED) {
				cfmtssize = hd.num_frame;
				getparam(&hd,"formats",PFINT,
					&cfmtssize,&cfmts);
				if (cfmtssize != hd.num_frame)
					perr(HE_FMTSLEN,filelist[i]);
				setformat(&hd,cfmts[0]);
				alloc_image(&hd);
				currsize = hd.sizeimage;
				for (j=0;j<hd.num_frame;j++) {
					setformat(&hd,cfmts[j]);
					if (hd.sizeimage > currsize) {
						free(hd.image);
						alloc_image(&hd);
						currsize = hd.sizeimage;
					}
					fread_image(fp,&hd,j,filelist[i]);
					write_image(&hd,f);
					f++;
				}
			}
			else {
				for (j=0;j<hd.num_frame;j++) {
					fread_image(fp,&hd,j,filelist[i]);
					write_image(&hd,f);
					f++;
				}
			}
		}
		else {
			method = pset_conversion(&hd,&hdp,ofmt,filelist[i]);
			for (j=0;j<hdp.num_frame;j++) {
				fread_imagec(fp,&hd,&hdp,method,j,filelist[i]);
				write_image(&hdp,f);
				f++;
			}
		}
		free_hdrcon(&hd);
		if (!flagm)
			free_hdrcon(&hdp);
	}
	return(0);
}
