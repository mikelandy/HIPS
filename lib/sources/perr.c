/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

#include <hipl_format.h>
#include <stdio.h>

#ifdef HUSESTDARG
#include <stdarg.h>
#else
#include <varargs.h>
#endif

extern struct h_errstruct h_errors[];
char badfmt[] = "invalid error format code %d";

/* perr(errorcode,errorprintargs...) */

#ifdef HUSESTDARG

int perr(int n, ...)

{

#else

int perr(va_alist)

va_dcl

{
	int n;
#endif
	va_list ap;
	int i1,i2,i3,i4;
	char *s1,*s2,*s3,*s4;

#ifdef HUSESTDARG
	va_start(ap,n);
#else
	va_start(ap);
	n = va_arg(ap,int);
#endif
	hipserrno = n;
	sprintf(hipserr,"%s: ",Progname);
	if (n <=0 || n > MAXERR) {
		n = MAXERR+1;
		sprintf(hipserr+strlen(hipserr),h_errors[n-1].h_errstr,n);
	}
	else {
		switch(h_errors[n-1].h_errfmt) {
		case HEP_N:
			sprintf(hipserr+strlen(hipserr),"%s",h_errors[n-1].h_errstr);
			break;
		case HEP_D:
			i1 = va_arg(ap,int);
			sprintf(hipserr+strlen(hipserr),h_errors[n-1].h_errstr,
				i1);
			break;
		case HEP_S:
			s1 = va_arg(ap,char *);
			sprintf(hipserr+strlen(hipserr),h_errors[n-1].h_errstr,
				s1);
			break;
		case HEP_SD:
			s1 = va_arg(ap,char *);
			i1 = va_arg(ap,int);
			sprintf(hipserr+strlen(hipserr),h_errors[n-1].h_errstr,
				s1,i1);
			break;
		case HEP_DS:
			i1 = va_arg(ap,int);
			s1 = va_arg(ap,char *);
			sprintf(hipserr+strlen(hipserr),h_errors[n-1].h_errstr,
				i1,s1);
			break;
		case HEP_SS:
			s1 = va_arg(ap,char *);
			s2 = va_arg(ap,char *);
			sprintf(hipserr+strlen(hipserr),h_errors[n-1].h_errstr,
				s1,s2);
			break;
		case HEP_SDD:
			s1 = va_arg(ap,char *);
			i1 = va_arg(ap,int);
			i2 = va_arg(ap,int);
			sprintf(hipserr+strlen(hipserr),h_errors[n-1].h_errstr,
				s1,i1,i2);
			break;
		case HEP_SDS:
			s1 = va_arg(ap,char *);
			i1 = va_arg(ap,int);
			s2 = va_arg(ap,char *);
			sprintf(hipserr+strlen(hipserr),h_errors[n-1].h_errstr,
				s1,i1,s2);
			break;
		case HEP_SSS:
			s1 = va_arg(ap,char *);
			s2 = va_arg(ap,char *);
			s3 = va_arg(ap,char *);
			sprintf(hipserr+strlen(hipserr),h_errors[n-1].h_errstr,
				s1,s2,s3);
			break;
		case HEP_DDDD:
			i1 = va_arg(ap,int);
			i2 = va_arg(ap,int);
			i3 = va_arg(ap,int);
			i4 = va_arg(ap,int);
			sprintf(hipserr+strlen(hipserr),h_errors[n-1].h_errstr,
				i1,i2,i3,i4);
			break;
		case HEP_SDDDDS:
			s1 = va_arg(ap,char *);
			i1 = va_arg(ap,int);
			i2 = va_arg(ap,int);
			i3 = va_arg(ap,int);
			i4 = va_arg(ap,int);
			s2 = va_arg(ap,char *);
			sprintf(hipserr+strlen(hipserr),h_errors[n-1].h_errstr,
				s1,i1,i2,i3,i4,s2);
			break;
		case HEP_SSSS:
			s1 = va_arg(ap,char *);
			s2 = va_arg(ap,char *);
			s3 = va_arg(ap,char *);
			s4 = va_arg(ap,char *);
			sprintf(hipserr+strlen(hipserr),h_errors[n-1].h_errstr,
				s1,s2,s3,s4);
			break;
		default:
			sprintf(hipserr+strlen(hipserr),badfmt,
				h_errors[n-1].h_errfmt);
			break;
		}
	}
	va_end(ap);
	if (h_errors[n-1].h_errsev >= hipserrprt ||
	    h_errors[n-1].h_errsev >= hipserrlev)
		fprintf(stderr,"%s\n",hipserr);
	if (h_errors[n-1].h_errsev >= hipserrlev)
		exit(h_errors[n-1].h_errsev);
	return(HIPS_ERROR);
}
