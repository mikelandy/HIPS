/*
 *	getmin
 */
#include	<stdio.h>
#include	<sys/file.h>
#include	<math.h>
#include	<hipl_format.h>

char	usage[] = "usage: [-u] [-i] [-f] < inseq\n\
\n\
-u for this message;\n\
-i for output formatted as an integer (rounded to nearest int if inseq is\n\
PFFLOAT);\n\
-f for output formatted as a float.\n";

int main(argc, argv)
int	argc;
char	**argv;
{
			struct header	hd;
			h_boolean		INT_OUT = FALSE,
					FLOAT_OUT = FALSE;
	register	int		i;
	register	int		t;
			int		frames,
					framepix;

	Progname = strsave(*argv);
	for (i = argc; --i;	)
	/**/ if (argv[i][0] == '-')
	switch (argv[i][1]) {
	case 'u': perr(HE_MSG,usage);
	case 'i': INT_OUT = TRUE;
		continue;
	case 'f': FLOAT_OUT = TRUE;
		continue;
	default: fprintf(stderr,
			"getmin: unrecognized option: %s\n", argv[i]);
	}
	read_header(&hd);
	frames = hd.num_frame;
	framepix = hd.orows*hd.ocols;

	if (hd.pixel_format == PFFLOAT) {

		register	float		*ip;
				float		*ifr;
				float		min;
				int		framebytes;

		framebytes = framepix*sizeof(float);
		if ((ifr = (float *)malloc(framebytes)) == NULL)
			perr(HE_MSG,"can't get core.");

		for (t = 0; t < frames; t++)  {
			if (fread(ifr, framebytes,1,stdin)  != 1)
				perr(HE_MSG,"read error");
			if (t == 0)
				min = *ifr;
			for (ip = ifr, i = framepix; i; i--, ip++) {
				if (*ip < min)
					min = *ip;
			}
		}
		if (INT_OUT)
			printf("%d", (int)(min+.5));
		else
			printf("%f", min);
	}
	else if (hd.pixel_format == PFBYTE) {

		register	unsigned char		*ip;
				unsigned char		*ifr;
				unsigned char		min;
				int		framebytes;

		framebytes = framepix*sizeof(unsigned char);
		if ((ifr = (unsigned char *)malloc(framebytes)) == NULL)
			perr(HE_MSG,"can't get core.");

		for (t = 0; t < frames; t++)  {
			if (fread(ifr, framebytes,1,stdin)  != 1)
				perr(HE_MSG,"read error");
			if (t == 0)
				min = *ifr;
			for (ip = ifr, i = framepix; i; i--, ip++) {
				if (*ip < min)
					min = *ip;
			}
		}
		if (FLOAT_OUT)
			printf("%f", (float)min);
		else
			printf("%d", min);
	}
	else if (hd.pixel_format == PFINT) {

		register	int		*ip;
				int		*ifr;
				int		min;
				int		framebytes;

		framebytes = framepix*sizeof(int);
		if ((ifr = (int *)malloc(framebytes)) == NULL)
			perr(HE_MSG,"can't get core.");

		for (t = 0; t < frames; t++)  {
			if (fread(ifr, framebytes,1,stdin)  != 1)
				perr(HE_MSG,"read error");
			if (t == 0)
				min = *ifr;
			for (ip = ifr, i = framepix; i; i--, ip++) {
				if (*ip < min)
					min = *ip;
			}
		}
		if (FLOAT_OUT)
			printf("%f", (float)min);
		else
			printf("%d", min);
	}
	else
		perr(HE_MSG,"pixel format must be byte or int or float");
	exit(0);
}
