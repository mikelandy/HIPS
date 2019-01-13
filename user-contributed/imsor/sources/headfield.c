/*      Copyright (c) 1992  Karsten Hartelius, IMSOR.

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   

 headfield - returns value of header-field.  

 usage: see manual

 to load:
	cc -o headfield headfield.c -L /gsnk1/hips2/lib -lm -lhips

 to run:
     headfield  parameters  < header  > value 

 include-files: none.

 Karsten Hartelius, IMSOR, 11/5 1992.
*/

#include <stdio.h>
#include <hipl_format.h>

int main(argc,argv)
int argc;
char *argv[];
{
	int 		i, count;
	byte		val;
	char		*flag;
	struct header hd;

	read_header(&hd);
	Progname = strsave(*argv);
	for (i=1;i<argc;i++) {
		if (argv[i][0]=='-') {
		switch (argv[i][1]) {
			case 'e': flag=argv[++i];
					val=0;
					count=1;
					if (findparam(&hd,flag)!=NULLPAR)
						getparam(&hd,flag,PFBYTE,&count,&val);
					printf("%d ",val); break;
			case 'f': printf("%d ",hd.num_frame); break;
			case 'p': printf("%d ",hd.pixel_format); break;
			case 'r': printf("%d ",hd.rows); break;
			case 'c': printf("%d ",hd.cols); break;
			case 'o': switch (argv[i][2]) {
					case 'r': printf("%d ",hd.orows); break;
					case 'c': printf("%d ",hd.ocols); break;
					case 'n': printf("%s ",hd.orig_name); break;
					case 'd': printf("%s ",hd.orig_date); break;
					} break;
			case 's': switch (argv[i][2]) {
					case 'n': printf("%s ",hd.seq_name); break;
					case 'h': printf("%s ",hd.seq_history); break;
					case 'd': printf("%s ",hd.seq_desc); break;
					} break;
			case '?': 
	                    fprintf(stderr,"changehdr < inseq  > outseq \n");
					fprintf(stderr,"-or  number of rows\n");
					fprintf(stderr,"-oc  number of cols\n");
					fprintf(stderr,"-r  number of rows in roi.\n");
					fprintf(stderr,"-c  number of cols in roi. \n");
					fprintf(stderr,"-f  number of frames\n");
					fprintf(stderr,"-p  pixel format code\n");
					fprintf(stderr,"-on originator of sequence\n");
					fprintf(stderr,"-od originate sequence date\n");
					fprintf(stderr,"-sn sequence name\n");
					fprintf(stderr,"-sh sequence history\n");
					fprintf(stderr,"-sh sequence description\n");
					exit(1);
			default:
					perr(HE_MSG,"headfield options < inseq  > ascii value\n");
			}
		}
	}
	return(0);
}
