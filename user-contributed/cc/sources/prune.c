/*
 *	prune
 */
#include	<stdio.h>
#include	<sys/file.h>
#include	<math.h>
#include	<hipl_format.h>
#define		NUMBINS 1000
char	usage[] = 
"usage: prune [-u] min max mean proportion < inseq > outseq\n\
\n\
min is the minimum value of inseq; max is the maximum value; mean is the\n\
value you want midway between the final maximum and minimum values, and\n\
proportion is the proportion of values you want clipped.\n";

int getpid();

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
					framebytes,
					brace,
					sureclips;
			FILE		*fp;
			int		basicbin[NUMBINS];
			char		cmd[50],
					filename[50];
			double		minval,
					maxval,
					newmin,
					newmax,
					min,
					max,
					mean,
					prob,
					necprob,
					binsize,
					P;

	Progname = strsave(*argv);
	if (argc != 5)
		perr(HE_MSG,usage);
	min = (double)atof(argv[1]);
	max = (double)atof(argv[2]);
	mean = (double)atof(argv[3]);
	prob = (double)atof(argv[4]);
	read_header(&hd);
	if (hd.pixel_format != PFFLOAT)
		perr(HE_MSG,"input format must be float");
	frames = hd.num_frame;
	rows = hd.orows;
	cols = hd.ocols;
	update_header(&hd, argc, argv);
	write_header(&hd);
	framebytes = (framepix = rows*cols) * sizeof(float);

	if ((ifr = (float *)malloc(framebytes)) == NULL)
		perr(HE_MSG,"can't get core.");
	sprintf(filename, "tmp%d", getpid());
	if ((fp = fopen(filename, "w")) == NULL)
		perr(HE_OPEN, filename);

	if (max-mean > mean-min) {
		newmax = 2*mean-min;
		newmin = min;
	} else {
		newmax = max;
		newmin = 2*mean-max;
	}
	for (i = 0; i < NUMBINS; i++)
		basicbin[i] = 0;
	binsize = 1.000001*(newmax - newmin)/NUMBINS;

	for (sureclips = 0, t = 0; t < frames; t++)  {
		if (fread(ifr, framebytes,1,stdin)  != 1)
			perr(HE_MSG,"read error");
		for (ip = ifr, i = framepix; i; i--, ip++) {
			if (*ip < newmin || *ip > newmax)
				sureclips++;
			else
				basicbin[(int)((*ip - newmin)/binsize)]++;
		}
		if (fwrite(ifr, framebytes,1,fp) != 1)
			perr(HE_MSG,"write error 1");
	}
	if ((necprob =((double)sureclips)/((double)(framepix*frames))) > prob)
		fprintf(stderr, 
"must clip at least %f of the values to balance mean between max and min.\n",
			necprob);

	for (P = necprob, brace = 0; P < prob; brace++)
		P += ((double)(basicbin[brace]+basicbin[NUMBINS-1-brace]))
			/
			((double)(framepix*frames))
			;
	fprintf(stderr, "clipping %f of the sequence values.\n",
		P);
	maxval = newmax - ((double)brace)*binsize;
	minval = newmin + ((double)brace)*binsize;

	fclose(fp);
	if ((fp = fopen(filename, "r")) == NULL)
		perr(HE_OPEN, filename);
	for (t = 0; t < frames; t++)  {
		if (fread(ifr, framebytes,1,fp)  != 1)
			perr(HE_MSG,"read error");
		for (ip = ifr, i = framepix; i; i--, ip++) {
			if (((double)*ip) > maxval)
				*ip = (float)maxval;
			else if (*ip < minval)
				*ip = (float)minval;
		}
		if (fwrite(ifr, framebytes,1,stdout) != 1)
			perr(HE_MSG,"write error 2");
	}
	fclose(fp);
	sprintf(cmd,"rm %s", filename);
	system(cmd);
	exit(0);
}
