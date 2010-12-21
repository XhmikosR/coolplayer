
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
// Cooler Playlist
//
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
//
typedef enum _CPe_PlayMode
{
	pmCurrentItem,
	pmNextItem,
	pmPrevItem
} CPe_PlayMode;
//
//
typedef enum _CPe_PlayItemSortElement
{
	piseTrackStackPos,
	piseArtist,
	piseAlbum,
	piseTrackName,
	piseYear,
	piseComment,
	piseTrackNum,
	piseGenre,
	pisePath,
	piseFilename,
	piseLength
} CPe_PlayItemSortElement;
//
//
typedef enum _CPe_ItemStackState
{
	issUnstacked,
	issPlayed,
	issStacked_Top,
	issStacked
} CPe_ItemStackState;
//
//
// Playlist control
CP_HPLAYLIST CPL_CreatePlaylist();
void CPL_DestroyPlaylist(CP_HPLAYLIST hPlaylist);
//
void CPL_Empty(CP_HPLAYLIST hPlaylist);
void CPL_AddDroppedFiles(CP_HPLAYLIST hPlaylist, HDROP hDrop);
void CPL_AddSingleFile(CP_HPLAYLIST hPlaylist, const char* pcPath, const char* pcTitle); // Will not check for a playlist
void CPL_HandleAsyncNotify(CP_HPLAYLIST hPlaylist, WPARAM wParam, LPARAM lParam);
void CPL_AddFile(CP_HPLAYLIST hPlaylist, const char* pcPath); // Will add a file or decode a playlist file
void CPL_AddDirectory_Recurse(CP_HPLAYLIST hPlaylist, const char *pDirectory);
void CPL_RemoveItem(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hItem);
void CPL_SetActiveItem(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hItem);
CP_HPLAYLISTITEM CPL_GetActiveItem(CP_HPLAYLIST hPlaylist);
void CPL_PlayItem(CP_HPLAYLIST hPlaylist, const BOOL bStopFirst, const CPe_PlayMode enPlayMode);
void CPL_RemoveDuplicates(CP_HPLAYLIST hPlaylist);
void CPL_ExportPlaylist(CP_HPLAYLIST hPlaylist, const char* pcOutputName);
void CPL_SortList(CP_HPLAYLIST hPlaylist, const CPe_PlayItemSortElement enElement, const BOOL bDesc);
void CPL_InsertItemBefore(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hItem_Anchor, CP_HPLAYLISTITEM hItem_ToMove);
void CPL_InsertItemAfter(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hItem_Anchor, CP_HPLAYLISTITEM hItem_ToMove);
// Track stack
void CPL_Stack_Append(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hItem);
void CPL_Stack_Remove(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hItem);
void CPL_Stack_SetCursor(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hItem);
void CPL_Stack_Clear(CP_HPLAYLIST hPlaylist);
void CPL_Stack_RestackAll(CP_HPLAYLIST hPlaylist);
void CPL_Stack_Renumber(CP_HPLAYLIST hPlaylist);
CPe_ItemStackState CPL_Stack_GetItemState(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hItem);
void CPL_Stack_Shuffle(CP_HPLAYLIST hPlaylist, const BOOL bForceCurrentToHead);
void CPL_Stack_ClipFromCurrent(CP_HPLAYLIST hPlaylist);
void CPL_Stack_ClipFromItem(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hItem);
void CPL_Stack_PlayNext(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hItem);
void CPL_SyncLoadNextFile(CP_HPLAYLIST hPlaylist);
void CPL_SetAutoActivateInitial(CP_HPLAYLIST hPlaylist, const BOOL bAutoActivateInitial);
//
//
CP_HPLAYLISTITEM CPL_GetFirstItem(CP_HPLAYLIST hPlaylist);
CP_HPLAYLISTITEM CPL_GetLastItem(CP_HPLAYLIST hPlaylist);
CP_HPLAYLISTITEM CPL_GetActiveItem(CP_HPLAYLIST hPlaylist);
CP_HPLAYLISTITEM CPL_FindPlaylistItem(CP_HPLAYLIST hPlaylist, const char* pcPath);
//
//
// Callbacks - these functions are called by the playlist to allow the interface
// to reflect what the current contents of the playlist are
void CPL_cb_OnItemUpdated(const CP_HPLAYLISTITEM hItem);
void CPL_cb_OnPlaylistAppend(const CP_HPLAYLISTITEM hItem);
void CPL_cb_OnPlaylistItemDelete(const CP_HPLAYLISTITEM hItem);
void CPL_cb_OnPlaylistEmpty();
void CPL_cb_OnPlaylistActivationChange(const CP_HPLAYLISTITEM hItem, const BOOL bNewActiveState);
void CPL_cb_OnPlaylistActivationEmpty();
void CPL_cb_SetWindowToReflectList();
void CPL_cb_LockWindowUpdates(const BOOL bLock);
void CPL_cb_TrackStackChanged();
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Worker thread messages
#define CPPLWT_TERMINATE					(WM_APP+0x001)
#define CPPLWT_READTAG						(WM_APP+0x002)
#define CPPLWT_SYNCSHUFFLE					(WM_APP+0x003)
#define CPPLWT_SETACTIVE					(WM_APP+0x004)
//
// Notifies - ensure that these do not clash with those in CPI_Player_Messages.h
#define CPPLNM_TAGREAD						(WM_APP+0x201)
#define CPPLNM_SYNCSHUFFLE					(WM_APP+0x202)
#define CPPLNM_SYNCSETACTIVE				(WM_APP+0x203)
//
////////////////////////////////////////////////////////////////////////////////
