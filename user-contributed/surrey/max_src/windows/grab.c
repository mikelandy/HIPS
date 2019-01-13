/**************************************************************************
 *                                                                        * 
 * Function: grab()			          		                        *
 *                                                                        *
 * Usage:	   grab                   		                             *
 * Returns:  none							                        *
 * Defaults: filename ~/fgpic	 frame size 512x512                         *
 * Loads: cc -o -DDG grab grab.c -ldglib.a -lfslib.a -lmxlib.a -lsuntool  *
 *            -lsunwindow -lpixrect -Imaxvideo/include -Ilocaldir/include *
 * Modified: TK 21-VIII-87                                                *
 *                                                                        *
 * Description:A window tool for the Datacube system, allowing grab  fs0, *
 *             write to file and write to framestore.                     *
 *             No HIPS header created for file.File size standard 512x512.*
 **************************************************************************
 *                    Copyright (c) 1987                                  *
 *                    Captain Chaos                                       *
 **************************************************************************
 */
#include <fsHead.h>
#include <dgHead.h>
#include <suntool/sunview.h>
#include <suntool/canvas.h>
#include <suntool/panel.h>
#include <pixrect/pixrect_hs.h>
#include <maxdefs.h>

/* these objects are global so thier attributes may be retrieved or modified */
Frame frame;
Canvas canvas;
Panel panel;
Pixwin *pw;
Panel_item namestring;

FS_DESC *fsfd;	/* FRAMESTORE device descriptor placed here to allow*/
DG_DESC *dgfd;	/* DIGIMAX device descriptor   subfunctions to access*/

main()
{
    char opt;		/* user option */
    int verbose;	/* verbosity flag - set for printf's */
    int i;
    unsigned char red[256], green[256], blue[256];
    int grab_proc(), put_proc(), disp_proc();

    /* initialize parameters */
    verbose = 0;

    /* Allocate memory for the boards' register structures */
    fsfd = fsOpen(FS_BASE, verbose);
    dgfd = dgOpen(DG_BASE, verbose);f(stderr, "cant put picture\n");
		fflush(stderr);
		return;}

    
    /* initialize all boards */
    myfsInit(fsfd, FS_T50);
    dgInit(dgfd, DG_UNSGD);	/* unsigned lut's */
    
    /* clear the 3 framestores using the constant register set to 0 */
    fsFastClear(fsfd, FS_FS0, BLACK);
    fsFastClear(fsfd, FS_FS1, BLACK);
    fsFastClear(fsfd, FS_FS2, BLACK);

    /* now build a panel, with canvas 512x512, and a button for capture
       +show*/

    	frame=window_create(NULL, FRAME, FRAME_LABEL, "datacube panel",0);
	panel=window_create(frame, PANEL, WIN_HEIGHT, 100, 0);
	panel_create_item(panel, PANEL_BUTTON, PANEL_NOTIFY_PROC, grab_proc,
			PANEL_LABEL_IMAGE, panel_button_image(panel,
			"grab", 0, 0), 0);
	panel_create_item(panel, PANEL_BUTTON, PANEL_NOTIFY_PROC, put_proc,
			PANEL_LABEL_IMAGE, panel_button_image(panel,
			"put", 0, 0), 0);
	panel_create_item(panel, PANEL_BUTTON, PANEL_NOTIFY_PROC, disp_proc,
			PANEL_LABEL_IMAGE, panel_button_image(panel,
			"disp", 0, 0), 0);
	namestring=panel_create_item(panel, PANEL_TEXT, PANEL_LABEL_STRING,
				"Filename:", PANEL_VALUE, "~/fgpic", 0);
	window_set(panel, PANEL_CARET_ITEM, namestring, 0);
	canvas=window_create(frame, CANVAS, WIN_X, 0, WIN_Y, 100,
			CANVAS_WIDTH, 512,
			CANVAS_HEIGHT, 512, CANVAS_RETAINED, FALSE, 0);
	pw=canvas_pixwin(canvas);
        for (i=0;i<256;++i)
		red[i]=green[i]=blue[i]=(unsigned char) i;
 	pw_setcmsname(pw, "greylevel");
	pw_putcolormap(pw, 0, 256, red, green, blue);
 	window_set(canvas, CANVAS_RETAINED, TRUE, WIN_VERTICAL_SCROLLBAR,
			scrollbar_create(0), WIN_HORIZONTAL_SCROLLBAR, 
			scrollbar_create(0),0);
	fprintf (stderr, "Made canvas now; where is it\n");
	fflush (stderr);
	window_main_loop(frame);
	exit(0);}

