#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>

#include "settings.h"
#include "config.h"
#include "gtkaddons.h"


extern GList *config_ext;
extern GList *config_nimf;
extern GList *config_keys;
extern char *key_action_mapping[];

static GtkWidget *entry_ext, *entry_action, *entry_title;
static GtkWidget *win_action_edit;
static GtkWidget *win_action;
static GtkWidget *win_keys;
static GtkWidget *win_misc;

void cb_disabled (GtkWidget *widget, void *data) {
	/* nothing */
}

void ok (GtkWidget *widget, config_keyval **action) {
	*action = malloc(sizeof(struct config_keyval));
	
	(*action)->key = strdup(gtk_entry_get_text (GTK_ENTRY(entry_ext)));
	(*action)->value = strdup(gtk_entry_get_text (GTK_ENTRY(entry_action)));

	gtk_widget_destroy(win_action_edit);
	gtk_main_quit();
}

void settings_actions_edit (config_keyval **action, char *key, char *value) {
	GtkWidget *btn_ok, *btn_cancel;
	GtkWidget *table;
	GtkWidget *vbox;
	GtkWidget *hbox_btn, *vbox_entry;
	GtkWidget *label_ext, *label_action;

	win_action_edit = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title (GTK_WINDOW(win_action_edit), "Settings - Actions - Edit");
	gtk_signal_connect (
			GTK_OBJECT (win_action_edit),
			"delete_event",
			GTK_SIGNAL_FUNC(cb_disabled),
			NULL);

	table = gtk_table_new (2,2, FALSE);
	
	entry_ext = gtk_entry_new ();
	entry_action = gtk_entry_new ();
	entry_title = gtk_entry_new ();

	label_ext = gtk_label_new ("Extention");
	label_action = gtk_label_new ("Action");

	btn_ok = gtk_button_new_with_label ("Ok");
	btn_cancel = gtk_button_new_with_label ("Cancel");

	gtk_signal_connect (
			GTK_OBJECT(btn_ok),
			"clicked",
			GTK_SIGNAL_FUNC(ok),
			action);
		
	vbox = gtk_vbox_new (FALSE, 0);
	hbox_btn = gtk_hbox_new (FALSE, 0);
	vbox_entry = gtk_vbox_new (FALSE, 0);
		
	gtk_table_attach_defaults (GTK_TABLE(table), label_ext, 0, 1, 0, 1);
	gtk_table_attach_defaults (GTK_TABLE(table), entry_ext, 1, 2, 0, 1);
	gtk_table_attach_defaults (GTK_TABLE(table), label_action, 0, 1, 1, 2);
	gtk_table_attach_defaults (GTK_TABLE(table), entry_action, 1, 2, 1, 2);

	gtk_box_pack_start (GTK_BOX(hbox_btn), btn_ok,   TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(hbox_btn), btn_cancel,  TRUE, TRUE, 2);

	gtk_box_pack_start (GTK_BOX(vbox), table, FALSE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(vbox), hbox_btn, FALSE, TRUE, 2);
	

	gtk_container_add (GTK_CONTAINER(win_action_edit), vbox);

	gtk_table_set_homogeneous (GTK_TABLE(table), FALSE);

	if (key) {
		gtk_entry_set_text (GTK_ENTRY(entry_ext), key);
	}
	if (value) {
		gtk_entry_set_text (GTK_ENTRY(entry_action), value);
	}

	gtk_widget_show_all (win_action_edit);
	gtk_widget_grab_focus (entry_ext);

}

void settings_actions_cb_delete (GtkWidget *widget, GtkWidget *list) {
	char *key;
	
	if (!GTK_CLIST(list)->selection) {
		return;
	}

	gtk_clist_get_text (
			GTK_CLIST(list),
			GPOINTER_TO_INT(GTK_CLIST(list)->selection->data),
			0,
			&key);

	config_ext = config_delitem (config_ext, key);
	gtk_clist_clear (GTK_CLIST(list));
	settings_actions_populate (list);
}
	
void settings_actions_cb_edit (GtkWidget *widget, GtkWidget *list) {
	config_keyval *new;
	char *key;

	if (!GTK_CLIST(list)->selection) {
		return; 
	}
	
	gtk_clist_get_text (
			GTK_CLIST(list),
			GPOINTER_TO_INT(GTK_CLIST(list)->selection->data),
			0,
			&key);
		
	settings_actions_edit (&new, key, config_find_value(config_ext, key, ""));
	gtk_main();
	
	if (config_find_value(config_ext, new->key, NULL) ==  NULL) {
		config_ext = config_newitem (config_ext, new->key, new->value);
	} else {
		config_ext = config_change_value(config_ext, new->key, new->value);
	}

	free (new->key);
	free (new->value);
	free (new);

	gtk_clist_clear (GTK_CLIST(list));
	settings_actions_populate (list);
}

