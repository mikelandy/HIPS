/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * hips_basic.h - basic definitions for HIPS
 *
 * Michael Landy - 12/28/90
 */

/*
 * Machine-dependent portion
 *
 * The next lines are the only portion of the file which should be tailored
 * to an individual installation.
 */

typedef unsigned long hsize_t;	/* variable which can hold the size of an
				    image in bytes */
#ifdef AIX
#define HPUXORAIX
#endif

#ifdef HPUX
#define HPUXORAIX
#endif

#ifdef	HPUXORAIX

#define	H__RANDOM	lrand48	/* routine to call for random numbers */
#define H__RANDTYPE	long	/* type of H__RANDOM() */
#define	H__SRANDOM	srand48	/* routine to call to set the seed */
#define	H__MAXRAND	(0x7fffffff)	/* maximum random number */
#define	H__RANDBITS	(31)	/* number of random bits returned */

#else

#define	H__RANDOM	random	/* routine to call for random numbers */
#ifdef	__alpha
#define H__RANDTYPE	int	/* type of H__RANDOM() */
#else
#define H__RANDTYPE	long	/* type of H__RANDOM() */
#endif
#define	H__SRANDOM	srandom	/* routine to call to set the seed */
#define	H__MAXRAND	(0x7fffffff)	/* maximum random number */
#define	H__RANDBITS	(31)	/* number of random bits returned */

#endif

#ifdef	SOLARIS

#undef	H__RANDOM
#undef	H__SRANDOM
#define	H__RANDOM	rand
#define	H__SRANDOM	srand

#endif

/* *******************END OF MACHINE-DEPENDENT PORTION*********************/

typedef	unsigned char	byte;
typedef	char		sbyte;
typedef	unsigned short	h_ushort;
typedef	unsigned int	h_uint;
typedef float		h_complex[2];
typedef double		h_dblcom[2];
typedef char *		Filename;

union pixelval {
	byte v_byte;
	sbyte v_sbyte;
	short v_short;
	h_ushort v_ushort;
	int v_int;
	h_uint v_uint;
	float v_float;
	double v_double;
	h_complex v_complex;
	h_dblcom v_dblcom;
};

typedef union pixelval	Pixelval;

/*
 * For general readability
 */

#ifndef TRUE
# define	TRUE	1
# define	FALSE	0
#endif

typedef	int	h_boolean;

/*
 * Histogram structure
 *
 * The zero-th bin is underflows.  The last bin is overflows. So there are
 * nbins+2 slots.  The n'th bin counts pixels such that:
 *
 *	min + ((n-1)*binwidth) <= value < min + n*binwidth
 *
 * For complex and double complex images, the complex magnitude is
 * histogrammed, and min/binwidth are either floats (for complex images) or
 * doubles (for double complex images).
 */

struct hips_histo {
	int nbins;
	int *histo;
	hsize_t sizehist;
	h_boolean histodealloc;
	int pixel_format;
	Pixelval minbin;
	Pixelval binwidth;
};

/*
 * Statistics structure
 *
 * The variable nelem counts the number of pixels that contributed to  these
 * image statistics (which might be less than the number of pixels in the
 * region-of-interest if zero-valued pixels aren't included.
 */

struct hips_stats {
	int nelem;
	int pixel_format;
	Pixelval statmin;
	Pixelval statmax;
	double sum,ssq,mean,var,stdev;
};

/*
 * Convolution mask set structure
 */

struct hips_mask {
	char *name;		/* name of this mask set */
	int nmasks;		/* number of masks */
	int func_num;		/* function applied to mask outputs */
	int pixel_format;	/* format of mask elements */
	union {
	float **f_values;	/* float mask pointers */
	int **i_values;		/* int mask pointers */
	} vals;
	int *mask_rows;		/* number of rows in each mask */
	int *mask_cols;		/* number of columns in each mask */
	int *row_offset;	/* row number of mask value overlying image
					pixel */
	int *col_offset;	/* column number of mask value overlying image
					pixel */
};

/*
 * Mask function numbers
 */

#define MASKFUN_MAXABS          1
#define MASKFUN_MEANSQ          2
#define MASKFUN_SUMABS          3
#define MASKFUN_MAX             4
#define MASKFUN_MAXFLR          5
#define MASKFUN_MXASFLR         6
#define MASKFUN_MUL             7
#define MASKFUN_NORM            8
#define MASKFUN_DIFF            9
#define MASKFUN_ORIENT          10
#define MASKFUN_IDENT           11
#define MASKFUN_MAXMASKS        11

/*
 * Filter types and structure
 *
 * A bandpass filter is a concatenation (i.e., product) of a lowpass (using
 * highcut/highorder) and a highpass (using lowcut/loworder) filter.  A band
 * reject filter is one minus the corresponding bandpass filter.
 */

#define FILTMETHOD_IDEAL	1
#define FILTMETHOD_BUTTERWORTH	2
#define FILTMETHOD_EXPONENTIAL	3

#define	FILTDIST_ROW		1
#define	FILTDIST_COL		2
#define	FILTDIST_BOTH		3

