/*
  cc -o grimcolor  -O  grimcolor.c -lsuntool -lsunwindow -lpixrect
  exit
*/

#include <stdio.h>
#include <suntool/sunview.h>
#include <suntool/canvas.h>
#include <sunwindow/cms.h>

#define OK  0
#define ERR 1

void repaint_canvas();
void quit_proc();
void cmap_proc();

FILE *fopen();

#define CAN_HEIGHT 40
#define CMS_NAME   "grimcolor"
#define CMS_SIZE   256

#define GCMS0	  10
#define GCMS127	  GCMS0+127
#define GCMS_SIZE 128+2

#define FCOLOR	GCMS0-1
#define BCOLOR	GCMS0-2
#define BOM	GCMS0-2	/* Bottom Of (Grimcolor) Map */
#define BOGM	GCMS0	/* Bottom Of  Grayscale  Map */
#define TOGM	GCMS127	/* Top Of Grayscale Map */

int invertgraymap = 0;
int grimFGcolor, grimBGcolor;
unsigned char red[CMS_SIZE], green[CMS_SIZE], blue[CMS_SIZE]; 

char fname[80];
char lable1[80] = { 'g','r','i','m','c','o','l','o','r',':' ,' ' };
char lable2[80];
char clear_line[] = { '\r',
' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
'\r', 0
};

Pixwin	*canvas_pw;
Frame	base_frame;
Canvas	canvas;

main(argc, argv)
int argc;
char **argv;
{
	base_frame = window_create(NULL, FRAME,
		FRAME_ARGS,		argc, argv,
		FRAME_LABEL,		lable1,
		FRAME_DONE_PROC,	quit_proc,
		FRAME_INHERIT_COLORS,	FALSE,
		0);

	canvas = window_create(base_frame, CANVAS,
		WIN_HEIGHT,		CAN_HEIGHT,
		WIN_WIDTH,		2*CMS_SIZE,
		CANVAS_RETAINED,	FALSE,
		CANVAS_REPAINT_PROC,	repaint_canvas,
		WIN_CONSUME_KBD_EVENT,	WIN_ASCII_EVENTS,
		WIN_CONSUME_PICK_EVENT, WIN_MOUSE_BUTTONS,
		WIN_CONSUME_PICK_EVENT, WIN_IN_TRANSIT_EVENTS,
		WIN_CONSUME_PICK_EVENT, LOC_DRAG,
		WIN_CONSUME_PICK_EVENT, LOC_STILL,
		WIN_EVENT_PROC,		cmap_proc,
		0);

	window_fit(base_frame);
	canvas_pw = canvas_pixwin(canvas);

	grimBGcolor = FCOLOR;
	grimFGcolor = BCOLOR;

	graycolormap(canvas_pw, 0, 1);
	
	repaint_canvas(canvas, canvas_pw);

	signal(SIGQUIT, quit_proc);
	window_main_loop(base_frame);
}

void updatecolormap()
{
  red[BCOLOR]= green[BCOLOR]= blue[BCOLOR]= grimBGcolor;
  red[FCOLOR]= green[FCOLOR]= blue[FCOLOR]= grimFGcolor;
  pw_putcolormap(canvas_pw, BOM, GCMS_SIZE, red, green, blue);
}

void repaint_canvas(canvas, pw)
Canvas	 canvas;
Pixwin	 *pw;
{
	register int i;

	window_set(base_frame, FRAME_LABEL, lable1, 0);
	/* Draw ramp of colors */
	for(i=0; i<CMS_SIZE; i++)
		pw_rop(canvas_pw,
			i*2, 0, 2, CAN_HEIGHT, PIX_SRC|PIX_COLOR(i),0,0,0);
}

graycolormap(pw, sc, dc)
Pixwin *pw;
register int sc, dc;
{
	register i;

	dc = dc*2;
	pw_setcmsname(pw, CMS_NAME);
	for (i=TOGM; i<=TOGM; i++) {
		red[i] = green[i] = blue[i] = sc;
		sc += dc;
		if(sc<0) sc=0;
	}
	updatecolormap();
}

/************************************************************************/
#define RTNCMD 0x4000
static char mbuf[80+1], *mbP;
static int  l2n;
static int  rtncmd = 0;
static int  leadingspace = 1;

static int ms_left, ms_middle, ms_right;
static int c_start = 0;
static int c_repeat = 16;
static int c_gap = 1;

