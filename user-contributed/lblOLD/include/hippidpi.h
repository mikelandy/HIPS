/* HiPPI_DPI . DEF
#
%	Copyright (c)	Jin Guojun -	All rights reserved
%
% Convert non-standard HiPPI API to a unique HIPPI API
%
% Author:	Jin Guojun -	LBL	1/1/1994
*/

#ifndef	HiPPI_DPI_DEF
#define	HiPPI_DPI_DEF

#include <fcntl.h>

#define	HIPPI_MAGIC_D1	0xF8EAAE8FL	/* 32 --> 64 ?	*/
#define	NO_IORAM_HIPPI	/* IORAM is for MASPAR only	*/

typedef	struct	{
	long_32 magicN,
		d2_burst_pad;	/* non standard HiPPI D2 padding, such as:
				MASPAR pad its D2 to 1024 burst boundary */
	long_32	d1size,
		d2_pad;
} hippi_Tx_info_D1;

typedef	struct	{
	int	(*other_proto_hd)();
	void	*d1, *d2;
	char	*host;
	int	d1size_change;	/* returned or modified by other_proto_hd() */
	int	d2_size;	/* ip (udp/tcp) packet size	*/
	int	d2_start_pos,	/* at the end of all headers	*/
		d2_pad;		/* pad size if d2 is padded	*/
	short	sport, dport;	/* ports for transport layers	*/
	short	proto, resved;	/* tcp/udp	*/
} hippi_Embed_Header_t;


#if	defined	sparc	/* CHI	*/

#include "hipd.h"

#define	HIPPI_ULP_NONE	-1
#define	HIPPI_ULP_ANY	-2
#define	DEFHIPDEV	"/dev/hs0"
#define	hippi_open	open
#define	hippi_close(dev)	ioctl(dev, HIP_CLOSE, 0),	close(dev)
#define	hippi_make_one_packet(dev, total_size)

#elif	defined	sgi

#include <sys/hippi.h>

#undef	LITTLE_ENDIAN
#define	NO_HIPPI_IOVEC
#define	HIPPI_ULP_NONE	HIPPI_ULP_PH	/* ???	*/
#define	HIPPI_ULP_ANY	-2
#define	DEFHIPDEV	"/dev/hippi0"
#define	hippi_open	open
#define	hippi_close(dev)	ioctl(dev, HIPIOCW_DISCONN, 0),	close(dev)
#define	hippi_make_one_packet(dev, total_size)	\
	ioctl(dev, HIPIOCW_START_PKT, total_size)
#define	hippi_set_rx	hippi_set_tx
#define	hippi_set_rxtx	hippi_set_tx

#elif	defined	MASPAR || defined __maspar

# ifdef	ultrix	/* also __mips	*/

#include <maspar/hpraw.h>
#include <maspar/hippi.h>
#include <maspar/hpulp.h>
#include <maspar/hippi_fp.h>

# else	/* __osf__ & __alpha Assume Alpha as front end	*/

# ifdef	USE_HPRAW_H	/* EXCLUSIVE ?	*/
#  include <su/io/maspar/hpraw.h>
# endif
#include <su/io/maspar/hippi.h>
#include <su/io/maspar/hippi_fp.h>
#include <su/io/maspar/hippi_opc.h>

# endif

#define	HIPPI_ULP_NONE	-1
#define	HIPPI_ULP_ANY	-2
#define	DEFHIPDEV	"/dev/hippi0"
#define	hippi_make_one_packet(dev, total_size)

#ifndef	__STDMPL__
#define	visible	extern
/* Nullify meaning of "visible" for front end code */
#endif

#ifndef	NOT_USE_IORAM_HIPPI
#undef	NO_IORAM_HIPPI	/* IORAM is OK for HIPPI IO	*/
#endif

typedef struct	HiPPI_ent	{
	hippi_opc_pkt_t	opc_io[2];
	bool		async_io[2];	/* since current HiPPI status is a
					single channel based I/O mothed, so
					all control data are global	*/
	int	flags,			/* AUX to indicate buffer mode	*/
#	define	HiPPI_SCMEM	1		/* memory at frontend	*/
#	define	HiPPI_IORAM	(1 << 1)	/* static mpbuf mode	*/
#	define	HiPPI_MPBUF	(1 << 2)	/* dynamic mbuf mode	*/
#	define	HiPPI_OTHER	(1 << 3)
		state;
	cookie_t	f_io[2],	/* pmem buffer pointers	*/
			mbuf[2];	/* iobufs	*/
	} HiPPI_ent_t;

extern	HiPPI_ent_t **hippi_Ep;

