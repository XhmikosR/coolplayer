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
#include "CPI_Playlist.h"
#include "CPI_PlaylistWindow.h"
#include "DLG_Find.h"
#include "CPI_Player.h"


////////////////////////////////////////////////////////////////////////////////
//
wp_Verb glb_pfnAllVerbs[] =
{

	CPVERB_TogglePlaylistWindow,
	CPVERB_ToggleRepeat,
	CPVERB_ToggleShuffle,
	CPVERB_ToggleEqualiser,
	CPVERB_ToggleFindDialog,
	
	CPVERB_PlaylistClearSelected,
	CPVERB_PlaylistClearAll,
	
	CPVERB_Play,
	CPVERB_Stop,
	CPVERB_Pause,
	CPVERB_NextTrack,
	CPVERB_PrevTrack,
	CPVERB_SkipForwards,
	CPVERB_SkipBackwards,
	CPVERB_VolumeUp,
	CPVERB_VolumeDown,
	
	CPVERB_OpenFile,
	CPVERB_AddFile,
	CPVERB_About,
	CPVERB_Exit,
	
	CPVERB_SavePlaylist,
	CPVERB_PlaylistShuffle,
	CPVERB_PlaylistOffsetUp,
	CPVERB_PlaylistOffsetDown,
	CPVERB_AddDirectory,
	CPVERB_PlaylistMinimise,
	CPVERB_PlaylistMaximise,
	
	NULL
};
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
//
//
void CPVERB_TogglePlaylistWindow(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
	{
		options.show_playlist = !options.show_playlist;
		CPlaylistWindow_SetVisible(options.show_playlist);
	}
	
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "TogglePlaylistWindow") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_ToggleRepeat(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
	{
		options.repeat_playlist = !options.repeat_playlist;
		InvalidateRect(windows.wnd_main, NULL, FALSE);
	}
	
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "ToggleRepeat") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_ToggleShuffle(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
	{
		options.shuffle_play = !options.shuffle_play;
		
		if (options.shuffle_play)
		{
			CPL_Stack_Shuffle(globals.m_hPlaylist, FALSE);
		}
		
		else
		{
			CPL_Stack_RestackAll(globals.m_hPlaylist);
		}
		
		InvalidateRect(windows.wnd_main, NULL, FALSE);
	}
	
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "ToggleShuffle") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_ToggleEqualiser(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
	{
		options.equalizer = !options.equalizer;
		main_set_eq();
		InvalidateRect(windows.wnd_main, NULL, FALSE);
	}
	
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "ToggleEqualiser") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_ToggleFindDialog(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
	{
		if (windows.m_hWndFindDialog == NULL)
		{
			windows.m_hWndFindDialog = CreateDialog(GetModuleHandle(NULL),
													MAKEINTRESOURCE(IDD_QUICKFIND),
													windows.wnd_main,
													wp_FindDialog);
		}
		
		else
			SendMessage(windows.m_hWndFindDialog, WM_CLOSE, 0L, 0L);
	}
	
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "ToggleFindDialog") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_PlaylistClearSelected(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
		CPlaylistWindow_ClearSelectedItems();
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "PlaylistClearSelected") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}


