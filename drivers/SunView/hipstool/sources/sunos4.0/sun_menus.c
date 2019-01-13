/* sun_menus.c
 * Max Rible
 *
 * Menus to control HIPStool.
 * 
 * The procedures all pass information through the "generic" variable
 * for debugging purposes.  None of this will be heavily used enough
 * to make it matter whether we assign directly or not.
 */

#include "hipstool.h"

#ifdef SUNTOOLS
struct function_menu_returned_information menu_info = {
    PRIMIT_BOX, 1, FUNC_BOX, 0
    };

#define Return(x) return((caddr_t)(x))

#define Enter(x) fprintf(stderr, "entering %s. ", x); fflush(stderr)
#define Leave(x) fprintf(stderr, "leaving %s. ", x); fflush(stderr)

static Menu line_modes,
    dimensions[3], trace_menu, histo_menu, angle_modes,
    primit_menu, function_menu, edit_menu;

static Menu dimension_generate();

static caddr_t line_proc(), 
    trace_proc(), histo_proc(), angle_proc(),
    primit_proc(), function_proc(), edit_proc(),
    wakeup();

static caddr_t generic;

/* The dimensions[] array should always have, on query, a value
 * appropriate to "enum primit".
 * line_modes takes care of setting the mode and still returns
 * the appropriate value.
 * The primits menu is just a way of selecting dimensions[].
 * The functions menu should return an enum function_index, and
 * the edit menu should return something appropriate.
 */

/* It's annoying to have to duplicate things like line_modes to
 * angle_modes and dimensions[1] to trace_menu and dimensions[2]
 * to histo_menu; I *hate* sunfools menus.  If any other opportunity
 * to redesign this to be more elegant comes up, it should happen.
 */

enum dimension {
    POINT, LENGTH, AREA, LINE_MODES,
};

void
create_menus()
{
    line_modes = dimension_generate(LINE_MODES, line_proc);
    angle_modes = dimension_generate(LINE_MODES, angle_proc);

    dimensions[0] = dimension_generate(POINT, NULL);
    dimensions[1] = dimension_generate(LENGTH, NULL);
    dimensions[2] = dimension_generate(AREA, NULL);
    trace_menu = dimension_generate(LENGTH, trace_proc);
    histo_menu = dimension_generate(AREA, histo_proc);

    primit_menu =
	menu_create(MENU_PULLRIGHT_ITEM, "Points", dimensions[0],
		    MENU_PULLRIGHT_ITEM, "Lengths", dimensions[1],
		    MENU_PULLRIGHT_ITEM, "Areas", dimensions[2],
		    MENU_NOTIFY_PROC, primit_proc,
		    0);

    function_menu =
	menu_create(MENU_STRING_ITEM, "Box", FUNC_BOX,
		    MENU_PULLRIGHT_ITEM, "Trace", trace_menu,
		    MENU_PULLRIGHT_ITEM, "Histogram", histo_menu,
		    MENU_STRING_ITEM, "Distance", FUNC_DISTANCE,
		    MENU_PULLRIGHT_ITEM, "Angle measure", angle_modes,
		    MENU_NOTIFY_PROC, function_proc,
		    0);

    edit_menu = 
	menu_create(MENU_STRING_ITEM, "Edit log", EDIT_LOG,
		    MENU_STRING_ITEM, "Abuse colormap", EDIT_COLORMAP,
		    MENU_STRING_ITEM, "Comment file", EDIT_HEADERTEXT,
		    MENU_NOTIFY_PROC, edit_proc,
		    0);

    io.rootmenu = 
	menu_create(MENU_PULLRIGHT_ITEM, "Primitives", primit_menu,
		    MENU_PULLRIGHT_ITEM, "Functions", function_menu,
		    MENU_PULLRIGHT_ITEM, "Editing", edit_menu,
		    MENU_NOTIFY_PROC, wakeup,
		    0);
}

