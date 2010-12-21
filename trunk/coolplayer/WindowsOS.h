
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
////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////
//
// OS functions
//
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
// Win2000/98 API stuff
// needed only for VC6 compile **/
#ifndef MONITOR_DEFAULTTONULL
typedef void* HMONITOR;

typedef struct _MONITORINFO
{
	DWORD  cbSize;
	RECT   rcMonitor;
	RECT   rcWork;
	DWORD  dwFlags;
} MONITORINFO;

#define MONITOR_DEFAULTTONULL       0x00000000
#define MONITOR_DEFAULTTOPRIMARY    0x00000001
#define MONITOR_DEFAULTTONEAREST    0x00000002
#endif
typedef BOOL (WINAPI *wp_GetMonitorInfo)(HMONITOR hMonitor, MONITORINFO* lpmi);
typedef HMONITOR(WINAPI *wp_MonitorFromWindow)(HWND hwnd, DWORD dwFlags);
typedef BOOL (WINAPI *wp_TrackMouseEvent)(LPTRACKMOUSEEVENT lpEventTrack);
//
extern wp_GetMonitorInfo pfnGetMonitorInfo;
extern wp_MonitorFromWindow pfnMonitorFromWindow;
extern wp_TrackMouseEvent pfnTrackMouseEvent;
//
////////////////////////////////////////////////////////////////////////////////
void CP_InitWindowsRoutines();
