/* Functions for working with files */
/*
 * Nimf
 * Copyright (C) 2002 Ferry Boender.
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
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <gtk/gtk.h>

#include "file.h"

char *file_fullpath (char *path, char *filename) {
    char *fullpath;

    fullpath = (char *) malloc (sizeof(char) * (strlen(path) + strlen(filename) +3));

    strcpy (fullpath, path);
	
	if (fullpath[strlen(fullpath)-1] != '/' && strlen(fullpath) > 1) {
	    strcat (fullpath, "/");
	}
    strcat (fullpath, filename);

    return (fullpath);
}

int file_pathcmp (char *src, char *dest) {
    char *fullpath_src;
	char *fullpath_dest;
	int result;
	
	fullpath_src = strdup(src);
	fullpath_dest = strdup(dest);
	
	if (fullpath_src[strlen(fullpath_src)-1] == '/') {
		fullpath_src[strlen(fullpath_src)-1] = '\0';
	}
	if (fullpath_dest[strlen(fullpath_dest)-1] == '/') {
		fullpath_dest[strlen(fullpath_dest)-1] = '\0';
	}

	result = strcmp(fullpath_src, fullpath_dest);
	
	free (fullpath_src);
	free (fullpath_dest);

    return (result);
}


fileinfo *file_info (char *path, char *filename) {
	struct fileinfo *finfo;
	struct stat filestats;
	char *fullpath;

	fullpath = file_fullpath (path, filename);
	finfo = (struct fileinfo *) malloc (sizeof(struct fileinfo));
	
	if (lstat(fullpath, &filestats) == -1) {
		/* Couldn't stat */
		perror (filename);
		return (NULL);
	}
	
	finfo->name = strdup(filename);
	finfo->size = filestats.st_size;
	finfo->mode = filestats.st_mode;
	finfo->uid = filestats.st_uid;
	finfo->gid = filestats.st_gid;
	finfo->type = file_type_withlink (fullpath);
	finfo->mtime = filestats.st_mtime;
	
	free (fullpath);
	
	return (finfo);
}

int file_type (char *pathtofile) {
    struct stat filestats;

    if (stat (pathtofile, &filestats) == -1) {
		return (-1); /* Error while stating */
	}

	if (S_ISDIR(filestats.st_mode)) { 
		return (FTYPE_DIR); 
	} else {
		return (FTYPE_NORMAL);
	}
}

int file_type_withlink (char *filename) {
    struct stat filestats;

    if (lstat (filename, &filestats) == -1) {
		return (-1); /* Error while stating */
	}

	if (S_ISLNK(filestats.st_mode)) { 
		return (FTYPE_LINKNORMAL); 
	} else 	
	if (S_ISDIR(filestats.st_mode)) { 
		return (FTYPE_DIR); 
	} else {
		return (FTYPE_NORMAL);
	}
			
}

char *file_striplastdir (char *path) {
	char *newpath;
	char *temp;
	char *lastslash;
	
	temp = strdup(path);

	/* Remove slash at end if present */
	if (temp[strlen(temp)] == '/') {
		temp[strlen(temp)] = '\0';
	}
	
	/* Find last dir in path */
	lastslash = strrchr(temp, '/');
	lastslash = '\0';

	if (strcmp(temp, "\0") == 0) {
		/* new path is empty, make root dir */
		newpath = strdup ("/");
	} else {
		/* Save everything up to the last slash in newpath */
		newpath = strdup (temp);
	}
	
	free (temp);
	
	return (newpath);
}

char *file_lastdir (char *path) {
	char *lastdir;

	lastdir = strrchr(path, '/');
	lastdir++;

	return (lastdir);
}

char *file_ext (char *pathtofile) {
	char *extention;
	char *slashpos;

	slashpos = strrchr (pathtofile,'/');
	extention = strchr(slashpos,'.');
	if (extention != NULL) {
		extention++;
	}

	return (extention);
}

char *file_nicesize (int size) {
	char *strsize;
	char *strsize_base;
	char *nicesize;

	strsize = (char *) malloc (sizeof(char) * 17);
	strsize_base = strsize;
	nicesize = (char *) malloc (sizeof(char) * 17);
	
	nicesize[0]='\0';
	sprintf (strsize, "%i", size);
	
	if (strlen(strsize) % 3 > 0) {
		strncpy (nicesize, strsize, strlen(strsize) % 3);
		nicesize[strlen(strsize) % 3] = '\0';
		strcat (nicesize, ".");
		strsize += (strlen(strsize) % 3);

	}

	while (strcmp(strsize, "\0") != 0) {
		strncat (nicesize, strsize, 3);
		strcat (nicesize, ".");
		strsize += 3;
	}
	nicesize[strlen(nicesize)-1] = '\0';
	strsize = strsize_base;

	free (strsize);

	return (nicesize);
}

