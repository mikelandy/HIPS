/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * addparam.c - add parameters to a HIPS image header
 *
 * usage:	addparam [-F paramfile]
 *			[-b name bytevalue]
 *			[-s name shortvalue]
 *			[-i name intvalue]
 *			[-f name floatvalue]
 *			[-S name stringvalue]
 *			[-SF name stringfile] <iseq >oseq
 *
 * Addparam is used to add parameters to a HIPS image header.  A few
 * parameters may be specified in the command line, or any number of
 * parameters may be specified in a file.  Single-valued parameters may
 * be specified in the command line with -b, -s, -i or -f for a single
 * byte, short, integer or float parameter.  -S may be used to specify an Ascii
 * array (a string of length 1 or more).  -SF also defines an Ascii
 * array, setting the parameter equal to the entire contents of the named
 * file.  Note that each of these parameters may be specified at most once.
 * Thus, to specify more than one of a given parameter type, either pipe the
 * results into another addparam, or use the -f option.
 *
 * The -f option allows the user to specify any number of parameters in a
 * file.  The format of the file is as a series of descriptions of single
 * parameters.  Each parameter description takes the following form:
 *
 *		name format count values
 *
 * Name is the name of the parameter.  Format is the the parameter type.  This
 * is a string which may be b, s, i or f for byte, short, integer or
 * floating point.  Count is the count of values.  The scanf format %d is used
 * for byte, short and integer types, %f is used for floating point types.
 * For Ascii parameters there are two forms.  First, there is:
 *
 *		name string "value"
 *
 * which sets parameter name to be a string array with the indicated value.
 * The quotation marks are stripped off and the string array is terminated
 * with a null.  A backslash character may be used to quote a quotation
 * character if desired.  Finally:
 *
 *		name stringfile filename
 *
 * sets the value of parameter name to be the entire contents of the named
 * file.
 *
 * to load:	cc -o addparam addparam.c -lhips
 *
 * Mike Landy - 8/14/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"b",{LASTFLAG},2,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","name"},
		{PTINT,"","bytevalue"},LASTPARAMETER}},
	{"s",{LASTFLAG},2,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","name"},
		{PTINT,"","shortvalue"},LASTPARAMETER}},
	{"i",{LASTFLAG},2,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","name"},
		{PTINT,"","intvalue"},LASTPARAMETER}},
	{"f",{LASTFLAG},2,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","name"},
		{PTDOUBLE,"","floatvalue"},LASTPARAMETER}},
	{"S",{LASTFLAG},2,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","name"},
		{PTSTRING,"","stringvalue"},LASTPARAMETER}},
	{"SF",{LASTFLAG},2,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","name"},
		{PTFILENAME,"","stringfile"},LASTPARAMETER}},
	{"F",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTFILENAME,"","paramfile"},LASTPARAMETER}},
	LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	Filename filename,stringfile,paramfile;
	FILE *fp,*fpp;
	h_boolean bflag,sflag,iflag,fflag,Sflag,SFflag,Fflag;
	int bval,sval,ival,*ivals,i,j,count,c,*fmts,fmtssize;
	hsize_t currsize;
	byte *bvals;
	short *svals;
	float *fvals;
	char *bname,*sname,*iname,*fname,*Sname,*Sval,*SFname,stringfile2[100];
	char *getfile(),*getstring(),name[50],fmt[50],msg[100];
	double fval;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&bflag,&bname,&bval,&sflag,&sname,&sval,
		&iflag,&iname,&ival,&fflag,&fname,&fval,&Sflag,&Sname,&Sval,
		&SFflag,&SFname,&stringfile,&Fflag,&paramfile,
		FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	if (bflag)
		setparam(&hd,bname,PFBYTE,1,(byte) bval);
	if (sflag)
		setparam(&hd,sname,PFSHORT,1,(short) sval);
	if (iflag)
		setparam(&hd,iname,PFINT,1,ival);
	if (fflag)
		setparam(&hd,fname,PFFLOAT,1,(float) fval);
	if (Sflag)
		setparam(&hd,Sname,PFASCII,strlen(Sval)+1,strsave(Sval));
	if (SFflag) {
		Sval = getfile(stringfile);
		setparam(&hd,SFname,PFASCII,strlen(Sval)+1,Sval);
	}
	if (Fflag) {
		fpp = hfopenr(paramfile);
		while ((i = fscanf(fpp,"%50s %50s",name,fmt)) != EOF) {
			if (i < 2 || strlen(name) > 49 || strlen(fmt) > 49) {
				sprintf(msg,
					"parameter file error: %s %s",
					name,fmt);
				perr(HE_MSG,msg);
			}
			if (strcmp(fmt,"b") == 0) {
				if (fscanf(fpp,"%d",&count) != 1)
					perr(HE_READFILE,paramfile);
				if (count < 1)
					perr(HE_MSG,"bad count value");
				bvals = (byte *) memalloc(count,sizeof(byte));
				for (i=0;i<count;i++) {
					if (fscanf(fpp,"%d",&j) != 1)
						perr(HE_READFILE,paramfile);
					bvals[i] = j;
				}
				if (count == 1)
					setparam(&hd,name,PFBYTE,1,*bvals);
				else
					setparam(&hd,name,PFBYTE,count,bvals);
			}
			else if (strcmp(fmt,"s") == 0) {
				if (fscanf(fpp,"%d",&count) != 1)
					perr(HE_READFILE,paramfile);
				if (count < 1)
					perr(HE_MSG,"bad count value");
				svals = (short *) memalloc(count,sizeof(short));
				for (i=0;i<count;i++) {
					if (fscanf(fpp,"%d",&j) != 1)
						perr(HE_READFILE,paramfile);
					svals[i] = j;
				}
				if (count == 1)
					setparam(&hd,name,PFSHORT,1,*svals);
				else
					setparam(&hd,name,PFSHORT,count,svals);
			}
			else if (strcmp(fmt,"i") == 0) {
				if (fscanf(fpp,"%d",&count) != 1)
					perr(HE_READFILE,paramfile);
				if (count < 1)
					perr(HE_MSG,"bad count value");
				ivals = (int *) memalloc(count,sizeof(int));
				for (i=0;i<count;i++) {
					if (fscanf(fpp,"%d",ivals+i) != 1)
						perr(HE_READFILE,paramfile);
				}
				if (count == 1)
					setparam(&hd,name,PFINT,1,*ivals);
				else
					setparam(&hd,name,PFINT,count,ivals);
			}
			else if (strcmp(fmt,"f") == 0) {
				if (fscanf(fpp,"%d",&count) != 1)
					perr(HE_READFILE,paramfile);
				if (count < 1)
					perr(HE_MSG,"bad count value");
				fvals = (float *) memalloc(count,sizeof(float));
				for (i=0;i<count;i++) {
					if (fscanf(fpp,"%f",fvals+i) != 1)
						perr(HE_READFILE,paramfile);
				}
				if (count == 1)
					setparam(&hd,name,PFFLOAT,1,*fvals);
				else
					setparam(&hd,name,PFFLOAT,count,fvals);
			}
			else if (strcmp(fmt,"string") == 0) {
				Sval = getstring(fpp);
				setparam(&hd,name,PFASCII,strlen(Sval)+1,Sval);
			}
			else if (strcmp(fmt,"stringfile") == 0) {
				if (fscanf(fpp,"%s",stringfile2) != 1)
					perr(HE_READFILE,paramfile);
				Sval = getfile(stringfile2);
				setparam(&hd,name,PFASCII,strlen(Sval)+1,Sval);
			}
			else {
				sprintf(msg,
					"parameter file unknown format: %s %s",
					name,fmt);
				perr(HE_MSG,msg);
			}
		}
	}
	write_headeru(&hd,argc,argv);
	if (hd.sizeimage) {
		for (i=0;i<hd.num_frame;i++) {
			fread_image(fp,&hd,i,filename);
			write_image(&hd,i);
		}
	}
	else if (hd.pixel_format == PFMIXED) {
		fmtssize = hd.num_frame;
		getparam(&hd,"formats",PFINT,&fmtssize,&fmts);
		if (fmtssize != hd.num_frame)
			perr(HE_FMTSLEN,filename);
		setformat(&hd,fmts[0]);
		alloc_image(&hd);
		currsize = hd.sizeimage;
		for (i=0;i<hd.num_frame;i++) {
			setformat(&hd,fmts[i]);
			if (hd.sizeimage > currsize) {
				free(hd.image);
				alloc_image(&hd);
				currsize = hd.sizeimage;
			}
			fread_image(fp,&hd,i,filename);
			write_image(&hd,i);
		}
	}
	else {
		while ((c=getc(fp)) != EOF) putchar(c);
	}
	return(0);
}

