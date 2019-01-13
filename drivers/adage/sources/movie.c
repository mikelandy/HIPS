/*	Copyright (c) 1989 Michael Landy

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/* movie - to transfer images to Adage in real time (60 fps).
 *
 * arguments and defaults -- as those for "wframe",
 *		-[number]  causes the movie to run "number" times and
 *			then return control (e.g. "-1", for one time only).
 *		-r starts the Betamax just prior to outputting.
 *		-c outputs a cue spot before running the movie.
 *		-p repeats the first frame 60/lapse times.
 *		-b adds a blank frame at the end.
 *		-u "unsync", cancels the sync feature.
 *		-a 'all' is shorthand for -s -r -p -b.
 *		-ln time lapse, slows the movie down (in synced mode only) by a
 *			factor of 'n', where n is an integer.
 *		-S step the movie frame by frame (use 'x' to exit,
 *			and 'b' for backwards step), always unsynced
 *		-tTwinfile twin movies run interlaced.
 *		-o red green blue, sets the color map for pixels which are one
 *			(for binary movies only)
 *		-z red green blue, sets the color map for pixels which are zero
 *			(for binary movies only)
 *		-g red green blue, sets the color map for background pixels
 *			(for binary movies only)
 *
 * usage:	movie [startrow [startcol]]
 *				[-number][-r][-c][-p][-b][-u][-a][-ln][-S]
 *				[-o r g b][-z r g b][-g r g b]
 *
 * 		movie [startrow [startcol [twinstartrow [twinstartcol]]]]
 *				[-number][-r][-c][-p][-b][-u][-a][-ln][-S]
 *				[-o r g b][-z r g b][-g r g b]
 *				-tfilename
 *					< input_sequence
 *
 * Note that for binary movies, the starting column is truncated to an integer
 * multiple of 32, and number of columns in each frame is rounded up to the
 * nearest integer multiple of 32.
 *
 * to load:	cc -o movie movie.c -lhips -lIkonas
 *
 * Mike Landy - 7/12/85
 *
 * Note:	extensive test should be added
 *		(e.g. open /dev/gr; write's; arguments etc.)
 */

#include <hipl_format.h>
#include <stdio.h>
#include <sys/ikio.h>
#include <vaxuba/drcmd.h>
#include <graphics/ik_const.h>

int binary = 0;
int single = -1;
int osingle = -1;
int cuesw = 0;
int recsw = 0;
int psw = 0;
int blanksw = 0;
int unsync = 0;
int lapse = 1;
int twin = 0;
int ss = 0;
int loop = 0;
int ir = 0, ic = 0;
int ir1 = 0, ir2 = 0, ic1 = 0, ic2 = 0;
int rg=0, gg=0, bg=0;
int r0=0, g0=0, b0=0;
int r1=1020, g1=1020, b1=1020;
char *store;

main(argc,argv)

int argc;
char *argv[];

