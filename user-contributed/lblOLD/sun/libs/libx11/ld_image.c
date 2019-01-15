/*	X	LoaD_IMAGE . C
#
%	Copyright (c)	see included header files
%
% AUTHOR:	Jin Guojun - LBL	10/1/91
*/

#include "function.h"

Image*
dup_mv_hd(img, previous)	/* is to duplicate all faster ?	*/
Image	*img, *previous;	/* ERROR if no EXTENDED_COLOR	*/
{
#if	(INT_BITS != POINTER_BITS)
#define	NCP1	10
#else
#define	NCP1	9	/* must be 9 !!	*/
#endif
#define	cpl1	((sizeof(int) + sizeof(cookie_t)) * NCP1)
#define	cpl2	((sizeof(int)<<1) + sizeof(cookie_t)) << 2
#define INHERIT(thing)	img->thing = previous->thing
		memcpy(&img->colormap, &previous->colormap, cpl1);
		memcpy(&img->dpy_depth, &previous->dpy_depth, cpl1);
		memcpy(&img->in_type, &previous->in_type, cpl2);
		memcpy(&img->map_scanline, &previous->map_scanline, cpl2);
		if (stingy_flag)	img->image = NULL;
		INHERIT(fn);	/* for movie only!	*/
		INHERIT(icn_pixmap);
		INHERIT(divN);
		INHERIT(modN);
		INHERIT(dm16);
		INHERIT(pixel_table);
		INHERIT(lvls);
		INHERIT(lvls_squared);
		INHERIT(visual_class);
#undef	INHERIT
return	previous;
}

/*
%	still_win contains the maximum number of images can be displayed.
%	Once the number of loaded image is greater than the maximum,
%	the new incoming image will be replaced in the same name window,
%	or in the last image window.
*/
#define	INFINIT_F	99999999	/* means no frame info.	*/

