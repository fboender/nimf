/* Functions for creating dialogs (errors, warnings, confirmations, etc) */
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
#include <string.h>
#include <malloc.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/types.h>

#include <pwd.h>
#include <grp.h>
#include <sys/types.h>

#include "dialogs.h"
#include "logo.h"
#include "file.h"

/* These must be static for some reason, or the dialog can't be 
 * re-used. If anybody knows why, please let me know */
static GtkWidget *dialog;
static GtkWidget *entry;

void dialog_error_ev_button_clicked (
		GtkWidget *widget,
		GtkWidget *win) {
	gtk_widget_destroy (win);
}

void dialog_about_btn_ok_cb (GtkWidget *widget, GtkWidget *win) {
	gtk_widget_destroy(win);
}

void dialog_about (GtkWidget *widget, gpointer data) {
	GtkWidget *win, *pixmapwid;
	GdkPixmap *logo;
	GtkStyle *style;
	GtkWidget *label;
	GtkWidget *vbox;
	GtkWidget *frame;
	GtkWidget *btn_ok;
	GdkBitmap *mask;
	
	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW(win), "About Nimf");
	gtk_widget_realize(win);

	style = gtk_widget_get_style( win );
    logo = gdk_pixmap_create_from_xpm_d(
			win->window,  
			&mask,
			&style->bg[GTK_STATE_NORMAL],
			(gchar **)logo_xpm );
    pixmapwid = gtk_pixmap_new( logo, mask );
    gtk_widget_show( pixmapwid );

	frame = gtk_frame_new (NULL);
	btn_ok = gtk_button_new_with_label ("Ok");
	
	label = gtk_label_new ("\nNimf v%%VERSION\n\nCopyright, 2002-2003, by Ferry Boender\n\n%%HOMEPAGE\nReleased under the GPL\n<%%EMAIL>");
	vbox = gtk_vbox_new (FALSE, 0);
	
	gtk_signal_connect (
			GTK_OBJECT(btn_ok),
			"clicked",
			GTK_SIGNAL_FUNC(dialog_about_btn_ok_cb),
			win);

	gtk_box_pack_start (GTK_BOX(vbox), pixmapwid, TRUE, TRUE, 5);
	gtk_box_pack_start (GTK_BOX(vbox), label, TRUE, TRUE, 5);
	gtk_box_pack_start (GTK_BOX(vbox), btn_ok, TRUE, TRUE, 5);

	gtk_container_add (GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width (GTK_CONTAINER(frame), 5);
	gtk_container_add (GTK_CONTAINER(win), frame);

	gtk_widget_show_all (win);

}

void dialog_error (char *text) {
	GtkWidget *win;
	GtkWidget *label;
	GtkWidget *vbox;
	GtkWidget *b_ok;
		
	win = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title (GTK_WINDOW(win), "Error");
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(vbox), 5);
	label = gtk_label_new (text);
	b_ok = gtk_button_new_with_label ("Ok");

	gtk_signal_connect (
			GTK_OBJECT(b_ok),
			"clicked",
			GTK_SIGNAL_FUNC(dialog_error_ev_button_clicked),
			win);

	gtk_box_pack_start (GTK_BOX(vbox), label, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(vbox), b_ok, FALSE, FALSE, 5);
	gtk_container_add (GTK_CONTAINER(win), vbox);

	gtk_widget_show_all (win);
	gtk_widget_grab_focus(b_ok);
	gtk_grab_add (win);
}

void dialog_report_error_ev_b_ok_clicked (
		GtkWidget *widget,
		int *action) {
	*action = 0;
	gtk_main_quit();
}

void dialog_report_error_ev_b_cancel_clicked (
		GtkWidget *widget,
		int *action) {
	*action = 1;
	gtk_main_quit();
}