char *file_niceusername (int uid) {
	struct passwd *pwent;
	char *niceusername;

	pwent = getpwuid (uid);

	if (pwent != NULL) {
		niceusername = malloc(sizeof(char) * (strlen(pwent->pw_name)+2));
		strcpy (niceusername, pwent->pw_name);
	} else {
		char uid_string[8];

		sprintf (uid_string, "%i", uid);
		niceusername = strdup(uid_string);
	}
	
	
	return (niceusername);
}

char *file_nicegroupname (int gid) {
	struct group *grent;
	char *nicegroupname;
	
	grent = getgrgid (gid);

	if (grent != NULL) {
		nicegroupname = (char *) malloc(sizeof(char) * (strlen(grent->gr_name)+2));
		strcpy (nicegroupname, grent->gr_name);
	} else {
		char gid_string[8];

		sprintf (gid_string, "%i", gid);
		nicegroupname = strdup(gid_string);
	}

	return (nicegroupname);
}

char *file_nicemode (int mode) {
	/* This routine was for the main part stolen from Emelfm <:) */
	char *perm_sets[] = {"---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx"};
	char *permstring;
	int u,g,o;

	permstring = (char *) malloc (sizeof(char) * 10);
	
	u = (mode & S_IRWXU) >> 6;
	g = (mode & S_IRWXG) >> 3; 
	o = (mode & S_IRWXO);
		  
	sprintf (permstring, "%s%s%s",perm_sets[u],perm_sets[g],perm_sets[o]);

	return (permstring);
}

int file_copy (int actionpipe, int reportpipe, char *src, char *dest) {
	FILE *in,*out;
	char buf[4096];
	int i = 0;
	struct stat filestats;
	
	if (file_type_withlink(src) == FTYPE_LINKNORMAL) { 
		int len;
		/* FIXME: error checking */
		len = readlink (src, buf, 4096);
		if (len == -1) {
			return (len);
		}
		buf[len] = '\0';
		return (file_lnk (buf, dest)); 
	} else {
		if ((in=fopen(src,"r"))==NULL) {
			return (-1);
		}
		if ((out=fopen(dest,"wt"))==NULL) {
			return (-1);
			fclose(in);
		}

		
		while ((i=fread(buf,1,4096,in))) {
			if (fwrite(buf,1,i,out)!=i)
				break;
		}
		
		i=(ferror(in)||ferror(out))?-1:0;
		if (i == -1) {
			fclose (in);
			fclose (out);
			return (-1);
		}
		
		fclose(in);
		fclose(out);

		/* Preserve rights */
		/* FIXME: Error checking! */
		
		if (stat (src, &filestats) == 0) {
			return (chmod (dest, filestats.st_mode));
		} else {
			return (-1);
		}
	}

	return(i);
}

int file_move (int actionpipe, int reportpipe, char *src, char *dest) {
	if (rename(src, dest) == -1) {
		if (errno == EXDEV) {
			/* Cross-device moving (copy) */
			if (file_copy (actionpipe, reportpipe, src, dest) == -1) {
				return (-1);
			}
			if (file_del (src) == -1) {
				return (-1);
			}
		} else {
			return (-1);
		}
	}
	return (0);
}

int file_del (char *src) {
	return (unlink(src));
}

int file_mkdir (char *dest) {
	return (mkdir (dest, S_IRWXU|S_IRWXG|S_IRWXO));
}

int file_lnk (char *src, char *dest) {
	return (symlink (src, dest));
}

int file_chmod (char *src, mode_t mode) {
	return (chmod (src, mode));
}

int file_chown (char *src, uid_t uid, gid_t gid) {
	return (chown(src, uid, gid));
}

int dir_chmod (int actionpipe, int reportpipe, char *src, mode_t mode) {
	/* FIXME: Function is lacking error reporting */
	/* FIXME: Function does not honour options */
	DIR *dir;
	struct dirent *direntry;
//	struct stat filestats;
	char *fullpath_src;
	int op_stop = 0;

	dir = opendir (src);
	if (dir == NULL) {
		report_error (reportpipe, src, NULL);
		return (action_read (actionpipe));
	}

	while ((direntry = readdir(dir)) && op_stop != 1) {
		if (strcmp(direntry->d_name, ".") != 0 &&
			strcmp(direntry->d_name, "..") != 0) {
			fullpath_src = file_fullpath(src, (char *)direntry->d_name);

			if (file_type(fullpath_src) == FTYPE_DIR) {
				op_stop = dir_chmod (actionpipe, reportpipe, fullpath_src, mode);
			} else {
				if (file_chmod (fullpath_src, mode) == -1) {
					report_error (reportpipe, fullpath_src, NULL);

					if (action_read(actionpipe) == 1) {
						op_stop = 1;
					}
				}

				
			}

			free (fullpath_src);
		}
	}

	closedir (dir);

	return (0);
}