LoadGXImage(num_images, parent, pic, movie, win_geom, still_win)
int	*num_images;
Image	*parent, ***pic;
char	*win_geom;
{
int	loaded=0, rle_cnt=rle_dflt_hdr.ncmap=0, stov=0;
XImage	*save_icon_i=NULL;
    Loop {
	Image	*img, *previous_img=NULL;
	int	status, vbs = movie < *num_images;

	if (movie > 1 && (stov || vbs))	{
	    if (vbs)	{	/*	rotating	*/
		img = (*pic)[0];
		previous_img = (*pic)[--*num_images];
		if (img->icon_image)
			save_icon_i = img->icon_image,
			img->icon_image = NULL;
		if (img->frames > 1 && img->frames - rle_cnt < movie)	{
			memcpy(*pic, *pic+1, *num_images * sizeof(*pic));
			(*pic)[0]->frames = img->frames;	/* save frm#s */
			(*pic)[*num_images] = img;
		} else	{	/* start over	*/
			stov++;
			*num_images = 0;
		}
	   }	else	{	/* may save some time?	*/
		previous_img = img;
		img = (*pic)[*num_images];
	    }
	/*  img->fn = rle_cnt - 1;	/* fancy and not very useful	*/
	} else	{
	    if (!(vbs=verify_buffer_size(pic, -sizeof(**pic), *num_images+1, No)))
		prgmerr(RLE_NO_SPACE, "out of memory!");
	    if (vbs != -1)
	    /*	(*pic)[*num_images] = NULL;	below ensure for all machines	*/
		for (vbs=pointer_buffer_size(*pic) / sizeof(**pic);
					vbs > *num_images;)
			(*pic)[--vbs] = NULL;
	    status = *num_images;
	    if (still_win)	while (status--)
		if (!strcmp((*pic)[status]->name, parent->name))	break;

	/* critical condition for still window manipulation	*/
	    if ((!still_win || *num_images < still_win) &&
			!(*pic)[*num_images])	{	/* create a new image */
		img = (*pic)[*num_images] = (Image*)ZALLOC(1, sizeof(*img), "img");

		img->visual_class = parent->visual_class;
		init_img_info(img, Dpy, RLE, parent->dpy);

		/*	we need pixmaps for movie mode	*/
		img->pixmap_failed = parent->pixmap_failed;
		img->gamma = parent->gamma;
		img->name = parent->name;
		img->IN_FP = parent->IN_FP;
		if (((BUFFER*)img->IN_FP)->flags == BUFFER_MAGIC)
			link_buffer(img, 0);
	    } else	{
		if (loaded)	/* only reload one image at a time	*/
		{	rle_cnt = 0;	goto	EOF_case;	}
		--*num_images;	/* re-use the same or last image	*/
		if (status < 0)	status = *num_images;
		previous_img = img = (*pic)[status];
		img->IN_FP = parent->IN_FP;
		/* should we reset it for every image types ?	*/
		if (img->in_type == GIF)	img->in_type = HIPS;
	    }
	    if (movie && *num_images)
		previous_img = dup_mv_hd(img, (*pic)[*num_images-1]);
	    else	init_img_flag(img),	multi_hd = 0;

	    if ((img->frames = rle_cnt) &&
		++multi_hd == 1 &&	/* may be a sequence RLE image	*/
		img->in_type == JPEG)	/* for movie made on JPEG board	*/
		(*pic)[0]->frames = INFINIT_F;	/* JPEG board has no F#	*/
	}

	status = get_pic(*num_images, win_geom, previous_img, *pic,
				parent->frame ? USE_ROOT_WIN : 1);

	{
	register int	mult_f = img->frames > 1 &&
				(img->in_type < RLE || img->in_type > TiFF);
#ifdef	OLD_VER
	register int	i_t = img->in_type;
#endif
	switch (status) {
	case SUCCESS:
		loaded = ++*num_images;	/*	effective load	*/
		if (parent->frame)	CopyToRootWindow(parent, img, No);
#ifdef	OLD_VER
		if (i_t!=HIPS && i_t!=FITS && i_t!=JPEG || movie)	{
#else
		if (movie || !mult_f)	{	/* eqv OLD_VER	*/
#endif
			rle_cnt++;	/* total load	*/
			continue;
		}
		if (mult_f)	{	/* not movies	*/
			verify_buffer_size(&img->data, -img->dpy_channels,
				img->width*img->height << 1, "realloc MultiF");
			(*img->std_swif)(FI_RLOAD_BUF, img, img->data +
				img->width*img->height*img->dpy_channels, OsameI);
			if (!tuner_flag)
				img->scan_data = img->data;
		}
		img->fn++;

	case RLE_EMPTY:
	case RLE_EOF:
	case EOF:	/* = RLE_NOT_RLE	*/
		if ((status==RLE_NOT_RLE || !rle_cnt) && (DEBUGANY | verbose))
			rle_get_error(status, Progname, img->name);
EOF_case:	if (!mult_f)	(*img->r_close)(img->IN_FP);
#ifndef	BSD4
		else if (((BUFFER*)img->IN_FP)->flags != BUFFER_MAGIC)
			fflush(img->IN_FP);	/* kill SVR fork() bug	*/
#endif
		if (rle_cnt /* | !loaded */)
			CFREE(img->marray),
			CFREE(img /* === pic[*num_images] */),
				(*pic)[*num_images] = NULL;
		if (movie)	{
			img = (*pic)[0];
			if (movie > 1)	{
			    if (save_icon_i)
				img->icon_image = save_icon_i;
			    if (/* stov && !vbs &&	*/ movie < *num_images)
				*num_images = movie;
			} else if (img->frames == INFINIT_F)
				img->frames = rle_cnt;
		}
		return	loaded ? mult_f : EOF;	/* report error	*/
	case MALLOC_FAILURE:
	case RLE_NO_SPACE:
		prgmerr(0, "Out of Memory! Trying to continue\n");
		break;
	case FATAL_FAILURE:
		prgmerr(1, movie ? "Can't start movie, sorry ..." : " ");
	}
	}
    }
}
