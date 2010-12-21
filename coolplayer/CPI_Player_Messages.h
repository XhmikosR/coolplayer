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
// Cooler player engine control messages and message data
//
////////////////////////////////////////////////////////////////////////////////

// Engine control structure

typedef struct __CPs_PlayEngine
{
	HANDLE m_hThread;
	DWORD m_dwThreadID;
	HANDLE m_hEvtThreadReady;
	HWND m_hWndNotify;
	void* m_pContext;
	
	// Volume control mixer
	HMIXER m_hVolumeMixer;
	DWORD m_dwMixerControlID;
} CPs_PlayEngine;


// Engine entry point
extern DWORD WINAPI CPI_Player__EngineEP(void* pCookie);


// Messages
#define CPTM_QUIT							(WM_APP+0x001)
#define CPTM_OPENFILE						(WM_APP+0x002)
#define CPTM_SEEK							(WM_APP+0x003)
#define CPTM_PLAY							(WM_APP+0x004)
#define CPTM_STOP							(WM_APP+0x005)
#define CPTM_PAUSE							(WM_APP+0x006)
#define CPTM_SETPROGRESSTRACKLENGTH			(WM_APP+0x008)
#define CPTM_SENDSYNCCOOKIE					(WM_APP+0x009)
#define CPTM_BLOCKMSGUNTILENDOFSTREAM		(WM_APP+0x00A)
#define CPTM_ENUMOUTPUTDEVICES				(WM_APP+0x00B)
#define CPTM_SETEQSETTINGS					(WM_APP+0x00C)
#define CPTM_ONOUTPUTMODULECHANGE			(WM_APP+0x00D)
#define CPTM_ASSOCIATEFILEEXTENSIONS		(WM_APP+0x00E)
#define CPTM_SETINTERNALVOLUME				(WM_APP+0x00F)


// Notifies
#define CPNM_first							(WM_APP+0x101)
//
#define CPNM_PLAYERSTATE					(WM_APP+0x101)
#define CPNM_FILEINFO						(WM_APP+0x102)
#define CPNM_FILEOFFSET_SECS				(WM_APP+0x103)
#define CPNM_FILEOFFSET_PROP				(WM_APP+0x104)
#define CPNM_SYNCCOOKIE						(WM_APP+0x105)
#define CPNM_FOUNDOUTPUTDEVICE				(WM_APP+0x106)
#define CPNM_SETSTREAMINGSTATE				(WM_APP+0x107)
// ..
// ..
//
#define CPNM_last							(WM_APP+0x107)
