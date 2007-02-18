/* Nimf
 *
 * Non Irritating Manager of Files
 *
 * Description: Norton Commander like file manager
 * Author     : Ferry Boender <%%EMAIL>
 * Homepage   : %%HOMEPAGE
 *
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
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

#include "bookmarks.h"
#include "config.h"
#include "panel.h"
#include "cmdline.h"
#include "file.h"
#include "operations.h"
#include "nimf.h"
#include "dialogs.h"
#include "settings.h"
#include "sysinfo.h"
#include "gtkaddons.h"

panel *panels[2];          /* array of panel structs */
cmdline_ *cmdline;         /* Command line */
int panel_current;         /* Array index of current active panel */
GList *config_keys = NULL; /* Key configuration */
GList *config_ext = NULL;  /* Extentions configuration */
GList *config_nimf = NULL; /* Main configuration */
GList *bookmarks = NULL;   /* Bookmarks */

static GtkItemFactoryEntry menu_items[] = {
	{ "/_File",           NULL,         NULL,          0, "<Branch>" },
	{ "/File/Quit",       "<control>Q", gtk_main_quit, 0, NULL },
	{ "/_Settings",       NULL,         NULL,          0, "<Branch>" },
	{ "/Settings/Misc",   NULL,         settings,      SETTINGS_MISC, NULL },
	{ "/Settings/Actions",NULL,         settings,      SETTINGS_ACTIONS, NULL },
	{ "/Settings/Keys",   NULL,         settings,      SETTINGS_KEYS, NULL },
	{ "/_Help",           NULL,         NULL,          0, "<LastBranch>" },
	{ "/_Help/About",     NULL,         dialog_about,  0, NULL },
};

/* FIXME: Move this to somewhere else?! */
char *key_action_mapping[] = {
	"operation_help", 
	"operation_view", 
	"operation_edit", 
	"operation_copy", 
	"operation_move", 
	"operation_mkdir", 
	"operation_delete", 
	"operation_symlink",
	"operation_quit", 
	"operation_rename", 
	"operation_permissions", 
	"operation_ownership", 
	"sel_toggle",
	"sel_toggleall",
	"sel_pattern_plus",
	"sel_pattern_minus",
	"cursor_up",
	"cursor_down",
	"cursor_top",
	"cursor_bottom",
	"action_dirup",
	"action_run",
	"focus_pathentry",
	"focus_cmdline",
	"toggle_hidden",
	"refresh",
	"home",
	"root",
	"mount",
	"jumpsearch_cycle",
	"panel_cycle",
	"toggle_cmd_output",
	"selection_to_cmdline",
	NULL 
};

GtkWidget *vpaned; /* FIXME: Oi, this doesn't belong here, but I'm lazy! :) */
GtkWidget *mainwindow;

