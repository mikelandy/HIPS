/* window.c	Bit depth windowing.
 * Max Rible	1989
 */
/*   This program is copyright (C) 1990, Regents  of  the
University  of  California.   Anyone may reproduce this software,
in whole or in part, provided that:
(1)  Any copy  or  redistribution  must  show  the
     Regents  of  the  University of California, through its
     Lawrence Berkeley Laboratory, as the source,  and  must
     include this notice;
(2)  Any use of this software must reference this  distribu-
     tion,  state that the software copyright is held by the
     Regents of the University of California, and  that  the
     software is used by their permission.

     It is acknowledged that the U.S. Government has  rights
to this software under  Contract DE-AC03-765F00098 between the U.S.
Department of Energy and the University of California.

     This software is provided as a professional  academic  contribu-
tion  for  joint exchange.  Thus it is experimental, is pro-
vided ``as is'', with no warranties of any kind  whatsoever,
no  support,  promise  of updates, or printed documentation.
Bug reports or fixes may be sent to the author, who may or may
not act on them as he desires.
*/

#include <stdio.h>
#include <hipl_format.h>

#define TOP(pat,bit) ((pat & (0xFFFFFFFF << bit)) ? (pat | (1 << (bit-1))) : pat)

#define NONE 0
#define ALL 1
#define RGB 2
#define ALL_MARGIN 3
#define RGB_MARGIN 4

static int parse_args();
static void all(), rgb(), nuke();

FILE *input, *output;
char *inname,*outname;

union datum {
    int all;
    struct {
	char r;
	char g;
	char b;
    } c;
} shift, and, or, xor, other;

int bytesize, type, hips, marg;
struct header hipshead;

main(argc, argv)
     int argc;
     char *argv[];
{
    register int tmp, margin;

    Progname = strsave(*argv);
    input = stdin;
    output = stdout;
    inname = "<stdin>";
    outname = "<stdout>";
    type = NONE;
    shift.all = or.all = xor.all = 0;
    and.all = 0xFFFFFFFF;
    bytesize = 4;
    hips = 0; margin = 0;

    if(tmp = parse_args(argc, argv)) exit(tmp);

    if(marg) switch(type) {
    case ALL:
	type = ALL_MARGIN;
	margin = marg - (shift.all > 0 ? shift.all : 0);
	break;
    case RGB:
	type = RGB_MARGIN;
	margin = marg - (shift.all > 0 ? shift.all : 0);
	break;
    }

    if(hips) {
	fread_header(input, &hipshead,inname);
	switch(bytesize) {
	case 1:
	    hipshead.pixel_format = PFBYTE; break;
	case 2:
	    hipshead.pixel_format = PFSHORT; break;
	case 4:
	    hipshead.pixel_format = PFINT; break;
	default:
	    fprintf(stderr, "Illegal HIPS format.\n"); exit(-1);
	}
	update_header(&hipshead, argc, argv);
	fwrite_header(output, &hipshead,outname);
    }

    switch(type) {
    case NONE:
	while(!feof(input))
	    nuke(getw(input));
	break;
    case ALL:
	if(shift.all < 0) {
	    other.all = -shift.all;
	    shift.all = 0;
	}
	while(!feof(input)) {
	    tmp = getw(input);
	    nuke( (((((tmp << shift.all) >> other.all) ^ xor.all) & and.all) | or.all) );
	}
	break;
    case RGB:
	if(shift.c.r < 0) { other.c.r = -shift.c.r; shift.c.r = 0; }
	if(shift.c.g < 0) { other.c.g = -shift.c.g; shift.c.g = 0; }
	if(shift.c.b < 0) { other.c.b = -shift.c.b; shift.c.b = 0; }
	while(!feof(input)) {
	    tmp = getw(input);
	    nuke( (((((((tmp << shift.c.r) >> other.c.r) ^ xor.c.r) & and.c.r) | or.c.r) & 0xFF) |
		   (((((((tmp << shift.c.g) >> other.c.g) ^ xor.c.g) & and.c.g) | or.c.g) & 0xFF) << 8) |
		   (((((((tmp << shift.c.b) >> other.c.b) ^ xor.c.b) & and.c.b) | or.c.b) & 0xFF) << 16)));
	}
	break;
    case ALL_MARGIN:
	if(shift.all < 0) {
	    other.all = -shift.all;
	    shift.all = 0;
	}
	while(!feof(input)) {
	    tmp = getw(input);
	    nuke( (((((TOP(tmp,margin) << shift.all) >> other.all) ^ xor.all) & and.all) | or.all) );
	}
	break;
    case RGB_MARGIN:
	if(shift.c.r < 0) { other.c.r = -shift.c.r; shift.c.r = 0; }
	if(shift.c.g < 0) { other.c.g = -shift.c.g; shift.c.g = 0; }
	if(shift.c.b < 0) { other.c.b = -shift.c.b; shift.c.b = 0; }
	while(!feof(input)) {
	    tmp = getw(input);
	    nuke( (((((((TOP(tmp,margin) << shift.c.r) >> other.c.r) ^ xor.c.r) & and.c.r) | or.c.r) & 0xFF) |
		   (((((((TOP(tmp,margin) << shift.c.g) >> other.c.g) ^ xor.c.g) & and.c.g) | or.c.g) & 0xFF) << 8) |
		   (((((((TOP(tmp,margin) << shift.c.b) >> other.c.b) ^ xor.c.b) & and.c.b) | or.c.b) & 0xFF) << 16)));
	}
	break;
    }
}

