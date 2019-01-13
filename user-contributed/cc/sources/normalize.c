/*
 *	normalize
 */
#include	<stdio.h>
#include	<sys/file.h>
#include	<math.h>
#include	<hipl_format.h>

#define		errexit(msg)	fprintf(stderr,"%s\n", msg), exit(1)

char	usage[] = 
"usage: normalize rt_mn_sq seq";

int main(argc, argv)
int	argc;
char	**argv;
{
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
			double		energy,
					sq_rt_out_energy;

	Progname = strsave(*argv);
	switch (argc) {
	case 3: sq_rt_out_energy = atof(argv[1]);
		if ((fp = fopen(argv[2], "r")) == NULL)
			perr(HE_OPEN, argv[2]);
		fread_header(fp, &hd,argv[2]);
		if (hd.pixel_format != PFFLOAT)
			perr(HE_MSG,"inseq must be float\n");
		break;
	default: perr(HE_MSG,usage);
	}
	frames = hd.num_frame;
	rows = hd.orows;
	cols = hd.ocols;
	framebytes = (framepix = rows*cols) * sizeof(float);

	if ((ifr = (float *)malloc(framebytes)) == NULL)
		errexit("normalize: can't get core.");
	if ((ofr = (float *)malloc(framebytes)) == NULL)
		errexit("normalize: can't get core.");

	for (energy = 0., t = 0; t < frames; t++)  {
		if (fread(ifr, framebytes,1,fp)  != 1)
			errexit("normalize: read error");
		for (ip = ifr, i = framepix; i; i--, ip++) {
			energy += *ip * *ip;
		}
	}
	fclose(fp);
	if ((fp = fopen(argv[2], "r")) == NULL)
		perr(HE_OPEN, argv[2]);
	fread_header(fp, &hd,argv[2]);
	if (hd.pixel_format != PFFLOAT)
		perr(HE_MSG,"input pixel format must be float");

	update_header(&hd, argc, argv);
	write_header(&hd);

	sq_rt_out_energy /= sqrt(energy);

	for (energy = 0., t = 0; t < frames; t++)  {
		if (fread(ifr, framebytes,1,fp)  != 1)
			errexit("normalize: read error");
		for (op = ofr, ip = ifr, i = framepix; i; i--, ip++, op++) {
			*op = sq_rt_out_energy * *ip;
		}
		if (fwrite(ofr, framebytes,1,stdout) != 1)
			errexit("normalize: write error");
	}
	exit(0);
}
