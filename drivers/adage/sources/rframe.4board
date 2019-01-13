/*	Copyright (c) 1989 Michael Landy

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/* this is the 4board version for systems with 4 DR256 memories */

/*
 * rframe.c - read a frame from the Grinnell
 *
 * Usage:	rframe [rows [cols [initialrow [initialcol]]]] [-v]
 *
 * Defaults:	rows: 512, cols: 512, initialrow: 0, initialcol: 0
 *
 * Load:	cc -o rframe rframe.c -lhips
 *
 * Michael Landy - 6/22/85
 *
 * Reads a frame from the Adage starting at (initialrow,initialcol)
 * with size rows x cols.  There is no wraparound so large sizes will
 * be truncated.  The -v switch uses standard 30Hz video.
 */

#include <hipl_format.h>
#include <stdio.h>
#include <sys/ikio.h>
#include <graphics/ik_const.h>

int videosw=0;

main(argc,argv)
char *argv[];
{
	extern int Ikonas;
	int r,c,ir,ic,or,oc,i,argcc;
	char *fr;
	struct header hd;

	Progname = strsave(*argv);
	argcc=argc;
	r=c=512; ir=ic=0;
	if (strcmp(argv[argc-1],"-v")==0) {
		videosw++;
		argc--;
	}
	if(argv[argc-1][0]=='-')
		argc--;
	if(argc>1)
		r=atoi(argv[1]);
	if(argc>2)
		c=atoi(argv[2]);
	if(argc>3)
		ir=atoi(argv[3]);
	if(argc>4)
		ic=atoi(argv[4]);

	or=512-ir;or=r<or?r:or;
	oc=(512-ic) & (~01);oc=c<oc?c:oc;
	if((or<1)||(oc<1))
		perr(HE_MSG,"wrong dimensions");
	if (oc & 01)
		perr(HE_MSG,"number of columns must be even");
	if ((fr = (char *) calloc(or*oc,sizeof(char))) == 0)
		perr(HE_MSG,"can't allocate core");
	init_header(&hd,"","",1,"",or,oc,PFBYTE,1,"");
	update_header(&hd,argcc,argv);
	write_header(&hd);

	Ik_open();
	Ik_init(videosw ? IK_30INT_HIRES : IK_60NON_HIRES);

	Ik_set_mode(SET_8_BIT_MODE);
	Ik_windowdma(ic,oc,IK_HXY_ADDR);
	Ik_dmard8(IK_HXY_ADDR,ic,ir,fr,or*oc);
	if (fwrite(fr,or*oc*sizeof(char),1,stdout) != 1)
		perr(HE_MSG,"error during write");

	Ik_close();
	return(0);
}