#define	FILTTYPE_LOWPASS	1
#define	FILTTYPE_HIGHPASS	2
#define	FILTTYPE_BANDPASS	3
#define	FILTTYPE_BANDREJ	4

struct hips_filter {
	int method;		/* Ideal/Butterworth/Exponential */
	int disttype;		/* scale by number of rows, columns or both */
	int ftype;		/* lowpass/highpass/bandpass/bandreject */
	double dmetric;		/* Minkowski metric */
	double lowcut;
	int loworder;
	double highcut;
	int highorder;
};

char *strsave(),*memalloc(),*formatheader(),*formatheadera();
char *formatheaderc(),*hformatname(),*hformatname_f(),*hformatname_t();
byte *halloc(),*hmalloc();
hsize_t hsizepix(),hbitsperpixel();
struct extpar *findparam(),*grepparam();
FILE *hfopenr(),*ffopen(),*ffreopen();
h_boolean swallownl(),hfgets(),type_is_col3(),ptype_is_col3();
void setupconvback();
int ffind_method(),fset_converstion(),in_typeslist(),pfind_closest();
int alloc_image(),dup_header(),pset_conversion(),setformat(),setpyrformat();
int fhconvert(),free_image(),init_hdr_alloc(),setsize();
int history_indentadd();
int addpoint(),addvec(),addend(),alloc_histo(),alloc_histobins(),alloc_image();
int alloc_imagez(),free_image(),readcmap(),find_closest(),ffind_closest();
int pfind_closest(),in_typeslist(),find_method(),ffind_method();
int set_conversion(),fset_conversion(),pset_conversion(),hconvert();
int fhconvert(),hconvertback(),read_imagec(),fread_imagec(),write_imagec();
int fwrite_imagec(),cut_frame(),desc_set(),desc_append();
int desc_indentadd(),dup_header(),dup_headern(),fread_oldhdr();
int free_header(),free_hdrcon(),ffread(),ffwrite(),getplot(),h_col1torgb();
int h_col1torgbz(),h_col1tozrgb(),h_col1tobgr(),h_col1tobgrz(),h_col1tozbgr();
int h_btorgb_1(),h_itorgb_1(),h_btorgbz_1(),h_itorgbz_1(),h_btozrgb_1();
int h_itozrgb_1(),h_btobgr_1(),h_itobgr_1(),h_btobgrz_1(),h_itobgrz_1();
int h_btozbgr_1(),h_itozbgr_1(),h_1to3_b(),h_1to3_i(),h_1to4_b();
int h_1to4_i(),h_col3tob(),h_rgbtob_1(),h_rgbztob_1(),h_zrgbtob_1();
int h_bgrtob_1(),h_bgrztob_1(),h_zbgrtob_1(),h_3to1_b(),h_4to1_b();
int h_torgb(),h_torgbz(),h_tozrgb(),h_tobgr(),h_tobgrz(),h_tozbgr();
int h_rgbztorgb(),h_zrgbtorgb(),h_bgrtorgb(),h_bgrztorgb(),h_zbgrtorgb();
int h_rgbtorgbz(),h_zrgbtorgbz(),h_bgrtorgbz(),h_bgrztorgbz(),h_zbgrtorgbz();
int h_rgbtozrgb(),h_rgbztozrgb(),h_bgrtozrgb(),h_bgrztozrgb(),h_zbgrtozrgb();
int h_rgbtobgr(),h_rgbztobgr(),h_zrgbtobgr(),h_bgrztobgr(),h_zbgrtobgr();
int h_rgbtobgrz(),h_rgbztobgrz(),h_zrgbtobgrz(),h_bgrtobgrz(),h_zbgrtobgrz();
int h_rgbtozbgr(),h_rgbztozbgr(),h_zrgbtozbgr(),h_bgrtozbgr(),h_bgrztozbgr();
int h_col3toi(),h_rgbtoi_1(),h_rgbztoi_1(),h_zrgbtoi_1(),h_bgrtoi_1();
int h_bgrztoi_1(),h_zbgrtoi_1(),h_btorgb(),h_btorgbz(),h_rgbtob();
int h_rgbztob(),h_rgbtob2(),h_rbgztob2(),h_tob(),h_toc(),h_tod(),h_todc();
int h_tof(),h_toi(),h_tolp(),h_tomp(),h_tos(),h_tosb(),h_toui(),h_tous();
int hgetdepth(),hsetdepth(),init_header(),init_hdr_alloc(),init_header_d();
int init_hdr_alloc_d(),read_num_mask(),read_mask(),free_maskcon();
int mask_itof(),hformatlevel(),maxformat(),set_defaults(),set_flag_defaults();
int set_filename_defaults(),pix_code();
int def_ipyr(),alloc_ipyr(),free_ipyr(),def_fpyr(),alloc_fpyr(),free_fpyr();
int alloc_iimage(),alloc_fimage(),free_iimage(),free_fimage(),copy_itoii();
int copy_ftoff(),getpyrfilters(),read_iimage(),read_fimage(),write_iimage();
int write_fimage(),read_ipyr(),read_fpyr(),write_ipyr(),write_fpyr();
int pyrnumpix(),freduce(),fexpand(),ireduce(),iexpand(),read_frame();
int read_header(),fread_header(),read_histo(),fread_histo(),hdr_to_histo();
int read_hdr_a(),fread_hdr_a(),read_hdr_cpf(),fread_hdr_cpf(),read_hdr_cpfa();
int fread_hdr_cpfa(),fread_hdr_cpfac(),read_hdr_cc(),fread_hdr_cc();
int read_hdr_cca(),fread_hdr_cca(),fread_hdr_ccac(),read_image(),fread_image();
int read_roi(),fread_roi(),setformat(),setpyrformat(),setroi(),setroi2();
int getroi(),clearroi(),setsize(),trans_frame(),update_header();
int update_headern(),update_headerc(),view_frame(),write_frame();
int write_headeru(),write_headeru2(),write_headerun(),write_header();
int fwrite_header(),write_histo(),fwrite_histo(),histo_to_hdr(),write_image();
int fwrite_image(),write_roi(),fwrite_roi(),wnocr(),dfprintf();
int clearparam(),mergeparam();
int h_mptob(),h_lptob(),h_stob(),h_itob(),h_ftob();
int h_itoc(),h_ftoc(),h_dtoc(),h_dctoc();
int h_itod(),h_ftod(),h_ctod(),h_dctod();
int h_itodc(),h_ftodc(),h_dtodc(),h_ctodc();
int h_btof(),h_stof(),h_itof(),h_dtof(),h_ctof(),h_dctof();
int h_mptoi(),h_lptoi(),h_btoi(),h_sbtoi(),h_stoi(),h_ustoi(),h_stoi();
int h_uitoi(),h_ftoi(),h_dtoi(),h_ctoi(),h_dctoi();
int h_btolp(),h_itolp();
int h_btomp(),h_itomp();
int h_btos(),h_sbtos(),h_itos(),h_ftos();
int h_stosb(),h_itosb();
int h_itoui();
int h_itous();

