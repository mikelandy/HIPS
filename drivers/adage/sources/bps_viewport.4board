/* This is the 4 board version of bps_viewport, for systems with 4 DR256
	memory boards */
#define FBC (0xc00000)
#define LUVO (0x830000)
#define SRH 130
#define CXBAR (0x830400)
#define	MSTART (0x020000)	/* SR8 offset from MCM */

int cuesw,cueptr,r,c,nr,nc,binary,f,extras,blanksw,osingle,lapse;
int colorg,color0,color1,colorq,videosw,yz,xz;

int xvl,xvs,xwl,yvl,yvs,ywl,fbc0,fbc1,fbc2,fbc3;
int plane,frame,x,y,numf,i,single,half,bit;
int c1,c2,w1,r1,r2,r3,h1,numfrow,numfcol,numfcl2,numfrc,offset;
int *ctrl,*octrl,nctrl,cframe,cnumf,olapse,inited,fbc2yonly,*cmap,*pcmap;

main(argc,argv)

int argc;
char **argv;

{
	inited = 0;
	octrl = (int *) MSTART;
	nctrl = *octrl++;
	cuesw=atoi(argv[1]);
	cueptr=atoi(argv[2]);
	r=atoi(argv[3]);
	c=atoi(argv[4]);
	nr=atoi(argv[5]);
	nc=atoi(argv[6]);
	binary=atoi(argv[7]);
	f=atoi(argv[8]);
	extras=atoi(argv[9]);
	blanksw=atoi(argv[10]);
	osingle=atoi(argv[11]);
	olapse=lapse=atoi(argv[12]);
	if (olapse) {
		cmap = (int *) MSTART;
		offset = 0;
	}
	else {
		cmap = ((int *) MSTART) +2*nctrl+5;
		offset = 2*nctrl+5;
	}
	colorg=atoi(argv[13]);
	color0=atoi(argv[14]);
	color1=atoi(argv[15]);
	colorq=atoi(argv[16]);
	videosw=atoi(argv[17]);
	yz=atoi(argv[18]);
	xz=atoi(argv[19]);
	single = osingle;
	numfrow = 2048/nr;
	numfcol = binary ? ((1024/nc)*2) : (2048/nc);
	numfcl2 = numfcol/2;
	numfrc = numfrow*numfcol;
	if (binary)
		wr(LUVO,colorg);

	/*
	 * If 30Hz/Hi Res, default to a zoom of 2 (since that is how we fake
	 * 512 pixels, the pixel clock can't be slowed down sufficiently)
	 */

	if (videosw) {
		c *= 2;
		xz *= 2;

	}
	wr(CXBAR,077);
	while (!vblank())
		;
	while (vblank())
		;
	if (cuesw) {
		frame = cueptr;
		compport(colorq);
		setport();
		delay(cuesw);
		frame = f;
		compport(colorq);
		setport();
		delay(cuesw);
	}

	do {
		numf = f;
		if (blanksw && single == 1)
			numf++;
		cnumf = olapse ? numf : nctrl;
		ctrl = octrl;
		for (cframe=0;cframe<cnumf;cframe++) {
			if (olapse)
				frame = cframe;
			else {
				frame = *ctrl++;
				lapse = *ctrl++;
			}
			compport(color1);
			setport();
			if (frame == 0) {
				for (i=0;i<extras;i++) {
					delay(lapse);
					while (!vblank())
						;
				}
			}
			delay(lapse);
		}
	} while (--single != 0);
}

compport(color)

int color;

{
	y = ((frame % numfrc) / numfcol) * nr;
	if (binary)
		x = (((frame % numfcol) < numfcl2) ? 0 : 1024) +
			((frame % numfcl2) * nc);
	else
		x = (frame % numfcol) * nc;
	plane = frame / numfrc;

	/*
	 * First compute the x viewport
	 */

	c1 = c & ~03;
	if (xz>1 && c1<=xz)
		c1 = (xz+1) & ~03;
	if (c1 == 0)
		xwl = x;
	else
		xwl = x - (((c1+xz-2)/xz) + 1);
	if (x==xwl)
		c2 = 0;
	else
		c2 = 1 + (x-xwl-1)*xz;
	xvl = c2 & ~03;
	w1 = xz*nc + (c2 - xvl);
	xvs = (w1 & ~03) - 4;

	/*
	 * Now we deal with the y viewport.  Here, the difference between 30Hz
	 * and 60Hz video is more crucial, and we simplify things by treating
	 * them as separate cases.
	 */

	if (videosw) {
		r1 = r & ~03;
		if (r1 < yz-1)
			r1 = (yz + 2) & ~03;
		ywl = y - (((r1+28)/yz) + 1);
		r2 = (y-ywl)*yz - 29;
		r3 = (r2 + 3) & ~03;
		yvl = r3 + 32;
		h1 = nr*yz - (r3 - r2);
		yvs = (h1 & ~03) - 4;
	}
	else {
		r1 = r & ~01;
		if (r1 < yz-1)
			r1 = yz & ~01;
		ywl = y - (((r1+30)/yz) + 1);
		r2 = (y-ywl)*yz - 31;
		r3 = (r2 + 1) & ~01;
		yvl = r3*2 + 64;
		h1 = nr*yz - (r3 - r2);
		yvs = (h1 & ~01)*2 - 4;
	}

	fbc0 = yvl<<16 | (xvl & 0xffff);
	fbc1 = yvs<<16 | (xvs & 0xffff);
	if (inited == 0)
		fbc2yonly = ywl<<16 | (xwl & 0xffff);
	else
		fbc2yonly = ywl<<16 | (fbc2 & 0xffff);
	fbc2 = ywl<<16 | (xwl & 0xffff);
	fbc3 = (yz-1)<<16 | ((xz-1) & 0xffff);
	if (binary) {
		bit = 1 << plane;
		pcmap = cmap;
		for (i=0;i<256;i++)
			*pcmap++ = (i & bit) ? color : color0;
	}
	return;
}

delay(n)

int n;

{
	while(vblank())
		;
	while (--n) {
		while(!vblank())
			;
		while(vblank())
			;
	}
}

setport()

{
	if (vblank()) {
		*octrl = 0xf0f0f0f0;
		while(1)
			;
	}
	wr(FBC+2,fbc2yonly);
	while (!vblank())
			;
	wr(FBC+2,fbc2);
	if (inited == 0) {
		wr(FBC,fbc0);
		wr(FBC+1,fbc1);
		wr(FBC+3,fbc3);
		wr(CXBAR,0); /* PSEUDO-RED */
		inited++;
	}
	if (binary)
		cp256((SRH << 16) | offset,LUVO);
}
