#include <fopenw.h>


#include <QuickDraw.h>
#include <EventMgr.h>
#include <WindowMgr.h>
#include <ToolboxUtil.h>
#include <ColorToolbox.h>

#include "hipl_format.h" 

#define STRCLASS	extern
#include "globals.h"

extern	FILE	*info;

	StdWindowOptions infoWindOpt = {		/*options for the info window style, see <fopnw.h>*/
	24, 60, TRUE, TRUE, 4, TRUE,			/*easy to initialize all here !*/
	FALSE, FALSE, TRUE, FALSE, FALSE
};


main()
  {
  	
  	InitMac();
   	Init();				/*my init's*/

   	EventLoop();
  
  }
  
/*
 *	Initialize the MAC
 */
InitMac()
  {
  	int Resume();
  	WindowPtr std_wind;
  	Point upperLeft;
  	
  	MaxApplZone();
  	InitGraf( &thePort );
  	InitFonts();
  	InitWindows();
  	InitMenus();
  	InitDialogs( (ProcPtr)Resume );
  	InitCursor();
  	TEInit();
  	FlushEvents( everyEvent, 0 );
  	
  	Stdio_MacInit(TRUE);
  	Click_On(FALSE);

	/*initialize the info window */
	/* for now set it to stderr */
	info = stderr;

	HideWindow((WindowPtr) Get_WindowPtr(stdout)); /*an undocumented LSC function ? */
	
	upperLeft.h = 230;  upperLeft.v = 40;
	info = fopenw("\Information", upperLeft, &infoWindOpt);	


  }
  
/*
 *	In case a Bomb message!
 */
Resume()
  {
  	Debugger();
  	ExitToShell();
  }
  
/*
 *	My initializations
 */
Init()
  {
  	Rect bounds;

  	watchCursHndl = GetCursor ( watchCursor );
  	SetUpMenus();
  	
  	SetDeskCPat( 0L );
  	
  	
  	bounds.top = 40;
	bounds.left = 2;
	bounds.bottom = bounds.top + 70;
	bounds.right = bounds.left + 220;

  	coord_wind_ptr = NewCWindow(NULL, &bounds, "\pCoordinates",
					TRUE, documentProc, (WindowPtr)(-1L), FALSE, (long)COORD_WIND_ID  );

  }

/*
 *	loop forever
 */
EventLoop()
  {
  	EventRecord	event;
  	char theChar;
  	
  	for(;;)
  	  {
  		SystemTask();
  		MouseCoord();	/*always track mouse */
  		if ( GetNextEvent(everyEvent,&event) == FALSE ) continue;	/*no event to process*/
 		if ( StdEvent(&event) == TRUE) continue; 	/*event was a std one*/
  		switch(event.what)
  		  {
  		  	case nullEvent:
  		  		break;
  		  	case mouseDown:
  		  	  	DoMouseDownEvent(&event);
  		  	  	break;
  		  	 case keyDown:
  		  	 case autoKey:
  		  	 	theChar = event.message & charCodeMask;
  		  	 	if ( (event.modifiers & cmdKey) != 0 )
  		  	 	  {
  		  	 		DoMenu( MenuKey( theChar ) );
		  	  		HiliteMenu(0);
  		  	 	  }
  		  	 	else
  		  	 	  {
					/* fprintf(info,"a key was hit.\n"); */
				  }
  		  	  	break;
  		  	 case updateEvt:
				DoUpdateEvent(&event);
  		  	  	break;
  		  	 case activateEvt:
				DoActivateEvent(&event);
  		  	  	break;
  		  	 default:
  		  	 	break;
  		  }
  	  }/*for ever*/
  }
  
/*
 *	Deal with mouse down events
 */