void dialog_report_error (char *pre_msg, char *path, char *error, int *action) {
	GtkWidget *win;
	GtkWidget *label_pre_msg;
	GtkWidget *label_path;
	GtkWidget *label_error;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *b_ok;
	GtkWidget *b_cancel;
	
	win = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_position (GTK_WINDOW(win), GTK_WIN_POS_CENTER);
	
	gtk_window_set_title (GTK_WINDOW(win), "Error");
	vbox = gtk_vbox_new (FALSE, 0);
	hbox = gtk_hbox_new (FALSE, 0);
	
	gtk_container_set_border_width (GTK_CONTAINER(vbox), 5);
	gtk_container_set_border_width (GTK_CONTAINER(hbox), 5);
	
	label_pre_msg = gtk_label_new (pre_msg);
	label_path = gtk_label_new (path);
	label_error = gtk_label_new (error);
	b_ok = gtk_button_new_with_label ("Ok");
	b_cancel = gtk_button_new_with_label ("Cancel");

	gtk_signal_connect (
			GTK_OBJECT(b_ok),
			"clicked",
			GTK_SIGNAL_FUNC(dialog_report_error_ev_b_ok_clicked),
			action);
	gtk_signal_connect (
			GTK_OBJECT(b_cancel),
			"clicked",
			GTK_SIGNAL_FUNC(dialog_report_error_ev_b_cancel_clicked),
			action);

	gtk_box_pack_start (GTK_BOX(hbox), b_ok, TRUE, TRUE, 5);
	gtk_box_pack_start (GTK_BOX(hbox), b_cancel, TRUE, TRUE, 5);
	
	gtk_box_pack_start (GTK_BOX(vbox), label_pre_msg, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(vbox), label_path, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(vbox), label_error, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(vbox), hbox, TRUE, TRUE, 5);
	
	gtk_container_add (GTK_CONTAINER(win), vbox);

	gtk_widget_show_all (win);
	gtk_widget_grab_focus(b_ok);
	gtk_grab_add (win);

	gtk_main();
	gtk_widget_destroy (win);
}

void dialog_enterpath_cb_ok (GtkWidget *widget, char **path) {
	char *s;

	s = gtk_entry_get_text(GTK_ENTRY(entry));
	*path = g_strdup(s);

	gtk_widget_destroy (dialog);
	gtk_main_quit();
}

void dialog_enterpath_cb_cancel (GtkWidget *widget, char **path) {
	*path = NULL;
	
	gtk_widget_destroy (dialog);
	gtk_main_quit();
}

void dialog_enterpath_cb_key (GtkWidget *widget, GdkEventKey *event, 
		char **path) {
	if (event->keyval == GDK_Escape) {
		dialog_enterpath_cb_cancel(widget, path);
	} else
	if (event->keyval == GDK_KP_Enter) { 
		dialog_enterpath_cb_ok (widget, path);
	}
}

