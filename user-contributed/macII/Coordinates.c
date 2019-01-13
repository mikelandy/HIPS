
#include <MacTypes.h>
#include <QuickDraw.h>
#include <EventMgr.h>
#include <WindowMgr.h>
#include <FontMgr.h>
#include <ScrapMgr.h>
#include <DialogMgr.h>
#include <MemoryMgr.h>
#include <ColorToolbox.h>


#include "hipl_format.h" 

#define STRCLASS	extern	
#include "globals.h"

extern	FILE	*info;

#define	X_SCALE_ID	129
#define	Y_SCALE_ID	130
#define	Z_SCALE_ID	131


/*	convert from the original pixel coordinate system to the viewing ports' coordinate system
 *  note that the viewing port might be a stretched one, or already a zoomed one.
 */
OriginalToWindowRect( index, w_ptr, orig_rect_ptr,  wind_rect_ptr )
long index;
WindowRecord *w_ptr;
Rect *orig_rect_ptr;
Rect *wind_rect_ptr;
{
	Rect dest_rect, temp_rect;
	double hscale, vscale;
	
	dest_rect = ((CGrafPtr)w_ptr)->portRect;


	/*first find coordinates of the requested rect in the currently viweing pic_sorc_rect */
	temp_rect.top = (*orig_rect_ptr).top - pic_sorc_rect[index].top;
	temp_rect.left = (*orig_rect_ptr).left - pic_sorc_rect[index].left;
	temp_rect.bottom = (*orig_rect_ptr).bottom - pic_sorc_rect[index].top;
	temp_rect.right = (*orig_rect_ptr).right - pic_sorc_rect[index].left;
	
	
	vscale = ((double)(dest_rect.bottom - dest_rect.top)) /
			 ((double)(pic_sorc_rect[index].bottom - pic_sorc_rect[index].top)) ; 

	hscale = ((double)(dest_rect.right - dest_rect.left)) /
			 ((double)(pic_sorc_rect[index].right - pic_sorc_rect[index].left)) ;
			 
	temp_rect.top = (int)((vscale * (double)temp_rect.top ) + 0.99);
	temp_rect.left = (int)((hscale * (double)temp_rect.left ) + 0.99);
	temp_rect.bottom = (int)((vscale * (double)temp_rect.bottom ) + 0.99);
	temp_rect.right = (int)((hscale * (double)temp_rect.right ) + 0.99);
	
	(*wind_rect_ptr) = temp_rect;

}

/*	convert from the viewing ports' coordinate system to the original pixel coordinate system
 *  note that the viewing port might be a stretched one, or already a zoomed one.
 */
WindowToOriginalRect( index, w_ptr, wind_rect_ptr, orig_rect_ptr )
long index;
WindowRecord *w_ptr;
Rect *orig_rect_ptr;
Rect *wind_rect_ptr;
{
	Rect dest_rect, temp, base_rect;
	double hscale, vscale;
	
	dest_rect = ((CGrafPtr)w_ptr)->portRect;

	/* digitize the rectangle to discrete coordinates of the pixel map*/
	/* find the true coordinates of the ractangle that has been chosen, the current
	view on the screen may already be a zoomed one. The current view is in pic_sorc_rect */

	base_rect = pic_sorc_rect[index];
	
	vscale = ((double)(base_rect.bottom - base_rect.top)) / 
			 					((double)(dest_rect.bottom)  );
	hscale = ((double)(base_rect.right - base_rect.left)) / 
			 					((double)(dest_rect.right)   );
			
	temp.top =    base_rect.top  + vscale * (*wind_rect_ptr).top + 0.00;
	temp.left =   base_rect.left + hscale * (*wind_rect_ptr).left + 0.00;
	temp.bottom = base_rect.top  + vscale * (*wind_rect_ptr).bottom + 0.00;
	temp.right =  base_rect.left + hscale * (*wind_rect_ptr).right + 0.00;
	
	/*if the chosen rect in the original picture ends up being of size zero, then 
	set it to size 1 pixel */
	if ( ( temp.bottom - temp.top ) ==  0  &&
		 ( temp.right - temp.left ) ==  0 )
		{
			temp.bottom ++;
			temp.right ++;
		}

	(*orig_rect_ptr) = temp;

}





DiscretizeRect( r_ptr, index, w_ptr )
Rect *r_ptr;
long index;
WindowRecord *w_ptr;
{	
	Rect rect_on_pixmap;
		
	WindowToOriginalRect( index, w_ptr, r_ptr,  &rect_on_pixmap );
	
	OriginalToWindowRect( index, w_ptr, &rect_on_pixmap,  r_ptr );
	
}