void cmap_proc(canvas, event)
Canvas canvas;
Event *event;
{
	register int c;
	int n, t;

	l2n = strlen(lable2);
	if(l2n > 80) l2n = 0;

	c = event_id(event);
	if(c > META_LAST){
		ms_left   = (int)window_get(canvas, WIN_EVENT_STATE, MS_LEFT);
		ms_middle = (int)window_get(canvas, WIN_EVENT_STATE, MS_MIDDLE);
		ms_right  = (int)window_get(canvas, WIN_EVENT_STATE, MS_RIGHT);
	}

	if( (c==LOC_DRAG&&ms_left) || (c==MS_LEFT&&event_is_down(event)) ){
		c_start = event_x(event)>>1;
		op0_c();
		if(c_start<=BOGM) c_start = BOGM;
		if(c_start>=TOGM) c_start = TOGM;
		cbarmap(canvas_pw, c_start, c_gap, c_repeat);
		return;
	}

	if(c>ASCII_LAST || event_is_up(event) )
		return;

	if(rtncmd){
		if(c!='\r' && c!='\n' && mbP<(mbuf+80)){
			if(c=='\b'){
				mbP--;
			}else
				if( !(c==' ' && leadingspace) ){
					*mbP++ = c;
					leadingspace = 0;
				}

			if((l2n+1) > 80) l2n = 0;
			sprintf(&lable2[l2n], "%c", c);
			window_set(base_frame, FRAME_LABEL, lable2, 0);

			return;
		}
	
		c = rtncmd;
		*mbP = '\0';
	}
	mbP = mbuf;
	rtncmd = 0;
	leadingspace = 1;

	switch(c){
	case 'c': op0_c();		goto L_setrtn0;
	case 'l': op0_l();		goto L_setrtn0;
	case 'q': op0_q();		goto L_setrtn0;
	case '>': op0_rt_arrow();	goto L_setrtn0;

    L_setrtn0:
		rtncmd = c+RTNCMD;
		return;

	case 'c'+RTNCMD: op1_c(mbuf);		break;
	case 'l'+RTNCMD: op1_l(mbuf);		break;
	case 'q'+RTNCMD: op1_q(mbuf);		break;
	case '>'+RTNCMD: op1_rt_arrow(mbuf);	break;

	case 'I':		/* interchange foreground & background */
		t = grimFGcolor;  grimFGcolor = grimBGcolor;  grimBGcolor = t;  
		updatecolormap();
		pw_setcmsname(canvas_pw, CMS_NAME);
		break;
	case 'i':		/* invert gray map */
		if(invertgraymap){
			invertgraymap = 0;
			graycolormap(canvas_pw, 0, 1);
		}else{
			invertgraymap = 1;
			graycolormap(canvas_pw, 255, -1);
		}
		break;
	case '?':
		help();
		break;
	}
}

op0_l()					/* load colormap file */
{
	sprintf(&lable2[0], "Enter file %s : ", fname);
	window_set(base_frame, FRAME_LABEL, lable2, 0);
}
op1_l(mbuf)				/* load picture cmd */
char *mbuf;
{
	FILE *fpr, *fopen();

	if(mbuf[0]!=0)
		sprintf(&fname[0], "%s", mbuf);
	if(fname[0]==0)
		return;

	if((fpr=fopen(fname, "r"))==NULL) {
		sprintf(&lable2[0], " Can't open %s :\n", fname);
		window_set(base_frame, FRAME_LABEL, lable2, 0);
		return;
	}

	setcolormap(fpr, canvas_pw);
	fclose(fpr);

	sprintf(&lable1[11], "file name = %s", fname);
	window_set(base_frame, FRAME_LABEL, lable1, 0);
}

op0_c()				/* cbar */
{
	sprintf(&lable2[0], "cbar st=%d wd=%d gap=%d : ",
		c_start-GCMS0, c_repeat, c_gap);
	window_set(base_frame, FRAME_LABEL, lable2, 0);
}
op1_c(mbuf)
char *mbuf;
{
	if(mbuf[0]!=0)
		sscanf(mbuf, "%d %d %d", &c_start, &c_repeat, &c_gap);
	if(c_start<=BOGM) c_start = BOGM;
	if(c_start>=TOGM) c_start = TOGM;
	cbarmap(canvas_pw, c_start, c_gap, c_repeat);
	op0_c();
}

int
setcolormap(fpr, pw)
FILE *fpr;
Pixwin *pw;
{	
	register int i, swG, swR, swB, ch;
	int start;
	short tk;


/*		FORMAT of colormap data file
 * rgbs0		r:red   g:green   b:blue   s:start adr
 * 1 2 3 .....		data up to 256 bytes
 * e			end
 */
	while(1){
		swG = swR = swB = 0;
		start = 0;

		do{
			ch = getc(fpr);
			switch(ch){
			case 'r': case 'R': swR = ch; break;
			case 'g': case 'G': swG = ch; break;
			case 'b': case 'B': swB = ch; break;
			case 'e': case 'E': goto L_end; 
			case EOF: return(ERR);
			}
		}while(ch!='s');

		fscanf(fpr, "%d", &start);
/*printf("\n%c%c%c%cs%d \n", swG, swR, swB, start); /*debug*/
		if(start<BOGM) start = BOGM;
		if(start>TOGM) start = TOGM;

		for (i=start; i<=TOGM; i++) {
			if(fscanf(fpr, "%hd", &tk)!=1)
				break;
			if(swR) red[i]   = tk;
			if(swG) green[i] = tk;
			if(swB) blue[i]  = tk;
		}
	}

    L_end:
	pw_setcmsname(pw, CMS_NAME);
	updatecolormap();
	return(OK);
}


