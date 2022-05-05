/*
 * sys/wininclude.h
 */

#ifndef _SYS_WININCLUDE_H_
#define _SYS_WININCLUDE_H_

#define WINVER 0x0501 //_WIN32_WINNT_WINXP   
#define _WIN32_WINNT 0x0501
#define NTDDI_VERSION 0x05010300 //NTDDI_WINXPSP3 
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX // Some compilers enable this by default
#define NOMINMAX
#endif
#define NODRAWTEXT
#define NOBITMAP
#define NOMCX
#define NOSERVICE
#define NOHELP
#undef _MINWINDEF_
#include <windows.h>

#endif
