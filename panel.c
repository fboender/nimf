/* panel handling. 
 *
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
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <fnmatch.h>

#include "file.h"
#include "panel.h"
#include "operations.h"
#include "dialogs.h"
#include "config.h"
#include "bookmarks.h"
#include "gtkaddons.h"
#include "cmdline.h"
#include "context.h"

extern panel *panels[2]; /* defined in nimf.c */
extern int panel_current; /* defined in nimf.c */
extern cmdline_ *cmdline; /* defined in nimf.c */
extern GList *config_ext;
extern GList *config_keys;
extern GList *bookmarks;
extern char *key_action_mapping[]; /* nimf.c */

void panel_ev_list_keypress (GtkCList *widget, GdkEventKey *event, panel *p) {
	int i;
	char *key_pressed;
	char *key = NULL;
	action_ action = 0;

	/* Serialize the key so it's easier to process it. Serializing means that
	 * modifiers and the pressed keys are represented in one char* */
	key_pressed = gtk_key_serialize (event);
		
	/* Jump search */
	if (
			strlen(key_pressed) == 1 || 
			event->keyval == GDK_BackSpace ||
			event->keyval == GDK_period
		) {
		/* Single key or backspace pressed: jumpsearch */
		panel_list_jump (p, event);
	} else {
		/* Any other (combination of) keys cancel jumpsearch */
		if (p->jumpfile != NULL) {
			free (p->jumpfile);
			p->jumpfile = NULL;
		}
	}
	
	i = 0;
	action = key_action_map (config_keys, key_pressed);
	
	if (action != -1 && p->jumpfile == NULL) {	
		switch (action) {
			case operation_rename: op_start(OP_MOVE, panel_getselectedfilelist(p),	p->path, panel_getselectedfilelist(p)->data, p); break;
			case operation_help: panel_refresh(p, 1); break;
			case operation_view: printf ("unimplemented\n"); break;
			case operation_edit: op_edit(panel_getselectedfilelist(p), p->path); break;
			case operation_copy: op_start(OP_COPY, panel_getselectedfilelist(p), p->path, p->next->path, p); break;
			case operation_move: op_start(OP_MOVE, panel_getselectedfilelist(p), p->path, p->next->path, p); break;
			case operation_mkdir: op_start(OP_MKDIR, panel_getselectedfilelist(p), p->path, p->next->path, p); break;
			case operation_delete: op_start(OP_DEL, panel_getselectedfilelist(p), p->path, p->next->path, p); break;
			case operation_symlink: op_start(OP_LNK, panel_getselectedfilelist(p), p->path, p->next->path, p); break;
			case operation_permissions: op_start(OP_PERM, panel_getselectedfilelist(p), p->path, p->next->path, p); break;
			case operation_ownership: op_start(OP_OWN, panel_getselectedfilelist(p), p->path, p->next->path, p); break;
			case operation_quit: gtk_main_quit(); break;
			case sel_toggle: panel_toggleselectcurrent(p); panel_info_update(p); break;
			case sel_toggleall: panel_selectinvertall(p); panel_info_update(p); break;
			case cursor_top: panel_list_totop(p); break;
			case cursor_bottom: panel_list_tobottom(p); break;
			case action_dirup: panel_dirup(p); break;
			case action_run: panel_filerun(p); gtk_signal_emit_stop_by_name ((GtkObject *)widget, "key-press-event"); break;
			case focus_pathentry: gtk_widget_grab_focus (GTK_WIDGET(GTK_COMBO(p->pathentry)->entry)); break;
			case focus_cmdline: gtk_widget_grab_focus (GTK_WIDGET(cmdline->cmd)); break;
			case toggle_hidden: gtk_signal_emit_by_name(GTK_OBJECT(p->button_togglehidden), "clicked"); break;
			case sel_pattern_plus: panel_select_pat (p); break;
			case sel_pattern_minus: panel_unselect_pat (p); break;
			case refresh: panel_refresh(p, 1); break;
			case home: panel_dirhome (p); gtk_widget_grab_focus (GTK_WIDGET(p->list)); break;
			case root: panel_change_dir (p, "/"); gtk_widget_grab_focus (GTK_WIDGET(p->list)); break;
			case selection_to_cmdline: panel_selection_to_cmdline(p); break;
			default: printf ("Unknown action (%s) for key %s\n", key, key_pressed);
		}

		/* Some keys interfere with the way nimf works. Disable them 
		 * This may cause some trouble when user's choose their own keys */
		switch (event->keyval) {
			case GDK_Left : gtk_signal_emit_stop_by_name (GTK_OBJECT(widget), "key_press_event"); break;
			default: break;
		}

	} else {
		/* unknown key */
	}
	
	free (key_pressed);

	if (action != focus_pathentry && action != focus_cmdline) {
		gtk_widget_grab_focus (GTK_WIDGET(p->list));
	}
}

gboolean panel_ev_list_select_row (GtkCList *widget, int row, int col, 
		GdkEventButton *event, panel *p) {	
	
	if (event != NULL) {
		printf ("event-type: %i\n", event->type);
		printf ("event-button: %i\n", event->button);
		if ((int)event->type == GDK_BUTTON_PRESS) {
			printf ("GDK_BUTTON_PRESS\n");
			/* Single click on row */
			if ((int)event->button == 3) {
				char *filename;

				gtk_clist_get_text(GTK_CLIST(p->list), row, 0, &filename);
				
//				contextmenu_file ((GdkEventButton *) event, p->path, filename);
			} else
			if ((int)event->button == 1) {
				if (event->state == GDK_SHIFT_MASK) {
					/* Select from previous location to new location */
					int i, from, to;
					
					if (p->prevselectedrow < row) {
						from = p->prevselectedrow;
						to = row;
					} else {
						from = row;
						to = p->prevselectedrow;
					}
				
					for (i = from; i <= to; i++) {
						panel_toggleselectrow (p, i);
					}

					panel_info_update(p);
				} else 
				if (event->state == GDK_CONTROL_MASK) {
					panel_toggleselectrow (p, row);
					panel_info_update(p);
				}
			}
		} else
		if ((int)event->type == GDK_2BUTTON_PRESS) {
			/* Double click on row */
			printf ("GDK_2BUTTON_PRESS\n");
			panel_filerun(p);
			row = p->prevselectedrow; /* prevselectedrow set by _filerun */
		} else {
			/* Check if user is selecting some files */
			printf ("OTHER\n");
			if (event->state == GDK_SHIFT_MASK) {
				/* Select from previous location to new location */
				int i, from, to;
				
				if (p->prevselectedrow < row) {
					from = p->prevselectedrow;
					to = row;
				} else {
					from = row;
					to = p->prevselectedrow;
				}
			
				for (i = from; i <= to; i++) {
					panel_toggleselectrow (p, i);
				}

				panel_info_update(p);
			} else 
			if (event->state == GDK_CONTROL_MASK) {
				panel_toggleselectrow (p, row);
				panel_info_update(p);
			}
		}
	}

	p->prevselectedrow = row;

	return TRUE;
}

