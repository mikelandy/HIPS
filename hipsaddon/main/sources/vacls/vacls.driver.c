static char *SccsId = "%W%      %G%";

/*----------------------------------------------------------
 * Control module for VACLS.
 * Barry Shepherd, 1985.
 * ----------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "vacls.def.h"
#include "vacls.ext.h"
#include "vacls.cmds.h"
#include "bineye.h"
#include "satts.h"

#define HELPTEXT	"vacls.cmds.txt"

#define true		1
#define false		0
#define	MAXRULES	150
#define MAXSECSETS	100
#define OPENCRS		true	/* for bineye additions only */

extern	int depth,leafclass[],usedatts[] ;
extern  float avpathlen,avuniqlen ;
extern	int cutoff,auto_term ;
extern	float	chilimit,inflimit ;

int	totdiffatts,numleaves ;

int	numrules,nodecnt[MAXRULES] ;
struct	node	*proot[MAXRULES],*parserule(),*readstring() ;
char attfnam[80];
char cat_help[MAXLIN];
char cat_help2[MAXLIN];
char *strsave();
extern char *Progname;
FILE *filopn();
int readattr(),readline(),loadexamp(),ruleclass(),getlimits(),buildrule();
int checksec(),readexamp();
void readclass(),setrule(),err(),vacls_command(),changeorder(),printatt();
void changegs(),del_examp(),del_class(),eg_expand(),setdebug(),cprinttree();
void induce(),storerule(),printnodes(),move(),printexamp(),printtree();
void user(),printconts(),committee(),exprompt();

/* ------------------------------------------------------------------------ */
/* --------------------------------------------------------------------------*/
#ifdef BINEYE
#else
int main(argc,argv)

int argc;
char *argv[];

{
	char line[MAXLIN],*s;
	FILE *strm;
	int i;
	int ruleOK=false,attsOK=false,classify=false ;

	Progname = strsave(*argv);
	sprintf(cat_help, "more %s/%s", HELPDIR, HELPTEXT);

	while (--argc > 0 && (*++argv)[0] == '-')
		for (s=argv[0]+1;*s!='\0';s++)
			switch (*s) {
				case 'a':
					s++ ;
					sscanf(s,"%s",line);
					if ((strm=fopen(line,"r"))==NULL) {
						printf("can't open %s\n",line);
						exit(-1);
					}
 					if (readattr(strm) <= 0) {
						printf("\nerror in %s\n",line);
						exit(-1) ;
					}
					attsOK = true ;
					readclass(strm);
					fclose(strm) ;
					while ( *s != '\0') s++ ;
					s-- ;
					break ;
				case 'c':
					classify = true;
					break ;
				case 'r':
					classify = true;
					s+=1;
					if (!attsOK) {
						printf("the -a argument must preceed a -r\n");
						exit(-1) ;
					}
					sscanf(s,"%s",line);
					if ((strm=fopen(line,"r"))==NULL) {
						printf("can't open %s\n",line);
						exit(-1);
					}
					while ((proot[numrules]=parserule(strm,&nodecnt[numrules]))!=NULL) {
						numrules++ ;
					}
					ruleOK = true ;
					fclose(strm);
					while ( *s != '\0') s++ ;
					s-- ;
					break ;
			}

	/*---------------------------------------------------------------  
	 * This bit allows the program to be used in a hips command line
	 * rather than in an interactive mode. 
	 * In a hips command line it is used only to classify an 
	 * attribute vector (supplied on stdin) being output from binatts.

	 * The vector can also be in a file (its name is the last argument)
	 * hence this facility can also be used to classify attribute vectors
	 * in any problem domain (ie not just hips generated ones).
	 * If the attribute vector is input via a file then the file can contain
	 * any number of such vectors. This facility is thus identical
	 * to the 'xs' command when useing vacls in an interactive mode. */

	if (classify) {
		if (!ruleOK) {
			printf("no rule with which to classify the data!!!\n");
			printf("supply rule filename using the -r flag\n");
			exit(-1) ;
		}
		if (!attsOK) {
			printf("no attribute-vector file!, use the -a flag\n");
			exit(-1) ;
		}
		if (argc == 1) {
			if ((strm=fopen(argv[0],"r")) == NULL)
			{	printf("can't open %s\n",argv[0]) ;
				exit(-1) ;
			}
		}
		else
			strm = stdin ;

		/* put the example (or examples) into prim store */
		/* and classify all egs in the primary store */
		if ( strm == stdin) { 
			readline(strm,line);
			loadexamp(line,PRIM,ALLATTS) ;
		}
		else
			while (readline(strm,line))
				loadexamp(line,PRIM,ALLATTS) ;
			
		/*if more than one rule was read then they must be
		 * single concept rules!! */
		if (numrules == 1)
			setrule(0) ;

		for (i=1;i<=numprim;i++)
			printf("example classified as: %s\n",
				classname(ruleclass(exlist[i],root))) ;
		exit(0) ;
	}

	/* ------------------------------------------------------------------*/
	/* now start the normal interactive ACLS */

 	setbuf (stdout, (char *)NULL);	/* force no buffering	*/

	if (!attsOK) {
		while ((strm=filopn("Enter attribute file name:",attfnam,"r"))==NULL);
		while (1) {
			strm=filopn("Enter attribute file name:",attfnam,"r");
			if (strm != NULL)
				break;
		}
 		if (readattr(strm) <= 0)
			err("main: error return from readattr");
		readclass (strm);
		fclose(strm);
		printf("************************************************\n");
		printf("*   Type 'h' for summary of available commands *\n");
		printf("************************************************\n");
		vacls_command(stdin,stdout) ;
	}
	return(0);
}
#endif

