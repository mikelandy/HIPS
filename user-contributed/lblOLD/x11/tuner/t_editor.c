/*	Tuner_EDITOR . C
#
%	Copyright (c)	Jin Guojun	-	All Rights Reserved
%
%	The ELASTIC TUNER editor routines.
%
% Author:	Jin Guojun - Lawrence Berkeley Laboratory	4/1/91
*/

#include "tuner.h"
#include "udir.h"


GetImageBufNSize(img)
register Image	*img;
{
register int	size = img->width * img->height;
	img->cnvt = img->data + (img->color_dpy ? 0 : img->fn * size);
return	size;
}


void
PaintImage(Image* img, int*	y0, int s,
	int r, int g, int b)
{
int	wh, d = img->sub_img_w = img->sub_img_h = (s<<1) + 1,
	gs = AdoptColor(img, r, g, b, precision);

    while (1) {
	wh = WaitButtonPress_n_InfoImage(img, &histinfo, y0,
		XCreateFontCursor(img->dpy, XC_crosshair));
	RemoveImageEvent(img, ButtonAction)	XBell(img->dpy, 0);
	if (wh == Button3)	break;
	if (wh == Button2)	{
		gs = PickUpColor(img, &r, &g, &b);
		continue;
	}
	XDefineCursor(img->dpy, img->win, XCreateFontCursor(img->dpy, XC_pencil));
	img->update = True;
	while (!ImageEvent(img, ButtonReleaseMask))
	    if (ImageEvent(img, PointerMotionMask))
		PaintArea(img, img->cnvt, img->event->xbutton.x,
			img->event->xbutton.y, r, g, b, gs, s);
    }	/* end while (1) */
}

CopyOrCutSubImage(Image	*img, bool ed_cut, int	bgd_c)
{

	I_ED.x0 = img->sub_img_x;
	I_ED.y0 = img->sub_img_y;
	I_ED.w = img->sub_img_w;
	I_ED.h = img->sub_img_h;
	if (I_ED.copyarea)
		CFREE(I_ED.copyarea);
	I_ED.copyarea = nzalloc(I_ED.w * I_ED.h, img->channels, "I_ED.copy");
	{
	register byte*	cp = (byte *) I_ED.copyarea,
		*srcp = img->cnvt + img->channels*img->width*I_ED.y0 + I_ED.x0;
	register int	i;
		for (i=I_ED.h*img->channels; i--;) {
			memcpy(cp, srcp, I_ED.w);
			cp += I_ED.w;
			srcp += img->width;
		}
		if (I_ED.cut = ed_cut)	{ /* using paint is slow but easy */
		int	r, g, b=bgd_c;
			if (img->in_cmap)	{
				r = img->in_cmap[0][b];
				g = img->in_cmap[1][b];
				b = img->in_cmap[2][b];
			} else	r = g = b;
		PaintArea(img, img->data, RELATIVExy(img, I_ED.x0, I_ED.y0),
			r, g, b, bgd_c, 0);
		}
		I_ED.fill = True;
		I_ED.src = img;
	}
}

PrepareToEdit(imgp, num_imgs, msg, lbuf, size, event)
Image**	imgp;
XEvent*	event;
char*	msg, *lbuf;
{
	sprintf(lbuf, "R=%d, G=%d, B=%d", fnt_r, fnt_g, fnt_b);
	InputFrom_Panel(MsgButton, lbuf, size, 8, NULL, "R, G, B ", Green, No);
	sscanf(lbuf, "%d %d %d", &fnt_r, &fnt_g, &fnt_b);
	DisplayMessage(NoteWin, msg, 4, 0);
	XMaskEvent(Dpy, ButtonPressMask, event);
return	WhichImage(event->xany.window, imgp, num_imgs);
}


static	struct	eda_list	{
	ED_Append_t	**eda;
	int	num, nsp;
	} eda_list;


