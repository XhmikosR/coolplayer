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
#include "CPI_PlaylistItem.h"
#include "CPI_Playlist.h"




////////////////////////////////////////////////////////////////////////////////
//
//
//
void CPL_cb_OnItemUpdated(const CP_HPLAYLISTITEM hItem)
{
	if (globals.m_hPlaylistViewControl)
	{
		const int iItemIDX = CPLI_GetCookie(hItem);
		
		if (iItemIDX != CPC_INVALIDITEM)
			CLV_SetItemData(globals.m_hPlaylistViewControl, iItemIDX, hItem);
	}
}

//
//
//
void CPL_cb_OnPlaylistAppend(const CP_HPLAYLISTITEM hItem)
{
	int iNewItemIDX;
	
	iNewItemIDX = CLV_AddItem(globals.m_hPlaylistViewControl, hItem);
	CPLI_SetCookie(hItem, iNewItemIDX);
}

//
//
//
void CPL_cb_OnPlaylistItemDelete(const CP_HPLAYLISTITEM hItem)
{
	CP_HPLAYLISTITEM hCursor;
	int iItemIDX = CPLI_GetCookie(hItem);
	
	// Remove the item from the list
	CLV_DeleteItem(globals.m_hPlaylistViewControl, iItemIDX);
	
	// We are storing the item number on the item cookie - we must now renumber all
	// list items after this item
	
	for (hCursor = CPLI_Next(hItem); hCursor; hCursor = CPLI_Next(hCursor), iItemIDX++)
		CPLI_SetCookie(hCursor, iItemIDX);
}

//
//
//
void CPL_cb_OnPlaylistEmpty()
{
	if (globals.m_hPlaylistViewControl)
		CLV_RemoveAllItems(globals.m_hPlaylistViewControl);
}

//
//
//
void CPL_cb_OnPlaylistActivationChange(const CP_HPLAYLISTITEM hItem, const BOOL bNewActiveState)
{
	if (bNewActiveState == TRUE)
	{
		main_update_title_text();
		main_draw_title(windows.wnd_main);
		main_draw_tracknr(windows.wnd_main);
	}
}

//
//
//
void CPL_cb_OnPlaylistActivationEmpty()
{
	main_update_title_text();
	main_draw_title(windows.wnd_main);
	main_draw_tracknr(windows.wnd_main);
}

//
//
//
void CPL_cb_SetWindowToReflectList()
{
	CP_HPLAYLISTITEM hCursor;
	CP_HPLAYLISTITEM hSelected;
	
	if (globals.m_hPlaylistViewControl)
		CLV_BeginBatch(globals.m_hPlaylistViewControl);
		
	// Unselect active item
	hSelected = CPL_GetActiveItem(globals.m_hPlaylist);
	
	// Add items to list
	CLV_RemoveAllItems(globals.m_hPlaylistViewControl);
	
	for (hCursor = CPL_GetFirstItem(globals.m_hPlaylist); hCursor; hCursor = CPLI_Next(hCursor))
		CPL_cb_OnPlaylistAppend(hCursor);
		
	// Set active item
	if (hSelected)
		CPL_cb_OnPlaylistActivationChange(hSelected, TRUE);
		
	if (globals.m_hPlaylistViewControl)
		CLV_EndBatch(globals.m_hPlaylistViewControl);
}

//
//
//
void CPL_cb_LockWindowUpdates(const BOOL bLock)
{
	if (globals.m_hPlaylistViewControl)
	{
		if (bLock)
			CLV_BeginBatch(globals.m_hPlaylistViewControl);
		else
			CLV_EndBatch(globals.m_hPlaylistViewControl);
	}
}

//
//
//
void CPL_cb_TrackStackChanged()
{
	if (globals.m_hPlaylistViewControl)
		CLV_Invalidate(globals.m_hPlaylistViewControl);
}

//
//
//