/*
 *	when this command is invoked the zoom_sorc_rect[index] already has a rect in it,
 *	which defines the point as the top.left of the rect.
 */
SetXYCoordOrigin(  )
{
	long index;
	WindowRecord *w_ptr;
	
	GetPort( &w_ptr );
	index = w_ptr->refCon;
	if ( index < 0 || index >= MAXHIPS )
	{
		fprintf(info,"Front window must be a picture window.\n");
		return;
	}

	x_origin[index]	= zoom_sorc_rect[index].left;
	y_origin[index] = zoom_sorc_rect[index].top;
	
}



/*
 *	when this command is invoked the zoom_sorc_rect[index] already has a rect in it,
 *	which defines the point as the top.left of the rect.
 */
SetXYCoordMaxs(  )
{
	long index;
	WindowRecord *w_ptr;
	
	GetPort( &w_ptr );
	index = w_ptr->refCon;
	if ( index < 0 || index >= MAXHIPS )
	{
		fprintf(info,"Front window must be a picture window.\n");
		return;
	}

	x_max[index]	= zoom_sorc_rect[index].left;
	y_max[index] 	= zoom_sorc_rect[index].top;

}

/*
 *	set default coordinate systems for the new opened window 
 */
DefaultCoords(new)
int new;
{

	x_origin[new] = 0;
 	y_origin[new] = hd[new].orows;
 	z_origin[new] = 0;
 	x_max[new] = hd[new].ocols;
 	y_max[new] = 0;
 	z_max[new] = 0;				/*unused */
	x_offset[new] = 0;
	y_offset[new] = 0;
	z_offset[new] = 0;			
	x_scale_per_pixel[new] = 1;
	y_scale_per_pixel[new] = 1;
	z_scale_per_pixel[new] = 1;

	strcpy( x_units[new],"Units" );
	strcpy( y_units[new],"Units" );
	strcpy( z_units[new],"Units" );

}
SetXScale()
{
 	DialogPtr	dptr;
 	int		itemHit,kind;
 	Handle	itemH;
 	Rect	dbox;
	char 	text[256];
	
	long index;
	WindowRecord *w_ptr;
	
	double max;
	
	GetPort( &w_ptr );
	index = w_ptr->refCon;
	if ( index < 0 || index >= MAXHIPS )
	{
		fprintf(info,"Front window must be a picture window.\n");
		return;
	}
	
 	dptr = GetNewDialog(X_SCALE_ID,NULL,-1L);
	
 	do
 	{
 		ModalDialog(NULL,&itemHit);
 		
 	} while ( itemHit != OK  && itemHit != Cancel );
 	
 	if ( itemHit == OK )
 	{
	 	GetDItem(dptr,6,&kind,&itemH,&dbox);
	  	GetIText(itemH,text);
	  	PtoCstr(text);
	  	sscanf(text,"%le", &x_offset[index] );
	
	 	GetDItem(dptr,7,&kind,&itemH,&dbox);
	  	GetIText(itemH,text);
	  	PtoCstr(text);
	  	sscanf(text,"%le", &max );
	  	
	  	x_scale_per_pixel[index] = ( max - x_offset[index] ) / 
	  									(double)(x_max[index] - x_origin[index] );

	 	GetDItem(dptr,9,&kind,&itemH,&dbox);
	  	GetIText(itemH,x_units[index]);  	
		PtoCstr( x_units[index]);  
  	}
 	DisposDialog(dptr);
 	
}


SetYScale()
{
 	DialogPtr	dptr;
 	int		itemHit,kind;
 	Handle	itemH;
 	Rect	dbox;
	char 	text[256];
	
	long index;
	WindowRecord *w_ptr;
	
	double max;
	
	GetPort( &w_ptr );
	index = w_ptr->refCon;
	if ( index < 0 || index >= MAXHIPS )
	{
		fprintf(info,"Front window must be a picture window.\n");
		return;
	}
	
 	dptr = GetNewDialog(Y_SCALE_ID,NULL,-1L);
	
 	do
 	{
 		ModalDialog(NULL,&itemHit);
 		
 	} while ( itemHit != OK  && itemHit != Cancel );
 	
 	if ( itemHit == OK )
 	{
	 	GetDItem(dptr,6,&kind,&itemH,&dbox);
	  	GetIText(itemH,text);
	  	PtoCstr(text);
	  	sscanf(text,"%le", &y_offset[index] );
	
	 	GetDItem(dptr,7,&kind,&itemH,&dbox);
	  	GetIText(itemH,text);
	  	PtoCstr(text);
	  	sscanf(text,"%le", &max );
	  	
	  	y_scale_per_pixel[index] = -( max - y_offset[index] ) / 
	  									(double)(y_max[index] - y_origin[index] );
  	
	 	GetDItem(dptr,9,&kind,&itemH,&dbox);
	  	GetIText(itemH,y_units[index]);  	
		PtoCstr( y_units[index]);  
  	}
 	DisposDialog(dptr);
 	
}


