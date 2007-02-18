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

#define FTYPE_UNKOWN 0
#define FTYPE_NORMAL 1
#define FTYPE_DIR 2
#define FTYPE_LINKNORMAL 3
#define FTYPE_LINKDIR 4
#define FTYPE_LINKDANG 5

#define OPT_CHMOD_RECURSE         1 /* Recurse into subdirectories */
#define	OPT_CHMOD_DIRS_NOPRESERVE 2 /* Do not preserve executable flag on dirs */
#define	OPT_CHMOD_EXEC_NOPRESERVE 4 /* Do not preserve executable flag on executables */

#define OPT_CHOWN_RECURSE         1


typedef struct fileinfo {
	char *name;
	uid_t uid;
	gid_t gid;
	mode_t mode;
	off_t size;
	int type;
	long int mtime;
} fileinfo;

/* statusreport is used to report back the current status of a operation. */
typedef struct statusreport {
	char *filename;
	int percent;
	int errornr;
} statusreport;

/* Name    : file_fullpath
 * Descr   : Construct a full path to a file by appending FILENAME to PATH
 * Params  : path     = string containing the path
 *           filename = string containing the filename
 * Pre     : -
 * Returns : Newly allocated mem containing zero terminated string with full 
 *           path to file.
 * Notes   : The pointer returned by file_fullpath should be freed by the calling
 *           function using free().
 *           The path may or may not have a trailing '/'.
 */
char *file_fullpath (char *path, char *filename);

/* Name    : file_pathcmp
 * Descr   : Compares two paths taking in account any trailing '/'s
 * Params  : src  = First path to compare
 *           dest = Second path to compare
 * Pre     : -
 * Returns :   0 if the paths are equal
 *           < 0 if SRC is smaller than DEST
 *           > 0 if DEST is smaller than SRC
 * Notes   : Comparison is case-sensitive.
 */
int file_pathcmp (char *src, char *dest);
	
/* Name    : file_info
 * Descr   : Returns a struct containing a files information.
 * Params  : path     = Path the dir containing the file
 *           filename = Name of the file itself.
 * Pre     : PATH must NOT have a trialing '/'.
 * Returns : Newly allocated file_info struct containing the file's info or
 *           NULL if an error occured while reading the file's info. 
 * Notes   : See struct fileinfo in file.h to see what info is returned.
 */
fileinfo *file_info (char *path, char *filename);

/* Name    : file_type
 * Descr   : Returns the type of a file.
 * Params  : pathtofile = Full path to the file to get the type of.
 * Pre     : -
 * Returns : -1 if an error occured while getting the fileinfo.
 *            0 if PATHTOFILE points to a unknown (normal) file.
 *            1 if PATHTOFILE points to a directory
 * Notes   : file_type follows symlinks. See also file_type_withlink
 */
int file_type (char *pathtofile);

/* Name    : file_type_withlink
 * Descr   : Returns the type of a file.
 * Params  : pathtofile = Full path to the file to get the type of.
 * Pre     : -
 * Returns : -1 if an error occured while getting the fileinfo.
 *            0 if PATHTOFILE points to a unknown (normal) file.
 *            1 if PATHTOFILE points to a directory
 *            2 if PATHTOFILE points to a (sym)link
 * Notes   : See also file_type_withlink
 */
int file_type_withlink (char *filename);

/* Name    : file_striplastdir
 * Descr   : Returns a path similer to PATH but with the last dir stripped off.
 * Params  : path = Full path to strip last dir from
 * Pre     : -
 * Returns : A pointer to newly allocated mem containing the stripped path
 * Notes   : Unused, check if works
 */
char *file_striplastdir (char *path);

/* Name    : file_lastdir
 * Descr   : Returns the last dir in a path
 * Params  : path = Full path to get the lastest dir from
 * Pre     : -
 * Returns : A pointer to everything after the last '/'.
 * Notes   : The returned pointer points to the same memory space as PATH.
 */
char *file_lastdir (char *path);

/* Name    : file_ext
 * Descr   : Returns a file's extention
 * Params  : pathtofile = Full path to the file from which to get the extention.
 * Pre     : -
 * Returns : A pointer to the file's extention
 * Notes   : The returned pointer points to the same memory space as PATH.
 *           You may also specify only a filename
 */
char *file_ext (char *pathtofile);

/* Name    : file_nicesize
 * Descr   : Returns a integer in a dotted way (1.024)
 * Params  : size = The integer size to convert to dotted
 * Pre     : -
 * Returns : A pointer to newly allocated space containing the string.
 * Notes   : Caller needs to free the pointer returned using free()
 */
char *file_nicesize (int size);