int dir_chown (int actionpipe, int reportpipe, char *src, uid_t uid, gid_t gid) {
	/* FIXME: Function is lacking error reporting */
	/* FIXME: Function does not honour options */
	DIR *dir;
	struct dirent *direntry;
//	struct stat filestats;
	char *fullpath_src;
	int op_stop = 0;

	dir = opendir (src);
	if (dir == NULL) {
		report_error (reportpipe, src, NULL);
		return (action_read (actionpipe));
	}

	while ((direntry = readdir(dir)) && op_stop != 1) {
		if (strcmp(direntry->d_name, ".") != 0 &&
			strcmp(direntry->d_name, "..") != 0) {
			fullpath_src = file_fullpath(src, (char *)direntry->d_name);

			if (file_type(fullpath_src) == FTYPE_DIR) {
				op_stop = dir_chown (actionpipe, reportpipe, fullpath_src, uid, gid);
			} else {
				if (file_chown (fullpath_src, uid, gid) == -1) {
					report_error (reportpipe, fullpath_src, NULL);
					
					if (action_read(actionpipe) == 1) {
						op_stop = 1;
					}
				}
			}

			free (fullpath_src);
		}
	}

	closedir (dir);

	if (op_stop == 1){
		return (1);
	} else {
		return (0);
	}
}

int dir_copy (int actionpipe, int reportpipe, char *src, char *dest) {
	/* FIXME: Function is lacking error reporting?? */
	DIR *dir;
	struct dirent *direntry;
	struct stat filestats;
	char *fullpath_src;
	char *fullpath_dest;
	
	if (file_type_withlink(src) == FTYPE_LINKNORMAL) { 
		int len;
		char buf[4096];
		
		/* FIXME : error checking */
		len = readlink (src, buf, 4096);
		buf[len] = '\0';
		file_lnk (buf, dest);
	} else {
		dir = opendir(src);
		if (dir == NULL) {
			report_error (reportpipe, src, NULL);
			return (action_read (actionpipe));
		}

		if (file_mkdir (dest) == -1) {
			if (errno != EEXIST) {
				report_error (reportpipe, dest, NULL);
				return (action_read (actionpipe));
			}
		}
		
		while ((direntry = readdir(dir))) {
			if (
					strcmp(direntry->d_name, ".") != 0 &&
					strcmp(direntry->d_name, "..") != 0) {
				fullpath_src = file_fullpath (src, (char *)direntry->d_name);
				fullpath_dest = file_fullpath (dest, (char *)direntry->d_name);
				
				
				if (file_type(fullpath_src) == FTYPE_DIR) {
					dir_copy (actionpipe, reportpipe, fullpath_src, fullpath_dest);
				} else {
					file_copy (actionpipe, reportpipe, fullpath_src, fullpath_dest);
				}

				free (fullpath_src);
				free (fullpath_dest);
			}
		}
		
		closedir (dir);
		
		/* Preserve permissions */
		/* FIXME: Check errors */
		if (stat(src, &filestats) == 0) {
			chmod (dest, filestats.st_mode);
		}
	}
		
	return (0);
}

int dir_del (int actionpipe, int reportpipe, char *src) {
	DIR *dir;
	struct dirent *direntry;
	char *fullpath;
	int op_stop = 0;
	
	dir = opendir(src);
	if (dir == NULL) {
		report_error (reportpipe, src, NULL);
		return (action_read (actionpipe));
	}

	while ((direntry = readdir(dir)) && op_stop != 1) {
		int blaat;
		
		if ( strcmp(direntry->d_name, ".") != 0 &&
				strcmp(direntry->d_name, "..") != 0) {
			fullpath = file_fullpath (src, direntry->d_name);
			
			switch (file_type_withlink(fullpath)) {
				case FTYPE_NORMAL:
					if (file_del(fullpath) == -1) {
						report_error (reportpipe, fullpath, NULL);
						if (action_read (actionpipe) == 1) {
							op_stop = 1;
						}
					}
					break;
				case FTYPE_DIR: 
					op_stop = dir_del(actionpipe, reportpipe, fullpath);
					break;
				case FTYPE_LINKNORMAL: 
					if (file_del(fullpath) == -1) {
						report_error (reportpipe, fullpath, NULL);
						blaat = action_read (actionpipe);
						if (blaat == 1) {
							op_stop = 1;
						}
					}
					break;
			}
					
			free (fullpath);
		}
	}
	
	closedir (dir);

	if (!op_stop) {
		if (rmdir (src) == -1) {
			report_error (reportpipe, src, NULL);
			if (action_read (actionpipe) == 1) {
				op_stop = 1;
			}
		}
	}

	if (op_stop == 1) {
		return (1);
	} else {
		return (0);
	}
	
}