SetZScale()
{
 	DialogPtr	dptr;
 	int		itemHit,kind;
 	Handle	itemH;
 	Rect	dbox;
	char 	text[256];
	
	long index;
	WindowRecord *w_ptr;
	
	double max;
	
	GetPort( &w_ptr );
	index = w_ptr->refCon;
	if ( index < 0 || index >= MAXHIPS )
	{
		fprintf(info,"Front window must be a picture window.\n");
		return;
	}
	
 	dptr = GetNewDialog(Z_SCALE_ID,NULL,-1L);
	
 	do
 	{
 		ModalDialog(NULL,&itemHit);
 		
 	} while ( itemHit != OK  && itemHit != Cancel );
 	
 	if ( itemHit == OK )
 	{
	 	GetDItem(dptr,7,&kind,&itemH,&dbox);
	  	GetIText(itemH,text);
	  	PtoCstr(text);
	  	sscanf(text,"%le", &z_offset[index] );	/*value for black pixels */
	
	 	GetDItem(dptr,6,&kind,&itemH,&dbox);
	  	GetIText(itemH,text);
	  	PtoCstr(text);
	  	sscanf(text,"%le", &max );				/*white*/
	  	
	  	z_scale_per_pixel[index] = ( max - z_offset[index] ) / 
	  									(double)(256);
  	
	 	GetDItem(dptr,9,&kind,&itemH,&dbox);
	  	GetIText(itemH,z_units[index]);  	
		PtoCstr( z_units[index]);  
  	}
 	DisposDialog(dptr);
 	
}


CoordWindUpdate()
{
	WindowRecord *w_ptr, *fw_ptr;
	long	index;
	
	fw_ptr = (WindowRecord *)FrontWindow();
	index = fw_ptr->refCon;
	
	/*get the ptr to the window that is to be updated. This should be the current one */
	GetPort( &w_ptr );

	TextFont( monaco );
	TextSize( 12 );
	
	BeginUpdate( w_ptr );
	
	EraseRect( &((GrafPtr)w_ptr)->portRect );
		
	EndUpdate( w_ptr );

}


/*
 *	if mouse is on top of a picture window, then give coord info.
 */
MouseCoord()
{
	WindowRecord 	*w_ptr, *fw_ptr;
	long			index;
	static Point 	p;
	static Point 	prev_p;
	char 			t[256];
	Rect			xyz_rect, point_rect, orig_rect;
	double			x,y,z;
	char 			*pixel_value_pr;
	long 			pixel_value;
	RGBColor		rgb;
	char			temp[256];
	
	fw_ptr = (WindowRecord *)FrontWindow();
	index = fw_ptr->refCon;
	
	if ( index < 0 || index >= MAXHIPS ) 
		return;
	
	SetPort ( fw_ptr );
	prev_p = p;
	GetMouse( &p );
	if ( !PtInRect( p, &(((GrafPtr)fw_ptr)->portRect)  ) )
		return;
		 
	if ( (prev_p.v == p.v) && (prev_p.h == p.h)  )	/*mouse has not moved */
		return;
		
	SetPort( coord_wind_ptr );
	
	xyz_rect.top = 0;
	xyz_rect.left = StringWidth("\pX value: ");
	xyz_rect.bottom = xyz_rect.top + 70;
	xyz_rect.right = xyz_rect.left + StringWidth("\p12345.123");
	
	EraseRect( &xyz_rect );
	
	/*convert the location info from local coordinates to the pixel map ones */
	point_rect.top = p.v;
	point_rect.left = p.h;
	point_rect.bottom = p.v + 1;
	point_rect.right = p.h + 1;
	
	WindowToOriginalRect( index, fw_ptr, &point_rect, &orig_rect );
	
	/*get the pixel value at that location */
	pixel_value_pr =	( **(myCGrafPtr[index]->portPixMap)  ).baseAddr + 
						((**(myCGrafPtr[index]->portPixMap)  ).rowBytes & 0x7FFF) * (long)orig_rect.top + 
						(long)orig_rect.left ;
	pixel_value = (long)(*pixel_value_pr) & 0x000000FF;
	Index2Color( pixel_value, &rgb );	/*look in color table */
	pixel_value = (rgb.red >> 8) ;
	
	x = x_offset[index] + x_scale_per_pixel[index] * (orig_rect.left - x_origin[index]);
	MoveTo (2,15 );
	TextFace( bold );
	DrawString("\pX value: ");
	sprintf( t, "%9.3lf", x );
	CtoPstr(t);
	TextFace( 0 );
	DrawString(t);
	TextFace( bold );
	Move(10,0);
	strcpy(temp,x_units[index]);
	CtoPstr(temp);
	DrawString( temp );
	
	y = y_offset[index] + y_scale_per_pixel[index] * (- (orig_rect.top - y_origin[index])  );
	MoveTo (2,30 );
	TextFace( bold );
	DrawString("\pY value: ");
	sprintf( t, "%9.3lf", y );
	CtoPstr(t);
	TextFace( 0 );
	DrawString(t);
	TextFace( bold );
	Move(10,0);
	strcpy(temp,y_units[index]);
	CtoPstr(temp);
	DrawString( temp );

	z = z_offset[index] + z_scale_per_pixel[index] * pixel_value  ;
	MoveTo (2,45 );
	TextFace( bold );
	DrawString("\pZ value: ");
	sprintf( t, "%9.3lf", z );
	CtoPstr(t);
	TextFace( 0 );
	DrawString(t);
	TextFace( bold );
	Move(10,0);
	strcpy(temp,z_units[index]);
	CtoPstr(temp);
	DrawString( temp );
}