void dialog_enterpath (char *title, char *message, char *preentry, char **path) {
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *button_ok;
	GtkWidget *button_cancel;
//	GtkWidget *option_preserve_attribs;
//	GtkWidget *option_resolve_links;
	
	/* dialog is global because of [random gtk reason] */
	dialog = gtk_dialog_new();
	gtk_window_set_position (GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	gtk_window_set_title (GTK_WINDOW(dialog), title);
	gtk_window_set_default_size (GTK_WINDOW(dialog), 300,40);
	
	vbox = GTK_DIALOG(dialog)->vbox;
	hbox = GTK_DIALOG(dialog)->action_area;
	
	label = gtk_label_new (message);
	
	entry = gtk_entry_new();
	gtk_signal_connect(
			GTK_OBJECT(entry),
			"activate",
			GTK_SIGNAL_FUNC(dialog_enterpath_cb_ok),
			path);
	gtk_entry_set_text (GTK_ENTRY(entry), preentry);
	gtk_editable_set_position (GTK_EDITABLE(entry), 0);
	gtk_editable_select_region (GTK_EDITABLE(entry), 0, -1);

//	option_preserve_attribs = gtk_check_button_new_with_label ("Preserve attributes");
//	option_resolve_links = gtk_check_button_new_with_label ("Resolve links");
	
	button_ok = gtk_button_new_with_label ("Ok");

	button_cancel = gtk_button_new_with_label ("Cancel");
	gtk_signal_connect (GTK_OBJECT(button_ok), "clicked", 
			GTK_SIGNAL_FUNC(dialog_enterpath_cb_ok), path);
	gtk_signal_connect (GTK_OBJECT(button_cancel), "clicked", 
			GTK_SIGNAL_FUNC(dialog_enterpath_cb_cancel), path);
	
	gtk_signal_connect (GTK_OBJECT(dialog), "key-press-event", 
			GTK_SIGNAL_FUNC(dialog_enterpath_cb_key), path);
	
	gtk_container_set_border_width (GTK_CONTAINER(vbox), 5);
	
	gtk_box_pack_start (GTK_BOX(vbox), label, 0, 0, 2);
	gtk_box_pack_start (GTK_BOX(vbox), entry, 0, 0, 2);
//	gtk_box_pack_start (GTK_BOX(vbox), option_preserve_attribs, 0, 0, 2);
//	gtk_box_pack_start (GTK_BOX(vbox), option_resolve_links, 0, 0, 2);

	gtk_box_pack_start (GTK_BOX(hbox), button_ok, 1, 1, 0);
	gtk_box_pack_start (GTK_BOX(hbox), button_cancel, 1, 1, 0);

	gtk_widget_show_all (dialog);
	gtk_widget_grab_focus (entry);

}

GtkWindow *dialog_progress (char *title, GtkWidget *label, GtkWidget *progress_bar) {
	GtkWidget *dialog;
	GtkWidget *vbox;
		
	dialog = gtk_dialog_new ();
	gtk_progress_set_show_text (GTK_PROGRESS(progress_bar), TRUE);
	gtk_progress_set_format_string (GTK_PROGRESS(progress_bar), "%p%%");
		
	gtk_window_set_title (GTK_WINDOW(dialog), title);
	gtk_window_set_default_size (GTK_WINDOW(dialog), 400, 100);
	gtk_window_set_policy (GTK_WINDOW(dialog), TRUE, TRUE, FALSE);

	vbox = GTK_DIALOG(dialog)->vbox;
	
	gtk_box_pack_start (GTK_BOX(vbox), label, 0, 0, 2);
	gtk_box_pack_start (GTK_BOX(vbox), progress_bar, 0,0,2);
	
	gtk_widget_show_all(dialog);
	
	return (GTK_WINDOW(dialog));
}

GtkWidget *chkbox_perms[12];
int chkbox_blocksignal = 0;
mode_t perms_int = 0; /* Keeps an integer representation of the permission mode across the dialog */
GtkWidget *entry_octal;
GtkWidget *entry_text;

void dialog_permissions_update (void) {
	char perms_set[] = "...rwxrwxrwx";
	char perms_octal[5];
	char perms_text[12];
	int shiftbit;
	int i;
	
	/* Disable events for checkboxes (against recursive updates) */
	chkbox_blocksignal = 1;

	/* Convert permissions integer to octal */
	sprintf (perms_octal, "%o", perms_int);

	shiftbit = 1 << 11;
	/* Convert permissions integer to text and turn on buttons */
	for (i = 0; i < 12; i++) {
		if ((perms_int & shiftbit) == shiftbit) {
			perms_text[i] = perms_set[i];
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(chkbox_perms[11-i]), TRUE);
		} else {
			perms_text[i] = '-';
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(chkbox_perms[11-i]), FALSE);
		}

		shiftbit >>= 1;
	}

	gtk_entry_set_text (GTK_ENTRY(entry_octal), perms_octal);
	gtk_entry_set_text (GTK_ENTRY(entry_text), perms_text);

	chkbox_blocksignal = 0;
}

void dialog_permissions_toggles (GtkWidget *widget, gpointer *data) {
	if (chkbox_blocksignal == 1) { return; }

	perms_int ^= (1 << (GPOINTER_TO_INT(data)));

	dialog_permissions_update();
}

void dialog_permissions_octal(GtkWidget *widget, void *data) {
	char *perms_octal;

	perms_octal = gtk_entry_get_text(GTK_ENTRY(widget));
	sscanf (perms_octal, "%4o", &perms_int);
	
	dialog_permissions_update();
}

void dialog_permissions_text(GtkWidget *widget, void *data) {
	char perms_set[] = "...rwxrwxrwx"; 
	char *perms_text;
	int i;
	
	perms_text = gtk_entry_get_text(GTK_ENTRY(widget));
	perms_int = 0;
	for (i = 0; i < strlen(perms_text); i++) {
		if (perms_text[i] == perms_set[i]) {
			perms_int |= (1 << (9-i));
		}
	}

	dialog_permissions_update();
}

void dialog_permissions_cb_ok_clicked (void) {
	gtk_main_quit();
}

void dialog_permissions_cb_cancel_clicked (GtkWidget *widget, int *cancel) {
	*cancel = 1;
	gtk_main_quit();
}

void dialog_permissions_toggleoption (GtkWidget *widget, int *data) {
	*data ^= 1;
}