void panel_ev_list_clickcolumn (GtkCList *widget, int column, 
		panel *p) {
	char *filename;

	gtk_widget_grab_focus (GTK_WIDGET(p->list));

	switch (column) {
		case 0:
			if (widget->sort_column == 0) {
				if (widget->sort_type == GTK_SORT_ASCENDING) {
					gtk_clist_set_sort_type (widget, GTK_SORT_DESCENDING);
				} else {
					gtk_clist_set_sort_type (widget, GTK_SORT_ASCENDING);
				}
			}
			gtk_clist_set_sort_column (GTK_CLIST(widget), 0);
			gtk_clist_set_compare_func (GTK_CLIST(widget), 
					panel_sort_name);
			break;
		case 1:
			if (widget->sort_column == 1) {
				if (widget->sort_type == GTK_SORT_ASCENDING) {
					gtk_clist_set_sort_type (widget, GTK_SORT_DESCENDING);
				} else {
					gtk_clist_set_sort_type (widget, GTK_SORT_ASCENDING);
				}
			}
			gtk_clist_set_sort_column (GTK_CLIST(widget), 1);
			gtk_clist_set_compare_func (GTK_CLIST(widget), 
					panel_sort_size);
			break;
	}

	gtk_clist_get_text (
			GTK_CLIST(widget),
			GPOINTER_TO_INT(GTK_CLIST(widget)->selection->data),
			0,
			&filename);
	
	panel_sort (p);
	gtk_widget_grab_focus (GTK_WIDGET(widget));
}

void panel_ev_button_refresh_clicked (
		GtkCList *widget,
		panel *p) {
	
	panel_refresh (p, 1);
//	panel_change_dir(p, p->path);
	gtk_widget_grab_focus (GTK_WIDGET(p->list));
}

void panel_ev_button_togglehidden_clicked (
		GtkWidget *widget, 
		panel *p) {
	p->showhiddenfiles ^= 1;
	
	panel_refresh(p, 1);
	gtk_widget_grab_focus (GTK_WIDGET(p->list));
}

void panel_ev_button_dirup_clicked (
		GtkCList *widget,
		panel *p) {
	panel_dirup (p);
	gtk_widget_grab_focus (GTK_WIDGET(p->list));
}
		
void panel_ev_button_dirroot_clicked (
		GtkCList *widget,
		panel *p) {
	panel_dirroot (p);
	gtk_widget_grab_focus (GTK_WIDGET(p->list));
}

void panel_ev_button_dirhome_clicked (
		GtkCList *widget,
		panel *p) {
	panel_dirhome (p);
	gtk_widget_grab_focus (GTK_WIDGET(p->list));
}

void panel_ev_row_destroyed (gpointer data) {
	free (((fileinfo *)data)->name);
	free (data);
}

void panel_ev_list_focussed(GtkWidget *widget, panel *p) {
	panel_current = p->id;
	gtk_clist_unselect_row (GTK_CLIST(p->next->list), p->next->prevselectedrow, 0);
	cmdline_setpath (cmdline, panels[panel_current]->path);

}

void panel_ev_pathentry_activate (GtkEntry *widget, panel *p) {
	panel_change_dir (p, gtk_entry_get_text(widget));
	gtk_widget_grab_focus (GTK_WIDGET(p->list));
}

void panel_ev_pathentry_focussed (GtkWidget *widget, panel *p) {
	gtk_editable_set_position (GTK_EDITABLE(widget), 0);
	gtk_editable_select_region (GTK_EDITABLE(widget), 0, -1);
}

void panel_ev_pathentry_bookmark_chosen (GtkWidget *widget, panel *p) {
	panel_ev_pathentry_activate (GTK_ENTRY(GTK_COMBO(p->pathentry)->entry), p);
}

void panel_ev_button_bookmark_clicked (GtkWidget *widget, panel *p) {
	bookmarks = bookmarks_toggle (bookmarks, p->path);
	if (bookmarks) {
		gtk_combo_set_popdown_strings (GTK_COMBO(p->pathentry), bookmarks);
		gtk_combo_set_popdown_strings (GTK_COMBO(p->next->pathentry), bookmarks);
	} else {
		gtk_list_clear_items(GTK_LIST(GTK_COMBO(p->pathentry)->list), 0, -1);
		gtk_list_clear_items(GTK_LIST(GTK_COMBO(p->next->pathentry)->list), 0, -1);
	}

	panel_set_path (p, p->path);
	panel_set_path (p->next, p->next->path);
}