int h_abdou(),h_abdou_b(),h_abdou_B(),h_abs(),h_abs_i(),h_abs_f(),h_abs_I(),
	h_abs_F(),h_absdiff(),h_absdiff_b(),h_absdiff_s(),
	h_absdiff_i(),h_absdiff_f(),
	h_absdiff_d(),h_absdiff_B(),h_absdiff_S(),h_absdiff_I(),h_absdiff_F(),
	h_absdiff_D(),h_add(),h_add_bii(),h_add_bsb(),h_add_s(),h_add_i(),
	h_add_f(),h_add_d(),h_add_c(),h_add_dc(),h_add_ip(),h_add_fp(),
	h_add_BII(),h_add_BSB(),h_add_S(),h_add_I(),h_add_F(),h_add_D(),
	h_add_C(),h_add_DC(),h_addcos(),h_addcos_f(),h_addcos_F(),
	h_addgabor(),h_addgabor_f(),h_addgabor_F(),h_affine(),h_affine_b(),
	h_affine_B(),h_and(),h_and_mp(),h_and_lp(),h_and_b(),h_and_MP(),
	h_and_LP(),h_and_B(),h_applylut(),h_applylut_b(),h_applylut_s(),
	h_applylut_i(),h_applylut_B(),h_applylut_S(),h_applylut_I(),
	h_avg(),h_avg_b(),h_avg_s(),h_avg_i(),h_avg_f(),h_avg_B(),
	h_avg_S(),h_avg_I(),h_avg_F(),h_bclean(),h_bclean_b(),h_bclean_B(),
	h_bnoise(),h_bnoise_b(),h_bnoise_i(),h_bnoise_f(),h_bnoise_B(),
	h_bnoise_I(),h_bnoise_F(),h_btorle(),h_Btorle(),h_checkers();
int h_checkers_b(),h_checkers_B(),h_clearhisto(),h_Clearhisto(),h_colorkey(),
	h_colorkey_bb(),h_colorkey_bi(),h_colorkey_bf(),h_colorkey_ib(),
	h_colorkey_ii(),h_colorkey_if(),h_colorkey_ipip(),h_colorkey_ipfp(),
	h_colorkey_BB(),h_colorkey_BI(),h_colorkey_BF(),h_colorkey_IB(),
	h_colorkey_II(),h_colorkey_IF(),h_combine(),h_combine_f(),
	h_combine_d(),h_combine_F(),h_combine_D(),h_conj(),h_conj_c(),
	h_conj_dc(),h_conj_C(),h_conj_DC(),h_convolve(),h_convolve_i(),
	h_convolve_f(),h_convolve_I(),h_convolve_F(),h_copy(),h_copy_mp(),
	h_copy_lp(),h_copy_b(),h_copy_s(),h_copy_i(),h_copy_f(),h_copy_d(),
	h_copy_c(),h_copy_dc(),h_copy_MP(),h_copy_LP(),h_copy_B(),h_copy_S(),
	h_copy_I(),h_copy_F(),h_copy_D(),h_copy_C(),h_copy_DC(),h_correl(),
	h_correl_f(),h_correl_F(),h_dct(),h_dct_f(),h_dct_d(),h_dct_F(),
	h_dct_D(),h_dctinv(),h_dctinv_f(),h_dctinv_d(),h_dctinv_F(),
	h_dctinv_D(),h_dct2d_f(),h_dctinv2d_f(),h_dct2d_d(),h_dctinv2d_d(),
	h_diff(),h_diff_s(),h_diff_i(),h_diff_ibi(),h_diff_f(),
	h_diff_d(),h_diff_c(),h_diff_dc(),h_diff_ip(),h_diff_fp(),h_diff_S(),
	h_diff_I(),h_diff_IBI(),h_diff_F(),h_diff_D(),h_diff_C(),h_diff_DC(),
	h_discedge(),h_discedge_b(),h_discedge_B(),h_discedge2(),h_discedge2_b();
