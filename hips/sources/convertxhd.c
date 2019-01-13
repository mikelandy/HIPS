/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * convertxhd.c - convert a HIPS-1 image with extended header to HIPS-2 format
 *
 * usage:	convertxhd [-f descriptionfile] <iseq >oseq
 *
 * Convertxhd is used to convert a HIPS-1 image which includes extended header
 * information (using the HIPS-1 XHEADER package) to HIPS-2 format, storing
 * the extended header information as HIPS-2 extended parameters.  Two
 * header entries are handled automatically:  colormap and cmap.  Colormap is
 * assumed to be an integer, and is stored as parameter `maplength'.  Cmap is
 * assumed to be a color map with the length specified by colormap, and
 * encoded as a string of hex digits (two hex digits per entry).  If the user
 * wants to convert other extended header entries, they must be specified in
 * a description file.  The description file consists of a series of lines,
 * one for each parameter.  Each line contains the following information:
 *
 *		oldname format count newname
 *
 * Oldname is the name of the parameter as stored in the input header.  A
 * warning will be printed if the entry is not found.  Newname is the name
 * which will be used in the output header parameters section.  Format is the
 * the parameter type.  This is a single character which may be b, c, s, i or
 * f for byte, Ascii, short, integer or floating point.  Count is the count
 * of values.  If count is -1, then the count is determined by reading as many
 * values as possible from the next header line (and for Ascii, the string is
 * null-terminated).  The scanf format %d is used
 * for byte, short and integer types, %f is used for floating point types, and
 * the raw uninterpreted Ascii data are used for the Ascii type.
 * Each extended header entry may be converted only once, and a warning
 * message is printed if any entries remain unconverted.
 *
 * to load:	cc -o convertxhd convertxhd.c -lhips
 *
 * Mike Landy - 8/14/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"f",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTFILENAME,"","descriptionfile"},LASTPARAMETER}},
	LASTFLAG};

#define	NADA	'\377'
#define NULLP	((char *) 0)

char *desc,*findpar(),*stepval();
int size,lenpar();
void clearval(),perrn();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int c,i,j,k,count;
	Filename filename,dfile;
	FILE *fp,*fpd;
	char *findpar(),*val,*pval,name[50],oldname[50],fmt[50],msg[200];
	char *stepval();
	h_boolean fflag;
	byte *b,*pb;
	short *sval;
	int *ival;
	float *fval;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&fflag,&dfile,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	desc = hd.seq_desc;
	size = hd.sizedesc;
	if ((val = findpar("colormap")) != NULLP) {
		if (sscanf(val,"%d",&i) != 1)
			perrn("colormap");
		setparam(&hd,"maplength",PFINT,1,i);
		clearval(val);
		if ((val = findpar("cmap")) != NULLP) {
			pval = val;
			pb = b = (byte *) memalloc(i,sizeof(byte));
			for (j=0;j<i;j++) {
				if (sscanf(pval,"%02x",&k) != 1)
					perrn("cmap");
				*pb++ = k;
				pval += 2;
			}
			setparam(&hd,"cmap",PFBYTE,i,b);
			clearval(val);
		}
	}
	if (fflag) {
		fpd = hfopenr(dfile);
		while ((i = fscanf(fpd,"%50s %50s %d %50s",oldname,fmt,&count,
		    name)) != EOF) {
			if (i < 4 || strlen(oldname) > 49 || strlen(fmt) > 49
			    || strlen(name) > 49 || count == 0) {
				sprintf(msg,
					"description file error: %s %s %d %s",
					oldname,fmt,count,name);
				perr(HE_MSG,msg);
			}
			if ((val = findpar(oldname)) == NULLP) {
				sprintf(msg,"warning: %s not found",oldname);
				perr(HE_IMSG,msg);
				continue;
			}
			if (count > 0) {
				if (strcmp(fmt,"c") == 0) {
					if (strlen(val)+1 < count)
						perrn(oldname);
					b = (byte *) strsave(val);
					if (count == 1)
						setparam(&hd,name,PFASCII,1,*b);
					else
						setparam(&hd,name,PFASCII,
							count,b);
				}
				else if (strcmp(fmt,"b") == 0) {
					pval = val;
					b = (byte *) memalloc(count,
						sizeof(byte));
					for (i=0;i<count;i++) {
						if (sscanf(pval,"%d",&j) != 1)
							perrn(oldname);
						b[i] = j;
						pval = stepval(pval);
					}
					if (count == 1)
						setparam(&hd,name,PFBYTE,1,*b);
					else
						setparam(&hd,name,PFBYTE,
							count,b);
				}
				else if (strcmp(fmt,"s") == 0) {
					pval = val;
					sval = (short *) memalloc(count,
						sizeof(short));
					for (i=0;i<count;i++) {
						if (sscanf(pval,"%d",&j) != 1)
							perrn(oldname);
						sval[i] = j;
						pval = stepval(pval);
					}
					if (count == 1)
						setparam(&hd,name,PFSHORT,1,
							*sval);
					else
						setparam(&hd,name,PFSHORT,
							count,sval);
				}
				else if (strcmp(fmt,"i") == 0) {
					pval = val;
					ival = (int *) memalloc(count,
						sizeof(int));
					for (i=0;i<count;i++) {
						if (sscanf(pval,"%d",ival+i)
						    != 1)
							perrn(oldname);
						pval = stepval(pval);
					}
					if (count == 1)
						setparam(&hd,name,PFINT,1,
							*ival);
					else
						setparam(&hd,name,PFINT,
							count,ival);
				}
				else if (strcmp(fmt,"f") == 0) {
					pval = val;
					fval = (float *) memalloc(count,
						sizeof(float));
					for (i=0;i<count;i++) {
						if (sscanf(pval,"%f",fval+i)
						    != 1)
							perrn(oldname);
						pval = stepval(pval);
					}
					if (count == 1)
						setparam(&hd,name,PFFLOAT,1,
							*fval);
					else
						setparam(&hd,name,PFFLOAT,
							count,fval);
				}
				else {
					sprintf(msg,"illegal format %s",fmt);
					perr(HE_MSG,msg);
				}
			}
			else {
				if (strcmp(fmt,"c") == 0) {
					b = (byte *) strsave(val);
					setparam(&hd,name,PFASCII,
						strlen((const char * )b)+1,b);
				}
				else if (strcmp(fmt,"b") == 0) {
					pval = val;
					b = (byte *) memalloc(strlen(val)/2,
						sizeof(byte));
					count = 0;
					while (TRUE) {
						if (sscanf(pval,"%d",&j) != 1)
							break;
						b[count++] = j;
						pval = stepval(pval);
					}
					if (count == 0)
						perrn(oldname);
					else if (count == 1)
						setparam(&hd,name,PFBYTE,1,*b);
					else
						setparam(&hd,name,PFBYTE,
							count,b);
				}
				else if (strcmp(fmt,"s") == 0) {
					pval = val;
					sval = (short *) memalloc(strlen(val)/2,
						sizeof(short));
					count = 0;
					while (TRUE) {
						if (sscanf(pval,"%d",&j) != 1)
							break;
						sval[count++] = j;
						pval = stepval(pval);
					}
					if (count == 0)
						perrn(oldname);
					else if (count == 1)
						setparam(&hd,name,PFSHORT,1,
							*sval);
					else
						setparam(&hd,name,PFSHORT,
							count,sval);
				}
				else if (strcmp(fmt,"i") == 0) {
					pval = val;
					ival = (int *) memalloc(strlen(val)/2,
						sizeof(int));
					count = 0;
					while (TRUE) {
						if (sscanf(pval,"%d",ival+count)
						    != 1)
							break;
						count++;
						pval = stepval(pval);
					}
					if (count == 0)
						perrn(oldname);
					else if (count == 1)
						setparam(&hd,name,PFINT,1,
							*ival);
					else
						setparam(&hd,name,PFINT,
							count,ival);
				}
				else if (strcmp(fmt,"f") == 0) {
					pval = val;
					fval = (float *) memalloc(strlen(val)/2,
						sizeof(float));
					count = 0;
					while (TRUE) {
						if (sscanf(pval,"%f",fval+count)
						    != 1)
							break;
						count++;
						pval = stepval(pval);
					}
					if (count == 0)
						perrn(oldname);
					else if (count == 1)
						setparam(&hd,name,PFFLOAT,1,
							*fval);
					else
						setparam(&hd,name,PFFLOAT,
							count,fval);
				}
				else {
					sprintf(msg,"illegal format %s",fmt);
					perr(HE_MSG,msg);
				}
			}
			clearval(val);
		}
	}
	pval = desc;
	for (i=0;i<size;i++) {
		if (desc[i] == NADA || desc[i] == ' ' || desc[i] == '\0' ||
		    desc[i] == '\n')
			continue;
		j = lenpar(desc+i,size-i);
		fprintf(stderr,"%s: warning, unconverted parameter `%s'\n",
			Progname,desc+i);
		i += j-1;
	}
	hd.seq_desc = "";
	hd.sizedesc = 0;
	write_headeru(&hd,argc,argv);
	if (hd.sizeimage) {
		for (i=0;i<hd.num_frame;i++) {
			fread_image(fp,&hd,i,filename);
			write_image(&hd,i);
		}
	}
	else {
		while ((c=getc(fp)) != EOF) putchar(c);
	}
	return(0);
}

