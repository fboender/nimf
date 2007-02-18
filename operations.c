/* Operations on files in a panel */
/*
 * Nimf
 * Copyright (C) 2002 Ferry Boender.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <gtk/gtk.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

extern GList *config_nimf;

#include "gtkaddons.h"
#include "panel.h"
#include "operations.h"
#include "file.h"
#include "dialogs.h"
#include "config.h"

void op_start (OPERATION_ operation, GList *filelist, char *src, char *dest, panel *p) {
	int pid;
	int reportpipe[2];
	int actionpipe[2];
	int options = 0;
	char *finaldest = NULL;
	int flags;
	int eventid;
	void *data;
	char *srcpath; /* inclusion of '/' */
	GtkWindow *progress_dialog = NULL;
	GtkWidget *progress_path_label = NULL;
	GtkWidget *progress_bar = NULL;
	char *progress_title = NULL;
	struct stat fileinfo;
	mode_t mode;
	uid_t uid;
	gid_t gid;
//	int options;

	/* Prepare operation */
	switch (operation) {
		case OP_COPY :
			dialog_enterpath ("Copy", "Copy files to", dest, &finaldest);
			gtk_main();
			progress_title = strdup ("Copy");
			break;
		case OP_MOVE :
			dialog_enterpath ("Move","Move files to", dest, &finaldest);
			gtk_main();
			progress_title = strdup ("Move");
			break;
		case OP_MKDIR :
			if (src[strlen(src)] != '/') {
				srcpath = malloc (sizeof(char) * (strlen(src) + 2));
				sprintf (srcpath, "%s/", src);
			} else {
				srcpath = strdup(src);
			}
			
			dialog_enterpath ("Make dir","Make directory", srcpath, &finaldest);
			gtk_main();
			progress_title = strdup ("Make Dir");
			
			free (srcpath);
			break;
		case OP_DEL :
			finaldest = strdup (dest);
			progress_title = strdup ("Delete");
			break;
		case OP_LNK :
			dialog_enterpath ("Link","Link files in", dest, &finaldest);
			progress_title = strdup ("Link");
			gtk_main();
			break;
		case OP_PERM :
			srcpath = malloc (sizeof(char) * (strlen(src) + strlen(filelist->data) + 2));
			sprintf (srcpath, "%s/%s", src, (char *)filelist->data);
			stat(srcpath, &fileinfo);
			mode = fileinfo.st_mode;
			if (dialog_permissions (mode, &mode, &options) == 0) {
				finaldest = strdup(src);
			} else {
				finaldest = NULL;
			}

			break;
			
		case OP_OWN :
			srcpath = malloc (sizeof(char) * (strlen(src) + strlen(filelist->data) + 2));
			sprintf (srcpath, "%s/%s", src, (char *)filelist->data);
			stat(srcpath, &fileinfo);
			uid = fileinfo.st_uid;
			gid = fileinfo.st_gid;
			if (dialog_ownership (uid, gid, &uid, &gid, &options) == 0) {
				finaldest = strdup(src);
			} else {
				finaldest = NULL;
			}
			break;
		default:
			perror ("op_start: Unknow operation passed\n");
			break;
	}
	
	if (finaldest != NULL) {
		/* FIXME: Comment on the next line is incorrect? */
		/* action canceled */

		p->operation_busy = 1;
		p->next->operation_busy = 1;
		
		panel_disable (p);
		panel_disable (p->next);

		pipe (reportpipe);
		pipe (actionpipe);
		
		progress_path_label = gtk_label_new ("");
		progress_bar = gtk_progress_bar_new();
		progress_dialog = dialog_progress(progress_title, progress_path_label, progress_bar);

		data = gtk_param_pack ("iipppp", reportpipe[0], actionpipe[1], (void *)p, (void *)progress_path_label, (void *)progress_dialog, (void *)progress_bar);
		eventid = gtk_timeout_add (100, (GtkFunction) op_progress, (gpointer) data);
		
		if ((pid = fork()) == 0) {
			close (reportpipe[0]);
			close (actionpipe[1]);

			switch (operation) {
				case OP_COPY  :
					filelist_copy (actionpipe[0], reportpipe[1], filelist, src, finaldest, options);
					break;
				case OP_MOVE : 
					filelist_move (actionpipe[0], reportpipe[1], filelist, src, finaldest, options);
					break;
				case OP_MKDIR :
					filelist_mkdir (actionpipe[0], reportpipe[1], finaldest, src, options);
					break;
				case OP_DEL :
					filelist_del (actionpipe[0], reportpipe[1], filelist, src, options);
					break;
				case OP_LNK :
					filelist_lnk (actionpipe[0], reportpipe[1], filelist, src, finaldest, options);
					break;
				case OP_PERM :
					filelist_chmod (actionpipe[0], reportpipe[1], filelist, src, mode, options);
					break;
				case OP_OWN :
					filelist_chown (actionpipe[0], reportpipe[1], filelist, src, uid, gid, options);
					break;
				default:
					perror ("op_start: Unknow operation passed");
			}
			
			close (reportpipe[0]);
			close (reportpipe[1]);
			
			close (actionpipe[0]);
			close (actionpipe[1]);
			
			_exit (0);
		}
	
	    /* Set pipe read to non-blocking */
	    flags = fcntl(reportpipe[0], F_GETFL, 0);
	    fcntl(reportpipe[0], F_SETFL, flags | O_NONBLOCK);
	}    

	free (progress_title);
	free (finaldest);
}

