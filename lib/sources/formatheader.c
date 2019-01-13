/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * formatheader.c - format a sequence header into readable form
 *
 * string = (char *) formatheader(&hd);
 *
 * HIPS-2 version - msl - 1/3/91
 */

#include <hipl_format.h>

char *hstring,*hsptr;
int msglen;
void adddec(),addstr();
int hstrlen = 0;

char *formatheader(h)

struct header *h;

{
	return(formatheaderc(h,FALSE));
}

char *formatheadera(h)

struct header *h;

{
	return(formatheaderc(h,TRUE));
}

char *formatheaderc(h,aflag)

struct header *h;
h_boolean aflag;

{
	char s[100];
	struct extpar *xp;
	int i,j,lim,*pi,nd;
	byte *pb;
	short *ps;
	float *pf;

	hsptr = hstring;
	msglen = 0;
	addstr("Original name:\t\t\t");
	addstr(h->orig_name);
	addstr("Sequence name:\t\t\t");
	addstr(h->seq_name);
	if ((nd = hgetdepth(h)) == HIPS_ERROR)
		return((char *) nd);
	addstr("Number of frames:\t\t");
	adddec(h->num_frame);
	if (h->numcolor > 1) {
		if (nd > 1) {
			addstr("\nNumber of 3D color frames:\t");
			adddec(h->num_frame/(nd * h->numcolor));
		}
		else {
			addstr("\nNumber of color frames:\t\t");
			adddec(h->num_frame/h->numcolor);
		}
	}
	else if (nd > 1) {
		addstr("\nNumber of 3D frames:\t\t");
		adddec(h->num_frame/nd);
	}
	addstr("\nOriginal date:\t\t\t");
	addstr(h->orig_date);
	addstr("Number of stored rows:\t\t");
	adddec(h->orows);
	addstr("\nNumber of stored columns:\t");
	adddec(h->ocols);
	addstr("\nNumber of ROI rows:\t\t");
	adddec(h->rows);
	addstr("\nNumber of ROI columns:\t\t");
	adddec(h->cols);
	addstr("\nFirst ROI row:\t\t\t");
	adddec(h->frow);
	addstr("\nFirst ROI column:\t\t");
	adddec(h->fcol);
	if (h->pixel_format == PFTOSPACE)
		addstr("\nFormat:\t\t\t\t");
	else
		addstr("\nPixel format:\t\t\t");
	addstr(hformatname(h->pixel_format));
	addstr("\nNumber of color planes:\t\t");
	adddec(h->numcolor);
	addstr("\nNumber of depth planes:\t\t");
	adddec(nd);
	if (h->sizehist > 1) {
		addstr("\n\nSequence history:\n\n");
		addstr(h->seq_history);
		addstr("\n");
	}
	else
		addstr("\n\nNo sequence history\n\n");
	if (h->sizedesc > 1) {
		addstr("Sequence Description:\n\n");
		addstr(h->seq_desc);
		addstr("\n");
	}
	else
		addstr("No sequence description\n");
	addstr("\nNumber of parameters:\t\t");
	adddec(h->numparam);
	addstr("\n");
	xp = h->params;
	while (xp != NULLPAR) {
		if (xp->count == 1) {
			switch (xp->format) {
			case PFASCII:	sprintf(s,"%s (1 char) = '%c'\n",
						xp->name,xp->val.v_b);
					break;
			case PFBYTE:	sprintf(s,"%s (1 byte) = %d\n",
						xp->name,(int) xp->val.v_b);
					break;
			case PFSHORT:	sprintf(s,"%s  (1 short) = %d\n",
						xp->name,(int) xp->val.v_s);
					break;
			case PFINT:	sprintf(s,"%s (1 int) = %d\n",
						xp->name,xp->val.v_i);
					break;
			case PFFLOAT:	sprintf(s,"%s (1 float) = %f\n",
						xp->name,xp->val.v_f);
					break;
			default:	sprintf(s,"%s (1 unknown format %d)\n",
						xp->name,xp->format);
					break;
			}
			addstr(s);
		}
		else {
			lim = ((xp->count < 5) || aflag) ? xp->count : 5;
			switch (xp->format) {
			case PFASCII:	sprintf(s,"%s (%d chars) = \"",xp->name,
						xp->count);
					addstr(s);
					lim = ((xp->count < 40) || aflag)
						? xp->count : 40;
					pb = xp->val.v_pb;
					j = 11;
					for (i=0;i<lim;i++) {
						if ((j>62) && (*pb!='\n')) {
							addstr("\n\t\t@");
							j = 1;
						}
						if (*pb == '\n') {
							addstr("\n\t\t");
							j = 0;
							pb++;
						}
						else {
							j++;
							sprintf(s,"%c",*pb++);
							addstr(s);
						}
					}
					addstr("\"");
					break;
			case PFBYTE:	sprintf(s,"%s (%d bytes) =",xp->name,
						xp->count);
					addstr(s);
					pb = xp->val.v_pb;
					for (i=0;i<lim;i++) {
						if ((i>0) && ((i%10)==0))
							addstr("\n\t\t");
						sprintf(s," %d",(int) *pb++);
						addstr(s);
					}
					break;
			case PFSHORT:	sprintf(s,"%s (%d shorts) =",xp->name,
						xp->count);
					addstr(s);
					ps = xp->val.v_ps;
					for (i=0;i<lim;i++) {
						if ((i>0) && ((i%5)==0))
							addstr("\n\t\t");
						sprintf(s," %d",(int) *ps++);
						addstr(s);
					}
					break;
			case PFINT:	sprintf(s,"%s (%d ints) =",xp->name,
						xp->count);
					addstr(s);
					pi = xp->val.v_pi;
					for (i=0;i<lim;i++) {
						if ((i>0) && ((i%5)==0))
							addstr("\n\t\t");
						sprintf(s," %d",*pi++);
						addstr(s);
					}
					break;
			case PFFLOAT:	sprintf(s,"%s (%d floats) =",xp->name,
						xp->count);
					addstr(s);
					pf = xp->val.v_pf;
					for (i=0;i<lim;i++) {
						if ((i>0) && ((i%5)==0))
							addstr("\n\t\t");
						sprintf(s," %f",*pf++);
						addstr(s);
					}
					break;
			default:	return((char *) perr(HE_HDPTYPE,
						"formatheader",xp->format));
			}
			if (lim < xp->count)
				addstr(" ...\n");
			else
				addstr("\n");
		}
		xp = xp->nextp;
	}
	*hsptr = 0;
	return(hstring);
}

void adddec(i)

int i;

{
	char s[20];

	sprintf(s,"%d",i);
	addstr(s);
}

void addstr(s)

char *s;

{
	int i,incr;
	char *tmp;

	i = strlen(s);
	if (i+msglen+1 > hstrlen) {
		incr = (512 > (i+1)) ? 512 : (i+1);
		tmp = memalloc(hstrlen+incr,sizeof(char));
		if (hstrlen) {
			strcpy(tmp,hstring);
			free(hstring);
		}
		hstrlen += incr;
		hstring = tmp;
		hsptr = hstring + msglen;
	}
	strcpy(hsptr,s);
	msglen += i;
	hsptr += i;
}