int dir_move (int actionpipe, int reportpipe, char *src, char *dest) {
	DIR *dir;
	struct dirent *direntry;
	char *fullpath_src;
	char *fullpath_dest;
	
	dir = opendir(src);
	if (dir == NULL) {
		return (-1);
	}

	file_mkdir (dest);
	
	while ((direntry = readdir(dir))) {
		if (
				strcmp(direntry->d_name, ".") != 0 &&
				strcmp(direntry->d_name, "..") != 0) {
			fullpath_src = file_fullpath (src, (char *)direntry->d_name);
			fullpath_dest = file_fullpath (dest, (char *)direntry->d_name);
			
			
			switch (file_type_withlink(fullpath_src)) {
				case FTYPE_LINKNORMAL : file_move (actionpipe, reportpipe, fullpath_src, fullpath_dest); break;
				case FTYPE_DIR : dir_move (actionpipe, reportpipe, fullpath_src, fullpath_dest); break;
				case FTYPE_NORMAL : file_move (actionpipe, reportpipe, fullpath_src, fullpath_dest); break;
			}
 
			free (fullpath_src);
			free (fullpath_dest);
		}
	}
	
	closedir (dir);
	
	if (rmdir (src) == -1) {
		report_error (reportpipe, src, NULL);
		action_read (actionpipe);
	}
	
	return (0);
}

void report_done (int reportpipe) {
	int reporttype = 0;
	
	write (reportpipe, &reporttype, sizeof(int));
}

void report_error (int reportpipe, char *path, char *filename) {
	int reporttype = 2;
	char eostring = '\0';
	
	/* Report is an error */
	write (reportpipe, &reporttype, sizeof(int));
	write (reportpipe, &errno, sizeof(errno));
	write (reportpipe, path, sizeof(char) * strlen(path));
	write (reportpipe, &eostring, sizeof(char));
}

void report_progress (int reportpipe, char *path, char *filename, int percent) {
	int reporttype = 1;
	char eostring = '\0';
	
	write (reportpipe, &reporttype, sizeof(int));
	write (reportpipe, &percent, sizeof(int));
	write (reportpipe, path, sizeof(char) * strlen(path));
	write (reportpipe, &eostring, sizeof(char));
}

int action_read (int actionpipe) {
	int readresult = 0;
	int action = 0;

	readresult = read(actionpipe, &action, sizeof(int));
	
	return (action);
}

/* FIXME: Please, clean this up. */
//void filelist_copy (int actionpipe, int reportpipe, GList *srcfiles, char *srcpath, char *destpath, int options) {
//	GList *cur;
//	char *dest;
//	int i = 0, nroffiles = 0;
//	int error = 0;
//	int op_stop = 0;
//	
//	cur = srcfiles;
//	nroffiles = g_list_length (srcfiles);
//	
//	if (destpath[0] != '/') {
//		/* Resolve relative paths */
//		dest = malloc(sizeof(char) * (strlen(srcpath) + strlen(destpath) + 2));
//		dest = file_fullpath(srcpath, destpath);
//	} else {
//		dest = strdup(destpath);
//	}
//	
//	if (g_list_length(cur) == 1) {
//		char *final_src;
//		char *final_dest;
//		char linkpath[1024];
//		int linkpath_len;
//		
//		if (strcmp(cur->data, "..") != 0) {
//			if (file_type(dest) == FTYPE_DIR) {
//				final_dest = file_fullpath(dest, cur->data);
//			} else {
//				final_dest = strdup(dest);
//			}
//			
//			final_src = file_fullpath(srcpath, cur->data);
//			
//			i++;
//			
//			switch (file_type_withlink(final_src)) {
//				case FTYPE_LINKNORMAL : 
//					linkpath_len = readlink (final_src, linkpath, 1022);
//					linkpath[linkpath_len] = '\0';
//					error = file_lnk (linkpath, final_dest); 
//					break;
//				case FTYPE_DIR : error = dir_copy (actionpipe, reportpipe, final_src, final_dest); break;
//				case FTYPE_NORMAL : error = file_copy (actionpipe, reportpipe, final_src, final_dest); break;
//			}
//			
//			report_progress (reportpipe, final_src, NULL, (int) ((float)i/nroffiles*100) );
//
//			switch (error) {
//				case -1: 
//					report_error (reportpipe, final_dest, NULL);
//					if (action_read(actionpipe) == 1) {
//						op_stop = 1;
//					}
//					break;
//				case 1:
//					op_stop = 1;
//					break;
//				default:
//					break;
//			}
//			
//			free (final_src);
//			free (final_dest);
//			free (dest);
//		}
//		
//		report_done (reportpipe);
//
//		return;
//	}
//	
//	while (cur && op_stop == 0) {
//		char *fullpath_src = malloc (sizeof(char) * (strlen(cur->data) + strlen(srcpath) + 2 ));
//		char *fullpath_dest = malloc (sizeof(char) * (strlen(cur->data) + strlen(dest) + 2));
//		char linkpath[1024];
//		int linkpath_len;
//		
//		if (strcmp(cur->data, "..") != 0) { /* Do not operate on .. */
//			fullpath_src = file_fullpath (srcpath, cur->data);
//			fullpath_dest = file_fullpath (dest, cur->data);
//	
//			i++;
//			
//			
//			switch (file_type_withlink(fullpath_src)) {
//				case FTYPE_LINKNORMAL : 
//					linkpath_len = readlink (fullpath_src, linkpath, 1022);
//					linkpath[linkpath_len] = '\0';
//					error = file_lnk (linkpath, fullpath_dest); 
//					break;
//				case FTYPE_NORMAL : error = file_copy (actionpipe, reportpipe, fullpath_src, fullpath_dest); break;
//				case FTYPE_DIR : error = dir_copy (actionpipe, reportpipe, fullpath_src, fullpath_dest); break;
//			}
//			
//			report_progress (reportpipe, fullpath_src, NULL, (int) ((float)i/nroffiles*100) );
//
//			switch (error) {
//				case -1: 
//					report_error (reportpipe, fullpath_dest, NULL);
//					if (action_read(actionpipe) == 1) {
//						op_stop = 1;
//					}
//					break;
//				case 1:
//					op_stop = 1;
//					break;
//				default:
//					break;
//			}
//			
//			free (fullpath_src);
//			free (fullpath_dest);
//		}
//		
//		cur = cur->next;
//	}
//
//	free (dest);
//	report_done (reportpipe);
//}

