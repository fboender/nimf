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

/* Enumaration types for operation_start */
typedef enum {
	OP_COPY,
	OP_MOVE,
	OP_MKDIR,
	OP_DEL,
	OP_LNK,
	OP_PERM,
	OP_OWN
} OPERATION_;

typedef struct popup_mkdir { 
	panel *p;
	GtkWidget *entrywindow;
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *entry;
} popup_mkdir;

typedef struct popup_copy { 
	panel *src, *dest;
	GtkWidget *entrywindow;
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *entry;
} popup_copy;

void op_start (OPERATION_ operation, GList *filelist, char *src, char *dest, panel *p);
int op_progress (void *data);
void op_progress_readerror (int reportpipe, int *errornr, char **path);
void op_progress_readprogress (int reportpipe, int *percent, char **path);
void op_action_send (int actionpipe, int action);
void op_edit (GList *filelist, char *path);
