/*
 *	shiftmin
 */
#include	<stdio.h>
#include	<sys/file.h>
#include	<math.h>
#include	<hipl_format.h>

#define		errexit(msg)	fprintf(stderr,"%s\n", msg), exit(1)

char	usage[] = "usage: shiftmin [-f seq] [-m newmin] [-V] [-u] > outfile\n\
\n\
newmin defaults to 0.; -V for verbose; -u for this message.";


int main(argc, argv)
int	argc;
char	**argv;
{
			h_boolean		VERBOSE = FALSE;
			struct header	hd;
			FILE		*fp;
	register	float		*ip;
	register	float		*op;
	register	int		i;
	register	int		t;

			float		*ofr,
					*ifr;

			int		frames,
					rows,
					cols,
					framepix,
					framebytes;
			float		shift;
			double		min,
					newmin = 0.;
			char		*file;

	Progname = strsave(*argv);

	for (i = argc; --i;	)
	/**/ if (argv[i][0] == '-')
	switch (argv[i][1]) {
	case 'u': errexit(usage);
	case 'f': if ((fp = fopen(file = argv[i+1], "r")) == NULL)
			fprintf(stderr,"shiftmin: can't open %s\n", argv[i+1]);
		continue;
	case 'V': VERBOSE = TRUE;
		continue;
	case 'm': newmin = (double)atof(argv[i+1]);
		continue;
	default: fprintf(stderr,
			"shiftmin: unrecognized option: %s\n", argv[i]);
	}
	fread_header(fp,&hd,file);
	if (hd.pixel_format != PFFLOAT)
		perr(HE_MSG,"pixel format must be float");
	frames = hd.num_frame;
	rows = hd.orows;
	cols = hd.ocols;
	update_header(&hd, argc, argv);
	write_header(&hd);
	framebytes = (framepix = rows*cols) * sizeof(float);

	if ((ifr = (float *)malloc(framebytes)) == NULL)
		errexit("shiftmin: can't get core.");
	if ((ofr = (float *)malloc(framebytes)) == NULL)
		errexit("shiftmin: can't get core.");

	for (t = frames; t; t--)  {
		if (fread(ifr, framebytes,1,fp)  != 1)
			errexit("shiftmin: read error");
		if (t == frames)
			min = *ifr;
		for (ip = ifr, i = framepix; i; i--, ip++)
		/**/ if (*ip < min)
			min = (double)*ip;
	}

	if (VERBOSE)
		fprintf(stderr, "shiftmin: %s minimum = %f\n", file,(float)min);
	fclose(fp);
	if ((fp = fopen(file, "r")) == NULL) {
		fprintf(stderr, "shiftmin: can't open %s\n", argv[2]);
		exit(1);
	}
	fread_header(fp, &hd,file);
	
	for (shift = (float)(newmin - min), t = frames; t; t--)  {
		if (fread(ofr, framebytes,1,fp)  != 1)
			errexit("shiftmin: read error");
		for (op = ofr, i = framepix; i; i--, op++)
			*op += shift;
		if (fwrite(ofr, framebytes,1,stdout) != 1)
			errexit("shiftmin: write error");
	}
	exit(0);
}
