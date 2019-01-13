
#include <MacTypes.h>
#include <QuickDraw.h>
#include <EventMgr.h>
#include <WindowMgr.h>
#include <FontMgr.h>
#include <ScrapMgr.h>
#include <DialogMgr.h>
#include <MemoryMgr.h>
#include <ColorToolbox.h>
#include <StdFilePkg.h>


#include "hipl_format.h" 

#define STRCLASS	extern	
#include "globals.h"

extern	FILE	*info;

DataAvgX()
{
	long 			index;
	WindowRecord 	*w_ptr;
	FILE			*fptr;
	int 			yi,xi;

	double			x,y,z, sum;
	char 			*pixel_value_pr;
	long 			pixel_value;
	RGBColor		rgb;
	
	char 			fname[256];
	int				r;
	
	GetPort( &w_ptr );
	index = w_ptr->refCon;
	if ( index < 0 || index >= MAXHIPS )
	{
		fprintf(info,"Front window must be a picture window.\n");
		return;
	}

	if ( 	zoom_sorc_rect[index].top  == zoom_sorc_rect[index].bottom  ||
			zoom_sorc_rect[index].left == zoom_sorc_rect[index].right  )
		return;
	
	r = GetFileName( fname );
	if ( r != 0 )
		return;
	
	fptr = fopen( fname,"w");
	
	for ( yi = zoom_sorc_rect[index].bottom -1 ; yi >= zoom_sorc_rect[index].top; yi -- )
	{
		sum = 0.0;
		for ( xi = zoom_sorc_rect[index].left; xi < zoom_sorc_rect[index].right; xi ++ )
		{
			/*get the pixel value at that location */
			pixel_value_pr =	( **(myCGrafPtr[index]->portPixMap)  ).baseAddr + 
								((**(myCGrafPtr[index]->portPixMap)  ).rowBytes & 0x7FFF) * (long)yi + 
								(long)xi ;
			pixel_value = (long)(*pixel_value_pr) & 0x000000FF;
			Index2Color( pixel_value, &rgb );	/*look in color table */
			pixel_value = (rgb.red >> 8) ;

			z = z_offset[index] + z_scale_per_pixel[index] * pixel_value  ;
			sum += z;
		}
		
		x = sum / (zoom_sorc_rect[index].right - zoom_sorc_rect[index].left );
		y = y_offset[index] + y_scale_per_pixel[index] * (- (yi - y_origin[index])  );
		
		fprintf(fptr,"%le\t%le\n",y, x );
	}
	
	
	

	fclose (fptr );

}

DataAvgY()
{
	long 			index;
	WindowRecord 	*w_ptr;
	FILE			*fptr;
	int 			yi,xi;

	double			x,y,z, sum;
	char 			*pixel_value_pr;
	long 			pixel_value;
	RGBColor		rgb;
	
	char 			fname[256];
	int				r;
	
	GetPort( &w_ptr );
	index = w_ptr->refCon;
	if ( index < 0 || index >= MAXHIPS )
	{
		fprintf(info,"Front window must be a picture window.\n");
		return;
	}

	if ( 	zoom_sorc_rect[index].top  == zoom_sorc_rect[index].bottom  ||
			zoom_sorc_rect[index].left == zoom_sorc_rect[index].right  )
		return;

	r = GetFileName( fname );
	if ( r != 0 )
		return;
	
	fptr = fopen( fname,"w");
	
	for ( xi = zoom_sorc_rect[index].left; xi < zoom_sorc_rect[index].right; xi ++ )
	{
		sum = 0.0;
		for ( yi = zoom_sorc_rect[index].bottom -1 ; yi >= zoom_sorc_rect[index].top; yi -- )
		{
			/*get the pixel value at that location */
			pixel_value_pr =	( **(myCGrafPtr[index]->portPixMap)  ).baseAddr + 
								((**(myCGrafPtr[index]->portPixMap)  ).rowBytes & 0x7FFF) * (long)yi + 
								(long)xi ;
			pixel_value = (long)(*pixel_value_pr) & 0x000000FF;
			Index2Color( pixel_value, &rgb );	/*look in color table */
			pixel_value = (rgb.red >> 8) ;

			z = z_offset[index] + z_scale_per_pixel[index] * pixel_value  ;
			sum += z;
		}
		
		y = sum / (zoom_sorc_rect[index].bottom - zoom_sorc_rect[index].top );
		x = x_offset[index] + x_scale_per_pixel[index] * (   xi - x_origin[index]);
		
		fprintf(fptr,"%le\t%le\n",x, y );		
	}
	
	
	

	fclose (fptr );

}







GetFileName( fname )
char *fname;
{
  	SFTypeList 	typeList;
  	ProcPtr 	dlgHook = NULL;
  	SFReply 	reply;
  	char 		prompt[256];
  	Point 		where;
  	char		fName[256];
  	
  	where.v = 75;
  	where.h = 100;
  	strcpy(prompt,"Save the selection as:");
  	CtoPstr(prompt);
  	strcpy(fName,"Data.Data");
	CtoPstr(fName);
	
	SFPutFile(where,prompt,fName,dlgHook,&reply);
	if (reply.good == TRUE)
	  {
	  	SetVol("\p",reply.vRefNum); 		/* this changes the disk volume stuff */
		PtoCstr( (char *)reply.fName );
		strcpy( fname, reply.fName );
		return(0);
	  }
	else
	  {
	  	return(-1);
	  }

}
