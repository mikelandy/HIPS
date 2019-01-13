/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * add_header.c - append text to the sequence history
 *
 * Michael Landy - 1/3/91
 */

#include <hipl_format.h>

/* history_set(header,argformat,fmtstring,sprintfargs...) */

#ifdef HUSESTDARG

#include <stdarg.h>

int history_set(struct header *hd, ...)

{
#else

#include <varargs.h>

int history_set(va_alist)

va_dcl

{
	struct header *hd;
#endif

	va_list ap;
	int fmt,i1,i2,i3,i4,len;
	h_boolean nflag;
	char *s,*s1,*s2,*s3,*s4,buf[LINELENGTH];

#ifdef HUSESTDARG
	va_start(ap,hd);
#else
	va_start(ap);
	hd = va_arg(ap,struct header *);
#endif
	fmt = va_arg(ap,int);
	s = va_arg(ap,char *);
	switch(fmt) {
	case HEP_N:
		sprintf(buf,"%s",s);
		break;
	case HEP_D:
		i1 = va_arg(ap,int);
		sprintf(buf,s,i1);
		break;
	case HEP_S:
		s1 = va_arg(ap,char *);
		sprintf(buf,s,s1);
		break;
	case HEP_SD:
		s1 = va_arg(ap,char *);
		i1 = va_arg(ap,int);
		sprintf(buf,s,s1,i1);
		break;
	case HEP_DS:
		i1 = va_arg(ap,int);
		s1 = va_arg(ap,char *);
		sprintf(buf,s,i1,s1);
		break;
	case HEP_SS:
		s1 = va_arg(ap,char *);
		s2 = va_arg(ap,char *);
		sprintf(buf,s,s1,s2);
		break;
	case HEP_SDD:
		s1 = va_arg(ap,char *);
		i1 = va_arg(ap,int);
		i2 = va_arg(ap,int);
		sprintf(buf,s,s1,i1,i2);
		break;
	case HEP_SDS:
		s1 = va_arg(ap,char *);
		i1 = va_arg(ap,int);
		s2 = va_arg(ap,char *);
		sprintf(buf,s,s1,i1,s2);
		break;
	case HEP_SSS:
		s1 = va_arg(ap,char *);
		s2 = va_arg(ap,char *);
		s3 = va_arg(ap,char *);
		sprintf(buf,s,s1,s2,s3);
		break;
	case HEP_DDDD:
		i1 = va_arg(ap,int);
		i2 = va_arg(ap,int);
		i3 = va_arg(ap,int);
		i4 = va_arg(ap,int);
		sprintf(buf,s,i1,i2,i3,i4);
		break;
	case HEP_SDDDDS:
		s1 = va_arg(ap,char *);
		i1 = va_arg(ap,int);
		i2 = va_arg(ap,int);
		i3 = va_arg(ap,int);
		i4 = va_arg(ap,int);
		s2 = va_arg(ap,char *);
		sprintf(buf,s,s1,i1,i2,i3,i4,s2);
		break;
	case HEP_SSSS:
		s1 = va_arg(ap,char *);
		s2 = va_arg(ap,char *);
		s3 = va_arg(ap,char *);
		s4 = va_arg(ap,char *);
		sprintf(buf,s,s1,s2,s3,s4);
		break;
	default:
		va_end(ap);
		return(perr(HE_BADFMT,fmt));
	}
	va_end(ap);
	len = strlen(buf);
	if (buf[len-1] == '\n')
		nflag = FALSE;
	else {
		nflag = TRUE;
		len++;
	}
	if ((hd->seq_history = (char *) hmalloc(len+1)) == (char *) HIPS_ERROR)
		return(HIPS_ERROR);
	strcpy(hd->seq_history,buf);
	if (nflag) {
		hd->seq_history[len-1] = '\n';
		hd->seq_history[len] = '\0';
	}
	hd->sizehist = len;
	hd->histdealloc = TRUE;
	return(HIPS_OK);
}

/* history_append(header,argformat,fmtstring,sprintfargs...) */

#ifdef HUSESTDARG

int history_append(struct header *hd, ...)

{

#else

int history_append(va_alist)

va_dcl

{
	struct header *hd;

#endif
	va_list ap;
	int fmt,i1,i2,i3,i4,len,lenb;
	h_boolean nflag;
	char *s,*s1,*s2,*s3,*s4,buf[LINELENGTH],*newh;

#ifdef HUSESTDARG
	va_start(ap,hd);
#else
	va_start(ap);
	hd = va_arg(ap,struct header *);
#endif
	fmt = va_arg(ap,int);
	s = va_arg(ap,char *);
	switch(fmt) {
	case HEP_N:
		sprintf(buf,"%s",s);
		break;
	case HEP_D:
		i1 = va_arg(ap,int);
		sprintf(buf,s,i1);
		break;
	case HEP_S:
		s1 = va_arg(ap,char *);
		sprintf(buf,s,s1);
		break;
	case HEP_SD:
		s1 = va_arg(ap,char *);
		i1 = va_arg(ap,int);
		sprintf(buf,s,s1,i1);
		break;
	case HEP_DS:
		i1 = va_arg(ap,int);
		s1 = va_arg(ap,char *);
		sprintf(buf,s,i1,s1);
		break;
	case HEP_SS:
		s1 = va_arg(ap,char *);
		s2 = va_arg(ap,char *);
		sprintf(buf,s,s1,s2);
		break;
	case HEP_SDD:
		s1 = va_arg(ap,char *);
		i1 = va_arg(ap,int);
		i2 = va_arg(ap,int);
		sprintf(buf,s,s1,i1,i2);
		break;
	case HEP_SDS:
		s1 = va_arg(ap,char *);
		i1 = va_arg(ap,int);
		s2 = va_arg(ap,char *);
		sprintf(buf,s,s1,i1,s2);
		break;
	case HEP_SSS:
		s1 = va_arg(ap,char *);
		s2 = va_arg(ap,char *);
		s3 = va_arg(ap,char *);
		sprintf(buf,s,s1,s2,s3);
		break;
	case HEP_DDDD:
		i1 = va_arg(ap,int);
		i2 = va_arg(ap,int);
		i3 = va_arg(ap,int);
		i4 = va_arg(ap,int);
		sprintf(buf,s,i1,i2,i3,i4);
		break;
	case HEP_SDDDDS:
		s1 = va_arg(ap,char *);
		i1 = va_arg(ap,int);
		i2 = va_arg(ap,int);
		i3 = va_arg(ap,int);
		i4 = va_arg(ap,int);
		s2 = va_arg(ap,char *);
		sprintf(buf,s,s1,i1,i2,i3,i4,s2);
		break;
	case HEP_SSSS:
		s1 = va_arg(ap,char *);
		s2 = va_arg(ap,char *);
		s3 = va_arg(ap,char *);
		s4 = va_arg(ap,char *);
		sprintf(buf,s,s1,s2,s3,s4);
		break;
	default:
		va_end(ap);
		return(perr(HE_BADFMT,fmt));
	}
	va_end(ap);
	lenb = strlen(buf);
	len = lenb + hd->sizehist;
	if (buf[lenb-1] == '\n')
		nflag = FALSE;
	else {
		nflag = TRUE;
		len++;
	}
	if ((newh = (char *) hmalloc(len+1)) == (char *) HIPS_ERROR)
		return(HIPS_ERROR);
	strcpy(newh,hd->seq_history);
	strcat(newh,buf);
	if (nflag) {
		newh[len-1] = '\n';
		newh[len] = '\0';
	}
	hd->sizehist = len;
	if (hd->histdealloc)
		free(hd->seq_history);
	hd->seq_history = newh;
	hd->histdealloc = TRUE;
	return(HIPS_OK);
}

int history_indentadd(hd,s)

struct header *hd;
char *s;

{
	char *newh,*ps,*pnh;
	int len,len2;
	h_boolean flag;

	ps = s;
	len2 = len = 0;
	while (*ps) {
		len2++;
		if (*ps++ == '\n')
			len += 5;
		else
			len++;
	}
	if (len2 == 0 || s[len2-1] != '\n')
		len += 5;
	if ((newh = (char *) hmalloc(len + hd->sizehist + 2))
		== (char *) HIPS_ERROR)
			return(HIPS_ERROR);
	strcpy(newh,hd->seq_history);
	pnh = newh + hd->sizehist;
	*pnh++ = ' ';
	*pnh++ = ' ';
	*pnh++ = ' ';
	*pnh++ = ' ';
	ps = s;
	flag = FALSE;
	while (*ps) {
		if (*ps == '\n') {
			*pnh++ = *ps++;
			flag = TRUE;
		}
		else {
			if (flag) {
				flag = FALSE;
				*pnh++ = ' ';
				*pnh++ = ' ';
				*pnh++ = ' ';
				*pnh++ = ' ';
			}
			*pnh++ = *ps++;
		}
	}
	if (!flag)
		*pnh++ = '\n';
	*pnh++ = '\0';
	hd->sizehist += len;
	if (hd->histdealloc)
		free(hd->seq_history);
	hd->seq_history = newh;
	hd->histdealloc = TRUE;
	return(HIPS_OK);
}
