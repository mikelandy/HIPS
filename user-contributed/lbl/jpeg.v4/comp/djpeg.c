/*
 * jdmain.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains a command-line user interface for the JPEG decompressor.
 * It should work on any system with Unix- or MS-DOS-style command lines.
 *
 * Two different command line styles are permitted, depending on the
 * compile-time switch TWO_FILE_COMMANDLINE:
 *	djpeg [options]  inputfile outputfile
 *	djpeg [options]  [inputfile]
 * In the second style, output is always to standard output, which you'd
 * normally redirect to a file or pipe to some other program.  Input is
 * either from a named file or from standard input (typically redirected).
 * The second style is convenient on Unix but is unhelpful on systems that
 * don't support pipes.  Also, you MUST use the first style if your system
 * doesn't do binary I/O to stdin/stdout.
%
% Modified:     Jin Guojun - LBL, Image Technology Group
%       Date:   April 14, 1992
%       Goal:   To be easily handled by conversion library - CCS (c)
%		HIPS, FITS, GIF, RLE, SUN-raster, PNM, TIFF, PICT ...
%		can be compressed by cjpeg now, directly displayed by tuner,
%		decompressed to other image type by torle, torast, and color_ps.
%		These type of images can be determined by program `headers'.
 */

#include "jinclude.h"
#ifdef INCLUDES_ARE_ANSI
#include <stdlib.h>		/* to declare exit() */
#endif
#include <ctype.h>		/* to declare isupper(), tolower() */
#ifdef NEED_SIGNAL_CATCHER
#include <signal.h>		/* to declare signal() */
#endif
#ifdef USE_SETMODE
#include <fcntl.h>		/* to declare setmode() */
#endif

#ifdef THINK_C
#include <console.h>		/* command-line reader for Macintosh */
#endif

#ifdef DONT_USE_B_MODE		/* define mode parameters for fopen() */
#define READ_BINARY	"r"
#define WRITE_BINARY	"w"
#else
#define READ_BINARY	"rb"
#define WRITE_BINARY	"wb"
#endif

#ifndef EXIT_FAILURE		/* define exit() codes if not provided */
#define EXIT_FAILURE  1
#endif
#ifndef EXIT_SUCCESS
#ifdef VMS
#define EXIT_SUCCESS  1		/* VMS is very nonstandard */
#else
#define EXIT_SUCCESS  0
#endif
#endif


#include "jversion.h"		/* for version message */


#ifndef DEFAULT_FMT		/* so can override from CFLAGS in Makefile */
#define DEFAULT_FMT	FMT_PPM
#endif

extern IMAGE_FORMATS requested_fmt;

/*
 * Signal catcher to ensure that temporary files are removed before aborting.
 * NB: for Amiga Manx C this is actually a global routine named _abort();
 * see -Dsignal_catcher=_abort in CFLAGS.  Talk about bogus...
 */

#ifdef NEED_SIGNAL_CATCHER

static external_methods_ptr emethods; /* for access to free_all */

GLOBAL void
signal_catcher (int signum)
{
  if (emethods != NULL) {
    emethods->trace_level = 0;	/* turn off trace output */
    (*emethods->free_all) ();	/* clean up memory allocation & temp files */
  }
  exit(EXIT_FAILURE);
}

#endif


/*
 * Optional routine to display a percent-done figure on stderr.
 * See jddeflts.c for explanation of the information used.
 */

#ifdef PROGRESS_REPORT

METHODDEF void
progress_monitor (decompress_info_ptr cinfo, long loopcounter, long looplimit)
{
  if (cinfo->total_passes > 1) {
    fprintf(stderr, "\rPass %d/%d: %3d%% ",
	    cinfo->completed_passes+1, cinfo->total_passes,
	    (int) (loopcounter*100L/looplimit));
  } else {
    fprintf(stderr, "\r %3d%% ",
	    (int) (loopcounter*100L/looplimit));
  }
  fflush(stderr);
}

#endif


