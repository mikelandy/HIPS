/*
 * @(#)appl.c	2.11	11/29/91
 * XView previewer for HIPS images V1.0
 * appl.c : 	contains the application functionality ....
 *		Saving, Loading, Resizing etc. of images.
 *		This file should be (reasonably) toolkit independent.
 *
 * Author: 	Cathy Waite
 * 		(c) The Turing Institute
 *
 * Mon Aug 12 13:25:15 BST 1991
 *
 */

#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <xview/notice.h>
#include "xv_frame.h"
#include "appl.h"
#include "colors.h"

#include "xv_canvas.h"
#include "xv_adjust.h"
#include "xv_animation.h"
#include "xv_header.h"

/* 
 * Several copies of the image are held, depending on the operations
 * carried out. Each version has a header and an associated set of frames.
 *
 * orig_header/frames is the version initially displayed.  This is the
 * version read in and converted (if required) to PFBYTE.  Revert will
 * revert to the version held in orig_header/frames.
 *
 * extract_header/frames is the version which holds the extracted portion
 * of the image.  Initially the header will be the same as org_header, and
 * the extrat_frames will be a pointer to orig_frames.  When a portion is
 * extracted, extract_frames is allocated new versions of the image.
 *
 * resized_header/frames is the version that holds the resized image.  The
 * image is always rsize from the extract version (which will initially be te
 * same as the original version.  When the image has been extracted, and
 * not resized, the resizied frames are the same as the extracted frame.
 * THE RESIZED VERSION IS THE ONE DISPLAYED, so it is the one saved etc.
 *
 * input_header does not have frames associated.  It is the header as read
 * form the original file, before any conversion, and is kept around so the
 * image can be converted back if required.
 *
 */
struct 	header	input_header, 
		orig_header, 
		resized_header, 
		extract_header;

frame_ptr	*orig_frames, 
		*resized_frames, 
		*extract_frames; 

/* Various dimensions of the image ... */
int		image_width, image_height, no_frames, image_size;

/* The number of the frame currently being displayed */
int		current_frame;

/* The types which xanim can cope with.  Currently this is just PFBYTE.
 * Attempts will be made to convert everything to PFBYTE
 */
int		types[] = {PFBYTE, LASTTYPE};

/* The method used to convert the input file This should be either 
 * METH_IDENT if the input file was PFBYTE format, or
 * METH_BYTE  if the input file was anything other than PFBYTE.
 */
int		method;

/* The resize algorithm used.  If slow_resize is TRUE, the image will be
 * resized by interolation (using the HIPS h_stretchimage), if it is set to
 * FALSE, the method used will be replication, which is faster but gives
 * poorer results.  
 * The value can be interactively changed in the adjust command window.
 * It defaults to FALSE (i.e. resize by replication)
 */
h_boolean		slow_resize = FALSE;

/* These are copies of argc, argv as passed into main().  They are used to
 * update the headers before saving them.
 */
int		global_argc;
char		**global_argv;


/* BF 2 */
/* Flag used to check that the image can only be saved to stdout once in a
   a session.
*/
h_boolean	saved_stdout = FALSE;

/* Can read in selected number of frames */
h_boolean		select_frames = FALSE;
int		first_frame, last_frame;
/* Destroys the current image, freeing as much memory as possible ...
   This is only called just before an new image is read in.
 */


/* Same as hips fset_conversion, but does not allocate space for
 * converted image as I will be doing that explicitly.
 */
int	my_fset_conversion(hd, hdp, typeslist, fname)
struct header *hd,*hdp;
int *typeslist;
Filename fname;
{
        int ptype;
	int method;

        if ((ptype = ffind_closest(hd,typeslist,fname)) == HIPS_ERROR)
                return(HIPS_ERROR);
	  if ((method = ffind_method(hd->pixel_format,ptype,fname)) ==
HIPS_ERROR)
                return(HIPS_ERROR);
        dup_header(hd,hdp);
        if (method == METH_IDENT)
                return(method);
        hd->imdealloc = FALSE;
        setformat(hdp,ptype);
        if (method < 0)
                perr(HE_CONVI,hformatname_f(hd->pixel_format,ptype),
                        hformatname_t(hd->pixel_format,ptype),fname);
        else
                perr(HE_CONV,hformatname_f(hd->pixel_format,ptype),
                        hformatname_t(hd->pixel_format,ptype),fname);
        return(method);
}

	
void	destroy_current_image()
{
	register int	i;
	if (valid_image == TRUE)
	{
			
		for (i=0; i< no_frames; i++)
			free(orig_frames[i]);
		if (resized_frames != orig_frames)
			for (i=0; i< no_frames; i++)
				free(resized_frames[i]);
		if ((extract_frames != orig_frames) && (extract_frames !=
		resized_frames))
			for (i=0; i< no_frames; i++)
				free(extract_frames[i]);

		free_header(&resized_header);
		free_header(&orig_header);
		free_header(&extract_header);
		free_header(&input_header);

		free(orig_frames);

		if (resized_frames != orig_frames)
			free(resized_frames);

		if ((extract_frames != orig_frames) && (extract_frames !=
		resized_frames))
			free(extract_frames);

		/* Clear image from window */
		XClearWindow(display, canvas_xwin);
	}
}


