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

#include "hipl_format.h" 

#define STRCLASS	
#include "globals.h"

FILE	*info;

HipsOpen( fname )
char *fname;
{
	InitHipsWindow(fname);

}

InitHipsWindow(fname)
char *fname;
{
	int 	i;
	Rect	bounds;
	FILE 	*fpin;
	int 	new;
	
	/*find a new available index for the window */
	for ( i=0; i<MAXHIPS; i++)
	{
		if	(usedIndex[i] == NULL )
			break;
	}
	if ( i != MAXHIPS )
	{
		new = i;
		usedIndex[i] = 1;
	}
	else
	{
		SysBeep(10);
		fprintf(info,"can't have more open pictures, please close some.\n");
		return;
	}
	
	/*open the input file */
	if ( (fpin = fopen(fname,"r")) == NULL) {
	    fprintf(info,"can't open for read\n");
	    exit(1);
	    }
	fread_header(fpin,&hd[new],fname); 

 	SetCursor( *watchCursHndl );

	bounds.top = 40 + new * 10 ;
	bounds.left = 40 + new * 10 ;
	bounds.bottom = bounds.top + hd[new].rows;
	bounds.right = bounds.left + hd[new].cols;
	
/*	sprintf(fname+strlen(fname),"  #[%d]", new ); */
	CtoPstr(fname); 
	hips_wind_ptr[new] = NewCWindow(NULL, &bounds, fname ,
					TRUE, documentProc+8, (WindowPtr)(-1L), TRUE, (long)new );

	RequestPalette( new );			/*attach a palette to the new window */
	
	CreatePicture( fpin, new );		/*draw to an offscreen pixmap, and create a picture */
		
	fclose (fpin );
	
	/*read coordinate transformations file, if present */
	PtoCstr(fname);
	ReadCoords(fname,new);

	SetCursor( &arrow );
}

/*
 *	
 */
HipsUpdate()
{	
	WindowRecord *w_ptr;
	long	index;
	int 	i;
	Rect 	orig_sorc_rect,dest_rect, temp_rect;
	double 	hscale, vscale; 		/*scale of current viewed pic to orig*/
	
	/*get the ptr to the window that is to be updated. This should be the current one */
	GetPort( &w_ptr );
	index = w_ptr->refCon;
	orig_sorc_rect = myCGrafPtr[index]->portRect;
	dest_rect = ((CGrafPtr)w_ptr)->portRect;
	
	SetCursor( *watchCursHndl );
	
	BeginUpdate( w_ptr );
	
	CopyBits(*( myCGrafPtr[index]->portPixMap ),*( ((CGrafPtr)w_ptr)->portPixMap ),
	 						&pic_sorc_rect[index], &dest_rect, 0, NULL );

	OriginalToWindowRect( index, w_ptr, &zoom_sorc_rect[index],  &temp_rect  );
	PenMode( srcXor );
	FrameRect( &temp_rect );
	
	EndUpdate( w_ptr );
	SetCursor( &arrow );

} 

HipsClose()
{
	WindowRecord *w_ptr;
	long	index;
	
	/*get the ptr to the window that is to be closed. This should be the current one */
	GetPort( &w_ptr );
	index = w_ptr->refCon;
	
	
	DisposePalette( pal_hndl[index] );
	CloseWindow (hips_wind_ptr[index]);
	
	CloseCPort( myCGrafPtr[index] );
	DisposPtr( myBits[index] );
	DisposHandle( ourCMHandle[index] );
	
	zoomisdrawn[index] = 0;
	SetRect( &zoom_sorc_rect[index], 0, 0, 0, 0 );
				
	usedIndex[index] = 0;
}

/*
 *	copy the complete picture in the top most window to the clipboard 
 */