int dialog_permissions (mode_t initial_mode, mode_t *return_mode, int *return_options) {
	GtkWidget *window;
	GtkWidget *vbox, *hbox, *vbox_options;
	GtkWidget *frame_perm, *frame_options;
	GtkWidget *table;
	GtkWidget *label;
//	GSList *group;
//	GtkWidget *radio_recurse_allfiles;
//	GtkWidget *radio_recurse_xonly;
	GtkWidget *btn_ok, *btn_cancel;
	GtkWidget *chkbtn_recurse;
//	GtkWidget *chkbtn_preservedirs;
//	GtkWidget *chkbtn_preserveexec;
	int option_recurse = 0;
//	int option_dirs, options_execs;
	
	int cancel = 0;
	
	char *perms[] = {"Execute", "Write", "Read"};
	char *perms_special[] = {"Sticky", "Set UID", "Set GID"};
	int row, col;
	
	initial_mode &= ~(S_IFMT); /* No need for file type */

	window = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_window_set_position (GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_title (GTK_WINDOW(window), "Permissions");

	vbox = gtk_vbox_new (FALSE, 0);
	
	frame_perm = gtk_frame_new ("Permissions");
	gtk_container_set_border_width (GTK_CONTAINER(frame_perm), 5);
	
	frame_options = gtk_frame_new ("Options");
	gtk_container_set_border_width (GTK_CONTAINER(frame_perm), 5);

	table = gtk_table_new (3, 4, FALSE);
	gtk_table_set_col_spacing (GTK_TABLE(table), 0, 10);
	gtk_container_set_border_width (GTK_CONTAINER(table), 5);
	
	label = gtk_label_new ("User");  gtk_table_attach_defaults (GTK_TABLE(table), label, 0, 1, 0, 1);
	label = gtk_label_new ("Group"); gtk_table_attach_defaults (GTK_TABLE(table), label, 0, 1, 1, 2);
	label = gtk_label_new ("Other"); gtk_table_attach_defaults (GTK_TABLE(table), label, 0, 1, 2, 3);
	
	/*
	 * User   : R(2) W(1) X(0)
	 * Group  : R(5) W(4) X(3)
	 * Other  : R(8) W(7) X(6)
	 * Sticky : 9
	 * SGID   : 10
	 * SUID   : 11
	 */
	for (row = 0; row != 3; row++) {
		for (col = 0; col != 3; col++) {
			chkbox_perms[row*3+col] = gtk_check_button_new_with_label (perms[col]); 
			gtk_signal_connect (
					GTK_OBJECT(chkbox_perms[row*3+col]), 
					"toggled", 
					GTK_SIGNAL_FUNC(dialog_permissions_toggles), 
					GINT_TO_POINTER(row*3+col));
			/* Build table from right-bottom towards left-top because of binary inverted permissions */
			gtk_table_attach_defaults (
					GTK_TABLE(table), 
					chkbox_perms[row*3+col], 
					(3-col),(3-col)+1, 
					(2-row), (2-row)+1);
		}
	}

	for (row = 0; row != 3; row++) {
		chkbox_perms[9+row] = gtk_check_button_new_with_label (perms_special[row]); 
		gtk_signal_connect (
				GTK_OBJECT(chkbox_perms[9+row]),
				"toggled",
				GTK_SIGNAL_FUNC(dialog_permissions_toggles),
				GINT_TO_POINTER(9+row));
		gtk_table_attach_defaults (GTK_TABLE(table), chkbox_perms[9+row], 1, 2, row+3, row+4);
	}

	label = gtk_label_new ("Octal"); gtk_table_attach_defaults (GTK_TABLE(table), label, 0, 1, 7, 8);
	label = gtk_label_new ("Text");  gtk_table_attach_defaults (GTK_TABLE(table), label, 0, 1, 8, 9);
	
	entry_octal = gtk_entry_new_with_max_length (4);
	entry_text = gtk_entry_new_with_max_length (12);
	
	gtk_signal_connect (
			GTK_OBJECT(entry_octal),
			"focus_out_event",
			GTK_SIGNAL_FUNC(dialog_permissions_octal),
			NULL);
	gtk_signal_connect (
			GTK_OBJECT(entry_text), 
			"focus_out_event",
			GTK_SIGNAL_FUNC(dialog_permissions_text),
			NULL);

	gtk_table_attach_defaults (GTK_TABLE(table), entry_octal, 1, 3, 7, 8);
	gtk_table_attach_defaults (GTK_TABLE(table), entry_text , 1, 3, 8, 9);
	
	gtk_container_add (GTK_CONTAINER(frame_perm), table);

	vbox_options = gtk_vbox_new (FALSE, 0);
	
	chkbtn_recurse      = gtk_check_button_new_with_label ("Recurse into subdirectories");
//	chkbtn_preservedirs = gtk_check_button_new_with_label ("Preserve directories");
//	chkbtn_preserveexec = gtk_check_button_new_with_label ("Preserve executables");
	gtk_signal_connect (
			GTK_OBJECT(chkbtn_recurse),
			"toggled",
			GTK_SIGNAL_FUNC(dialog_permissions_toggleoption),
			&option_recurse);
	
//	radio_recurse_allfiles = gtk_radio_button_new_with_label (NULL, "All files");
//	group = gtk_radio_button_group (GTK_RADIO_BUTTON(radio_recurse_allfiles));
//	radio_recurse_xonly = gtk_radio_button_new_with_label (group, "Execute on directories and executables only");

	gtk_box_pack_start (GTK_BOX(vbox_options), chkbtn_recurse     , FALSE, TRUE, 1);
//	gtk_box_pack_start (GTK_BOX(vbox_options), chkbtn_preservedirs, FALSE, TRUE, 1);
//	gtk_box_pack_start (GTK_BOX(vbox_options), chkbtn_preserveexec, FALSE, TRUE, 1);
	
//	gtk_box_pack_start (GTK_BOX(vbox_recurse), radio_recurse_allfiles, FALSE, TRUE, 1);
//	gtk_box_pack_start (GTK_BOX(vbox_recurse), radio_recurse_xonly, FALSE, TRUE, 1);
	
	gtk_container_add (GTK_CONTAINER(frame_options), vbox_options);
	
	hbox = gtk_hbox_new(TRUE, 0);
	
	btn_ok = gtk_button_new_with_label ("Ok");
	btn_cancel = gtk_button_new_with_label ("Cancel");

	gtk_signal_connect (
			GTK_OBJECT(btn_ok),
			"clicked",
			GTK_SIGNAL_FUNC(dialog_permissions_cb_ok_clicked),
			NULL);
	gtk_signal_connect (
			GTK_OBJECT(btn_cancel),
			"clicked",
			GTK_SIGNAL_FUNC(dialog_permissions_cb_cancel_clicked),
			&cancel);

	gtk_box_pack_start (GTK_BOX(hbox), btn_ok, FALSE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(hbox), btn_cancel, FALSE, TRUE, 2);
	
	gtk_box_pack_start (GTK_BOX(vbox), frame_perm, FALSE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(vbox), frame_options, FALSE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(vbox), hbox, FALSE, TRUE, 2);

	gtk_container_add (GTK_CONTAINER(window), vbox);

	gtk_widget_show_all (window);
	gtk_widget_grab_focus (entry_octal);

	perms_int = initial_mode; 
	dialog_permissions_update();
	
	gtk_main();

	gtk_widget_destroy(window);

	if (return_mode != NULL) {
		*return_mode = perms_int;
	}
	if (option_recurse == 1) {
		*return_options = OPT_CHMOD_RECURSE;
	}

	return (cancel);
	
}

void dialog_ownership_cb_ok_clicked (void) {
	gtk_main_quit();
}

void dialog_ownership_cb_cancel_clicked (GtkWidget *widget, int *cancel) {
	*cancel = 1;
	gtk_main_quit();
}

void dialog_ownership_toggleoption (GtkWidget *widget, int *data) {
	*data ^= 1;
}

int dialog_ownership (uid_t uid, gid_t gid, uid_t *return_uid, gid_t *return_gid, int *return_options) {
	GtkWidget *window;
	GtkWidget *vbox, *hbox, *vbox_recurse;
	GtkWidget *frame_own, *frame_recurse;
	GtkWidget *table;
//	GtkWidget *label;
//	GSList *group;
//	GtkWidget *radio_recurse_allfiles;
//	GtkWidget *radio_recurse_xonly;
	GtkWidget *btn_ok, *btn_cancel;
	GtkWidget *combo_users, *combo_groups;
	GList *list_users = NULL, *list_groups = NULL;
	struct passwd *users = NULL;
	struct group *groups = NULL;
	int cancel = 0;
	int option_recurse;
	GtkWidget *chkbtn_recurse = 0;

	
	window = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_window_set_position (GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_title (GTK_WINDOW(window), "Ownership");

	vbox = gtk_vbox_new (FALSE, 0);
	
	frame_own = gtk_frame_new ("User and group");
	gtk_container_set_border_width (GTK_CONTAINER(frame_own), 5);
	
	frame_recurse = gtk_frame_new ("Recursion");
	gtk_container_set_border_width (GTK_CONTAINER(frame_own), 5);

	table = gtk_table_new (3, 4, FALSE);
	gtk_table_set_col_spacing (GTK_TABLE(table), 0, 10);
	gtk_container_set_border_width (GTK_CONTAINER(table), 5);
	

	/* Load users and groups */
	/* next operations should be atomic, but that's not possible? */
	setpwent();
	while ((users = getpwent())) {
			list_users = g_list_insert_sorted (list_users, strdup(users->pw_name), (GCompareFunc)strcmp);
	}
	endpwent();

	setgrent();
	while ((groups = getgrent())) {
			list_groups = g_list_insert_sorted (list_groups, strdup(groups->gr_name), (GCompareFunc)strcmp);
	}
	endgrent();

	combo_groups = gtk_combo_new();
	combo_users  = gtk_combo_new();
	
	if (list_users) {
		gtk_combo_set_popdown_strings (GTK_COMBO(combo_users), list_users);
	}
	if (list_groups) {
		gtk_combo_set_popdown_strings (GTK_COMBO(combo_groups), list_groups);
	}

	g_list_free (list_users);
	g_list_free (list_groups);
	/* FIXME: Free dynamically alloced memory (during getpwent() loop) */
	
	gtk_table_attach_defaults (GTK_TABLE(table), combo_users, 0, 1, 0, 1);
	gtk_table_attach_defaults (GTK_TABLE(table), combo_groups, 0, 1, 1, 2);

	gtk_container_add (GTK_CONTAINER(frame_own), table);

	vbox_recurse = gtk_vbox_new (FALSE, 0);
	
//	radio_recurse_allfiles = gtk_radio_button_new_with_label (NULL, "All files");
//	group = gtk_radio_button_group (GTK_RADIO_BUTTON(radio_recurse_allfiles));
//	radio_recurse_xonly = gtk_radio_button_new_with_label (group, "Execute on directories and executables only");
	chkbtn_recurse      = gtk_check_button_new_with_label ("Recurse into subdirectories");
//	chkbtn_preservedirs = gtk_check_button_new_with_label ("Preserve directories");
//	chkbtn_preserveexec = gtk_check_button_new_with_label ("Preserve executables");
	gtk_signal_connect (
			GTK_OBJECT(chkbtn_recurse),
			"toggled",
			GTK_SIGNAL_FUNC(dialog_ownership_toggleoption),
			&option_recurse);

	gtk_box_pack_start (GTK_BOX(vbox_recurse), chkbtn_recurse, FALSE, TRUE, 1);
	
	gtk_container_add (GTK_CONTAINER(frame_recurse), vbox_recurse);
	
	hbox = gtk_hbox_new(TRUE, 0);
	
	btn_ok = gtk_button_new_with_label ("Ok");
	btn_cancel = gtk_button_new_with_label ("Cancel");

	gtk_signal_connect (
			GTK_OBJECT(btn_ok),
			"clicked",
			GTK_SIGNAL_FUNC(dialog_ownership_cb_ok_clicked),
			NULL);
	gtk_signal_connect (
			GTK_OBJECT(btn_cancel),
			"clicked",
			GTK_SIGNAL_FUNC(dialog_ownership_cb_cancel_clicked),
			&cancel);

	gtk_box_pack_start (GTK_BOX(hbox), btn_ok, FALSE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(hbox), btn_cancel, FALSE, TRUE, 2);
	
	gtk_box_pack_start (GTK_BOX(vbox), frame_own, FALSE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(vbox), frame_recurse, FALSE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(vbox), hbox, FALSE, TRUE, 2);

	gtk_container_add (GTK_CONTAINER(window), vbox);

	gtk_widget_show_all (window);

	gtk_widget_grab_focus (GTK_COMBO(combo_users)->entry);

	gtk_main();

	/* Return new UID en GID */
	if (( users = getpwnam(gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(combo_users)->entry))) )) {
		*return_uid = users->pw_uid;	
	} else {
		/* FIXME: Report error */
		*return_uid = -1;
	}
		
	if (( groups = getgrnam(gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(combo_groups)->entry))) )) {
		*return_gid = groups->gr_gid;
	} else {
		/* FIXME: Report error */
		*return_gid = -1;
	}
	
	if (option_recurse == 1) {
		*return_options = OPT_CHOWN_RECURSE;
	}
	
	
	gtk_widget_destroy(window);

	return (cancel);
	
}
