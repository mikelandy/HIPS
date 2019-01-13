#include <stdio.h>
#include <MenuMgr.h>
#include <WindowMgr.h>
#include <DialogMgr.h>
#include <StdFilePkg.h>

#include "hips_menu.h"

static MenuHandle	AppleMenu;
static MenuHandle	FileMenu;
static MenuHandle	EditMenu;
static MenuHandle	ViewsMenu;
static MenuHandle	CoordsMenu;
static MenuHandle	DataMenu;

#define	APPLE_ABOUT_ID	128


/*
 *	Setup the menu
 */
SetUpMenus()
  {
	char	appleTitle[256];

	appleTitle[0] = 1;		
	appleTitle[1] = 0x14;				/* "apple" character will be in color if color exists*/
  	AppleMenu = NewMenu(APPLE_ID,appleTitle);
	AppendMenu(AppleMenu, "\pAbout View HIPS");
	AppendMenu(AppleMenu, "\p-");
  	AddResMenu( AppleMenu, 'DRVR');
  	

	FileMenu = NewMenu(FILE_ID,"\pFile");
	AppendMenu(FileMenu, "\pOpen HIPS/O");
	AppendMenu(FileMenu, "\pWrite PICT");
	AppendMenu(FileMenu, "\p-");
	AppendMenu(FileMenu, "\pPage Setup");
	AppendMenu(FileMenu, "\pPrint");
	AppendMenu(FileMenu, "\p-");
	AppendMenu(FileMenu, "\pQuit/Q");

	EditMenu = NewMenu(EDIT_ID,"\pEdit");
	AppendMenu(EditMenu, "\pUndo");
	AppendMenu(EditMenu, "\p-");
	AppendMenu(EditMenu, "\pCut/X");
	AppendMenu(EditMenu, "\pCopy/C");
	AppendMenu(EditMenu, "\pPaste/V");

	ViewsMenu = NewMenu(VIEWS_ID,"\pViews");
	AppendMenu(ViewsMenu, "\pView Header/H");
	AppendMenu(ViewsMenu, "\p-");
	AppendMenu(ViewsMenu, "\pMagnify/M");
	AppendMenu(ViewsMenu, "\pDeMagnify/D");
	AppendMenu(ViewsMenu, "\pOriginal Size");
	AppendMenu(ViewsMenu, "\p-");
	AppendMenu(ViewsMenu, "\pGrays Or Colors");
	AppendMenu(ViewsMenu, "\pInvert Colors/I");
  	

	CoordsMenu = NewMenu(COORDS_ID,"\pCoordinates");
	AppendMenu(CoordsMenu, "\pSet XY Origin");
	AppendMenu(CoordsMenu, "\pSet XY Maximums");
	AppendMenu(CoordsMenu, "\p-");
	AppendMenu(CoordsMenu, "\pSet X Scale & Offset");
	AppendMenu(CoordsMenu, "\pSet Y Scale & Offset");
	AppendMenu(CoordsMenu, "\pSet Z Scale & Offset");
	AppendMenu(CoordsMenu, "\p-");
	AppendMenu(CoordsMenu, "\pSave the current coordinates");
  	
	DataMenu = NewMenu(DATA_ID,"\pData");
	AppendMenu(DataMenu, "\pSave Data, average across X");
	AppendMenu(DataMenu, "\pSave Data, average across Y");

  	InsertMenu(AppleMenu, 0);
  	InsertMenu(FileMenu, 0);
  	InsertMenu(EditMenu, 0);
  	InsertMenu(ViewsMenu, 0);
  	InsertMenu(CoordsMenu, 0);
  	InsertMenu(DataMenu, 0);

  	DrawMenuBar();
  }

/*
 *	Execute the menu selction
 */
DoMenu( menuResult )
long	menuResult;
  {
  	int	menuItem;
  	
  	menuItem = LoWord(menuResult);
  	switch( HiWord(menuResult) )
  	  {
  		case APPLE_ID:
  			DoAppleMenu(menuItem);
  			break;
  		case FILE_ID:
  			DoFileMenu(menuItem);
  			break;
  		case EDIT_ID:
  			DoEditMenu(menuItem);
  			break;
  		case VIEWS_ID:
  			DoViewsMenu(menuItem);
  			break;
  		case COORDS_ID:
  			DoCoordsMenu(menuItem);
  			break;
  		case DATA_ID:
  			DoDataMenu(menuItem);
  			break;
  		default:
  		  	break;
  	  }
  }

/*
 *	file menu operations
 */