void filelist_copy (int actionpipe, int reportpipe, GList *srcfiles, char *srcpath, char *destpath, int options) {
	GList *cur;
	char *dest;
	int i = 0, nroffiles = 0;
	int error = 0;
	int op_stop = 0;
	
	cur = srcfiles;
	nroffiles = g_list_length (srcfiles);
	
	if (destpath[0] != '/') {
		/* Destination is relative to current dir */
		dest = malloc(sizeof(char) * (strlen(srcpath) + strlen(destpath) + 2));
		dest = file_fullpath(srcpath, destpath);
	} else {
		/* Destination is absolute (to root) */
		dest = strdup(destpath);
	}
	
	printf ("dest: %s\n", dest);
	
//	if (g_list_length(cur) == 1) {
//		char *final_src;
//		char *final_dest;
//		char linkpath[1024];
//		int linkpath_len;
//		
//		if (strcmp(cur->data, "..") != 0) {
//			if (file_type(dest) == FTYPE_DIR) {
//				final_dest = file_fullpath(dest, cur->data);
//			} else {
//				final_dest = strdup(dest);
//			}
//			
//			final_src = file_fullpath(srcpath, cur->data);
//			
//			i++;
//			
//			switch (file_type_withlink(final_src)) {
//				case FTYPE_LINKNORMAL : 
//					linkpath_len = readlink (final_src, linkpath, 1022);
//					linkpath[linkpath_len] = '\0';
//					error = file_lnk (linkpath, final_dest); 
//					break;
//				case FTYPE_DIR : error = dir_copy (actionpipe, reportpipe, final_src, final_dest); break;
//				case FTYPE_NORMAL : error = file_copy (actionpipe, reportpipe, final_src, final_dest); break;
//			}
//			
//			report_progress (reportpipe, final_src, NULL, (int) ((float)i/nroffiles*100) );
//
//			switch (error) {
//				case -1: 
//					report_error (reportpipe, final_dest, NULL);
//					if (action_read(actionpipe) == 1) {
//						op_stop = 1;
//					}
//					break;
//				case 1:
//					op_stop = 1;
//					break;
//				default:
//					break;
//			}
//			
//			free (final_src);
//			free (final_dest);
//			free (dest);
//		}
//		
//		report_done (reportpipe);
//
//		return;
//	}
	
	while (cur && op_stop == 0) {
		char *fullpath_src = malloc (sizeof(char) * (strlen(cur->data) + strlen(srcpath) + 2 ));
		char *fullpath_dest = malloc (sizeof(char) * (strlen(cur->data) + strlen(dest) + 2));
		char linkpath[1024];
		int linkpath_len;

		if (strcmp(cur->data, "..") != 0) { /* Do not operate on .. */
			fullpath_src = file_fullpath (srcpath, cur->data);
			printf ("fullpath_src: %s\n", fullpath_src);
			
			/* Determine destination */
			if (file_type(dest) != FTYPE_DIR) {
				/* Copy to file */
				printf ("Copy to single file\n");
				if (nroffiles > 1) {
					/* Copying more than one file to a single file */
					printf ("Trying to copy more than one file to a single file!\n");
					/* FIXME: Trying to copy more than one file to a single file. */
				}
				strcpy(fullpath_dest, dest); /* Copy to new filename */
			} else {
				/* Copy to directory */
				printf ("Copy to dir\n");
				fullpath_dest = file_fullpath (dest, cur->data); /* Copy to directory */
			}
			printf ("fullpath_dest: %s\n", fullpath_dest);


	
			report_progress (reportpipe, fullpath_src, NULL, (int) ((float)i/nroffiles*100) );
			
			i++;
			
			/* Perform operation */
			switch (file_type_withlink(fullpath_src)) {
				case FTYPE_LINKNORMAL : 
					linkpath_len = readlink (fullpath_src, linkpath, 1022);
					linkpath[linkpath_len] = '\0';
					error = file_lnk (linkpath, fullpath_dest); 
					break;
				case FTYPE_NORMAL : error = file_copy (actionpipe, reportpipe, fullpath_src, fullpath_dest); break;
				case FTYPE_DIR : error = dir_copy (actionpipe, reportpipe, fullpath_src, fullpath_dest); break;
			}
			
			report_progress (reportpipe, fullpath_src, NULL, (int) ((float)i/nroffiles*100) );

			/* Error reporting */
			switch (error) {
				case -1: 
					report_error (reportpipe, fullpath_dest, NULL);
					if (action_read(actionpipe) == 1) {
						op_stop = 1;
					}
					break;
				case 1:
					op_stop = 1;
					break;
				default:
					break;
			}
			
			free (fullpath_src);
			free (fullpath_dest);
		}
		
		cur = cur->next;
	}

	free (dest);
	report_done (reportpipe);
}

