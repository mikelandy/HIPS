/*     Copyright (c) 1992  Karsten Hartelius, IMSOR.

 Disclaimer:  No guarantees of performance accompany this software,
 nor is any responsibility assumed on the part of the authors.  All the
 software has been tested extensively and every effort has been made to
 insure its reliability.   

 asc2hips - transforms an ascii-file to a HIPS-file.

 language: c. Program works with HIPS2. 

 usage: see manual

 to load:
	cc -o asc2hips asc2hips.C -lm -lhips

 to run:
    asc2hips < ASCII-file > HIPS-file

 include-files: util.h

 Karsten Hartelius, IMSOR, 16/11 1992. 
*/

#include "util.h"
#define  MAXFORM 20

struct Form {
  int num, size, type;
  };

struct Form format[MAXFORM];

struct header hd;

int  	Nelem, 		/* number of variables including coordinates */
		Nrec,		/* number of data-records */ 
		Nrows,		/* number of rows in output */ 
		Ncols,		/* number of coloumns in output */
		Nformat,		/* number of format-types */ 
		xpos,		/* coloumn-number of x-coordinate */ 
		ypos,		/* coloumn-number of y-coordinate */ 
		out_format;   	/* format of output */ 

Boolean	defaultformat, /* input is alined in separate coloumns */ 
		defaultsplit,	/* end-of-record marker is space or newline */
		irregular,	/* output is a file of Irregular format */ 
		element_seq;	/* data listet as record-wise */ 

Fmatrix	data;		/* matrix containing data */

char		splitmark;	/* end-of-record marker */
char		nextchar();
			

int main(argc,argv)
int argc;
char* argv[];
{
	void		rparam(), read_default(), read_special(), shift_order(),
			printout();

	Progname = strsave(*argv);
	rparam(argc,argv);	
	data=fmatrix(Nelem,Nrec);
	if (defaultformat)
		read_default();
	else
		read_special();
	if (element_seq)
		shift_order();
	printout(argc,argv);	
	return(0);
}



void printout(argc,argv)
int argc;
char* argv[];
{
	int	i;
	struct header hd;

  	init_header(&hd,"IMSOR","",Nelem,"",Nrows,Ncols,out_format,1);
	if (irregular)
		setparam(&hd,"Irregular",PFBYTE,1,1);
	update_header(&hd,argc,argv);
	write_header(&hd);

	fwrite_from_fvec(stdout,data[xpos],Nrec,out_format);
	if (Nelem > 1) 
		fwrite_from_fvec(stdout,data[ypos],Nrec,out_format);
	for (i=0;i<Nelem;i++) 
		if ((i!=xpos) && (i!=ypos)){
			fwrite_from_fvec(stdout,data[i],Nrec,out_format);
			}

	return;
}


void rparam(argc,argv)
int argc;
char* argv[];
{
	int	i, j, l;

	splitmark = 	' ';
	defaultformat=	TRUE;
	defaultsplit=	TRUE;
	irregular=	TRUE;
	element_seq =	FALSE;
	Nelem =		0;
	Nrec =		0;
	Nrows = 		1;
	out_format = 	PFFLOAT;
	xpos =		0;
	ypos =		1;

	/* read parameters  */
	for (i=1;i<argc;i++){
		if ( (!strcmp(argv[i],"-f")) || (!strcmp(argv[i],"-e"))){
			Nelem=atoi(argv[++i]); 
			continue;
			}

		if ( (!strcmp(argv[i],"-n")) || (!strcmp(argv[i],"-nc"))){
			Nrec=atoi(argv[++i]); 
			continue;
			}

		if (!strcmp(argv[i],"-r")){
			Nrows=atoi(argv[++i]); 
			irregular=FALSE; 
			continue;
			}

		if (!strcmp(argv[i],"-x")){
			xpos=atoi(argv[++i]); 
			continue;
			}

		if (!strcmp(argv[i],"-y")){
			ypos=atoi(argv[++i]); 
			continue;
			}
		if (!strcmp(argv[i],"-a")){
			element_seq = TRUE; 
			continue;
			}

		if (!strcmp(argv[i],"-form")){
		    	defaultformat=FALSE;
			Nelem = 0;
			i++;
		    	for (j=0; ((i<argc) && (argv[i][0]!='-'));j++,i++){
		     	if ((format[j].num = atoi(argv[i++])) == 0)
					perr(HE_MSG,"option f: error in argument");
				if (!strcmp("f",argv[i])){
					format[j].type = 1;
					Nelem += format[j].num;
					format[j].size = atoi(argv[++i]);
        				}
				else 
					if (!strcmp("s",argv[i]))
						format[j].type=2; 
					else
			    			perr(HE_MSG,"error in format-specification");
		         	}
		       	Nformat=j; 
			i--;
			continue;
			}

		if (!strcmp(argv[i],"-s")){
			defaultsplit=FALSE;
			splitmark=argv[++i][0]; 
			continue;
			}

		if (!strcmp(argv[i],"-w8")){
			out_format=PFDOUBLE; 
			continue;
			}
		perr(HE_MSG,"wrong argument");	
    	}

	if (Nelem == 0) 
		perror("missing specification of record size");
	if (Nrec == 0) 
		perror("missing number of records");
	if ((xpos >= Nelem) || (ypos >= Nelem))
		perror("bad position of x- or y-coordinates");
	if (Nrec % Nrows != 0)
		perror("wrong number of rows");
	else
		Ncols=Nrec/Nrows;
	return;
}


void	read_default()
{
	int 	i, j;
	char	c;

	i=0;
	while ((i < Nrec) && (!feof(stdin))){
    	  	for (j=0;j<Nelem;j++)
		if (!scanf(" %f ", &data[j][i])) 
			perr(HE_MSG,"error during read");
		if ((!defaultsplit) && (i<Nrec-1)) 
			do c=nextchar();
			while ((!feof(stdin)) && (splitmark!=c));
		i++;
      	}
	if (Nrec != i)
		perr(HE_MSG,"bad number of records"); 
	return;
}  


void read_special()
{
	int	i, j, k, l, f;
	char	streng[10], c; 

	i=0;
	while ((i < Nrec) && (!feof(stdin))){
		j=0;
		for (f=0;f<Nformat;f++)
			switch (format[f].type){
			case 1: 	for (k=0;k<format[f].num;k++){
						for (l=0;l<format[f].size;l++)
							streng[l]=nextchar();
						streng[format[f].size]='\0';
						data[j++][i]=atof(streng);
						}
					break; 
			case 2: 	for (k=0;k<format[f].num;k++)
						c=nextchar();
					break; 
			}
		if (!defaultsplit && (i<Nrec-1))
			do c=nextchar();
			while ((!feof(stdin)) && (c!=splitmark));
		i++;
		}
	if (Nrec != i)
		perror("wrong number of records in input");  
	return;
}    


char nextchar()
{
	char	c;
	do {
		if ((c=getchar())==EOF)
		perr(HE_MSG,"end-of-file during read");
	} while (c=='\n');
	return c;	
}


void shift_order()
{
	int	i, j, pos=0;	
	/* shift frames and coloumns, corresponding to the case where data are
	   listet in element-sequence instead of record-sequence. */ 
	Fmatrix data2;
	data2=fmatrix(Nrec,Nelem);
	for (i=0;i<Nelem;i++) for (j=0;j<Nrec;j++)
		data2[j][i] = data[i][j];

	for (i=0;i<Nelem;i++)
		for (j=0;j<Nrec;j++){
			data[i][j]=data2[pos/Nelem][pos%Nelem];
			pos++;
			}
	return;
}
