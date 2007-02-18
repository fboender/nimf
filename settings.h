#define SETTINGS_MISC 1
#define SETTINGS_ACTIONS 2
#define SETTINGS_KEYS 3

typedef enum {
	SETTING_TOGGLE,
	SETTING_INT,
	SETTING_STRING,
} SETTING_TYPE_;

void settings_actions(void);
void settings (GtkWidget *widget, gpointer data);
void settings_actions_populate (GtkWidget *list);
void settings_keys_edit_populate_actions (GtkCombo *combo);
void settings_keys_populate (GtkWidget *list) ;