grab_proc(item, event)
Panel_item item;
Event *event;
{short *tptr;
struct pixrect *tpr;
int op;

	fs0Acquire(fsfd);
	fs0WaitFrm(fsfd);
	fs0Freeze(fsfd);
	/*and scribble fs0 over pw*/	
	/* this may be nastier than it sounds */
	tptr=(short *) fsfd->fs0_base;
	tpr=mem_point(512, 512, 1, tptr);
 	op=PIX_SRC ;
 	pw_rop(pw, 0, 0, 512, 512, op, tpr, 0, 0);
	}

put_proc(item, event)
Panel_item item;
Event *event;
{char filename[80];
 strcpy(filename, (char *) panel_get_value(namestring));
 putpic (fsfd->fs0_base, filename);}

putpic(pic, string)
unsigned char *pic;
char *string;
{FILE *fopen(), *fp;
int i, j, jj;

	if ((fp=fopen(string, "w"))==NULL)
		{fprintf(stderr, "cant put picture\n");
		fflush(stderr);
		return;}
	putc(0x01, fp);
	putc(0x00, fp);
	putc(0x08, fp);
	putc(0x00, fp);
	putc(0x00, fp);
	putc(0x02, fp);
	putc(0x00, fp);
	putc(0x02, fp);
	for (i=0;i<512;++i)
		{jj=i*512;
			fwrite((pic+jj), 1, 512, fp);}
	fclose(fp);}

disp_proc(item, event)
Panel_item item;
Event *event;
{char filename[80];
 strcpy(filename, (char *) panel_get_value(namestring));
 disppic (fsfd->fs0_base, filename);}

disppic(pic, string)
unsigned char *pic;
char *string;
{FILE *fopen(), *fp;
int i, j, jj;

	if ((fp=fopen(string, "r"))==NULL)
		{fprintf(stderr, "cant read picture\n");
		fflush(stderr);
		return;}
	for (i=0;i<8;i++)
                { getc(fp); }
	for (i=0;i<512;++i)
		{jj=i*512;
			fread((pic+jj), 1, 512, fp);}
	fclose(fp);}

myfsInit(fsfd, timing)
FS_DESC *fsfd;
int timing;
{


/*************************************************
	what we want here is:
	50 Hz
	IL
	P5 - fs0
        fs0 in - P10
        fs1 in - P4
        fs2 in - P9
      
*/
	
    fs0FrmMd(fsfd);		/* frame mode for fs0 */
    fs12FrmMd(fsfd);		/* frame mode for fs1 and fs2 */
    fsIntlace(fsfd);		/* interlace mode */
    if (timing == FS_T50)
	fs50HzMd(fsfd);		/* 50 Hz mode */
    else
	fs60HzMd(fsfd);		/* 60 Hz mode */
    fs0AcqField(fsfd, 0);	/* acquire next field for fs0 */
    fs0NormTim(fsfd);		/* normal timing for fs0 */
    fs0Freeze(fsfd);		/* no acquire mode for fs0 */
    fs12AcqField(fsfd, 0);	/* acquire next field for fs1 and fs2 */
    fs12NormTim(fsfd);		/* normal timing for fs1 and fs2 */
    fs12Freeze(fsfd);		/* no acquire mode for fs1 and fs2 */
    fsSel0Input(fsfd, FS_0P10);	/* select fs0 input as P10 */
    fsSel1Input(fsfd, FS_1P4);	/* select fs1 input as P4 */
    fsSel2Input(fsfd, FS_2P9);	/* select fs2 input as P9 */
    fsSelOutSrc(fsfd, 0);	/* select output source as fs0 */
    fs0HPipe(fsfd);		/* select pipeline for fs0 horizontal */
    fs0VPipe(fsfd);		/* select pipeline for fs0 vertical */
    fs12HPipe(fsfd);		/* select pipeline for fs1/2 horizontal */
    fs12VPipe(fsfd);		/* select pipeline for fs1/2 verrtical */
    fs12Separate(fsfd);		/* select byte mode for fs1 and fs2 */
    fs0WrEn(fsfd, 0);		/* set fs0 write enable to all enable */
    fsP10Main(fsfd);		/* select P10 main channel */
    fsP9Main(fsfd);		/* select P9 main channel */
    fsP4Main(fsfd);		/* select P4 main channel */
    fs1WrEn(fsfd, 0);		/* select fs1 write enable to all enable */
    fs2WrEn(fsfd, 0);		/* select fs2 write enable to all enable */
    fs0Pan(fsfd, 0);
    fs12Pan(fsfd, 0);
    fs0Scroll(fsfd, 0);
    fs12Scroll(fsfd, 0);
    return;
}