int main (int argc, char *argv[]) {
	GtkWidget *box_panels;
	GtkWidget *box_bars;
	char *config_location_keys, *config_location_ext, *config_location_nimf, 
	     *config_panel_current, *bookmarks_location;
	
	gtk_init (&argc, &argv);
	
	/* Create config dir at first nimf start */
	config_location_nimf = file_fullpath(getenv("HOME"), ".nimf");
	config_create (config_location_nimf);
	free (config_location_nimf);
	
	/* Load configuration */
	config_location_keys = file_fullpath(getenv("HOME"), ".nimf/keys");
	config_location_ext = file_fullpath(getenv("HOME"), ".nimf/extentions");
	config_location_nimf = file_fullpath(getenv("HOME"), ".nimf/nimf");
	bookmarks_location = file_fullpath(getenv("HOME"), ".nimf/bookmarks");
  
	if ((config_keys  = config_read (config_location_keys)) == NULL) {
		config_keys = config_create_keys();
	}
	if ((config_ext  = config_read (config_location_ext)) == NULL) {
		config_ext = config_create_ext();
	}
	if ((config_nimf = config_read (config_location_nimf)) == NULL) {
		config_nimf = config_create_nimf();
	}
	bookmarks = bookmarks_load (bookmarks_location);
	
	/* Create panels */
	panels[0] = panel_new(0);
	panels[1] = panel_new(1);
	
	/* Create commandline widget */
	cmdline = cmdline_new();
	
	panel_set_path (panels[0],
			config_find_value(config_nimf, "panel_dir_left", getenv("HOME")));
	if (panel_refresh (panels[0], 0) == -1) {
		panel_set_path (panels[0], "/");
		if (panel_refresh (panels[0],0 ) == -1) {
			printf ("Couldn't read path in left panel. Giving up");
			return(-1);
		}
	};
	panel_set_path (panels[1],
			config_find_value(config_nimf, "panel_dir_right", getenv("HOME")));
	if (panel_refresh (panels[1], 0) == -1) {
		panel_set_path (panels[1], "/");
		if (panel_refresh (panels[1], 0) == -1) {
			printf ("Couldn't read path in right panel. Giving up");
		}
	};

	panels[0]->next = panels[1];
	panels[1]->next = panels[0];

	sscanf (config_find_value(config_nimf, "panel_current", "0"), "%i", &panel_current);

	/* Construct the main window */
	mainwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW(mainwindow), "Nimf %%VERSION");

	gtk_window_set_default_size (
			GTK_WINDOW(mainwindow),
			atoi(config_find_value(config_nimf, "nimf_width", "300")),
			atoi(config_find_value(config_nimf, "nimf_height", "400")));
	
	gtk_signal_connect (GTK_OBJECT(mainwindow),
			"delete_event",
			GTK_SIGNAL_FUNC (main_ev_quit),
			NULL);
	gtk_signal_connect (GTK_OBJECT(mainwindow),
			"key_press_event",
			GTK_SIGNAL_FUNC (main_ev_keypress),
			mainwindow);
	gtk_signal_connect (GTK_OBJECT(mainwindow),
			"size-allocate",
			GTK_SIGNAL_FUNC (main_ev_resize),
			NULL);

	/* Panels (horizontal) */
	box_panels = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX(box_panels), panels[0]->vbox, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(box_panels), panels[1]->vbox, TRUE, TRUE, 0);

	/* Panels and cmdline (vertical; adjustable) */
	vpaned = gtk_vpaned_new ();
	gtk_paned_pack1 (GTK_PANED(vpaned), box_panels, TRUE, FALSE);
	gtk_paned_pack2 (GTK_PANED(vpaned), cmdline->vbox, FALSE, FALSE);
	gtk_paned_set_handle_size (GTK_PANED(vpaned), 0);

	/* Bars (vertical) */
	box_bars = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX(box_bars), menu_bar(mainwindow), FALSE, FALSE, 0);
	if (strcmp(config_find_value(config_nimf, "bar_title", "1"), "1") == 0) {
		gtk_box_pack_start (GTK_BOX(box_bars), title_bar(), FALSE, FALSE, 0);
	}
	gtk_box_pack_start (GTK_BOX(box_bars), vpaned, TRUE, TRUE, 0);
	if (strcmp(
				config_find_value(config_nimf, "bar_operations", "1"), 
				"1") == 0) {
		gtk_box_pack_start (GTK_BOX(box_bars), operations_bar(), FALSE, FALSE, 0);
	}

	gtk_container_add (GTK_CONTAINER(mainwindow), box_bars);
	gtk_widget_show_all (mainwindow);
	gtk_widget_hide (cmdline->output);
	gtk_widget_queue_resize (mainwindow);
	gtk_paned_set_position (GTK_PANED(vpaned), -1);

    gtk_widget_grab_focus (GTK_WIDGET(panels[panel_current]->list));
			
	gtk_main();
	
	/* Quit */
	
	/* Save configuration */
	config_panel_current = (char *)malloc(sizeof(char) * 2);
	sprintf (config_panel_current, "%i", panel_current);
	
	config_nimf = config_change_value (config_nimf, "panel_dir_left", panels[0]->path);
	config_nimf = config_change_value (config_nimf, "panel_dir_right", panels[1]->path);
	config_nimf = config_change_value (config_nimf, "panel_current", config_panel_current);
	config_save (config_nimf, config_location_nimf);
	config_save (config_ext, config_location_ext);
	config_save (config_keys, config_location_keys);
	bookmarks_save (bookmarks, bookmarks_location);

	free (config_panel_current);
	
	/* Clean up */
	free (panels[0]);
	free (panels[1]);
	free (config_location_ext);
	free (config_location_nimf);
	free (bookmarks_location);
	
	return (0);
}

/* Delete event for main window */
int main_ev_quit (
		GtkWidget *widget,
		GdkEvent *event,
		gpointer data) {

	gtk_main_quit();
	return (0);
}

/* key pressed in main window */
void main_ev_keypress (
		GtkObject *widget,
		GdkEventKey *event,
		void *data) {
	char *key_pressed;
	action_ action;
	int key_for_main = 0; /* set to true if a key for main was pressed */
	
	key_pressed = gtk_key_serialize(event);
	
	action = key_action_map (config_keys, key_pressed);

	if ((action = key_action_map (config_keys, key_pressed)) != -1) {
		switch (action) {
			case operation_quit :
				gtk_main_quit();
				break;
			case selection_to_cmdline:
				panel_selection_to_cmdline (panels[panel_current]);
				key_for_main = 1;
				break;
			case panel_cycle:
				panel_next(); /* FIXME: Not in panel.c? */
				key_for_main = 1;
				break;
			case toggle_cmd_output : 
				if (GTK_WIDGET_VISIBLE(cmdline->output) == 1) {
					/* Hide commandline output */
					/* FIXME: Move this to cmdline.c? */
					gtk_paned_set_handle_size (GTK_PANED(vpaned), 0);
					gtk_widget_hide (cmdline->output);
					// gtk_widget_hide (cmdline->vbox);
					gtk_widget_queue_resize(GTK_WIDGET(data)); 
					gtk_paned_set_position (GTK_PANED(vpaned), -1);
				} else {
					/* Show commandline output */
					/* FIXME: move this to cmdline.c? */
					gtk_paned_set_handle_size (GTK_PANED(vpaned), 10);
					gtk_widget_show (cmdline->output);
					//gtk_widget_show (vpaned);
				}
				
				key_for_main = 1;
				break;
			default:
				break;
		}
	}

	free (key_pressed);
	
	if (key_for_main == 1) {
		gtk_signal_emit_stop_by_name (widget, "key_press_event");
	}
}