HipsCopy()
{
	long 			pic_size;
	char 			*p;
	WindowRecord 	*w_ptr;
	long			index;
	PicHandle 		pHndl;
	
	/*get a pointer to the front most window */
	w_ptr = (WindowRecord *)FrontWindow();
	index = w_ptr->refCon;
	
	if ( index < 0  || index >= MAXHIPS)		/*it is not a picture window*/
	{
		SysBeep(10);
		fprintf(info,"Front most window must be a picture window to copy.\n" );
		return;
	}
	
	/*now form a picture of the complete image */
	
	/* see IM I-187,188,o.w. can't offset and redraw the picture later */
	SetPort( myCGrafPtr[index] );
	ClipRect( &( myCGrafPtr[index]->portRect ) );  
	
	pHndl = OpenPicture( &( myCGrafPtr[index]->portRect ) ); /*copy back into itself to get a picture of it */

	CopyBits(*( myCGrafPtr[index]->portPixMap ),*( myCGrafPtr[index]->portPixMap ), 
				 &( myCGrafPtr[index]->portRect ) , &( myCGrafPtr[index]->portRect ) , 0, NULL );

	ClosePicture();
	

	pic_size = GetHandleSize(pHndl);
	HLock( pHndl );
	p = (char *)* ( pHndl ) ;
	
	/*copy the complete picture to the clipboard */
	ZeroScrap();
	PutScrap( pic_size,'PICT',p );
	
	HUnlock( pHndl );
	DisposHandle( pHndl );
}


/*
 *	display header for the front window, modified version of HIPS stuff 
 */
/*	Copyright (c) 1982 Michael Landy, Yoav Cohen, and George Sperling

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */
HipsHeader()
{
	WindowRecord *w_ptr;
	long index;
	char fname[256];
	
	w_ptr = (WindowRecord *)FrontWindow();
	index = w_ptr->refCon;
	
	GetWTitle( w_ptr, fname );
	PtoCstr(fname);
	fprintf(info,"----------- Header HIPs file/window: %s -------------------------\n", fname );	
	if ( index < 0 || index >= MAXHIPS )
	{
		fprintf(info,"Front window must be a HIPs window to get header info.\n");
		return;
	}


	/*print the header that is in hd[index], this code is from see_header() */
	fprintf(info,"Original name:		%s\
Sequence name:		%s\
Number of frames:	%d\n\
Original date:		%s\
Number of rows:		%d\n\
Number of columns:	%d\n\",
		hd[index].orig_name,hd[index].seq_name,hd[index].num_frame,
		hd[index].orig_date,hd[index].rows,hd[index].cols);
	fprintf(info,"\nPixel format:		");
	switch(hd[index].pixel_format) {
		case PFBYTE:
			fprintf(info,"Bytes");
			break;
		case PFSHORT:
			fprintf(info,"Short integers");
			break;
		case PFINT:
			fprintf(info,"Integers");
			break;
		case PFFLOAT:
			fprintf(info,"Reals");
			break;
		case PFCOMPLEX:
			fprintf(info,"Complex numbers");
			break;
		case PFQUAD1:
			fprintf(info,"Quadtree");
			break;
		case PFBHIST:
			fprintf(info,"Byte image histogram");
			break;
		case PFSPAN:
			fprintf(info,"Spanning tree");
			break;
		case PLOT3D:
			fprintf(info,"3D plot");
			break;
		case PFAHC:
			fprintf(info,"adaptive hierarchical encoding");
			break;
		case PFOCT:
			fprintf(info,"oct-tree encoding");
			break;
		case PFAHC3:
			fprintf(info,"3-d adaptive hierarchical enocoding");
			break;
		case PFBQ:
			fprintf(info,"binquad encoding");
			break;
		case PFRLED:
			fprintf(info,"run-length encoding");
			break;
		case PFRLEB:
			fprintf(info,"black run-length encoding");
			break;
		case PFRLEW:
			fprintf(info,"white run-length encoding");
			break;
		default:
			fprintf(info,"Unknown format code");
	}
	if (strlen(hd[index].seq_history) > 1)
		fprintf(info,"\n\nSequence history:\n\n%s\n",hd[index].seq_history);
	else
		fprintf(info,"\n\nNo sequence history\n\n");
	if (strlen(hd[index].seq_desc) > 1)
		fprintf(info,"Sequence Description:\n\n%s\n",hd[index].seq_desc);
	else
		fprintf(info,"No sequence description\n");

}


