/*
 *	hupdown
 */
#include	<stdio.h>
#include	<sys/file.h>
#include	<math.h>
#include	<hipl_format.h>

#define	COINFLIP ( ( H__RANDOM() >> ( H__RANDOM() & ( H__RANDBITS >> 1))) & 01)

char	usage[] = "usage: hupdown [-s seed] < floatseq > outfile";

int main(argc, argv)
int	argc;
char	**argv;
{
			struct header	hd;
	register	float		*op;
	register	float		*pixp;
	register	int		i;
	register	int		x;
	register	int		y;
			int		t;

			float		*ofr;

			int		ord[16];
			int		frames,
					rows,
					cols,
					seed,
					jump, 
					framebytes;
			void		randord();

	Progname = strsave(*argv);
	for (i = argc; --i;	)
	/**/ if (argv[i][0] == '-')
	switch (argv[i][1]) {
	case 'u': perr(HE_MSG,usage);
	case 's': seed = atoi(argv[i+1]);
		continue;
	default: fprintf(stderr,
			"hupdown: unrecognized option: %s\n", argv[i]);
	}
	read_header(&hd);
	if (hd.pixel_format != PFFLOAT)
		perr(HE_MSG,"pixel format must be float");
	rows = hd.orows;
	cols = hd.ocols;
	if (rows % 4 || cols % 4)
		perr(HE_MSG,"image width/height must be multiples of 4");
	frames = hd.num_frame;
	update_header(&hd, argc, argv);
	write_header(&hd);
	framebytes = rows*cols * sizeof(float);

	if ((ofr = (float *)malloc(framebytes)) == NULL)
		perr(HE_MSG,"can't get core.");

	for (H__SRANDOM(seed), jump = 3*cols, t = 0; t < frames; t++)  {
		if (fread(ofr, framebytes,1,stdin) != 1)
			perr(HE_MSG,"read error");
		for (op = ofr, y = 0; y < rows; y+=4, op+=jump) {
			for (x = 0; x < cols; x+=4, op+=4) {
				randord(ord, 16);
				for (i = 0;  i < 16; i++) {
					pixp = op+(i>>2)*cols+(i&03);
					*pixp = ord[i] ? *pixp : -*pixp;
				}
			}
		}
		if (fwrite(ofr, framebytes,1,stdout) != 1)
			perr(HE_MSG,"write error");
	}
	exit(0);
}
void
randord(ord, numtrials)
int	*ord;
int	numtrials;
{
			int	randx;
	register	int	i,
				j;

	for (i = 0; i < numtrials; i++)
		ord[i] = -1;

	for (i = numtrials; i ; i--) {
		randx =  H__RANDOM() % i;
		j = -1;
		do {
			j++;
			while (ord[j] != -1)
				j++;
		} while (randx--);
		ord[j] = (i & 01);
	}
}