/*
 * Argument-parsing code.
 * The switch parser is designed to be useful with DOS-style command line
 * syntax, ie, intermixed switches and file names, where only the switches
 * to the left of a given file name affect processing of that file.
 * The main program in this file doesn't actually use this capability...
 */


static char * progname;		/* program name for error messages */

LOCAL void
usage ()
/* complain about bad command line */
{
  fprintf(stderr, "usage: %s [switches] ", progname);
#ifdef TWO_FILE_COMMANDLINE
  fprintf(stderr, "inputfile outputfile\n");
#else
  fprintf(stderr, "[inputfile]\n");
#endif

  fprintf(stderr, "Switches (names may be abbreviated):\n");
  fprintf(stderr, "  -colors N      Reduce image to no more than N colors\n");
#ifdef GIF_SUPPORTED
  fprintf(stderr, "  -gif           Select GIF output format\n");
#endif
#ifdef PPM_SUPPORTED
  fprintf(stderr, "  -pnm           Select PBMPLUS (PPM/PGM) output format (default)\n");
#endif
  fprintf(stderr, "  -quantize N    Same as -colors N\n");
#ifdef	HIPS_IMAGE
  fprintf(stderr, "  -hips          Select HIPS output format\n");
#endif
#ifdef RLE_SUPPORTED
  fprintf(stderr, "  -rle           Select Utah RLE output format\n");
#endif
#ifdef TARGA_SUPPORTED
  fprintf(stderr, "  -targa         Select Targa output format\n");
#endif
  fprintf(stderr, "Switches for advanced users:\n");
#ifdef BLOCK_SMOOTHING_SUPPORTED
  fprintf(stderr, "  -blocksmooth   Apply cross-block smoothing\n");
#endif
  fprintf(stderr, "  -grayscale     Force grayscale output\n");
  fprintf(stderr, "  -nodither      Don't use dithering in quantization\n");
#ifdef QUANT_1PASS_SUPPORTED
  fprintf(stderr, "  -onepass       Use 1-pass quantization (fast, low quality)\n");
#endif
  fprintf(stderr, "  -maxmemory N   Maximum memory to use (in kbytes)\n");
  fprintf(stderr, "  -verbose  or  -debug   Emit debug output\n");
  exit(EXIT_FAILURE);
}


LOCAL boolean
keymatch (char * arg, const char * keyword, int minchars)
/* Case-insensitive matching of (possibly abbreviated) keyword switches. */
/* keyword is the constant keyword (must be lower case already), */
/* minchars is length of minimum legal abbreviation. */
{
  register int ca, ck;
  register int nmatched = 0;

  while ((ca = *arg++) != '\0') {
    if ((ck = *keyword++) == '\0')
      return FALSE;		/* arg longer than keyword, no good */
    if (isupper(ca))		/* force arg to lcase (assume ck is already) */
      ca = tolower(ca);
    if (ca != ck)
      return FALSE;		/* no good */
    nmatched++;			/* count matched characters */
  }
  /* reached end of argument; fail if it's too short for unique abbrev */
  if (nmatched < minchars)
    return FALSE;
  return TRUE;			/* A-OK */
}


LOCAL int
parse_switches (decompress_info_ptr cinfo, int last_file_arg_seen,
		int argc, char **argv)
