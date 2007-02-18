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

/* Name    : dialog_about
 * Descr   : Create a default 'about' dialog 
 * Params  : *
 * Pre     : -
 * Returns : -
 * Notes   : -
 */
void dialog_about (GtkWidget *widget, gpointer data);

/* Name    : dialog_error_ev_button_clicked
 * Descr   : Event for error dialog
 * Params  : *
 * Pre     : -
 * Returns : -
 * Notes   : -
 */
void dialog_error_ev_button_clicked (GtkWidget *widget, GtkWidget *win); 

/* Name    : dialog_error
 * Descr   : Creates a dialog displaying an error TEXT
 * Params  : text = Error message to display
 * Pre     : -
 * Returns : -
 * Notes   : -
 */
void dialog_error (char *text);

/* Name    : dialog_report_error_ev_b_ok_clicked
 * Descr   : *
 * Params  : -
 * Pre     : -
 * Returns : -
 * Notes   : -
 */
void dialog_report_error_ev_b_ok_clicked (GtkWidget *widget, int *action); 

/* Name    : dialog_report_error_ev_b_cancel_clicked
 * Descr   : *
 * Params  : -
 * Pre     : -
 * Returns : -
 * Notes   : -
 */
void dialog_report_error_ev_b_cancel_clicked (GtkWidget *widget, int *action); 

/* Name    : dialog_report_error
 * Descr   : Shows a dialog with an error and containing some options 
 *           from which the user can choose
 * Params  : pre_msg = Additional error message text
 *           path    = the file or path on which the error occured
 *           error   = the actual error
 *           action  = pointer to int which will contain the option the user chose
 * Pre     : -
 * Returns : call by reference : action, containing the option the user chose
 * Notes   : Execution of GTK is stopped (gtk_main()) until the user chooses an option.
 *           This dialog is normally used during operations (copy) to show errors.
 */
void dialog_report_error (char *pre_msg, char *path, char *error, int *action);

/* Name    : 
 * Descr   : 
 * Params  : 
 * Pre     : -
 * Returns : 
 * Notes   : -
 */
void dialog_enterpath_cb_ok (GtkWidget *widget, gchar **path);

/* Name    : 
 * Descr   : 
 * Params  : 
 * Pre     : -
 * Returns : 
 * Notes   : -
 */
void dialog_enterpath_cb_cancel (GtkWidget *widget, gchar **path);

/* Name    : 
 * Descr   : 
 * Params  : 
 * Pre     : -
 * Returns : 
 * Notes   : -
 */
void dialog_enterpath_cb_key (GtkWidget *widget, GdkEventKey *event, char **path);

/* Name    : dialog_enterpath
 * Descr   : Presents the user with a dialog in which he can enter a path
 * Params  : title = Title of the dialog window
 *           message = the message to show
 *           preentry = Text to pre-fill the entry with (default value(
 *           path     = the path in which to store the user's input.
 * Pre     : -
 * Returns : newly allocated space to PATH containing the input of the user
 * Notes   : Ususally called before an operation, to determine where the user wants 
 *           the files to go.
 */
void dialog_enterpath(char *title, char *message, char *preentry, char **path);

/* Name    : dialog_progress
 * Descr   : 
 * Params  : 
 * Pre     : -
 * Returns : 
 * Notes   : -
 */
GtkWindow *dialog_progress (char *title, GtkWidget *label, GtkWidget *progress_bar);

int dialog_permissions (mode_t initial_mode, mode_t *return_mode, int *return_options);

void dialog_ownership_cb_ok_clicked (void);

void dialog_ownership_cb_cancel_clicked (GtkWidget *widget, int *cancel);

int dialog_ownership (uid_t uid, gid_t gid, uid_t *return_uid, gid_t *return_gid, int *return_options);

