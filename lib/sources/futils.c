#include <hipl_format.h>
#include <stdio.h>

FILE *ffopen(name,mode)

Filename name;
char *mode;

{
	FILE *fp = fopen(name, mode);

	if (fp == (FILE *) NULL)
		return((FILE *) perr(HE_OPEN,name));
	return(fp);
}

FILE *ffreopen(name,mode,stream1)

Filename name;
char *mode;
FILE *stream1;

{
	FILE *stream2;

	if ((stream2=freopen(name,mode,stream1)) == (FILE *) NULL)
		return((FILE *) perr(HE_OPEN,name));
	else
		return(stream2);
}

int ffread(ptr,size,nelem,stream)

char *ptr;
int size, nelem;
FILE *stream;

{
	int rsiz;

	if ((rsiz=fread(ptr,size,nelem,stream)) != nelem)
		return(perr(HE_REQ,"fread",nelem,rsiz));
	return(HIPS_OK);
}

int ffwrite(ptr,size,nelem,stream)

char *ptr;
int size,nelem;
FILE *stream;

{
	int rsiz;

	if ((rsiz=fwrite(ptr,size,nelem,stream)) != nelem)
		return(perr(HE_REQ,"fwrite",nelem,rsiz));
	return(HIPS_OK);
}

static int stdinopen = 0;

FILE *hfopenr(filename)

Filename filename;

{
	FILE *fp;

	if (strcmp(filename,"<stdin>") == 0) {
		if (stdinopen++)
			return((FILE *) perr(HE_STDIN));
		return(stdin);
	}
	fp = fopen(filename,"r");

	if (fp == (FILE *) NULL)
		return((FILE *) perr(HE_OPEN,filename));
	return(fp);
}
