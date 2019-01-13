/*
 *	rflipix
 */
#include	<stdio.h>
#include	<sys/file.h>
#include	<math.h>
#include	<hipl_format.h>

#define		errexit(msg)	fprintf(stderr,"%s\n", msg), exit(1)
#define	COINFLIP ( ( H__RANDOM() >> ( H__RANDOM() & ( H__RANDBITS >> 1))) & 01)

#define		DMAXSIGNED	((double)((int)(((unsigned)(~0)) >> 1)))

char	usage[] = 
"usage: rflipix [-u] [-s seed] [-F] < inseq > outseq\n\
\n\
inseq must be PFFLOAT ranging between 0. and 1.\n";

int main(argc, argv)
int	argc;
char	**argv;
{
			struct header	hd;
			int		FIRST = FALSE;
	register	float		*ip;
	register	float		*op;
	register	int		i;
	register	int		t;

			float		*ofr;
			float		*ifr;

			int		frames,
					rows,
					cols,
					framepix,
					framebytes,
					seed = 0;

	Progname = strsave(*argv);
	for (i = argc; --i;	)
	/**/ if (argv[i][0] == '-')
	switch (argv[i][1]) {
	case 'u': errexit(usage);
	case 's': seed = atoi(argv[i+1]);
		continue;
	case 'F': FIRST = TRUE;
		continue;
	default: fprintf(stderr, "rflipix: unrecognized option: %s\n", argv[i]);
	}
	read_header(&hd);
	frames = hd.num_frame;
	if (FIRST)
		hd.num_frame++;
	rows = hd.orows;
	cols = hd.ocols;
	if (hd.pixel_format !=  PFFLOAT)
		perr(HE_MSG,"pixel_format must be PFFLOAT.");
	hd.pixel_format = PFFLOAT;
	update_header(&hd, argc, argv);
	write_header(&hd);
	framebytes = (framepix = rows*cols) * sizeof(float);
	

	if ((ifr = (float *)malloc(framebytes)) == NULL)
		errexit("rflipix: can't get core.");
	if ((ofr = (float *)malloc(framebytes)) == NULL)
		errexit("rflipix: can't get core.");

	for (H__SRANDOM(seed), i = framepix, op = ofr; i; i--, op++)
		*op = COINFLIP ? -1. : 1.;
	if (FIRST) {
		if (fwrite(ofr, framebytes,1,stdout) != 1)
			errexit("rflipix: write error");
	}
	for (t = 0; t < frames; t++)  {
		if (fread(ifr, framebytes,1,stdin)  != 1)
			errexit("rflipix: read error");
		for (op = ofr, ip = ifr, i = framepix; i; i--, op++, ip++)
			if (*ip * DMAXSIGNED > H__RANDOM())
				*op = -*op;
		if (fwrite(ofr, framebytes,1,stdout) != 1)
			errexit("rflipix: write error");
	}
	exit(0);
}