void settings_actions_cb_new (GtkWidget *widget, GtkWidget *list) {
	config_keyval *new;

	settings_actions_edit (&new, NULL, NULL);
	gtk_main();
	
	config_ext = config_newitem (config_ext, new->key, new->value);

	free (new->key);
	free (new->value);
	free (new);

	gtk_clist_clear (GTK_CLIST(list));
	settings_actions_populate (list);
}

void settings_actions_cb_ok (GtkWidget *widget, void *data) {
	gtk_widget_destroy (win_action);
	gtk_main_quit();
}

void settings_actions(void) {
	GtkWidget *btn_edit, *btn_del, *btn_new, *btn_ok;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *action_list;
	GtkWidget *list_scroll;
	char *titles[3] = {
		"Extention",
		"Action"
	};
	
	win_action = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title (GTK_WINDOW(win_action), "Settings - Actions");
	gtk_window_set_default_size (GTK_WINDOW(win_action), 400, 400);

	gtk_signal_connect (
			GTK_OBJECT (win_action),
			"delete_event",
			GTK_SIGNAL_FUNC(cb_disabled),
			NULL);
	
	list_scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (
			GTK_SCROLLED_WINDOW(list_scroll),
			GTK_POLICY_AUTOMATIC,
			GTK_POLICY_ALWAYS);
	
	action_list = gtk_clist_new_with_titles(2, titles);
	gtk_clist_set_selection_mode (GTK_CLIST(action_list), GTK_SELECTION_BROWSE);
	gtk_clist_set_column_width (GTK_CLIST(action_list),
			1, 120);
	
	vbox = gtk_vbox_new (FALSE, 0);
	hbox = gtk_hbox_new (FALSE, 0);
		
	btn_edit = gtk_button_new_with_label ("Edit");
	btn_del = gtk_button_new_with_label ("Delete");
	btn_new = gtk_button_new_with_label ("New");
	btn_ok = gtk_button_new_with_label ("Ok");

	gtk_signal_connect (
			GTK_OBJECT(btn_edit),
			"clicked",
			GTK_SIGNAL_FUNC(settings_actions_cb_edit),
			action_list);
	gtk_signal_connect (
			GTK_OBJECT(btn_del),
			"clicked",
			GTK_SIGNAL_FUNC(settings_actions_cb_delete),
			action_list);
	gtk_signal_connect (
			GTK_OBJECT(btn_new),
			"clicked",
			GTK_SIGNAL_FUNC(settings_actions_cb_new),
			action_list);
	gtk_signal_connect (
			GTK_OBJECT(btn_ok),
			"clicked",
			GTK_SIGNAL_FUNC(settings_actions_cb_ok),
			NULL);

	gtk_box_pack_start (GTK_BOX(hbox), btn_edit, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(hbox), btn_del,  TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(hbox), btn_new,  TRUE, TRUE, 2);
//	gtk_box_pack_start (GTK_BOX(hbox), btn_ok,   TRUE, TRUE, 2);

	gtk_box_pack_start (GTK_BOX(vbox), list_scroll, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(vbox), hbox, FALSE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(vbox), btn_ok, FALSE, TRUE, 2);
	
	gtk_container_add (
			GTK_CONTAINER(list_scroll),
			action_list);

	gtk_container_add (GTK_CONTAINER(win_action), vbox);

	
	// Populate the list with the current available
	// action
	settings_actions_populate (action_list);
	gtk_widget_show_all (win_action);
	gtk_widget_grab_focus (action_list);
	gtk_main();

}

void settings_actions_populate (GtkWidget *list) {
	GList *actions;
	char *cols[3];

	actions = config_ext;

	while (actions) {
		config_keyval *item;
		
		item = actions->data;
		
		cols[0] = item->key;
		cols[1] = item->value;
		cols[2] = strdup ("");
	
		gtk_clist_append (GTK_CLIST(list), cols);
	
		actions = actions->next;
	}
}

