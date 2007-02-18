/* context.c handles all context sensitive pop-up menu's.
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
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

void contextmenu_deactivate_cb (GtkWidget *widget, void *data) {
	printf ("deactivate\n");
	gtk_main_quit();

}
void contextmenu_file (GdkEventButton *event, GtkWidget *widget, char *path, char* filename) {
	GtkWidget *menu;
	GtkWidget *menu_items;

	menu = gtk_menu_new();
	gtk_signal_connect_after(
			GTK_OBJECT(menu),
			"deactivate",
			GTK_SIGNAL_FUNC(contextmenu_deactivate_cb),
			NULL);
	menu_items = gtk_menu_item_new_with_label ("bier");
	gtk_menu_append (GTK_MENU (menu), menu_items);
	gtk_widget_show (menu_items);
    gtk_widget_show (menu);

	gtk_menu_popup (GTK_MENU(menu), NULL, NULL, NULL, NULL, event->button, event->time);
	gtk_main();
	gtk_menu_popdown (GTK_MENU(menu));
	gtk_widget_grab_focus(widget);

	printf ("context!\n");
	
}


