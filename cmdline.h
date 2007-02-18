typedef struct {
	GtkWidget *vbox;
	GtkWidget *output;
	GtkWidget *prompt;
	GtkWidget *cmd;
	GtkWidget *hbox;
	GdkFont *font_output;
} cmdline_;

cmdline_ *cmdline_new(void);
void cmdline_setpath(cmdline_ *cmdline, char *path);
void cmdline_insertfilename(cmdline_ *cmdline, char *filename);
char *cmdline_getcmd (cmdline_ *cmdline);
void cmdline_output_open (cmdline_ *cmdline);
void cmdline_output_close (cmdline_ *cmdline);
void cmdline_output_toggle (cmdline_ *cmdline);

