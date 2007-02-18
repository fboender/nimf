/* Functions for reading, setting and writing of configuration option */
/*
 * Copyright (C) 2002, 2003 Ferry Boender.
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
#include <malloc.h>
#include <string.h>
#include <gtk/gtk.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "dialogs.h"
#include "config.h"

extern char *key_action_mapping[];

int key_action_map (GList *config_keys, char *key_pressed) {
	int i = 0;
	char *key = NULL;

	key = config_find_value (config_keys, key_pressed, NULL);

	if (key != NULL) {
		while (key_action_mapping[i]) {
			if (strcmp(key_action_mapping[i], key) == 0) {
				return (i);
			}
			i++;
		}
	}
	
	return (-1);
}

int config_keycompare (config_keyval *listdata, char *finddata) {
	if (strcasecmp(listdata->key, finddata) == 0) {
		return (0);
	}
	return (1);
}

char *config_find_value (GList *configlist, char *key, char *default_val) {
	GList *tmp;
	config_keyval *found;
	
	tmp = g_list_find_custom (configlist, key, (GCompareFunc)config_keycompare);
	if (tmp != NULL) {
		found = g_list_nth_data(configlist, g_list_position(configlist, tmp));
		return (found->value);
	} else {
		return (default_val);
	}
}

config_keyval *config_get_item (GList *configlist, char *key) {
	GList *tmp;
	config_keyval *found;
	
	tmp = g_list_find_custom (configlist, key, (GCompareFunc)config_keycompare);
	if (tmp != NULL) {
		found = g_list_nth_data(configlist, g_list_position(configlist, tmp));
		return (found);
	} else {
		return (NULL);
	}
}

GList *config_delitem (GList *configlist, char *key) {
	config_keyval *item;

	item = config_get_item (configlist, key);
	
	free (item->key);
	free (item->value);

	return(g_list_remove (configlist, item));
}

GList *config_newitem (GList *configlist, char *key, char *val) {
	GList *conf;
	config_keyval *item;
	
	conf = configlist;
	
	item = malloc (sizeof(struct config_keyval));
			
	item->key = strdup (key);
	item->value = strdup (val);
	
	conf = g_list_append (conf, item);
	
	return (conf);
}

GList *config_change_key (GList *configlist, char *key, char *newkey) {
	GList *conf;
	config_keyval *item = NULL;
	int found = 0;

	conf = configlist;

	while (conf) {
		item = conf->data;
		if (strcmp(item->key, key) == 0) {
			free (item->key);
			item->key = strdup(newkey);
			found = 1;
			break;
		}
		conf = conf->next;
	}

	conf = configlist; /* no modification in list itself, only data */

	return (conf);
}

GList *config_change_value (GList *configlist, char *key, char *newvalue) {
	GList *conf;
	config_keyval *item = NULL;
	int found = 0;
	
	conf = configlist;
	
	while (conf) {
		item = conf->data;
		if (strcmp(item->key, key) == 0) {
			free (item->value);
			item->value = strdup(newvalue);
			found = 1;
			break;
		}
		conf = conf->next;
	}

	if (!found) {
		conf = config_newitem (configlist, key, newvalue);
	} else {
		conf = configlist;
	}

	return (conf);
}

char *config_get_key (char *line) {
	char *dest = NULL;
	int i;
	/* Find length of key */
	for (i=0; i!=strlen(line); i++) {
		if (line[i] == '=') {
			break;
		}
	}
	/* check if line is valid configuration option */
	if (i < strlen(line)) {
		dest = (char *) malloc (sizeof(char) * (i+2+9) );
		strncpy (dest, line, i);
		dest[i]='\0';
#ifdef DEBUG
		printf ("config_get_key: key=%s\n", dest);
#endif
		return (dest);
	} else {
#ifdef DEBUG
		printf ("config_get_key: [invalid key/no key]\n");
#endif
		return (NULL);
	}
}

char *config_get_value (char *line) {
	char *dest;
	char *i;

	i = strchr(line, '=');
	
	if (i == NULL) {
		return (NULL);
	} else {
		i++;
		if (i[strlen(i)-1] == '\n') {
			i[strlen(i)-1] = '\0';
		}

		dest = (char *) malloc(sizeof(char) * (strlen(i)+2));
		strcpy (dest, i);
		return (dest);
	}
	
}

