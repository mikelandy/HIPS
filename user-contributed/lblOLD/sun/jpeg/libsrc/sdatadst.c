/* sdatadst.c
 *
 * Copyright (C) 1994, Jin Guojun
 */

#include "jinclude.h"
#include "jpeglib.h"		/* not jpegint.h; this is not a core module */
#include "jerror.h"		/* get error codes too */


/* Expanded data destination object for stdio output */

typedef struct {
	struct jpeg_destination_mgr pub;	/* public fields */

	FILE*	outfile;		/* target stream */
	int	buffer_size;
} stream_destination_mgr;

typedef stream_destination_mgr * stream_dest_ptr;


/*
 * Initialize destination --- called by jpeg_start_compress
 * before any data is actually written.
 */

METHODDEF void
init_destination (j_compress_ptr cinfo)
{
stream_dest_ptr dest = (stream_dest_ptr) cinfo->dest;
U_IMAGE	*img = cinfo->img;

/* application has to allocate image_size + JPEG_header_size buffer, and
	this routine puts header in tail space (buffer + image_size)	*/
  dest->pub.next_output_byte = (JOCTET *) img->dest + (dest->buffer_size =
  dest->pub.free_in_buffer = img->height * img->width * 3);
}


METHODDEF boolean
empty_output_buffer (j_compress_ptr cinfo)
{
register stream_dest_ptr dest = (stream_dest_ptr) cinfo->dest;
register U_IMAGE *img = cinfo->img;
register int	oszie = dest->buffer_size - dest->pub.free_in_buffer;

  if (img->o_form != IFMT_STREAM || (JFWRITE(dest->outfile,
	dest->pub.next_output_byte - oszie, oszie) != oszie))
    ERREXIT(cinfo, JERR_FILE_WRITE);

  img->tmp_offset += oszie;
  dest->pub.next_output_byte = img->dest;
  dest->pub.free_in_buffer = dest->buffer_size;

  return TRUE;
}


/*
 * Terminate destination --- called by jpeg_finish_compress
 * after all data has been written.  Usually needs to flush buffer.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

METHODDEF void
term_destination (j_compress_ptr cinfo)
{
stream_dest_ptr dest = (stream_dest_ptr) cinfo->dest;
size_t datacount = dest->buffer_size - dest->pub.free_in_buffer;

	/* Write any data remaining in the buffer.
		free_in_buffer < 0 means to keep remaining data	*/
  if (datacount > 0 && dest->pub.free_in_buffer > 0) {
    if (JFWRITE(dest->outfile, cinfo->img->dest, datacount) != datacount)
      ERREXIT(cinfo, JERR_FILE_WRITE);
#ifdef	SOI_SOF_SOS_EOI
    cinfo->img->tmp_offset += datacount;	/* not for stream	*/
#endif
  }
  fflush(dest->outfile);
  /* Make sure we wrote the output file OK */
  if (ferror(dest->outfile))
    ERREXIT(cinfo, JERR_FILE_WRITE);
}


/*
 * Prepare for output to a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing compression.
 */

GLOBAL void
stream_stdio_dest (j_compress_ptr cinfo, FILE * outfile)
{
stream_dest_ptr dest;

  /* The destination object is made permanent so that multiple JPEG images
   * can be written to the same file without re-executing jpeg_stdio_dest.
   * This makes it dangerous to use this manager and a different destination
   * manager serially with the same JPEG object, because their private object
   * sizes may be different.  Caveat programmer.
   */
  if (cinfo->dest == NULL) {	/* first time for this JPEG object? */
    cinfo->dest = (struct jpeg_destination_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  SIZEOF(stream_destination_mgr));
  }

  dest = (stream_dest_ptr) cinfo->dest;
  dest->pub.init_destination = init_destination;
  dest->pub.empty_output_buffer = empty_output_buffer;
  dest->pub.term_destination = term_destination;
  dest->outfile = outfile;
}

GLOBAL
stream_jpeg_dst_size(j_compress_ptr cinfo)
{
stream_dest_ptr	dest = (stream_dest_ptr) cinfo->dest;
	return	dest->buffer_size - dest->pub.free_in_buffer;
}