panel *panel_new (int id) {
	panel *newpanel;
	char *titles[6];
	
	titles[0] = strdup("Filename");
	titles[1] = strdup("Size");
	titles[2] = strdup("Perm");
	titles[3] = strdup("Owner");
	titles[4] = strdup("Group");
	titles[5] = strdup("Last Change");

	newpanel = malloc (sizeof(struct panel));
	newpanel->id = id;
	newpanel->path = NULL;
	newpanel->selected_files = NULL;
	newpanel->jumpfile = NULL;
	newpanel->jumpfile_skip = 0;
	newpanel->nroffiles = 0;
	newpanel->vbox = gtk_vbox_new (FALSE,0);
	newpanel->bar_top = gtk_hbox_new (FALSE,0);
	newpanel->refresh = 1;
	newpanel->showhiddenfiles = 0;
	newpanel->prevselectedrow = 0;
	newpanel->operation_busy = 0;
	
	/* Path entry box */
	newpanel->pathentry = gtk_combo_new ();
	gtk_combo_disable_activate (GTK_COMBO(newpanel->pathentry));
	gtk_signal_connect (
			GTK_OBJECT(GTK_COMBO(newpanel->pathentry)->entry),
			"activate",
			GTK_SIGNAL_FUNC(panel_ev_pathentry_activate),
			(panel *)newpanel);
	gtk_signal_connect (
			GTK_OBJECT(GTK_COMBO(newpanel->pathentry)->entry),
			"grab-focus",
			GTK_SIGNAL_FUNC(panel_ev_pathentry_focussed),
			(panel *)newpanel);
	gtk_signal_connect (
			GTK_OBJECT(GTK_COMBO(newpanel->pathentry)->popwin),
			"hide",
			GTK_SIGNAL_FUNC(panel_ev_pathentry_bookmark_chosen),
			(panel *)newpanel);

	if (bookmarks) {
		gtk_combo_set_popdown_strings (GTK_COMBO(newpanel->pathentry), bookmarks);
	}
	
	/* refresh button */
	newpanel->button_dirrefresh = gtk_button_new_with_label (" R ");
	gtk_signal_connect (
			GTK_OBJECT(newpanel->button_dirrefresh),
			"clicked",
			GTK_SIGNAL_FUNC(panel_ev_button_refresh_clicked),
			(panel *)newpanel);
	/* toggle hidden files button */
	newpanel->button_togglehidden = gtk_toggle_button_new_with_label (" H ");
	gtk_signal_connect (
			GTK_OBJECT(newpanel->button_togglehidden),
			"clicked",
			GTK_SIGNAL_FUNC(panel_ev_button_togglehidden_clicked),
			(panel *)newpanel);
	/* Dir up button */
	newpanel->button_dirup = gtk_button_new_with_label (" .. ");
	gtk_signal_connect (
			GTK_OBJECT(newpanel->button_dirup),
			"clicked",
			GTK_SIGNAL_FUNC(panel_ev_button_dirup_clicked),
			(panel *)newpanel);
	/* Dir root button */
	newpanel->button_dirroot = gtk_button_new_with_label (" / ");
	gtk_signal_connect (
			GTK_OBJECT(newpanel->button_dirroot),
			"clicked",
			GTK_SIGNAL_FUNC(panel_ev_button_dirroot_clicked),
			(panel *)newpanel);
	/* Dir home button */
	newpanel->button_dirhome = gtk_button_new_with_label (" ~ ");
	gtk_signal_connect (
			GTK_OBJECT(newpanel->button_dirhome),
			"clicked",
			GTK_SIGNAL_FUNC(panel_ev_button_dirhome_clicked),
			(panel *)newpanel);
	newpanel->button_bookmark = gtk_button_new_with_label (" B ");
	gtk_signal_connect (
			GTK_OBJECT(newpanel->button_bookmark),
			"clicked",
			GTK_SIGNAL_FUNC(panel_ev_button_bookmark_clicked),
			(panel *)newpanel);
	
	/* Scrollwindow for Dir list */
	newpanel->winscroll = gtk_scrolled_window_new (
		NULL, 
		NULL);
	gtk_scrolled_window_set_policy (
		GTK_SCROLLED_WINDOW (newpanel->winscroll),
		GTK_POLICY_AUTOMATIC, 
		GTK_POLICY_ALWAYS);

	/* Dir list */
	newpanel->list = gtk_clist_new_with_titles (6, titles);
	gtk_clist_set_column_justification (GTK_CLIST(newpanel->list), 1, 
			GTK_JUSTIFY_RIGHT);
	gtk_clist_set_button_actions(GTK_CLIST(newpanel->list), 2, GTK_BUTTON_SELECTS);
	gtk_clist_set_button_actions(GTK_CLIST(newpanel->list), 3, GTK_BUTTON_SELECTS);
	gtk_clist_set_button_actions(GTK_CLIST(newpanel->list), 4, GTK_BUTTON_SELECTS);

	gtk_clist_set_column_auto_resize (GTK_CLIST(newpanel->list), 0, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST(newpanel->list), 1, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST(newpanel->list), 2, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST(newpanel->list), 3, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST(newpanel->list), 4, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST(newpanel->list), 5, TRUE);
	gtk_clist_set_column_visibility (GTK_CLIST(newpanel->list), 5, FALSE);

	gtk_clist_set_sort_column (GTK_CLIST(newpanel->list), 0);
	gtk_clist_set_compare_func (GTK_CLIST(newpanel->list), panel_sort_name);
	gtk_clist_set_sort_type (GTK_CLIST(newpanel->list), GTK_SORT_ASCENDING);
	gtk_clist_set_selection_mode(GTK_CLIST(newpanel->list),GTK_SELECTION_BROWSE);
	gtk_signal_connect (
		GTK_OBJECT(newpanel->list),
		"select_row",
		GTK_SIGNAL_FUNC(panel_ev_list_select_row),
		(panel *)newpanel);
	gtk_signal_connect (
		GTK_OBJECT(newpanel->list),
		"key-press-event",
		GTK_SIGNAL_FUNC(panel_ev_list_keypress),
		(panel *)newpanel);
	gtk_signal_connect (
		GTK_OBJECT(newpanel->list),
		"grab-focus",
		GTK_SIGNAL_FUNC(panel_ev_list_focussed),
		(panel *)newpanel);
	gtk_signal_connect (
		GTK_OBJECT(newpanel->list),
		"click-column",
		GTK_SIGNAL_FUNC(panel_ev_list_clickcolumn),
		(panel *)newpanel);

	gtk_container_add (
		GTK_CONTAINER(newpanel->winscroll), 
		newpanel->list);

	newpanel->info = gtk_entry_new ();
	gtk_entry_set_editable (GTK_ENTRY(newpanel->info), FALSE);

	/* Pathentry, dir-up, dir-root button */
	gtk_box_pack_start (GTK_BOX(newpanel->bar_top), newpanel->button_dirrefresh, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(newpanel->bar_top), newpanel->button_togglehidden, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(newpanel->bar_top), newpanel->pathentry, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(newpanel->bar_top), newpanel->button_bookmark, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(newpanel->bar_top), newpanel->button_dirup, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(newpanel->bar_top), newpanel->button_dirroot, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(newpanel->bar_top), newpanel->button_dirhome, FALSE, FALSE, 0);
	
	/* pathentry bar and dir list */
	gtk_box_pack_start (GTK_BOX(newpanel->vbox), newpanel->bar_top, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(newpanel->vbox), newpanel->winscroll, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(newpanel->vbox), newpanel->info, FALSE, FALSE, 0);

	free (titles[0]);
	free (titles[1]);
	free (titles[2]);
	free (titles[3]);
	free (titles[4]);
	free (titles[5]);

	gtk_timeout_add (500, (GtkFunction) panel_refresh_timeout, (gpointer) newpanel);

	return (newpanel);
}