/*
 * Load an image from the given file name.
 * Convert if required ...
 * At the end, orig_frames contains a pointer to each frame, and
 * resized_frames and extract_frames all point to orig_frames
 *
 * If an error occurs before the header is read, the previous image will
 * remain the current image.  After the header has been read, an error will
 * mean there is no current image.
 *
 * Returns FALSE if the file was loaded incorrectly, so that the Load
 * File command window can be kept up so the user can try again.
 * Otherwise, returns TRUE.
 */

/* BF: 5.  Added extra startup boolean.  Can only load from stdin at
 * startup. Can never load from stdout.
 */
h_boolean	load_file(new_file, startup)
char	*new_file;
h_boolean	startup;
{
	register FILE	*input_file;
	struct stat	file_status;
	register int	i;
	h_boolean		error_occurred;
	h_boolean	result;

	if ( (strcmp(new_file, "stdin") == 0) || 
		((strcmp(new_file, "<stdin>") == 0) && (startup == FALSE)))
	{
		report_error("You can only load a file from stdin when \
starting up xanim, using redirection and pipes.");
		return(FALSE);
	}
	if ((strcmp(new_file, "stdout") == 0))
	{
		report_error("You cannot load a file from the stdout.");
		return(FALSE);
	}

	/* Check status of file ... */
	if (strcmp(new_file, "<stdin>") != 0)
		if ((stat(new_file, &file_status) == -1) ||
			 ((file_status.st_mode&S_IFMT) != S_IFREG) ||
			 (access(new_file, R_OK) == -1))
		{
			sprintf(info,"Cannot open file %s with read permission.",new_file);
			report_error(info);
			return(FALSE);		
		}
	
	/*
	if (!input_file) 
	input_file = hfopenr(new_file);
	*/
	if ((input_file = hfopenr(new_file)) == (FILE *)HIPS_ERROR)
	{
		sprintf(info,"Failed to open file: %s", new_file);
		report_error(info); 
		return(FALSE);
	}

	/* Up till now, any errors resulted in existing file still being
	   displayed.  After this point, existing image has been destroyed
	   so any errors result in no image being displayed, so all options
	   are disabled, except load and quit.
	*/

	print_message("Loading ...");
	/* Change hips <stdin> to stdin */
	if (strcmp(new_file, "<stdin>") == 0)
		strcpy(filename, "stdin");
	else
		strcpy(filename, new_file);
	
	
	destroy_current_image();


	/* Set changed flag to false, and disable revert option */
	disable_revert();
	
	/* Read the header from the file, and allocate memory for one
	   image in input_header
	*/
	if (fread_hdr_a(input_file,&input_header,filename) == HIPS_ERROR)
	{
		sprintf(info,"File '%s' has an invalid HIPS, \
header, the image was not loaded successfully");
		report_error(info);
		disable_options(filename);
		fclose(input_file);
		print_message("");
		return(FALSE);
	}
	if (input_header.numcolor > 1 || type_is_col3(&input_header))
	{
		report_error("Can't handle color images");
		disable_options(filename);
		fclose(input_file);
		print_message("");
		return(FALSE);
	}


	/* Set up image dimensions */
	image_height = input_header.orows;
	image_width = input_header.ocols;
	image_size = image_width*image_height;

	no_frames = input_header.num_frame;
	if (select_frames)
	{
		h_boolean changed = FALSE;
		if (first_frame < 0)
		{
			changed = TRUE;
			first_frame = 0;
		}
		else if (first_frame >= no_frames)
		{
			changed = TRUE;
			first_frame = no_frames-1;
		}
		if (last_frame == -999)
			last_frame = no_frames-1;

		if (last_frame < first_frame)
		{
			changed = TRUE;
			last_frame = first_frame;
		}
		else if (last_frame >= no_frames)
		{
			changed = TRUE;
			last_frame = no_frames-1;
		}
		
		no_frames = last_frame - first_frame+1;

		if (changed)
		{
			sprintf(info,"Invalid frame range specified. Reading frames %d to %d\n", first_frame, last_frame);
			report_error(info);
		}
		input_header.num_frame = no_frames;
	}
	else
	{
		first_frame = 0;
		last_frame = no_frames -1;
	}
	set_frames(first_frame, last_frame);
		
	method = my_fset_conversion(&input_header,&orig_header,types,filename);
	if (method == HIPS_ERROR)
	/* XXX
	if ((method != METH_BYTE) && (method != METH_IDENT))
	*/
	{
		report_error("Could not convert this image to byte format \
so it cannot be displayed");
		fclose(input_file);
		disable_options(filename); 
		print_message("");
		return(FALSE);
	}

	/* Free memory allocated to orig_header, as it will be explicitly
	   allocated for each frame read.
	*/
	/*
	free_image(&orig_header);
	*/

	/* Allocate memory for the array which contains pointers to each
	  frame.
	*/
	orig_frames = (frame_ptr *)calloc(no_frames, sizeof(frame_ptr));
	if (orig_frames==NULL)
	{
		report_error("Could not allocate memory to read in frames");
		fclose(input_file);
		disable_options(filename);
		print_message("");
		return(FALSE);
	}


	/* Now read in each frame ... */
	i = 0;
	no_frames = 0;
	error_occurred = FALSE;
	while((error_occurred == FALSE) && (i<= last_frame))
	{
		/* Allocate memory for next frame */
		if (alloc_image(&orig_header) == HIPS_ERROR)
		{
			/* Ran out of memory, continue with the frames
			   already read in.
			*/
			error_occurred = TRUE;
			if (no_frames>0)
				sprintf(info,"Ran out of memory when \
loading the image, there is only enough memory to continue with %d frames",no_frames);
			else
				/* No frames were successfully read in */
				strcpy(info,"Ran out of memory before any  frames were successfully read in");
			continue;
		}


		if (i<first_frame) 
			result = fread_image(input_file, &input_header,i,filename);
		else
		/* Read in image, converting it if required.
		 * Resulting image is in orig_header->image.
		 */
			result = fread_imagec(input_file, &input_header, &orig_header, method, i, filename);

		if (result == HIPS_ERROR)
		{
			error_occurred = FALSE;
			if (no_frames>0)
				sprintf(info,"Error reading frame %d \
of %s, will continue with %d frames\n",i, filename, no_frames);
			else
				sprintf(info,"Error reading file %s, \
no frames were successfully loaded");
			continue;
		}

		if (i >=  first_frame)
		{
			orig_frames[no_frames] = orig_header.image;
			no_frames++;
		}
		i++;
	}

	if (error_occurred)
	{
		report_error(info);
		if (no_frames > 1 )
			orig_header.num_frame = no_frames;
		else
		{
				free(orig_frames);
				free_image(&orig_header);
				disable_options();
				fclose(input_file);
				print_message("");
				return(FALSE);
		}
	}

	/* Originally the extracted and resized frames are the same as the
	   orignal frames
	*/
	extract_frames= resized_frames = orig_frames;

	/* From now on, header does not control image allocation/deallocation*/
	orig_header.imdealloc = FALSE;
	dup_headern(&orig_header,&resized_header);
	dup_headern(&orig_header,&extract_header);

	current_frame = 0;

	/* RF: 1.  May be using 64 colors ... so need to modify image */
	/* Test for substitution in sub-routine ... */
	modify_image_colors();

	/* enable all options ... and update all command windows. */
	enable_options(filename);
	update_frame(image_width, image_height, TRUE);

	update_hdr(orig_header,filename);
	update_animation(orig_header,filename);
	update_adjust(filename);

	fclose(input_file);
	render_image();
	print_message("");
	return(TRUE);
}