void filelist_move (int actionpipe, int reportpipe, GList *srcfiles, char *srcpath, char *destpath, int options) {
	char *dest;
	GList *cur;
	int i = 0, nroffiles = 0, error = 0, op_stop = 0;
	
	cur = srcfiles;
	nroffiles = g_list_length (srcfiles);

	if (destpath[0] != '/') {
		/* Destination is relative to current dir */
		dest = malloc(sizeof(char) * (strlen(srcpath) + strlen(destpath) + 2));
		dest = file_fullpath(srcpath, destpath);
	} else {
		/* Destination is absolute (to root) */
		dest = strdup(destpath);
	}
	
//	if (g_list_length(cur) == 1) {
//		char *final_src;
//		char *final_dest;
//		
//		if (strcmp(cur->data, "..") != 0) {
//			final_src = file_fullpath(srcpath, cur->data);
//			
//			if (file_type(dest) == FTYPE_DIR) {
//				final_dest = file_fullpath(dest, cur->data);
//			} else {
//				final_dest = strdup(dest);
//			}
//			
//			
//			switch (file_type_withlink(final_src)) {
//				case FTYPE_LINKNORMAL : error = file_move (actionpipe, reportpipe, final_src, final_dest); break;
//				case FTYPE_DIR : error = dir_move (actionpipe, reportpipe, final_src, final_dest); break;
//				case FTYPE_NORMAL : error = file_move (actionpipe, reportpipe, final_src, final_dest); break;
//			}
//
//			switch (error) {
//				case -1: 
//					report_error (reportpipe, final_dest, NULL);
//					if (action_read(actionpipe) == 1) {
//						op_stop = 1;
//					}
//					break;
//				case 1:
//					op_stop = 1;
//					break;
//				default:
//					break;
//			}
//			
//
//			
//			free (final_src);
//			free (final_dest);
//			free (dest);
//		}
//		
//		report_done (reportpipe);
//		return;
//	}
	
	while (cur && op_stop == 0) {
		char *fullpath_src = malloc (sizeof(char) * (strlen(cur->data) + strlen(srcpath) + 2 ));
		char *fullpath_dest = malloc (sizeof(char) * (strlen(cur->data) + strlen(destpath) + 2 ));
			
		if (strcmp(cur->data, "..") != 0) {
			fullpath_src = file_fullpath(srcpath, cur->data);

			if (file_type(dest) != FTYPE_DIR) {
				/* Rename / Move to file */
				printf ("Move to single file (rename)\n");
				if (nroffiles > 1) {
					printf ("Trying to move multiple files to single file\n");
				}
				strcpy (fullpath_dest, dest);
			} else {
				/* Move to directory */
				fullpath_dest = file_fullpath(dest, cur->data);
			}

			i++;
			
			switch (file_type(fullpath_src)) {
				case FTYPE_NORMAL : error = file_move(actionpipe, reportpipe, fullpath_src, fullpath_dest); break;
				case FTYPE_DIR : error = dir_move(actionpipe, reportpipe, fullpath_src, fullpath_dest); break;
			}
			
			report_progress (reportpipe, fullpath_src, NULL, (int) ((float)i/nroffiles*100) );

			switch (error) {
				case -1: 
					report_error (reportpipe, fullpath_dest, NULL);
					if (action_read(actionpipe) == 1) {
						op_stop = 1;
					}
					break;
				case 1:
					op_stop = 1;
					break;
				default:
					break;
			}

			free (fullpath_src);
			free (fullpath_dest);
		}

		cur = cur->next;
	}

	report_done (reportpipe);
}