/*
 *
 */
SaveXYZScales()
{
	long index;
	WindowRecord *w_ptr;
	char title[256];
	FILE *fptr;
	
	w_ptr = (WindowRecord *)FrontWindow();
	index = w_ptr->refCon;
	if ( index < 0 || index >= MAXHIPS )
	{
		fprintf(info,"Front window must be a picture window.\n");
		return;
	}

	GetWTitle( w_ptr, title );
	PtoCstr( title );
	
	strcat( title, ".coord" );
	fptr = fopen( title , "w" );
	
	fprintf( fptr, "%d\n",  x_origin[index] );
	fprintf( fptr, "%d\n",  y_origin[index] );
	fprintf( fptr, "%d\n",  z_origin[index] );
	fprintf( fptr, "%d\n",  x_max[index] );
	fprintf( fptr, "%d\n",  y_max[index] );
	fprintf( fptr, "%d\n",  z_max[index] );
	fprintf( fptr, "%le\n", x_offset[index] );
	fprintf( fptr, "%le\n", y_offset[index] );
	fprintf( fptr, "%le\n", z_offset[index] );
	fprintf( fptr, "%le\n", x_scale_per_pixel[index] );
	fprintf( fptr, "%le\n", y_scale_per_pixel[index] );
	fprintf( fptr, "%le\n", z_scale_per_pixel[index] );
	fprintf( fptr, "%s\n",  x_units[index] );
	fprintf( fptr, "%s\n",  y_units[index] );
	fprintf( fptr, "%s\n",  z_units[index] );


	fclose( fptr );
}


/*
 *
 */
ReadCoords(fname,index)
char *fname;
int index;
{
	char title[256];
	FILE *fptr;
	
	strcpy(title, fname );
	
	strcat( title, ".coord" );
	fptr = fopen( title , "r" );
	
	if ( fptr == NULL )
	{
		fprintf(info,"No coordinates file. Will assume defaults.\n");
		DefaultCoords(index);
		return;
	}
	
	fscanf( fptr, "%d",  &x_origin[index] );
	fscanf( fptr, "%d",  &y_origin[index] );
	fscanf( fptr, "%d",  &z_origin[index] );
	fscanf( fptr, "%d",  &x_max[index] );
	fscanf( fptr, "%d",  &y_max[index] );
	fscanf( fptr, "%d",  &z_max[index] );
	fscanf( fptr, "%le", &x_offset[index] );
	fscanf( fptr, "%le", &y_offset[index] );
	fscanf( fptr, "%le", &z_offset[index] );
	fscanf( fptr, "%le", &x_scale_per_pixel[index] );
	fscanf( fptr, "%le", &y_scale_per_pixel[index] );
	fscanf( fptr, "%le", &z_scale_per_pixel[index] );
	fscanf( fptr, "%s",  x_units[index] );
	fscanf( fptr, "%s",  y_units[index] );
	fscanf( fptr, "%s",  z_units[index] );


	fclose( fptr );
}
