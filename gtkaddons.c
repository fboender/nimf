/* Copyright (C) 2002 Ferry Boender.
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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

void *gtk_param_pack (char *fmt, ...) {
	va_list ap;
	int len = 0;
	void *ptr = NULL;

	char *s;
	int i;
	char c;
	void *p = NULL;
	
	char *curpos_s = NULL;
	int *curpos_i = NULL;
	char *curpos_c = NULL;
	void **curpos_p = NULL;
	
	va_start (ap, fmt);
	while (*fmt) {
		switch (*fmt++) {
			case 's':
				s = va_arg(ap, char *);

				ptr = realloc (ptr, len + (sizeof(char) * (strlen(s) + 1)));
				curpos_s = ptr + len;
				sprintf (curpos_s, "%s", s);
				len = len + sizeof(char) * (strlen(s) + 1);

				break;
			case 'i':
				i = va_arg(ap, int);
				
				ptr = realloc (ptr, len + sizeof(int));
				curpos_i = ptr + len;
				*curpos_i = i;
				
				len = len + sizeof(int);

				break;
			case 'c':
				c = (char) va_arg(ap, int);
				
				ptr = realloc (ptr, len + sizeof(char));
				curpos_c = ptr + len;
				*curpos_c = c;
				
				len = len + sizeof(char);
				
				break;
			case 'p':
				p = va_arg(ap, void *);

				ptr = realloc(ptr, len + sizeof(void *));
				curpos_p = ptr + len;
				*curpos_p = p;

				len += sizeof(void *);
				
				break;
			default :
				printf ("pack error, unknown format char. Aborted\n");
				exit(0);
				break;
		}
		va_end(ap);
	}
	
	return (ptr);
}

void gtk_param_unpack (void *data, char *fmt, ...) {
	va_list ap;
	char **s = NULL;
	int *i;
	char *c;
	void **p = NULL;
	
	void *ptr = data;
	
	va_start (ap, fmt);
	while (*fmt) {
		switch (*fmt++) {
			case 's':
				s = va_arg(ap, char **);
				*s = strdup(ptr);
				ptr += (sizeof(char) * (strlen(ptr) + 1));
				break;
			case 'i':
				i = va_arg(ap, int *);
				*i = *(int *)ptr;
				ptr += sizeof(int);
				break;
			case 'c':
				c = va_arg(ap, char *);
				*c = *(char *)ptr;
				ptr += sizeof(char);
				break;
			case 'p':
				p = va_arg(ap, void **);

				memcpy(p, ptr, sizeof(void *));
				ptr += sizeof(void *);

				break;

			default :
				printf ("pack error, unknown format char. Aborted\n");
				exit(0);
				break;
		}
		va_end(ap);
	}
}

char *gtk_key_serialize (GdkEventKey *keyevent) {
	char *serialized_key = strdup("");

	/* Read modifiers. (Ugly, I know) */
	if ((keyevent->state & GDK_CONTROL_MASK) > 0) { 
		/* Control */
		serialized_key = realloc(serialized_key, sizeof(char) * (3 + strlen(serialized_key)));
		serialized_key = strcat (serialized_key, "c+");
	} 
	if ((keyevent->state & GDK_MOD1_MASK) > 0) {
		/* Alt */
		serialized_key = realloc(serialized_key, sizeof(char) * (4 + strlen(serialized_key)));
		serialized_key = strcat (serialized_key, "m1+");
	} 
	if ((keyevent->state & GDK_MOD3_MASK) > 0) {
		/* Win */
		serialized_key = realloc(serialized_key, sizeof(char) * (4 + strlen(serialized_key)));
		serialized_key = strcat (serialized_key, "m3+");
	} 

	/* add key */
	if (keyevent->keyval) {
		serialized_key = realloc(serialized_key, sizeof(char) * (strlen(gdk_keyval_name(keyevent->keyval)) + strlen(serialized_key) + 1));
		serialized_key = strcat (serialized_key, gdk_keyval_name(keyevent->keyval));
	}

	return (serialized_key);
}

GdkEventKey *gtk_key_unserialize (char *serialized_key) {
	int i, pos = 0;
	char key_part[1024];
	GdkEventKey *keyevent = malloc(sizeof(GdkEventKey));

	if (strlen(serialized_key) > 1024) {
		printf ("Serialized key size too long. Abort.\n");
		exit(0);
	}

	keyevent->state = 0;
	
	for (i = 0; i < strlen(serialized_key); i++) {
		
		if (serialized_key[i] != '+') {
			key_part[pos++] = serialized_key[i];
			key_part[pos] = '\0';
			
		} else {
			pos = 0;
			if (strcmp(key_part, "c") == 0) {
				/* Control */
				keyevent->state = keyevent->state | GDK_CONTROL_MASK;
			} else 
			if (strcmp(key_part, "m1") == 0) {
				/* Alt */
				keyevent->state = keyevent->state | GDK_MOD1_MASK;
			} else 
			if (strcmp(key_part, "m3") == 0) {
				/* Win */
				keyevent->state = keyevent->state | GDK_MOD3_MASK;
			}
		}
	}

	keyevent->keyval = gdk_keyval_from_name (key_part);

	return (keyevent);
}

