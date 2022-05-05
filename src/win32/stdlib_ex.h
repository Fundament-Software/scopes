/*
 * sys/stdlib_ex.h
 */

#ifndef _SYS_STDLIB_EX_H_
#define _SYS_STDLIB_EX_H_

#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif

char *realpath(const char *path, char *resolved_path);

#ifdef _MSC_VER
// We just define this to 260 instead of trying to set it to MAX_PATH because this requires including the windows header files everywhere.
#define PATH_MAX 260
#define STDOUT_FILENO _fileno(stdout)
#define isatty(i) _isatty(i)
#define strdup(arg) _strdup(arg)
typedef ptrdiff_t ssize_t;

struct dirent
{
	unsigned long	d_fileno;	/* file number of entry */
	unsigned short	d_reclen;	/* length of this record */
	unsigned char	d_type; 	/* file type, see below */
	unsigned char	d_namlen;	/* length of string in d_name */
#ifdef _POSIX_SOURCE
	char	d_name[255 + 1];	/* name must be no longer than this */
#else
#define	MAXNAMLEN	255
	char	d_name[MAXNAMLEN + 1];	/* name must be no longer than this */
#endif
};

struct __DIR;
typedef struct __DIR DIR;

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#define getcwd(buffer,maxlen) _getcwd_dbg(buffer, maxlen, _NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define getcwd(buffer,maxlen) _getcwd(buffer,maxlen)
#endif

char* dirname(char* path);
char* basename(char* path);
DIR* opendir(const char* name);
int closedir(DIR* dirp);
struct dirent* readdir(DIR* dirp);
void sincos(double x, double* sin, double* cos);
void sincosf(float x, float* sin, float* cos);

#endif

#ifdef __cplusplus
}
#endif

#endif