/*
 *
 */
RequestPalette( new )
int new;
{
	int	entries, usage, tolerance;
	CTabHandle	ct_hndl;
	int ctF, ctSiz;
	long ctS;
	unsigned int val,i;
	ColorSpec *ctTab;
	RGBColor rgb;

	tolerance = 0;
	entries = 256;
	pal_hndl[new] = NewPalette( entries, NULL,pmTolerant, tolerance );

	for ( i =0; i < entries; i++ )
	{
		val = (  65535  / (entries-1) ) * i ;

		rgb.red = rgb.green = rgb.blue = val;
		SetEntryColor( pal_hndl[new], i, &rgb );
	}
	
	SetPalette( hips_wind_ptr[new], pal_hndl[new], TRUE );
	
}




/*	See Technical Note #120
 *
 *	new is the index of the hips window array.
 *	this routine draws the data into an offscreen pixmap
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
	Rect destRect;
	Rect globRect;
	Rect bRect;
	int theDepth;
	int	i, err;

	GDHandle theMaxDevice;
	GDHandle oldDevice;
	
	WindowPtr myCWindow;
	Rect bounds;
	
	
	offTop = 0;
	offLeft = 0;
	offBottom = hd[new].rows;
	offRight = hd[new].cols;
	
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


	myCGrafPtr[new] = &myCGrafPort[new];			/*open a new color port */
	OpenCPort( myCGrafPtr[new] );
	
	
	theDepth =   (  **(myCGrafPtr[new]->portPixMap)   ).pixelSize;
	
	/*now calculate the size of the pixel image that we need */

	offRowBytes = ((((theDepth * (offRight-offLeft)) + 15)) / 16 ) * 2 ; /*will be a word boundary*/
	
	sizeOfOff = (long) ( (long)offBottom - (long)offTop ) * offRowBytes ;
	OffsetRect( &bRect, -offLeft, -offTop );	/*adjust local coordinates*/
	
	/*set up base addr, row bytes, bounds, and pixel size of the PixMap in our new CPort */
	myBits[new] = NewPtr( sizeOfOff );				/*allocate space */
		
	if ( myBits[new] == NULL )
	{
		fprintf(info,"can't allocate memory for the off screen pixel map \n");
		fprintf(info,"was asking for %ld bytes \n", sizeOfOff);
		SysBeep(10);
		sleep(5);
		exit(1);
	}
	
	( **(myCGrafPtr[new]->portPixMap)  ).baseAddr = myBits[new];
	( **(myCGrafPtr[new]->portPixMap)  ).rowBytes = offRowBytes + 0x8000;	/*distinguish from a bitmap*/
	( **(myCGrafPtr[new]->portPixMap)  ).bounds = bRect;
	
	 myCGrafPtr[new]->portRect = bRect;		/*we'll be using portRect later to know original size */
	 
	/*now clone the color table of maxDevice so we can put it in our offscreen pixmap (in case
	the offscreen pix maps color table is diffrent), we also
	need to convert it from a color table for a device, to a color table for a pixmap (in
	case we are CopyBits-ing into a picture */
	
	ourCMHandle[new] = (  **(**theMaxDevice).gdPMap   ).pmTable;
	err = HandToHand( &ourCMHandle[new]  );
	
	if ( err != NULL )
	{
		fprintf(info, "Error in HandToHand()  \n");
		SysBeep(10);
	}
	
	/*now convert from device color table to pixmap color table */
	
	for ( i=0; i<= (**ourCMHandle[new]).ctSize; i++ )
	{
		(**ourCMHandle[new]).ctTable[i].value = i;			/*put in the indices */
	
		/*now clear the high bit of transindex to indicate it is a pixmap color table*/
		
		(**ourCMHandle[new]).transIndex &= 0x7FFF;
	}
		
		
	(  **(myCGrafPtr[new]->portPixMap)   ).pmTable = ourCMHandle[new]; /*put the cloned color table into offscreen pixmap */
	
	SetPort( myCGrafPtr[new] );		/*set port to the offscreen port */

	/*now draw whatever is needed in the offscreen port*/
	DrawContents( fpin, new );

	/* clean up before exiting */
		
	SetPort( myCWindow );
	SetGDevice( oldDevice );
		
	pic_sorc_rect[new] = myCGrafPtr[new]->portRect;
	
	/*all allocated stuff are deallocated only wha the picture is closed */
}

