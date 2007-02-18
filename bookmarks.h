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

/* Name    : bookmarks_load
 * Descr   : Load a set of bookmarks for in the full-file panel from FILE
 * Params  : file = full path to file including filename fron which to load bookmarks
 * Pre     : -
 * Returns : New GList containing the bookmarks.
 * Notes   : -
 */
GList *bookmarks_load (char *file);

/* Name    : bookmarks_save
 * Descr   : Save a set of bookmarks to FILE
 * Params  : bookmarks = a GList containg the bookmarks to save
 *           file = full path to file including filename to which to save
 * Pre     : -
 * Returns : -1 if bookmarks couldn't be saved. 0 if saved succesful.
 * Notes   : -
 */
int bookmarks_save (GList *bookmarks, char *file);

/* Name    : bookmarks_toggle
 * Descr   : Add or remove BOOKMARK_PATH from the bookmarks list
 * Params  : bookmarks = a GList containg the bookmarks.
 *           bookmark_path = Text to add or remove from bookmarks list
 * Pre     : -
 * Returns : New GList containing bookmarks
 * Notes   : -
 */
GList *bookmarks_toggle (GList *bookmarks, char *bookmark_path);

