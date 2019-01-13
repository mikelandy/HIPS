#import <appkit/appkit.h>
#include <hipl_format.h>
#include <stdio.h>

unsigned char *data;
int rows,cols;
struct header hdr;
char *Progname = "nhips";
int types[] = {PFBYTE,LASTTYPE};

int read_hdr_cpf(struct header *,int *);
int alloc_image(struct header *);
int read_image(struct header *,int);

void setUp(void)
{
	id	 myWindow, myMenu, myBitmap;
	NXRect	 aRect,bRect;
        NXPoint origin = {0.0,0.0};

        /* step 0; read the file */
        read_hdr_cpf(&hdr,types);
	alloc_image(&hdr);
	read_image(&hdr,0);
        rows=hdr.orows; cols=hdr.ocols;
        data=hdr.image;
        NXSetRect(&bRect, 0.0, 0.0, (NXCoord)cols, (NXCoord)rows);

	NXSetRect(&aRect, 0.0,0.0,cols,rows);
	myWindow = [Window newContent:&aRect
				style:NX_TITLEDSTYLE
				backing:NX_BUFFERED
				buttonMask:NX_ALLBUTTONS
				defer:YES];
	[myWindow setTitle:"nhips"];

        myBitmap = [Bitmap newSize:(NXCoord)cols:(NXCoord)rows
                           type:NX_UNIQUEBITMAP];
        [myBitmap image:data
                  width:cols
                  height:rows
                  bps:8
                  spp:1];

	myMenu = [Menu newTitle:"nhips"];
	[myMenu addItem:"Quit"
			action:@selector(terminate:)
			keyEquivalent:'q'];
	[myMenu sizeToFit];
	[NXApp setMainMenu:myMenu];

	[myWindow display];
	[myWindow orderFront:nil];
        [[myWindow contentView] lockFocus];
        [myBitmap composite:NX_SOVER fromRect:&bRect toPoint:&origin];
        [[myWindow contentView] unlockFocus];
        [myWindow flushWindow];
}


main()
{
	[Application new];
	setUp();
	[NXApp run];
	[NXApp free];
}