/*
 *	the port is already set to the offscreen port when this function is called
 *
 */
DrawContents(fpin, new)
FILE fpin;
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
	
	scanLine = calloc( hd[new].cols, sizeof(char) );
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

	
	num_cols = hd[new].cols;
	for (y = 0; y < hd[new].rows; y++) 
	{
		/*read a scan line */
		fread(scanLine,num_cols,1,fpin);

		d = bitPtr + (long)y * (long)rowBytes;
		
	    for (x = 0; x < hd[new].cols; x++) 
	    {
	   		pixel_value =  0x00FF & (int)scanLine[x] ;
	   					
	   		*d++ = (char)index[pixel_value];

/*-			pixel_value =  0x00FF & (int)scanLine[x] ;
			color.red = color.green = color.blue = pixel_value << 8 ;
			SetCPixel( x,y, &color );
*/
		}
	} 

	free( scanLine );
}



/*
 *	
 *
 */
HipsContent(e_pr)
EventRecord *e_pr;
{
	WindowRecord *w_ptr;
	long	index;
	Point	mouseStart;
	
	Point	mouseLoc;
	Rect	frame_rect,select_rect, temp_rect;
		
	/*get the ptr to the window that is to be processed */
	GetPort( &w_ptr );
	index = w_ptr->refCon;

	PenMode( srcXor );
		
	/*first redraw the zoom selection rectangle so that it disappears if it already exists*/
	OriginalToWindowRect( index, w_ptr, &zoom_sorc_rect[index],  &temp_rect  );
	FrameRect( &temp_rect );
	
	/*track the mouse */
		
	mouseStart = (*e_pr).where ;
	GlobalToLocal( &mouseStart );
	
	select_rect.top = mouseStart.v;
	select_rect.left = mouseStart.h;
	select_rect.bottom = mouseStart.v;
	select_rect.right = mouseStart.h;

		
	frame_rect = select_rect;
	while ( StillDown() )
	{
		FrameRect( &frame_rect );
		
		GetMouse( &mouseLoc );
		select_rect.bottom = mouseLoc.v;
		select_rect.right = mouseLoc.h;
		
		FixRect( &select_rect, &frame_rect );
		DiscretizeRect( &frame_rect, index, w_ptr );	/*make sure it falls on pixel boundaries*/
		FrameRect( &frame_rect );
	}	
	
	/* if only a point has been selected then return */
	if ( frame_rect.top == frame_rect.bottom && frame_rect.left == frame_rect.right )
		return;

	WindowToOriginalRect( index, w_ptr, &frame_rect,  &zoom_sorc_rect[index]  );
}


HipsMagnify( wPtr )
WindowRecord *wPtr;
{
	long	index;

	index = wPtr->refCon;
	
	if ( index < 0 || index >= MAXHIPS )
	{
		fprintf(info,"Front window must be a picture window to zoom.\n");
		return;
	}
	
	zoomisdrawn[index] = 1;
	pic_sorc_rect[index] = zoom_sorc_rect[index];
	SetPort ( wPtr );
	InvalRect( &(   ((GrafPtr)wPtr) ->portRect )   );

	return;
}

HipsDeMagnify( wPtr )
WindowRecord *wPtr;
{
	long	index;

	index = wPtr->refCon;
	
	if ( index < 0 || index >= MAXHIPS )
	{
		fprintf(info,"Front window must be a picture window to zoom.\n");
		return;
	}
	
	zoomisdrawn[index] = 0;
	pic_sorc_rect[index] = myCGrafPtr[index]->portRect;
	SetPort ( wPtr );
	InvalRect( &(   ((GrafPtr)wPtr) ->portRect )   );

	return;
}