void settings_misc_togglesetting_cb (GtkWidget *widget, char *setting_name) {
	char *toggle_value = NULL;

	toggle_value = malloc(sizeof(char) * 2);
	sprintf (toggle_value, "%i", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(widget)));
	config_change_value (config_nimf, setting_name, toggle_value);
	free (toggle_value);
}
void settings_misc_stringsetting_cb (GtkWidget *widget, GdkEventCrossing *event, char *setting_name) {
	config_change_value (config_nimf, setting_name, gtk_entry_get_text(GTK_ENTRY(widget)));
}

GtkWidget *settings_misc_addsetting (GtkWidget *table, int table_row_pos, SETTING_TYPE_ type, char *description, char *setting_name) {
	GtkWidget *label;
	GtkWidget *setting = NULL;
	char *setting_value = NULL;
	label = gtk_label_new (description);
	
	switch (type) {
		case SETTING_TOGGLE: 
			setting = gtk_check_button_new();
			setting_value = config_find_value(config_nimf, setting_name, "0");
			/* NOTE: Legacy shit from old configuration files */
			if (strcmp(setting_value, "yes") == 0 || strcmp(setting_value, "on") == 0 || strcmp(setting_value, "1") == 0) {
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(setting), TRUE);
			}
			gtk_signal_connect (
					GTK_OBJECT(setting),
					"toggled",
					GTK_SIGNAL_FUNC(settings_misc_togglesetting_cb),
					setting_name);
			break;
		case SETTING_STRING: 
			setting = gtk_entry_new();
			gtk_entry_set_text (GTK_ENTRY(setting), config_find_value(config_nimf, setting_name, ""));
			/* NOTE: Will the delete event not catch up with the focus-out-event and
			 * the handler function?? */
			gtk_signal_connect (
					GTK_OBJECT(setting),
					"focus-out-event",
					GTK_SIGNAL_FUNC(settings_misc_stringsetting_cb),
					setting_name);
			break;
		case SETTING_INT: 
			setting = gtk_entry_new();
			break;
	}

	gtk_table_attach_defaults (GTK_TABLE(table),   label, 0, 1, table_row_pos, table_row_pos + 1);
	gtk_table_attach_defaults (GTK_TABLE(table), setting, 1, 2, table_row_pos, table_row_pos + 1);

	return (setting);
}

void settings_misc_cb_ok (GtkWidget *widget, GtkWidget *win) {
	gtk_widget_destroy (win);
	gtk_main_quit();
}

void settings_misc (void) {
	GtkWidget *table;
	GtkWidget *btn_ok;
	GtkWidget *vbox, *hbox;
	GtkWidget *label;
	GtkWidget *first_setting;
	
	win_misc = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title (GTK_WINDOW(win_misc), "Settings - Actions");
	gtk_window_set_default_size (GTK_WINDOW(win_misc), 400, 164);

	gtk_signal_connect (
			GTK_OBJECT (win_misc),
			"delete_event",
			GTK_SIGNAL_FUNC(cb_disabled),
			NULL);
	
	vbox = gtk_vbox_new (FALSE, 0);
	hbox = gtk_hbox_new (FALSE, 0);
	/* NOTE: Shouldn't the nr of rows be dynamic?? */
	table = gtk_table_new (2,2, FALSE);
		
	btn_ok = gtk_button_new_with_label ("Ok");
	label = gtk_label_new ("Some settings may not take effect until you restart Nimf");

	first_setting = 
	settings_misc_addsetting (table, 1, SETTING_TOGGLE, "Show title bar", "bar_title");
	settings_misc_addsetting (table, 2, SETTING_TOGGLE, "Show operation bar", "bar_operations");
	settings_misc_addsetting (table, 3, SETTING_TOGGLE, "Sort directories first", "sort_dirsfirst");
	settings_misc_addsetting (table, 4, SETTING_TOGGLE, "Sort files/dirs case sensitive", "sort_casesensitive");
	settings_misc_addsetting (table, 5, SETTING_TOGGLE, "Jump search is case sensitive", "jump_case_sensitive");
	settings_misc_addsetting (table, 6, SETTING_TOGGLE, "Show keysym values (debug)", "show_keys");
	settings_misc_addsetting (table, 7, SETTING_STRING, "Editor", "editor");
	gtk_table_attach_defaults (GTK_TABLE(table), label, 0, 2, 0, 1);
	
	gtk_signal_connect (
			GTK_OBJECT(btn_ok),
			"clicked",
			GTK_SIGNAL_FUNC(settings_misc_cb_ok),
			win_misc);

	gtk_box_pack_start (GTK_BOX(hbox), btn_ok,   TRUE, TRUE, 2);

	gtk_box_pack_start (GTK_BOX(vbox), table, FALSE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(vbox), hbox, FALSE, TRUE, 2);
	
	gtk_container_add (GTK_CONTAINER(win_misc), vbox);

	gtk_widget_show_all (win_misc);
	gtk_widget_grab_focus (first_setting);
	gtk_main();
}

