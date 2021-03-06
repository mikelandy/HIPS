/*
 *	rtex
 */
#include	<stdio.h>
#include	<sys/file.h>
#include	<math.h>
#include	<hipl_format.h>

#define		MAXCOMPS	10
#define	COINFLIP ( ( H__RANDOM() >> ( H__RANDOM() & ( H__RANDBITS >> 1))) & 01)

char	usage[] = 
"usage: rtex [-u] pattfile [-s seed] < inseq > outseq\n\
\n\
pattfile is formatted as follows: line 1:\n\
\n\
			%d %d %d\n\
\n\
There follow 2*i*j*k floating point fields, for i, j and k the\n\
three integers given\n\
in line 1.  The first i*j*k of these post-first-line\n\
floats define one rtexture\n\
subpattern.  The second i*j*k floats define another rtexture subpattern.  \n\
Each subpattern contains i lines and j cols and is obtained as follows:\n\
successive i*j bunches of floats (for each subpattern there are k of these)\n\
are taken as jointly independent subpattern subcomponents.  Each subpattern\n\
subcomponent is multiplied by a random variable that takes the value 1 or -1\n\
with equal probability.  Each subpattern is the sum of its k independent\n\
subpcomponents.\n\
Inseq is byte formatted and contains only 0's and 1's.  This file defines\n\
a rtexture generated by \n\
substituting an independent realization of the first subpattern for\n\
each 0 and an independent realization of the second subpattern for each 1.";

int main(argc, argv)
int	argc;
char	**argv;
{
			FILE		*fp;
			struct header	hd;
	register	int		p,
					q;
	register	int		x;
	register	int		y;
	register	int		i;
	register	float		*op;
	register	unsigned char	*ip;
			int		t;

			float		*ofr;
			unsigned char	*ifr;

			int		frames,
					rows,
					cols,
					c_rows,
					c_cols,
					c_bytes,
					ncomps,
					iframebytes,
					oframebytes,
					seed;
			float		*(A[MAXCOMPS]),
					*(B[MAXCOMPS]),
					rv;

	Progname = strsave(*argv);
	if (argc == 1 || !strcmp(argv[1], "-u"))
		perr(HE_MSG,usage);
	else if ((fp = fopen(argv[1], "r")) == NULL)
		perr(HE_OPEN, argv[1]);
	fscanf(fp, "%d%d%d", &c_rows, &c_cols, &ncomps);
	c_bytes = c_rows*c_cols*sizeof(float);
	for (i = 0; i < ncomps; i++) {
		if ((A[i] = (float *)malloc(c_bytes)) == NULL)
			perr(HE_MSG,"can't get core.");
		for (y = 0; y < c_rows; y++) {
			for (x = 0; x < c_cols; x++) {
				fscanf(fp,"%f", A[i]+y*c_cols + x);
			}
		}
	}
	for (i = 0; i < ncomps; i++) {
		if ((B[i] = (float *)malloc(c_bytes)) == NULL)
			perr(HE_MSG,"can't get core.");
		for (y = 0; y < c_rows; y++) {
			for (x = 0; x < c_cols; x++) {
				fscanf(fp,"%f", B[i]+y*c_cols + x);
			}
		}
	}
	for (i = argc; --i;	)
	/**/ if (argv[i][0] == '-')
	switch (argv[i][1]) {
	case 's': seed = atoi(argv[i+1]);
		continue;
	default: fprintf(stderr, "rtex: unrecognized option: %s\n", argv[i]);
	}
	read_header(&hd);
	if (hd.pixel_format != PFBYTE)
		perr(HE_MSG,"input pixel format must be byte");
	iframebytes = hd.orows * hd.ocols * sizeof(unsigned char);
	hd.orows = rows = hd.orows * c_rows;
	hd.ocols = cols = hd.ocols * c_cols;
	frames = hd.num_frame;
	hd.pixel_format = PFFLOAT;
	update_header(&hd, argc, argv);
	write_header(&hd);
	oframebytes = rows*cols * sizeof(float);
	H__SRANDOM(seed);

	if ((ifr = (unsigned char *)malloc(iframebytes)) == NULL)
		perr(HE_MSG,"can't get core.");
	if ((ofr = (float *)malloc(oframebytes)) == NULL)
		perr(HE_MSG,"can't get core.");

	for (t = 0; t < frames; t++)  {
		if (fread(ifr, iframebytes,1,stdin) != 1)
			perr(HE_MSG,"read error");
		for (ip = ifr, op = ofr, y = 0; y < rows;
		/**/ y+=c_rows, op+=(c_rows-1)*cols) {
			for (x = 0; x < cols; x+=c_cols, op+=c_cols, ip++) {
				if (*ip) {
					for (i = 0; i < ncomps; i++) {
						rv = COINFLIP? -1. : 1.;
						for (p = 0; p < c_rows; p++) {
						  for (q=0; q<c_cols; q++) {
						    *(op+p*cols+q) +=
							*(A[i]+p*c_cols+q) * rv;
						  }
						}
					}
				} else {
					for (i = 0; i < ncomps; i++) {
						rv = COINFLIP? -1. : 1.;
						for (p = 0; p < c_rows; p++) {
						  for (q=0; q<c_cols; q++) {
						    *(op+p*cols+q) +=
							*(B[i]+p*c_cols+q) * rv;
						  }
						}
					}
				}
			}
		}
		if (fwrite(ofr, oframebytes,1,stdout) != 1)
			perr(HE_MSG,"write error");
	}
	exit(0);
}
