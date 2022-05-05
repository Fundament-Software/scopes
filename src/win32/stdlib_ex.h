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

#ifdef __cplusplus
}
#endif

#endif