/* Change the version just saved to be the original, that will be
   "Reverted" to on request.
*/
void	make_saved_original(new_file)
char	*new_file;
{
	register int i;

	if (orig_frames != resized_frames)
	{
		for(i=0; i<no_frames; i++)
			free(orig_frames[i]);
		free(orig_frames);
		
		dup_headern(&resized_header, &orig_header);
		orig_frames = resized_frames;
	}

	if (extract_frames != orig_frames)
	{	
		for (i=0;i<no_frames;i++)
			free(extract_frames[i]);
		free(extract_frames);
		dup_headern(&orig_header,&extract_header);
		extract_frames = orig_frames;
	}

	disable_revert();
	if (strcmp(filename, new_file) != 0)
	{
		strcpy(filename, new_file);
		enable_options(filename);
		update_hdr(orig_header,filename);
		update_animation(orig_header,filename);
		update_adjust(filename);
	}
}

/* Save the current frame as an image with a single frame */
h_boolean	do_save_frame(fp, file_name)
FILE	*fp;
char	*file_name;
{
	struct header	single_header, convback_header;

	xv_set(frame, FRAME_BUSY, TRUE, NULL);

	sprintf(info,"Saving frame %d", current_frame);
	print_message(info);
	
	/* Create a header with a single frame which is the current frame
	*/
	dup_headern(&resized_header, &single_header);
	single_header.num_frame = 1;
	if (alloc_image(&single_header) == HIPS_ERROR)
	{
		sprintf(info,"Did not have enough memory to save current \
frame to file %s",file_name);
		report_error(info);
		fclose(fp);
		print_message("");
		return(FALSE);
	}

	/* Copy the values from the current frame to the single frame,
	 * adjusting them for the current contrast and brightness
	 */
	 set_cb_single_frame(single_header.image, current_frame);

	/* Update header to include information about xanim */
	if (update_header(&single_header, global_argc, global_argv) == HIPS_ERROR)
	{
		sprintf(info,"Error occurred whilst updating header for %s\n", file_name); 
		report_error(info);
		fclose(fp);
		free_image(&single_header);
		xv_set(frame, FRAME_BUSY, FALSE, NULL);
		print_message("");
		return(FALSE);
	}

	/* Write header, convertinf format back if required */
	if (hips_convback == TRUE)
	{
		setupconvback(&input_header,&single_header, &convback_header);
		if (fwrite_header(fp, &convback_header, file_name) == HIPS_ERROR)
		{
			sprintf(info,"Error occurred whilst \
writing header to %s\n", file_name); 
			report_error(info);
			fclose(fp);
			xv_set(frame, FRAME_BUSY, FALSE, NULL);
			free_image(&single_header);
			free_header(&convback_header);
			print_message("");
			return(FALSE);
		}
	}
	else
		if (fwrite_header(fp, &single_header, file_name) ==
		HIPS_ERROR)
		{
			sprintf(info,"Error occurred whilst \
writing header to %s\n", file_name); 
			report_error(info);
			xv_set(frame, FRAME_BUSY, FALSE, NULL);
			free_image(&single_header);
			free_header(&convback_header);
			fclose(fp);
			return(FALSE);
		}

	/* set_contrast_brightness_current_frame(); */
	/* Write (and convert) image */
	if (fwrite_imagec(fp, &convback_header,&single_header, method, hips_convback, file_name) == HIPS_ERROR)
	{
		sprintf(info,"Error occurred whilst saving header to file %s\n",file_name);
		report_error(info); 
		fclose(fp); 
		xv_set(frame, FRAME_BUSY, FALSE, NULL);
		free_image(&single_header);
		free_header(&convback_header);
		print_message("");
		return(FALSE);
	}

	fclose(fp);
	free_image(&single_header);
	free_header(&convback_header);
	print_message("");
	xv_set(frame, FRAME_BUSY, FALSE, NULL);
	return(TRUE);
}