static Menu
dimension_generate(dim, procedure)
     enum dimension dim;
     caddr_t (*procedure)();
{
    Menu temp;

    /* Either default or argument. */
    if(procedure == NULL) procedure = menu_return_value;

    switch(dim) {
    case POINT:
	temp =
	    menu_create(MENU_STRING_ITEM, "point", PRIMIT_POINT,
			MENU_STRING_ITEM, "text", PRIMIT_TEXT,
			0);
	return(temp);
    case LENGTH:
	/* Note:  preassumes you've generated line_modes already. */
	temp = 
	    menu_create(MENU_PULLRIGHT_ITEM, "line", line_modes,
			MENU_STRING_ITEM, "open spline", PRIMIT_OPEN_SPLINE,
			MENU_NOTIFY_PROC, procedure,
			0);
	return(temp);
    case AREA:
	temp = 
	    menu_create(MENU_STRING_ITEM, "box", PRIMIT_BOX,
			MENU_STRING_ITEM, "polygon", PRIMIT_POLYGON,
			MENU_STRING_ITEM, "closed spline",PRIMIT_CLOSED_SPLINE,
			MENU_NOTIFY_PROC, procedure,
			0);
	return(temp);
    case LINE_MODES:
	temp = 
	    menu_create(MENU_ITEM,
			MENU_STRING, "click and click",
			MENU_VALUE, CLICK_AND_CLICK,
			MENU_NOTIFY_PROC, procedure,
			0,
			MENU_ITEM,
			MENU_STRING, "click and drag",
			MENU_VALUE, CLICK_AND_DRAG,
			MENU_NOTIFY_PROC, procedure,
			0,
			MENU_ITEM,
			MENU_STRING, "least squares fit",
			MENU_VALUE, LEAST_SQUARES_FIT,
			MENU_NOTIFY_PROC, procedure,
			0,
			0);
	return(temp);
    }
    return(NULL);
}

static caddr_t
line_proc(menu, item)
     Menu menu;
     Menu_item item;
{
    generic = menu_get(item, MENU_VALUE);

    change_line_drawing_mode((enum line_select_mode) generic);
    menu_set(menu, MENU_DEFAULT_ITEM, item, 0);

    Return(PRIMIT_LINE);
}

static caddr_t
angle_proc(menu, item)
     Menu menu;
     Menu_item item;
{
    generic = menu_get(item, MENU_VALUE);

    change_line_drawing_mode((enum line_select_mode) generic);
    menu_set(menu, MENU_DEFAULT_ITEM, item, 0);

    Return(FUNC_ANGLE);
}

static caddr_t
trace_proc(menu, item)
     Menu menu;
     Menu_item item;
{
    generic = menu_get(item, MENU_VALUE);

    menu_info.breed = (enum primit) generic;
    menu_info.num = 1;
    menu_set(menu, MENU_DEFAULT_ITEM, item, 0);

    Return(FUNC_TRACE);
}

static caddr_t
histo_proc(menu, item)
     Menu menu;
     Menu_item item;
{
    generic = menu_get(item, MENU_VALUE);

    menu_info.breed = (enum primit) generic;
    menu_info.num = 1;
    menu_set(menu, MENU_DEFAULT_ITEM, item, 0);

    Return(FUNC_HISTO);
}

/* ARGSUSED */
static caddr_t
primit_proc(menu, item)
     Menu menu;
     Menu_item item;
{
    generic = menu_get(item, MENU_VALUE);

    menu_info.breed = (enum primit) generic;
    menu_info.num = 1;

    Return(RETURN_PRIMIT);
}

/* ARGSUSED */
static caddr_t
function_proc(menu, item)
     Menu menu;
     Menu_item item;
{
    generic = menu_get(item, MENU_VALUE);
    
    menu_info.function = (enum function_index) generic;

    Return(RETURN_FUNCTION);
}

/* ARGSUSED */
static caddr_t
edit_proc(menu, item)
     Menu menu;
     Menu_item item;
{
    generic = menu_get(item, MENU_VALUE);

    menu_info.edit = (enum edit_mode) generic;

    Return(RETURN_EDIT);
}

/* ARGSUSED */
static caddr_t
wakeup(menu, item)
     Menu menu;
     Menu_item item;
{
    generic = menu_get(item, MENU_VALUE);

    Return(generic);
}
#endif