/* Name    : file_niceuser
 * Descr   : Returns a string containing the username for UID
 * Params  : uid = The user_id to 'resolve'
 * Pre     : -
 * Returns : A pointer to newly allocated space containing the username.
 * Notes   : Caller needs to free the pointer returned using free()
 */
char *file_niceusername (int uid);

/* Name    : file_nicegroup
 * Descr   : Returns a string containing the groupname for GIF
 * Params  : gid = The group_id to 'resolve'
 * Pre     : -
 * Returns : A pointer to newly allocated space containing the groupname.
 * Notes   : Caller needs to free the pointer returned using free()
 */
char *file_nicegroupname (int gid);

/* Name    : file_nicesize
 * Descr   : Converts MODE to a 'rwxrwxrwx' format.
 * Params  : gid = The group_id to 'resolve'
 * Pre     : -
 * Returns : A pointer to newly allocated space containing the format.
 * Notes   : Caller needs to free the pointer returned using free()
 *           No file type ([dlcpbw-]) is prepended at this time.
 */
char *file_nicemode (int mode);

/* FIXME: Edit */
/* Name    : file_copy
 * Descr   : Copy a file from SRC to DEST path
 * Params  : src  = full path to file to copy from
 *           dest = full path to file to copy to.
 * Pre     : -
 * Returns : -1 if something went wrong
 *            0 if everything went ok
 * Notes   : Errno will be set if something went wrong.
 */
int file_copy (int actionpipe, int reportpipe, char *src, char *dest);

/* Name    : file_del
 * Descr   : Delete the file in SRC
 * Params  : src  = full path to file to delete
 * Pre     : -
 * Returns : -1 if something went wrong.
 *            0 if everything went ok.
 * Notes   : 
 */
int file_del (char *src);

/* Name    : file_mkdir
 * Descr   : Create a new directory DEST
 * Params  : dest = full path + name for the new dir.
 * Pre     : -
 * Returns : -1 if something went wrong.
 *            0 if everything went ok.
 * Notes   : Errno will be set on error
 */
int file_mkdir (char *dest);

/* Name    : file_lnk
 * Descr   : Creates a symlink DEST which points to SRC
 * Params  : src = full path + name to file to which to symlink
             dest = full path + name to file of link to create
 * Pre     : -
 * Returns : -1 if something went wrong.
 *            0 if everything went ok.
 * Notes   : Errno will be set on error
 *           Also works on dirs.
 */
int file_lnk (char *src, char *dest);

/* Name    : dir_copy
 * Descr   : Recursively copy the dir SRC to the dir DEST
 * Params  : src  = Full path to a dir to copy
 *           dest = Full path to a dir to where to copy SRC to.
 * Pre     : -
 * Returns : Useless crap for now.
 * Notes   : DEST will be created if necessary
 */
 /* FIXME : edit comments */
int dir_copy (int actionpipe, int reportpipe, char *src, char *dest);

/* Name    : dir_del
 * Descr   : Recursively delete the SRC path
 * Params  : src  = Full path to a dir to copy
 * Pre     : -
 * Returns : Useless crap for now.
 * Notes   : -
 */
 /* FIXME : Change comments */
int dir_del (int actionpipe, int reportpipe, char *src);

/* FIXME: Comments */
void report_done (int reportpipe);
void report_error (int reportpipe, char *path, char *filename);
void report_progress (int reportpipe, char *path, char *filename, int percent);
int action_read (int actionpipe);

/* Name    : filelist_copy
 * Descr   : Copy the files in SRC to DESTPATH using options OPTIONS
 * Params  : reportpipe = the unix pipe through which to report errors.
 *           srcfiles  = a glist with filenames which need to be copied
 *           srcpath   = the source path where SRCFILES reside.
 *           destpath  = the destination path to copy to.
 *           options   = Currently unused.
 * Pre     : srcpath should not have a '/' appended (ex. root dir)
 *           destpath should not have a '/' appended (ex. root dir)
 * Returns : -
 * Notes   : -
 */
void filelist_copy (int actionpipe, int reportpipe, GList *srcfiles, char *srcpath, char *destpath, int options);
void filelist_move (int actionpipe, int reportpipe, GList *srcfiles, char *srcpath, char *destpath, int options);
void filelist_mkdir (int actionpipe, int reportpipe,char *destpath, char *currentpath, int options);
void filelist_del (int actionpipe, int reportpipe, GList *srcfiles, char *srcpath, int options);
void filelist_lnk (int actionpipe, int reportpipe, GList *srcfiles, char *srcpath, char *destpath, int options);
void filelist_chmod (int actionpipe, int reportpipe, GList *files, char *path, mode_t mode, int options);
void filelist_chown (int actionpipe, int reportpipe, GList *files, char *path, uid_t uid, gid_t gid, int options);

