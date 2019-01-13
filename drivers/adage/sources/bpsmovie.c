/*	Copyright (c) 1989 Michael Landy

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/* This is the 4 board version of bps_viewport, for systems with 4 DR256
	memory boards */

/* bpsmovie - to transfer images to Adage in real time (60 fps) using bps
 *		to set the viewport.
 *
 * arguments and defaults -- as those for "wframe",
 *		-[number]  causes the movie to run "number" times and
 *			then return control (e.g. "-1", for one time only).
 *		-r starts the Betamax just prior to outputting.
 *		-c outputs a cue spot before running the movie.
 *			-cnnn uses nnn sync pulses for the cue, and another
 *			nnn for the blank following the cue.
 *		-p repeats the first frame 60/lapse times.
 *		-b adds a blank frame at the end.
 *		-a 'all' is shorthand for -r -p -b.
 *		-ln time lapse, sets the number of 60ths of a second (sync
 *			pulses) per frame.  Defaults to 4 (15 fps).
 *		-lseqfile uses the contents of "seqfile" to control the
 *			movie.  The file consists of a line with "nctrl", the
 *			number of control lines which follow, and "nctrl"
 *			lines consisting of a frame number of lapse amount
 *			to be used for that frame.
 *		-tTwinfile twin movies run interlaced.
 *		-o red green blue, sets the color map for pixels which are one
 *			(for binary movies only)
 *		-z red green blue, sets the color map for pixels which are zero
 *			(for binary movies only)
 *		-g red green blue, sets the color map for background pixels
 *			(for binary movies only)
 *		-q red green blue, sets the color map for the cue spot
 *			(for binary movies only)
 *		-v standard video (30Hz interlaced)
 *		-Vnnn	Vertical zoom by a factor of nnn
 *		-Hnnn	Horizontal zoom by a factor of nnn
 *		-C file		load color map from file: file must contain
 *			256 lines; each line must contain 3
 *			hexadecimal numbers separated by spaces.  For
 *			any line number k (0<=k<=255), the three numbers assign
 *			respectively the red, green and blue levels
 *			assigned to pixel value k.  The lookup tables have
 *			room for 10 bits for each color.
 *
 * usage:	bpsmovie [startrow [startcol]]
 *				[-number][-r][-c][-p][-b][-a][-ln or -lseqfile]
 *				[-tfilename][-Vnnn][-Hnnn][-C colorfile]
 *				[-o r g b][-z r g b][-g r g b][-q r g b][-v]
 *					< input_sequence
 *
 * Note that for binary movies, the number of columns in each frame is rounded
 * up to the nearest integer multiple of 32.  Also note that in binary movies,
 * no frames may cross the 1023 to 1024 column boundary (because word mode
 * writes in hi resolution mode don't cross this boundary correctly - the high
 * x bit is elsewhere in the address); thus, the number of frames per column
 * is twice that for 1024, and the frame that would have crossed that boundary
 * is bumped to be 1024.
 *
 * to load:	cc -o bpsmovie bpsmovie.c -lhips -lIkonas
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
#include <ctype.h>

#define	strow(imageno) (((imageno % (numfrow*numfcol)) / numfcol) * outr)
#define	stcolb(imageno) ((((imageno % numfcol) < numfcol2) ? 0 : 1024) + \
			((imageno % numfcol2) * movoutc))
#define	stcoln(imageno) ((imageno % numfcol) * movoutc)
#define stcol(imageno) (binary ? stcolb(imageno) : stcoln(imageno))
#define	stbitplane(imageno) (imageno / (numfrow*numfcol))
#define	stplane(imageno) (stbitplane(imageno) % 2)
#define	stcard(imageno) (02 + (stbitplane(imageno) / 2))

int binary = 0;
int single = -1;
int cuesw = 0;
int recsw = 0;
int psw = 0;
int blanksw = 0;
int videosw = 0;
int lapse = 4;
int twin = 0;
int ir = 0, ic = 0;
int rg=0, gg=0, bg=0;
int r0=0, g0=0, b0=0;
int r1=1020, g1=1020, b1=1020;
int rq=500, gq=500, bq=500;
int yzoom=1, xzoom=1;
char *store;
char *cmap = (char *) 0;

main(argc,argv)

int argc;
char *argv[];

{
	char *st,*ifr,*pifr,k[11],sys[512],*tfile;
	int i,j,ii,jj,numf,frame;
	int air,sptr,extras,r,c,rcb,f,blankptr,cueptr,vmode;
	int numfcol,numfrow,fcol,frow,outr,outc,movoutc,outrcb,numoutf,conv;
	int numfcol2,cb,ocb;
	int lastrc,currarg,rowmask,colmask;
	struct 	header hd,hd1;
	int fd,fd2,nctrl,ctrlf,ctrll;
	FILE *FP3,*SEQF,*fp;

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
		case '9':	single=atoi(&argv[currarg][1]);
				break;
		case 'c':	cuesw=60;
				if (argv[currarg][2] != '\0')
					cuesw = atoi(&argv[currarg][2]);
				break;
		case 'r':	recsw++;
				break;
		case 'p':	psw++;
				break;
		case 'b':	blanksw++;
				break;
		case 'v':	videosw++;
				break;
		case 'l':	if (isdigit(argv[currarg][2])) {
					lapse = atoi(&argv[currarg][2]);
					if (lapse < 1)
					    perr(HE_MSG,
						"-l parameter must be > 0");
				}
				else {
					if ((SEQF=fopen((argv[currarg]+2),"r"))
						== NULL)
					    perr(HE_MSG,
						"can't open -l seq file");
					lapse = 0;
				}
				break;
		case 'V':	yzoom = atoi(&argv[currarg][2]);
				if (yzoom < 1)
					perr(HE_MSG,
						"-V parameter must be > 0");
				break;
		case 'H':	xzoom = atoi(&argv[currarg][2]);
				if (xzoom < 1)
					perr(HE_MSG,
						"-H parameter must be > 0");
				break;
		case 't':	if ((fp = fopen(&argv[currarg][2],"r")) == NULL)
				    perr(HE_MSG,
					"can't open twin file -t ignored");
				else
				    twin++;
				break;
		case 'a':	single=1;
				recsw++;
				psw++;
				blanksw++;
				break;
		case 'o':	if (currarg+3 >= argc)
					perr(HE_MSG,
						"-o takes r g b as arguments");
				r1 = atoi(argv[currarg+1]);
				g1 = atoi(argv[currarg+2]);
				b1 = atoi(argv[currarg+3]);
				currarg += 3;
				break;
		case 'z':	if (currarg+3 >= argc)
					perr(HE_MSG,
						"-z takes r g b as arguments");
				r0 = atoi(argv[currarg+1]);
				g0 = atoi(argv[currarg+2]);
				b0 = atoi(argv[currarg+3]);
				currarg += 3;
				break;
		case 'g':	if (currarg+3 >= argc)
					perr(HE_MSG,
						"-g takes r g b as arguments");
				rg = atoi(argv[currarg+1]);
				gg = atoi(argv[currarg+2]);
				bg = atoi(argv[currarg+3]);
				currarg += 3;
				break;
		case 'q':	if (currarg+3 >= argc)
					perr(HE_MSG,
						"-q takes r g b as arguments");
				rq = atoi(argv[currarg+1]);
				gq = atoi(argv[currarg+2]);
				bq = atoi(argv[currarg+3]);
				currarg += 3;
				break;
		case 'C':	if (currarg+1 >= argc)
					perr(HE_MSG,
						"-C takes cmapfilename as arg");
				cmap = argv[++currarg];
				break;
		default:	fprintf(stderr,
					"bpsmovie: unrecognized option %1s\n",
					&argv[currarg][1]);
		}
	    }
	    currarg++;
	}
	if (hd.pixel_format != PFBYTE && hd.pixel_format != PFLSBF)
		perr(HE_MSG,"frame must be in byte or LSBF packed format");
	else if (hd.pixel_format == PFLSBF)
		binary++;
	if (videosw)
		vmode = IK_30INT_HIRES;
	else
		vmode = IK_60NON_HIRES;
	if (twin) {
		ir = 256 - (hd.orows * yzoom)/2;
		ic = 256 - hd.ocols * xzoom;
	}
	else {
		ir = 256 - (hd.orows * yzoom)/2;
		ic = 256 - (hd.ocols * xzoom)/2;
	}
	if (lastrc>0)
		ir=(atoi(argv[1]));
	if (lastrc>1)
		ic=(atoi(argv[2]));

	r=hd.orows; cb=c=hd.ocols;
	outr = r;
	if (videosw) {
		if (xzoom==1 || yzoom==3)
			outr = (r + 3) & ~03;
		else if (xzoom==2)
			outr = (r + 1) & ~01;
	}
	else {
		if (xzoom==1 || yzoom==3)
			outr = (r + 1) & ~01;
	}
	if (binary) {
		cb = (c + 7)/8;
		outc = (c + 037) & ~037;
		ocb = outc/8;
	}
	else {
		cb = c;
		outc = ocb = c;
		if (videosw && binary) {
			if (xzoom==1)
				outc = ocb = (c+1) & (~01);
		}
		else {
			if (xzoom==1 || xzoom==3)
				outc = ocb = (c+3) & (~03);
			else
				outc = ocb = (c+1) & (~01);
		}
	}
	movoutc = outc;
	rcb=r*cb;
	outrcb=outr*ocb;
	numoutf=f=hd.num_frame;
	if (twin) {
		fread_header(fp,&hd1,tfile);
		if (r!=hd1.orows || c!=hd1.ocols || f!=hd1.num_frame
		    || hd.pixel_format!=hd1.pixel_format)
			perr(HE_MSG,"twin sequences inconsistent");
		movoutc *= 2;
	}

	numoutf += ((blanksw || cuesw) ? 1 : 0) + (cuesw ? 1 : 0);
	numfcol = binary ? ((1024/movoutc)*2) : (2048/movoutc);
	numfcol2 = numfcol/2;
	numfrow = 2048/outr;
	if (numfcol*numfrow*(binary ? 8 : 1) < numoutf)
		perr(HE_MSG,"can't fit sequence in Adage memory");
	if ((store=(char *)calloc(outrcb,sizeof(char))) == 0) 
		perr(HE_MSG,"not enough core");
	extras = (psw ? ((60/lapse)-1) : 0);
	fprintf(stderr,"bpsmovie: %d frames per transfer\n",numoutf+extras);
	blankptr = f;
	cueptr = f + 1;

	st = store;
	for (i=0;i<outrcb;i++)
		*st++ = 0;

	if (cb!=ocb) {
		if ((ifr = (char *) calloc(rcb,sizeof(char))) == 0)
			perr(HE_MSG,"can't allocate input frame core");
		conv = 1;
	}
	else {
		conv = 0;
		ifr = store;
	}

	Ik_open();


	Ik_reset_micro();
	Ik_xb_init();

	/*
	 * Clear screen by setting the mini xbar to 333.  (Its not possible
	 * to do this with the viewport in 30Hz/Hires.)
	 */

	Ik_diowr(IK_WD_ADDR,IK_CHANNEL_XBAR,0,CXB_VIDEO_OFF);

	Ik_init_noxb(vmode);
	Ik_set_mode(SET_32_BIT_MODE);

	if (!binary)
		Ik_cload_map(cmap,0);

	if (binary)
		Ik_set_mode(SET_32_BIT_MODE);
	else
		Ik_set_mode(SET_8_BIT_MODE);

	if (blanksw || cuesw) {
		fcol = stcol(blankptr);
		frow = strow(blankptr);
		if (binary) {
			Ik_windowdma(fcol,ocb,IK_WDHXY_ADDR);
			i = HI_WORDXYH(stcard(blankptr),stplane(blankptr),
				(fcol>>5),frow);
			j = LO_WORDXYH(stcard(blankptr),stplane(blankptr),
				(fcol>>5),frow);
			Ik_dmawr8(IK_WD_ADDR,i,j,store,outrcb);
			if (twin) {
				fcol += outc;
				Ik_windowdma(fcol,ocb,IK_WDHXY_ADDR);
				i = HI_WORDXYH(stcard(blankptr),
					stplane(blankptr),(fcol>>5),frow);
				j = LO_WORDXYH(stcard(blankptr),
					stplane(blankptr),(fcol>>5),frow);
				Ik_dmawr8(IK_WD_ADDR,i,j,store,outrcb);
			}
		}
		else {
			Ik_windowdma(fcol,ocb,IK_HXY_ADDR);
			Ik_dmawr8(IK_HXY_ADDR,fcol,frow,store,outrcb);
			if (twin) {
				fcol += outc;
				Ik_windowdma(fcol,ocb,IK_HXY_ADDR);
				Ik_dmawr8(IK_HXY_ADDR,fcol,frow,store,outrcb);
			}
		}
	}
	if (cuesw) {
		if (binary) {
		    ii = (r < 8) ? r : 8;
		    for (i=0;i<ii;i++)
			store[((r/2)-(ii/2)+i)*ocb+(ocb/2)]=255;
		}
		else {
		    ii = (r < 8) ? r : 8;
		    jj = (c < 8) ? c : 8;
		    for (i=0;i<ii;i++)
			for (j=0;j<jj;j++)
				store[((r/2)-(ii/2)+i)*ocb+((c/2)-(jj/2)+j)]=228;
		}
		fcol = stcol(cueptr);
		frow = strow(cueptr);
		if (binary) {
			Ik_windowdma(fcol,ocb,IK_WDHXY_ADDR);
			i = HI_WORDXYH(stcard(cueptr),stplane(cueptr),
				(fcol>>5),frow);
			j = LO_WORDXYH(stcard(cueptr),stplane(cueptr),
				(fcol>>5),frow);
			Ik_dmawr8(IK_WD_ADDR,i,j,store,outrcb);
			if (twin) {
				fcol += outc;
				Ik_windowdma(fcol,ocb,IK_WDHXY_ADDR);
				i = HI_WORDXYH(stcard(cueptr),stplane(cueptr),
					(fcol>>5),frow);
				j = LO_WORDXYH(stcard(cueptr),stplane(cueptr),
					(fcol>>5),frow);
				Ik_dmawr8(IK_WD_ADDR,i,j,store,outrcb);
			}
		}
		else {
			Ik_windowdma(fcol,ocb,IK_HXY_ADDR);
			Ik_dmawr8(IK_HXY_ADDR,fcol,frow,store,outrcb);
			if (twin) {
				fcol += outc;
				Ik_windowdma(fcol,ocb,IK_HXY_ADDR);
				Ik_dmawr8(IK_HXY_ADDR,fcol,frow,store,outrcb);
			}
		}
	}

	for (frame=0;frame<f;frame++) {
		if (fread(ifr,rcb*sizeof(char),1,stdin) != 1)
			perr(HE_MSG,"error reading primary input");
		if (conv) {
			st = store;
			pifr = ifr;
			for (i=0;i<r;i++) {
				for (j=0;j<cb;j++) {
					*st++ = *pifr++;
				}
				st += ocb-cb;
			}
		}
		fcol = stcol(frame);
		frow = strow(frame);
		if (binary) {
			Ik_windowdma(fcol,ocb,IK_WDHXY_ADDR);
			i = HI_WORDXYH(stcard(frame),stplane(frame),
				(fcol>>5),frow);
			j = LO_WORDXYH(stcard(frame),stplane(frame),
				(fcol>>5),frow);
			Ik_dmawr8(IK_WD_ADDR,i,j,store,outrcb);
		}
		else {
			Ik_windowdma(fcol,ocb,IK_HXY_ADDR);
			Ik_dmawr8(IK_HXY_ADDR,fcol,frow,store,outrcb);
		}
	}
	if (twin) {
		for (frame=0;frame<f;frame++) {
			if (fread(ifr,rcb*sizeof(char),1,fp)!=1)
				perr(HE_MSG,"error reading primary input");
			if (conv) {
				st = store;
				pifr = ifr;
				for (i=0;i<r;i++) {
					for (j=0;j<cb;j++) {
						*st++ = *pifr++;
					}
					st += ocb-cb;
				}
			}
			fcol = stcol(frame) + outc;
			frow = strow(frame);
			if (binary) {
				Ik_windowdma(fcol,ocb,IK_WDHXY_ADDR);
				i = HI_WORDXYH(stcard(frame),stplane(frame),
					(fcol>>5),frow);
				j = LO_WORDXYH(stcard(frame),stplane(frame),
					(fcol>>5),frow);
				Ik_dmawr8(IK_WD_ADDR,i,j,store,outrcb);
			}
			else {
				Ik_windowdma(fcol,ocb,IK_HXY_ADDR);
				Ik_dmawr8(IK_HXY_ADDR,fcol,frow,store,outrcb);
			}
		}
		fclose(fp);
	}

	Ik_set_mode(SET_32_BIT_MODE);
	if (lapse == 0) {
		fscanf(SEQF,"%d",&nctrl);
		Ik_diowr(IK_WD_ADDR,IK_SCRATCHPAD,0,nctrl);
		for (i=0;i<nctrl;i++) {
			if (fscanf(SEQF,"%d %d",&ctrlf,&ctrll)==EOF)
			    perr(HE_MSG,
				"unexpected end of file in -l control file");
			Ik_diowr(IK_WD_ADDR,IK_SCRATCHPAD+((1+2*i)>>10),
				1+2*i,ctrlf);
			Ik_diowr(IK_WD_ADDR,IK_SCRATCHPAD+((2+2*i)>>10),
				2+2*i,ctrll);
		}
	}
	if (recsw) {
		ii=0177377;	/* put betamax in motion */
		fd2=open("/dev/dr",2);lseek(fd2,(long) 2,0); write(fd2,&ii,2);
		Ik_delay(60);
		ii=0177777; lseek(fd2,(long) 2,0);write(fd2,&ii,2); close(fd2);
	}
	Ik_close();

	sprintf(sys,"bps_viewport -X %d %d %d %d %d %d %d %d %d %d %d %d %d \
		%d %d %d %d %d %d\n",
	    cuesw,cueptr,ir,ic,outr,movoutc,binary,f,extras,blanksw,
	    single,lapse,RGB10(rg,gg,bg),RGB10(r0,g0,b0),
	    RGB10(r1,g1,b1),RGB10(rq,gq,bq),videosw,yzoom,xzoom);
	fprintf(stderr,"bpsmovie:  running\n	%s\n",sys);
	system(sys);

	exit(0);
}
