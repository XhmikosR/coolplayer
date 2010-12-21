/*
 * CoolPlayer - Blazing fast audio player.
 * Copyright (C) 2000-2001 Niek Albers
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef _DEBUG

#ifndef _MSC_VER

int _CrtDbgReport(
	int reportType,
	const char *filename,
	int linenumber,
	const char *moduleName,
	const char *lpszFormat ,
	...
)
{
	char szBuffer[1024];
	va_list args;
	va_start(args, lpszFormat);
	_vsnprintf(szBuffer, sizeof(szBuffer) / sizeof(WCHAR), lpszFormat, args);
	OutputDebugString(szBuffer);
	va_end(args);
	return 1;
}

#else

#include <crtdbg.h>

#endif

#define CP_TRACE0(format) _CrtDbgReport(_CRT_WARN, __FILE__, __LINE__, NULL, format "\n")
#define CP_TRACE1(format, arg1) _CrtDbgReport(_CRT_WARN, __FILE__, __LINE__, NULL, format "\n", arg1)
#define CP_TRACE2(format, arg1, arg2) _CrtDbgReport(_CRT_WARN, __FILE__, __LINE__, NULL, format "\n", arg1, arg2)
#define CP_TRACE3(format, arg1, arg2, arg3) _CrtDbgReport(_CRT_WARN, __FILE__, __LINE__, NULL, format "\n", arg1, arg2, arg3)
#define CP_TRACE4(format, arg1, arg2, arg3, arg4) _CrtDbgReport(_CRT_WARN, __FILE__, __LINE__, NULL, format "\n", arg1, arg2, arg3, arg4)
#define CP_TRACE5(format, arg1, arg2, arg3, arg4, arg5) _CrtDbgReport(_CRT_WARN, __FILE__, __LINE__, NULL, format "\n", arg1, arg2, arg3, arg4, arg5)
//
#define CP_ASSERT(expr) if(!(expr)) { CP_TRACE1("ASSERTION %s FAILS", #expr); __asm { int 3 }  }
#define CP_FAIL(errstring) { CP_TRACE1("HARD FAILURE %s", #errstring); __asm { int 3 } }
//
#define CP_CHECKOBJECT(obj_ptr_typed) if(!obj_ptr_typed || !_CrtIsMemoryBlock(obj_ptr_typed, sizeof(*obj_ptr_typed), NULL, NULL, NULL)) { CP_TRACE1("POINTER %s is Bogus", #obj_ptr_typed); __asm { int 3 } }




#else

#define CP_CHECKOBJECT(obj_ptr_typed) {  }
#define CP_ASSERT(expr)     {  }
#define CP_FAIL(expr)     {  }
#define CP_TRACE0(f)     {  }
#define CP_TRACE1(f, e1)    {  }
#define CP_TRACE2(f, e1, e2)   {  }
#define CP_TRACE3(f, e1, e2, e3)  {  }
#define CP_TRACE4(f, e1, e2, e3, e4) {  }
#define CP_TRACE5(f, e1, e2, e3, e4, e5){  }

#endif