void filelist_mkdir (int actionpipe, int reportpipe, char *destpath, char *currentpath, int options) {
	char *dest;
	
	if (destpath[0] != '/') {
		/* Resolve relative paths */
		dest = malloc(sizeof(char) * (strlen(currentpath) + strlen(destpath) + 2));
		dest = file_fullpath(currentpath, destpath);
	} else {
		dest = strdup(destpath);
	}
	
	if ( mkdir(dest, S_IRWXU|S_IRWXG|S_IRWXO) == -1) {
		report_error (reportpipe, destpath, NULL);
		action_read (actionpipe);
	}

	report_done (reportpipe);
}

void filelist_del (int actionpipe, int reportpipe, GList *srcfiles, char *srcpath, int options) {
	GList *cur;
	int nroffiles = 0;
	int i = 0;
	int op_stop = 0;

	cur = srcfiles;
	nroffiles = g_list_length(cur);
	
	while (cur && op_stop != 1) {
		char *fullpath = malloc (sizeof(char) * (strlen(cur->data)+strlen(srcpath)
			+ 2));
		int error = 0;
		
		report_progress (reportpipe, fullpath, NULL, (int) ((float)i/nroffiles*100) );
		
		if (strcmp(cur->data, "..") != 0) {
			/* FIXME : Fullpath function? */
			sprintf (fullpath, "%s/%s", srcpath, (char *)cur->data);
			i++;
			
			/* FIXME: Do NOT use unlink, but use file_del */
			switch (file_type_withlink(fullpath)) {
				case FTYPE_NORMAL : error = unlink(fullpath); break;
				case FTYPE_DIR : error = dir_del(actionpipe, reportpipe, fullpath); break;
				case FTYPE_LINKNORMAL : error = unlink(fullpath); break;
			}

			report_progress (reportpipe, fullpath, NULL, (int) ((float)i/nroffiles*100) );

			switch (error) {
				case -1: 
					report_error (reportpipe, fullpath, NULL);
					if (action_read(actionpipe) == 1) {
						op_stop = 1;
					}
					break;
				case 1:
					op_stop = 1;
					break;
				default:
					break;
			}
			free (fullpath);
		}
		
		cur = cur->next;
	}

	report_done (reportpipe);
}

void filelist_lnk (int actionpipe, int reportpipe, GList *srcfiles, char *srcpath, char *destpath, int options) {
	char *dest;
	GList *cur;
	
	cur = srcfiles;
	
	if (destpath[0] != '/') {
		/* Resolve relative paths */
		dest = malloc(sizeof(char) * (strlen(srcpath) + strlen(destpath) + 2));
		dest = file_fullpath(srcpath, destpath);
	} else {
		dest = strdup(destpath);
	}
	
	/* FIXME: Clean this up. Conditional code below can be integrated with while(cur) condition */
	if (g_list_length(cur) == 1) {
		char *final_src;
		char *final_dest;
		
		if (file_type(dest) == FTYPE_DIR) {
			final_dest = file_fullpath(dest, cur->data);
		} else {
			final_dest = strdup(dest);
		}
		
		final_src = file_fullpath(srcpath, cur->data);
		
		/* FIXME: Error checking? */
		if (file_lnk (final_src, final_dest) < 0) {
			report_error (reportpipe, final_dest, NULL);
			action_read (actionpipe);
		};
		
		free (final_src);
		free (final_dest);
		free (dest);
		
		report_done (reportpipe);
		return;
	}
	
	/* FIXME: Where is the op_stop != 1 check?? */
	while (cur) {
		char *fullpath_src = malloc (sizeof(char) * (strlen(cur->data) + strlen(srcpath) + 2 ));
		char *fullpath_dest = malloc (sizeof(char) * (strlen(cur->data) + strlen(destpath) + 2 ));
			
		fullpath_src = file_fullpath(srcpath, cur->data);
		fullpath_dest = file_fullpath(dest, cur->data);
		
		if (file_lnk (fullpath_src, fullpath_dest) < 0) {
			report_error (reportpipe, fullpath_dest, NULL);
			action_read (actionpipe);
		};

		free (fullpath_src);
		free (fullpath_dest);
		
		cur = cur->next;
	}

	report_done (reportpipe);
}

