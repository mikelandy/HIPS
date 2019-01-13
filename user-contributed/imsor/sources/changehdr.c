/*      Copyright (c) 1992  Karsten Hartelius, IMSOR.

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   

 changehdr - changes header-fields.   

 usage: see manual

 to load:
	cc -o changehdr changehdr.c -L /gsnk1/hips2/lib -lm -lhips

 to run:
     changehdr parameters  <HIPS-header >HIPS-header

 include-files: none. 
*/

#include <stdio.h>
#include <hipl_format.h>

#define byte unsigned char

int main(argc,argv)
int argc;
char *argv[];
{
	int 		i;
	struct header hd;
	char		*flag;

	read_header(&hd);
	Progname = strsave(*argv);
	for (i=1;i<argc;i++) {
		if (argv[i][0]=='-') {
		switch (argv[i][1]) {
			case 'e':	flag=argv[++i];
					setparam(&hd,flag,PFBYTE,1,atoi(argv[++i]));
					break;
			case 'f': hd.num_frame = atoi(argv[++i]); break;
			case 'p': hd.pixel_format = atoi(argv[++i]); break;
			case 'r': hd.rows = atoi(argv[++i]); break;
			case 'c': hd.cols = atoi(argv[++i]); break;
			case 'o': switch (argv[i][2]) {
				case 'r': hd.orows = atoi(argv[++i]); break;
				case 'c': hd.ocols = atoi(argv[++i]); break;
				case 'n': hd.orig_name = argv[++i]; break;
				case 'd': hd.orig_date = argv[++i]; break;
				} break;
			case 's': switch (argv[i][2]) {
				case 'n': hd.seq_name = argv[++i]; break;
				case 'h': hd.seq_history = argv[++i]; break;
				} break;
		    	case '?': 
				fprintf(stderr,"changehdr param < inseq  > outseq \n");
				fprintf(stderr,"-or  number of rows\n");
				fprintf(stderr,"-oc  number of cols\n");
				fprintf(stderr,"-r  number of rows in roi.\n");
				fprintf(stderr,"-c  number of cols in roi. \n");
				fprintf(stderr,"-f  number of frames\n");
				fprintf(stderr,"-p  pixel format code\n");
				fprintf(stderr,"-on originator of sequence\n");
				fprintf(stderr,"-od originate sequence date\n");
				fprintf(stderr,"-sn sequence name\n");
				exit(1);
				break;
			default:
			perr(HE_MSG,"changehdr -[?rcfp(or)(oc)(on)(od)(sn)]  < inseq  > ascii value\n");
		}
	    }
	}
	write_header(&hd);
	return(0);
}
