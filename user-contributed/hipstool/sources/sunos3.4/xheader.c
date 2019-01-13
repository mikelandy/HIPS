/* xheader.c
 * Max Rible
 *
 * Extended header read/write functions.
 */

#include "hipstool.h"

void
xheader_to_log(hips)
     struct header *hips;
{
    char *begin;
    int plumbing[2],two=2;
    FILE *toilet, *sewer;
    extern char *XHget();

    if (findparam(hips,HEADER_LOG_DATA_NAME) == NULLPAR)
	return;
    getparam(hips,HEADER_LOG_DATA_NAME,PFASCII,&two,&begin);

    /* The most elegant way to read this thing is as a stream,
     * so I'm going to open a pipe, fwrite the string down it,
     * and then read from the other end of the pipe.
     * We'll make provision for buffering if there's more than
     * 4K if data, but I doubt we'll have that much.
     */
    pipe(plumbing);
    toilet = fdopen(plumbing[1], "w");
    sewer = fdopen(plumbing[0], "r");
    Fwrite(begin, char, strlen(begin), toilet);
    fflush(toilet);
    fclose(toilet);
    close(plumbing[1]);

    load_log(sewer);

    fclose(sewer); close(plumbing[0]);
    Update_info("Valid log present.");
}

void
xheader_to_comment(hips)
     struct header *hips;
{
    int i, size,two=2;

    if (findparam(hips,HEADER_COMMENT_DATA_NAME) == NULLPAR)
	return;
    getparam(hips,HEADER_COMMENT_DATA_NAME,PFASCII,&two,&header_comment);

    size = strlen(header_comment);

    for(i = 0; i < size; i++)
	if(header_comment[i] == '\177') header_comment[i] = '\n';

    save_menu_funcs[SAVE_LOGGED_IMAGE].active = 1;
    save_menu_funcs[SAVE_COMMENT_FILE].active = 1;
    update_save_menu();
}

void
log_to_xheader(hips)
     struct header *hips;
{
    int size = 0, plumbing[2];
    FILE *toilet;
    char *buffer;

    if(actions == NULLOG) return;

    pipe(plumbing);
    toilet = fdopen(plumbing[1], "w");

    size = save_log(toilet, '\177');

    fflush(toilet);

    buffer = (char *) malloc((unsigned)size+1);
    read(plumbing[0], buffer, size);
    buffer[size] = '\0';
    fclose(toilet);
    close(plumbing[0]); close(plumbing[1]);
    setparam(hips,HEADER_LOG_DATA_NAME,PFASCII,buffer,strlen(buffer)+1);
}

void
comment_to_xheader(hips)
     struct header *hips;
{
    int i, size;

    if(header_comment == NULL) return;

    size = strlen(header_comment);

    for(i = 0; i < size; i++)
	if(header_comment[i] == '\n') header_comment[i] = '\177';

    setparam(hips,HEADER_COMMENT_DATA_NAME,PFASCII,header_comment,
	strlen(header_comment)+1);

    for(i = 0; i < size; i++)
	if(header_comment[i] == '\177') header_comment[i] = '\n';
}
