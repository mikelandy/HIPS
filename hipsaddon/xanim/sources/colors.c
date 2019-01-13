/*
 * %W% 	%G%
 *
 * XView preview for HIPS images V1.0
 * colors.c : 	contains the routines to create and manipulate the color
 *		table.
 *		This file should be toolkit independent.
 *
 * Author: 	Cathy Waite
 * 		(c) The Turing Institute
 *
 * 		Tue Aug 27 14:38:54 BST 1991
 *
 */

#include "xv_frame.h"
#include <xview/cms.h> 
#include "appl.h"

#include "xv_adjust.h"
#include "xv_canvas.h"
#include "xv_animation.h"
#include "xv_header.h"

/* RF1: Major change to this file.  Now use XView color segments.  May also
 * request 64 rather than 256 colors.
 */

/*
 * This file contains routines to create a grayscale ramp of 256 colors
 * for rendering the image.  However, the control panel uses about 6 of
 * these values, so they grays scale values for these six entries will be
 * wrong.  (by default ... -r flag *)
 * if the -s flag is set, these six xolours will be substiteud for the nearest
 * neighbour.
 * This is slow as it means changing the image as it is read in.

 * RF 4
 * If the -i flag is set, the control panel colors will not be maintained, so
 * image is displated correctly, but control panel will be very dark.
 */

 /* Warning these flags have to be mutually exclusive ... I depend on
  * pareseargs to ensure this.

  * If the control panel is not displayed (due to the -w option)
  * igonre_panel will be forced to TRUE and the others to FALSE
  */
h_boolean	do_substitute = FALSE;	/* Set by -s */
h_boolean	retain_panel = TRUE;	/* DEFAULT, also set by -r */
h_boolean	ignore_panel = FALSE;	/* set by -i */

/* Structure used to get colors used in control panel */
Xv_cmsdata 	*cms_data;
Cms		cms;

struct pairs{
	int pixval;
	int nearest;
};
typedef struct pairs pixpairs;
pixpairs	*neighbours;

/* Strctures for color manipulation */
int		no_colors = 256; 
XColor		colors[256];
unsigned long	*actual_colors;
int		brightness = 0;
float		contrast = 1.0;
int		std_brightness = 0;
float		std_contrast = 1.0;

/* Routine to find the nearest neighbour for a value which uses a control
 * panel value. 
*/

int	nearestNeighbour(colordef, colors,noOfColors)
XColor	colordef;
XColor	*colors;
int	noOfColors;
{
  	long dist = -1;
  	long newdist = 0 ;
  	register int i;
  	int index = 0;
  	register unsigned int r1, r2, b1, b2, g1, g2;

  	r1 = colordef.red;
  	g1 = colordef.green;
  	b1 = colordef.blue;
  	for ( i = 0; i < noOfColors; ++i )
    	{
      		r2 = colors[i].red;
      		g2 = colors[i].green;
      		b2 = colors[i].blue;

      		newdist = abs((long)(( r1 - r2 ) * ( r1 - r2 ) +
			( g1 - g2 ) * ( g1 - g2 ) +
	  		( b1 - b2 ) * ( b1 - b2 )));

      		if (( newdist < dist ) || (dist == -1))
		{
	  		index = i;
	  		dist = newdist;
		}
    	}
  	return index;
}


/* Creates a grayscale color map and installs it in those windows which
 * need to have the image dislpayed correctly when the mouse is over them.
 * i.e. the actual image canvas, and those windows with controls which
 * mannipulate the image
*/
void	create_default_color_map()
{
	register int i;
	XColor		colordef, colorSought;
	Colormap	color_map;
	Cms		cms2;
	char		cm_name[TEXT_LENGTH];

	for (i=0; i<no_colors; i++)
	{
		colors[i].pixel = (u_long)i;
		colors[i].red = colors[i].green = colors[i].blue =
				(u_short)i<<((no_colors == 64) ? 10 : 8); 
		colors[i].flags = DoRed | DoGreen | DoBlue;
	}
	
	/* find colors of panel */
	/* RF: 4 ... have option to ignore control panel colors */
	if ((no_colors != 64) && (ignore_panel == FALSE))
	{
		color_map = DefaultColormap(display, screen);
		cms_data = (Xv_cmsdata *)xv_get(control_panel, WIN_CMS_DATA); 
		neighbours = (pixpairs *)
			halloc(cms_data->size,sizeof(pixpairs));
		for (i=0 ; i < cms_data->size ; i++) 
		{ 
			colordef.red = cms_data->red[i]<<8; 
			colordef.green = cms_data->green[i]<<8;
			colordef.blue = cms_data->blue[i]<<8; 
			XAllocColor(display, color_map, &colordef);
			colorSought = colors[colordef.pixel];
			colors[colordef.pixel] = colordef;
			colors[colordef.pixel].flags = DoRed | DoGreen | DoBlue;
			neighbours[i].pixval = colordef.pixel;
			if (do_substitute == TRUE)
				neighbours[i].nearest = nearestNeighbour(colorSought,
				     colors, no_colors);
		}
	}

	/* This hould work ... it should find any existing xanim
	 * colourmaps for this screen ... but I get 
	 * Xview warning: invalid object(not a pointer), xv_get
	 * error at runtime.
	 */
	/*
	sprintf(cm_name,"xa_gram_%d\n",no_colors);
	cms = (Cms)xv_find(screen, CMS,
		CMS_NAME,		cm_name,
		XV_AUTO_CREATE,		FALSE,
		NULL);

	if (cms == NULL)
	{
	*/
		cms = (Cms)xv_create(screen, CMS,
			CMS_NAME,		cm_name,
			CMS_TYPE,               XV_DYNAMIC_CMS,
			CMS_SIZE,               no_colors,
			CMS_X_COLORS,           colors,
			NULL);
	/*
	}
	*/


        actual_colors = (unsigned long *)xv_get(cms, CMS_INDEX_TABLE);

	if (no_colors != 64) 
        {
		xv_set(frame, WIN_CMS, cms, NULL);
		if (ignore_panel == FALSE)
		{
			/* CAn I explicitly set the thingor for the
			control panel so it works ?
			Turn inherited colours offf ?
			*/
			xv_set(adjust_command_frame, WIN_CMS, cms, NULL);
			xv_set(animation_command_frame, WIN_CMS, cms, NULL);
			xv_set(header_command_frame, WIN_CMS, cms, NULL);
		}
        }
}