/* Initialize cinfo with default switch settings, then parse option switches.
 * Returns argv[] index of first file-name argument (== argc if none).
 * Any file names with indexes <= last_file_arg_seen are ignored;
 * they have presumably been processed in a previous iteration.
 * (Pass 0 for last_file_arg_seen on the first or only iteration.)
 */
{
  int argn;
  char * arg;

  /* (Re-)initialize the system-dependent error and memory managers. */
  jselerror(cinfo->emethods);	/* error/trace message routines */
  jselmemmgr(cinfo->emethods);	/* memory allocation routines */
/*  cinfo->methods->d_ui_method_selection = d_ui_method_selection;	*/

  /* Now OK to enable signal catcher. */
#ifdef NEED_SIGNAL_CATCHER
  emethods = cinfo->emethods;
#endif

  /* Set up default JPEG parameters. */
  j_d_defaults(cinfo, TRUE);
  requested_fmt = DEFAULT_FMT;	/* set default output file format */

  /* Scan command line options, adjust parameters */

  for (argn = 1; argn < argc; argn++) {
    arg = argv[argn];
    if (*arg != '-') {
      /* Not a switch, must be a file name argument */
      if (argn <= last_file_arg_seen)
	continue;		/* ignore it if previously processed */
      break;			/* else done parsing switches */
    }
    arg++;			/* advance past switch marker character */

    if (keymatch(arg, "blocksmooth", 1)) {
      /* Enable cross-block smoothing. */
      cinfo->do_block_smoothing = TRUE;

    } else if (keymatch(arg, "colors", 1) || keymatch(arg, "colours", 1) ||
	       keymatch(arg, "quantize", 1) || keymatch(arg, "quantise", 1)) {
      /* Do color quantization. */
      int val;

      if (++argn >= argc)	/* advance to next argument */
	usage();
      if (sscanf(argv[argn], "%d", &val) != 1)
	usage();
      cinfo->desired_number_of_colors = val;
      cinfo->quantize_colors = TRUE;

    } else if (keymatch(arg, "debug", 1) || keymatch(arg, "verbose", 1)) {
      /* Enable debug printouts. */
      /* On first -d, print version identification */
      if (last_file_arg_seen == 0 && cinfo->emethods->trace_level == 0)
	fprintf(stderr, "Independent JPEG Group's DJPEG, version %s\n%s\n",
		JVERSION, JCOPYRIGHT);
      cinfo->emethods->trace_level++;

    } else if (keymatch(arg, "gif", 1)) {
      /* GIF output format. */
      requested_fmt = FMT_GIF;

    } else if (keymatch(arg, "grayscale", 2) || keymatch(arg, "greyscale",2)) {
      /* Force monochrome output. */
      cinfo->out_color_space = CS_GRAYSCALE;

    } else if (keymatch(arg, "maxmemory", 1)) {
      /* Maximum memory in Kb (or Mb with 'm'). */
      long lval;
      char ch = 'x';

      if (++argn >= argc)	/* advance to next argument */
	usage();
      if (sscanf(argv[argn], "%ld%c", &lval, &ch) < 1)
	usage();
      if (ch == 'm' || ch == 'M')
	lval *= 1000L;
      cinfo->emethods->max_memory_to_use = lval * 1000L;

    } else if (keymatch(arg, "nodither", 3)) {
      /* Suppress dithering in color quantization. */
      cinfo->use_dithering = FALSE;

    } else if (keymatch(arg, "onepass", 1)) {
      /* Use fast one-pass quantization. */
      cinfo->two_pass_quantize = FALSE;

    } else if (keymatch(arg, "pnm", 1)) {
      /* PPM/PGM output format. */
      requested_fmt = FMT_PPM;

    } else if (keymatch(arg, "rle", 1)) {
      /* RLE output format. */
      requested_fmt = FMT_RLE;

    } else if (keymatch(arg, "targa", 1)) {
      /* Targa output format. */
      requested_fmt = FMT_TARGA;

#ifdef	HIPS_IMAGE
    } else if (keymatch(arg, "hips", 1)) {
	/* HIPS output format. */
	format_init(&uimg, IMAGE_INIT_TYPE, HIPS, uimg.color_dpy=-1, "djpeg", "D2-2");
	requested_fmt = FMT_DEFAULT;
#endif

    } else {
      usage();			/* bogus switch */
    }
  }

  return argn;			/* return index of next arg (file name) */
}


/*
 * The main program.
 */

U_IMAGE	uimg;
extern	struct Decompress_methods_struct	dc_methods;