int op_progress (void *data) {
	int reporttype;
	int reportpipe;
	int actionpipe;
	panel *p;
	int readresult;
	int pd;
	GtkWidget *progress_path_label;
	GtkWindow *progress_dialog;
	GtkProgress *progress_bar;
	
	gtk_param_unpack (data, "iipppp", &reportpipe, &actionpipe, &p, &progress_path_label, &progress_dialog, &progress_bar);
	
	pd = (int) reportpipe; 
	while ( (readresult = read(pd, &reporttype, sizeof(int))) > 0) {
		if (readresult > 0) { 
			if (reporttype == 0) { /* Operation completed */
				free (data);
				wait (NULL);
				
				panel_selectnone (p);
				panel_refresh (p->next, 1);
				panel_refresh (p, 1);
				
				gtk_widget_destroy (GTK_WIDGET(progress_dialog));

				panel_enable (p->next);
				panel_enable (p);
				
				p->operation_busy = 0;
				p->next->operation_busy = 0;

				return (0);
			} else 
			if (reporttype == 1) { /* Operation progress report */
				int percent;
				char *path;
				
				op_progress_readprogress (pd, &percent, &path);
				
				gtk_label_set_text(GTK_LABEL(progress_path_label), path);
				gtk_entry_set_text (GTK_ENTRY(p->info), path);
				gtk_progress_set_percentage (GTK_PROGRESS(progress_bar), (double)(percent)/100);
				
				free (path);
			} else
			if (reporttype == 2) { /* Error */
				int errornr;
				int action = 0;
				char *path;
					
				op_progress_readerror (pd, &errornr, &path);
				dialog_report_error ("Error operating on\n", path, strerror (errornr), &action);
				write (actionpipe, &action, sizeof(int));
				
				free (path);
			}
		}
	}
	
	return (1);
}

void op_progress_readerror (int reportpipe, int *errornr, char **path) {
	char p[1024];
	int i = 0;
	
	read (reportpipe, errornr, sizeof(int));
	for (;;i++) {
		read(reportpipe, &(p[i]), sizeof(char));
		if (p[i] == '\0') break;
	}

	*path = strdup(p);
}

void op_progress_readprogress (int reportpipe, int *percent, char **path) {
	char p[1024];
	int i = 0;
	
	read (reportpipe, percent, sizeof(int));
	for (;;i++) {
		read(reportpipe, &(p[i]), sizeof(char));
		if (p[i] == '\0') break;
	}

	*path = strdup(p);
}

void op_edit (GList *filelist, char *path) {
	char *fullpath;
	char *editor;
	char *command;
	
	fullpath = file_fullpath(path, filelist->data);
	
	/* Check if editor defined */
	if ((editor = config_find_value(config_nimf, "editor", NULL)) == NULL) {
		dialog_error ("No editor is assigned.");
		return;
	}
	
	command = malloc (sizeof(char) * (strlen(editor) + strlen(fullpath) + 2));
	sprintf (command, editor, fullpath);
	
	system (command); /* Yikes! */
	
	free (command);
	free (fullpath);
}
