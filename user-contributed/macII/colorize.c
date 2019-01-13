#include <MacTypes.h>
#include <QuickDraw.h>
#include <EventMgr.h>
#include <WindowMgr.h>
#include <FontMgr.h>
#include <ScrapMgr.h>
#include <DialogMgr.h>
#include <MemoryMgr.h>
#include <ColorToolbox.h>
#include <stdio.h>



/*	See Technical Note #120
 *
 *	new is the index of the hips window array.
 *	this routine draws the data into an offscreen pixmap, so that all of the data can be
 *	then copies to a Picture;
 *  The rooutine checks that there is some intersection between where we want to draw and
 *	the window where it will be copied to. This is unncessary since the two rects are identical.
 *	
 */
CreatePicture( fpin, new )
FILE *fpin;
int new;
{
	int offLeft, offTop, offRight, offBottom;
	
	long offRowBytes;
	long sizeOfOff;
	char *myBits;
	Rect destRect;
	Rect globRect;
	Rect bRect;
	int theDepth;
	int	i, err;
	CGrafPort myCGrafPort;
	CGrafPtr myCGrafPtr;
	CTabHandle ourCMHandle;
	GDHandle theMaxDevice;
	GDHandle oldDevice;
	
	WindowPtr myCWindow;
	Rect bounds;
	
	
	offTop = 0;
	offLeft = 0;
	offBottom = hd[new].orows;
	offRight = hd[new].ocols;
	
	myCWindow = hips_wind_ptr[new];
	
	SetPort( myCWindow );
	SetRect( &bRect, offLeft, offTop, offRight, offBottom );

	if ( SectRect( &(myCWindow->portRect), &bRect, &globRect ) == false )
	{
		SysBeep(10);
		fprintf(info, "No intersection \n");
		return;
	}
	
	LocalToGlobal( & topLeft ( globRect )  );
	LocalToGlobal( & botRight ( globRect )  );

	
	/*globRect is the rect of where the image will be copied into (in the color window),
	so we want to know which device has most of that window in it  */
	
	theMaxDevice = GetMaxDevice( &globRect );		/*get the maxDevice*/

	oldDevice = GetGDevice();						/*save theGDevice for restoring later*/
	SetGDevice( theMaxDevice );


	myCGrafPtr = &myCGrafPort;						/*open a new color port */
	OpenCPort( myCGrafPtr );
	
	
	theDepth =   (  **(myCGrafPtr->portPixMap)   ).pixelSize;
	
	/*now calculate the size of the pixel image that we need */

	offRowBytes = ((((theDepth * (offRight-offLeft)) + 15)) / 16 ) * 2 ; /*will be a word boundary*/
	
	sizeOfOff = (long) ( (long)offBottom - (long)offTop ) * offRowBytes ;
	OffsetRect( &bRect, -offLeft, -offTop );	/*adjust local coordinates*/
	
	/*set up base addr, row bytes, bounds, and pixel size of the PixMap in our new CPort */
	myBits = NewPtr( sizeOfOff );				/*allocate space */
		
	if ( myBits == NULL )
	{
		fprintf(info,"can't allocate memory for the off screen pixel map \n");
		fprintf(info,"was asking for %ld bytes \n", sizeOfOff);
		SysBeep(10);
		sleep(5);
		exit(1);
	}
	
	( **(myCGrafPtr->portPixMap)  ).baseAddr = myBits;
	( **(myCGrafPtr->portPixMap)  ).rowBytes = offRowBytes + 0x8000;	/*distinguish from a bitmap*/
	( **(myCGrafPtr->portPixMap)  ).bounds = bRect;
	
	/*now clone the color table of maxDevice so we can put it in our offscreen pixmap (in case
	the offscreen pix maps color table is diffrent), we also
	need to convert it from a color table for a device, to a color table for a pixmap (in
	case we are CopyBits-ing into a picture */
	
	ourCMHandle = (  **(**theMaxDevice).gdPMap   ).pmTable;
	err = HandToHand( &ourCMHandle  );
	
	if ( err != NULL )
	{
		fprintf(info, "Error in HandToHand()  \n");
		SysBeep(10);
	}
	
	/*now convert from device color table to pixmap color table */
	
	for ( i=0; i<= (**ourCMHandle).ctSize; i++ )
	{
		(**ourCMHandle).ctTable[i].value = i;			/*put in the indices */
	
		/*now clear the high bit of transindex to indicate it is a pixmap color table*/
		
		(**ourCMHandle).transIndex &= 0x7FFF;
	}
		
		
	(  **(myCGrafPtr->portPixMap)   ).pmTable = ourCMHandle; /*put the cloned color table into offscreen pixmap */
	
	SetPort( myCGrafPtr );		/*set port to the offscreen port */

	/*now draw whatever is needed in the offscreen port, and create a picture of it */
	DrawContents( fpin, new );

	/* clean up before exiting */
		
	SetPort( myCWindow );
	SetGDevice( oldDevice );
		
	/*clean up */
	CloseCPort( myCGrafPtr );
	DisposPtr( myBits );
	DisposHandle( ourCMHandle );		/*color map */
	
}