static int
parse_args(argc, argv)
     int argc;
     char *argv[];
{
    int i, tmp;

    for(i = 1; i < argc; i++) {
	if(argv[i][0] != '-') {
	    fprintf(stderr, "Bad argument: %s\n", argv[i]);
	    return(-1);
	} else {
	    switch(argv[i][1]) {
	    case 's':		/* Shift */
		switch(argv[i][2]) {
		case '\0':
		    sscanf(argv[++i], "%i", &tmp); shift.all = tmp; all(); break;
		case 'r':
		    sscanf(argv[++i], "%i", &tmp); shift.c.r = tmp; rgb(); break;
		case 'g':
		    sscanf(argv[++i], "%i", &tmp); shift.c.g = tmp; rgb(); break;
		case 'b':
		    sscanf(argv[++i], "%i", &tmp); shift.c.b = tmp; rgb(); break;
		}
		break;
	    case 'A':		/* AND */
		switch(argv[i][2]) {
		case '\0':
		    sscanf(argv[++i], "%i", &tmp); and.all = tmp; all(); break;
		case 'r':
		    sscanf(argv[++i], "%i", &tmp); and.c.r = tmp; rgb(); break;
		case 'g':
		    sscanf(argv[++i], "%i", &tmp); and.c.g = tmp; rgb(); break;
		case 'b':
		    sscanf(argv[++i], "%i", &tmp); and.c.b = tmp; rgb(); break;
		}
		break;
	    case 'O':		/* OR */
		switch(argv[i][2]) {
		case '\0':
		    sscanf(argv[++i], "%i", &tmp); or.all = tmp; all(); break;
		case 'r':
		    sscanf(argv[++i], "%i", &tmp); or.c.r = tmp; rgb(); break;
		case 'g':
		    sscanf(argv[++i], "%i", &tmp); or.c.g = tmp; rgb(); break;
		case 'b':
		    sscanf(argv[++i], "%i", &tmp); or.c.b = tmp; rgb(); break;
		}
		break;
	    case 'X':		/* XOR */
		switch(argv[i][2]) {
		case '\0':
		    sscanf(argv[++i], "%i", &tmp); xor.all = tmp; all(); break;
		case 'r':
		    sscanf(argv[++i], "%i", &tmp); xor.c.r = tmp; rgb(); break;
		case 'g':
		    sscanf(argv[++i], "%i", &tmp); xor.c.g = tmp; rgb(); break;
		case 'b':
		    sscanf(argv[++i], "%i", &tmp); xor.c.b = tmp; rgb(); break;
		}
		break;
	    case 'i':		/* Input */
		inname = argv[++i];
		if((input = fopen(inname, "r")) == NULL)
		    perror("input file");
		break;
	    case 'o':		/* Output */
		outname = argv[++i];
		if((output = fopen(outname, "w")) == NULL)
		    perror("output file");
		break;
	    case 'H':
		hips = 1;
		break;
	    case 'M':
		marg = atoi(argv[++i]);
		break;
	    case 'w':
		switch(bytesize = atoi(argv[++i])) {
		case 8:
		case 16:
		case 24:
		case 32:
		    bytesize /= 8;
		case 1:
		case 2:
		case 3:
		case 4:
		    break;
		default:
		    fprintf(stderr, "Bad byte size.\n");
		    return(-1);
		}
		break;
	    case 'h':
	    default:
puts("Bitdepth windowing: specify with integers in bases 8, 10, 16.");
puts("-s shift     Left is positive, right is negative.");
puts("-A andmask   AND mask: default 255.");
puts("-O ormask    OR mask: default 0.");
puts("-X xormask   XOR mask: default 0.");
puts("Append r, g, or b to any of the above arguments to specify a single");
puts("output point; without rgb the program will deal with complete long ints.");
puts("Shift operates on the long as a whole; rgb masks operate on each byte.");
puts("The program operates in the order shift, XOR, AND, OR.");
puts("-i input     Default stdin.");
puts("-o output    Default stdout.");
puts("-w bytesize  Give 1-4 or 8,16,24,32.  With rgb, you must use 24 or 32.");
puts("-M margin    Set left bit margin to keep bits shifted left.");
puts("-H           Read and write a file with a HIPS header.");
		return(-1);
	    }
	}
    }

    return(0);
}

static void
all()
{
    if(type == 0) type = ALL;
    if(type != ALL) { fprintf(stderr,
			    "Don't mix rgb and overall arguments!\n");
		      exit(-1);
		  }
    return;
}

static void
rgb()
{
    if(type == 0) type = RGB;
    if(type != RGB) { fprintf(stderr,
			    "Don't mix rgb and overall arguments!\n");
		      exit(-1);
		  }
    return;
}

static void
nuke(it)
     int it;
{
    switch(bytesize) { 
    case 1:
	putc((char) (it & 0xFF), output);
	break;
    case 2:
	putc((char)((it & 0xFF00) >> 8), output);
	putc((char)(it & 0xFF), output);
	break;
    case 3:
	putc((char)((it & 0xFF0000) >> 16), output);
	putc((char)((it & 0xFF00) >> 8), output);
	putc((char)(it & 0xFF), output);
	break;
    case 4:
	putw(it, output);
	break;
    }
}
