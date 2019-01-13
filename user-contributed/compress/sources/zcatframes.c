/*
 *  modified to `zcat' files if necessary; Pat Flynn 8/89
 */


/*	Copyright (c) 1982 Michael Landy, Yoav Cohen, and George Sperling

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/* catframes.c - to concatenate several frame-files into one file.
 *
 * usage:	catframes file1 [file2 ... ] > newfile
 *
 * to load:	cc -o catframes catframes.c -lhipl
 *
 * Y. Cohen 2/23/82
 *
 * notes:	header of output file is identical to header
 *		of first input file, except for updated
 *		history and changed "num_frame".
 *		Headers of subsequent files have to agree with first
 *		header in pixel-format, packing and dimensions.
 *		In contrast to the system's "cat" program,
 *		this program completely ignores the standard input.
 *
 *              The files may be compressed.  If so, their names must end with
 *              the standard '.Z' extension used by the UNIX compress
 *              utility.
 *
 * modified for new fread_header - msl 4/26/82
 * modified to use read/write - msl 8/5/83
 * fixed a bug for binary images - msl 11/6/85
 * modified to work for short/int/float/complex - msl 3/22/88
 * modified to work with pyramids - msl 3/7/89
 * modified for 'zcat'ing - pjf 8/89
 */

#include <stdio.h>
#include <hipl_format.h>
char pipemode; /* ick, a new global variable. this is TRUE if the file
                  currently being read is actually a pipe. Needed to pick
                  between fclose and pclose when we're done reading. */

FILE *myfopen(name,mode)
  char *name,*mode;
  {
    int l=strlen(name);
    if ((name[--l]=='Z')&&(name[--l]=='.')) {
      /* we're a compressed file. open a pipe. */
      char opencmd[1024];
      pipemode=TRUE;
      sprintf(opencmd,"/usr/ucb/zcat %s",name);
      return popen(opencmd,mode);
      }
    else {
      pipemode=FALSE;
      return fopen(name,"r");
      }
  }

void myfclose(fp)
  FILE *fp;
  {
    if (pipemode)
      pclose(fp);
    else
      fclose(fp);
  }

int main(argc,argv)

int	argc;
char	**argv;

{
	struct header hd1,hd2,*thd;
	int runs,targc,sumframes,sizepix,numbytes,i,f,toplev,one=1;
	char *fr,**targv,tmp[100];
        FILE *fp;

	Progname = strsave(*argv);
	if (argc <= 1)
		perr(HE_MSG,"usage: catframes file1 [file2 ...]");
	if (argv[argc-1][0]=='-' && argv[argc-1][1]=='D')
		argc--;

	sumframes=0;
	for(runs=1;runs<=2;runs++) {
		thd=(&hd1); 
		targc=argc; 
		targv=argv;

		while(--targc>0) {
			++targv;
                        if (!(fp=myfopen(*targv,"r")))
				perr(HE_OPEN,*targv);
			fread_header(fp,thd,*targv);
			if (runs==1 && targc==argc-1) {
				if (hd1.pixel_format == PFBYTE)
					sizepix = sizeof(char);
				else if (hd1.pixel_format == PFLSBF)
					sizepix = sizeof(char);
				else if (hd1.pixel_format == PFMSBF)
					sizepix = sizeof(char);
				else if (hd1.pixel_format == PFSHORT)
					sizepix = sizeof(short);
				else if (hd1.pixel_format == PFINT)
					sizepix = sizeof(int);
				else if (hd1.pixel_format == PFFLOAT)
					sizepix = sizeof(float);
				else if (hd1.pixel_format == PFCOMPLEX)
					sizepix = 2*sizeof(float);
				else if (hd1.pixel_format == PFDOUBLE)
					sizepix = sizeof(double);
				else if (hd1.pixel_format == PFDBLCOM)
					sizepix = 2*sizeof(double);
				else if (hd1.pixel_format == PFINTPYR)
					sizepix = sizeof(int);
				else if (hd1.pixel_format == PFFLOATPYR)
					sizepix = sizeof(float);
				else
					perr(HE_MSG,"sequence must be in byte, float,\
int, complex, integer pyramid, or floating pyramid format");
				if (hd1.pixel_format == PFINTPYR ||
				    hd1.pixel_format == PFFLOATPYR) {
					getparam(&hd1,"toplev",PFINT,&one,
						&toplev);
					i = pyrnumpix(toplev,
					    hd1.orows,hd1.ocols);
					numbytes = sizepix * i;
				}
				else
					numbytes = sizepix * hd1.orows *
					    hd1.ocols;
				if (hd1.pixel_format == PFLSBF ||
				    hd1.pixel_format==PFMSBF)
					numbytes = hd1.orows*((hd1.ocols + 7)/8);
				fr = (char *) halloc(numbytes,sizeof(char));
				sumframes = hd1.num_frame;
			}
			else if (runs==1) {
				sumframes += hd2.num_frame;
				if (hd2.pixel_format != hd1.pixel_format ||
				    hd2.orows != hd1.orows ||
				    hd2.ocols != hd1.ocols) {
					sprintf(tmp,
						"header of %s does not match",
						*targv);
					perr(HE_MSG,tmp);
				}
				if (hd1.pixel_format == PFINTPYR ||
				    hd1.pixel_format == PFFLOATPYR) {
					getparam(&hd2,"toplev",PFINT,&one,
						&i);
					if (i != toplev)
					    perr(HE_MSG,
						"number of levels mismatch");
				}
			}
			else {	/* second round - read and write frames */
				f = thd->num_frame;
				if (targc==argc-1) {
					hd1.num_frame=sumframes;
					update_header(&hd1,argc,argv);
					write_header(&hd1);
				}
				for (i=0;i<f;i++) {
					if (fread(fr,numbytes*sizeof(char),1,fp)
						!=1)
						    perr(HE_MSG,
							"error during read");
					if (fwrite(fr,numbytes*sizeof(char),1,
					    stdout) != 1)
						    perr(HE_MSG,
							"error during write");
				}
			}
			myfclose(fp);
			thd=(&hd2);
		}
	}
	return(0);
}