int h_discedge2_B(),h_discedge2_B(),h_disphist(),h_disphist_b(),h_disphist_B(),
	h_dither(),h_dither_b(),h_dither_B(),h_div(),h_div_b(),h_div_s(),
	h_div_i(),h_div_f(),h_div_fc(),h_div_d(),h_div_ddc(),h_div_cf(),
	h_div_c(),h_div_dcd(),h_div_dc(),h_div_ip(),h_div_fp(),h_div_B(),
	h_div_S(),h_div_I(),h_div_F(),h_div_FC(),h_div_D(),h_div_DDC(),
	h_div_CF(),h_div_C(),h_div_DCD(),h_div_DC(),h_divscale(),
	h_divscale_s(),h_divscale_i(),h_divscale_ib(),h_divscale_if(),
	h_divscale_f(),h_divscale_c(),h_divscale_dc(),h_divscale_S(),
	h_divscale_I(),h_divscale_IB(),h_divscale_IF(),h_divscale_F(),
	h_divscale_C(),h_divscale_DC(),h_dotdiff(),h_dotdiff_b(),h_dotdiff_S(),
	h_enlarge(),h_enlarge_b(),h_enlarge_i(),h_enlarge_f(),h_enlarge_c(),
	h_enlarge_B(),h_enlarge_I(),h_enlarge_F(),h_enlarge_C(),h_entropycnt(),
	h_entropycnt_b(),h_entropycnt_B(),h_exp(),h_exp_b(),h_exp_s(),
	h_exp_i(),h_exp_f(),h_exp_B(),h_exp_S(),h_exp_I(),h_exp_F(),
	h_extract(),h_extremum(),h_extremum_b(),h_extremum_B(),h_fastaddcos(),
	h_fastaddcos_f(),h_fastaddcos_F(),h_fft_ri_c(),h_fft2d_ri_c();
int h_fft2dgen_ri_c(),h_fftn_ri_c(),h_fft_ri_dc(),h_fft2d_ri_dc(),
	h_fft2dgen_ri_dc(),h_fftn_ri_dc(),h_filter(),h_filter_f(),h_filter_d(),
	h_filter_c(),h_filter_dc(),h_filter_F(),h_filter_D(),h_filter_C(),
	h_filter_DC(),h_flipquad(),h_flipquad_b(),h_flipquad_f(),
	h_flipquad_d(),h_flipquad_B(),h_flipquad_F(),h_flipquad_D(),
	h_fourtr(),h_fourtr_c(),h_fourtr_dc(),h_fourtr_C(),h_fourtr_DC(),
	h_fft_c(),h_fft2d_c(),h_fft2dgen_c(),h_fftn_c(),h_fft_dc(),
	h_fft2d_dc(),h_fft2dgen_dc(),h_fftn_dc(),h_fourtr3d(),h_fourtr3d_c(),
	h_fourtr3d_C(),h_gnoise(),h_gnoise_s(),h_gnoise_f(),h_gnoise_S(),
	h_gnoise_F(),srand_g(),h_greybar(),h_greybar_b(),h_greybar_B(),
	h_gridwarp(),h_gridwarp_b(),h_gridwarp_B(),h_halftone(),h_halftone2(),
	h_halftone_b(),h_halftone_B(),h_hardthresh(),h_hardthresh_b(),
	h_hardthresh_i(),h_hardthresh_f(),h_hardthresh_c(),h_hardthresh_B(),
	h_hardthresh_I(),h_hardthresh_F(),h_hardthresh_C(),h_hconvolve(),
	h_hconvolve_i(),h_hconvolve_f(),h_hconvolve_I(),h_hconvolve_F(),
	h_histo(),h_histo_b(),h_histo_sb(),h_histo_s(),h_histo_us(),
	h_histo_i(),h_histo_ui(),h_histo_f(),h_histo_d(),h_histo_c(),
	h_histo_dc(),h_histo_B(),h_histo_SB(),h_histo_S(),h_histo_US(),
	h_histo_I(),h_histo_UI(),h_histo_F(),h_histo_D(),h_histo_C(),
	h_histo_DC(),h_histoeq(),h_histoeq_b(),h_histoeq_B(),h_ienlarge();