DoMouseDownEvent(event_pr)
EventRecord	*event_pr;
  {
  	WindowRecord	*wPtr;
  	Rect dragRect;
  	int windowPart;
  	
  	SetRect(&dragRect,4,24,508,338);
  	
  	windowPart = FindWindow(event_pr->where,&wPtr);
  	switch( windowPart )
  	  {
  	  	case inDesk:
  	  		break;
  	  	case inSysWindow:
  	  		SystemClick(event_pr,wPtr);
  	  		break;
  	  	case inMenuBar:
  	  		DoMenu(MenuSelect(event_pr->where));
  	  		HiliteMenu(0);
  	  		break;
  	  	case inDrag:
  	  		DragWindow( wPtr,event_pr->where, &dragRect );
  	  		break;
  	  	case inContent:
  	  		if ( wPtr != (WindowRecord *)FrontWindow() )
  	  			SelectWindow( wPtr );
  	  		else
  	  			DoContent(wPtr,event_pr);
  	  		break;
  	  	case inGoAway:
  	  	 	DoGoAway ( wPtr,event_pr );
  	  		break;
  	  	case inGrow:
  	  		DoGrow ( wPtr,event_pr );
  	  		break;
		case inZoomIn:
		case inZoomOut:
			DoZoom (wPtr, event_pr, windowPart );
			break;
  		default:
  		  	 break;
  	  }
  }


/*
 *
 */
DoActivateEvent(e_pr)
EventRecord *e_pr;
  {
	WindowRecord *wPtr;
		  
	wPtr = (WindowRecord *) e_pr->message;
	if (e_pr->modifiers & activeFlag )			/*window becoming active*/
	  {

	  }
	else										/*window becoming inactive*/
	  {

	  }
  }
  

/* 
 *
 */
DoUpdateEvent(e_pr)
EventRecord *e_pr;
  {
	WindowRecord *wPtr;
	long index;
	  
	wPtr = (WindowRecord *) e_pr->message;
	index = wPtr->refCon;
	SetPort( wPtr );							/*so that update routine can find out who was it */
	
	if ( index >= 0 && index < MAXHIPS )
		HipsUpdate();	
	else if ( index == COORD_WIND_ID )
		CoordWindUpdate();

  }
  
  
  
/*
 *
 */
DoContent(wPtr,e_pr)
WindowRecord *wPtr;
EventRecord *e_pr;
  {
	long index;	
	
	index = wPtr->refCon;
	SetPort( wPtr );							/*so that update routine can find out who was it */
	
	if ( index >= 0 && index < MAXHIPS )
		HipsContent(e_pr);
  }


/*
 *
 */
DoGoAway (wPtr, ePtr)
WindowRecord *wPtr;
EventRecord *ePtr;
{
	long index;
	
	index = wPtr->refCon;
	
    if ( TrackGoAway (wPtr, ePtr->where  ) == true ) 
    {
		SetPort( wPtr );							/*so that routine can find out who was it */
		
	if ( index >= 0 && index < MAXHIPS )
			HipsClose();
  	}
}
  
/*
 *
 */
DoGrow ( wPtr, ePtr )
WindowRecord *wPtr;
EventRecord *ePtr;
  {
  	Rect sizeRect;
  	long dimens;
  	
  	sizeRect.top = sizeRect.left = 75;
  	sizeRect.bottom = sizeRect.right = 30000;
	dimens = GrowWindow( wPtr, ePtr->where, &sizeRect );
	
	SizeWindow( wPtr, LoWord(dimens), HiWord(dimens) , true );
	
	SetPort( wPtr );
	EraseRect( &(  ((GrafPtr)wPtr)->portRect) );
	InvalRect( &(  ((GrafPtr)wPtr)->portRect) );
  }
  
/*
 *
 */
DoZoom( wPtr, ePtr, windowPart )
WindowRecord *wPtr;
EventRecord *ePtr;
int windowPart;
{

	if( TrackBox(wPtr, ePtr->where, windowPart) )
	{
		SetPort( wPtr );
		EraseRect( &(  ((GrafPtr)wPtr)->portRect) );
		ZoomWindow (wPtr, windowPart, FALSE);

		switch( windowPart )
		{
			case inZoomIn:
			
				break;
			case inZoomOut:
			
				break;
		
		}
	}
}