int panel_refresh_timeout (panel *p) {
	struct stat *filestats = malloc(sizeof(struct stat));

	if (p->operation_busy != 1) {
		/* Has dir been modified? */
		if (stat (p->path, filestats) != -1 && p->refresh == 1) {
			if (p->path_mtime != filestats->st_mtime) {
				p->refresh = 0; /* Mutex */
				panel_refresh (p, 1);
				p->refresh = 1;

				/* Turn of cursor in updated panel if not the active panel */
				if (GTK_CLIST(p->list)->selection && p->id != panel_current) {
					gtk_clist_unselect_row (GTK_CLIST(p->list), GPOINTER_TO_INT(GTK_CLIST(p->list)->selection->data), 0);
				}
			}
		}

		free (filestats);

	}		
	
	return (1);
}

int panel_change_dir (panel *p, char *newpath) {
	char *oldpath;
	int refresh_dir = 0;
	char *prev_cursorfile = NULL;
	
	/* If refreshing the old dir, remember file on which cursor was */
	if (strcmp(p->path, newpath) == 0) {
		refresh_dir = 1;
		prev_cursorfile = panel_get_cursorfile(p);
	}
	
	oldpath = strdup(p->path);
	panel_set_path (p, newpath);
	if (panel_refresh(p, 1) == -1) {
		panel_set_path (p, oldpath);
		panel_refresh(p, 1);
	}

	free (oldpath);

	/* If refreshing the old dir, put cursor back on file where it was */
	if (refresh_dir == 1) {
		int rownr;

		rownr = panel_find_row_from_text (p, prev_cursorfile);
		panel_list_goto_row (p->list, rownr);
	}

	return (0);
}

void panel_set_path (panel *p, char *path) {
	struct stat *filestats = malloc(sizeof(struct stat));

	if (p->path == NULL) { /* first time path is copied */
		p->path = (char *) malloc (sizeof(char) * (strlen(path)+2));
	} else { /* path was previously copied, reallocate mem */
		p->path = (char *) realloc (p->path, sizeof(char) * (strlen(path)+2));
	}
	
	strcpy (p->path, path);
	
	/* strip any leading '/' from the path */
	while (1) {
		if (p->path[strlen(p->path)-1] == '/' && strlen(path) > 1) {
			p->path[strlen(p->path)-1] = '\0';
			p->path = (char *) realloc (p->path, sizeof(char) * (strlen(p->path)+2));
		} else {
			break;
		}
	}
	
	/* Check if path isn't empty */
	if (strlen(p->path) == 0) {
		p->path = (char *) realloc (p->path, sizeof(char) * 3);
		strcpy (p->path, "/");
	}
	
	gtk_entry_set_text (GTK_ENTRY(GTK_COMBO(p->pathentry)->entry), p->path);

	/* Save change time for refresh check */
	if (stat (p->path, filestats) != -1) {
		p->path_mtime = filestats->st_mtime;
	} else {
		p->path_mtime = -1;
	}

	/* Update command line */
	cmdline_setpath (cmdline, p->path);
	
	free (filestats);
}

int panel_refresh (panel *p, int force) {
	DIR *dir = NULL;
	struct dirent *direntry;
	int i;
	char *filename = NULL;
	char *fullpath = NULL;
	struct stat fstats;

#ifdef DEBUG
		printf ("\npanel_refresh: showhiddenfiles %i : ", p->showhiddenfiles);
#endif
	/* Read dir */
	stat (p->path, &fstats);
	p->path_mtime = fstats.st_mtime;

	gtk_clist_freeze(GTK_CLIST(p->list)); 

	/* Walk through the list of files and del what shouldn't be in it */
	for (i=0; i < p->nroffiles; i++) {
		
		gtk_clist_get_text (GTK_CLIST(p->list),i,0, &filename);
#ifdef DEBUG
		printf ("\npanel_refresh: listfile %s : ", filename);
#endif
	
		/* Remove files from list which are hidden */
		if (filename[0] == '.' && p->showhiddenfiles != 1) {
#ifdef DEBUG
		printf ("remove");
#endif
			panel_file_remove (p, filename);
			/* Keep list scroll due to del in mind */
			i--;
		} else {
			/* Remove files from list which are not in the dir */
			fullpath = file_fullpath (p->path, filename);
			if (lstat (fullpath, &fstats) == -1 || force) {
#ifdef DEBUG
		printf ("remove");
#endif
				panel_file_remove (p, filename);
				i--;
			}
			free (fullpath);
		}
	}
 
	/* Walk through dir and add what isn't in the list */
	dir = opendir(p->path);
	if (dir == NULL) {
		char *error = malloc(sizeof(char) * (strlen(p->path) + 16));
		sprintf (error, "Can't read dir %s",p->path);
		dialog_error (error);
		free (error);

		gtk_clist_thaw(GTK_CLIST(p->list)); 
		return (-1);
	}

	while ((direntry = readdir(dir))) {
#ifdef DEBUG
		printf ("\npanel_refresh: file %s : ", direntry->d_name);
#endif
		if (strcmp(direntry->d_name, ".") != 0) { /* Never add "." entry */
			if (
					(strcmp(direntry->d_name, "..") == 0) ||
					(direntry->d_name[0] != '.') || 
					(direntry->d_name[0] == '.' && p->showhiddenfiles == 1)) {
				fileinfo *finfo;
				char *mtime_row = NULL;
				char *mtime_file = malloc (sizeof(char) * 11);
	
#ifdef DEBUG
		printf ("*");
#endif
				finfo = file_info (p->path, direntry->d_name);
				if (finfo) {
					sprintf (mtime_file, "%li", finfo->mtime);
				
					gtk_clist_get_text (GTK_CLIST(p->list),
							panel_find_row_from_text (p, direntry->d_name),5, &mtime_row);
					
					if (mtime_row == NULL) {
						panel_file_insert (p, p->path, direntry->d_name);
#ifdef DEBUG
		printf ("add");
#endif
					} else
					if (strcmp(mtime_row, mtime_file) != 0) {
#ifdef DEBUG
		printf ("refresh");
#endif
					panel_file_remove (p, direntry->d_name);
						panel_file_insert (p, p->path, direntry->d_name);
					}
				} else {
					dialog_error ("Can't get file/dir info\n");
				}
				free (mtime_file);
			}
		}

	}
	closedir (dir);

	panel_sort (p);
	panel_info_update(p);
	gtk_clist_thaw(GTK_CLIST(p->list)); 
	
	
	return (0);
}