int h_ienlarge_b(),h_ienlarge_i(),h_ienlarge_f(),h_ienlarge_c(),h_ienlarge_B(),
	h_ienlarge_I(),h_ienlarge_F(),h_ienlarge_C(),h_ienlarge3(),
	h_ienlarge3_b(),h_ienlarge3_i(),h_ienlarge3_f(),h_ienlarge3_c(),
	h_ienlarge3_B(),h_ienlarge3_I(),h_ienlarge3_F(),h_ienlarge3_C(),
	h_invert(),h_invert_mp(),h_invert_lp(),h_invert_b(),h_invert_i(),
	h_invert_f(),h_invert_MP(),h_invert_LP(),h_invert_B(),h_invert_I(),
	h_invert_F(),h_invfourtr(),h_invfourtr_c(),h_invfourtr_dc(),
	h_invfourtr_C(),h_invfourtr_DC(),h_invfourtr3d(),h_invfourtr3d_c(),
	h_invfourtr3d_C(),h_linscale(),h_linscale_b(),h_linscale_s(),
	h_linscale_i(),h_linscale_f(),h_linscale_B(),h_linscale_S(),
	h_linscale_I(),h_linscale_F(),h_log(),h_log_b(),h_log_s(),
	h_log_i(),h_log_f(),h_log_B(),h_log_S(),h_log_I(),h_log_F(),
	h_mask(),h_mask_bif(),h_mask_bff(),h_mask_iif(),h_mask_iff(),
	h_mask_fff(),h_mask_BIF(),h_mask_BFF(),h_mask_IIF(),h_mask_IFF(),
	h_mask_FFF(),h_mask(),h_mask_b(),h_mask_sb(),h_mask_s(),h_mask_us(),
	h_mask_i(),h_mask_ui(),h_mask_f(),h_mask_d(),h_mask_B(),h_mask_SB(),
	h_mask_S(),h_mask_US(),h_mask_I(),h_mask_UI(),h_mask_F(),h_mask_D();
int h_max(),h_max_b(),h_max_sb(),h_max_s(),h_max_us(),h_max_i(),h_max_ui(),
	h_max_f(),h_max_d(),h_max_I(),h_maxabsp(),h_maxabsp_s(),
	h_maxabsp_i(),h_maxabsp_f(),h_maxabsp_d(),
	h_maxabsp_ip(),h_maxabsp_fp(),h_maxabsp_S(),h_maxabsp_I(),
	h_maxabsp_F(),h_maxabsp_D(),h_maxhisto(),h_Maxhisto(),h_maxp(),
	h_maxp_b(),h_maxp_s(),h_maxp_i(),h_maxp_f(),h_maxp_d(),h_maxp_ip(),
	h_maxp_fp(),h_maxp_B(),h_maxp_S(),h_maxp_I(),h_maxp_F(),h_maxp_D(),
	h_mean(),h_mean_f(),h_median(),h_median_b(),h_median_B(),
	h_minabsp(),h_minabsp_s(),h_minabsp_i(),h_minabsp_f(),h_minabsp_d(),
	h_minabsp_ip(),h_minabsp_fp(),h_minabsp_S(),h_minabsp_I(),
	h_minabsp_F(),h_minabsp_D(),h_minmax(),h_minmax_b(),h_minmax_sb(),
	h_minmax_s(),h_minmax_us(),h_minmax_i(),h_minmax_ui(),h_minmax_f(),
	h_minmax_d(),h_minmax_c(),h_minmax_dc(),h_minmax_B(),h_minmax_SB(),
	h_minmax_S(),h_minmax_US(),h_minmax_I(),h_minmax_UI(),h_minmax_F(),
	h_minmax_D(),h_minmax_C(),h_minmax_DC(),h_minp(),h_minp_b(),h_minp_s(),
	h_minp_i(),h_minp_f(),h_minp_d(),h_minp_ip(),h_minp_fp(),h_minp_B(),
	h_minp_S(),h_minp_I(),h_minp_F(),h_minp_D(),h_minroi(),h_minroi_b(),
	h_minroi_B(),h_morphdil(),h_morphdil_b(),h_morphdil_B(),h_morphero(),
	h_morphero_b(),h_morphero_B(),h_mul(),h_mul_b(),h_mul_s(),h_mul_i(),
	h_mul_f(),h_mul_fc(),h_mul_d(),h_mul_ddc(),h_mul_c(),h_mul_dc(),
	h_mul_ip(),h_mul_fp(),h_mul_B(),h_mul_S(),h_mul_I(),h_mul_F(),
	h_mul_FC(),h_mul_D(),h_mul_DDC(),h_mul_C(),h_mul_DC(),h_mulscale(),
	h_mulscale_b(),h_mulscale_s(),h_mulscale_i(),h_mulscale_f(),
	h_mulscale_d(),h_mulscale_B(),h_mulscale_S(),h_mulscale_I();