/* Save the the whole image to file, converting back if required */
h_boolean	do_save_file(fp, file_name)
FILE	*fp;
char	*file_name;
{
	register int i;
	
	struct header convback_header;
	struct header tmp_header;

	xv_set(frame, FRAME_BUSY, TRUE, NULL);

	print_message("Saving image ...");
	if (update_header(&resized_header,global_argc, global_argv) ==
	HIPS_ERROR)
	{
		sprintf(info,"Error occurred whilst saving header to file %s\n",file_name);
		report_error(info);
		fclose(fp);
		xv_set(frame, FRAME_BUSY, FALSE, NULL);
	}

	if (hips_convback == TRUE)
	{
		/* here  .... */
		setupconvback(&input_header,&resized_header,
					&convback_header);
		/* So conback_header is now the same as resized_header,
		   except it has the same pixel_format as input_header
		*/
		if (fwrite_header(fp, &convback_header, file_name) == HIPS_ERROR)
		{
			sprintf(info,"Error occurred whilst saving header to file %s\n",file_name);
			report_error(info);
			fclose(fp);
			xv_set(frame, FRAME_BUSY, FALSE, NULL);
			free_header(&convback_header);
			return(FALSE);
		}
		dup_headern(&resized_header, &tmp_header);
	}
	else
	{

		if (fwrite_header(fp, &resized_header, file_name) == HIPS_ERROR)
		{
			sprintf(info,"Error occurred whilst saving header to file %s\n",file_name);
			report_error(info);
			fclose(fp);
			xv_set(frame, FRAME_BUSY, FALSE, NULL);
			print_message("");
			return(FALSE);
		}
	}

	/* If the contrast and brightness has been changed, update
	   frames appropriately.
	*/
	set_contrast_brightness();
	for(i=0; i<no_frames; i++)
	{
		if (hips_convback == TRUE)
		{
			tmp_header.image = resized_frames[i];
			/* or here ... */
			setupconvback(&input_header,&tmp_header, &convback_header);
			printf("Conveting bavck \n");
			if (fwrite_imagec(fp, &convback_header,&tmp_header, method, hips_convback, file_name) == HIPS_ERROR)
			{
				sprintf(info,"Error occurred whilst saving header to file %s\n",file_name); 
				report_error(info); 
				fclose(fp); 
				xv_set(frame, FRAME_BUSY, FALSE, NULL);
				print_message("");
				free_header(&convback_header);
				return(FALSE);
			}
		}
		else
		{
			if (fwrite(resized_frames[i],image_size,1,fp) != 1)
			{
				sprintf(info,"Error occurred whilst saving to file %s\n",file_name);
				report_error(info);
				fclose(fp);
				xv_set(frame, FRAME_BUSY, FALSE, NULL);
				print_message("");
				return(FALSE);
			}
		}
	}
	fclose(fp);
	free_image(&convback_header);
	make_saved_original(file_name);
	print_message("");
	xv_set(frame, FRAME_BUSY, FALSE, NULL);
	return(TRUE);
}