int lenpar(s,n)

char *s;
int n;

{
	int i;

	i = 0;
	while (*s != '\n') {
		s++; i++;
		if (i >= n)
			perr(HE_MSG,
				"lenpar:  ran out of string - messed up XHDR");
	}
	*s++ = '\0'; i++;
	while (*s != '\n') {
		s++; i++;
		if (i >= n)
			perr(HE_MSG,
				"lenpar:  ran out of string - messed up XHDR");
	}
	i++;
	return(i);
}

char *findpar(s)

char *s;

{
	char *pd,*end,*pd2;

	pd = desc;
	end = desc + size;

	while (pd < end) {
		while (*pd == NADA || *pd == ' ') {
			if (++pd >= end)
				return(NULLP);
		}
		pd2 = pd;
		while (*pd2 != '\n') {
			if (++pd2 >= end)
				return(NULLP);
		}
		*pd2 = '\0';
		if (strcmp(s,pd) == 0) {
			while (pd <= pd2)
				*pd++ = NADA;
			pd2 = pd;
			while (*pd2 != '\n') {
				if (++pd2 >= end)
					return(NULLP);
			}
			*pd2 = '\0';
			return(pd);
		}
		*pd2 = '\n';
		pd = pd2 + 1;
		while (*pd != '\n') {
			if (++pd >= end)
				return(NULLP);
		}
		pd++;
	}
	return(NULLP);
}

char *stepval(s)

char *s;

{
	while (*s == ' ' || *s == '\t') {
		s++;
		if (s > desc+size)
			perr(HE_MSG,
				"stepval: ran out of string - messed up XHDR");
	}
	while (*s != ' ' && *s != '\t' && *s != '\0') {
		s++;
		if (s > desc+size)
			perr(HE_MSG,
				"stepval: ran out of string - messed up XHDR");
	}
	return(s);
}

void clearval(s)

char *s;

{
	while (*s != '\0')
		*s++ = NADA;
	*s++ = NADA;
}

void perrn(s)

char *s;

{
	char msg[100];

	sprintf(msg,"error reading parameter %s",s);
	perr(HE_MSG,msg);
}
