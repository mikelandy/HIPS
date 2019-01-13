/*
 *	revalue
 */
#include	<stdio.h>
#include	<hipl_format.h>

#define		errexit(msg)	fprintf(stderr,"%s\n", msg), exit(1)
#define		MAXVALS		256

char	usage[] = "usage: revalue [-u] ctrlfile < inseq > outseq\n\
\n\
ctrlfile is formatted as follows:\n\
\n\
		i_1 o_1\n\
		i_2 o_2\n\
		   .\n\
		   .\n\
		   .\n\
		i_n o_n\n\
\n\
For j = 1,2,...,n, every occurrence of the value i_j in inseq (inseq can be\n\
float, int, or byte) is replaced by the value o_j in outseq.";

int main(argc, argv)
int	argc;
char	**argv;
{
			FILE		*ifp,
					*fopen();
			struct header	hd;
	register	int		j;
	register	int		i;
	register	int		t;
			int		frames,
					rows,
					cols,
					framepix,
					framebytes,
					numval;

	Progname = strsave(*argv);
	if (argc != 2 || !strcmp(argv[1], "-u"))
		fprintf(stderr, "%s\n", usage), exit(1);
	if ((ifp = fopen(argv[1],"r")) == NULL) {
		fprintf(stderr,"revalue: can't open  %s\n",argv[1]);
		exit(1);
	}
	read_header(&hd);
	frames = hd.num_frame;
	rows = hd.orows;
	cols = hd.ocols;
	update_header(&hd, argc, argv);
	write_header(&hd);
	framepix = rows*cols;

if (hd.pixel_format == PFBYTE) {
	register	unsigned char	*op,
					*ip,
					*ivp,
					*ovp;
			unsigned char	*ofr,
					*ifr,
					ival[MAXVALS],
					oval[MAXVALS];
			int i1,i2;
	
	framebytes = framepix*sizeof(unsigned char);
	for (j = 0; fscanf(ifp,"%d%d", &i1, &i2) != EOF; j++) {
		ival[j] = i1;
		oval[j] = i2;
	}
	numval = j;

	if ((ifr = (unsigned char *)malloc(framebytes)) == NULL)
		errexit("revalue: can't get core.");
	if ((ofr = (unsigned char *)malloc(framebytes)) == NULL)
		errexit("revalue: can't get core.");

	for (t = frames; t; t--)  {
		if (fread(ifr, framebytes,1,stdin)  != 1)
			errexit("revalue: read error");
		for (op = ofr, ip = ifr, i = framepix; i; i--, op++, ip++) {
			*op = *ip;
			for (ivp = ival, ovp = oval, j = numval; j;
			/**/ j--, ivp++, ovp++)
				if (*ivp == *ip) {
					*op = *ovp;
					break;
				}
		}
		if (fwrite(ofr, framebytes,1,stdout) != 1)
			errexit("revalue: write error");
	}
} else if (hd.pixel_format == PFFLOAT) {
	register	float		*op,
					*ip,
					*ivp,
					*ovp;
			float		*ofr,
					*ifr,
					ival[MAXVALS],
					oval[MAXVALS];
	
	framebytes = framepix*sizeof(float);
	for (j = 0; fscanf(ifp,"%f%f", &ival[j], &oval[j]) != EOF; j++)
		;
	numval = j;

	if ((ifr = (float *)malloc(framebytes)) == NULL)
		errexit("revalue: can't get core.");
	if ((ofr = (float *)malloc(framebytes)) == NULL)
		errexit("revalue: can't get core.");

	for (t = frames; t; t--)  {
		if (fread(ifr, framebytes,1,stdin)  != 1)
			errexit("revalue: read error");
		for (op = ofr, ip = ifr, i = framepix; i; i--, op++, ip++) {
			*op = *ip;
			for (ivp = ival, ovp = oval, j = numval; j;
			/**/ j--, ivp++, ovp++)
				if (*ivp == *ip) {
					*op = *ovp;
					break;
				}
		}
		if (fwrite(ofr, framebytes,1,stdout) != 1)
			errexit("revalue: write error");
	}
} else if (hd.pixel_format == PFINT) {
	register	int		*op,
					*ip,
					*ivp,
					*ovp;
			int		*ofr,
					*ifr,
					ival[MAXVALS],
					oval[MAXVALS];
	
	framebytes = framepix*sizeof(int);
	for (j = 0; fscanf(ifp,"%d%d", &ival[j], &oval[j]) != EOF; j++)
		;
	numval = j;

	if ((ifr = (int *)malloc(framebytes)) == NULL)
		errexit("revalue: can't get core.");
	if ((ofr = (int *)malloc(framebytes)) == NULL)
		errexit("revalue: can't get core.");

	for (t = frames; t; t--)  {
		if (fread(ifr, framebytes,1,stdin)  != 1)
			errexit("revalue: read error");
		for (op = ofr, ip = ifr, i = framepix; i; i--, op++, ip++) {
			*op = *ip;
			for (ivp = ival, ovp = oval, j = numval; j;
			/**/ j--, ivp++, ovp++)
				if (*ivp == *ip) {
					*op = *ovp;
					break;
				}
		}
		if (fwrite(ofr, framebytes,1,stdout) != 1)
			errexit("revalue: write error");
	}
} else {
	perr(HE_MSG,"pixel_format must be one of int, float or byte");
}
exit(0);
}
