/* U DIR . H
#
%	Copyright (c)	Jin Guojun
%
% Author:	Jin Guojun - LBL	1/1/94
*/

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef	SOLARIS2_3_or_4	/* too bad for mismatch d_off	*/

struct	direct	{
	ino_t	d_ino;	/* "inode number" of entry	*/
	u_short	d_off;	/* offset of disk dirent (BAD)	*/
	u_short	d_reclen;	/*length of this record */
	char	d_name[1];	/* name of file */
};

#else

#define	direct	dirent	/* no problem for SunOS 4.1.x	*/

#endif

typedef	struct	{
	byte	type,	/* SFile_FMT. _IMAGES -> image type is in itype	*/
		itype;	/* 'b', 'c', 'd', 'f', 'l', or image type	*/
	ino_t	ind;	/* inode number of the file	*/
	short	flags,	/* mode & ~S_IF???	*/
		nls;	/* number of links	*/
	uid_t	uid;
	gid_t	gid;
	long	size,
		blks;
	time_t	time;
	char	*name,
		*linkto;
} afile_t;

typedef	struct	{
	char	**fname;
	afile_t	*finfo;
	int	ents,	/* number of entries allocated	*/
		nfiles;
} F_List_t;


#define	SFile_DIRS	1
#define	SFile_DEVICES	(1 << 1)	/* 'b' or 'c'	*/
#define	SFile_IMAGES	(1 << 2)
#define	SFile_NoUse	(1 << 3)
#define	SFile_CSOURCES	(1 << 4)	/* C source .c	*/
#define	SFile_FSOURCES	(1 << 5)	/* FORTRAN source .for	*/
#define	SFile_OBJECTS	(1 << 6)	/* object files	.o	*/
#define	SFile_TOUCHDONE	(1 << 7)	/* all .out files	*/
#define	SFile_ATime	(1 << 8)
#define	SFile_CTime	(1 << 9)
#define	SFile_FLink	(1 << 10)
#define	SFile_RECURSIVE	(1 << 11)
#define	SFile_FMT	0x0FF
#define	SFile_EXTFMT	0x0F4

#define	ShowDIR_Only(ft)	(ft == SFile_DIRS)
#define	ShowFILE_Only(ft)	!(ft & SFile_DIRS)
#define	ShowDEVICE_Only(ft)	(ft == SFile_DEVICES)
#define	ShowIMAGE_Only(ft)	(ft == SFile_IMAGES)
#define	ShowCSOURCE_Only(ft)	(ft == SFile_CSOURCES)
#define	ShowFSOURCE_Only(ft)	(ft == SFile_FSOURCES)
#define	ShowOBJECT_Only(ft)	(ft == SFile_OBJECTS)
#define	ShowDotOUT_Only(ft)	(ft == SFile_TOUCHDONE)

#ifndef	Dir_Size
#define	Dir_Size	48
#endif

afile_t	*gstat();
char	*cat();