/*
 *	the port is already set to the offscreen port when this function is called
 *
 */
DrawContents(fpin, new)
FILE *fpin;
int new;
{
	Rect clip_rect, sorc_rect, dest_rect;
	int num_cols;
	int x,y;
	int pixel_value;
	RGBColor	color;
	Rect	picture_rect;
	char 	*scanLine;
	CGrafPtr	drawingPortPtr;
	long		index[256];

	char 		*bitPtr, *d;
	int 		rowBytes;
	int			i;
	
	long 		siz;

	
	GetPort( &drawingPortPtr );		/*find out where we are drawing */
	
	scanLine = calloc( hd[new].ocols, sizeof(char) );
	if (scanLine == NULL )
	{
		fprintf(info,"can't allocate memory for a scan line \n" );
		SysBeep(10);
		exit(1);
	}
	
	/*get a copy of the color table (shades of gray) */
	for ( i=0; i < 256; i++ )
	{
		color.red = color.green = color.blue = i << 8 ;
		index[i] = Color2Index( &color);
	}

	/*find out the address of the first byte of the port */
	bitPtr =   ( **(drawingPortPtr->portPixMap)  ).baseAddr;
	rowBytes = ( **(drawingPortPtr->portPixMap)  ).rowBytes  & 0x7FFF  ; /*high bit flags a pixel versus a bit */

	
	num_cols = hd[new].ocols;
	for (y = 0; y < hd[new].orows; y++) 
	{
		/*read a scan line */
		fread(scanLine,num_cols,1,fpin);

		d = bitPtr + (long)y * (long)rowBytes;
		
	    for (x = 0; x < hd[new].ocols; x++) 
	    {
	   		pixel_value =  0x00FF & (int)scanLine[x] ;
	   					
	   		*d++ = (char)index[pixel_value];
/*
			pixel_value =  0x00FF & (int)scanLine[x] ;
			color.red = color.green = color.blue = pixel_value << 8 ;
			SetCPixel( x,y, &color );
*/
		}
	} 

	picture_rect.top = 0;
	picture_rect.left = 0;
	picture_rect.bottom = hd[new].orows;
	picture_rect.right = hd[new].ocols;

	/* see IM I-187,188,o.w. can't offset and redraw the picture later */
	clip_rect = hips_wind_ptr[new]->portRect;
	ClipRect(&clip_rect);  
	
	sorc_rect = picture_rect;
	dest_rect = picture_rect;
	
	hippic[new] = OpenPicture( &dest_rect ); /*copy back into itself to get a picture of it */

	CopyBits(*( drawingPortPtr->portPixMap ),*( drawingPortPtr->portPixMap ), &sorc_rect, &dest_rect, 0, NULL );

	ClosePicture();

	/*undo effect of making a small clipping window for the OpenPicture() */
	SetRect(&clip_rect,-30000,-30000,30000, 30000);	 
	ClipRect(&clip_rect);
	
	free( scanLine );

	/* need to check for errors in picture creation. CopyBits() may have failed-----------
	siz = GetHandleSize( hippic[new] );
	if ( siz == ??? )			
	{
		SysBeep(10);
		fprintf(info,"Picture couldn't be formed due to not enough memory\n");
	}
	else
	{
	
	}
	------------ */
	
}

