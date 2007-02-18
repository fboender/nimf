#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>


#include "cmdline.h"
#include "panel.h"
#include "dialogs.h"

extern panel *panels[];
extern cmdline_ *cmdline;
extern int panel_current;
extern GtkWidget *vpaned;
extern GtkWidget *mainwindow;

void cmdline_cmd_run (char *dir, char *cmd) {
	char buf[1024];
	FILE *cmd_output;
	int pos;

	cmdline_output_open (cmdline);

	pos = gtk_editable_get_position (GTK_EDITABLE(cmdline->output));
	
	chdir (dir);
	cmd_output = popen (cmd, "r");
	
	while (fgets (buf, 1024, cmd_output)) {
		gtk_text_insert (GTK_TEXT(cmdline->output), cmdline->font_output, NULL, NULL, buf, -1);
	}

	pclose (cmd_output);
}

void cmdline_output_open (cmdline_ *cmdline) {
	if (GTK_WIDGET_VISIBLE(cmdline->output) != 1) {
		gtk_paned_set_handle_size (GTK_PANED(vpaned), 0);
		gtk_widget_show (cmdline->output);
	}
}
void cmdline_output_close (cmdline_ *cmdline) {
	if (GTK_WIDGET_VISIBLE(cmdline->output) == 1) {
		gtk_paned_set_handle_size (GTK_PANED(vpaned), 0);
		gtk_widget_hide (cmdline->output);
		
		gtk_widget_queue_resize(GTK_WIDGET(mainwindow)); /* redraw widgets */
		gtk_paned_set_position (GTK_PANED(vpaned), -1);
	}
}
void cmdline_output_toggle (cmdline_ *cmdline) {
	if (GTK_WIDGET_VISIBLE(cmdline->output) == 1) {
		cmdline_output_close(cmdline);
//		gtk_paned_set_handle_size (GTK_PANED(vpaned), 0);
//		gtk_widget_hide (cmdline->output);
//		
//		gtk_widget_queue_resize(GTK_WIDGET(data)); /* redraw widgets */
//		gtk_paned_set_position (GTK_PANED(vpaned), -1);
	} else {
		cmdline_output_open (cmdline);
//		gtk_paned_set_handle_size (GTK_PANED(vpaned), 10);
//		gtk_widget_show (cmdline->output);
	}
}

void cmdline_cmd_activate (GtkWidget *widget, cmdline_ *cmdline) {
	char *cmd;
	char *full_cmd;

	cmd = gtk_entry_get_text (GTK_ENTRY(cmdline->cmd));
	full_cmd = malloc (sizeof(char) * (strlen(cmd) + 18));
	sprintf (full_cmd, "%s&", cmd);

	cmdline_cmd_run (panels[panel_current]->path, full_cmd);
	
	gtk_entry_set_text (GTK_ENTRY(cmdline->cmd), "");

}

void cmdline_cmd_ev_keypress (GtkWidget *widget, GdkEventKey *event, void *data) {
	if (event->keyval == GDK_Escape) {
		gtk_widget_grab_focus (GTK_WIDGET(panels[panel_current]->list));
	}
}

cmdline_ *cmdline_new(void) {
	cmdline_ *cmdline = malloc(sizeof(cmdline_));

	cmdline->vbox = gtk_vbox_new(FALSE, 0);
	cmdline->hbox = gtk_hbox_new(FALSE, 0);
	cmdline->prompt = gtk_label_new ("");
	cmdline->cmd = gtk_entry_new ();
	cmdline->output = gtk_text_new (NULL,NULL);

	gtk_text_set_editable (GTK_TEXT(cmdline->output), FALSE);

	gtk_signal_connect (
			GTK_OBJECT(cmdline->cmd),
			"key-press-event",
			GTK_SIGNAL_FUNC (cmdline_cmd_ev_keypress),
			NULL);
	gtk_signal_connect (
			GTK_OBJECT(cmdline->cmd), 
			"activate",
			GTK_SIGNAL_FUNC (cmdline_cmd_activate),
			cmdline);

	gtk_box_pack_start (GTK_BOX(cmdline->hbox), cmdline->prompt, FALSE, FALSE, 5);
	gtk_box_pack_start (GTK_BOX(cmdline->hbox), cmdline->cmd, TRUE, TRUE, 5);

	gtk_box_pack_start (GTK_BOX(cmdline->vbox), cmdline->output, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(cmdline->vbox), cmdline->hbox, FALSE, FALSE, 0);

//	gtk_widget_unrealize (cmdline->output);
	
	/* Output font */
	if ((cmdline->font_output = gdk_font_load ("fixed")) == NULL) {
		dialog_error ("Couldn't load output font");
	}
	
	return (cmdline);
}

void cmdline_setpath(cmdline_ *cmdline, char *path) {
	gtk_label_set_text (GTK_LABEL(cmdline->prompt), path);
}

void cmdline_insertfilename(cmdline_ *cmdline, char *filename) {
}

char *cmdline_getcmd (cmdline_ *cmdline) {
	return (gtk_entry_get_text(GTK_ENTRY(cmdline->cmd)));
}

