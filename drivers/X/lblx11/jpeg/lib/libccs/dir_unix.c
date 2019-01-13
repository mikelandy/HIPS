/* DIR_Unix .C
#
%	Copyright (c)	Jin Guojun
%
%	for UNIX directory entry
%
% Author:	Jin Guojun - LBL	10/1/94
*/

#include <imagedef.h>
#include <udir.h>

#define	begindir(dirp, dep, attr)	(dep = readdir(dirp))


dirFList(F_List_t *flist, char *dir, int ftype)
{
    if (flist)	{	/* flist must be initialized !	*/
	register struct direct	*dep;	/* it's in sys/dir.c	*/
	int	nb = 0;
	char	*lname;
	afile_t	*afp;
	DIR*	dirp;
	if (! (dirp = opendir(dir)) )
		goto	badir;
	errno = 0;
	if (!begindir(dirp, dep, 0) && errno)	{
		closedir(dirp);
badir:		return	prgmerr(-1, "unreadable %s", dir);
	}
	for (afp = flist->finfo; dep; dep = readdir(dirp))	{
	    if (!dep->d_ino)	continue;	/* when does this happen ? */
	    if (gstat(afp, lname=cat(dir, dep->d_name), ftype, &nb)) {
		/* afp->ind = dep->d_ino;
		/* afp->name =	/* not need now	*/
		flist->fname[flist->nfiles++] = str_save(lname);
		if (flist->nfiles < flist->ents)
			afp++;
		else	{
		register int	i = (flist->ents += Dir_Size);
			flist->finfo = (afile_t *)
				realloc(flist->finfo, i * sizeof(afile_t));
			flist->fname = (char **)
				realloc(flist->fname, i * sizeof(cookie_t));
			if (!flist->finfo || !flist->fname)
				prgmerr('M', "dirFList out of memory");
			afp = flist->finfo + i - Dir_Size;
		}
	    }
	}
	return	flist->nfiles;
    }
}



infiletype(char *file, int statype, afile_t	*afp)
{
	if (afp->type == 'd' && !(statype & SFile_DIRS))
		return	False;
	if (afp->type == '-')	{	/* plain file	*/
	int	l = strlen(file) - 2;
		switch (afp->type = (statype & SFile_EXTFMT))	{
		case SFile_CSOURCES:
			l = strcmp(file + l, ".c");	break;
		case SFile_OBJECTS:
			l = strcmp(file + l, ".o");	break;
		case SFile_FSOURCES:
			l = strcmp(file + l - 2, ".for");	break;
		case SFile_TOUCHDONE:
			l = strcmp(file + l - 2, ".out");	break;
		case SFile_IMAGES:
			if (afp->itype = pull_itype(file))
				l = 0;
			break;
		default:l = ShowDIR_Only(statype);
		}
		return	!l;
	}
return	True;	/* !ShowDIR_Only(stt)	*/
}


char *
cat(char *dir, char *file)
{
static char	dfile[BUFSIZ >> 1];	/* 1/2 K Bytes	*/

	if (strlen(dir)+1+strlen(file)+1 > BUFSIZ)
		prgmerr(1, "filename too long\n");

	if (!strcmp(dir, "") || !strcmp(dir, ".")) {
		(void) strcpy(dfile, file);
		return	dfile;
	}
	(void) strcpy(dfile, dir);
	if (dir[strlen(dir) - 1] != '/' && *file != '/')
		(void) strcat(dfile, "/");
	(void) strcat(dfile, file);
return	dfile;
}