/* Saves the image to a new file.
 * Initiate the save, by checking access permsions etc. on the file
*/
h_boolean	save_file_as(file_name, save_frame)
char	*file_name;
h_boolean	save_frame;
{
	register FILE	*fp;
	struct stat		file_status;

	if (strcmp(file_name, "stdin") == 0)
	{
		report_error("You cannot save a file to the stdin.");
		return(FALSE);
	}


	if (strcmp(file_name,"stdout") == 0)
	{
		/* BF: 2 */
		if (saved_stdout == TRUE)
		{

			report_error("You can only save to stdout once in \
a session, try saving to a file instead."); 
			return(FALSE);
		}
		else
		{
			fp = stdout;
			saved_stdout = TRUE;
		}
	}
	else
	{	
		if (stat(file_name, &file_status) != -1) 
		{
		/* file exists ... */
		if ((file_status.st_mode&S_IFMT) == S_IFREG) 
		{
			
			int result;

			sprintf(info,"File %s already exists, do \
you wish to overwrite it?",file_name);

			result = notice_prompt(frame, NULL,
				NOTICE_MESSAGE_STRINGS, info, NULL,
				NOTICE_BUTTON_YES,      "Yes",
				NOTICE_BUTTON_NO,       "No",
			   NULL);
			if (result == NOTICE_NO)
				return(FALSE);

			if (access(file_name, W_OK) == -1)
			{
				sprintf(info,"Invalid permission to save file as: %s",file_name);
				report_error(info);
				return(FALSE);
			}
		}
		else 
		{
			/* File exists ... but is not regular file */ 
			sprintf(info,"Cannot save  file as: %s",file_name);
			report_error(info);
			return(FALSE);
		}
		}
		fp = fopen(file_name,"w");
	}

	if (fp == NULL)
	{
		sprintf(info,"Cannot open file: %s for writing",file_name);
		report_error(info);
		return(FALSE);
	}
	if (save_frame)
		return(do_save_frame(fp, file_name));
	else
		return(do_save_file(fp, file_name));
}


