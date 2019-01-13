/*
%
%	TOVIDA . C
%
%	The format for the input file is any image.
%	The format for the output file is a color RLE file,
*/
char	usage[]="\
-k		keep going when error hanppened\n\
-o name_string	out going file name\n\
[<] input\n";
/*
% AUTHOR:	Jin Guojun, LBL - 1991
*/

#include "header.def"
#include "imagedef.h"

typedef	struct	{
	int	size;
	char	data_type[10],
		name[18];
	int	extents;
	short	session_error;
	char	regular,
		hkey_un0;
	}hdr_key;

typedef	struct	{
	short	dim[15],
		datatype,
		bitpix,
		dim_un0;
	float	pixdim[16];
	int	glmax, glmin;
	} image_dim;

typedef	struct	{
	char	descrip[80],
		aux_file[24],
		orient,
		originator[10],
		generated[10],
		scannum[10],
		patient_id[10],
		exp_date[10],
		exp_time[10],
		hist_un0[3];
	int	vieww,
		vols_added,
		start_field,
		omax, omin,
		smax, smin;
	} hdr_history;

typedef	struct	{
	hdr_key		key;
	image_dim	img;
	hdr_history	his;
	} analyze_db;

analyze_db	idb;
U_IMAGE	uimg;
bool	nostop;
#define	rows	uimg.height
#define	cols	uimg.width

main(ac, av)
int	ac;
char*	av[];
{
char	*fname, ofname[64];
int	row;
FILE	*hdf;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, HIPS, *av, "D20-1");

for (row=1; row<ac; row++)
    if (*av[row] == '-') {
	switch (av[row][1])
	{
	case 'k':	nostop++;	break;
	case 'o':	fname = av[++row];	break;
	default:
errout:		usage_n_options(usage, row, av[row]);
	}
    }
    else if (freopen(uimg.name=av[row], "rb", stdin) != stdin)
		syserr("input file -- %s", av[row]);

io_test(fileno(in_fp), goto	errout);
strncpy(idb.key.name, uimg.name, 17);
if (!fname)
	fname = uimg.name;
strcpy(ofname, fname);
strcat(ofname, ".hdr");
hdf = fopen(ofname, "w");
strcpy(ofname, fname);
strcat(ofname, ".img");
out_fp = freopen(ofname, "wb", stdout);
if (!hdf || !out_fp)
	syserr("open file %s", fname);

	if ((*uimg.header_handle)(HEADER_READ, &uimg, 0, 0, 0) < 0)
		syserr("unknown image type");

	(*uimg.std_swif)(FI_LOAD_FILE, &uimg, nostop ? NULL : uimg.name, 0);

	idb.key.size = sizeof(idb);
	idb.img.dim[0] = cols;
	idb.img.dim[1] = rows;
	idb.img.dim[2] = uimg.frames;
	idb.img.pixdim[0] = 1.0;
	idb.img.pixdim[1] = 1.0;
	idb.img.pixdim[2] = 1.0;
	idb.img.datatype = uimg.in_form==IFMT_SHORT ? 4 : 2;
	idb.img.bitpix = (uimg.pxl_out=uimg.pxl_in) << 3;

	fwrite(&idb, sizeof(idb), 1, hdf);	/* write header */
	(*uimg.std_swif)(FI_SAVE_FILE, &uimg, No, NULL); /* output data */
}