void panel_sort (panel *p) {
	char *filename;
	int rownr;
	
	filename = panel_get_cursorfile (p); /* FIXME: possibly removable (jump to row won't work during freeze? */
	gtk_clist_sort (GTK_CLIST(p->list));
	rownr = panel_find_row_from_text (p, filename);
	panel_list_goto_row (p->list, rownr);

}

int panel_sort_name (GtkCList *list, gconstpointer row1, gconstpointer row2) {
	struct fileinfo *file1_info, *file2_info;
	int to_top; /* should be returned if file should be at the top */
	int to_bottom;
	
	/* Sort ascending or descending? */
	if (list->sort_type == GTK_SORT_ASCENDING) {
		to_top = -1;
		to_bottom = 1;
	} else {
		to_top = 1;
		to_bottom = -1;
	}

	file1_info = ((GtkCListRow *)row1)->data;
	file2_info = ((GtkCListRow *)row2)->data;
 
	if (strcmp(file1_info->name, "..") == 0) {
		return (to_top);
	}

	/* Dir goes to the top of the list? */
	if (file1_info->type == FTYPE_DIR && strcmp( /* 1 */
				config_find_value(config_nimf, "sort_dirsfirst", "1")
				,"1") == 0) {
		if (file2_info->type != FTYPE_DIR) {
			return (to_top);
		} else {
			if (strcmp (
				config_find_value(config_nimf, "sort_casesensitive","0"), 
				"0") == TRUE
				) {
				return (strcmp(file1_info->name, file2_info->name));
			} else {
				return (strcasecmp(file1_info->name, file2_info->name));
			}
		}
	} 
	else
	if (file2_info->type == FTYPE_DIR && strcmp(
				config_find_value(config_nimf, "sort_dirsfirst", "1")
				,"1") == 0) {
		return (to_bottom);
	} else {
		if (strcmp (
			config_find_value(config_nimf, "sort_casesensitive","0"), 
			"0") == TRUE
			) {
			return (strcmp(file1_info->name, file2_info->name));
		} else {
			return (strcasecmp(file1_info->name, file2_info->name));
		}
	}
}

int panel_sort_size (GtkCList *list, gconstpointer row1, gconstpointer row2) {
	struct fileinfo *file1_info, *file2_info;
	int to_top; /* should be returned if file should be at the top */
	int to_bottom; /* Should be returned if file should be at the bottom */
	
	/* Sort ascending or descending? */
	if (list->sort_type == GTK_SORT_ASCENDING) {
		to_top = -1;
		to_bottom = 1;
	} else {
		to_top = 1;
		to_bottom = -1;
	}
	
	file1_info = ((GtkCListRow *)row1)->data;
	file2_info = ((GtkCListRow *)row2)->data;
 
	if (strcmp(file1_info->name, "..") == 0) {
		return (to_top);
	}

	if (file1_info->type == FTYPE_DIR && strcmp(
				config_find_value(config_nimf, "sort_dirsfirst", "1")
				,"1") == 0) {
		if (file2_info->type != FTYPE_DIR) { /* dir? */
			return (to_top);
		} else {
			if (file1_info->size > file2_info->size) 
				return (-1);
			else
				return (1);
		}
	} 
	else
	if (file2_info->type == FTYPE_DIR && strcmp(
				config_find_value(config_nimf, "sort_dirsfirst", "1")
				,"1") == 0) {
		return (to_bottom);
	} else {
		if (file1_info->size > file2_info->size)
			return (-1);
		else
			return (1);
	}
}

void panel_filerun (panel *p) {
	char *filename;
	char *fullpath;

	/* FIXME : check for muliple selection */
	gtk_clist_get_text (
			GTK_CLIST(p->list),
			GPOINTER_TO_INT(GTK_CLIST(p->list)->selection->data),
			0,
			&filename);

	if (strcmp(filename, "..") == 0) {
		panel_dirup(p);
	} else {
		fullpath = file_fullpath(p->path, filename);
	
		if (file_type(fullpath) == FTYPE_DIR) {
			/* Change into dir */
			panel_change_dir(p, fullpath);
		} else {
			char *config_command = NULL;
			char *walk_ext = NULL; 
			int found_runnable_ext = 0;

			walk_ext = filename;

			while ((walk_ext = strchr(walk_ext, '.')) != NULL && found_runnable_ext == 0) {
				walk_ext++;
				if ((config_command = config_find_value(config_ext, walk_ext, NULL)) != NULL) {
					/* Run the file */
					char *command = malloc(sizeof(char) * (strlen(config_command)+strlen(fullpath)+1));
							
					/* Replace %s in config_command with the full path and filename */
					sprintf (command, config_command, fullpath);
					command = strcat (command, "&");
					
					printf ("running: %s\n", command);
					system (command);
					found_runnable_ext = 1;
					free (command);
				}
			}

			if (found_runnable_ext == 0) {
				char *error = malloc(sizeof(char) * (126 + strlen(filename)));
				sprintf (error, 
						"No action was found which is linked to the\nextention of %s. You can specify an action\nin the Action editor (Settings menu).",
						filename);
				dialog_error (error);
				free (error);
			}
		}

		free (fullpath);
	}

}