void settings_keys_edit_cb_ent_key(GtkWidget *widget, GdkEventKey *event, void *data) {
	GtkWidget *chk_ctrl, *chk_mod1, *chk_mod3;
	int msk_ctrl = 0, msk_mod1 = 0, msk_mod3 = 0;
	
	gtk_param_unpack (data, "ppp", &chk_ctrl, &chk_mod1, &chk_mod3);
	
	gtk_entry_set_text (GTK_ENTRY(widget), gdk_keyval_name(event->keyval));
	
	if ((event->state & GDK_CONTROL_MASK) > 0) { msk_ctrl = 1; } /* Ctrl */
	if ((event->state & GDK_MOD1_MASK) > 0)    { msk_mod1 = 1; } /* Alt */
	if ((event->state & GDK_MOD3_MASK) > 0)    { msk_mod3 = 1; } /* Win */
	
	gtk_signal_emit_stop_by_name (GTK_OBJECT(widget), "key_press_event");

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(chk_ctrl), msk_ctrl);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(chk_mod1), msk_mod1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(chk_mod3), msk_mod3);
}

void settings_keys_edit_cb_ok(GtkWidget *btn_ok, void *data) {
	GtkWidget *win_keys;
	GtkWidget *ent_key;
	GtkWidget *cmb_action;
	GtkWidget *chk_ctrl, *chk_mod1, *chk_mod3;
	GdkEventKey *key = malloc(sizeof(GdkEventKey));
	config_keyval *key_item;
	char *action = NULL;
	char *key_new;
	
	gtk_param_unpack (data, "ppppppp", 
			&key_item, &win_keys, &ent_key, 
			&chk_ctrl, &chk_mod1, &chk_mod3, 
			&cmb_action);

	key->state = 0;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chk_ctrl)) == 1) {
		key->state = key->state | GDK_CONTROL_MASK;
	}
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chk_mod1)) == 1) {
		key->state = key->state | GDK_MOD1_MASK;
	}
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chk_mod3)) == 1) {
		key->state = key->state | GDK_MOD3_MASK;
	}

	key->keyval = gdk_keyval_from_name (gtk_entry_get_text(GTK_ENTRY(ent_key)));
	key_new = gtk_key_serialize (key);
	action = strdup (gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(cmb_action)->entry)));

	/* Put new key and value from widgets in configuration */
	if (key_item != NULL) {
		free (key_item->key); 
		free (key_item->value); 
		key_item->key = strdup(key_new);
		key_item->value = strdup(action);
	} else {
		config_keys = config_newitem (config_keys, key_new, action);
	}

	gtk_widget_destroy (win_keys);
	
	gtk_main_quit();
}

