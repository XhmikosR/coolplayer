#define _WIN32_WINDOWS 0x0410
#define WIN32_LEAN_AND_MEAN

#define _WIN32_IE 0x600

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "debug.h"
#include <process.h>
#include <wininet.h>
// #include <search.h>
#include <malloc.h>
#include <time.h>
#include <shlobj.h>
#include <commdlg.h>
#include <mmsystem.h>
#include <shellapi.h>
#include <io.h>

#define sprintf wsprintf
#define strcpy lstrcpy
#define strcmp lstrcmp
#define strcat lstrcat
#define stricmp lstrcmpi
#define strlen lstrlen
#define strncpy lstrcpyn

int __cdecl  memcmp(const void*, const void*, size_t);
void* __cdecl  memcpy(void*, const void*, size_t);
void* __cdecl memset(void*, int, size_t);
#ifdef __MINGW32__
void* __cdecl memmove(void*, const void*, size_t);
char* __cdecl strchr(const char*, int) ;
char* __cdecl strrchr(const char*, int) ;
char* __cdecl strstr(const char*, const char*);
int __cdecl _stricmp(const char*, const char*);
int __cdecl _strnicmp(const char*, const char*, size_t);
int __cdecl tolower(int);
#endif
#if 0
char* __cdecl strncat(char*, const char*, size_t);
int __cdecl strncmp(const char*, const char*, size_t) ;
int __cdecl strnicmp(const char*, const char*, size_t);
char* __cdecl strtok(char*, const char*);
int __cdecl toupper(int);

int __cdecl  _memicmp(const void*, const void*, size_t);
char* __cdecl  _strdup(const char*) __MINGW_ATTRIB_MALLOC;
int __cdecl _strcmpi(const char*, const char*);
int __cdecl _stricoll(const char*, const char*);
char* __cdecl _strlwr(char*);
char* __cdecl _strnset(char*, int, size_t);
char* __cdecl _strrev(char*);
char* __cdecl _strset(char*, int);
char* __cdecl _strupr(char*);
void __cdecl _swab(const char*, char*, size_t);
#endif


#ifndef OFN_ENABLESIZING
#define OFN_ENABLESIZING             0x00800000
#endif

#ifndef IRF_NO_WAIT
#define IRF_NO_WAIT     0x00000008
#endif

#ifndef IDC_STATIC  /* May be predefined by resource compiler.  */
#define IDC_STATIC (-1)
#endif