void panel_dirup (
		panel *p) {
	char *newpath = malloc(sizeof(char) * (strlen(p->path)+1));
	char *lastdirinpath = NULL;
	int row;
	
	if (strcmp(p->path, "/") != 0) {
		strcpy (newpath, p->path);

		/* Remove last dir from path by ending string before it */
		lastdirinpath = strrchr(newpath, '/');
		lastdirinpath[0] = '\0';
		lastdirinpath++;
		
		/* Read new path in panel */
		panel_change_dir (p, newpath);

		/* Jump to dir we just left */
		row = panel_find_row_from_text (p, lastdirinpath);
		panel_list_goto_row (p->list, row);
	}

	free (newpath);
}

void panel_dirroot (
		panel *p) {
	panel_change_dir (p, "/");
}

void panel_dirhome (
		panel *p) {
	char *env_home;
	
	env_home = strdup(getenv("HOME"));
	if (env_home == NULL) {
		panel_change_dir (p, "/");
	} else {
		panel_change_dir (p, env_home);
		free (env_home);
	}
}

void panel_toggle_hidden (panel *p) {
	p->showhiddenfiles ^= 1;
	
	/* Can't cat showhiddenfiles to gboolean, or gtk will crash for some reason */
	if (p->showhiddenfiles == 1) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p->button_togglehidden), TRUE);
	} else {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p->button_togglehidden), FALSE);
	}
	panel_refresh(p, 1);
}


void panel_list_goto_row (GtkWidget *clist, int row) {
	gtk_clist_freeze(GTK_CLIST(clist)); 
	
	gtk_clist_unselect_all(GTK_CLIST(clist));
	gtk_clist_select_row(GTK_CLIST(clist), row, 0);
	GTK_CLIST(clist)->focus_row = row;
  
	if (gtk_clist_row_is_visible(GTK_CLIST(clist), row) != GTK_VISIBILITY_FULL) {
		/* If the row containing the cursor is not visible, scroll the list */
		gtk_clist_moveto(GTK_CLIST(clist), row, -1, 0.5, 0.0);
	}
	
	gtk_clist_thaw(GTK_CLIST(clist)); 
}

void panel_list_totop (
		panel *p) {
	panel_list_goto_row (p->list, 0);
}

void panel_list_tobottom (
		panel *p) {
	panel_list_goto_row (p->list, p->nroffiles-1);
}

void panel_next (void) {
	panel_current ^= 1;
	gtk_widget_grab_focus (GTK_WIDGET(panels[panel_current]->list));
}

void panel_info_update (panel *p) {
	if (p->selected_files == NULL) {
		/* Show single file information */

		char *filename;
		char *fullpath;
		struct stat *filestats = malloc(sizeof(struct stat));
		
		/* Get a full path to current selected file */
		if (GTK_CLIST(p->list)->selection != NULL) {
		    gtk_clist_get_text (
					GTK_CLIST(p->list),
					GPOINTER_TO_INT(GTK_CLIST(p->list)->selection->data),
					0,
					&filename);
			fullpath = file_fullpath (p->path, filename);
		
			/* Get information on the file */
			if (lstat (fullpath, filestats) != -1) {
				if (S_ISLNK(filestats->st_mode)) {
					char buf[1024];
					int buffilled;
					
					buffilled = readlink (fullpath, buf, 1024);
					buf[buffilled] = '\0';
					
					gtk_entry_set_text (GTK_ENTRY(p->info), buf);
				} else {
					gtk_entry_set_text (GTK_ENTRY(p->info), "");
				}
			}
			free (fullpath);
		}
	} else {

		char *fullpath;
		
		/* Show selected files summary */
		GList *cur;
		char *info_message = malloc (sizeof(char) * 1000);
		char *nicesize;
		long unsigned int totalsize = 0;
		struct stat *filestats = malloc(sizeof(struct stat));
		
		/* Calculate total filesize */
		cur = g_list_first (p->selected_files);
		
		/* FIXME: OPTIMIZE */
		while (cur) {
			fullpath = file_fullpath (p->path, (char *)cur->data);

			if (stat (fullpath, filestats) != -1) {
				totalsize = totalsize + filestats->st_size;
			}
			free (fullpath);
			cur = cur->next;
		}

		nicesize = file_nicesize (totalsize);
				
		sprintf (info_message, "%i/%i selected (%s Bytes).", 
			g_list_length(p->selected_files),
			p->nroffiles-1,
			nicesize);
		
		gtk_entry_set_text (GTK_ENTRY(p->info),  info_message);
		
		free (nicesize);
		free (filestats);
		free (info_message);
	}
}

int panel_find_row_from_text (panel *p, char *srchtext) {
	char *text = NULL;
	int found = -1;
	int i;

	/* Find the row in the list who's text is srchtext and return it */
	for (i=0; i != p->nroffiles; i++) {
		gtk_clist_get_text (GTK_CLIST(p->list),i,0, &text);
				
		if (strcmp(srchtext,text) == 0) {
			found = i;
			break;
		}
	}
	
	return (found);
}

int panel_find_row_from_partial_text (panel *p, char *srchtext) {
	char *text;
	int i;
	int cmpresult;
	int row = -1;

	for (i=0; i!=p->nroffiles; i++) {
		gtk_clist_get_text (GTK_CLIST(p->list),i,0, &text);
				
		if (strcasecmp(
				config_find_value(config_nimf, "jump_case_sensitive", "1"), 
				"1") == 0) {
			/* Case sensitive jump search */
			cmpresult = strncmp(srchtext,text, strlen(srchtext));
		} else {
			cmpresult = strncasecmp(srchtext,text, strlen(srchtext));
			
		}
		
		if (cmpresult == 0) {
			/* found a match */
			row = i;
			break;
		}

	}
	
	return (row);
}

