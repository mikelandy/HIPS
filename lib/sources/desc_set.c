/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * desc_set.c - HIPL Picture Format description management
 *
 * Michael Landy - 1/3/91
 */

#include <hipl_format.h>

#ifdef HUSESTDARG
#include <stdarg.h>
#else
#include <varargs.h>
#endif

int desc_set(hd,s)

struct header *hd;
char *s;

{
	hd->seq_desc = s;
	hd->sizedesc = strlen(s);
	hd->seqddealloc = FALSE;
	return(HIPS_OK);
}

int desc_append(hd,s)

struct header *hd;
char *s;

{
	int i,lens;
	h_boolean nflag;
	char *news,*olds;

	lens = strlen(s);
	i = strlen(hd->seq_desc) + strlen(s) + 1;
	if (s[lens-1] == '\n')
		nflag = FALSE;
	else {
		nflag = TRUE;
		i++;
	}
	if ((news = (char *) hmalloc(i)) == (char *) HIPS_ERROR)
		return(HIPS_ERROR);
	olds = hd->seq_desc;
	hd->seq_desc = news;
	strcpy(news,olds);
	strcat(news,s);
	if (nflag) {
		news[i-2] = '\n';
		news[i-1] = '\0';
	}
	hd->sizedesc = i-1;
	if (hd->seqddealloc)
		free(olds);
	hd->seqddealloc = TRUE;
	return(HIPS_OK);
}

/* desc_set2(header,argformat,fmtstring,sprintfargs...) */

#ifdef HUSESTDARG

int desc_set2(struct header *hd, ...)

{

#else

int desc_set2(va_alist)

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
	if ((hd->seq_desc = (char *) hmalloc(len+1)) == (char *) HIPS_ERROR)
		return(HIPS_ERROR);
	strcpy(hd->seq_desc,buf);
	if (nflag) {
		hd->seq_desc[len-1] = '\n';
		hd->seq_desc[len] = '\0';
	}
	hd->sizedesc = len;
	hd->seqddealloc = TRUE;
	return(HIPS_OK);
}

/* desc_append2(header,argformat,fmtstring,sprintfargs...) */

#ifdef HUSESTDARG

int desc_append2(struct header *hd, ...)

{
#else

int desc_append2(va_alist)

va_dcl

{
	struct header *hd;
#endif
	va_list ap;
	int fmt,i1,i2,i3,i4,len,lenb;
	h_boolean nflag;
	char *s,*s1,*s2,*s3,*s4,buf[LINELENGTH],*newd;

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
	len = lenb + hd->sizedesc;
	if (buf[lenb-1] == '\n')
		nflag = FALSE;
	else {
		nflag = TRUE;
		len++;
	}
	if ((newd = (char *) hmalloc(len+1)) == (char *) HIPS_ERROR)
		return(HIPS_ERROR);
	strcpy(newd,hd->seq_desc);
	strcat(newd,buf);
	if (nflag) {
		newd[len-1] = '\n';
		newd[len] = '\0';
	}
	hd->sizedesc = len;
	if (hd->seqddealloc)
		free(hd->seq_desc);
	hd->seq_desc = newd;
	hd->seqddealloc = TRUE;
	return(HIPS_OK);
}

int desc_indentadd(hd,s)

struct header *hd;
char *s;

{
	char *newd,*ps,*pnd;
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
	if ((newd = (char *) hmalloc(len + hd->sizedesc + 2))
		== (char *) HIPS_ERROR)
			return(HIPS_ERROR);
	strcpy(newd,hd->seq_desc);
	pnd = newd + hd->sizedesc;
	*pnd++ = ' ';
	*pnd++ = ' ';
	*pnd++ = ' ';
	*pnd++ = ' ';
	ps = s;
	flag = FALSE;
	while (*ps) {
		if (*ps == '\n') {
			*pnd++ = *ps++;
			flag = TRUE;
		}
		else {
			if (flag) {
				flag = FALSE;
				*pnd++ = ' ';
				*pnd++ = ' ';
				*pnd++ = ' ';
				*pnd++ = ' ';
			}
			*pnd++ = *ps++;
		}
	}
	if (!flag)
		*pnd++ = '\n';
	*pnd++ = '\0';
	hd->sizedesc += len;
	if (hd->seqddealloc)
		free(hd->seq_desc);
	hd->seq_desc = newd;
	hd->seqddealloc = TRUE;
	return(HIPS_OK);
}