afile_t	*
gstat(register afile_t	*afp, char *file, int	statype,
	int	*pnb	/* (int *)0 if file is ISARG */	)
{
int	lstat(), (*statf)() = stat /* : lstat -- for symlink */;
struct stat	stb, stb1;

	afp->flags = afp->ind = 0;
	afp->type = '-';

	if ((*statf)(file, &stb) < 0) {
	    if (/* statf == lstat || */ lstat(file, &stb) < 0) {
		if (errno == ENOENT)
			message("%s not found\n", file);
		else
			prgmerr(0, file);
		return	0;
	    }
	}
	afp->blks = stb.st_blocks;
	afp->size = stb.st_size;
	switch (stb.st_mode & S_IFMT) {
	case S_IFDIR:
		afp->type = SFile_DIRS;	break;
	case S_IFBLK:
		afp->itype = 'b';	goto	devcom;
	case S_IFCHR:
		afp->itype = 'c';
devcom:		afp->type = SFile_DEVICES; afp->size = stb.st_rdev;	break;
	case S_IFSOCK:
		afp->itype = 's';	afp->size = 0;	break;
	case S_IFIFO:
		afp->itype = 'p';	afp->size = 0;	break;
	case S_IFLNK:
		afp->type = 'l';
		if (statype & SFile_RECURSIVE) {
		char	buf[BUFSIZ];
		int	cc = readlink(file, buf, BUFSIZ);
		    if (cc >= 0) {
			/*
			 * here we follow the symbolic
			 * link to generate the proper
			 * Fflg marker for the object,
			 * eg, /bin -> /pub/bin/
			 */
			buf[cc] = 0;
			if ((statype & SFile_FLink) && !stat(buf, &stb1))
				switch (stb1.st_mode & S_IFMT)	{
				case S_IFDIR:
					buf[cc++] = '/';
					break;
				case S_IFSOCK:
					buf[cc++] = '=';
					break;
				default:
				if ((stb1.st_mode & ~S_IFMT) & 0111)
					buf[cc++] = '*';
					break;
				}
				buf[cc] = 0;
				afp->linkto = str_save(buf);
		    }
		    break;
		}
		/*
		 *  this is a hack from UCB to avoid having
		 *  ls /bin behave differently from ls /bin/
		 *  when /bin is a symbolic link.  We hack the
		 *  hack to have that happen, but only for
		 *  explicit arguments, by inspecting pnb.
		 */
		if (pnb != (int *)0 || stat(file, &stb1) < 0)
			break;
		if ((stb1.st_mode & S_IFMT) == S_IFDIR) {
			stb = stb1;
			afp->type = 'd';
			afp->size = stb.st_size;
			afp->blks = stb.st_blocks;
		}
		break;
	}
	if (!infiletype(file, statype, afp))
		return	NULL;	/* ignore this file such as .c .o ...	*/

	afp->ind = stb.st_ino;
	afp->flags = stb.st_mode & ~S_IFMT;
	afp->nls = stb.st_nlink;
	afp->uid = stb.st_uid;
	afp->gid = stb.st_gid;
	if (statype & SFile_ATime)
		afp->time = stb.st_atime;
	else if (statype & SFile_CTime)
		afp->time = stb.st_ctime;
	else
		afp->time = stb.st_mtime;
	if (pnb)
		*pnb += stb.st_blocks;
return	afp;
}


pull_itype(char *fname)
{
register int	t;
FILE*	fp = fopen(fname, "rb");
	if (!fp)	return	0;
	switch (getc(fp))	{
	case 'H':	t = HIPS;	break;
	case 'S':	t = FITS;	break;
	case 'R':	t = RLE;	break;
	case 0x95:	/* Little Endian	*/
	case 'Y':	t = RAS;	break;
	case 'G':	t = GIF;	break;
	case 'I':	/* L.E.	*/
	case 'M':	t = TiFF;	break;
	case 'P':	t = PNM;	break;
	case 0xFF:	t = JPEG;	break;
	/* try only ICC, other 0s no piping, and must be after ICC	*/
	case 0	:	t = ICC;	break;
	default:	t = 0;
	}
	fclose(fp);
return	t;
}

void
init_dirList(F_List_t *fList, int dsize)
{
	fList->fname = ZALLOC(dsize, sizeof(cookie_t), "FLfn");
	fList->finfo = ZALLOC(dsize, sizeof(afile_t), "FLfi");
	fList->ents = dsize;
	fList->nfiles = 0;
}


#ifdef	DEMO_MAIN

char	*Progname;
arg_fmt_list_string	arg_list[] =	{
	{"-m", "%d", 0x0F, 1, 1, "filter:\n\
		1	dirs\n\
		2	files\n\
		4	images\n\
		16	C\n\
		32	FORTRAN\n\
		64	.o\n\
		128	.out\n\tDefault is %.f"},
	NULL,	};


main(int argc, char **argv)
{
F_List_t	FList;
char**	fl;
int	i, mode;

	if ((i=parse_argus(&fl, argc, argv, arg_list, &mode)) < 0)
		exit(i);
	init_dirList(&FList, Dir_Size);
	dirFList(&FList, i ? fl[i-1] : ".", mode);
	for (i=FList.nfiles; i--;)
		printf("%s\n", FList.fname[i]);
}

#endif