void filelist_chmod (int actionpipe, int reportpipe, GList *files, char *srcpath, mode_t mode, int options) {
	GList *cur;
	int nroffiles = 0;
	int i = 0;
	int op_stop = 0;

	cur = files;
	
//	while (cur) {
//		char *fullpath = malloc (sizeof(char) * (strlen(cur->data) + strlen(path) + 2));
//		
//		sprintf (fullpath, "%s/%s", path, (char *)cur->data);
//
//		if (chmod (fullpath, mode) == -1) {
//			report_error (reportpipe, fullpath, NULL);
//		}
//
//		free (fullpath);
//		cur = cur->next;
//	}
//
//	report_done (reportpipe);
	nroffiles = g_list_length (files);

	while (cur && op_stop != 1) {
		char *fullpath = malloc (sizeof(char) * (strlen(cur->data)+strlen(srcpath)
			+ 2));
		int error = 0;
		
		report_progress (reportpipe, fullpath, NULL, (int) ((float)i/nroffiles*100) );
		
		if (strcmp(cur->data, "..") != 0) {
			/* FIXME : Fullpath function? */
			sprintf (fullpath, "%s/%s", srcpath, (char *)cur->data);
			i++;

			printf ("filelist_chmod: %s (%i%%)\n", fullpath, (int) ((float)i/nroffiles*100) );
			
			/* FIXME: Do NOT use unlink, but use file_del */
			switch (file_type_withlink(fullpath)) {
				case FTYPE_NORMAL : error = chmod(fullpath, mode); break;
				case FTYPE_DIR : 
					if ((options & OPT_CHMOD_RECURSE) == OPT_CHMOD_RECURSE) {
						error = dir_chmod(actionpipe, reportpipe, fullpath, mode);
					}
					error = file_chmod(fullpath, mode);
					break;
				case FTYPE_LINKNORMAL : error = file_chmod(fullpath, mode); break;
				/* FIXME: default: unknown file? */
			}

			report_progress (reportpipe, fullpath, NULL, (int) ((float)i/nroffiles*100) );

			switch (error) {
				case -1: 
					report_error (reportpipe, fullpath, NULL);
					if (action_read(actionpipe) == 1) {
						op_stop = 1;
					}
					break;
				case 1:
					op_stop = 1;
					break;
				default:
					break;
			}
			free (fullpath);
		}
		
		cur = cur->next;
	}

	report_done (reportpipe);
	
}

void filelist_chown (int actionpipe, int reportpipe, GList *files, char *srcpath, uid_t uid, gid_t gid, int options) {
	GList *cur;
	int nroffiles = 0;
	int i = 0;
	int op_stop = 0;


	cur = files;
	
//	while (cur) {
//		char *fullpath = malloc (sizeof(char) * (strlen(cur->data) + strlen(path) + 2));
//		
//		sprintf (fullpath, "%s/%s", path, (char *)cur->data);
//
//		if (chown (fullpath, uid, gid) == -1) {
//			report_error (reportpipe, fullpath, NULL);
//		};
//		
//		chmod (fullpath, mode);
//
//		printf ("%s\n", fullpath);
//
//		free (fullpath);
//		cur = cur->next;
//	}

//	report_done (reportpipe);
	nroffiles = g_list_length (files);
	
	while (cur && op_stop != 1) {
		char *fullpath = malloc (sizeof(char) * (strlen(cur->data)+strlen(srcpath)
			+ 2));
		int error = 0;
		
		report_progress (reportpipe, fullpath, NULL, (int) ((float)i/nroffiles*100) );
		
		if (strcmp(cur->data, "..") != 0) {
			/* FIXME : Fullpath function? */
			sprintf (fullpath, "%s/%s", srcpath, (char *)cur->data);
			printf ("fullpath %s", fullpath);
			i++;
			
			switch (file_type_withlink(fullpath)) {
				case FTYPE_NORMAL : error = chown(fullpath, uid, gid); break;
				case FTYPE_DIR : 
					if ((options & OPT_CHOWN_RECURSE) == OPT_CHOWN_RECURSE) {
						printf ("filelist_chown: recurse\n");
						error = dir_chown(actionpipe, reportpipe, fullpath, uid, gid);
					}
					error = file_chown (fullpath, uid, gid);
					break;
				case FTYPE_LINKNORMAL : error = file_chown(fullpath, uid, gid); break;
				/* FIXME: default: unknown file? */
			}

			report_progress (reportpipe, fullpath, NULL, (int) ((float)i/nroffiles*100) );

			switch (error) {
				case -1: 
					report_error (reportpipe, fullpath, NULL);
					if (action_read(actionpipe) == 1) {
						op_stop = 1;
					}
					break;
				case 1:
					op_stop = 1;
					break;
				default:
					break;
			}
			free (fullpath);
		}
		
		cur = cur->next;
	}

	report_done (reportpipe);
	
}
