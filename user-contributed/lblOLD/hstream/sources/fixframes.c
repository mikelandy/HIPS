
/*
 *  fixframes.c  : correct the number of frames in a HIPS stream file
 *
 *  This program must be used with hvideo when writing to stdout
 *
 *
 * Usage:       fixframes infile outfile
 *
 *  Brian Tierney, LBL
 */

#include <hipl_format.h>
#include <stdio.h>

/*
#define DEBUG
*/

main(argc, argv)
    int       argc;
    char    **argv;

{
    struct header hd1, hd2;
    sframe_header fhd;
    int       frame;
    int       stream_type;
    Filename  filename1, filename2;
    FILE     *fp1, *fp2;
    stream_info sinfo;
    int       get_sinfo();
    long      data_end_loc, data_start_loc;

    Progname = strsave(*argv);
    hipserrlev = HEL_SEVERE;	/* only exit if severe errors */

    if (argc != 2 && argc != 3) {
	fprintf(stderr, "Usage:  %s infile outfile \n\n", argv[0]);
	exit(0);
    }
    if (argc == 2) {
	filename1 = "<stdin>";
	filename2 = argv[1];
    } else {
	filename1 = argv[1];
	filename2 = argv[2];
    }
    fprintf(stderr, "%s: Input file= %s;  Output file= %s \n", argv[0],
	    filename1, filename2);

    fp1 = hfopenr(filename1);
    fp2 = ffopen(filename2, "w+");

    fread_header(fp1, &hd1, filename1);

    stream_type = get_sinfo(&sinfo, 0);
    if (!stream_type) {
	fprintf(stderr, "This program for stream files only! \n");
	exit(0);
    }
#ifdef HAVE_JPEG
    if (sinfo.comp_type == 1) {
	if (findparam(&hd1, "JPEG-info") != NULLPAR) {
	    int       two = 2, *xjpeg;
	    getparam(&hd1, "JPEG-info", PFINT, &two, &xjpeg);
	    sinfo.qfactor = xjpeg[0];
	    sinfo.dsize = xjpeg[1];
	}
    }
#endif

    dup_headern(&hd1, &hd2);
    set_stream_param(&hd2, stream_type, sinfo.comp_type, sinfo.fps,
		     sinfo.qfactor, sinfo.dsize);
    fwrite_header(fp2, &hd2, filename2);

    for (frame = 0; frame < hd1.num_frame; frame++) {
	fprintf(stderr, "%s: frame %d \n", argv[0], frame);

	if (sinfo.comp_type) {
	    if (copy_jfif_header(fp1, fp2) != HIPS_OK) {
		break;
	    }
	}
	if (copy_stream_header(fp1, fp2, &hd1, &fhd, frame) != HIPS_OK) {
	    break;
	}
	hd1.sizeimage = fhd.size;
	if (sinfo.comp_type && hd1.image != NULL) {
	    free(hd1.image);
	    hd1.image = NULL;
	}
	alloc_image(&hd1);
	if (fread(hd1.image, fhd.size, 1, fp1) != 1) {
	    fprintf(stderr, "%s error: reading frame data, file %s\n",
		    argv[0], filename1);
	    break;
	}
	if (fwrite(hd1.image, fhd.size, 1, fp2) != 1) {
	    fprintf(stderr, "%s error: writing frame data, file %s\n",
		    argv[0], filename2);
	}
    }

    fprintf(stderr, "found %d frames in this file, fixing header... \n", frame);

    fflush(fp2);
    data_end_loc = ftell(fp2);

    fseek(fp2, (long) 0, 0);	/* to beggining */
    fread_header(fp2, &hd2, filename2);
    data_start_loc = ftell(fp2);

    hd2.num_frame = frame;

    sinfo.dsize = (int) (data_end_loc - data_start_loc);
    set_stream_param(&hd2, stream_type, sinfo.comp_type, sinfo.fps,
		     sinfo.qfactor, sinfo.dsize);

    fseek(fp2, (long) 0, 0);	/* to beggining */
    fwrite_header(fp2, &hd2, filename2);
    fclose(fp1);
    fclose(fp2);

    exit(0);

}

/**********************************************************************/
copy_jfif_header(fp1, fp2)
    FILE     *fp1, *fp2;
{
    int       skip = 0, i;
    register int c;

    /*
     * NOTE: for frame 0, this also copies the initial header and Q-tables
     */

    while (1) {
	c = fgetc(fp1);
	skip++;
	if (c == EOF) {		/* dont write the EOF */
	    fprintf(stderr, "Found EOF \n");
	    return (-1);
	}
	fputc(c, fp2);
	if (c == 0xFF) {
	    c = fgetc(fp1);
	    fputc(c, fp2);
	    skip++;
	    if (c == 0xDA)
		break;
	}
    }
    c = fgetc(fp1);
    fputc(c, fp2);
    skip++;

#ifdef DEBUG
    fprintf(stderr, "copied %d bytes of JFIF header \n", skip);
#endif

    c = fgetc(fp1);
    fputc(c, fp2);
    skip = c - 2;

    for (i = 0; i < skip; i++) {
	c = fgetc(fp1);
	fputc(c, fp2);
    }
#ifdef DEBUG
    fprintf(stderr, "copied %d more bytes of JFIF header \n", skip);
#endif

    return HIPS_OK;
}


/***********************************************************/
copy_stream_header(fp1, fp2, hd, fhd, frame)
    FILE     *fp1, *fp2;
    struct header *hd;
    sframe_header *fhd;
    int       frame;
{
    register int i, s;

    if (read_stream_header(fp1, hd, fhd, frame) == HIPS_ERROR)
	return (HIPS_ERROR);

    if (write_stream_header(fp2, hd, fhd, frame) == HIPS_ERROR)
	return (HIPS_ERROR);

    return HIPS_OK;
}