/* Returns TRUE if the value is one of the values used for the control
 * panel.
 */
h_boolean	is_panel_color(pixel)
int	pixel;
{
	register int i;
	for (i=0; i<cms_data->size; i++)
		if (pixel == neighbours[i].pixval)
			return(TRUE);
	return(FALSE);
}
	
/* Changes the contrast and brightness by animating the colormap.
   Uses the values of contrast and brightness set in xv_adjust.c
   in the adjust control panel.
 */
void	update_color_map()
{
	register int i,adjusted;

	for (i=0; i<no_colors; i++)
	{
		if ((no_colors != 64) && (ignore_panel == FALSE))
		{
			if (is_panel_color(i) == TRUE)
			/* Do not update panel colors ... */
				continue;
		}

		adjusted= MAX(0, MIN(no_colors-1, (contrast*i+brightness)));
		colors[i].pixel = (u_long)i;
		colors[i].red = colors[i].green = colors[i].blue =
				(u_short)adjusted<<((no_colors == 64) ? 10:8);
		colors[i].flags = DoRed | DoGreen | DoBlue;
	}

	xv_set(cms, CMS_X_COLORS, colors, NULL);

	if ((brightness != std_brightness) || (contrast != std_contrast))
		enable_revert();
	print_message("");

}

/* Revert the colormap to be a standard greyscale ramp */
void 	revert_colors()
{
	if ((brightness != std_brightness) || (contrast  != std_contrast))
	{
		brightness = std_brightness;
		contrast = std_contrast;
		set_bright_contr(std_brightness, std_contrast);
		update_color_map();
	}
}

/* Update the actual image so that its pixel values are those indicated by
   the contrast and brightness, and then reset the color map.
*/
void	set_contrast_brightness()
{
	register int  i,j ;
	int	adjusted_cols[256];

	if ((brightness != std_brightness) || (contrast  != std_contrast))
	{
		/* Create tables of colors */
		for(i=0; i< no_colors; i++)
			adjusted_cols[i] = MAX(0, MIN(no_colors-1, 
				(contrast*i+brightness)));
		for (i=0; i<no_frames; i++)
			for(j=0; j<image_size; j++)
				resized_frames[i][j] = adjusted_cols[resized_frames[i][j]];
		revert_colors();
		render_image();
	}
}


/* Set the contrast and brightness for one frame ... */
void	set_cb_single_frame(single_frame, frame_no)
frame_ptr	single_frame;
int		frame_no;
{
	int 		i, adjusted_cols[256];
	frame_ptr	source_frame = resized_frames[frame_no];
	if ((brightness != std_brightness) || (contrast  != std_contrast))
	{
		/* Create tables of colors */
		for(i=0; i< no_colors; i++)
			adjusted_cols[i] = MAX(0, MIN(no_colors-1, 
				(contrast*i+brightness)));
		for(i=0; i<image_size; i++) 
			single_frame[i] = adjusted_cols[source_frame[i]];
	}
}


/* Return the pixel value at pixel (x,y) */
int	get_pixel_value(x,y)
int	x,y;
{
	int val = resized_frames[current_frame][(y*image_width)+x];
	return(MAX(0, MIN(no_colors-1, (contrast*val+brightness))));
}


void modify_image_colors()
{
        register int i,j,x;
        if (no_colors == 64)
                for(i=0; i< no_frames; i++)
                        for (j=0; j<image_size; j++)
                                orig_frames[i][j] = 
					actual_colors[((orig_frames[i][j])>>2)];

        else if (do_substitute)
                for (i=0; i < no_frames; i++)
                        for (j=0; j< image_size; j++)
                                for (x=0 ; x < cms_data->size ; x++)
                                        if (orig_frames[i][j]==neighbours[x].pixval)
                                                orig_frames[i][j] = neighbours[x].nearest;
}


