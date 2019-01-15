/*
 *	timer.c - for use with SEGAL
 *
 *	By Bryan Skene
 *
 *	Usage:
 *	1. strcpy a message into timer.message
 *	2. call begin timer()
 *	3. at various intervals, call set_timer(percentage_done)
 *	4. when 100% done, call end_timer()
 */

#include "common.h"

/*****************************************/
void
begin_itimer(sec, usec)
int sec, usec;
{
	itimer.it_value.tv_sec = sec;
	itimer.it_interval.tv_sec = sec;
	itimer.it_value.tv_usec = usec;
	itimer.it_interval.tv_usec = usec;
}

/******************************************/
LOGIC
begin_timer()
{
	unsigned long standout();
	void set_timer();

	if(timer.semaphore == LOCKED) return(FALSE);

	timer.semaphore = LOCKED;

	xv_set(View_pop_timer->pop_timer,
		XV_SHOW, TRUE,
		NULL);
	xv_set(View_pop_timer->msg_timer,
		PANEL_LABEL_STRING, timer.message,
		NULL);

	timer.width = (int) xv_get(View_pop_timer->canv_timer,
		XV_WIDTH, NULL);
	timer.height = (int) xv_get(View_pop_timer->canv_timer,
		XV_HEIGHT, NULL);

	XSetForeground(display, gc, standout(CWHITE));
	XFillRectangle(display, timer.xid, gc,
		0, 0,
		timer.width, timer.height);

	set_timer(0.0);

	return(TRUE);
}

/******************************************/
void
set_timer(percent)
float percent;
{
	unsigned long standout();

	XSetForeground(display, gc, standout(PURPLE));
	XFillRectangle(display, timer.xid, gc,
		0, 0,
		(int) ((float) timer.width * percent), timer.height);
}

/******************************************/
void
end_timer()
{
	xv_set(View_pop_timer->pop_timer,
		XV_SHOW, FALSE,
		NULL);

	timer.semaphore = UNLOCKED;
}

/******************************************/
void
timer_resize_proc()
{
	timer.width = (int) xv_get(View_pop_timer->pop_timer,
		XV_WIDTH, NULL);

	xv_set(View_pop_timer->canv_timer,
		XV_WIDTH, timer.width,
		NULL);
}

/******************************************/
void
enq_bg_job(job, arg)
int job, arg;
{
/* Enter a job into the background queue because there is already a job
 * running in the bg.
 */
	timer.queue[timer.qrear].job = job;
	timer.queue[timer.qrear].arg = arg;

	timer.qrear++;
	if(timer.qrear == MAX_JOBS) timer.qrear = 0;
}

/******************************************/
void
deq_bg_job()
{
/* If there is a bg job in the queue, setup the appropriate values and then
 * invoke notify_set_itimer_func().  If not, turn off the bg job stuff.
 * Called at the completion of a bg job.
 */
	Notify_value bg_load_image_frame();
	Notify_value bg_load_mask_frame();
	Notify_value bg_save_image_frame();
	Notify_value bg_save_mask_frame();
	LOGIC begin_timer();

	if(timer.qfront == timer.qrear) return;

	begin_itimer(INTERVAL_SEC, INTERVAL_uSEC);

	switch(timer.queue[timer.qfront].job) {
	case JOB_LOAD_IMAGE :
		segal.bg_i = segal.f1;
		begin_itimer(INTERVAL_SEC, INTERVAL_uSEC);
		sprintf(timer.message, "Loading frames: %s ...", img.fname);
		notify_set_itimer_func(File_pop_load_image->pop_load_image,
			bg_load_image_frame, ITIMER_REAL, &itimer, NULL);
		break;
	case JOB_LOAD_MASK :
		segal.new_m = timer.queue[timer.qfront].arg;
		m[segal.new_m].f = 0;
		begin_itimer(INTERVAL_SEC, INTERVAL_uSEC);
		sprintf(timer.message, "Loading frames: %s ...", m[segal.new_m].fname);
		notify_set_itimer_func(File_pop_load_mask->pop_load_mask,
			bg_load_mask_frame, ITIMER_REAL, &itimer, NULL);
		break;
	case JOB_SAVE_IMAGE :
		segal.bg_i = 0;
		begin_itimer(INTERVAL_SEC, INTERVAL_uSEC);
		sprintf(timer.message, "Saving frames: %s ...", img.fname);
		notify_set_itimer_func(File_pop_load_image->pop_load_image,
			bg_save_image_frame, ITIMER_REAL, &itimer, NULL);
		break;
	case JOB_SAVE_MASK :
		segal.new_m = timer.queue[timer.qfront].arg;
		m[segal.new_m].f = 0;
		begin_itimer(INTERVAL_SEC, INTERVAL_uSEC);
		sprintf(timer.message, "Saving frames: %s ...", m[segal.new_m].fname);
		notify_set_itimer_func(File_pop_load_mask->pop_load_mask,
			bg_save_mask_frame, ITIMER_REAL, &itimer, NULL);
		break;
	case JOB_QUIT :
		xv_set(View_win->win,
			FRAME_NO_CONFIRM, TRUE,
			NULL);

		xv_destroy_safe(View_win->win);
		exit(0);
	default :
		break;

	}

	timer.qfront++;
	if(timer.qfront == MAX_JOBS) timer.qfront = 0;

	if(!begin_timer()) prgmerr(0, "Job queue list not completed");
}