void settings_keys_edit (config_keyval *key_item) {
	GtkWidget *win_keys;
	GtkWidget *tbl_keys;
	GtkWidget *btn_ok;
	GtkWidget *lbl_key, *lbl_mod, *lbl_action;
	GtkWidget *ent_key;
	GtkWidget *cmb_action;
	GtkWidget *chk_ctrl, *chk_mod1, *chk_mod3;

	win_keys   = gtk_window_new(GTK_WINDOW_DIALOG);
	tbl_keys   = gtk_table_new(4, 5, FALSE);
	btn_ok     = gtk_button_new_with_label ("Ok");
	lbl_key    = gtk_label_new ("Key:");
	lbl_mod    = gtk_label_new ("Modifiers:");
	lbl_action = gtk_label_new ("Action:");
	ent_key    = gtk_entry_new();
	cmb_action = gtk_combo_new();
	chk_ctrl   = gtk_check_button_new_with_label ("Control");
	chk_mod1    = gtk_check_button_new_with_label ("Alt");
	chk_mod3    = gtk_check_button_new_with_label ("Win");

	settings_keys_edit_populate_actions (GTK_COMBO(cmb_action));

	gtk_signal_connect (
			GTK_OBJECT(ent_key),
			"key_press_event",
			GTK_SIGNAL_FUNC(settings_keys_edit_cb_ent_key),
			gtk_param_pack ("ppp", chk_ctrl, chk_mod1, chk_mod3));
	gtk_signal_connect ( 
			GTK_OBJECT(btn_ok),
			"pressed",
			GTK_SIGNAL_FUNC(settings_keys_edit_cb_ok),
			gtk_param_pack ("ppppppp", key_item, win_keys, ent_key, chk_ctrl, chk_mod1, chk_mod3, cmb_action));

	gtk_table_set_row_spacings (GTK_TABLE(tbl_keys), 5);
	gtk_table_set_col_spacings (GTK_TABLE(tbl_keys), 5);

	gtk_table_attach_defaults (GTK_TABLE(tbl_keys), lbl_key   , 0,1, 0,1);
	gtk_table_attach_defaults (GTK_TABLE(tbl_keys), lbl_mod   , 0,1, 1,2);
	gtk_table_attach_defaults (GTK_TABLE(tbl_keys), lbl_action, 0,1, 2,3);
	gtk_table_attach_defaults (GTK_TABLE(tbl_keys), ent_key    , 1,7, 0,1);
	gtk_table_attach_defaults (GTK_TABLE(tbl_keys), chk_ctrl   , 1,2, 1,2);
	gtk_table_attach_defaults (GTK_TABLE(tbl_keys), chk_mod1   , 2,3, 1,2);
	gtk_table_attach_defaults (GTK_TABLE(tbl_keys), chk_mod3   , 4,5, 1,2);
	gtk_table_attach_defaults (GTK_TABLE(tbl_keys), cmb_action , 1,7, 2,3);
	gtk_table_attach_defaults (GTK_TABLE(tbl_keys), btn_ok     , 0,7, 3,4);

	gtk_container_add (GTK_CONTAINER(win_keys), tbl_keys);

	gtk_widget_show_all (win_keys);

	/* Convert config key to dialog widgets */
	if (key_item != NULL) {
		GdkEventKey *key;
		key = gtk_key_unserialize(key_item->key);

		if ((key->state & GDK_CONTROL_MASK) > 0) {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_ctrl),1);
		}
		if ((key->state & GDK_MOD1_MASK) > 0) {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_mod1),1);
		}
		if ((key->state & GDK_MOD3_MASK) > 0) {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_mod3),1);
		}
		
		gtk_entry_set_text (GTK_ENTRY(ent_key),gdk_keyval_name(key->keyval));
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(cmb_action)->entry), key_item->value);
	}

	gtk_widget_grab_focus (ent_key);
	gtk_main();
}

void settings_keys_edit_populate_actions (GtkCombo *combo) {
	int i = 0;
	GList *action_list = NULL;

	while (key_action_mapping[i] != NULL) {
		action_list = g_list_append (action_list, key_action_mapping[i]);
		i++;
	}
	gtk_combo_set_popdown_strings (combo, action_list);
}

void settings_keys_cb_edit (GtkWidget *widget, GtkCList *list) {
	config_keyval *key_item = NULL;
	
	if (!GTK_CLIST(list)->selection) {
		settings_keys_edit (key_item);
	} else {
		key_item = gtk_clist_get_row_data (GTK_CLIST(list), GPOINTER_TO_INT(GTK_CLIST(list)->selection->data));
		settings_keys_edit (key_item);
	}

	gtk_clist_clear (list);
	settings_keys_populate (GTK_WIDGET(list));
}

void settings_keys_cb_new (GtkWidget *widget, GtkCList *list) {
	settings_keys_edit (NULL);
	gtk_clist_clear (list);
	settings_keys_populate (GTK_WIDGET(list));
}

void settings_keys_cb_del (GtkWidget *widget, GtkCList *clst_keys) {
	config_keyval *key_item;
	
	if (GTK_CLIST(clst_keys)->selection) {
		key_item = gtk_clist_get_row_data (GTK_CLIST(clst_keys), GPOINTER_TO_INT(GTK_CLIST(clst_keys)->selection->data));
		config_keys = config_delitem (config_keys, key_item->key);
	}
	gtk_clist_clear (clst_keys);
	settings_keys_populate (GTK_WIDGET(clst_keys));
}

void settings_keys_cb_ok (GtkWidget *btn_ok, GtkWidget *win_keys) {
	gtk_widget_destroy(win_keys);
	gtk_main_quit();
}

