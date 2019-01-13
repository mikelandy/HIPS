/*
 *     isobp	
 */
#include	<stdio.h>
#include	<sys/file.h>
#include	<math.h>
#include	<hipl_format.h>

#define	INIT_HEADER(hdp, frames, rows, cols)\
		init_header(hdp, " ", " ", frames, " ", rows, cols,\
			8*sizeof(complex), 0, PFCOMPLEX, " ")
#define	DMAXSIGNED	((double)(((unsigned)(~0)) >> 1))
typedef         float           complex[2];
char	usage[] = 
"usage: [-u] [-C center_freq] [-S standard_devaition] < inseq > outseq\n\
\n\
defaults: center_freq = 32, standard_deviation = 8";

int main(argc, argv)
int	argc;
char	**argv;
{
			struct header	hd;
	register	int		x;
	register	float		*amplp;
	register	int		y;
	register	int		i;
			int		t;

			complex		*ofr;

			int		frames,
					rows,
					cols,
					rp1_o_2,
					cp1_o_2,
					framebytes;
			double		exparg,
					X,
					Y,
					two_sig_sq = 2.*8.*8.;
			float		*ampl;
			double		C = 32.;

	Progname = strsave(*argv);
	for (i = argc; --i;	)
	/**/ if (argv[i][0] == '-')
	switch (argv[i][1]) {
	case 'u': perr(HE_MSG,usage);
	case 'C': C = (double)atof(argv[i+1]);
		continue;
	case 'S': two_sig_sq = (double)atof(argv[i+1]);
		two_sig_sq *= (2.*two_sig_sq);
                continue;
	default: fprintf(stderr,
			"isobp: unrecognized option: %s\n", argv[i]);
	}
	read_header(&hd);
	if (hd.pixel_format != PFCOMPLEX)
		perr(HE_MSG,"input must be PFCOMPLEX");
	frames = hd.num_frame;
	rows = hd.orows;
	cols = hd.ocols;
	update_header(&hd, argc, argv);
	write_header(&hd);
	framebytes = rows*cols * sizeof(complex);
		
	if ((ampl = (float *)malloc(rows*cols*sizeof(float))) == NULL)
		perr(HE_MSG,"can't get core.");
	
	if ((ofr = (complex *)malloc(framebytes)) == NULL)
		perr(HE_MSG,"can't get core.");
	cp1_o_2 = (cols+1)/2;
	rp1_o_2 = (rows+1)/2;
	for (amplp = ampl, y = 0; y < rows; y++)
	/**/ for (x = 0; x < cols; x++, amplp++) {
		if (x < cp1_o_2)
			X = ((double)x);
		else
			X = ((double)(cols-x));
		if (y < rp1_o_2)
			Y = ((double)y);
		else
			Y = ((double)(rows-y));
		exparg = C - sqrt(X*X + Y*Y);
		*amplp = exp(-exparg*exparg/two_sig_sq);
	}
	for (t = frames; t; t--) {
		if (fread(ofr, framebytes,1,stdin) != 1)
			perr(HE_MSG,"read error");
		for (amplp = ampl, y = 0; y < rows; y++)
		/**/ for (x = 0; x < cols; x++, amplp++) {
			(*(ofr + y*cols + x))[0] *= *amplp;
			(*(ofr + y*cols + x))[1] *= *amplp;
		}
		if (fwrite(ofr, framebytes,1,stdout) != 1)
			perr(HE_MSG,"write error");
	}
	exit(0);
}
