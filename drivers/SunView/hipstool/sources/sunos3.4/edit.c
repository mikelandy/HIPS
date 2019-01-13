/* edit.c
 * Max Rible
 *
 * Editing of modes for HIPStool.  Shouldn't affect current function.
 */

#include "hipstool.h"

static char string[100];
static char null = '\0';

void
log_edit()
{
    int x;
    char cmd[10];

    strcpy(string, getstring(io.command));
    if(strlen(string) == 0) { 
	display_logged_actions(DRAW_IN_PIXWIN, ON); 
	return;
    }

    sscanf(string, "%s %d", cmd, &x);
    if(strncmp(cmd, "disable", strlen(cmd)) == 0) {
	if(warp_action(x, WARP_ABLE, OFF) == OFF) {
	    sprintf(string, "Action %d already disabled!", x);
	    Update_info(string);
	    return;
	}
	refresh(&base.winfo, NULL);
	display_logged_actions(DRAW_IN_PIXWIN, ON);
	sprintf(string, "Disabled action %d.", x);
	Update_info(string);
    } else if(strncmp(cmd, "enable", strlen(cmd)) == 0) {
	if(warp_action(x, WARP_ABLE, ON) == ON) {
	    sprintf(string, "Action %d already enabled!", x);
	    Update_info(string);
	    return;
	}
	refresh(&base.winfo, NULL);
	display_logged_actions(DRAW_IN_PIXWIN, ON);
	sprintf(string, "Enabled action %d.", x);
	Update_info(string);
    } else if((strncmp(cmd, "remove", strlen(cmd)) == 0) ||
	      (strncmp(cmd, "delete", strlen(cmd)) == 0)) {
	if(warp_action(x, WARP_DELETE, ON) == OFF) {
	    sprintf(string, "Action %d doesn't exist!", x);
	    Update_info(string);
	    return;
	} else {
	    sprintf(string, "Deleted action %d.", x);
	    Update_info(string);
	}
	refresh(&base.winfo, NULL);
	display_logged_actions(DRAW_IN_PIXWIN, ON);
    } else if(strncmp(cmd, "select", strlen(cmd)) == 0) {
	refresh(&base.winfo, NULL);
	warp_action(x, WARP_SELECT, ON);
    } else if(strncmp(cmd, "show", strlen(cmd)) == 0) {
	refresh(&base.winfo, NULL);
	display_logged_actions(DRAW_IN_PIXWIN, TEST);
    } else {
	Update_info("I can't handle that, man.");
    }
    putstring(io.command, &null);
}

/* ARGSUSED */
void
colormap_edit()
{
    char cmd[10];
    int r, g, b, start, end, num, val, range, i;
    double gammaval, delta;

    strcpy(string, getstring(io.command));
    sscanf(string, "%s", cmd);
    if(strncmp(cmd, "standout", strlen(cmd)) == 0) {
	sscanf(string, "%*s %d %d %d", &r, &g, &b);
	red[STANDOUT] = r;	green[STANDOUT] = g;	blue[STANDOUT] = b;
	colormap256(&base.winfo, red, green, blue);
	Update_info("Congratulations on your new standout color.");
    } else if(strncmp(cmd, "scale", strlen(cmd)) == 0) {
	num = sscanf(string, "%*s %d %d", &start, &end);
	if(num != 2) { start = 0; end = 255; }
	delta = (double) (end - start);
	for(i = 0; i < start; i++)
	    base.user_lut[i] = tr[0];
	for(i = start; i < end; i++)
	    base.user_lut[i] = tr[SCALE((double)(i - start)/delta)];
	for(i = end; i < base.extremes[1]; i++)
	    base.user_lut[i] = tr[255];
	pixrefresh(&base);
	Update_info("Image rescaled.");
    } else if(strncmp(cmd, "window", strlen(cmd)) == 0) {
	num = sscanf(string, "%*s %d %d", &val, &range);
	if(num == 1) range = 256;
	start = val - range/2;
	end = val + range/2 - (range & 1);
	delta = (double) (end - start);
	for(i = 0; i < start; i++)
	    base.user_lut[i] = tr[0];
	for(i = start; i < end; i++)
	    base.user_lut[i] = tr[SCALE((double)(i - start)/delta)];
	for(i = end; i < base.extremes[1]; i++)
	    base.user_lut[i] = tr[255];
	pixrefresh(&base);
	Update_info("Image rescaled.");
    } else if(strncmp(cmd, "gamma", strlen(cmd)) == 0) {
	num = sscanf(string, "%*s %lf", &gammaval);
	if(num != 1) gammaval = 1.0;
	for(i = STANDOUT+1; i < SUN_WHITE; i++) {
	    red[i] = green[i] = blue[i] = 
		(int) (255.0 * pow((double)i / 255.0, 1.0/gammaval));
	}
	colormap256(&base.winfo, red, green, blue);
	Update_info("The colormap is transformed by gamma radiation...");
    } else {
	Update_info("I can't handle that, man.");
    }
    putstring(io.command, &null);
}

/* ARGSUSED */
void
comment_edit()
{
    char comment[100];
    int len;

    strcpy(comment, getstring(io.comment));
    len = strlen(comment);
    comment[len++] = '\n'; comment[len] = '\0';

    if(header_comment != NULL)
	header_comment = 
	    Realloc(header_comment, strlen(header_comment) + len, char);
    else
	header_comment =
	    Calloc(len + 1, char);

    strcpy(header_comment + strlen(header_comment), comment);

    save_menu_funcs[SAVE_COMMENT_FILE].active = 1;
    update_save_menu();
}