/* Save the image to the file it was read from, cheking first for write
   permision to that file
*/
h_boolean	save_file(file_name)
char	*file_name;
{
	FILE	*fp;

	if (access(file_name, W_OK) == -1)
	{
		sprintf(info,"Cannot save  file as : %s",file_name);
		report_error(info);
		return(FALSE);
	}
	fp = fopen(file_name,"w");
	if (fp == NULL)
	{
		sprintf(info,"Cannot open file: %s for writing",file_name);
		report_error(info);
		return(FALSE);
	}
	return(do_save_file(fp, file_name));
}



/* Resize each frame, using the HIPS h_stretchimage (interpolation) method.  
 * This is used when slow_resize is TRUE.
*/
void	resize_frames(new_width, new_height)
int 	new_width, new_height;
{
	register int		i,j;

	/* Resize each frame */
	for (i=0; i<no_frames; i++)
	{
		if (extract_frames != resized_frames)
			free(resized_frames[i]);

		if ((resized_frames[i] = (byte *)malloc((unsigned)image_size))==NULL)
		{
			report_error("There is not enough memory to resize the frame[s]");
			for (j=0; j<no_frames; j++)
				free(resized_frames[j]);
			free(resized_frames);
			resized_frames = extract_frames;
			dup_headern(&extract_header, &resized_header);
			image_size = image_width*image_height;
			return;
		}
		else
		{
			h_stretchimg_B(extract_frames[i],resized_frames[i],
				extract_header.orows,
				extract_header.ocols,
				extract_header.ocols,
				resized_header.orows,
				resized_header.ocols,
				resized_header.ocols);
		}
	}
	image_width = new_width;
	image_height = new_height;
	enable_revert();
	update_hdr(resized_header,filename);
	render_image();
}

/* Resize each frame using replication algorithm.  This is called when
   slow_resize is FALSE.
*/
void fast_resize_frames(new_width,new_height)
int new_width, new_height;
{

	register int		i,j, new_y, x, y, *row_arr, *row_arr_ptr;
	register byte		*col_ptr, *row_ptr, *el_ptr;

	if ((row_arr = (int *) malloc(new_width * sizeof(int))) == NULL)
	{
		report_error("Could not allocate enough memory to resize the image.");
		return;
	}
	for (x=0; x<new_width; x++) 
		row_arr[x] = (extract_header.ocols * x) / new_width;

	/* Reszie each frame */
	for (i=0; i< no_frames; i++)
	{
		if (extract_frames != resized_frames)
			free(resized_frames[i]);

		if ((resized_frames[i] = (frame_ptr)malloc((unsigned)image_size))==NULL)
		{
			report_error("There is not enough memory to resize the frame[s]");
			for (j=0; j<no_frames; j++)
				free(resized_frames[j]);
			free(resized_frames);
			resized_frames = extract_frames;
			dup_headern(&extract_header, &resized_header);
			image_size = image_width*image_height;
			free(row_arr);
			return;
		}
		else
		{
			row_ptr = el_ptr = resized_frames[i];
			for (y=0;  y<new_height;  y++, row_ptr+=new_width) 
			{
				new_y = (extract_header.orows * y) / new_height;
				el_ptr = row_ptr;
				col_ptr = extract_frames[i] + (new_y * extract_header.ocols);
				for (x=0, row_arr_ptr = row_arr;  x<new_width;  
							x++, el_ptr++) 
					*el_ptr = col_ptr[*row_arr_ptr++];
			}
		}
	}
	image_width = new_width;
	image_height = new_height;
	enable_revert();
	update_hdr(resized_header,filename);
	free(row_arr);
	render_image();
}

void	resize_image(new_width, new_height)
int	new_width, new_height;
{

	if (((image_width == new_width)  && (image_height == new_height)) 
		|| (valid_image == FALSE))
		return;

	if (resized_frames == extract_frames)
	{
		/* Allocate separate "modified" frames ... */
		resized_frames = (frame_ptr *)calloc(no_frames, sizeof(frame_ptr));
		if (resized_frames==NULL)
		{
			report_error("Could not allocate memory to resize frames"); 
			return;
		}
	}

	print_message("Resizing Image ...");
	xv_set(frame, FRAME_BUSY, TRUE, NULL);
	image_size = new_width * new_height;

	/* Maintain correct dimensions for header */
	setsize(&resized_header, new_height, new_width);

	if (slow_resize == TRUE)
		resize_frames(new_width,new_height);
	else
		fast_resize_frames(new_width, new_height);

	print_message("");
	xv_set(frame, FRAME_BUSY, FALSE, NULL);
}