int h_mulscale_F(),h_mulscale_D(),h_neg(),h_neg_mp(),h_neg_lp(),h_neg_b(),
	h_neg_s(),h_neg_i(),h_neg_f(),h_neg_MP(),h_neg_LP(),h_neg_B(),
	h_neg_S(),h_neg_I(),h_neg_F(),h_noise(),h_noise_b(),h_noise_B(),
	h_nonisot(),h_nonisot_i(),h_nonisot_I(),h_or(),h_or_mp(),h_or_lp(),
	h_or_b(),h_or_MP(),h_or_LP(),h_or_B(),h_pixmap(),h_pixmap_b(),
	h_pixmap_B(),h_power(),h_power_b(),h_power_s(),h_power_i(),
	h_power_f(),h_power_B(),h_power_S(),h_power_I(),h_power_F(),
	h_pyrdisp(),h_pyrdisp_i(),h_pyrdisp_f(),h_pyrdisp_I(),h_pyrdisp_F(),
	h_quadscale(),h_quadscale_b(),h_quadscale_s(),h_quadscale_i(),
	h_quadscale_f(),h_quadscale_B(),h_quadscale_S(),h_quadscale_I(),
	h_quadscale_F(),h_reduce(),h_reduce_bi(),h_reduce_s(),h_reduce_i(),
	h_reduce_f(),h_reduce_c(),h_reduce_BI(),h_reduce_S(),h_reduce_I(),
	h_reduce_F(),h_reduce_C(),h_reflect(),h_reflect_b(),h_reflect_i(),
	h_reflect_f(),h_reflect_B(),h_reflect_I(),h_reflect_F(),h_rletob(),
	h_rletoB(),h_rot180(),h_rot180_b(),h_rot180_i(),h_rot180_f(),
	h_rot180_B(),h_rot180_I(),h_rot180_F(),h_rot90(),h_rot90_b(),
	h_rot90_i(),h_rot90_f(),h_rot90_B(),h_rot90_I(),h_rot90_F(),
	h_sample(),h_sample_b(),h_sample_s(),h_sample_i(),h_sample_f();
int h_sample_d(),h_sample_c(),h_sample_dc(),h_sample_B(),h_sample_S(),
	h_sample_I(),h_sample_F(),h_sample_D(),h_sample_C(),h_sample_DC(),
	h_scaleadd(),h_scaleadd_f(),h_scaleadd_F(),h_sepconv(),h_sepconv_i(),
	h_sepconv_f(),h_seqord(),h_seqord_i(),h_seqord_f(),h_seqord_I(),
	h_seqord_F(),h_invseqord(),h_invseqord_i(),h_invseqord_f(),
	h_invseqord_I(),h_invseqord_F(),h_setimage(),h_setimage_mp(),
	h_setimage_lp(),h_setimage_b(),h_setimage_sb(),h_setimage_s(),
	h_setimage_us(),h_setimage_i(),h_setimage_ui(),h_setimage_f(),
	h_setimage_d(),h_setimage_c(),h_setimage_dc(),h_setimage_MP(),
	h_setimage_LP(),h_setimage_B(),h_setimage_SB(),h_setimage_S(),
	h_setimage_US(),h_setimage_I(),h_setimage_UI(),h_setimage_F(),
	h_setimage_D(), h_setimage_C(),h_setimage_DC(),h_shift(),h_shift_b(),
	h_shift_i(),h_shift_B(),h_shift_I(),h_shuffleadd(),h_shuffleadd_bsb(),
	h_shuffleadd_f(),h_shuffleadd_BSB(),h_shuffleadd_F(),h_slice(),
	h_slice_b(),h_slice_B(),h_softthresh(),h_softthresh_b(),
	h_softthresh_i(),h_softthresh_f(),h_softthresh_c(),h_softthresh_B(),
	h_softthresh_I(),h_softthresh_F(),h_softthresh_C(),h_stats(),
	h_stats_b(),h_stats_f(),h_stats_B(),h_stats_F(),h_stretch(),
	h_stretch_b(),h_stretch_s(),h_stretch_B(),h_stretch_S(),h_stretchimg(),
	h_stretchimg_b(),h_stretchimg_B(),h_thicken(),h_thicken_b();
int h_thicken_B(),h_thin(),h_thin_b(),h_thin_B(),h_translate(),h_translate_b(),
	h_translate_s(),h_translate_i(),h_translate_f(),h_translate_d(),
	h_translate_c(),h_translate_dc(),h_translate_B(),h_translate_S(),
	h_translate_I(),h_translate_F(),h_translate_D(),h_translate_C(),
	h_translate_DC(),h_transpose(),h_transpose_b(),h_transpose_i(),
	h_transpose_f(),h_transpose_B(),h_transpose_I(),h_transpose_F(),
	h_vconvolve(),h_vconvolve_i(),h_vconvolve_f(),h_vconvolve_I(),
	h_vconvolve_F(),h_walshtr(),h_walshtr_i(),h_walshtr_f(),h_walshtr_I(),
	h_walshtr_F(),h_fwt_i(),h_fwt_f(),h_wgauss(),h_wgauss_f(),
	h_wgauss_F(),h_wtsum3(),h_wtsum3_b(),h_wtsum3_s(),h_wtsum3_i(),
	h_wtsum3_f(),h_wtsum3_B(),h_wtsum3_S(),h_wtsum3_I(),h_wtsum3_F(),
	h_xor(),h_xor_mp(),h_xor_lp(),h_xor_b(),h_xor_MP(),h_xor_LP(),
	h_xor_B(),h_zc(),h_zc_f(),h_zc_F(),h_zoneplate(),h_zoneplate_f(),
	h_zoneplate_F();
