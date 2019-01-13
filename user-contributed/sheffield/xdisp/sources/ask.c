/***********************************************
 * ask & tell - general user interface routines
 ***********************************************/

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xutil.h>
#include <Xol/OpenLook.h>
#include <Xol/TextField.h>
#include <X11/Shell.h>
#include <xdisp.h>

static void (*ask_var_proc)();

void ask_yes_callback();

void ask(title,message,defname,proc)
  char *title;
  char *message;
  char *defname;
  void (*proc)();
{    
    XtVaSetValues(a_caption,
		XtNlabel,message,
		NULL);
    XtVaSetValues(a_text,
		XtNstring,defname,
		NULL);
    XtVaSetValues(a_popup,
		XtNtitle,title,
		NULL);

    ask_var_proc = proc;

    XtPopup(a_popup,XtGrabNone);

}


void ask_verification_callback(w,client_data,call_data)
  Widget	w;
  XtPointer	client_data;
  XtPointer	call_data;
{
    OlTextFieldVerify *tfv = (OlTextFieldVerify *)call_data;

    if (tfv->reason == OlTextFieldReturn) {
	ask_yes_callback(w,client_data,call_data);
    	XtPopdown(a_popup);
	}
}


void ask_yes_callback(w,client_data,call_data)
  Widget	w;
  XtPointer	client_data;
  XtPointer	call_data;
{
    String reply;
    int len;

    reply = OlTextFieldGetString(a_text,&len);

    (*ask_var_proc)(reply);
}

void ask_no_callback(w,client_data,call_data)
  Widget	w;
  XtPointer	client_data;
  XtPointer	call_data;
{
}