ed_append_add(cookie_t eid, struct eda_list *eda_lp, pw_event *pw_ep)
{
int	w, h;
ED_Append_t	*eda = eda_lp->eda[pw_ep->argu0];
PressButton	*pb = (PressButton *) eid;
Panel	*pan = pb->pan;
Image	*img = (Image *) pw_ep->CooKie, *simg = eda->simg;
	ResetPressButton(pb);
	if (!(simg->width | simg->height))	{
		simg->width = img->width;
		simg->height = img->height;
		simg->dpy_channels = img->dpy_channels;
		simg->color_dpy = img->color_dpy;
		simg->in_type = -1;	/* unknown type	*/
	}
	w = simg->width;	h = simg->height;
	if (img->width != w || img->height != h)
		return	prgmerr(0, "size %d x %d not match %d x %d",
			img->width, img->height, w, h);
	if (++eda->frames >= eda->nbufs)
		eda->buf_list = realloc(eda->buf_list,
				sizeof(cookie_t) * (eda->nbufs += 16));
	eda->buf_list[eda->frames - 1] = (char *)img->data + img->fn * w * h;
	w = 0;	/* no update	*/
	if (eda->adir == PrevFRAME)	{
		if (w = img->fn > 0)	img->fn--;
	} else	if (w = img->fn < img->frames-1)	img->fn++;
	if (w)	w = Update_imageNpanel(img);
return	w;
}

ed_append_finish(cookie_t eid, struct eda_list *eda_lp, pw_event *pw_ep)
{
ED_Append_t*	eda = eda_lp->eda[pw_ep->argu0];
Image	*simg = eda->simg;
int	i, chan = simg->dpy_channels, fsz = simg->width * simg->height;
	if (simg->IN_FP)	{
		simg->load_all = simg->frames;
		(*simg->std_swif)(FI_LOAD_FILE, simg, 0, 0, No);
		fclose(simg->IN_FP);
	}
	if (!(simg->OUT_FP = fopen(eda->f_name, "wb")))
		prgmerr(0, "open %s for write", eda->f_name);
	else	{
		simg->frames += eda->frames;
		(*simg->header_handle)(HEADER_WRITE, simg, 1, &eda->f_name, 0);
		fwrite(simg->data, (simg->frames - eda->frames) * chan, fsz,
				simg->OUT_FP);
		for (i=0; i < eda->frames; i++)
			fwrite(eda->buf_list[i], chan, fsz, simg->OUT_FP);
	}
	CFREE(simg->data);
	CFREE(simg);
	CFREE(eda->f_name);
	CFREE(eda->buf_list);
	PW_DestroyPanel(eda->pan);	/* this release all childen	*/
	/* PW_Destroy(eda->add);	PW_Destroy(eda->finish);	*/
	CFREE(eda);	eda_lp->eda[pw_ep->argu0] = NULL;
	eda_lp->num--;
}

create_append(cookie_t	any)
{
char	name[256];
ED_Append_t*	eda;
Image	*simg;
    if (GetFilenameFromX(NULL, name, SFile_DIRS | SFile_IMAGES)) {
	int	i = eda_list.num;
	while (i--)
		if  (!strcmp(name, eda_list.eda[i]->f_name))
			return	prgmerr(0, "name %s is used", name);

	i = ++eda_list.num;	/* -i reallocating eda	*/
	i = verify_buffer_size(&eda_list.eda, -i, sizeof(eda), "edap");
	if (i && i != -1)
		eda_list.nsp++,	i = eda_list.num - 1;
	else if ((i=eda_list.num) < eda_list.nsp)
		while (eda_list.eda[--i]);
	eda = eda_list.eda[i] = ZALLOC(1, sizeof(ED_Append_t), "eda");
	eda->simg = simg = ZALLOC(sizeof(Image), 1, "simg");
	simg->color_dpy = -1;
	format_init(simg, IMAGE_INIT_TYPE, HIPS, HIPS, "eda", "Nov1-4");
	if (simg->IN_FP = fopen(name, "rb"))
		(*simg->header_handle)(HEADER_READ, simg, 0, 0, False);
	eda->nbufs = 16;
	eda->buf_list = ZALLOC(16, sizeof(cookie_t), "apbuf");
	eda->f_name = str_save(name);
	eda->pan = PW_CreatePanel(Monitor, PW_Label, name,
				PW_Height, 128, PW_Width, 192, NULL);
	eda->add = PW_CreatePressButton(eda->pan, PW_Label, "add_frame",
			PW_Origin, 16, 96, PW_FgColor, Gray);
	eda->finish = PW_CreatePressButton(eda->pan, PW_Label, "finish",
			PW_Origin, 112, 96, PW_FgColor, Gray);
	PW_AddCallback(eda->add, PW_BUTTONPRESS, ed_append_add, &eda_list, i);
	PW_AddCallback(eda->finish, PW_BUTTONPRESS, ed_append_finish, &eda_list,i);
	ShowPanel(eda->pan);
	XFlush(eda->pan->dpy);
	PW_ShowNode(eda->pan, NULL, EOF);
    }
return	0;
}