byte h_max_B();
sbyte h_max_SB();
short h_max_S();
h_ushort h_max_US();
h_uint h_max_UI();
float h_max_F(),h_mean_F();
double h_entropy(),h_gaussmask(),h_max_D();

/* definitions of hipsaddon library routines */

int h_asl(),h_asl_b(),h_asl_f(),h_asl_B(),h_asl_F(),h_average(),h_average_b(),
	h_average_B(),h_cshift(),h_cshift_b(),h_cshift_s(),h_cshift_i(),
	h_cshift_f(),h_cshift_d(),h_cshift_c(),h_cshift_dc(),h_cshift_B(),
	h_cshift_S(),h_cshift_I(),h_cshift_F(),h_cshift_D(),h_cshift_C(),
	h_cshift_DC(),h_csinegen(),h_csinegen_f(),h_csinegen_d(),
	h_csinegen_F(),h_csinegen_D(),h_ellipse(),h_ellipse_b(),h_ellipse_B(),
	h_extend(),h_extend_b(),h_extend_f(),h_extend_d(),h_extend_B(),
	h_extend_F(),h_extend_D(),h_framing(),h_framing_b(),h_framing_B(),
	h_grey2disp(),h_grey2disp_b(),h_grey2disp_B(),h_meanfilt(),
	h_meanfilt_b(),h_meanfilt_B(),h_mls(),h_mls_b(),h_mls_B(),
	h_nns(),h_nns_b(),h_nns_B(),h_project(),h_project_b(),h_project_B(),
	h_rank(),h_rank_b(),h_rank_s(),h_rank_i(),h_rank_f(),h_rank_d(),
	h_rank_B(),h_rank_S(),h_rank_I(),h_rank_F(),h_rank_D(),h_hor_rank_B(),
	h_hor_rank_S(),h_hor_rank_I(),h_hor_rank_F(),h_hor_rank_D(),
	h_ver_rank_B(),h_ver_rank_S(),h_ver_rank_I(),h_ver_rank_F(),
	h_ver_rank_D(),h_rotate(),h_rotate_b(),h_rotate_f(),h_rotate_B(),
	h_rotate_F(),h_rsinegen(),h_rsinegen_f(),h_rsinegen_d(),h_rsinegen_F(),
	h_rsinegen_D(),h_sasmooth(),h_sasmooth_b(),h_sasmooth_B(),
	h_sigmaspat(),h_sigmaspat_b(),h_sigmaspat_B(),h_sigmatemp(),
	h_sigmatemp_b(),h_sigmatemp_B(),h_snn(),h_snn_b(),h_snn_B();
int h_translatei(),h_translatei_b(),h_translatei_B(),h_unoise(),h_unoise_b(),
	h_unoise_f(),h_unoise_B(),h_unoise_F();

#ifdef HUSESTDARG

int perr(int n, ...);

#else
#ifdef MATMEX

int perr();

#else

int perr(va_alist);

#endif
#endif

/*
 * avoid hassles of including string.h or strings.h
 */

#ifndef _STRING_H_
#ifndef _STRING_H
#ifndef __string_h
extern char *strcat(),*strncat(),*strcpy(),*strncpy(),*index(),*rindex();
extern char *strchr(),*strdup(),*strstr();
extern int strcmp(),strncmp();
#endif
#endif
#endif

/* omit strlen so that it defaults to int, but can also be size_t as in gcc */

/*
 * image and pyramid type declarations for the pyramid routines.
 *
 * The pyramid utilities are derived from code originally written by
 * Raj Hingorani at SRI/David Sarnoff Research Institute.  The original
 * Gaussian and Laplacian pyramid algorithms were designed by Peter Burt (also
 * currently at SRI/DSRC).  See:  Computer Graphics and Image Processing,
 * Volume 16, pp. 20-51, 1981, and IEEE Transactions on Communications,
 * Volume COM-31, pp. 532-540, 1983.
 */

#define MAXLEV 12

typedef struct {
   float **ptr;
   int nr;
   int nc;
} FIMAGE;

typedef struct {
   int **ptr;
   int nr;
   int nc;
} IIMAGE;

typedef FIMAGE FPYR[MAXLEV];
typedef IIMAGE IPYR[MAXLEV];

typedef struct {
   float *k;
   int taps2;		/* the number of taps from the center rightward,
				total number is 2*taps2-1 */
} FILTER;

/* function definitions */

float		**_read_fimgstr();
int		**_read_iimgstr();
float		**_alloc_fimage();
int		**_alloc_iimage();