/* 0:violet, 1:blue, 2:cyan, 3:green, 4:yellow, 5:orange, 6:red, 7:magenta */

			/*     0    1    2    3    4    5    6    7	   */
static unsigned char R[8] = { 128,  0 ,  0 ,  0 , 255, 255, 255, 255};
static unsigned char G[8] = {  0 ,  0 , 255, 255, 255, 128,  0 ,  0 };
static unsigned char B[8] = { 178, 255, 255,  0 ,  0 ,  0 ,  0 , 255};

cbarmap(pw, start, gap, repeat)
Pixwin *pw;
{
	register int	i, j, k;
   	unsigned char	lR, lG, lB;

	if(invertgraymap)
		graycolormap(canvas_pw, 255, -1);
	else
		graycolormap(canvas_pw, 0, 1);

	if(start<BOGM) start = BOGM;
	if(start>TOGM) start = TOGM;
	i = start;
	for(k=0; k<8; k++){
		lR = R[k]; lG = G[k]; lB = B[k];
		j = repeat;
		while(--j>=0){
			red[i] = lR; green[i] = lG; blue[i] = lB;
			i += gap;
			if(i>TOGM)
				goto L_end;
		}
	}
    L_end:
	pw_setcmsname(pw, CMS_NAME);
	updatecolormap();
}

op0_rt_arrow()				/* > */
{			/* change directory from grimtool */
	sprintf(&lable2[0], "chdir<Enter new dirname> ? ");
	window_set(base_frame, FRAME_LABEL, lable2, 0);
}

static int  op1_rt_beenhere = 0;
static FILE *GCfpr;
static char *GCfnameP;

op1_rt_arrow(mbuf)
char *mbuf;
{
	char cmdstr[80], *mktemp();

	if(!op1_rt_beenhere){
		GCfnameP = mktemp("/tmp/GCXXXXXX");
		if((GCfpr=fopen(GCfnameP, "w"))==NULL) {
			printf("Grimcolor can't open %s\n", GCfnameP);
			return;
		}	
		op1_rt_beenhere = 1;
		fclose(GCfpr);
	}

	if(mbuf[0]!=0){
		if((GCfpr=fopen(GCfnameP, "w+r"))==NULL) {
			printf("Grimcolor can't open %s\n", GCfnameP);
			return;
		}	
		chdir(mbuf);
		sprintf(&cmdstr[0], "pwd > %s", GCfnameP);
			/* printf("cmdstr= %s\n", cmdstr); */
		system(cmdstr);
		rewind(GCfpr);
	}else{
		if((GCfpr=fopen(GCfnameP, "r"))==NULL) {
			printf("Grimcolor can't open %s\n", GCfnameP);
			return;
		}
	}	
	sprintf(&lable2[0], "%s", clear_line);
	sprintf(&lable2[0], "pwd: ");
	fread(&lable2[5], sizeof(*lable2), 80-5, GCfpr);
	window_set(base_frame, FRAME_LABEL, lable2, 0);
		/* printf("lable2= %s\n", lable2); fflush(stdout); */
	fclose(GCfpr);
}

op0_q()				/* quit */
{
	sprintf(&lable2[0], "Type 'y' return to Confirm quit: ");
	window_set(base_frame, FRAME_LABEL, lable2, 0);
}
op1_q(mbuf)			/* quit */
char *mbuf;
{
	if(mbuf[0]=='y'){
		window_set(base_frame, FRAME_NO_CONFIRM, TRUE, 0);
		quit_proc();
		fclose(GCfpr);
	}
	window_set(base_frame, FRAME_LABEL, lable1, 0);
}
void
quit_proc()
{
	window_destroy(base_frame);
	exit(0);
}


help()
{
  printf("\n\n");
  printf("\t\t\tGRIMCOLOR Commands:\n");
/*
  printf("\tB #     = set Background to #.\n");
  printf("\tF #     = set Foreground to #.\n");
  printf("\t              If #==-1  disable interchange of fore/background.\n");
*/
  printf("\tI       = Interchange foreground & background.\n");
  printf("\ti       = Invert gray-scale map.\n");
  printf("\tl name  = Load color map file \"name\".\n");
  printf("\tc s w g = generate Color bar start at s, with width w, & gap g.\n");
  printf("\t>       = execute chdir system cmd.\n");
  printf("\tq       = Quit.\n");
  printf("\n\n");
}
