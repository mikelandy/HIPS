/*
 *     idealbp	
 */
#include	<stdio.h>
#include	<sys/file.h>
#include	<math.h>
#include	<hipl_format.h>

#define	MYPI	3.141592653589793238
typedef         float           complex[2];
char	usage[] = 
"usage: [-u] [-l lfrq] [-h hfrq] [-a startangle endangle] < inseq > outseq\n\
defaults: lfrq = 0, hfrq = 128";

int main(argc, argv)
int	argc;
char	**argv;
{
			struct header	hd;
	register        complex		*op;
	register        complex		*ip;
	register	int		x;
	register	int		y;
			int		i,
					t;

			complex		*ofr,
					*ifr;

			int		frames,
					rows,
					cols,
					r_o_2,
					framepix,
					framebytes;
			double		hfrq = 128,
                                        lfrq = 0,
					hfrqsq,
					lfrqsq,
					theta1 = 0.,
					theta2 = MYPI,
					angle;
	Progname = strsave(*argv);
	if (!strcmp(argv[1], "-u"))
		fprintf(stderr, "%s\n", usage), exit(0);
	read_header(&hd);
	if (hd.pixel_format != PFCOMPLEX)
		perr(HE_MSG,"input sequence pixel format must be complex");
	rows = hd.orows;
	cols = hd.ocols;
	frames = hd.num_frame;
	update_header(&hd, argc, argv);
	write_header(&hd);

	for (i = argc; --i;	)
	/**/ if (argv[i][0] == '-')
	switch (argv[i][1]) {
	case 'u': perr(HE_MSG,usage);
	case 'l': lfrq = (double)atof(argv[i+1]);
		continue;
        case 'h': hfrq = (double)atof(argv[i+1]);
                continue;
	case 'a': theta1 = MYPI*(double)atof(argv[i+1]);
		theta2 = MYPI*(double)atof(argv[i+2]);
		continue;
	default: fprintf(stderr,
			"idealbp: unrecognized option: %s\n", argv[i]);
	}
	framebytes = (framepix = rows*cols) * sizeof(complex);
		
	if ((ifr = (complex *)calloc(framepix, sizeof(complex))) == NULL)
		perr(HE_MSG,"can't get core.");
	if ((ofr = (complex *)calloc(framepix, sizeof(complex))) == NULL)
		perr(HE_MSG,"can't get core.");

	for (r_o_2 = rows/2, lfrqsq = lfrq*lfrq,
	/**/ hfrqsq = hfrq*hfrq, t = frames; t; t--) {
		if (fread(ifr, framebytes,1,stdin) != 1)
			perr(HE_MSG,"read error");
		for (op = ofr, ip = ifr, y = 0; y < r_o_2; y++)
		/**/ for (x = 0; x < cols; x++, op++, ip++)
		/**/ if (
		(
			(angle = atan2(((double)y), ((double)x))) > theta1
			&&
			angle < theta2
		)
		&&
		(
			(
				x*x + y*y >= lfrqsq
				&&
				x*x + y*y < hfrqsq
			)
			||
			(
				(x-cols)*(x-cols) + y*y >= lfrqsq
				&& 
				(x-cols)*(x-cols) + y*y < hfrqsq
			)
			||  
			(
				(y-rows)*(y-rows) + x*x >= lfrqsq
				&&
				(y-rows)*(y-rows) + x*x < hfrqsq
			)
			||
			(
				(x-cols)*(x-cols) + (y-rows)*(y-rows) >= lfrqsq
				&&
				(x-cols)*(x-cols) + (y-rows)*(y-rows) < hfrqsq
			)
		)
		) { 	
			(*op)[0] = (*ip)[0];
			(*op)[1] = (*ip)[1];
			if (x && y) {
			       (*(ofr + (rows-y)*cols + (cols-x)))[0]
								= (*op)[0];
			       (*(ofr + (rows-y)*cols + (cols-x)))[1]
								= -(*op)[1];
			}
		}
		if (fwrite(ofr, framebytes,1,stdout) != 1)
			perr(HE_MSG,"write error");
	}
	exit(0);
}
