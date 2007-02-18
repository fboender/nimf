/*
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

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <gtk/gtk.h>

GList *bookmarks_load (char *file) {
	FILE *f_in;
	char line[4096];
	GList *bookmarks = NULL;
	
	f_in = fopen(file, "r");
	if (!f_in) {
		return (NULL);
	}

	while (fgets(line, 4094, f_in)) {
		/* Strip newline */
		if (line[strlen(line)-1] == '\n') {
			line[strlen(line)-1] = '\0';
		}
		bookmarks = g_list_append (bookmarks, strdup(line));
	}

	fclose (f_in);

	return (bookmarks);
}


int bookmarks_save (GList *bookmarks, char *file) {
	FILE *f_out;
	GList *current_bm = NULL;
	char line[4096];
	
	f_out = fopen(file, "w");
	if (!f_out) {
		return (-1);
	}

	current_bm = bookmarks;
	while (current_bm) {
		sprintf (line, "%s\n", (char *)current_bm->data);
		fputs (line, f_out);
		
		current_bm = current_bm->next;
	}
	
	return (0);
}

GList *bookmarks_toggle (GList *bookmarks, char *bookmark_path) {
	GList *current_bm = NULL;
	gpointer *data = NULL;

	current_bm = bookmarks;
	while (current_bm) {
		if (strcmp(current_bm->data, bookmark_path) == 0) {
			data = current_bm->data;
			break;
		}
		current_bm = current_bm->next;
	}
	
	if (data) {
		bookmarks = g_list_remove (bookmarks, data);
		free (data);
	} else {
		bookmarks = g_list_append (bookmarks, strdup(bookmark_path));
	}

	return (bookmarks);
}

