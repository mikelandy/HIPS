#define	MAXHIPS			10
#define COORD_WIND_ID	100

STRCLASS	CursHandle		watchCursHndl;

STRCLASS 	WindowPtr		hips_wind_ptr[MAXHIPS];
STRCLASS	PaletteHandle	pal_hndl[MAXHIPS];

STRCLASS 	WindowPtr		coord_wind_ptr;

STRCLASS	struct 			header 	hd[MAXHIPS];


STRCLASS	GrafPort		myCGrafPort[MAXHIPS];
STRCLASS	CGrafPtr 		myCGrafPtr[MAXHIPS];
STRCLASS	char			*myBits[MAXHIPS];
STRCLASS	CTabHandle     	ourCMHandle[MAXHIPS];
STRCLASS	int				usedIndex[MAXHIPS];

STRCLASS	int		  		zoomisdrawn[MAXHIPS];
STRCLASS	Rect			zoom_sorc_rect[MAXHIPS];	/*this is needed for zooms, etc. */
STRCLASS	Rect			pic_sorc_rect[MAXHIPS];


STRCLASS	int				x_origin[MAXHIPS];
STRCLASS	int				y_origin[MAXHIPS];
STRCLASS	int				z_origin[MAXHIPS];
STRCLASS	int				x_max[MAXHIPS];
STRCLASS	int				y_max[MAXHIPS];
STRCLASS	int				z_max[MAXHIPS];
STRCLASS	double			x_offset[MAXHIPS];
STRCLASS	double			y_offset[MAXHIPS];
STRCLASS	double			z_offset[MAXHIPS];
STRCLASS	double			x_scale_per_pixel[MAXHIPS];
STRCLASS	double			y_scale_per_pixel[MAXHIPS];
STRCLASS	double			z_scale_per_pixel[MAXHIPS];
STRCLASS	char			x_units[MAXHIPS][256];
STRCLASS	char			y_units[MAXHIPS][256];
STRCLASS	char			z_units[MAXHIPS][256];