DoFileMenu(menuItem)
int	menuItem;
  {
  	SFReply	reply;
  	Point where;
  	
	switch(menuItem) {
		case FILE_OPEN:		/* open */
			where.v = 75;  where.h = 100;
			SFGetFile(where, "\p", NULL, 1, "TEXT", NULL, &reply);
			if(reply.good) {
				SetVol(NULL, reply.vRefNum);
				PtoCstr(reply.fName);
				HipsOpen( reply.fName );
			}
			break;
		case FILE_WRITE:		/* write picture */
			SysBeep(1);
			break;
		case FILE_PAGESETUP:		/* page setup */
			SysBeep(1);
			break;
		case FILE_PRINT:		/* print */
			SysBeep(1);
 			break;
		case FILE_QUIT:
			ExitToShell();
			break;
	}
  }

/*
 *	Apple menu operations
 */
DoAppleMenu(menuItem)
int	menuItem;
  {
  	char	accName[256];
  	
  	switch(menuItem)
  	  {
  	  	case APPLE_ABOUT:
  	  		DoAbout();
  	  		break;
  	  	default:
  	  		GetItem(AppleMenu,menuItem,&accName);
  	  		OpenDeskAcc(&accName);
  	  		break;
  	  }
  }
  
  
/*
 *	Hips menu operations
 */
DoViewsMenu(menuItem)
int	menuItem;
  {
  	WindowRecord  *wPtr;
  	long refConstant;
  	Size siz;
  	int grow;

	wPtr = (WindowRecord *)FrontWindow();
  	
	switch( menuItem )
	{
		case HIPS_HEADER:
			HipsHeader();
			break;
		case HIPS_MAGNIFY:
			HipsMagnify( wPtr );
		  	break;
		case HIPS_DEMAGNIFY:
			HipsDeMagnify( wPtr );
		  	break;
		case HIPS_ORIGINAL:
			HipsOriginalSize( wPtr );
		  	break;
		case HIPS_GRAY_COLOR:
			HipsGrayColor( wPtr );
		  	break;
		case HIPS_INVERT_COLOR:
			HipsInvertColor( wPtr );
		  	break;

	}
  }


/*
 *
 */
DoEditMenu(menuItem)
int	menuItem;
  {
  	WindowRecord *wPtr;
  	
	if (SystemEdit (menuItem - 1))		/* check DA edit choice */
	{
		return;
	}

  	wPtr = (WindowRecord *)FrontWindow();
  	
  	switch(menuItem)
	{
		case EDIT_UNDO:
			SysBeep(10);
			break;
		case EDIT_CUT:
			SysBeep(10);
			break;
		case EDIT_COPY:
			DoEditCopy(wPtr);
			break;
		case EDIT_PASTE:
			SysBeep(10);
			break;
		default:
			break;
	}

}


DoEditCopy(wPtr)
WindowPtr	wPtr;
{
	HipsCopy();
}


DoAbout()
{
  {
 	DialogPtr	dptr;
 	int	itemHit,kind;
 	Handle	itemH;
 	Rect	dbox;
 	Size grow,siz;
 	char m[256];
 	
 	dptr = GetNewDialog(APPLE_ABOUT_ID,NULL,-1L);
	
  	/*display the memory message in item number 3 of the dialog  */
 	siz = MaxMem( &grow );
  	sprintf(m,"Available memory is %ld bytes.", siz );
 	GetDItem(dptr,3,&kind,&itemH,&dbox);
 	CtoPstr( m );
	SetIText(itemH,m);

 	do
 	{
 		ModalDialog(NULL,&itemHit);
 		
 	} while ( itemHit != OK );
 	
 	
 	DisposDialog(dptr);
 	
  }
}

DoCoordsMenu(menuItem)
int	menuItem;
{
  	WindowRecord  *wPtr;

	wPtr = (WindowRecord *)FrontWindow();
  	SetPort( wPtr );						/*so the rest can know */
  	
	switch( menuItem )
	{
		case COORD_XY_ORIGIN:
			SetXYCoordOrigin( );
			break;
		case COORD_XY_MAXS:
			SetXYCoordMaxs( );
		  	break;
		case COORD_X_SCALE:
			SetXScale();
			break;
		case COORD_Y_SCALE:
			SetYScale();
			break;
		case COORD_Z_SCALE:
			SetZScale();
			break;
		case COORD_SAVE:
			SaveXYZScales();
			break;
	}
}



DoDataMenu(menuItem)
int menuItem;
{
  	WindowRecord  *wPtr;

	wPtr = (WindowRecord *)FrontWindow();
  	SetPort( wPtr );						/*so the rest can know */
  	
	switch( menuItem )
	{
		case DATA_AVG_X:
			DataAvgX( );
			break;
		case DATA_AVG_Y:
			DataAvgY( );
		  	break;
	}
}