HipsOriginalSize( wPtr )
WindowRecord *wPtr;
{
	long	index;
	Rect	r;
	Point	br;
	
	index = wPtr->refCon;
	
	if ( index < 0 || index >= MAXHIPS )
	{
		fprintf(info,"Front window must be a picture window.\n");
		return;
	}

	/*resize the front window to the original HIPS size */

	SizeWindow( wPtr, hd[index].cols , hd[index].rows, true );
	
	SetPort( wPtr );
	EraseRect( &(  ((GrafPtr)wPtr)->portRect) );
	InvalRect( &(  ((GrafPtr)wPtr)->portRect) );

	/* if after resizing the window is not visible, then make it visible */
	r =  ((GrafPtr)wPtr)->portRect;
	br = botRight ( r ) ;
	LocalToGlobal( & br );
	if ( br.h < 0 )
	{
		MoveWindow(wPtr,40,40,TRUE);
	}
}


HipsGrayColor( wPtr )
WindowRecord *wPtr;
{
	long	index;
	int	entries, usage, tolerance;
	CTabHandle	ct_hndl;
	int ctF, ctSiz;
	long ctS;
	unsigned int val,i;
	ColorSpec *ctTab;
	RGBColor rgb;

	PaletteHandle	newPal;
	CTabHandle newCT;
	
	ColorSpec	newCSpecArray[256];

SysBeep(10);
return;	
	index = wPtr->refCon;
	
	if ( index < 0 || index >= MAXHIPS )
	{
		fprintf(info,"Front window must be a picture window.\n");
		return;
	}


	/*change the device color table, assuming an 8 bit screen, and picture on only one screen */
	
	/*white at position 0, black at 255 */
	rgb.red = rgb.green = rgb.blue = 65535;
	newCSpecArray[0].value = 8;
	newCSpecArray[0].rgb = rgb;
	SetEntries( 0, 0, newCSpecArray );

	for ( i=1; i < 255; i++)
	{
		newCSpecArray[i].value = 8;
		
		val = (  65535  / (200) ) * i ;
		rgb.red = val;
		rgb.green = rgb.blue = 0;

		newCSpecArray[i].rgb = rgb;
	}
	SetEntries( 1, 253, newCSpecArray );

	rgb.red = rgb.green = rgb.blue = 0;
	newCSpecArray[0].value = 8;
	newCSpecArray[0].rgb = rgb;
	SetEntries( 255, 0, newCSpecArray );
	





return;
/*	DisposePalette( pal_hndl[index] );	*/	/*get rid of old palette */
	
	tolerance = 0;
	entries = 256;
	newPal = NewPalette( entries, NULL, /*pmAnimated*/ pmTolerant, tolerance );

	for ( i =0; i < entries; i++ )
	{
		val = (  65535  / (entries-1) ) * i ;
		rgb.red = val;
		rgb.green = rgb.blue = 0;
		SetEntryColor( newPal, i, &rgb );
		
	}
	
	SetPalette( hips_wind_ptr[index], newPal, TRUE ); 

return;	
	newCT = (CTabHandle) NewHandle( 10L );

	Palette2CTab( newPal, newCT );
	AnimatePalette( hips_wind_ptr[index],newCT,0,0,256); 

}


HipsInvertColor( wPtr )
WindowRecord *wPtr;
{
	long	index;
SysBeep(10);
return;	

	index = wPtr->refCon;
	
	if ( index < 0 || index >= MAXHIPS )
	{
		fprintf(info,"Front window must be a picture window.\n");
		return;
	}

}

/*
 *	fix a rectangle to be top left, bottom right, in correct ordering 
 */
FixRect( r, s)
Rect *r, *s;
{

	if ( r->bottom < r->top )
	{
		s->bottom = r->top;
		s->top = r->bottom;
	} else {
		s->bottom = r->bottom;
		s->top = r->top;
	}	

	if ( r->right < r->left )
	{
		s->right = r->left;
		s->left = r->right;
	} else {
		s->left = r->left;
		s->right = r->right;
	}
}