GList *config_read (char *path) {
	FILE *f_in;
	char line[4096];
	config_keyval *item;
	GList *conf = NULL;
	
	f_in = fopen (path, "r");
	if (!f_in) {
		return (NULL);
	}
	while (fgets(line, 4094, f_in)) {
		int error = 0;
#ifdef DEBUG
		printf ("config_read: %s", line);
#endif
		if (line[0] != '#') {
			item = malloc (sizeof(struct config_keyval));
			
			if ((item->key = config_get_key (line)) == NULL) {
				error = 1;
			}
			if ((item->value = config_get_value (line)) == NULL) {
				error = 1;
			}

			if (error == 0) {
				conf = g_list_append (conf, item);
			} else {
				char *error = malloc(sizeof(char) * (30 + strlen(line) + strlen(path)));

				sprintf (error, "Error in config file: %s\nline: %s",path, line);
				dialog_error (error);
				
				free (error);
				free (item);
			}
		}
	}

	fclose (f_in);
	return (conf);
}

int config_save (GList *configlist, char *path) {
	FILE *f_out;
	GList *conf;
	config_keyval *item;
	char line[4096];
	
	conf = configlist;

	f_out = fopen (path, "w+");
	if (!f_out) {
		return (-1);
	}
	
	while (conf) {
		item = conf->data;
		
#ifdef DEBUG
			printf ("config_save: key=%s, value=%s\n", item->key, item->value);
#endif
		sprintf (line, "%s=%s\n", item->key, item->value);
		fputs (line, f_out);
		
		conf = conf->next;
	}
	
	fclose (f_out);
	
	return (0);
}

int config_create (char *path) {
	return (mkdir(path, S_IRWXU|S_IRWXG|S_IRWXO));
}

GList *config_create_keys (void) {
	GList *conf = NULL;

	conf = config_newitem (conf, "Return","action_run");
	conf = config_newitem (conf, "F1","operation_help");
	conf = config_newitem (conf, "F3","operation_edit");
	conf = config_newitem (conf, "F4","operation_edit");
	conf = config_newitem (conf, "F5","operation_copy");
	conf = config_newitem (conf, "F6","operation_move");
	conf = config_newitem (conf, "F7","operation_mkdir");
	conf = config_newitem (conf, "F8","operation_delete");
	conf = config_newitem (conf, "F9","operation_symlink");
	conf = config_newitem (conf, "F10","operation_quit");
	conf = config_newitem (conf, "F11","operation_permissions");
	conf = config_newitem (conf, "F12","operation_ownership");
	conf = config_newitem (conf, "c+F6","operation_rename");
	conf = config_newitem (conf, "Insert","sel_toggle");
	conf = config_newitem (conf, "space","sel_toggle");
	conf = config_newitem (conf, "KP_Multiply","sel_toggleall");
	conf = config_newitem (conf, "Home","cursor_top");
	conf = config_newitem (conf, "End","cursor_bottom");
	conf = config_newitem (conf, "Right","action_run");
	conf = config_newitem (conf, "Left","action_dirup");
	conf = config_newitem (conf, "c+l","focus_pathentry");
	conf = config_newitem (conf, "KP_Add","sel_pattern_plus");
	conf = config_newitem (conf, "KP_Subtract","sel_pattern_minus");
	conf = config_newitem (conf, "c+h","toggle_hidden");
	conf = config_newitem (conf, "c+grave","home");
	conf = config_newitem (conf, "c+slash","root");
	conf = config_newitem (conf, "BackSpace","action_dirup");
	conf = config_newitem (conf, "Tab","panel_cycle");
	conf = config_newitem (conf, "c+o","toggle_cmd_output");
	conf = config_newitem (conf, "c+Return", "selection_to_cmdline");
	conf = config_newitem (conf, "c+c", "focus_cmdline");
	conf = config_newitem (conf, "c+r", "refresh");

	return conf;
}

GList *config_create_ext (void) {
	GList *conf = NULL;
	return conf;
}
GList *config_create_nimf (void) { 
	GList *conf = NULL;
	
	conf = config_newitem (conf, "nimf_width","640");
	conf = config_newitem (conf, "nimf_height","480");
	conf = config_newitem (conf, "bar_title","1");
	conf = config_newitem (conf, "bar_operations","1");
	conf = config_newitem (conf, "sort_dirsfirst","1");
	conf = config_newitem (conf, "sort_casesensitive","0");
	conf = config_newitem (conf, "jump_case_sensitive","0");
	conf = config_newitem (conf, "panel_current","0");
	conf = config_newitem (conf, "show_keys","0");

	return conf;
}
