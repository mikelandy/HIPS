/*
 *	getmean
 */
#include	<stdio.h>
#include	<sys/file.h>
#include	<math.h>
#include	<hipl_format.h>

#define		errexit(msg)	fprintf(stderr,"%s\n", msg), exit(1)

char	usage[] = 
"usage: getmean";

int main(argc, argv)
int	argc;
char	**argv;
{
			struct header	hd;
	register	float		*ip;
	register	int		i;
	register	int		t;

			float		*ifr;

			int		frames,
					rows,
					cols,
					framepix,
					framebytes;
			double		mean;

	Progname = strsave(*argv);
	for (i = argc; --i;	)
	/**/ if (argv[i][0] == '-')
	switch (argv[i][1]) {
	case 'u': errexit(usage);
	default: fprintf(stderr,
			"getmean: unrecognized option: %s\n", argv[i]);
	}
	read_header(&hd);
	frames = hd.num_frame;
	rows = hd.orows;
	cols = hd.ocols;
	framebytes = (framepix = rows*cols) * sizeof(float);
	if (hd.pixel_format != PFFLOAT)
		perr(HE_MSG,"pixel format must be float");

	if ((ifr = (float *)malloc(framebytes)) == NULL)
		errexit("getmean: can't get core.");

	for (t = 0; t < frames; t++)  {
		if (fread(ifr, framebytes,1,stdin)  != 1)
			errexit("getmean: read error");
		for (ip = ifr, i = framepix; i; i--, ip++)
			mean += *ip;
	}
	printf("%f\n",mean/(double)(frames*framepix));
	exit(0);
}