GetFilenameFromX(cookie_t panp, char *rbuf, int ftype)
{
char	*fdir = ".";
cookie_t	fmenu=0;
F_List_t	flist;
	init_dirList(&flist, Dir_Size);
	if (!ftype)	/* imply dirs only	*/
		ftype = SFile_DIRS;
    Loop	{
	int	ft, ncd, nf = dirFList(&flist, fdir, ftype);
	if (!fmenu)	{
	    if (!(fmenu = PW_CreatePopMenu(Epanel, &Monitor, PW_FreeMem,
			PW_Lists, flist.fname, nf, PW_Label, "Dir", NULL)))
		return	0;
	} else	ChangePopMenu(fmenu, flist.fname, nf);
	if (!ShowDIR_Only(ftype))
		ftype &= SFile_EXTFMT;	/* don't return	on DIRS	*/
Lookf:	nf = PopingMenu(fmenu, -2 /* don't wait */, NULL);
	strcpy(rbuf, flist.fname[--nf]);
	ncd = strcmp(rbuf, ".");
	if ((ft = flist.finfo[nf].type) & ftype && ncd)	{
Foundf:		PW_Destroy(fmenu);
		return	nf;
	}
	if (!(nf=ncd))
		if (ft & SFile_DIRS)	{
			if (!panp)
				panp = MsgButton ? MsgButton : fButton;
		} else	goto	Foundf;	/* abort look	*/
	if (ft & SFile_DIRS)	{
	    if (panp)	{
		struct stat	stb;
		nf = strlen(rbuf);
		if (!ncd)
			rbuf[0] = nf = 0;	/* = ./	*/
		else if(rbuf[nf-1] != '/')
			rbuf[nf++] = '/';
		InputFrom_Panel(panp, rbuf + nf, 256 - nf, 2,
			"type a File or Dir name", NULL, Blue, No);
		if (rbuf[0] == '~')	{	/* interpolate $HOME	*/
		char	*home = getenv("HOME");
			if (home && (ncd = strlen(home)))
				bcopy(rbuf + 1, rbuf + ncd, strlen(rbuf)),
				strncpy(rbuf, home, ncd);
		}
		nf = !(stat(rbuf, &stb) && (stb.st_mode & S_IFMT) == S_IFDIR);
		goto	Foundf;
	    }
	    for (nf=flist.nfiles=0; flist.fname[nf]; nf++)
		CFREEnNULL(flist.fname[nf]);
	    fdir = rbuf;	/* temporally change dir	*/
	    nf = strlen(fdir);
	    if (!strcmp(fdir + nf - 2, "/."))
		strcpy(fdir, ".");
	} else	if (YesOrNo("wrong file, continue to look?", 0))
		goto	Lookf;
	else	{
		nf = 0;	goto	Foundf;	/* abort	*/
	}
    }
}
