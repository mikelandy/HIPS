/*
 *	updown
 */
#include	<stdio.h>
#include	<hipl_format.h>

#define		errexit(msg)	fprintf(stderr,"%s\n", msg), exit(1)

#define	COINFLIP ( ( H__RANDOM() >> ( H__RANDOM() & ( H__RANDBITS >> 1))) & 01)

char	usage[] = "usage: updown [-u] [-s seed] < inseq > outseq";

int main(argc, argv)
int	argc;
char	**argv;
{
			struct header	hd;
	register	float		*op;
	register	int		i;
	register	int		t;

			float		*ofr;

			int		frames,
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
	default: fprintf(stderr,
			"updown: unrecognized option: %s\n", argv[i]);
	}
	read_header(&hd);
	if (hd.pixel_format != PFFLOAT)
		perr(HE_MSG,"pixel format must be float");
	frames = hd.num_frame;
	update_header(&hd, argc, argv);
	write_header(&hd);
	framebytes = (framepix = hd.orows*hd.ocols) * sizeof(float);

	if ((ofr = (float *)malloc(framebytes)) == NULL)
		errexit("updown: can't get core.");

	for (H__SRANDOM(seed), t = frames; t; t--)  {
		if (fread(ofr, framebytes,1,stdin) != 1)
			errexit("updown: read error");
		for (op = ofr, i = framepix; i; i--, op++)
			*op = COINFLIP ? *op : -*op;
		if (fwrite(ofr, framebytes,1,stdout) != 1)
			errexit("updown: write error");
	}
	exit(0);
}