#define	local_hep_setup(fd)	struct	HiPPI_ent*	h_entp = hippi_Ep[fd]
#define	local_hep_comfirm(n)	(hippi_Ep && hippi_Ep[n])
#define	async_hippi	h_entp->async_io
#define	h_opc_io	h_entp->opc_io

#define	h_opc_in	h_opc_io
#define	h_opc_out	h_opc_io + 1
#define	async_HiPPI_I	async_hippi[0]
#define	async_HiPPI_O	async_hippi[1]


extern	void *	change_iobuf_func();
extern	int	(*set_mp_iobuf_funcp)(),
	bypass_set_mp_ramiobuf(/*mp_aiobuf_t *, caddr_t, int, int*/),
	set_mp_iobuf(/*mp_aiobuf_t *io_p, caddr_t buf, int size, int vfd */),
	set_mp_ramiobuf(mp_aiobuf_t *io_p, caddr_t bname, int size, int mpfd);

visible	dpu_mpbuf_setup(long fd, struct mpbuf_s *mpbuf);
extern	int	mpbuf_alloc(struct mpbuf_s *, long),
		mpbuf_free(struct mpbuf_s *);
extern	void	mpl_set_ioname(void *ioram_name, long namelen);
extern	mp_aiobuf_t*	ioram_alloc();

visible	mpl_ioram_IO(struct mpbuf_s *mpbp, long size, long IO,
			char* pmem, long offset),
	pmem_IO(long fd, long size, long Nproc, char* pmem, long ptof);
visible	cookie_t	pmemalloc(long psize);
visible	void	pmemfree(char* pmem);
/*
*		Following four macros are not useful for protocol
*	because they must be called through callRequest().
*	They are used as compiling flags.
*/
#define	mpl_ioram_read(mpbp, sz, pmem)	mpl_ioram_IO(mpbp, sz, 0L, pmem, 0L)
#define	mpl_ioram_write(mpbp, sz, pmem)	mpl_ioram_IO(mpbp, sz, 1L, pmem, 0L)
#define	pmem_to_file(fd, sz, Np, IO)	pmem_IO(fd, sz, Np, IO, 1L)
#define	file_to_pmem(fd, sz, Np, IO)	pmem_IO(fd, sz, Np, IO, 0L)

#define	MPL_ioram_read(mpbp, sz, off, pmem)	\
	callRequest(mpl_ioram_IO, sizeof(long)*5, mpbp, (long)sz, 0L, pmem, off)
#define	MPL_ioram_write(mpbp, sz, off, pmem)	\
	callRequest(mpl_ioram_IO, sizeof(long)*5, mpbp, (long)sz, 1L, pmem, off)
#define	MPL_pmem_to_file(fd, sz, Np, pmem)	callRequest(pmem_IO, \
		sizeof(long)*5, (long)fd, (long)sz, (long)Np, pmem, 1L)
#define	MPL_file_to_pmem(fd, sz, Np, pmem)	callRequest(pmem_IO, \
		sizeof(long)*5, (long)fd, (long)sz, (long)Np, pmem, 0L)

#define	ASSIGN_iobuf_func(func)		set_mp_iobuf_funcp = (int(*)()) func
#define	default_iobuf_func(func)	set_mp_iobuf_funcp = set_mp_iobuf

#endif


typedef	struct	{
#ifdef	LITTLE_ENDIAN
	long_32	D2_offset : 3,
		D1_size : 8,
		reserved : 11,
		B : 1,	/* Burst Boundary for first short packet.	*/
		P : 1,	/* D1 Data is Presented	*/
		ULP_id : 8;
#else
	long_32	ULP_id : 8,
		P : 1,
		B : 1,
		reserved : 11,
		D1_size : 8,
		D2_offset : 3;
#endif
	long_32	D2_size;
	} FP_header_t;	/*	total size is two words	*/

typedef	struct	{
	long_32	L : 1,	/* Local or Standard Format	*/
		VU : 2,	/* Vendor Unique Bits	*/
		W : 1,	/* Width : 1 -> 64 -> 1.6 Gb	*/
		D : 1,	/* Direction, 1 MSB Routing Control (RC) field	*/
		PS : 2,	/* Path Selection	*/
		C : 1,	/* Camp-on	*/
		RC : 24;
	} I_Field_t;	/*	total size is one WORD	*/


/*	flags for FP	*/

#define	HIPPI_MODE_SHT_BURST	1
#define	HIPPI_MODE_CONNECT_P	2

#define	HIPPI_MODE_IFIELD	(1 << 8)	/* not really needed	*/
#define	HIPPI_MODE_D1SIZE	(1 << 9)


#endif	/*	HiPPI_DPI_DEF	*/

