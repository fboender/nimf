/* Functions and event handlers for the main window 
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
 */

/* Handle keypresses in the main window. */
void main_ev_keypress (
		GtkObject *widget,  /* Widget in which the key was pressed           */
		GdkEventKey *event, /* Key that was pressed                          */
		void *data);        /* Unused pointer to extra data                  */

/* Handle the delete_event for main window: quit the application. */
int main_ev_quit (
		GtkWidget *widget, /* Widget that caused the event                   */
		GdkEvent *event,   /* Event which was generated                      */
		gpointer data);    /* Pointer to extra data (NULL)                   */

void main_ev_resize (
		GtkObject *widget,
		GtkAllocation *allocation,
		void *data);

GtkWidget *menu_bar (GtkWidget *mainwindow);

/* Returns a hbox with available operations (quick help) */
GtkWidget *operations_bar (void);

/* Returns a label with Nimf information */
GtkWidget *title_bar (void);

GtkWidget *cmdline_bar (void);