char *getfile(filename)

char *filename;

{
	FILE *fp;
	char *s,*tmp;
	int currlen,len,c,i;

	fp = ffopen(filename,"r");
	s = (char *) memalloc(512,sizeof(char));
	len = 512;
	currlen = 0;
	while ((c = getc(fp)) != EOF) {
		s[currlen++] = c;
		if (currlen >= len) {
			tmp = (char *) memalloc(512+len,sizeof(char));
			for (i=0;i<len;i++)
				tmp[i] = s[i];
			free(s);
			s = tmp;
		}
	}
	s[currlen] = '\0';
	tmp = strsave(s);
	if (strlen(tmp) < 2)
		perr(HE_MSG,"file contents too short");
	free(s);
	return(tmp);
}

char *getstring(fp)

FILE *fp;

{
	char *s,*tmp;
	int currlen,len,c,i;

	while (TRUE) {
		if ((c = getc(fp)) == EOF)
			perr(HE_MSG,"early end of file getting string");
		if (c != ' ' && c != '\t')
			break;
	}
	if (c != '"')
		perr(HE_MSG,"can't fine '\"' to begin string");
	s = (char *) memalloc(512,sizeof(char));
	len = 512;
	currlen = 0;
	while (TRUE) {
		if ((c = getc(fp)) == EOF)
			perr(HE_MSG,"early end of file getting string");
		if (c == '\\') {
			if ((c = getc(fp)) == EOF)
				perr(HE_MSG,"early end of file getting string");
		}
		if (c == '"')
			break;
		s[currlen++] = c;
		if (currlen >= len) {
			tmp = (char *) memalloc(512+len,sizeof(char));
			for (i=0;i<len;i++)
				tmp[i] = s[i];
			free(s);
			s = tmp;
		}
	}
	s[currlen] = '\0';
	tmp = strsave(s);
	if (strlen(tmp) < 1)
		perr(HE_MSG,"null string parameter");
	free(s);
	return(tmp);
}