int panel_list_jump(panel *p, GdkEventKey *event) {
	char key;
	int len;
	int nojumpfile = 0; /* toggle if no match for new jumpsearch critirea */

	key = event->keyval;
	printf ("key %i\n", event->keyval);
	if ((event->keyval >= 0 && event->keyval <= 255)) { /* jump key pressed */
		int row;
		
		if (p->jumpfile == NULL) { /* First time jump */
			p->jumpfile = (char *) malloc (sizeof(char));
			p->jumpfile[0] = 0;
		}
	
		/* Add the jump key to the search */
		len = strlen(p->jumpfile);
		p->jumpfile = (char *) realloc (p->jumpfile, sizeof(char) * (len + 2));
		
		p->jumpfile[len] = key;
		p->jumpfile[len+1] = '\0';

		/* Jump to search */
		row = panel_find_row_from_partial_text (p, p->jumpfile);
		if (row != -1) {
			panel_list_goto_row (p->list, 
					row);
		} else {
			/* No match found for new jumpsearch criteria, so mark the latest added jumpsearch key for removal */
			nojumpfile = 1;
		}

		/* Show search in panel info box */
		gtk_entry_set_text (GTK_ENTRY(p->info), p->jumpfile);
	}
	
	if ((event->keyval == GDK_BackSpace && p->jumpfile != NULL) || nojumpfile) { /* Remove a jump key from search */
		int row;

		len = strlen(p->jumpfile);
		if (len == 0) {
			free (p->jumpfile);
			p->jumpfile = NULL;
		} else {

			p->jumpfile = (char *) realloc (p->jumpfile, sizeof(char) * len);
			p->jumpfile[len-1] = 0;

			/* Jump to search */
			row = panel_find_row_from_partial_text (p, p->jumpfile);
			if (row != -1) {
				panel_list_goto_row (p->list, 
						row);
			}
			
			/* Show search in panel info box */
			gtk_entry_set_text (GTK_ENTRY(p->info), p->jumpfile);
		}

	} else 
	if (event->keyval == GDK_Escape || event->keyval == GDK_Return) {
		/* Cancel search */
		free (p->jumpfile);
		p->jumpfile = NULL;
		panel_info_update(p);
	}

	return 0;
}

void panel_colorrow (panel *p, int rownr) {
	char *filename;
	char *color = NULL;
	GdkColor gtkcolor; 
	
	/* Determine which color the file should get */
	gtk_clist_get_text(GTK_CLIST(p->list), rownr, 0, &filename);
	if (g_list_find(p->selected_files, filename)) {
		/* File is selected */
		color = strdup ("#FF0000");
	} else {
		char *pathtofile;
		int type;
	
		pathtofile = file_fullpath (p->path, filename);
		type = file_type_withlink (pathtofile);
		free (pathtofile);
		
		switch (type) {
			case FTYPE_NORMAL : color = strdup ("#000000"); break;
			case FTYPE_DIR : color = strdup ("#0000FF"); break;
			case FTYPE_LINKNORMAL : color = strdup ("#007C7C"); break;
			default: color = strdup ("#000000"); break;
		}
	}
	
	if (color != NULL) {
		gdk_color_parse (color, &gtkcolor);
		gdk_color_alloc(gtk_widget_get_colormap(p->list), &gtkcolor);
		gtk_clist_set_foreground(GTK_CLIST(p->list), rownr, &gtkcolor);
		
		free (color);
	}
}

void panel_toggleselectcurrent(panel *p) {
	int rownr;

	rownr = GPOINTER_TO_INT((GTK_CLIST(p->list)->selection)->data);
	panel_toggleselectrow (p, rownr);
	
	if (rownr+1 < p->nroffiles) {
		panel_list_goto_row(p->list, rownr+1);
	}
}

void panel_toggleselectrow (panel *p, int rownr) {
	char *filename;

	gtk_clist_get_text(GTK_CLIST(p->list), rownr, 0, &filename);
	
	if (g_list_find(p->selected_files, filename) == NULL) {
		panel_selectrow (p,rownr);
	} else {
		panel_unselectrow (p, rownr);
	}

	if (g_list_length (p->selected_files) == 0) {
		p->selected_files = NULL;
	}
}

void panel_selectrow (panel *p, int rownr) {
	char *filename;

	gtk_clist_get_text(GTK_CLIST(p->list), rownr, 0, &filename);
	
	if (strcmp(filename, "..") != 0) { /* Skip ".." */
		if (g_list_find(p->selected_files, filename) == NULL) {
			/* Add to selection list */
			p->selected_files = g_list_append (p->selected_files, filename);
			panel_colorrow (p, rownr);
		}
	}
//	panel_info_update (p); /* Removed for speed increase */
}

void panel_unselectrow (panel *p, int rownr) {
	char *filename;

	gtk_clist_get_text(GTK_CLIST(p->list), rownr, 0, &filename);
	
	if (strcmp(filename, "..") != 0) { /* Skip ".." */
		if (g_list_find(p->selected_files, filename) != NULL) {
			/* Remove from seletion list */
			p->selected_files = g_list_remove (p->selected_files, filename);
			panel_colorrow (p, rownr);
		}
	
		if (g_list_length (p->selected_files) == 0) {
			p->selected_files = NULL;
		}
	}
//	panel_info_update (p); /* Removed for speed increase */
}

void panel_selectinvertall (panel *p) {
	int row;

	for (row = 0; row < p->nroffiles; row++) {
		panel_toggleselectrow (p, row);
	}

	panel_info_update(p);
}

/* FIXME: This function unused?? */
void panel_selectall (panel *p) {
	int row;

	for (row = 0; row < p->nroffiles; row++) {
		panel_selectrow (p, row);
	}

	panel_info_update(p);
}

/* FIXME: This function unused?? */
void panel_selectnone (panel *p) {
	int row;

	for (row = 0; row < p->nroffiles; row++) {
		panel_unselectrow (p, row);
	}	
	panel_info_update(p);
}