/* externals */

extern int Image_border;

/* image macros */

#ifndef MAX
# define MAX(A,B)  ((A) > (B) ? (A) : (B))
#endif
#ifndef MIN
# define MIN(A,B)  ((A) < (B) ? (A) : (B))
#endif
#ifndef ABS
# define ABS(A)    ((A) > 0 ? (A) : (-(A)))
#endif
#ifndef BETWEEN
# define BETWEEN(A,B,C) (((A) < (B)) ? (B) : (((A) > (C)) ? (C) : (A)))
#endif
#ifndef SIGN
# define SIGN(A,B) (((B) > 0) ? (A) : (-(A)))
#endif
#ifndef TOascii
# define TOascii(c) ((c) & 0x7f)
#endif

/* compatibilities, type lists, etc. */

#define	LASTTYPE	-1	/* the last type in a type list */

#define	CM_ROWS		01	/* check compatibility: ROI rows */
#define	CM_COLS		02	/* check compatibility: ROI cols */
#define	CM_FRAMES	04	/* check compatibility: frames & depths */
#define	CM_FORMAT	010	/* check compatibility: pixel format */
#define	CM_NUMCOLOR	020	/* check compatibility: numcolor */
#define	CM_NUMLEV	040	/* check compatibility: pyramid levels */
#define	CM_OROWS	0100	/* check compatibility: total # rows */
#define	CM_OCOLS	0200	/* check compatibility: total # cols */
#define	CM_FRAMESC	0400	/* check compatibility: check frames & depths
					if numcolor != 1 or numdepth != 1 */
#define	CM_NUMCOLOR3	01000	/* check compatibility: numcolor (treat
					RGB, etc. as if numcolor=3) */
#define	CM_DEPTH	02000	/* check compatibility: numdepth */

/* converting to packed formats */

#define MSBF_PACKING	1	/* use most significant bit first packing */
#define LSBF_PACKING	2	/* use least significant bit first packing */

/* converting complex numbers to single-valued numbers */

#define CPLX_MAG	1	/* complex magnitude */
#define CPLX_PHASE	2	/* complex phase */
#define CPLX_REAL	3	/* complex - real part only */
#define CPLX_IMAG	4	/* complex - imaginary part only */

/* converting single-valued numbers to complex numbers */

#define CPLX_RVI0	1	/* real part = value, imaginary = 0 */
#define CPLX_R0IV	2	/* real part = 0, imaginary = value */
#define CPLX_RVIV	3	/* real part = value, imaginary = same value */

/*
 * type conversion methods
 *
 * Note: because find_method returns a method identifier, or METH_IDENT,
 * or the negative of a method identifier (for conversion via PFINT), it is
 * essential that none of these possible values be identical to HIPS_ERROR so
 * that it also can give a normal hips error return.
 */

#define METH_IDENT	2
#define METH_BYTE	3
#define METH_COMPLEX	4
#define METH_DOUBLE	5
#define METH_DBLCOM	6
#define METH_FLOAT	7
#define METH_INT	8
#define METH_LSBF	9
#define METH_MSBF	10
#define METH_SHORT	11
#define METH_SBYTE	12
#define METH_UINT	13
#define METH_USHORT	14
#define	METH_RGB	15
#define	METH_RGBZ	16
#define	METH_ZRGB	17
#define	METH_BGR	18
#define	METH_BGRZ	19
#define	METH_ZBGR	20

/* conversion-related variables */

extern int hips_packing;
extern byte hips_lchar;
extern byte hips_hchar;
extern int hips_cplxtor;
extern int hips_rtocplx;
extern int hips_convback;
extern int hips_lclip,hips_hclip;
extern int hips_zdiv;
extern h_boolean hips_oldhdr;

/* header handling */

extern int hips_fullhist;
extern int hips_fulldesc;
extern int hips_fullxpar;

struct h_types {	/* the type names structure defined in htypes.c */
	int h_pfmt;		/* pixel format */
	char *h_fmtname;	/* sprintf string for error code */
};

struct h_convs {	/* the conversion names structure defined in htype.c */
	int h_cnvtype;		/* pixel format */
	char *h_cnvname;	/* sprintf string for error code */
};

/* useful constants */

#define	H_E		2.7182818284590452354
#define	H_LOG2E		1.4426950408889634074
#define	H_LOG10E	0.43429448190325182765
#define	H_LN2		0.69314718055994530942
#define	H_LN10		2.30258509299404568402
#define	H_PI		3.14159265358979323846
#define	H_2PI		6.28318530717958647692
#define	H_PI_2		1.57079632679489661923
#define	H_PI_4		0.78539816339744830962
#define	H_ONE_PI	0.31830988618379067154
#define	H_TWO_PI	0.63661977236758134308
#define	H_TWO_SQRTPI	1.12837916709551257390
#define	H_180_PI	57.2957795130823208768	/* degrees to radians */
#define	H_SQRT2		1.41421356237309504880
#define	H_SQRT1_2	0.70710678118654752440
#define H_SQRT3OVER2	0.86602540378443864676
