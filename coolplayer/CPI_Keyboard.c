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



#include "stdafx.h"
#include "globals.h"
#include "resource.h"
#include "CPI_Player.h"
#include "CPI_Playlist.h"
#include "CPI_PlaylistItem.h"


////////////////////////////////////////////////////////////////////////////////
//
//
//
void CP_HandleKeyPress_Playlist(HWND hWnd, const int iVKey, const BOOL bAlt, const BOOL bCtrl, const BOOL bShift)
{
	switch (iVKey)
	{
	
		case 'F':
			CPVERB_ToggleFindDialog(vaDoVerb, hWnd);
			return;
	
		case 'V':
			CPVERB_SavePlaylist(vaDoVerb, hWnd);
			return;

		case VK_DELETE:
			// FALLTHROUGH
		case VK_BACK:
		
			if (bCtrl)
				CPVERB_PlaylistClearAll(vaDoVerb, hWnd);
			else
				CPVERB_PlaylistClearSelected(vaDoVerb, hWnd);
				
			return;
			
		case VK_UP:
			if (bAlt)
			{
				CPVERB_PlaylistOffsetUp(vaDoVerb, hWnd);
				return;
			}
			
			break;
			
		case VK_DOWN:
		
			if (bAlt)
			{
				CPVERB_PlaylistOffsetDown(vaDoVerb, hWnd);
				return;
			}
			
			break;
			
		case VK_ADD:
		
			if (bAlt)
			{
				CPVERB_PlaylistMaximise(vaDoVerb, hWnd);
				return;
			}
			
			break;
			
		case VK_SUBTRACT:
		
			if (bAlt)
			{
				CPVERB_PlaylistMinimise(vaDoVerb, hWnd);
				return;
			}
			
			break;
	}
	
	CP_HandleKeyPress_Player(hWnd, iVKey, bAlt, bCtrl, bShift);
}

//
//
//
void CP_HandleKeyPress_Player(HWND hWnd, const int iVKey, const BOOL bAlt, const BOOL bCtrl, const BOOL bShift)
{
	switch (iVKey)
	{
	
		case 'P':
			CPVERB_TogglePlaylistWindow(vaDoVerb, hWnd);
			break;
			
		case 'E':
		
		case 'Q':
			CPVERB_ToggleEqualiser(vaDoVerb, hWnd);
			break;
			
		case 'S':
			CPVERB_ToggleShuffle(vaDoVerb, hWnd);
			break;
			
		case 'R':
			CPVERB_ToggleRepeat(vaDoVerb, hWnd);
			break;
			
		case 'X':	// FALLTHROUGH
		case VK_RETURN:
		case VK_NUMPAD5:
			CPVERB_Play(vaDoVerb, hWnd);
			break;
			
		case 'V':	// FALLTHROUGH
		case VK_DECIMAL:
			CPVERB_Stop(vaDoVerb, hWnd);
			break;
			
		case 'C':	// FALLTHROUGH
		case VK_PAUSE:
		case VK_NUMPAD0:
			CPVERB_Pause(vaDoVerb, hWnd);
			break;
			
		case 'B':	// FALLTHROUGH
		case VK_NUMPAD6:
			CPVERB_NextTrack(vaDoVerb, hWnd);
			break;
			
		case 'Z':	// FALLTHROUGH
		case VK_NUMPAD4:
			CPVERB_PrevTrack(vaDoVerb, hWnd);
			break;
			
		case VK_LEFT:
		
			if (bCtrl)
				CPVERB_PrevTrack(vaDoVerb, hWnd);
			else
				CPVERB_SkipBackwards(vaDoVerb, hWnd);
				
			break;
			
		case VK_RIGHT:
			if (bCtrl)
				CPVERB_NextTrack(vaDoVerb, hWnd);
			else
				CPVERB_SkipForwards(vaDoVerb, hWnd);
				
			break;
			
		case VK_ADD:	// FALLTHROUGH
		case VK_NUMPAD8:
		case VK_UP:
			if (!bAlt)
				CPVERB_VolumeUp(vaDoVerb, hWnd);
				
			break;
			
		case VK_SUBTRACT:	// FALLTHROUGH
		case VK_NUMPAD2:
		case VK_DOWN:
			if (!bAlt)
				CPVERB_VolumeDown(vaDoVerb, hWnd);
				
			break;
			
		case '0':	// FALLTHROUGH
		case 'M':
		case VK_TAB:
			globals.m_iVolume = 0;
			
			main_draw_vu_from_value(windows.wnd_main, VolumeSlider, globals.m_iVolume);
			
			CPI_Player__SetVolume(globals.m_hPlayer, globals.m_iVolume);
			
			break;
			

		case '1':	// FALLTHROUGH
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			globals.m_iVolume = 10 + ((iVKey - '1') * 10);
			
			main_draw_vu_from_value(windows.wnd_main, VolumeSlider, globals.m_iVolume);
			
			CPI_Player__SetVolume(globals.m_hPlayer, globals.m_iVolume);
			
			break;
			
		case VK_F1:
			CPVERB_About(vaDoVerb, hWnd);
			
			break;
			
		case VK_F4:
			if (bAlt)
				CPVERB_Exit(vaDoVerb, hWnd);
			else if (bCtrl)
				CPVERB_TogglePlaylistWindow(vaDoVerb, hWnd);
				
			break;
			
		case VK_ESCAPE:
			CPVERB_Exit(vaDoVerb, hWnd);
			
			break;
			
		case 'L':	// FALLTHROUGH
		case 'O':
			CPVERB_OpenFile(vaDoVerb, hWnd);
			break;
			
		case 'A':
			CPVERB_AddFile(vaDoVerb, hWnd);
			break;
			
		case VK_DELETE:
			if (bShift)
			{
				CP_HPLAYLISTITEM hCurrent = CPL_GetActiveItem(globals.m_hPlaylist);
				CPVERB_Stop(vaDoVerb, hWnd);
				
				if (hCurrent)
				{
					const char *pcText = CPLI_GetPath(hCurrent);
					DeleteFile(pcText);
					CPL_RemoveItem(globals.m_hPlaylist, hCurrent);
					
				}
			}
			break;

		case 'H':
			CPVERB_PlaylistShuffle(vaDoVerb, hWnd);
			return;
			
		case 'D':
			CPVERB_AddDirectory(vaDoVerb, hWnd);
			return;
	}
}

//
//
//