GLOBAL int
main (int argc, char **argv)
{
  int file_index;

  /* On Mac, fetch a command line. */
#ifdef THINK_C
  argc = ccommand(&argv);
#endif

  progname = argv[0];

  /* Set up links to method structures. */
  dinfo.methods = &dc_methods;
  dinfo.emethods = &e_methods;

  /* Install, but don't yet enable signal catcher. */
#ifdef NEED_SIGNAL_CATCHER
  emethods = NULL;
  signal(SIGINT, signal_catcher);
#ifdef SIGTERM			/* not all systems have SIGTERM */
  signal(SIGTERM, signal_catcher);
#endif
#endif

  /* Scan command line: set up compression parameters, input & output files. */

  file_index = parse_switches(&dinfo, 0, argc, argv);

#ifdef TWO_FILE_COMMANDLINE

  if (file_index != argc-2) {
    fprintf(stderr, "%s: must name one input and one output file\n", progname);
    usage();
  }
  if ((dinfo.input_file = fopen(argv[file_index], READ_BINARY)) == NULL) {
    fprintf(stderr, "%s: can't open %s\n", progname, argv[file_index]);
    exit(EXIT_FAILURE);
  }
  if ((dinfo.output_file = fopen(argv[file_index+1], WRITE_BINARY)) == NULL) {
    fprintf(stderr, "%s: can't open %s\n", progname, argv[file_index+1]);
    exit(EXIT_FAILURE);
  }

#else /* not TWO_FILE_COMMANDLINE -- use Unix style */

  dinfo.input_file = stdin;	/* default input file */
  dinfo.output_file = stdout;	/* always the output file */

#ifdef USE_SETMODE		/* need to hack file mode? */
  setmode(fileno(stdin), O_BINARY);
  setmode(fileno(stdout), O_BINARY);
#endif

  if (file_index < argc-1) {
    fprintf(stderr, "%s: only one input file\n", progname);
    usage();
  }
  if (file_index < argc) {
    if ((dinfo.input_file = fopen(argv[file_index], READ_BINARY)) == NULL) {
      fprintf(stderr, "%s: can't open %s\n", progname, argv[file_index]);
      exit(EXIT_FAILURE);
    }
  }

#endif /* TWO_FILE_COMMANDLINE */
  
  /* Set up to read a JFIF or baseline-JPEG file. */
  /* A smarter UI would inspect the first few bytes of the input file */
  /* to determine its type. */
#ifdef JFIF_SUPPORTED
  jselrjfif(&dinfo);
#else
  You shoulda defined JFIF_SUPPORTED.   /* deliberate syntax error */
#endif

#ifdef PROGRESS_REPORT
  /* Start up progress display, unless trace output is on */
  if (e_methods.trace_level == 0)
    dc_methods.progress_monitor = progress_monitor;
#endif

	uimg.IN_FP = dinfo.input_file;
	uimg.OUT_FP = dinfo.output_file;
	io_test(fileno(in_fp), usage());

#ifdef	STREAM_IMAGE
	format_init(&uimg, IMAGE_INIT_TYPE, HIPS, uimg.color_dpy=-1, "djpeg", "D2-2");
	if (!(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0, 0))	{
#else
	if (!jpeg_header_handle(HEADER_READ, &uimg, 0, 0, 0))	{
#endif
#ifdef	HIPS_IMAGE
	    if (requested_fmt == FMT_DEFAULT && uimg.o_type == HIPS)	{
		(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, 1);
		if (!uimg.src)
			uimg.src = nzalloc(uimg.width*uimg.height,
				uimg.dpy_channels, "jpsrc");
	    }
#endif
#ifdef	STREAM_IMAGE
	    if (uimg.in_type != JPEG)	{
		do	{
			(*uimg.std_swif)(FI_LOAD_FILE,&uimg, 0, 0, 0);
			fwrite(hhd.image, hhd.cols, hhd.rows, out_fp);
		} while (++uimg.fn < uimg.frames);
	    } else
#endif
		jpeg_decompress(&dinfo);
	}

#ifdef PROGRESS_REPORT
  /* Clear away progress display */
  if (e_methods.trace_level == 0) {
    fprintf(stderr, "\r                \r");
    fflush(stderr);
  }
#endif

  /* All done. */
  exit(EXIT_SUCCESS);
  return 0;			/* suppress no-return-value warnings */
}