void panel_unselect_pat (panel *p) {
	char *pattern_default = strdup("*");
	char *pattern_user = NULL;
	int row;
	char *filename;
	int match_result;
	int pattern_invalid = 0;
	
	dialog_enterpath ("Select", "Select files matching pattern", pattern_default, &pattern_user);
	gtk_main();
	
	if (pattern_user != NULL) {
		/* Walk through list of files, and select all that match */
		for (row = 0; row < p->nroffiles; row++) {
			gtk_clist_get_text(GTK_CLIST(p->list), row, 0, &filename);
			
			match_result = fnmatch(pattern_user, filename, 0);
			switch (match_result) {
				case 0           : panel_unselectrow (p, row); break;
				default          : break;
			}
			
			if (pattern_invalid) {
				break;
			}
		}
		
		free (pattern_user);
	}

	free (pattern_default);

	panel_info_update(p);
}

void panel_select_pat (panel *p) {
	char *pattern_default = strdup("*");
	char *pattern_user = NULL;
	int row;
	char *filename;
	int match_result;
	int pattern_invalid = 0;
	
	dialog_enterpath ("Select", "Select files matching pattern", pattern_default, &pattern_user);
	gtk_main();
	
	if (pattern_user != NULL) {
		/* Walk through list of files, and select all that match */
		for (row = 0; row < p->nroffiles; row++) {
			gtk_clist_get_text(GTK_CLIST(p->list), row, 0, &filename);
			
			match_result = fnmatch(pattern_user, filename, 0);
			
			switch (match_result) {
				case 0           : panel_selectrow (p, row); break;
				default          : break;
			}

			if (pattern_invalid) {
				break;
			}
		}
		
		free (pattern_user);
	}

	free (pattern_default);
	
	panel_info_update(p);
}

GList *panel_getselectedfilelist (panel *p) {
	GList *cur;
		
	cur = g_list_first (p->selected_files);
	
	if (cur == NULL) {
		int rownr;
		char *filename;
		
		rownr = GPOINTER_TO_INT((GTK_CLIST(p->list)->selection)->data);
		gtk_clist_get_text(GTK_CLIST(p->list), rownr, 0, &filename);
	
		cur = g_list_append (cur, filename);
	}

	return (cur);
}

void panel_disable (panel *p) {
	gtk_widget_set_sensitive (p->vbox, 0);
}

void panel_enable (panel *p) {
	gtk_widget_set_sensitive (p->vbox, 1);
	gtk_widget_grab_focus (p->list);
}

char *panel_get_cursorfile (panel *p) {
	char *filename = NULL;
	
	if (p->list != NULL && GTK_CLIST(p->list)->selection != NULL) {
		gtk_clist_get_text(
				GTK_CLIST(p->list), 
				GPOINTER_TO_INT((GTK_CLIST(p->list)->selection)->data),
				0,
				&filename);
	} else {
		gtk_clist_get_text(
				GTK_CLIST(p->list),
				p->prevselectedrow,
				0,
				&filename);
	}
	return (filename);
}

int panel_file_insert (panel *p, char *path, char *filename) {
	char *fullpath;
	int filetype;
	int rownr;
	struct fileinfo *finfo;
	char *nicesize, *nicemode, *niceowner, *nicegroup;
	char *cols[6];

	fullpath = file_fullpath(path, filename);
	
	filetype = file_type_withlink(fullpath);
	finfo = file_info(p->path, filename);
	
	if (finfo == NULL) {
		printf ("Couldn't get stats for file %s/%s. Fatal. Exiting.\
				you shouldn't have seen this.\n", p->path, 
				filename);
		return (-1);
	}
	
	nicesize = file_nicesize(finfo->size);
	nicemode = file_nicemode(finfo->mode);
	niceowner = file_niceusername(finfo->uid);
	nicegroup = file_nicegroupname(finfo->gid);
	
	cols[0] = strdup(filename);
	cols[1] = nicesize;
	cols[2] = nicemode;
	cols[3] = niceowner;
	cols[4] = nicegroup;
	cols[5] = malloc(sizeof(char) * 11);
	sprintf (cols[5], "%li", finfo->mtime);
	
	rownr = gtk_clist_append (GTK_CLIST(p->list), cols);
	panel_colorrow (p, rownr);
	
	/* link the fileinfo struct to the clist row */
	gtk_clist_set_row_data_full (GTK_CLIST(p->list), rownr, finfo,
			panel_ev_row_destroyed);
	
	free (cols[0]);
	free (cols[1]);
	free (cols[2]);
	free (cols[3]);
	free (cols[4]);
	free (cols[5]);
	free (fullpath);

	p->nroffiles++;
	
	return (rownr);
}

int panel_file_remove (panel *p, char *filename) {
	int rownr;
	
	rownr = panel_find_row_from_text (p, filename);
	/* If deleting last row in list, move cursor 1 up */
	if (rownr == p->nroffiles-1) {
		panel_list_goto_row (p->list, rownr-1);
	}
	panel_unselectrow (p, rownr);
	gtk_clist_remove (GTK_CLIST(p->list), rownr);

	p->nroffiles--;
	
	return (rownr);
}

char *panel_selection_to_string (panel *p) {
	GList *cur;
	char *selstr = malloc(sizeof(char));
		
	strcpy (selstr, ""); /* init */
	
	cur = panel_getselectedfilelist(p);
	while (cur) {
		selstr = realloc(selstr, sizeof(char) * (strlen(selstr)) + strlen(cur->data) + 4);
		sprintf (selstr, "%s\"%s\" ", selstr, (char *)cur->data);
		cur = cur->next;
	}

	return (selstr);
}

void panel_selection_to_cmdline (panel *p) {
	int pos;
	char *selstr;

	selstr = panel_selection_to_string(p);
	pos = gtk_editable_get_position (GTK_EDITABLE(cmdline->cmd));

	gtk_editable_insert_text (GTK_EDITABLE(cmdline->cmd), selstr, strlen(selstr), &pos);
	gtk_editable_set_position (GTK_EDITABLE(cmdline->cmd), pos);

	free (selstr);
}