{
	extern int Ikonas;
	char *st,*ifr,*pifr,k[11],*tfile;
	int i,j,ii,jj,numf,frame;
	int ir1,ic1,air,aic,sptr,extras,r,c,rc,stsize,f,twinptr,blankptr,cueptr;
	int cb,ocb;
	int lastrc,currarg;
	struct 	header hd,hd1;
	int fd,fd2;
	FILE *FP3,*fp;

	Progname = strsave(*argv);
	read_header(&hd);
	lastrc = argc-1;
	currarg = 1;
	while (currarg < argc) {
	    if (argv[currarg][0]=='-') {
		if (lastrc > currarg-1)
			lastrc = currarg-1;
		switch (argv[currarg][1]) {
		case 'D':	break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':	osingle=single=atoi(&argv[currarg][1]);
				break;
		case 'c':	cuesw++;
				break;
		case 'r':	recsw++;
				break;
		case 'p':	psw++;
				break;
		case 'b':	blanksw++;
				break;
		case 'u':	unsync++;
				break;
		case 'l':	lapse = atoi(&argv[currarg][2]);
				if (lapse < 1)
					perr(HE_MSG,"-l parameter must be > 0");
				break;
		case 'S':	ss++;
				if ((FP3 = fopen("/dev/tty","r")) == NULL)
					perr(HE_MSG,"can't open tty!");
				break;
		case 't':	if ((fp = fopen(&argv[currarg][2],"r")) == NULL)
				    perr(HE_MSG,
					"can't open twin file -t ignored");
				else
				    twin++;
				tfile = argv[currarg]+2;
				break;
		case 'a':	osingle=single=1;
				recsw++;
				psw++;
				blanksw++;
				break;
		case 'o':	if (currarg+3 >= argc)
					perr(HE_MSG,"-o takes r g b as arguments");
				r1 = atoi(argv[currarg+1]);
				g1 = atoi(argv[currarg+2]);
				b1 = atoi(argv[currarg+3]);
				currarg += 3;
				break;
		case 'z':	if (currarg+3 >= argc)
					perr(HE_MSG,"-z takes r g b as arguments");
				r0 = atoi(argv[currarg+1]);
				g0 = atoi(argv[currarg+2]);
				b0 = atoi(argv[currarg+3]);
				currarg += 3;
				break;
		case 'g':	if (currarg+3 >= argc)
					perr(HE_MSG,"-g takes r g b as arguments");
				rg = atoi(argv[currarg+1]);
				gg = atoi(argv[currarg+2]);
				bg = atoi(argv[currarg+3]);
				currarg += 3;
				break;
		default:	fprintf(stderr,
					"movie: unrecognized option %1s\n",
					&argv[currarg][1]);
		}
	    }
	    currarg++;
	}
	if (twin) {
		ir = ir1 = ir2 = (480-hd.orows)/2;
		ic = ic1 = 251-hd.ocols;
		ic2 = 261;
	}
	else {
		ir = ir1 = ir2 = (480-hd.orows)/2;
		ic = ic1 = ic2 = (512-hd.ocols)/2;
	}
	if (lastrc>0)
		ir=ir1=ir2=atoi(argv[1]);
	if (lastrc>1)
		ic=ic1=ic2=atoi(argv[2]);
	if (twin) {
		if (lastrc>2)
			ir2 = atoi(argv[3]);
		if (lastrc>3)
			ic2 = atoi(argv[4]);
	}

	if (hd.pixel_format != PFBYTE && hd.pixel_format != PFLSBF)
		perr(HE_MSG,"frame must be in byte or LSBF packed format");
	else if (hd.pixel_format == PFLSBF)
		binary++;
	if (binary) {
		ic &= ~037;
		ic1 &= ~037;
		ic2 &= ~037;
	}
	r=hd.orows; ocb=c=hd.ocols;
	if (binary) {
		cb = (c + 7)/8;
		ocb = ((c + 31)/32)*4;
	}
	else {
		cb = c;
		ocb = (c+1) & (~01);
	}
	rc=r*ocb;
	stsize=f=hd.num_frame;
	if (twin) {
		fread_header(fp,&hd1,tfile);
		if (r!=hd1.orows || c!=hd1.ocols || f!=hd1.num_frame
		    || hd.pixel_format!=hd1.pixel_format)
			perr(HE_MSG,"twin sequences inconsistent");
		stsize += f;
	}

	stsize += ((blanksw || cuesw) ? 1 : 0) + (cuesw ? 1 : 0);
	if ((store=(char *)calloc(rc*stsize,sizeof(char))) == 0) 
		perr(HE_MSG,"not enough core");
	extras = (psw ? ((60/lapse)-1) : 0) * (twin ? 2 : 1);
	fprintf(stderr,"movie: %d frames per transfer\n",stsize+extras);
	twinptr = i = rc*f;
	if (twin)
		i += rc*f;
	blankptr = i;
	cueptr = i + rc;

	if (cb==ocb) {
	    if (fread(store,rc*f*sizeof(char),1,stdin) != 1)
		perr(HE_MSG,"error reading primary input");
	}
	else {
		if ((ifr = (char *) calloc(r*cb,sizeof(char))) == 0)
			perr(HE_MSG,"can't allocate output frame core");
		st = store;
		for (frame=0;frame<f;frame++) {
		    if (fread(ifr,r*cb*sizeof(char),1,stdin) != 1)
			perr(HE_MSG,"error reading primary input");
		    pifr = ifr;
		    for (i=0;i<r;i++) {
			for (j=0;j<ocb;j++) {
				if (j < cb)
					*st++ = *pifr++;
				else
					*st++ = 0;
			}
		    }
		}
	}
	if (twin) {
		if (cb==ocb) {
		    if (fread(store+twinptr,rc*f*sizeof(char),1,fp) != 1)
			    perr(HE_MSG,"error reading twin input");
		}
		else {
			st = store+twinptr;
			for (frame=0;frame<f;frame++) {
			    if (fread(ifr,r*cb*sizeof(char),1,fp) != 1)
				    perr(HE_MSG,"error reading twin input");
			    pifr = ifr;
			    for (i=0;i<r;i++) {
				for (j=0;j<ocb;j++) {
					if (j < cb)
						*st++ = *pifr++;
					else
						*st++ = 0;
				}
			    }
			}
		}
		fclose(fp);
	}
	if (blanksw || cuesw) {
		st=store+blankptr;
		for (i=0;i<rc;i++)
			*st++ = 0;
	}
	if (cuesw) {
		st=store+cueptr;
		for (i=0;i<rc;i++)
			*st++ = 0;
		if (binary) {
		    ii = (r < 8) ? r : 8;
		    for (i=0;i<ii;i++)
			store[cueptr + ((r/2)-(ii/2)+i)*ocb+(ocb/2)]=255;
		}
		else {
		    ii = (r < 8) ? r : 8;
		    jj = (c < 8) ? c : 8;
		    for (i=0;i<ii;i++)
			for (j=0;j<jj;j++)
				store[cueptr +
				    ((r/2)-(ii/2)+i)*c+((c/2)-(jj/2)+j)]=228;
		}
	}

	Ik_open();
	if (binary) {
		Ik_init(IK_60NON_HIRES);
		Ik_cload_map(1,0);
		Ik_set_mode(SET_32_BIT_MODE);
	}
	else {
		Ik_init(IK_60NON_LORES);
		Ik_cload_map(0,0);
		Ik_set_mode(SET_8_BIT_MODE);
	}
	if (recsw) {
		ii=0177377;	/* put betamax in motion */
		fd2=open("/dev/dr",2);lseek(fd2,(long) 2,0); write(fd2,&ii,2);
		Ik_delay(60);
		ii=0177777; lseek(fd2,(long) 2,0);write(fd2,&ii,2); close(fd2);
	}
	if (cuesw) {
	    if (binary) {
		Ik_windowdma(ic,ocb,IK_WDHXY_ADDR);
		i = HI_WORDXYH(02,0,(ic>>5),ir);
		j = LO_WORDXYH(02,0,(ic>>5),ir);
		Ik_dmawr8(IK_WD_ADDR,i,j,store+cueptr,rc);
		Ik_delay(60);
		Ik_delay(60);
		Ik_dmawr8(IK_WD_ADDR,i,j,store+blankptr,rc);
		Ik_delay(60);
		Ik_delay(60);
	    }
	    else {
		Ik_windowdma(ic,ocb,IK_XY_ADDR);
		Ik_dmawr8(IK_XY_ADDR,ic,ir,store+cueptr,rc);
		Ik_delay(60);
		Ik_delay(60);
		Ik_dmawr8(IK_XY_ADDR,ic,ir,store+blankptr,rc);
		Ik_delay(60);
		Ik_delay(60);
	    }
	}
	do {
		numf = (twin ? 2 : 1)*f + extras;
		if (blanksw && single == 1)
			numf += (twin ? 2 : 1);
		for (frame=0;frame<numf;frame++) {
			aic = ic1;
			air = ir1;
			if (twin && (frame & 01)) {
				aic = ic2;
				air = ir2;
			}
			if (frame < extras) {
				if (twin)
					sptr = (frame & 01)*rc*f;
				else
					sptr = 0;
			}
			else if (frame >= (twin ? 2 : 1)*f + extras)
				sptr = blankptr;
			else if (twin)
				sptr = (((frame - extras)/2) * rc) +
					((frame & 01) ? rc*f : 0);
			else
				sptr = (frame - extras) * rc;

			if (binary) {
				Ik_windowdma(aic,ocb,IK_WDHXY_ADDR);
				i = HI_WORDXYH(02,0,(aic>>5),air);
				j = LO_WORDXYH(02,0,(aic>>5),air);
				Ik_dmawr8(IK_WD_ADDR,i,j,store+sptr,rc);
			}
			else {
				Ik_windowdma(aic,ocb,IK_XY_ADDR);
				Ik_dmawr8(IK_XY_ADDR,aic,air,store+sptr,rc);
			}
			if (ss!=0 && loop==0) {
				fprintf(stderr,"%d",frame);
				fgets(k,10,FP3);
				if (*k=='x')
					loop=1;
				else if (*k=='b') {
					if (frame > 0)
						frame -=2;
					else
						frame--;
				}
			}
			if (ss == 0 || unsync)
				Ik_delay(lapse);
		}
		loop = 0;
	} while (--single != 0);

	exit(0);
}