//
//
//
void CPVERB_PlaylistClearAll(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
		CPL_Empty(globals.m_hPlaylist);
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "PlaylistClearAll") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_Play(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
	{
		if (globals.m_enPlayerState == cppsPaused)
			CPI_Player__Play(globals.m_hPlayer);
		else
			CPL_PlayItem(globals.m_hPlaylist, TRUE, pmCurrentItem);
	}
	
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "Play") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_Stop(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
	{
		CPI_Player__Stop(globals.m_hPlayer);
		globals.main_bool_wavwrite_dir_already_known = FALSE;
	}
	
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "Stop") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_Pause(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
	{
		if (globals.m_enPlayerState == cppsPaused)
			CPI_Player__Play(globals.m_hPlayer);
		else
			CPI_Player__Pause(globals.m_hPlayer);
	}
	
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "Pause") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_NextTrack(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
		CPL_PlayItem(globals.m_hPlaylist, TRUE, pmNextItem);
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "NextTrack") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_PrevTrack(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
		CPL_PlayItem(globals.m_hPlaylist, TRUE, pmPrevItem);
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "PrevTrack") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_SkipForwards(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
	{
		if (globals.m_bStreaming == FALSE)
		{
			globals.main_int_track_position++;
			
			if (globals.main_int_track_position > Skin.Object[PositionSlider].w)
				globals.main_int_track_position = Skin.Object[PositionSlider].w;
				
			if (Skin.Object[PositionSlider].maxw == 1)
				CPI_Player__Seek(globals.m_hPlayer, globals.main_int_track_position, Skin.Object[PositionSlider].h);
			else
				CPI_Player__Seek(globals.m_hPlayer, globals.main_int_track_position, Skin.Object[PositionSlider].w);
		}
	}
	
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "SkipForwards") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_SkipBackwards(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
	{
		if (globals.m_bStreaming == FALSE)
		{
			globals.main_int_track_position = globals.main_int_track_position - 3;
			
			if (globals.main_int_track_position < 0)
				globals.main_int_track_position = 0;
				
			if (Skin.Object[PositionSlider].maxw == 1)
				CPI_Player__Seek(globals.m_hPlayer, globals.main_int_track_position, Skin.Object[PositionSlider].h);
			else
				CPI_Player__Seek(globals.m_hPlayer, globals.main_int_track_position, Skin.Object[PositionSlider].w);
		}
	}
	
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "SkipBackwards") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_VolumeUp(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
	{
		globals.m_iVolume++;
		
		if (globals.m_iVolume > 100)
			globals.m_iVolume = 100;
			
		main_draw_vu_from_value(windows.wnd_main, VolumeSlider, globals.m_iVolume);
		
		CPI_Player__SetVolume(globals.m_hPlayer, globals.m_iVolume);
	}
	
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "VolumeUp") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_VolumeDown(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
	{
		globals.m_iVolume--;
		
		if (globals.m_iVolume < 0)
			globals.m_iVolume = 0;
			
		main_draw_vu_from_value(windows.wnd_main, VolumeSlider, globals.m_iVolume);
		
		CPI_Player__SetVolume(globals.m_hPlayer, globals.m_iVolume);
	}
	
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "VolumeDown") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_OpenFile(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
	{
		if (playlist_open_file(TRUE))
			CPL_PlayItem(globals.m_hPlaylist, TRUE, pmCurrentItem);
	}
	
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "OpenFile") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_AddFile(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
	{
		if (playlist_open_file(FALSE))
			{}
	}
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "AddFile") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_About(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
		about_create((HWND)_pParam);
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "About") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_Exit(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
	{
		DestroyWindow(_pParam);
	}
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "Exit") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_SavePlaylist(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
		playlist_write();
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "SavePlaylist") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_PlaylistShuffle(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
	{
		CPL_Stack_Shuffle(globals.m_hPlaylist, TRUE);
	}
	
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "PlaylistShuffle") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_PlaylistOffsetUp(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
		CPlaylistWindow_OffsetSelectedItems(-1);
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "PlaylistOffsetUp") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_PlaylistOffsetDown(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
		CPlaylistWindow_OffsetSelectedItems(1);
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "PlaylistOffsetDown") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_AddDirectory(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
	{
		BROWSEINFO browseinfo;
		LPITEMIDLIST itemlist;
		int image = 0;
		char directorychoice[MAX_PATH];
		
		browseinfo.hwndOwner = (HWND)windows.wnd_main;
		browseinfo.pidlRoot = NULL;
		browseinfo.pszDisplayName = directorychoice;
		browseinfo.lpszTitle = "Choose a directory to add";
		browseinfo.ulFlags = BIF_EDITBOX;
		browseinfo.lpfn = NULL;
		browseinfo.lParam = 0;
		browseinfo.iImage = image;
		
		itemlist = SHBrowseForFolder(&browseinfo);
		
		if (itemlist == NULL)
			return;
			
		SHGetPathFromIDList(itemlist, globals.main_text_last_browsed_dir);
		
		CPL_SyncLoadNextFile(globals.m_hPlaylist);
		
		CPL_AddDirectory_Recurse(globals.m_hPlaylist, globals.main_text_last_browsed_dir);
		
		if (options.shuffle_play)
			CPL_Stack_Shuffle(globals.m_hPlaylist, TRUE);
	}
	
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "AddDirectory") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_PlaylistMinimise(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
		ShowWindow(IF_GetHWnd(windows.m_hifPlaylist), SW_MINIMIZE);
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "PlaylistMinimise") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
void CPVERB_PlaylistMaximise(const CPe_VerbAction enAction, void* _pParam)
{
	if (enAction == vaDoVerb)
	{
		if (IsZoomed(IF_GetHWnd(windows.m_hifPlaylist)))
			ShowWindow(IF_GetHWnd(windows.m_hifPlaylist), SW_RESTORE);
		else
			ShowWindow(IF_GetHWnd(windows.m_hifPlaylist), SW_MAXIMIZE);
	}
	
	else if (enAction == vaQueryName)
	{
		CPs_VerbQueryName* pParam = (CPs_VerbQueryName*)_pParam;
		
		if (stricmp(pParam->m_pcName, "PlaylistMaximise") == 0)
			pParam->m_bNameMatched = TRUE;
	}
}

//
//
//
