/*
 *	list.c - for use with SEGAL
 *
 *	By Bryan Skene
 *
 */

#include "common.h"

/***************************************/
recall_list()
{
	char *get_non_comment();

	FILE *fp;
        char s[MAXPATHLEN], filename[MAXPATHLEN];
	 
	strcpy(filename, (char *) xv_get(List_pop_list->text_l_fname,
		PANEL_VALUE, NULL));

	if(strcmp(&filename[strlen(filename) - 4 - 1], ".list") != 0) {
		/* append ".list" to filename */
		strcat(filename, ".list");
		xv_set(List_pop_list->text_l_fname,
			PANEL_VALUE, filename,
			NULL);
	}

	if((fp = fopen(filename, "r")) == NULL) {
		prgmerr(0, "Cannot find list");
		return;
	}

	/* image directory */
	if(strcmp(strcpy(s, get_non_comment(fp)), "**EOF") == 0) {
		fclose(fp);
		return;
	}
	xv_set(List_pop_list->text_i_dname,
		PANEL_VALUE, s,
		NULL);

	/* image filename */
	if(strcmp(strcpy(s, get_non_comment(fp)), "**EOF") == 0) {
		fclose(fp);
		return;
	}
	xv_set(List_pop_list->text_i_fname,
		PANEL_VALUE, s,
		NULL);

	/* clear out the mask directory and filenames first */
	xv_set(List_pop_list->text_m_dname,
		PANEL_VALUE, "<None>",
		NULL);
	xv_set(List_pop_list->text_m1_fname,
		PANEL_VALUE, "<None>",
		NULL);
	xv_set(List_pop_list->text_m2_fname,
		PANEL_VALUE, "<None>",
		NULL);
	xv_set(List_pop_list->text_m3_fname,
		PANEL_VALUE, "<None>",
		NULL);
	xv_set(List_pop_list->text_m4_fname,
		PANEL_VALUE, "<None>",
		NULL);
	xv_set(List_pop_list->text_m5_fname,
		PANEL_VALUE, "<None>",
		NULL);

	/* masks directory */
	if(strcmp(strcpy(s, get_non_comment(fp)), "**EOF") == 0) {
		fclose(fp);
		return;
	}
	xv_set(List_pop_list->text_m_dname,
		PANEL_VALUE, s,
		NULL);

	/* edit mask filename */
	if(strcmp(strcpy(s, get_non_comment(fp)), "**EOF") == 0) {
		fclose(fp);
		return;
	}
	xv_set(List_pop_list->text_m1_fname,
		PANEL_VALUE, s,
		NULL);

	/* other mask filenames */
	if(strcmp(strcpy(s, get_non_comment(fp)), "**EOF") == 0) {
		fclose(fp);
		return;
	}
	xv_set(List_pop_list->text_m2_fname,
		PANEL_VALUE, s,
		NULL);

	if(strcmp(strcpy(s, get_non_comment(fp)), "**EOF") == 0) {
		fclose(fp);
		return;
	}
	xv_set(List_pop_list->text_m3_fname,
		PANEL_VALUE, s,
		NULL);

	if(strcmp(strcpy(s, get_non_comment(fp)), "**EOF") == 0) {
		fclose(fp);
		return;
	}
	xv_set(List_pop_list->text_m4_fname,
		PANEL_VALUE, s,
		NULL);

	if(strcmp(strcpy(s, get_non_comment(fp)), "**EOF") == 0) {
		fclose(fp);
		return;
	}
	xv_set(List_pop_list->text_m5_fname,
		PANEL_VALUE, s,
		NULL);

	fclose(fp);
}

/***************************************/
store_list()
{
	FILE *fp;
        char s[MAXPATHLEN], filename[MAXPATHLEN];
	 
	strcpy(filename, (char *) xv_get(List_pop_list->text_l_fname,
		PANEL_VALUE, NULL));
	if((fp = fopen(filename, "w")) == NULL) {
		prgmerr(0, "Cannot create list");
		return;
	}

	/* list description */
	fprintf(fp, "**\n\tList for use with SEGAL 3D - by Bryan Skene\n**\n");

	fprintf(fp, "** Name: %s **\n", filename);

	fprintf(fp, "** Description: %s **\n",
		(char *) xv_get(List_pop_list->text_description,
			PANEL_VALUE, NULL));

	/* image directory */
	fprintf(fp, "** Image Directory **\n%s\n",
		(char *) xv_get(List_pop_list->text_i_dname,
			PANEL_VALUE, NULL));

	/* image filename */
	fprintf(fp, "** Image Filename **\n%s\n",
		(char *) xv_get(List_pop_list->text_i_fname,
			PANEL_VALUE, NULL));

	/* masks directory */
	fprintf(fp, "** Directory of all Masks **\n%s\n",
		(char *) xv_get(List_pop_list->text_m_dname,
			PANEL_VALUE, NULL));

	/* edit mask */
	fprintf(fp, "** Edit Mask **\n%s\n",
		(char *) xv_get(List_pop_list->text_m1_fname,
			PANEL_VALUE, NULL));

	/* other masks */
	fprintf(fp, "** Other Masks (optional) **\n%s\n",
		(char *) xv_get(List_pop_list->text_m2_fname,
			PANEL_VALUE, NULL));

	fprintf(fp, "%s\n",
		(char *) xv_get(List_pop_list->text_m3_fname,
			PANEL_VALUE, NULL));

	fprintf(fp, "%s\n",
		(char *) xv_get(List_pop_list->text_m4_fname,
			PANEL_VALUE, NULL));

	fprintf(fp, "%s\n",
		(char *) xv_get(List_pop_list->text_m5_fname,
			PANEL_VALUE, NULL));
	fclose(fp);
}