/* Main window resized */
void main_ev_resize (
		GtkObject *widget,
		GtkAllocation *allocation,
		void *data) {
	char s[9];

	/* Save new size to configuration */
	sprintf (s, "%i", allocation->width);
	config_nimf = config_change_value (config_nimf, "nimf_width", s);
	sprintf (s, "%i", allocation->height);
	config_nimf = config_change_value (config_nimf, "nimf_height", s);
}

void buttonbar_click(GtkWidget *widget, int button) {
	panel *p;

	p = panels[panel_current];

	switch (button) {
		case 0: dialog_error ("Not yet implemented"); break;
		case 1: dialog_error ("Not yet implemented"); break;
		case 2: dialog_error ("Not yet implemented"); break;
		case 3: op_edit(panel_getselectedfilelist(p), p->path); break;
		case 4: op_start(OP_COPY, panel_getselectedfilelist(p), p->path, p->next->path, p); break;
		case 5: op_start(OP_MOVE, panel_getselectedfilelist(p), p->path, p->next->path, p); break;
		case 6: op_start(OP_MKDIR, panel_getselectedfilelist(p), p->path, p->next->path, p); break;
		case 7: op_start(OP_DEL, panel_getselectedfilelist(p), p->path, p->next->path, p); break;
		case 8: op_start(OP_LNK, panel_getselectedfilelist(p), p->path, p->next->path, p); break;
		case 9: gtk_main_quit(); break;
		case 10: op_start(OP_PERM, panel_getselectedfilelist(p), p->path, p->next->path, p); break;
		case 11: op_start(OP_OWN, panel_getselectedfilelist(p), p->path, p->next->path, p); break;
		default: printf ("unknown button\n"); break;
	}

	gtk_widget_grab_focus (GTK_WIDGET(p->list));
}

GtkWidget *menu_bar (GtkWidget *mainwindow) {
	GtkItemFactory *item_factory;
	GtkAccelGroup *accel_group;
	gint nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);

	accel_group = gtk_accel_group_new ();
	item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>",
			   accel_group);
	gtk_item_factory_create_items (item_factory, nmenu_items, menu_items, NULL);
	gtk_window_add_accel_group (GTK_WINDOW (mainwindow), accel_group);
	
	return gtk_item_factory_get_widget (item_factory, "<main>");
}

/* Return a label with help about main file operation keys */
GtkWidget *operations_bar (void) {
	GtkWidget *button_bar;
	GtkWidget *button[13];
	int i;

	button_bar = gtk_hbox_new (FALSE, 0);
	
//	button[0] = gtk_button_new_with_label ("F1 Help");
//	button[1] = gtk_button_new_with_label ("F2 ");
//	button[2] = gtk_button_new_with_label ("F3 View");
	button[3] = gtk_button_new_with_label ("F4 Edit");
	button[4] = gtk_button_new_with_label ("F5 Copy");
	button[5] = gtk_button_new_with_label ("F6 Move");
	button[6] = gtk_button_new_with_label ("F7 MkDir");
	button[7] = gtk_button_new_with_label ("F8 Delete");
	button[8] = gtk_button_new_with_label ("F9 SymLnk");
	button[9] = gtk_button_new_with_label ("F10 Exit");
	button[10] = gtk_button_new_with_label ("F11 Permissions");
	button[11] = gtk_button_new_with_label ("F12 Ownership");
	
	for (i=3; i < 12; i++) {
		gtk_signal_connect (
				GTK_OBJECT(button[i]),
				"clicked",
				GTK_SIGNAL_FUNC(buttonbar_click),
				GINT_TO_POINTER(i));
		gtk_box_pack_start (GTK_BOX(button_bar), button[i], 
			TRUE, TRUE, 0);
	}
	
	gtk_widget_show_all (button_bar);
	
	return button_bar;
}

/* Return a label with information about this nimf version */
GtkWidget *title_bar (void) {
	GtkWidget *title_bar;

	title_bar = gtk_label_new ("Nimf - Non Irritating Manager of Files. Version %%VERSION. Released under GPL, (c) Ferry Boender");
	gtk_widget_show (title_bar);
	
	return title_bar;
}