#ifdef BINEYE
void vacls_command(wp,ifp,ofp)
struct wind *wp ;
FILE *ifp,*ofp ;
{
	struct blobstruct *bp, *make_blob() ;
	int x,y;
	struct ruleptr	*rp,*new_rule() ;
#else
void vacls_command(ifp,ofp)
FILE *ifp,*ofp ;
{
#endif
	char line[MAXLIN],conceptclass[11];
	char fileout[80],filname[80];
	int i,j,cmd,swopflag,leaflabel=false,labeltype=1 ;
	int st,fin,srches ;
	int sizesecset[MAXSECSETS],numsecsets,temp,temp2 ;
	FILE *strm ;

	outstream = ofp;
	strcpy(fileout,"stdout");	

#ifdef BINEYE
		/* otherwise training can be tedious */
		att[1].dontuse = true ;
		att[2].dontuse = true ;
		att[3].dontuse = true ;
		att[4].dontuse = true ;
		att[5].dontuse = true ;
		att[6].dontuse = true ;
		att[8].dontuse = true ;
		att[10].dontuse = true ;
		att[12].dontuse = true ;
		att[13].dontuse = true ;
		att[14].dontuse = true ;
		att[15].dontuse = true ;
		att[16].dontuse = true ;
		att[17].dontuse = true ;
		att[18].dontuse = true ;
		att[20].dontuse = true ;
		att[21].dontuse = true ;
#endif

	for(;;)
	{
		fprintf(ofp,"\n\nvacls command: ");
		fscanf(ifp,"%s",line);
		if (line[0] == '!') {
			i=1 ;	
		    	while(line[i++]!= '\0');
		    	line[--i] = ' ' ; 
		    	readline(stdin,&line[++i]);
		    	system(line+1);
			continue ;
		}
		cmd = hsh(line[0],line[1]) ;
		switch (cmd)
		{
		case ATT_SUPRESS:
			getlimits(&st,&fin,numattr,"attr") ;
			for (i=st;i<=fin;i++)
				att[i].dontuse = true ;
			break ;
		case ATT_RELEASE:
			getlimits(&st,&fin,numattr,"attr") ;
			for (i=st;i<=fin;i++)
				att[i].dontuse = false ;
			break ;
		case ATT_ORDER_SWOP:	
			fprintf(ofp,"attr? order?:");
			fscanf(ifp,"%d %d",&i,&st) ;
			changeorder(i,st) ;
			break ;
		case ATT_PRINT:
			printatt(outstream,line[2]);
			break ;
		case BUILD_RULE:
			auto_term=false;
			buildrule() ;
			break ;
		case BOOLEAN_EGS:
	 	    	fprintf(ofp,"which class?\n");
			fscanf(ifp,"%s",conceptclass);
			swopflag=0;
			for (i=1;i<=numclass;i++)
				if (!strcmp(classname(i),conceptclass))
				{	changegs(i);
					swopflag=1;
					break;
				}
			if (!swopflag)
			{	changegs(0);
				strcpy(conceptclass,"multiclass");
			}
		    	break ;
		case SET_CUTOFF:
		    	fprintf(ofp,"cut-off value?\n") ;
			fscanf(ifp,"%d",&cutoff) ;
			fprintf(ofp,"chi-limit?\n") ;
			fscanf(ifp,"%f",&chilimit) ;
			fprintf(ofp,"inf-limt\n") ;
			fscanf(ifp,"%f",&inflimit) ;
		    	break ;
		case CHECK_EGS:
			setrule(0) ;
		        fprintf(ofp,"%d contradictions in sec store!\n",checksec(&srches)) ;
			break ;
		case DELETE_PRIM:
			del_examp(PRIM);
			break;
		case DELETE_SEC:
			del_examp(SEC);
			break;
		case DELETE_EGS:
		    	del_examp(NULL);
			break;
		case DELETE_CLASS:
			fprintf(ofp,"what class?\n") ;
			fscanf(ifp,"%d",&temp) ;
			del_class(temp) ;
			break ;
		case EXPAND_EGS:
		    	eg_expand();
		    	break;
		case SET_DEBUGFLAGS:
		    	setdebug();
		    	break;
		case GENERATE_CODE:
		    	fprintf(outstream,"%srule(egpntr)\n",conceptclass);
		    	fprintf(outstream,"int *egpntr;\n");
		    	fprintf(outstream,"{\n");
		    	cprinttree(root,0,0,1,1,outstream);
		    	fprintf(outstream,"	return(class);\n}\n\n");
		    	break;
		case HELP:
		    	system(cat_help);
		    	break;
		case INDUCE_AUTOTERM:
			auto_term=true;
			numnodes=0 ;
			curlev=0;
		    	induce();
			storerule(numrules++) ;
		    	break;
		case INDUCE:
			auto_term=false;
			numnodes=0 ;
			curlev=0;
		    	induce();
			storerule(numrules++) ;
		    	break;
		case CONCEPT:
			for (i=classtart;i<=numclass;i++)
			{	changegs(i) ; 	/* create bool prim egs */
				numnodes=0 ;
				curlev=0 ;
				induce() ;
				storerule(numrules++) ;
				changegs(0) ;	/* back again */
			}
			fprintf(ofp,"rules %d to %d created\n",
				numrules-(numclass-classtart+1),numrules-1);
			break ;

#ifdef RINDUCE 
		case RANGE_INDUCE:
			numnodes=0 ;
			curlev=0 ;
			rinduce();
			storerule(numrules++) ;
			break ;
#endif

		case PRINT_NODES:		/* debug */
		    	printnodes(outstream);
		    	break;
		case MOVE_EGS:
		    	move(0);
		    	break;
		case PRINT_EGS:
		    	printexamp(outstream,false);
		    	break;
		case QUIT:
#ifdef BINEYE
			return(1);
#else
		    	exit(1);
#endif
			break ;
		case READ_PRIM:
			readexamp(PRIM) ;
			break;
		case READ_RULE:
			strm=(FILE *)filopn("Rule filename?: ",filname,"r") ;
			if (strm == NULL)
				break ;

			/*-------------------*/
			/* init tree stat varaiables  used in parserule */
			depth = 0 ;
			avpathlen = 0 ;
			avuniqlen = 0 ;
			for (i=1;i<=numclass;i++)
				leafclass[i] = 0 ;
			for (i=1;i<=numattr;i++)
				usedatts[i] = 0 ;
			/*-------------------------*/

			while ((proot[numrules] = parserule(strm,&nodecnt[numrules]))!= NULL)
			{	fprintf(ofp,"\nrule.num.%d = %d nodes\n",numrules,nodecnt[numrules]) ;
				numrules +=1 ;

				depth = 0 ;
				numleaves = 0 ;
				totdiffatts = 0 ;
				fprintf(ofp,"leavesperclass ") ;
				for (i=1;i<=numclass;i++)
				{	fprintf(ofp,"%d ",leafclass[i]) ;
					numleaves += leafclass[i] ;
					leafclass[i] = 0 ;
				}
				fprintf(ofp,"\ntotplen= %f totulen= %f numleaves= %d\n",avpathlen,avuniqlen,numleaves) ;
				fprintf(ofp,"avplen= %f avulen= %f\n",avpathlen/numleaves,avuniqlen/numleaves) ;
				fprintf(ofp,"atnods= ");
				for (i=1;i<=numattr;i++)
				{	if (usedatts[i])
						totdiffatts += 1 ;
					fprintf(ofp,"%d ",usedatts[i]); 
					usedatts[i] = 0 ;
				}
				fprintf(ofp,"\ntotdiffatts= %d\nendrule\n\n",totdiffatts) ;
				avpathlen = 0 ;
				avuniqlen = 0 ;
			}
			fclose(strm) ;
			break ;
		case READ_COVERS:
			strm=(FILE *)filopn("L.String filename?: ",filname,"r");
			if (strm == NULL)
				break ;
			while ((proot[numrules]=readstring(strm,&nodecnt[numrules]))!=NULL) {
				fprintf(ofp,"string num.%d = %d nodes\n",numrules,nodecnt[numrules]) ;
				numrules += 1 ;
			}
			fclose(strm) ;
			break ;
		case READ_SEC:
			temp = numsec ;
			readexamp(SEC) ;
			sizesecset[numsecsets++] = numsec-temp ;
			break;
		case READ_TEL_WHERE:
			fprintf(ofp,"dups in sec?\n");
			fscanf(ifp,"%s",line);
			fprintf(ofp,"clashes in prim?\n");
			fscanf(ifp,"%s",&line[1]);
			line[2] = '\0';
			if (!strcmp(line,"yy"))
				readexamp(DCCHNG);
			else
			{	if (line[0]=='y')
					readexamp(DSEC);
				else
					readexamp(CPRIM);
			}
			break;
		case READ_USING_RULE:
			getlimits(&st,&fin,numrules-1,"rule");
			setrule(st) ;	
			readexamp(SAME) ;
			break;
		case PRINT_COVERS:
		case PRINT_RULES:
			getlimits(&st,&fin,numrules-1,"rule");
			for (i=st;i<=fin;i++)
			{	setrule(i) ;	
				fprintf(ofp,"\nrule%d, nodes=%d\n",i,numnodes) ;
				fprintf(ofp,"********************\n") ;
#ifdef STRINGIO
				if (cmd==PRINT_COVERS)
					makestrings(root,outstream) ;
				else
#endif
					printtree(root,0,0,0,0,outstream,false);
			}
			break ;
		case RELABEL_RULE:
			sscanf(&line[2],"%d",&labeltype);	/*1 or 2*/	
			getlimits(&st,&fin,numrules-1,"rule");
			for (i=st;i<=fin;i++)
			{	setrule(i) ;	
				fprintf(ofp,"\nrule%d, nodes=%d\n",i,numnodes) ;
				fprintf(ofp,"********************\n") ;
				printtree(root,0,0,0,0,outstream,labeltype);
			}
			break ;
			
		case SWITCH_OUTPUT:
		    	fprintf(ofp,"output file [%s]: ",fileout);
		    	fscanf(ifp,"%s",fileout);
		    	if (outstream != stdout)  fclose(outstream);
		    	if (strcmp(fileout,"stdout") == 0)
				outstream = stdout;
		    	else
				outstream = fopen(fileout,"a");
		    	break;
		case USER_EG_INPUT:
		    	user(outstream);
		    	break;

		case LEAFCONF_MATRIX:
			leaflabel = true ;
		case CONF_MATRIX:
			if ((outstream =fopen("temprex","w"))==NULL) 
			{	fprintf(ofp,"can't open temprex\n") ;
				break ;
			}
		case LEAFIFY_SEC:
			if (cmd == LEAFIFY_SEC)
				leaflabel = true ;	/*fallthro from above!*/
		case CLASSIFY_SEC:
			sprintf(filname,"cmat %s %s temprex",&line[2],attfnam);
			if ((i=getlimits(&st,&fin,numrules-1,"rule"))==1) 
			{	if (st == -1)
				{	for (i=0;i<numrules;i++)
					{	setrule(i) ;
						fprintf(ofp,"rule %d..\n",i);
						printconts(outstream,leaflabel);
					}
				}
				else
				{	setrule(st) ;
					printconts(outstream,leaflabel) ;
				}
			}
			else
			{	if (i==2)
					committee(st,fin,outstream,leaflabel) ;
				else
				{	fprintf(ofp,"\nags req:start <fin> r.no\n") ;
					break ;
				}
			}
			if ((cmd== CONF_MATRIX) || (cmd == LEAFCONF_MATRIX))
			{	fclose(outstream) ;
				outstream = stdout ;
				system(filname) ;
			}
			leaflabel = false ;
			break ;

		case SUMMARISE_SEC:
			temp = numprim ;
			temp2= numsec  ;
			for (j=0;j<numsecsets;j++)
			{	numsec  = sizesecset[j] ;
				fprintf(outstream,"secset%d\n",j) ;
				for (i=0;i<numrules;i++)
				{	setrule(i) ;
					st = checksec(&srches) ;
fprintf(outstream,"rule=%d,incor=%d,corr=%d,succ=%f%%,srch=%f%%\n",i,st,numsec-st,(((float)numsec-st)*100)/numsec,((float)srches*100)/numsec) ;
				}
				numprim += sizesecset[j] ;
			}
			numprim = temp ;
			numsec  = temp2;
			break ;
		/* binatts commands only */
#ifdef BINEYE
		case LIVECAMERA:
			screen_refresh(wp->camnum);
			draw_box(wp->cxd,wp->cyd,wp->cxo,wp->cyo,255);
			freeze();
			break ;

		/* grabpic, threshold and comp.atts */
		case TRAIN:
			temp = false ;		/* use as loop flag */
			if (line[2] == 'l') {
				fprintf(ofp,"class name (for all objects): ");
				fscanf(ifp,"%s",fileout);
				if (classtoken(fileout,&temp)==ERROR) {
					savestr(fileout); /* new class added */
					numclass++;
				}
				temp = true ;
			}
			capture_window(wp,OPENCRS);
			while (find_blob(wp->pic,wp->pxd,wp->pyd,&x,&y)) {
				bp = make_blob(wp->pic,wp->pxd,wp->pyd,
								&wp->bloblist);
				bp->xst = x ;
				bp->yst = y ;
				line[0] = '\0' ;
				for (i=1;i<=numattr;i++) {
				    if (!att[i].dontuse) {
				    	sprintf(filname,"%d ",
					    getattval(getstrg(att[i].name),bp));
				    	strcat(line,filname);
				    }	
				    else
				       strcat(line,"- ");
				}
				if (!temp) {
					fprintf(ofp,"\nclass value?: ");
					fscanf(ifp,"%s",fileout) ;
					if (classtoken(fileout,&temp)==ERROR) {
						savestr(fileout); /* new class*/
						numclass++;
					}
				}
				strcat(line,fileout);
				loadexamp(line,PRIM,ALLATTS) ;
				fprintf(ofp,"%s\nexample added to primary store\n",line);
				delete_blob(bp->pic,bp->xd,bp->yd,
							bp->xst,bp->yst,false);
				free_blobs(&wp->bloblist);
			}
			break;
		/* export rules to bineye, ie enter in rule list (with name)*/
		case EXPORT:
			fprintf(ofp,"rule number?: ");
			fscanf(ifp,"%d",&st);
			fprintf(ofp,"what is this rule to be called?: ");
			fscanf(ifp,"%s",filname);
			rp = new_rule(filname);
			rp->rule = proot[st] ;
			rp->num_nodes = nodecnt[st] ; 
			break;
#endif
		default:
		    	fprintf(ofp,"<%s> not recognized\n",line);
		    	break;
		}
	}
} 

/* ----------------------------------------------------------------- */
/*  set the debug flags which control printout and interaction levels */
void setdebug()
{
	char *bugflag();
	int max,flag;
	max = 6;
	do {
	    printf(" 0      return to menu\n");
	    printf(" 1 %s (print tree as it is generated)\n",bugflag(1) );
	    printf(" 2 %s print entropy values\n",bugflag(2) );
	    printf(" 3 %s hand select split attributes\n",bugflag(3) );
	    printf(" 4 %s print examples for each level\n",bugflag(4) );
	    printf(" 5 %s print population arrays\n",bugflag(5) );
	    printf(" 6 %s print node nums and nexamp with tree\n",bugflag(6));
	    printf("\n  choice: ");
	    scanf("%d",&flag);
	    if (flag > 0 && flag <= max) {
		if (debug[flag] == NO )  debug[flag] = YES;
		else			 debug[flag] = NO;
	    }
	}
	while ( flag != 0);
	return;
}
/* ----------------------------------------------------------------- */
/*  get an example from the user					*/

void user(strm)
FILE *strm ;
{
	char line[MAXLIN],strg[40];
	int i ;

	exprompt(strm);
	line[0] = '\0' ;
	for (i=0;i<=numattr;i++) {	/* +1 to accept classname as well */
		scanf("%s",strg);
		strcat(line,strg);
		strcat(line," ");
	}
	if (loadexamp(line,PRIM,ALLATTS)==ERROR)
		printf("that wasn't a very good example... try again\n");
}

/* --------------------------------------------------------------------- */

/* bugflag  pass back a string to print */
char *bugflag(index)
int index;
{
	if (debug[index] == YES) strcpy(bugtext,"ON -");
	else			 strcpy(bugtext,"    ");
	return(bugtext);
}
/* ----------------------------------------------------------------- */

void setrule(ruleno)
int ruleno ;
{	
	numnodes= nodecnt[ruleno] ;
	root= proot[ruleno]  ;
}

void storerule(ruleno)
int ruleno ;
{
	proot[ruleno]= root;
	nodecnt[ruleno]=numnodes ;
}

int getlimits(bot,top,max,prompt)
int *bot,*top,max ;
char *prompt ;
{	int i ;
	char line[40] ;
	readline(stdin,line) ;	/* getrid of line feed */
	printf("%s: beginning & end of range?\n",prompt) ;
	readline(stdin,line) ;
	if ((i=sscanf(line,"%d %d",bot,top))==1)
		*top= *bot;
	else if (!i)
	{	*bot=0 ;
		*top = max ;
		i= 2 ;
	}
	if (*top > max )
	{	printf("invalid range!\n") ;
		i = 0 ;
	}
	if (*top == *bot)
		i = 1 ;
	return(i) ;
}
