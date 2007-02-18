/* panel handling */
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

extern GList *config_nimf;

/* This struct contains all data needed for a panel to work correctly. It will 
 * probably be needed throughout the different sources.
 * NOTICE: seperate .h file for this? 
 */
typedef struct panel {
	int id;                          /* ID field for panel (unused) */
	char *path;                      /* Current path for the panel */
	time_t path_mtime;               /* Change timestamp at dir read */
	int nroffiles;                   /* Number of files (and dirs) in panel */
	int showhiddenfiles;             /* Toggle for display of hidden files */
	char *jumpfile;                  /* Partial filename typed by user (jump) */
	int jumpfile_skip;               /* Skip initial matches during jump (cycle) */
	int prevselectedrow;             /* Previous selected row (for multi select)*/
	int refresh;                     /* Set to 1 if the panel needs to refresh */
	int operation_busy;              /* 1 if operation busy. Do not refresh */
	GList *selected_files;           /* Glist with selected files */
	GtkWidget *bar_top;              /* [H][pathentry][..][/][~] */
	GtkWidget *winscroll;            /* Scrolled win for clist */
	GtkWidget *list;                 /* The actual list. */
	GtkWidget *vbox;                 /* bar_top | winscroll | info */
	GtkWidget *pathentry;            /* Entry for path */
	GtkWidget *button_dirrefresh;    /* Button for refreshing the panel */
	GtkWidget *button_togglehidden;  /* Button for hidden toggle */
	GtkWidget *button_dirup;         /* Button for dir up */
	GtkWidget *button_dirroot;       /* Button for dir root */
	GtkWidget *button_dirhome;       /* Button for dir home */
	GtkWidget *button_bookmark;      /* Button for bookmark toggle (add/remove) */
	GtkWidget *info;                 /* Entry for file information dispaly */
	struct panel *next;              /* Pointer to next panel */
} panel;

/** EVENTS **/

/* Enter pressed in pathentry widget in a panel */
void panel_ev_pathentry_activate (
		GtkEntry *widget,
		panel *p);

/* Key pressed in list. */
void panel_ev_list_keypress (
		GtkCList *widget,
		GdkEventKey *event, 
		panel *p);

/* A row was selected in a panel. (mouse click, cursor moved, etc) */
gboolean panel_ev_list_select_row (
		GtkCList *widget,
		int row,
		int col,
		GdkEventButton *event,
		panel *p);

/* The refresh button was clicked. */
void panel_ev_button_refresh_clicked (
		GtkCList *widget,
		panel *p);

/* The show/hide-hidden-files button was clicked. */
void panel_ev_button_togglehidden_clicked (
		GtkWidget *widget,
		panel *p);

/* Dir up button clicked */
void panel_ev_button_dirup_clicked (
		GtkCList *widget,
		panel *p);

/* */
void panel_ev_button_dirroot_clicked (
		GtkCList *widget,
		panel *p);

/* */
void panel_ev_button_dirhome_clicked (
		GtkCList *widget,
		panel *p);

/** ACTIONS (usually called by EVENTS functions */

/* Makes the current working dir in panel one higher by searching for the first
 * occurence of a '/' in the path (from the right) and stripping everything of
 * the path from that position */
void panel_dirup (
		panel *p);

/* Makes the current working dir in panel the root ( / ) */
void panel_dirroot (
		panel *p);

/* Makes the current working dir in panel the users HOME by 
 * reading the enviroment variable */
void panel_dirhome (
		panel *p);
	
/* Create a new filemanager panel with the appropriate GTK widgets. Returns a 
 * pointer to a newly allocated panel memory block. Should be freed manually */
panel *panel_new (int id);

int panel_refresh_timeout (panel *p);

/* Reads a dir's filenames and adds them to a panel's list. P->LIST is filled 
 * with the files in PATH and PATH is copied to P->PATH */
int panel_readdir (
		panel *p,
		char *path);

int panel_updatedir (panel *p);

/* Rescan a panel's path. Refills P->LIST */
int panel_refresh (panel *p, int force);

void panel_sort (panel *p);

int panel_sort_name (GtkCList *list, gconstpointer row1, gconstpointer row2);
int panel_sort_size (GtkCList *list, gconstpointer row1, gconstpointer row2);

/* Execute the file(s) which are selected. For directories, read the selected
 * dir. Other actions may be connected to various different files */
void panel_filerun (panel *p);

/* Move the panel's cursor to ROW */
void panel_list_goto_row (GtkWidget *clist, int row);

/* Move the panel's cursor to the first file in panel */
void panel_list_totop (panel *p);

/* Move the panel's curosr to the last file in panel */
void panel_list_tobottom (panel *p);

/* Handles the setting of PATH for a panel struct */
void panel_set_path (panel *p, char *path);

/* Move the focus to the next panel */
void panel_next (void);

/* NOTICE: temp function.. */
void panel_showselected (panel *p);

/* Updates the info entry at the bottom of a panel. Shows file or selection
 * info */
void panel_info_update (panel *p);

/* Returns the row nr which contains SRCHTEXT */
int panel_find_row_from_text (panel *p, char *srchtext);

/* Jump to a file which is typed by user */
int panel_list_jump (panel *p, GdkEventKey *key);

void panel_colorrow (panel *p, int rownr);
	
void panel_showselection (panel *p);

void panel_toggleselectcurrent(panel *p);

void panel_toggleselectrow (panel *p, int rownr);

void panel_selectrow (panel *p, int rownr);

void panel_unselectrow (panel *p, int rownr);

void panel_selectinvertall (panel *p);

void panel_selectall (panel *p);

void panel_selectnone (panel *p);

GList *panel_getselectedfilelist (panel *p);

void panel_disable (panel *p);
void panel_enable (panel *p);
char *panel_get_cursorfile (panel *p);
int panel_file_insert (panel *p, char *path, char *filename);
int panel_file_remove (panel *p, char *filename);
int panel_change_dir (panel *p, char *newpath);
void panel_select_pat (panel *p);
void panel_unselect_pat (panel *p);
void panel_toggle_hidden (panel *p);
void panel_ev_pathentry_focussed (GtkWidget *widget, panel *p);
char *panel_selection_to_string (panel *p);
void panel_selection_to_cmdline (panel *p);

