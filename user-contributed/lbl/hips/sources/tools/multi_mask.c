/*	Copyright (c)	1990	Jin, Guojun
*
*	MULTI_MASK . C
*
*	Automatically pipe in & out.
*		-p will make no pipe out and send data to a file with
*		mask coded extension.
*/

#include "header.def"
#include "imagedef.h"
#include <math.h>

U_IMAGE	uimg;

#define	inbuf	uimg.src

typedef	union	{
	char	BYTE[4];
	short	INTEG[2];
	long	LINT;
	}BMap;

typedef	union	{
	char	Byte[2];
	short	Inte;
	}onion;

onion	sWap;
char	fname[32]={NULL}, PixBit='B', header=0, opt='&',
	npip, tmp, ct, buf[64], usage[]="options\n\
multi_mask [-mBit_Mask] [-h] [-BSFLIW] [-o^&o-x+] file_name [-p]\n";

BMap	Map;

BMap	*Mask(Lnt, mask)
register long	Lnt, mask;
{
BMap	TPBM;
/*fprintf(stderr,"Value = %x,	Mask = %x,	", Lnt, mask);*/
switch(opt)	{
	case '&':	TPBM.LINT = Lnt & mask;	break;
	case '^':	TPBM.LINT = Lnt ^ mask;	break;
	case '+':	TPBM.LINT = Lnt + mask;	break;
	case '-':	TPBM.LINT = Lnt - mask;	break;
	case 'x':	TPBM.LINT = Lnt * mask;	break;
	case 'o':	TPBM.LINT = Lnt | mask;	break;
	default:	perror("Wrong Operation");	exit('o');
    }
/*fprintf(stderr,"Result = %x	", TPBM.LINT);*/
return	&TPBM;
}

main(argc, argv)
int	argc;
char	*argv[];
{
short	nt;
long	lnt;
byte	I;
char	ch;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, -1, *argv, "S20-1");

Map.LINT = -1;
if (argc == 1)	goto	DFP;
message("BYTE = %d,	INTG = %d,	LONG = %ld\n",
	Map.BYTE[3], Map.INTEG[1], Map.LINT);

for (ct=1; ct<argc; ct++)	/* get all command line arguments	*/
{
   if (*argv[ct] == '-')
	{
	argv[ct]++;
	ch = *argv[ct];
	argv[ct]++;
	switch(ch)
	{
	case 'm':
		while(isdigit(*argv[ct]))
		{
		long	ltmp;
			if (*(argv[ct]++) == '1')
				ltmp = 1;
			else	ltmp = 0;
			Map.LINT <<= 1;
			Map.LINT |= ltmp;
		}
		break;
	case 'x':
		sscanf(argv[ct], "%lx", &Map.LINT);
		break;
	case 'B':
	case 'S':
	case 'I':
	case 'F':
	case 'W':
	case 'L':	PixBit = ch;	break;
	case 'h':	header = True;	break;
	case 'o':	opt = *argv[ct];break;
	case 'p':	npip = 1;	break;
	default:
DFP:		usage_n_options(usage, ct, argv[ct]);
	}/* end switch	*/
	}/* end if	*/
	else	strcpy(fname, argv[ct]);	/* get file name	*/
}/* end for	*/

io_test(stdin_fd, goto	DFP);

message("BYTE = %d, INTG = %d, LONG = %ld\n",
	Map.BYTE[3], Map.INTEG[1], Map.LINT);

if (strlen(fname)==0)
#ifdef	IBM_PC
{	fprintf(stderr,"Input Orginal File Name ");	gets(fname);	}
#else
;else
#endif
if ((in_fp=freopen(fname, "rb", stdin)) == NULL)
{
rerr:	perror("open reading file error");	exit('i');
}

if (npip)
{	sprintf(buf, ".%c%x", opt + 0x20, Map.LINT);	strcat(fname, buf);
	if ((out_fp=freopen(fname, "wb", stdout)) == NULL)
	{	perror("open writing file error");	exit('o');	}
}

io_test(fileno(in_fp), goto	DFP);

if (header) {	/*	pass hips header
	for (ch=0; ch < 100 && *buf != '.'; ch++)	{
		if (!fgets(buf, 80, in_fp))	goto	rerr;
		fputs(buf, out_fp);
	}	*/
	(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0);
	(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);
}

fprintf(stderr, "Header = %d,	PixBit = %c,	BitMap = %x, OPT = %c\n",
	header, PixBit, Map.LINT, opt);
switch(PixBit)
{	case 'B':
		while(True)
		{
		fread(&ch, sizeof(ch), 1, in_fp);
		if (feof(in_fp))	break;
		ch = Mask((long)ch, (long)Map.BYTE[3])->BYTE[3];
/*fprintf(stderr, "RET = %x\n", ch);*/
		fwrite(&ch, sizeof(ch), 1, out_fp);
		}
		break;
	case 'S':
		while(True)
		{
		fread(&nt, sizeof(nt), 1, in_fp);
		if (feof(in_fp))	break;
		nt = Mask((long)nt, (long)Map.INTEG[1])->INTEG[1];
/*fprintf(stderr, "RET = %x\n", nt);*/
		fwrite(&nt, sizeof(nt), 1, out_fp);
		}
		break;
	case 'L':
		while(True)
		{
		fread(&lnt, sizeof(lnt), 1, in_fp);
		if (feof(in_fp))	break;
		lnt = Mask(lnt, Map.LINT)->LINT;
		fwrite(&lnt, sizeof(lnt), 1, out_fp);
		}
		break;
	case 'I':
		I = 0;
		while(True)
		{
		fread(&tmp, sizeof(tmp), 1, in_fp);
		if (feof(in_fp))	break;
		I++;
		tmp = Mask((long)tmp, (long)I)->BYTE[3];
		fwrite(&tmp, sizeof(tmp), 1, out_fp);
		}
		break;
	case 'W':
		while(True)
		{
		fread(&sWap.Inte, sizeof(sWap.Inte), 1, in_fp);
		if (feof(in_fp))	break;
		I++;
		sWap.Byte[0] = Mask((long)sWap.Byte[0], (long)sWap.Byte[1])->BYTE[3];
		fwrite(&sWap.Inte, sizeof(sWap), 1, out_fp);
		}
}
fclose(in_fp);	fclose(out_fp);
}