void settings_keys(void) {
	GtkWidget *btn_edit, *btn_del, *btn_new, *btn_ok;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *clst_keys;
	GtkWidget *scrl_clst_keys;
	char *titles[3] = {
		"Modifiers",
		"Key",
		"Action"
	};
	
	win_keys = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title (GTK_WINDOW(win_keys), "Settings - Keys");
	gtk_window_set_default_size (GTK_WINDOW(win_keys), 400, 400);

	gtk_signal_connect (
			GTK_OBJECT (win_keys),
			"delete_event",
			GTK_SIGNAL_FUNC(cb_disabled),
			NULL);
	
	scrl_clst_keys = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (
			GTK_SCROLLED_WINDOW(scrl_clst_keys),
			GTK_POLICY_AUTOMATIC,
			GTK_POLICY_ALWAYS);
	
	clst_keys = gtk_clist_new_with_titles(3, titles);
	gtk_clist_set_column_auto_resize (GTK_CLIST(clst_keys), 0, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST(clst_keys), 1, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST(clst_keys), 2, TRUE);
	gtk_clist_set_selection_mode (GTK_CLIST(clst_keys), GTK_SELECTION_BROWSE);
	
	vbox = gtk_vbox_new (FALSE, 0);
	hbox = gtk_hbox_new (FALSE, 0);
		
	btn_edit = gtk_button_new_with_label ("Edit");
	btn_del = gtk_button_new_with_label ("Delete");
	btn_new = gtk_button_new_with_label ("New");
	btn_ok = gtk_button_new_with_label ("Ok");

	gtk_signal_connect (
			GTK_OBJECT(btn_edit),
			"clicked",
			GTK_SIGNAL_FUNC(settings_keys_cb_edit),
			clst_keys);
	gtk_signal_connect (
			GTK_OBJECT(btn_del),
			"clicked",
			GTK_SIGNAL_FUNC(settings_keys_cb_del),
			clst_keys);
	gtk_signal_connect (
			GTK_OBJECT(btn_new),
			"clicked",
			GTK_SIGNAL_FUNC(settings_keys_cb_new),
			clst_keys);
	gtk_signal_connect (
			GTK_OBJECT(btn_ok),
			"clicked",
			GTK_SIGNAL_FUNC(settings_keys_cb_ok),
			win_keys);

	gtk_box_pack_start (GTK_BOX(hbox), btn_edit, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(hbox), btn_del,  TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(hbox), btn_new,  TRUE, TRUE, 2);

	gtk_box_pack_start (GTK_BOX(vbox), scrl_clst_keys, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(vbox), hbox, FALSE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(vbox), btn_ok, FALSE, TRUE, 2);
	
	gtk_container_add (
			GTK_CONTAINER(scrl_clst_keys),
			clst_keys);

	gtk_container_add (GTK_CONTAINER(win_keys), vbox);

	// Populate the list with the current available action
	settings_keys_populate (clst_keys);
	gtk_widget_show_all (win_keys);

	gtk_widget_grab_focus (clst_keys);
	gtk_main();
	
}

void settings_keys_populate (GtkWidget *list) {
	GList *keys;
	char *cols[3];
	int added_rownr;

	keys = config_keys;
	
	while (keys) {
		char *modifiers = strdup("");
		config_keyval *item;
		GdkEventKey *modkey;
		
		item = keys->data;
		modkey = gtk_key_unserialize (item->key);

		if ((modkey->state & GDK_CONTROL_MASK) > 0) {
			modifiers = realloc (modifiers, sizeof(char) * (strlen(modifiers) + 10 + 1));
			strcat (modifiers, "Control + ");
		}
		if ((modkey->state & GDK_MOD1_MASK) > 0) {
			modifiers = realloc (modifiers, sizeof(char) * (strlen(modifiers) + 7 + 1));
			strcat (modifiers, "Mod1 + ");
		}
		if ((modkey->state & GDK_MOD3_MASK) > 0) {
			modifiers = realloc (modifiers, sizeof(char) * (strlen(modifiers) + 7 + 1));
			strcat (modifiers, "Mod3 + ");
		}

		cols[0] = modifiers;
		cols[1] = gdk_keyval_name(modkey->keyval);
		cols[2] = item->value;
		
		added_rownr = gtk_clist_append (GTK_CLIST(list), cols);
		gtk_clist_set_row_data (GTK_CLIST(list), added_rownr, item);
		
		keys = keys->next;
	}
}

void settings (GtkWidget *widget, gpointer data) {
	switch ((int)data) {
		case SETTINGS_MISC   : settings_misc(); break;
		case SETTINGS_KEYS   : settings_keys(); break;
		case SETTINGS_ACTIONS: settings_actions(); break;
	}
}
