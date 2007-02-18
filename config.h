/* Functions for reading, setting and writing configuration options */
/*
 * Nimf
 * Copyright (C) 2002,2003 Ferry Boender.
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

/* GList storage of keys and their values */
typedef struct config_keyval {
	char *key;
	char *value;
} config_keyval;

/* NOTE: Move to somewhere else: key configuration, seperate file? */
typedef enum action_ {
	operation_help, 
	operation_view, 
	operation_edit, 
	operation_copy, 
	operation_move, 
	operation_mkdir, 
	operation_delete, 
	operation_symlink, 
	operation_quit, 
	operation_rename,
	operation_permissions,
	operation_ownership,
	sel_toggle,
	sel_toggleall,
	sel_pattern_plus,
	sel_pattern_minus,
	cursor_up,
	cursor_down,
	cursor_top,
	cursor_bottom,
	action_dirup,
	action_run,
	focus_pathentry,
	focus_cmdline,
	toggle_hidden,
	refresh,
	home,
	root,
	mount,
	jumpsearch_cycle,
	panel_cycle,
	toggle_cmd_output,
	selection_to_cmdline
} action_;


int key_action_map (GList *config_keys, char *key_pressed);

/* Name    : config_find_value
 * Descr   : Finds a configuration value matching a certain key.
 * Params  : configlist  = GList containing configuration options filled by 
 *                         config_read()
 *           key         = string containing the key to look for
 *           default_val = Default value to return when KEY was not found.
 * Pre     : CONFIGLIST is a valid GList (non-empty) and read in by config_read
 * Returns : The value which goes with KEY or
 *           DEFAULT_VAL when KEY wasn't found in CONFIGLIST
 * Notes   : KEY is case-insensitive.
 */
char *config_find_value (GList *configlist, char *key, char *default_val);

/* Name    : config_newitem
 * Descr   : Add's a key/value pair to the configuration list.
 * Params  : configlist  = GList containing configuration options.
 *           key         = string containing the key to add
 *           value       = string containing the value to add
 * Pre     : CONFIGLIST is a valid GList 
 * Returns : The new GList with the configuration.
 * Notes   : -
 */
GList *config_newitem (GList *configlist, char *key, char *val);

/* Name    : config_change_key 
 * Descr   : Changes the key for a key/val configuration item
 * Params  : configlist = The configuration list in which to change the key
 *           key = The old key
 *           newkey = The new key
 * Pre     : -
 * Returns : The new GList containing the new configuration
 * Notes   : -
 */
GList *config_change_key (GList *configlist, char *key, char *newkey);

/* Name    : config_change_value
 * Descr   : Changes the value for a key
 * Params  : configlist  = GList containing configuration options.
 *           key         = Key whose value to change
 *           value       = New value
 * Pre     : CONFIGLIST is a valid GList 
 * Returns : The new GList with the configuration.
 * Notes   : It will call config_newitem when KEY can't be found in CONFIGLIST
 */
GList *config_change_value (GList *configlist, char *key, char *newvalue);

/* Name    : config_get_key
 * Descr   : Return key portion of a configuration line.
 * Params  : line = A line read from a configuration file.
 * Pre     : -
 * Returns : The key portion of LINE (everything after the '=') or
 *           NULL if no value was found.
 * Notes   : Used by config_read, and should not have to be used by you.
 */
char *config_get_key (char *line);

/* Name    : config_get_value
 * Descr   : Return value portion of a configuration line.
 * Params  : line = A line read from a configuration file.
 * Pre     : -
 * Returns : The value portion of LINE (everything in front of the '=') or
 *           NULL if no key was found.
 * Notes   : Used by config_read, and should not have to be used by you.
 */
char *config_get_value (char *line);

/* Name    : config_read
 * Descr   : Return a GList to be used as a configuration list.
 * Params  : path = Full path to a configuration file.
 * Pre     : -
 * Returns : A GList containing the various keys and values or
 *           NULL if the configuration file was not found or contained no 
 *           (valid) configuration lines.
 * Notes   : Use config_find_value to get values from the GList
 */
GList *config_read (char *path);

/* Name    : config_save
 * Descr   : Saves a GList configuration list to PATH
 * Params  : path = Full path to a configuration file.
 * Pre     : -
 * Returns : -1 if the configuration could not be saved.
 *           0 if the configuration was saved succesfull.
 * Notes   : Previous PATH will be overwritten (if exists)
 *           If PATH does not exist, it will be created.
 */
int config_save (GList *configlist, char *path);

/* Name    : config_create
 * Descr   : Creates a new placeholder for configuration files (simply said: a dir)
 * Params  : path = Full path to a configuration file.
 * Pre     : -
 * Returns : -1 if the configuration could not be saved.
 *           0 if the configuration was saved succesfull.
 * Notes   : -
 *           If PATH does not exist, it will be created.
 */
int config_create (char *path); 

/* Name    : config_create_keys
 * Descr   : Creates a new default keys configuration
 * Params  : - 
 * Pre     : -
 * Returns : New GList containing the default key configurtion
 * Notes   : -
 */
GList *config_create_keys (void);

/* Name    : config_create_ext
 * Descr   : Creates a new default extention configuration
 * Params  : - 
 * Pre     : -
 * Returns : New GList containing the default extention configurtion
 * Notes   : -
 */
GList *config_create_ext (void);

/* Name    : config_create_nimf
 * Descr   : Creates a new default Misc configuration
 * Params  : - 
 * Pre     : -
 * Returns : New GList containing the default Misc configurtion
 * Notes   : -
 */
GList *config_create_nimf (void);

/* Name    : config_delitem
 * Descr   : Deletes a configuration key and value from CONFIGLIST
 * Params  : configlist = The configuration from which to delete the key/val combi
 *           key = The key for the key/val combi to remove
 * Pre     : -
 * Returns : The new GList containing the new configuration
 * Notes   : -
 */
GList *config_delitem (GList *configlist, char *key);