/* Revert the image to the one orignally read in, stored in orig_frames */
void	revert_image()
{
	register int	i;

	if (resized_frames != orig_frames)
	{	
		for (i=0;i<no_frames;i++)
			free(resized_frames[i]);
		free(resized_frames);
	}

	if ((extract_frames != orig_frames) && (extract_frames !=
	resized_frames))
	{	
		for (i=0;i<no_frames;i++)
			free(extract_frames[i]);
		free(extract_frames);
	}
	dup_headern(&orig_header,&resized_header);
	dup_headern(&orig_header,&extract_header);

	resized_frames = extract_frames = orig_frames;

	image_width = orig_header.ocols;
	image_height = orig_header.orows;
	image_size = image_width*image_height;
	update_frame(image_width, image_height, TRUE);
	update_hdr(orig_header,filename);

	revert_colors();
	disable_revert();
	render_image();
}

/* Extract a portion of the image from the resized image.  Store the
 * results in the extract frames.  Resized frames is set to be the same
 * as the extracted frames.
*/ 
void	do_extract(x, y, new_width, new_height)
int x, y, new_width, new_height;
{

	register frame_ptr	new_image, old_image;
	register int		col, row, start_row;
	register int		i,j;

	/* Resize each frame */
	for (i=0; i<no_frames; i++)
	{
		old_image = resized_frames[i];
		if ((extract_frames != resized_frames) && (extract_frames
		!= orig_frames))
			free(extract_frames[i]);

		if ((extract_frames[i] = (frame_ptr)malloc((unsigned)image_size))==NULL)
		{
			report_error("There is not enough memory to extract the frame[s]");
			for (j=0; j<no_frames; j++)
				free(extract_frames[j]);
			free(extract_frames);
			extract_frames = resized_frames;
			dup_headern(&resized_header, &extract_header);
			image_size = image_width*image_height;
			return;
		}

		new_image = extract_frames[i];
		for(row =0; row < new_height; row++)
		{
			start_row = (image_width*(row+y)) + x;
			for (col = 0; col< new_width; col++)
				*new_image++ = old_image[start_row + col];
		}
		if (resized_frames != orig_frames)
			free(old_image);
	}

	/* Set resize_header and frames to be same a extract header and
	   and frames
	*/

	/* Extract frames have been freed, and resized frames have beend
	   freed if they are not the same as orig_frames ....
	   Just left to free resized_frames ptrs
	*/

	if (resized_frames  != orig_frames)
		free(resized_frames);

	resized_frames = extract_frames;
	dup_headern(&extract_header, &resized_header);
	image_width = new_width;
	image_height = new_height;
	enable_revert();
	update_frame(image_width, image_height, TRUE);
	update_hdr(extract_header,filename);
	render_image();
}

void	extract(x, y, new_width, new_height)
int	x, y, new_width, new_height;
{
	if (((x == 0) && (y == 0) && (image_width == new_width)  && (image_height == new_height)) 
		|| (valid_image == FALSE))
		return;

	image_size = new_width * new_height;

	if (extract_frames == orig_frames)
		/* Allocate separate "modified" frames ... */
		extract_frames = (frame_ptr *)calloc(no_frames, sizeof(frame_ptr));
		if (extract_frames == NULL)
		{
			report_error("There is not enough memory to extract a portion of the image");
			return;
		}

	print_message("Extracting ...");
	xv_set(frame, FRAME_BUSY, TRUE, NULL);

	/* Maintain correct dimensions for header */
	setsize(&extract_header, new_height, new_width);

	do_extract(x, y, new_width, new_height);

	print_message("");
	xv_set(frame, FRAME_BUSY, FALSE, NULL);
}

/* Creates an X-image from the current frame, and displays it */
void	render_image()
{
	XImage			*image;
	if (valid_image == TRUE)
	{
		image = XCreateImage(display, visual, 8, ZPixmap, 0, 
			(char *)resized_frames[current_frame], image_width, image_height, 
		8, 0);
		XPutImage(display, canvas_xwin, image_gc, image, 0, 0, 0, 0,
		image_width, image_height);
		print_new_pixel_value();
	}
}
