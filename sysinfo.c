#include <stdio.h>
#include <gtk/gtk.h>

void sysinfo (void) {
	GtkWidget *win;
//	GtkWidget *list;
	GtkWidget *btn_ok;
	
	win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW(win), "System Information");

	btn_ok = gtk_button_new_with_label ("Ok");

}

