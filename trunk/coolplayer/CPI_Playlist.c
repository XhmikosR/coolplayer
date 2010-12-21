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
#include "CPI_Playlist.h"
#include "CPI_PlaylistItem.h"
#include "CPI_PlaylistItem_Internal.h"
#include "CPI_Player.h"
#include "CPI_Player_Engine.h"

#define CPC_TRACKSTACK_BUFFER_QUANTISATION 32
typedef int (__cdecl *wp_SortFN)(const void *elem1, const void *elem2);
int __cdecl exp_CompareStrings(const void *elem1, const void *elem2);
DWORD WINAPI CPI_PlaylistWorkerThreadEP(void* pCookie);
////////////////////////////////////////////////////////////////////////////////
//

typedef struct _CPs_PlaylistWorkerThreadInfo
{
	DWORD m_dwHostThreadID;
	DWORD m_dwCurrentBatchID;
	
} CPs_PlaylistWorkerThreadInfo;

//
//

typedef struct _CPs_Playlist
{
	CP_HPLAYLISTITEM m_hFirst;
	CP_HPLAYLISTITEM m_hLast;
	CP_HPLAYLISTITEM m_hCurrent;
	
	CP_HPLAYLISTITEM* m_pTrackStack;
	unsigned int m_iTrackStackSize;
	unsigned int m_iTrackStackBufferSize;
	unsigned int m_iTrackStackCursor;
	
	HANDLE m_hWorkerThread;
	DWORD m_dwWorkerThreadID;
	CPs_PlaylistWorkerThreadInfo m_WorkerThreadInfo;
	BOOL m_bSyncLoadNextFile;
	BOOL m_bAutoActivateInitial;
	
} CPs_Playlist;

//
//
typedef enum _CPe_PlayListFileType
{
	pftUnknown,
	pftPLS,
	pftM3U
} CPe_PlayListFileType;
//

typedef struct _CPs_FilenameLLItem
{
	char* m_pcFilename;
	void* m_pNextItem;
} CPs_FilenameLLItem;

//
//
#define CPC_PLAYLISTWORKER_NOTIFYCHUNKSIZE 32

typedef struct _CPs_NotifyChunk
{
	int m_iNumberInChunk;
	CP_HPLAYLISTITEM m_aryItems[CPC_PLAYLISTWORKER_NOTIFYCHUNKSIZE];
	DWORD m_aryBatchIDs[CPC_PLAYLISTWORKER_NOTIFYCHUNKSIZE];
	
} CPs_NotifyChunk;

//
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
//
//
//
CP_HPLAYLIST CPL_CreatePlaylist()
{
	CPs_Playlist* pNewPlaylist = (CPs_Playlist*)malloc(sizeof(CPs_Playlist));
	pNewPlaylist->m_hFirst = NULL;
	pNewPlaylist->m_hLast = NULL;
	pNewPlaylist->m_hCurrent = NULL;
	
	pNewPlaylist->m_pTrackStack = NULL;
	pNewPlaylist->m_iTrackStackSize = 0;
	pNewPlaylist->m_iTrackStackBufferSize = 0;
	pNewPlaylist->m_iTrackStackCursor = 0;
	pNewPlaylist->m_bSyncLoadNextFile = FALSE;
	pNewPlaylist->m_bAutoActivateInitial = FALSE;
	
	pNewPlaylist->m_WorkerThreadInfo.m_dwHostThreadID = GetCurrentThreadId();
	pNewPlaylist->m_WorkerThreadInfo.m_dwCurrentBatchID = 0;
	
	// Create worker thread
	pNewPlaylist->m_hWorkerThread = CreateThread(NULL, 0, CPI_PlaylistWorkerThreadEP, &pNewPlaylist->m_WorkerThreadInfo, 0, &(pNewPlaylist->m_dwWorkerThreadID));
	CP_ASSERT(pNewPlaylist->m_hWorkerThread);
	
	return pNewPlaylist;
}

//
//
//
void CPL_DestroyPlaylist(CP_HPLAYLIST hPlaylist)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	CP_CHECKOBJECT(pPlaylist);
	
	// Stop the worker thread from processing any more pending ID3 reads
	pPlaylist->m_WorkerThreadInfo.m_dwCurrentBatchID++;
	
	// Request worker thread to shutdown
	PostThreadMessage(pPlaylist->m_dwWorkerThreadID, CPPLWT_TERMINATE, 0, 0);
	
	// Clean up list
	CPL_Empty(hPlaylist);
	
	// Delete an unattached active item
	
	if (pPlaylist->m_hCurrent && CPLII_DECODEHANDLE(pPlaylist->m_hCurrent)->m_bDestroyOnDeactivate)
		CPLII_DestroyItem(pPlaylist->m_hCurrent);
		
	// Wait for shutdown to actually happen
	WaitForSingleObject(pPlaylist->m_hWorkerThread, INFINITE);
	
	CloseHandle(pPlaylist->m_hWorkerThread);
	
	// Remove any read ID3s from our message queue
	{
		MSG msg;
		
		while (PeekMessage(&msg, NULL, CPPLNM_TAGREAD, CPPLNM_TAGREAD, PM_REMOVE))
		{
			CPs_NotifyChunk* pChunk = (CPs_NotifyChunk*)msg.wParam;
			int iChunkItemIDX;
			
			// Add all of the items in the chunk
			
			for (iChunkItemIDX = 0; iChunkItemIDX < pChunk->m_iNumberInChunk; iChunkItemIDX++)
				CPLII_DestroyItem(pChunk->m_aryItems[iChunkItemIDX]);
				
			free(pChunk);
		}
	}
	
	// Clean up object
	free(pPlaylist);
}

//
//
//
void CPL_UnlinkItem(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hItem)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	CPs_PlaylistItem* pItemToUnlink = CPLII_DECODEHANDLE(hItem);
	CP_CHECKOBJECT(pPlaylist);
	
	// Remove item from list
	
	if (pItemToUnlink->m_hPrev)
		CPLII_DECODEHANDLE(pItemToUnlink->m_hPrev)->m_hNext = pItemToUnlink->m_hNext;
	else
	{
		CP_ASSERT(pPlaylist->m_hFirst == hItem);
		pPlaylist->m_hFirst = pItemToUnlink->m_hNext;
	}
	
	if (pItemToUnlink->m_hNext)
		CPLII_DECODEHANDLE(pItemToUnlink->m_hNext)->m_hPrev = pItemToUnlink->m_hPrev;
	else
	{
		CP_ASSERT(pPlaylist->m_hLast == hItem);
		pPlaylist->m_hLast = pItemToUnlink->m_hPrev;
	}
}

//
//
//
void CPL_Empty(CP_HPLAYLIST hPlaylist)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	CP_HPLAYLISTITEM hCursor, hNext;
	
	CP_CHECKOBJECT(pPlaylist);
	
	// Stop the worker thread from processing any more pending ID3 reads
	pPlaylist->m_WorkerThreadInfo.m_dwCurrentBatchID++;
	
	// Unlink the active item
	
	if (pPlaylist->m_hCurrent)
	{
		// This is the active item - clear it's next and prev entries and mark it
		// so that it's destroyed when activation next changes
		CPs_PlaylistItem* pActiveItem = CPLII_DECODEHANDLE(pPlaylist->m_hCurrent);
		
		if (pActiveItem->m_bDestroyOnDeactivate == FALSE)
		{
			CPL_UnlinkItem(hPlaylist, pPlaylist->m_hCurrent);
			pActiveItem->m_hNext = NULL;
			pActiveItem->m_hPrev = NULL;
			pActiveItem->m_bDestroyOnDeactivate = TRUE;
			CPL_cb_OnPlaylistActivationChange(pPlaylist->m_hCurrent, FALSE);
			pActiveItem->m_iCookie = CPC_INVALIDITEM;
		}
	}
	
	// Callback
	CPL_cb_OnPlaylistEmpty();
	
	// Clean up items
	hCursor = pPlaylist->m_hFirst;
	
	while (hCursor)
	{
		hNext = CPLI_Next(hCursor);
		CPLII_DestroyItem(hCursor);
		hCursor = hNext;
	}
	
	// Reset state
	pPlaylist->m_hFirst = NULL;
	pPlaylist->m_hLast = NULL;
	
	// Clean up the trackstack
	if (pPlaylist->m_pTrackStack)
		free(pPlaylist->m_pTrackStack);
		
	pPlaylist->m_pTrackStack = NULL;
	pPlaylist->m_iTrackStackSize = 0;
	pPlaylist->m_iTrackStackBufferSize = 0;
	pPlaylist->m_iTrackStackCursor = 0;
}

//
//
//
void CPL_AddSingleFile_pt2(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hNewFile, const DWORD dwBatchID)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	const char* pcPath = CPLI_GetPath(hNewFile);
	
	CP_CHECKOBJECT(pPlaylist);
	
	// Batch has changed since this message was sent
	
	if (dwBatchID != pPlaylist->m_WorkerThreadInfo.m_dwCurrentBatchID)
	{
		CPLII_DestroyItem(hNewFile);
		return;
	}
	
	// If items are only allowed once - look for another instance of this item
	// and skip this add if it is found
	
	if (options.allow_file_once_in_playlist)
	{
		CP_HPLAYLISTITEM hCursor;
		hCursor = pPlaylist->m_hFirst;
		
		while (hCursor)
		{
			// Is this item in the list already?
			if (stricmp(CPLII_DECODEHANDLE(hCursor)->m_pcPath, pcPath) == 0)
			{
				CPLII_DestroyItem(hNewFile);
				return;
			}
			
			hCursor = CPLI_Next(hCursor);
		}
	}
	
	// Add item to the list
	CPLII_DECODEHANDLE(hNewFile)->m_hPrev = pPlaylist->m_hLast;
	
	if (pPlaylist->m_hLast)
		CPLII_DECODEHANDLE(pPlaylist->m_hLast)->m_hNext = hNewFile;
		
	pPlaylist->m_hLast = hNewFile;
	
	if (pPlaylist->m_hFirst == NULL)
		pPlaylist->m_hFirst = hNewFile;
		
	// If there is no track name (ID3 read off or failed) - create one from the path
	if (CPLII_DECODEHANDLE(hNewFile)->m_pcTrackName == NULL)
	{
		int iNumChars;
		int iCharIDX;
		int iLastSlashIDX = CPC_INVALIDCHAR;
		int iLastDotIDX = CPC_INVALIDCHAR;
		int iLastCharIDX = CPC_INVALIDCHAR;
		
		if (_strnicmp(pcPath, CIC_HTTPHEADER, strlen(CIC_HTTPHEADER)) == 0)
			iLastCharIDX = strlen(pcPath);
		else
			for (iCharIDX = 0; pcPath[iCharIDX]; iCharIDX++)
			{
				if (pcPath[iCharIDX] == '\\')
					iLastSlashIDX = iCharIDX;
					
				if (pcPath[iCharIDX] == '.')
					iLastDotIDX = iCharIDX;
					
				iLastCharIDX = iCharIDX;
			}
			
		// Correct indices
		
		if (iLastSlashIDX == CPC_INVALIDCHAR)
			iLastSlashIDX = 0;
		else
			iLastSlashIDX++; // We want the char after the last slash
			
		if (iLastDotIDX == CPC_INVALIDCHAR || iLastDotIDX < iLastSlashIDX)
			iLastDotIDX = iLastCharIDX;
		else
			iLastDotIDX--; // We want the string up to the char before the last dot
			
		// Create title buffer
		iNumChars = (iLastDotIDX - iLastSlashIDX) + 1;
		
		CPLII_DECODEHANDLE(hNewFile)->m_pcTrackName = (char*)malloc(iNumChars + 1);
		
		memcpy(CPLII_DECODEHANDLE(hNewFile)->m_pcTrackName, pcPath + iLastSlashIDX, iNumChars);
		
		CPLII_DECODEHANDLE(hNewFile)->m_pcTrackName[iNumChars] = '\0';
	}
	
	// Add to track stack
	CPL_Stack_Append(hPlaylist, hNewFile);
	
	// Callback
	CPL_cb_OnPlaylistAppend(hNewFile);
}

//
//
//
void CPL_AddSingleFile(CP_HPLAYLIST hPlaylist, const char* pcPath, const char* pcTitle)
{
	char name[256];
	int pos, last, len, dst;
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	CP_HPLAYLISTITEM hNewFile;
	CP_CHECKOBJECT(pPlaylist);
	
	// only allow items we can play
	{
		int i;
		BOOL valid = FALSE;
		CPs_PlayEngine* player = (CPs_PlayEngine*)globals.m_hPlayer;
		CPs_PlayerContext* pContext = (CPs_PlayerContext*)player->m_pContext;
		DWORD tempcookie;
		char *extension = NULL;

		{
			// Find  the extension
			char *dot = strrchr(pcPath, '.');
			if (dot) 
				extension = dot + 1;
		}
		
		if (extension == NULL)
			return;
			
		for (i = 0; i <= CP_CODEC_last; i++)
		{
			if (CPFA_IsAssociated(&pContext->m_CoDecs[i], extension, &tempcookie))
			{
				valid = TRUE;
				break;
			}
		}
		
		// we could get here and still be valid
		// it might get here if a stream is of the form http://ipaddr:port with no
		// file name such as http://ipaddr:port/filename.ogg
		
		if (strnicmp(CIC_HTTPHEADER, pcPath, 5) == 0
				|| strnicmp("https:", pcPath, 6) == 0
				|| strnicmp("ftp:", pcPath, 4) == 0)
			valid = TRUE;
			
		if (valid == FALSE)
			return;
	}
	
	hNewFile = CPLII_CreateItem(pcPath);
	
	// There was a title passed - setup the item accordingly
	
	if (pcTitle && pcTitle[0])
		STR_AllocSetString(&CPLII_DECODEHANDLE(hNewFile)->m_pcTrackName, pcTitle, FALSE);
		
	// Defer this add to the worker thread if we are reading tags
	if (options.read_id3_tag && options.read_id3_tag_in_background && !pPlaylist->m_bSyncLoadNextFile)
	{
		while (!PostThreadMessage(pPlaylist->m_dwWorkerThreadID, CPPLWT_READTAG, (WPARAM)pPlaylist->m_WorkerThreadInfo.m_dwCurrentBatchID, (LPARAM)hNewFile))
		{
			Sleep(50);
		}
		
		// remove all the .. in a path
		// ie: c:\mp3\song\..\somesong.mp3 would become c:\mp3\somesong.mp3
		len = strlen(pcPath);
		last = -1;
		pos = 0;
		dst = 0;
		
		memset(name, 0, 256);
		
		while (pos <= len)
		{
			if (pcPath[pos] == '\\')
			{
				if (pcPath[pos+1] == '.' && pcPath[pos+2] == '.' && last != -1)
				{
					pos += 3;
					dst = last;
					last--;
					
					while (last >= 0)
						if (name[last] == '\\')
							break;
						else
							last--;
				}
				
				else
				{
					last = dst;
					name[dst++] = pcPath[pos++];
				}
			}
			
			else
				name[dst++] = pcPath[pos++];
		}
		
		if (pPlaylist->m_bAutoActivateInitial && stricmp(name, options.initial_file) == 0)
			PostThreadMessage(pPlaylist->m_dwWorkerThreadID, CPPLWT_SETACTIVE, (WPARAM)pPlaylist->m_WorkerThreadInfo.m_dwCurrentBatchID, (LPARAM)hNewFile);
	}
	
	else
	{
		pPlaylist->m_bSyncLoadNextFile = FALSE;
		
		if (options.read_id3_tag)
			CPLI_ReadTag(hNewFile);
			
		// If we didn't get a track length from the tag - work it out
		if (CPLI_GetTrackLength(hNewFile) == 0
				&& options.work_out_track_lengths)
		{
			CPLI_CalculateLength(hNewFile);
		}
		
		CPL_AddSingleFile_pt2(hPlaylist, hNewFile, pPlaylist->m_WorkerThreadInfo.m_dwCurrentBatchID);
	}
}

//
//
//
void CPL_HandleAsyncNotify(CP_HPLAYLIST hPlaylist, WPARAM wParam, LPARAM lParam)
{
	CPs_NotifyChunk* pChunk = (CPs_NotifyChunk*)wParam;
	int iChunkItemIDX;
	
	// Add all of the items in the chunk
	CLV_BeginBatch(globals.m_hPlaylistViewControl);
	
	for (iChunkItemIDX = 0; iChunkItemIDX < pChunk->m_iNumberInChunk; iChunkItemIDX++)
		CPL_AddSingleFile_pt2(globals.m_hPlaylist, pChunk->m_aryItems[iChunkItemIDX], pChunk->m_aryBatchIDs[iChunkItemIDX]);
		
	CLV_EndBatch(globals.m_hPlaylistViewControl);
	
	// Cleanup
	free(pChunk);
}

//
//
//
void CPL_RemoveDuplicates(CP_HPLAYLIST hPlaylist)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	CP_HPLAYLISTITEM hCursor;
	CP_CHECKOBJECT(pPlaylist);
	
	// Scan the playlist removing duplicates
	hCursor = pPlaylist->m_hFirst;
	
	while (hCursor)
	{
		CP_HPLAYLISTITEM hCursor_Scan;
		
		// Look for duplicates after this item (as all items will be scanned
		// in this way there is no need to look for duplicates before this item)
		hCursor_Scan = CPLI_Next(hCursor);
		
		while (hCursor_Scan)
		{
			// Is this a duplicate
			if (stricmp(CPLII_DECODEHANDLE(hCursor_Scan)->m_pcPath,
						CPLII_DECODEHANDLE(hCursor)->m_pcPath) == 0)
			{
				CPL_RemoveItem(hPlaylist, hCursor_Scan);
				
				// Items before the current are already unique - stop scanning
				break;
			}
			
			hCursor_Scan = CPLI_Next(hCursor_Scan);
		}
		
		hCursor = CPLI_Next(hCursor);
	}
}

//
//
//
void CPL_PlayActiveItem(CP_HPLAYLIST hPlaylist, const BOOL bStopFirst)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	CP_CHECKOBJECT(pPlaylist);
	
	// Stop
	
	if (bStopFirst == TRUE)
		CPI_Player__Stop(globals.m_hPlayer);
		
	// Start playing
	if (pPlaylist->m_hCurrent)
	{
		CPI_Player__OpenFile(globals.m_hPlayer, CPLI_GetPath(pPlaylist->m_hCurrent));
		CPI_Player__Play(globals.m_hPlayer);
	}
}

//
//
//
void CPL_RemoveItem(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hItem)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	CP_CHECKOBJECT(pPlaylist);
	
	// Callback
	CPL_cb_OnPlaylistItemDelete(hItem);
	CPL_UnlinkItem(hPlaylist, hItem);
	
	// Remove item from track stack
	CPL_Stack_Remove(hPlaylist, hItem);
	
	if (hItem == pPlaylist->m_hCurrent)
	{
		// This is the active item - clear it's next and prev entries and mark it
		// so that it's destroyed when activation next changes
		CPs_PlaylistItem* pActiveItem = CPLII_DECODEHANDLE(hItem);
		pActiveItem->m_hNext = NULL;
		pActiveItem->m_hPrev = NULL;
		pActiveItem->m_bDestroyOnDeactivate = TRUE;
		CPL_cb_OnPlaylistActivationChange(hItem, FALSE);
		pActiveItem->m_iCookie = CPC_INVALIDITEM;
	}
	
	else
	{
		// Cleanup
		CPLII_DestroyItem(hItem);
	}
}

//
//
//
void CPL_SetActiveItem(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hItem)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	CP_CHECKOBJECT(pPlaylist);
	
	if (pPlaylist->m_hCurrent == hItem)
		return;
		
	// Unset any previous activation state
	if (pPlaylist->m_hCurrent)
	{
		if (CPLII_DECODEHANDLE(pPlaylist->m_hCurrent)->m_bDestroyOnDeactivate)
			CPLII_DestroyItem(pPlaylist->m_hCurrent);
		else
			CPL_cb_OnPlaylistActivationChange(pPlaylist->m_hCurrent, FALSE);
	}
	
	pPlaylist->m_hCurrent = hItem;
	
	// Set new activation state
	
	if (pPlaylist->m_hCurrent)
	{
		CPL_cb_OnPlaylistActivationChange(pPlaylist->m_hCurrent, TRUE);
		
		if (options.read_id3_tag_of_selected == TRUE)
			CPLI_ReadTag(hItem);
	}
	
	else
		CPL_cb_OnPlaylistActivationEmpty();
		
	// Update track stack
	CPL_Stack_SetCursor(hPlaylist, pPlaylist->m_hCurrent);
	
	// Setup the initial file buffer (for remember last played)
	if (pPlaylist->m_hCurrent)
		strncpy(options.initial_file, CPLI_GetPath(hItem), sizeof(options.initial_file));
}

//
//
//
void CPL_PlayItem(CP_HPLAYLIST hPlaylist, const BOOL bStopFirst, const CPe_PlayMode enPlayMode)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	CP_HPLAYLISTITEM hItemToPlay = NULL;
	CP_CHECKOBJECT(pPlaylist);
	
	// Decide on what to play
	
	switch (enPlayMode)
	{
	
		case pmCurrentItem:
		
			if (pPlaylist->m_hCurrent)
			{
				// If the current item is no longer in the list and we are stopping - play the first item
				if (bStopFirst == TRUE || CPLII_DECODEHANDLE(pPlaylist->m_hCurrent)->m_bDestroyOnDeactivate)
				{
					if (pPlaylist->m_iTrackStackSize > 0)
					{
						if (pPlaylist->m_iTrackStackCursor < pPlaylist->m_iTrackStackSize)
							hItemToPlay = pPlaylist->m_pTrackStack[pPlaylist->m_iTrackStackCursor];
						else
							hItemToPlay = pPlaylist->m_pTrackStack[0];
					}
				}
				
				else
					hItemToPlay = pPlaylist->m_hCurrent;
			}
			
			else
			{
				if (pPlaylist->m_iTrackStackCursor < pPlaylist->m_iTrackStackSize)
				{
					hItemToPlay = pPlaylist->m_pTrackStack[pPlaylist->m_iTrackStackCursor];
				}
				
				else
				{
					if (options.shuffle_play)
						CPL_Stack_Shuffle(globals.m_hPlaylist, FALSE);
						
					if (pPlaylist->m_iTrackStackSize > 0)
						hItemToPlay = pPlaylist->m_pTrackStack[0];
				}
			}
			
			break;
			
		case pmNextItem:
		
			// If the currently playing track is not the one at the head of the stack - play the head of the stack
			
			if (pPlaylist->m_hCurrent
					&& pPlaylist->m_iTrackStackCursor < pPlaylist->m_iTrackStackSize
					&& pPlaylist->m_hCurrent != pPlaylist->m_pTrackStack[pPlaylist->m_iTrackStackCursor])
			{
				hItemToPlay = pPlaylist->m_pTrackStack[pPlaylist->m_iTrackStackCursor];
			}
			
			// Play the next item from the track stack
			
			if (hItemToPlay == NULL)
			{
				if (pPlaylist->m_iTrackStackCursor < pPlaylist->m_iTrackStackSize)
					pPlaylist->m_iTrackStackCursor++;
					
				if (pPlaylist->m_iTrackStackCursor < pPlaylist->m_iTrackStackSize)
					hItemToPlay = pPlaylist->m_pTrackStack[pPlaylist->m_iTrackStackCursor];
			}
			
			if (hItemToPlay == NULL && options.repeat_playlist == TRUE)
			{
				if (options.shuffle_play)
					CPL_Stack_Shuffle(globals.m_hPlaylist, FALSE);
					
				if (pPlaylist->m_iTrackStackSize > 0)
					hItemToPlay = pPlaylist->m_pTrackStack[0];
			}
			
			break;
			
		case pmPrevItem:
			// Play the prev item in the track stack
			
			if (pPlaylist->m_iTrackStackCursor > 0)
				hItemToPlay = pPlaylist->m_pTrackStack[pPlaylist->m_iTrackStackCursor-1];
			else
			{
				if (options.repeat_playlist == TRUE)
				{
					if (pPlaylist->m_iTrackStackSize > 0)
						hItemToPlay = pPlaylist->m_pTrackStack[pPlaylist->m_iTrackStackSize-1];
				}
				
				else if (pPlaylist->m_iTrackStackSize > 0)
					hItemToPlay = pPlaylist->m_pTrackStack[0];
			}
			
			break;
			
		default:
			CP_FAIL(UnknownPlayMode);
	}
	
	CPL_SetActiveItem(hPlaylist, hItemToPlay);
	CPL_PlayActiveItem(hPlaylist, bStopFirst);
}

//
//
//
CP_HPLAYLISTITEM CPL_GetFirstItem(CP_HPLAYLIST hPlaylist)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	CP_CHECKOBJECT(pPlaylist);
	
	return pPlaylist->m_hFirst;
}

//
//
//
CP_HPLAYLISTITEM CPL_GetLastItem(CP_HPLAYLIST hPlaylist)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	CP_CHECKOBJECT(pPlaylist);
	
	return pPlaylist->m_hLast;
}

//
//
//
CP_HPLAYLISTITEM CPL_GetActiveItem(CP_HPLAYLIST hPlaylist)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	CP_CHECKOBJECT(pPlaylist);
	
	return pPlaylist->m_hCurrent;
}

//
//
//
CP_HPLAYLISTITEM CPL_FindPlaylistItem(CP_HPLAYLIST hPlaylist, const char* pcPath)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	CP_HPLAYLISTITEM hCursor;
	CP_CHECKOBJECT(pPlaylist);
	
	for (hCursor = pPlaylist->m_hFirst; hCursor; hCursor = CPLI_Next(hCursor))
	{
		CP_TRACE1("Looked at \"%s\"", CPLII_DECODEHANDLE(hCursor)->m_pcPath);
		
		if (stricmp(CPLII_DECODEHANDLE(hCursor)->m_pcPath, pcPath) == 0)
			return hCursor;
	}
	
	return NULL;
}

//
//
//
void WriteFile_Text(HANDLE hFile, const char* pcLine, const BOOL bAppendCR)
{
	DWORD dwBytesWritten;
	int iLineLen = strlen(pcLine);
	WriteFile(hFile, pcLine, iLineLen, &dwBytesWritten, NULL);
	
	if (bAppendCR)
		WriteFile(hFile, "\r\n", 2, &dwBytesWritten, NULL);
}

//
//
//
CPe_PlayListFileType CPL_GetFileType(const char* pcPath)
{
	// Determine format from file extension
	const char* pcExtension = NULL;
	int iCharIDX;
	
	// Find the extension (the chars after the last dot)
	
	for (iCharIDX = 0; pcPath[iCharIDX]; iCharIDX++)
	{
		if (pcPath[iCharIDX] == '.')
			pcExtension = pcPath + iCharIDX + 1;
		else if (pcPath[iCharIDX] == '\\')
			pcExtension = NULL;
	}
	
	// No extension - we don't know what format to use!
	
	if (pcExtension == NULL)
		return pftUnknown;
		
	if (stricmp(pcExtension, "pls") == 0)
		return pftPLS;
	else if (stricmp(pcExtension, "m3u") == 0)
		return pftM3U;
		
	return pftUnknown;
}

//
//
//
unsigned int CPL_GetPathVolumeBytes(const char* pcPath)
{
	// We understand volumes in the format of C:\ or \\SYSTEMNAME\SHAREPOINT\ so look for
	// these
	if (pcPath[1] == ':')
		return 3;
	else if (pcPath[0] == '\\' && pcPath[1] == '\\')
	{
		int iCharIDX;
		int iNumSlashesFound;
		
		// UNCs format is \\SERVER\SharePoint\path
		
		// Find the second slash (skipping the double slash at the start)
		iNumSlashesFound = 0;
		
		for (iCharIDX = 2; pcPath[iCharIDX]; iCharIDX++)
		{
			if (pcPath[iCharIDX] == '\\')
				iNumSlashesFound++;
				
			// We've found the second slash - build the prefix
			if (iNumSlashesFound == 2)
				return iCharIDX + 1;
		}
	}
	
	else if (_strnicmp(pcPath, CIC_HTTPHEADER, sizeof(CIC_HTTPHEADER) - 1) == 0)
		return sizeof(CIC_HTTPHEADER);
	else if (_strnicmp(pcPath, CIC_HTTPSHEADER, sizeof(CIC_HTTPSHEADER) - 1) == 0)
		return sizeof(CIC_HTTPSHEADER);
	else if (_strnicmp(pcPath, CIC_FTPHEADER, sizeof(CIC_FTPHEADER) - 1) == 0)
		return sizeof(CIC_FTPHEADER);
		
	// There is no volume information
	return 0;
}

//
//
//
unsigned int CPL_GetPathDirectoryBytes(const char* pcPath, const unsigned int iVolumeBytes)
{
	unsigned int iCharIDX;
	unsigned int iLastSlashIDX;
	
	// Find the last slash and trim everything before it
	// - if there is no directory stub then empty this string
	iLastSlashIDX = 0;
	
	for (iCharIDX = iVolumeBytes; pcPath[iCharIDX]; iCharIDX++)
	{
		if ((pcPath[iCharIDX] == '\\') || (pcPath[iCharIDX] == '/'))
			iLastSlashIDX = iCharIDX + 1;
	}
	
	return iLastSlashIDX;
}

//
//
//
void CPL_ExportPlaylist(CP_HPLAYLIST hPlaylist, const char* pcOutputName)
{
	HANDLE hOutputFile;
	CPe_PlayListFileType enFileType;
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	const unsigned int iPlaylist_VolumeBytes = CPL_GetPathVolumeBytes(pcOutputName);
	const unsigned int iPlaylist_DirectoryBytes = CPL_GetPathDirectoryBytes(pcOutputName, iPlaylist_VolumeBytes);
	
	CP_CHECKOBJECT(pPlaylist);
	
	// Check for known file types
	enFileType = CPL_GetFileType(pcOutputName);
	
	if (enFileType == pftUnknown)
		return;
		
	// Open the file
	hOutputFile = CreateFile(pcOutputName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if (hOutputFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(windows.wnd_main, "Could not open file", "Error", MB_ICONERROR);
		return;
	}
	
	// Go through all playlist items - outputting them to the file
	{
		CP_HPLAYLISTITEM hCursor;
		int iFileNumber;
		
		// If this is a PLS file then write some header info
		
		if (enFileType == pftPLS)
		{
			int iNumberOfEntries = 0;
			char cNumEntriesLine[32];
			
			WriteFile_Text(hOutputFile, "[PlayList]", TRUE);
			
			// Count the number of playlist items
			
			for (hCursor = pPlaylist->m_hFirst; hCursor; hCursor = CPLI_Next(hCursor))
				iNumberOfEntries++;
				
			sprintf(cNumEntriesLine, "NumberOfEntries=%d", iNumberOfEntries);
			
			WriteFile_Text(hOutputFile, cNumEntriesLine, TRUE);
		}
		
		iFileNumber = 0;
		
		for (hCursor = pPlaylist->m_hFirst; hCursor; hCursor = CPLI_Next(hCursor), iFileNumber++)
		{
			char cRelPath[MAX_PATH];
			const char* _pcFilename = CPLI_GetPath(hCursor);
			
			// We prefer relative paths in our playlist files - only works if playlist
			// and target are on the same volume
			
			if (_strnicmp(_pcFilename, pcOutputName, iPlaylist_VolumeBytes) == 0)
			{
				// - so strip off the directory stubs that the file and playlist may have in common
				const char* pcLastCommonSplitPoint = _pcFilename;
				unsigned int iCharIDX;
				
				for (iCharIDX = 0; _pcFilename[iCharIDX] && iCharIDX < iPlaylist_DirectoryBytes; iCharIDX++)
				{
					if (tolower(_pcFilename[iCharIDX]) != tolower(pcOutputName[iCharIDX]))
						break;
						
					if (_pcFilename[iCharIDX] == '\\')
						pcLastCommonSplitPoint = _pcFilename + iCharIDX + 1;
				}
				
				// - add a .. for every slash left in the playlist's path
				cRelPath[0] = '\0';
				
				for (; iCharIDX < iPlaylist_DirectoryBytes; iCharIDX++)
				{
					if (pcOutputName[iCharIDX] == '\\')
						strcat(cRelPath, "..\\");
				}
				
				strcat(cRelPath, pcLastCommonSplitPoint);
			}
			
			else
				strcpy(cRelPath, _pcFilename);
				
			// PLS files have the format FileXXX=pathname - we want to write the stuff up to (and including)
			// the equals sign
			if (enFileType == pftPLS)
			{
				char cPlsFileHeader[32];
				sprintf(cPlsFileHeader, "File%d=", iFileNumber + 1);
				WriteFile_Text(hOutputFile, cPlsFileHeader, FALSE);
			}
			
			// Write the filename
			WriteFile_Text(hOutputFile, cRelPath, TRUE);
		}
	}
	
	CloseHandle(hOutputFile);
}

//
//
//
void CPL_AddPrefixedFile(CP_HPLAYLIST hPlaylist,
						 const char* pcFilename, const char* pcTitle,
						 const char* pcPlaylistFile,
						 const unsigned int iPlaylist_VolumeBytes,
						 const unsigned int iPlaylist_DirBytes)
{
	const unsigned int iFile_VolumeBytes = CPL_GetPathVolumeBytes(pcFilename);
	
	// If the file has volume information - add it as it is
	
	if (iFile_VolumeBytes)
		CPL_AddSingleFile(hPlaylist, pcFilename, pcTitle);
		
	// If the filename has a leading \ then add it prepended by the playlist's volume
	else if (pcFilename[0] == '\\')
	{
		char cFullPath[MAX_PATH];
		memcpy(cFullPath, pcPlaylistFile, iPlaylist_VolumeBytes);
		strcpy(cFullPath + iPlaylist_VolumeBytes, pcFilename + 1);
		CPL_AddSingleFile(hPlaylist, cFullPath, pcTitle);
	}
	
	// Add the filename prepended by the playlist's directory
	
	else
	{
		char cFullPath[MAX_PATH];
		memcpy(cFullPath, pcPlaylistFile, iPlaylist_DirBytes);
		strcpy(cFullPath + iPlaylist_DirBytes, pcFilename);
		CPL_AddSingleFile(hPlaylist, cFullPath, pcTitle);
	}
}

//
//
//
/** // TODO: - make AddFile load playlists from URLs **/
// currently only supports loading m3u from URL
// and has code duplication for reading m3u file
void CPL_AddFile(CP_HPLAYLIST hPlaylist, const char* pcFilename)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	CPe_PlayListFileType enFileType;
	unsigned int iPlaylist_VolumeBytes;
	unsigned int iPlaylist_DirectoryBytes;
	CP_CHECKOBJECT(pPlaylist);
	
	// Check for known file types
	enFileType = CPL_GetFileType(pcFilename);
	
	if (enFileType == pftUnknown)
	{
		// This doesn't seem to be a playlist file - add it as a playlist item
		CPL_AddSingleFile(hPlaylist, pcFilename, NULL);
		return;
	}
	
	// Get playlist file information
	iPlaylist_VolumeBytes = CPL_GetPathVolumeBytes(pcFilename);
	iPlaylist_DirectoryBytes = CPL_GetPathDirectoryBytes(pcFilename, iPlaylist_VolumeBytes);
	
	// Load the playlist files
	CPL_cb_LockWindowUpdates(TRUE);
	
	if (enFileType == pftPLS)
	{
		int iNumFiles, iFileIDX;
		
		iNumFiles = GetPrivateProfileInt("playlist", "NumberOfEntries", 0, pcFilename);
		
		for (iFileIDX = 0; iFileIDX < iNumFiles; iFileIDX++)
		{
			DWORD dwNumCharsRead;
			char cPlsFileHeader[32];
			char cBuffer[MAX_PATH];
			char cTitle[1024];
			sprintf(cPlsFileHeader, "File%d", iFileIDX + 1);
			
			// Get the path - leave room for a drive
			dwNumCharsRead = GetPrivateProfileString("playlist", cPlsFileHeader, NULL, cBuffer, MAX_PATH, pcFilename);
			
			if (dwNumCharsRead == 0)
				continue;
				
			sprintf(cPlsFileHeader, "Title%d", iFileIDX + 1);
			
			dwNumCharsRead = GetPrivateProfileString("playlist", cPlsFileHeader, NULL, cTitle, 1024, pcFilename);
			
			if (dwNumCharsRead == 0)
				CPL_AddPrefixedFile(hPlaylist, cBuffer, NULL, pcFilename, iPlaylist_VolumeBytes, iPlaylist_DirectoryBytes);
			else
				CPL_AddPrefixedFile(hPlaylist, cBuffer, cTitle, pcFilename, iPlaylist_VolumeBytes, iPlaylist_DirectoryBytes);
		}
	}
	
	else
	{
		// Open file and load it all into memory
		HANDLE hFile;
		HINTERNET hURLStream;
		HINTERNET hInternet;
		DWORD dwTimeout, dwBytesRead;
		INTERNET_BUFFERS internetbuffer;
		char *pcPlaylistBuffer;
		unsigned int iLastLineStartIDX, iCharIDX;
		
		// Perform reading
		BOOL bReadResult;
		
		
		// If the path is a URL, we will read the playlist from the internet.
		
		if ((_strnicmp(pcFilename, CIC_HTTPHEADER, sizeof(CIC_HTTPHEADER) - 1) == 0) ||
				(_strnicmp(pcFilename, CIC_HTTPSHEADER, sizeof(CIC_HTTPSHEADER) - 1) == 0) ||
				(_strnicmp(pcFilename, CIC_FTPHEADER, sizeof(CIC_FTPHEADER) - 1) == 0))
		{
			// This playlist is located on the internet, so we have to download it.
			hInternet = InternetOpen(CP_COOLPLAYER,
									 INTERNET_OPEN_TYPE_PRECONFIG,
									 NULL, NULL, 0L);
			                         
			if (hInternet == NULL)
			{
				CP_TRACE0("CPL_AddFile::NoInternetOpen");
				return;
			}
			
			dwTimeout = 2000;
			InternetSetOption(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &dwTimeout, sizeof(dwTimeout));
			
			hURLStream = InternetOpenUrl(hInternet,
										 pcFilename,
										 NULL,
										 0,
										 INTERNET_FLAG_NO_CACHE_WRITE
										 | INTERNET_FLAG_PRAGMA_NOCACHE,
										 0);
			                             
			if (hURLStream == NULL)
			{
				InternetCloseHandle(hInternet);
				CP_TRACE1("CPL_AddFile::NoOpenURL %s", pcFilename);
				return;
			}
			
			// We set up a 256k buffer to download the playlist.  If it's not enough, we won't bother reading the playlist.
			pcPlaylistBuffer = (char*)malloc(0x40001);
			
			if (!pcPlaylistBuffer)
			{
				// Failed to allocate, we're done.
				InternetCloseHandle(hInternet);
				InternetCloseHandle(hURLStream);
				CP_TRACE1("CPL_AddFile::AllocateError %s", pcFilename);
				return;
			}
			
			// Setup the internet buffer
			internetbuffer.dwStructSize = sizeof(internetbuffer);
			internetbuffer.Next = NULL;
			internetbuffer.lpcszHeader = NULL;
			internetbuffer.lpvBuffer = pcPlaylistBuffer;
			internetbuffer.dwBufferLength = 0x40000;
			
			// We attemt to read the file in a single read.  If that doesn't work. we don't
			// bother to continue.
			bReadResult = InternetReadFileEx(hURLStream, &internetbuffer, IRF_NO_WAIT, 0);
			
			InternetCloseHandle(hURLStream);
			
			InternetCloseHandle(hInternet);
			
			if ((!bReadResult) || (!internetbuffer.dwBufferLength))
			{
				// We've got no data
				CP_TRACE1("CPL_AddFile::NoDataReturned %s", pcFilename);
				return;
			}
			
			// Read in the file line by line
			iLastLineStartIDX = 0;
			
			for (iCharIDX = 0; iCharIDX < internetbuffer.dwBufferLength + 1; iCharIDX++)
			{
				if ((pcPlaylistBuffer[iCharIDX] == '\r'
						|| pcPlaylistBuffer[iCharIDX] == '\n'
						|| iCharIDX == internetbuffer.dwBufferLength)
						&& iLastLineStartIDX < iCharIDX)
				{
					char cBuffer[512];
					
					// Is there a file on this line (strip whitespace from start)
					
					if (sscanf(pcPlaylistBuffer + iLastLineStartIDX, " %512[^\r\n]", cBuffer) == 1)
					{
						// Something has been read - ignore lines starting with #
						if (cBuffer[0] != '#')
							CPL_AddPrefixedFile(hPlaylist, cBuffer, NULL, pcFilename, iPlaylist_VolumeBytes, iPlaylist_DirectoryBytes);
					}
					
					// Set the line start for the next line
					
					if (pcPlaylistBuffer[iCharIDX + 1] == '\n')
						iCharIDX++;
						
					iLastLineStartIDX = iCharIDX + 1;
				}
			}
			
			free(pcPlaylistBuffer);
		}
		
		else
		{
			// It's not a URL, so we will read the file from a local (UNC) resource
			hFile = CreateFile(pcFilename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			
			if (hFile != INVALID_HANDLE_VALUE)
			{
				const DWORD dwFileSize = GetFileSize(hFile, NULL);
				
				// We will only load playlists that are smaller than 256K
				
				if (dwFileSize < 0x40000)
				{
					// The plan is to load the entire file into a memblock and then split it into lines
					// and scan off the whitepace and add the items to the list
					pcPlaylistBuffer = (char*)malloc(dwFileSize + 1);
					ReadFile(hFile, pcPlaylistBuffer, dwFileSize, &dwBytesRead, NULL);
					
					// Read in the file line by line
					iLastLineStartIDX = 0;
					
					for (iCharIDX = 0; iCharIDX < dwFileSize + 1; iCharIDX++)
					{
						if ((pcPlaylistBuffer[iCharIDX] == '\r'
								|| pcPlaylistBuffer[iCharIDX] == '\n'
								|| iCharIDX == dwFileSize)
								&& iLastLineStartIDX < iCharIDX)
						{
							char cBuffer[512];
							
							// Is there a file on this line (strip whitespace from start)
							
							if (sscanf(pcPlaylistBuffer + iLastLineStartIDX, " %512[^\r\n]", cBuffer) == 1)
							{
								// Something has been read - ignore lines starting with #
								if (cBuffer[0] != '#')
									CPL_AddPrefixedFile(hPlaylist, cBuffer, NULL, pcFilename, iPlaylist_VolumeBytes, iPlaylist_DirectoryBytes);
							}
							
							// Set the line start for the next line
							
							if (pcPlaylistBuffer[iCharIDX + 1] == '\n')
								iCharIDX++;
								
							iLastLineStartIDX = iCharIDX + 1;
						}
					}
					
					free(pcPlaylistBuffer);
				}
				
				CloseHandle(hFile);
			}
		}
	}
	
	if (options.shuffle_play)
		PostThreadMessage(pPlaylist->m_dwWorkerThreadID, CPPLWT_SYNCSHUFFLE, 0, 0);
		
	CPL_cb_LockWindowUpdates(FALSE);
}

//
//
//
int __cdecl cpl_sort_Path(const void *e1, const void *e2)
{
	const CPs_PlaylistItem* pElem1 = *(const CPs_PlaylistItem**)e1;
	const CPs_PlaylistItem* pElem2 = *(const CPs_PlaylistItem**)e2;
	
	return stricmp(pElem1->m_pcPath, pElem2->m_pcPath);
}

//
//
//
int __cdecl cpl_sort_Filename(const void *e1, const void *e2)
{
	const CPs_PlaylistItem* pElem1 = *(const CPs_PlaylistItem**)e1;
	const CPs_PlaylistItem* pElem2 = *(const CPs_PlaylistItem**)e2;
	
	return stricmp(pElem1->m_pcPath, pElem2->m_pcPath);
}

//
//
//
int __cdecl cpl_sort_TrackNum(const void *e1, const void *e2)
{
	const CPs_PlaylistItem* pElem1 = *(const CPs_PlaylistItem**)e1;
	const CPs_PlaylistItem* pElem2 = *(const CPs_PlaylistItem**)e2;
	
	if (pElem1->m_cTrackNum == pElem2->m_cTrackNum)
		return cpl_sort_Path(e1, e2);
	else if ((char)pElem1->m_cTrackNum < (char)pElem2->m_cTrackNum)
		return -1;
		
	return 1;
}

//
//
//
int __cdecl cpl_sort_Length(const void *e1, const void *e2)
{
	const CPs_PlaylistItem* pElem1 = *(const CPs_PlaylistItem**)e1;
	const CPs_PlaylistItem* pElem2 = *(const CPs_PlaylistItem**)e2;
	
	if (pElem1->m_iTrackLength == pElem2->m_iTrackLength)
		return 0;
	else if (pElem1->m_iTrackLength < pElem2->m_iTrackLength)
		return -1;
		
	return 1;
}

//
//
//
int __cdecl cpl_sort_TrackStackPos(const void *e1, const void *e2)
{
	const CPs_PlaylistItem* pElem1 = *(const CPs_PlaylistItem**)e1;
	const CPs_PlaylistItem* pElem2 = *(const CPs_PlaylistItem**)e2;
	
	if (pElem1->m_iTrackStackPos == pElem2->m_iTrackStackPos)
		return 0;
		
	if (pElem1->m_iTrackStackPos == CIC_TRACKSTACK_UNSTACKED)
		return 1;
		
	if (pElem2->m_iTrackStackPos == CIC_TRACKSTACK_UNSTACKED)
		return -1;
		
	if (pElem1->m_iTrackStackPos > pElem2->m_iTrackStackPos)
		return 1;
	else
		return -1;
}

//
//
//
int __cdecl cpl_sort_TrackName(const void *e1, const void *e2)
{
	const CPs_PlaylistItem* pElem1 = *(const CPs_PlaylistItem**)e1;
	const CPs_PlaylistItem* pElem2 = *(const CPs_PlaylistItem**)e2;
	
	return stricmp(pElem1->m_pcTrackName, pElem2->m_pcTrackName);
}

//
//
//
int __cdecl cpl_sort_Album(const void *e1, const void *e2)
{
	const CPs_PlaylistItem* pElem1 = *(const CPs_PlaylistItem**)e1;
	const CPs_PlaylistItem* pElem2 = *(const CPs_PlaylistItem**)e2;
	int iStringCompare;
	
	if (pElem1->m_pcAlbum == NULL && pElem2->m_pcAlbum == NULL)
		return cpl_sort_TrackNum(e1, e2);
	else if (pElem1->m_pcAlbum == NULL)
		return -1;
	else if (pElem2->m_pcAlbum == NULL)
		return 1;
		
	// Sort by artist - but fall back to track name
	iStringCompare = stricmp(pElem1->m_pcAlbum, pElem2->m_pcAlbum);
	
	if (iStringCompare != 0)
		return iStringCompare;
		
	return cpl_sort_TrackNum(e1, e2);
}

//
//
//
int __cdecl cpl_sort_Artist(const void *e1, const void *e2)
{
	const CPs_PlaylistItem* pElem1 = *(const CPs_PlaylistItem**)e1;
	const CPs_PlaylistItem* pElem2 = *(const CPs_PlaylistItem**)e2;
	int iStringCompare;
	
	if (pElem1->m_pcArtist == NULL && pElem2->m_pcArtist == NULL)
		return cpl_sort_Album(e1, e2);
	else if (pElem1->m_pcArtist == NULL)
		return -1;
	else if (pElem2->m_pcArtist == NULL)
		return 1;
		
	// Sort by artist - but fall back to track name
	iStringCompare = stricmp(pElem1->m_pcArtist, pElem2->m_pcArtist);
	
	if (iStringCompare != 0)
		return iStringCompare;
		
	return cpl_sort_Album(e1, e2);
}

//
//
//
int __cdecl cpl_sort_Year(const void *e1, const void *e2)
{
	const CPs_PlaylistItem* pElem1 = *(const CPs_PlaylistItem**)e1;
	const CPs_PlaylistItem* pElem2 = *(const CPs_PlaylistItem**)e2;
	int iStringCompare;
	
	if (pElem1->m_pcYear == NULL && pElem2->m_pcYear == NULL)
		return cpl_sort_Artist(e1, e2);
	else if (pElem1->m_pcYear == NULL)
		return -1;
	else if (pElem2->m_pcYear == NULL)
		return 1;
		
	// Sort by artist - but fall back to track name
	iStringCompare = stricmp(pElem1->m_pcYear, pElem2->m_pcYear);
	
	if (iStringCompare != 0)
		return iStringCompare;
		
	return cpl_sort_Artist(e1, e2);
}

//
//
//
int __cdecl cpl_sort_Genre(const void *e1, const void *e2)
{
	const CPs_PlaylistItem* pElem1 = *(const CPs_PlaylistItem**)e1;
	const CPs_PlaylistItem* pElem2 = *(const CPs_PlaylistItem**)e2;
	int iStringCompare;
	
	if (CPLI_GetGenre((CP_HPLAYLISTITEM)pElem1) == NULL && CPLI_GetGenre((CP_HPLAYLISTITEM)pElem2) == NULL)
		return cpl_sort_Artist(e1, e2);
	else if (CPLI_GetGenre((CP_HPLAYLISTITEM)pElem1) == NULL)
		return -1;
	else if (CPLI_GetGenre((CP_HPLAYLISTITEM)pElem2) == NULL)
		return 1;
		
	// Sort by artist - but fall back to track name
	iStringCompare = stricmp(CPLI_GetGenre((CP_HPLAYLISTITEM)pElem1), CPLI_GetGenre((CP_HPLAYLISTITEM)pElem2));
	
	if (iStringCompare != 0)
		return iStringCompare;
		
	return cpl_sort_Artist(e1, e2);
}

//
//
//
int __cdecl cpl_sort_Comment(const void *e1, const void *e2)
{
	const CPs_PlaylistItem* pElem1 = *(const CPs_PlaylistItem**)e1;
	const CPs_PlaylistItem* pElem2 = *(const CPs_PlaylistItem**)e2;
	int iStringCompare;
	
	if (pElem1->m_pcComment == NULL && pElem2->m_pcComment == NULL)
		return cpl_sort_Artist(e1, e2);
	else if (pElem1->m_pcComment == NULL)
		return -1;
	else if (pElem2->m_pcComment == NULL)
		return 1;
		
	// Sort by artist - but fall back to track name
	iStringCompare = stricmp(pElem1->m_pcComment, pElem2->m_pcComment);
	
	if (iStringCompare != 0)
		return iStringCompare;
		
	return cpl_sort_Artist(e1, e2);
}

//
//
//
int __cdecl cpl_sort_Random(const void *e1, const void *e2)
{
	const unsigned short r1 = rand();
	const unsigned short r2 = rand();
	
	if (r1 < r2)
		return -1;
	else if (r1 == r2)
		return 0;
	else
		return 1;
}

//
//
//
void CPL_SortList(CP_HPLAYLIST hPlaylist, const CPe_PlayItemSortElement enElement, const BOOL bDesc)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	CPs_PlaylistItem** pFlatArray;
	unsigned int iNumItems;
	wp_SortFN pfnSort = cpl_sort_TrackStackPos;
	CP_CHECKOBJECT(pPlaylist);
	
	// Skip if list is empty
	
	if (pPlaylist->m_hFirst == NULL)
		return;
		
	// Decide sort function
	{
		switch (enElement)
		{
		
			case piseTrackStackPos:
				pfnSort = cpl_sort_TrackStackPos;
				break;
				
			case piseTrackName:
				pfnSort = cpl_sort_TrackName;
				break;
				
			case piseArtist:
				pfnSort = cpl_sort_Artist;
				break;
				
			case piseAlbum:
				pfnSort = cpl_sort_Album;
				break;
				
			case piseYear:
				pfnSort = cpl_sort_Year;
				break;
				
			case piseTrackNum:
				pfnSort = cpl_sort_TrackNum;
				break;
				
			case piseGenre:
				pfnSort = cpl_sort_Genre;
				break;
				
			case piseComment:
				pfnSort = cpl_sort_Comment;
				break;
				
			case piseFilename:
				pfnSort = cpl_sort_Filename;
				break;
				
			case pisePath:
				pfnSort = cpl_sort_Path;
				break;
				
			case piseLength:
				pfnSort = cpl_sort_Length;
				break;
				
			default:
				CP_FAIL(UnknownSortOrder);
		}
	}
	
	// Count items
	{
		CP_HPLAYLISTITEM hCursor;
		iNumItems = 0;
		
		for (hCursor = pPlaylist->m_hFirst; hCursor; hCursor = CPLI_Next(hCursor))
			iNumItems++;
	}
	
	// Build flat array
	{
		CP_HPLAYLISTITEM hCursor;
		int iItemIDX;
		pFlatArray = (CPs_PlaylistItem**)malloc(sizeof(CPs_PlaylistItem*) * iNumItems);
		
		iItemIDX = 0;
		
		for (hCursor = pPlaylist->m_hFirst; hCursor; hCursor = CPLI_Next(hCursor), iItemIDX++)
			pFlatArray[iItemIDX] = CPLII_DECODEHANDLE(hCursor);
	}
	
	// Qsort it
	qsort(pFlatArray, iNumItems, sizeof(CPs_PlaylistItem*), pfnSort);
	
	// Relink list
	{
		int iFirstItem, iTermItem, iInc;
		CP_HPLAYLISTITEM* phCursor_Referrer = &(pPlaylist->m_hFirst);
		CP_HPLAYLISTITEM hCursor_Prev = NULL;
		CP_HPLAYLISTITEM hLastAssignment = NULL;
		int iItemIDX;
		
		// Work out how to traverse the flat array
		
		if (bDesc == FALSE)
		{
			iFirstItem = 0;
			iTermItem = iNumItems;
			iInc = 1;
		}
		
		else
		{
			iFirstItem = iNumItems - 1;
			iTermItem = -1;
			iInc = -1;
		}
		
		// Traverse the array
		
		for (iItemIDX = iFirstItem; iItemIDX != iTermItem; iItemIDX += iInc)
		{
			*phCursor_Referrer = pFlatArray[iItemIDX];
			pFlatArray[iItemIDX]->m_hPrev = hCursor_Prev;
			
			phCursor_Referrer = &(CPLII_DECODEHANDLE(*phCursor_Referrer)->m_hNext);
			hCursor_Prev = pFlatArray[iItemIDX];
			hLastAssignment = hCursor_Prev;
		}
		
		pPlaylist->m_hLast = hLastAssignment;
		
		CPLII_DECODEHANDLE(pPlaylist->m_hLast)->m_hNext = NULL;
	}
	
	free(pFlatArray);
	
	CPL_cb_SetWindowToReflectList();
}

//
//
//
void CPL_InsertItemBefore(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hItem_Anchor, CP_HPLAYLISTITEM hItem_ToMove)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	CPs_PlaylistItem* pAnchor = CPLII_DECODEHANDLE(hItem_Anchor);
	CP_CHECKOBJECT(pPlaylist);
	
	CPL_UnlinkItem(hPlaylist, hItem_ToMove);
	
	if (pAnchor->m_hPrev)
	{
		CPLII_DECODEHANDLE(pAnchor->m_hPrev)->m_hNext = hItem_ToMove;
		CPLII_DECODEHANDLE(hItem_ToMove)->m_hPrev = pAnchor->m_hPrev;
	}
	
	else
	{
		pPlaylist->m_hFirst = hItem_ToMove;
		CPLII_DECODEHANDLE(hItem_ToMove)->m_hPrev = NULL;
	}
	
	pAnchor->m_hPrev = hItem_ToMove;
	
	CPLII_DECODEHANDLE(hItem_ToMove)->m_hNext = hItem_Anchor;
}

//
//
//
void CPL_InsertItemAfter(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hItem_Anchor, CP_HPLAYLISTITEM hItem_ToMove)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	CPs_PlaylistItem* pAnchor = CPLII_DECODEHANDLE(hItem_Anchor);
	CP_CHECKOBJECT(pPlaylist);
	
	CPL_UnlinkItem(hPlaylist, hItem_ToMove);
	
	if (pAnchor->m_hNext)
	{
		CPLII_DECODEHANDLE(pAnchor->m_hNext)->m_hPrev = hItem_ToMove;
		CPLII_DECODEHANDLE(hItem_ToMove)->m_hNext = pAnchor->m_hNext;
	}
	
	else
	{
		pPlaylist->m_hLast = hItem_ToMove;
		CPLII_DECODEHANDLE(hItem_ToMove)->m_hNext = NULL;
	}
	
	pAnchor->m_hNext = hItem_ToMove;
	
	CPLII_DECODEHANDLE(hItem_ToMove)->m_hPrev = hItem_Anchor;
}

//
//
//
void SortLList(CPs_FilenameLLItem* pFirst)
{
	char** ppStrings;
	int iNumStrings;
	int iStringIDX;
	CPs_FilenameLLItem* pCursor;
	
	if (!pFirst)
		return;
		
	// Count the number of strings in the list
	iNumStrings = 0;
	
	for (pCursor = pFirst; pCursor; pCursor = (CPs_FilenameLLItem*)pCursor->m_pNextItem)
		iNumStrings++;
		
	// Allocate string buffer and assign strings to it
	ppStrings = (char**)malloc(sizeof(char*) * iNumStrings);
	
	iStringIDX = 0;
	
	for (pCursor = pFirst; pCursor; pCursor = (CPs_FilenameLLItem*)pCursor->m_pNextItem)
	{
		ppStrings[iStringIDX] = pCursor->m_pcFilename;
		iStringIDX++;
	}
	
	// Sort filelist
	qsort(ppStrings, iNumStrings, sizeof(char*), exp_CompareStrings);
	
	// Assign list to the now sorted string list
	iStringIDX = 0;
	
	for (pCursor = pFirst; pCursor; pCursor = (CPs_FilenameLLItem*)pCursor->m_pNextItem)
	{
		pCursor->m_pcFilename = ppStrings[iStringIDX];
		iStringIDX++;
	}
	
	free(ppStrings);
}

//
//
//
void CPL_AddDirectory_Recurse(CP_HPLAYLIST hPlaylist, const char *pDir)
{
	CPs_FilenameLLItem* m_pFirstFile = NULL;
	CPs_FilenameLLItem* m_pFirstDir = NULL;
	CPs_FilenameLLItem* pCursor;
	CPs_FilenameLLItem* pNextItem;
	char cFullPath[MAX_PATH];
	char cWildCard[MAX_PATH];
	const int iDirStrLen = strlen(pDir);
	WIN32_FIND_DATA finddata;
	HANDLE hFileFind;
#ifdef _DEBUG
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	CP_CHECKOBJECT(pPlaylist);
#endif
	
	// Build a full path to the directory
	strcpy(cFullPath, pDir);
	
	if (cFullPath[iDirStrLen-1] == '\\' && iDirStrLen > 1)
		cFullPath[iDirStrLen-1] = '\0';
		
	// Check that this is a correct path
	if (cFullPath[0] == '\0' || path_is_directory(cFullPath) == FALSE)
	{
		MessageBox(NULL, "Not a valid directory.", cFullPath, MB_ICONERROR);
		return;
	}
	
	//Scan directory building a list of filenames
	
	if (strcmp(cFullPath, "\\") == 0)
		strcpy(cWildCard, "\\*.*");
	else
	{
		strcpy(cWildCard, cFullPath);
		strcat(cWildCard, "\\*.*");
	}
	
	strcat(cFullPath, "\\");
	
	hFileFind = FindFirstFile(cWildCard, &finddata);
	
	if (hFileFind == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, "Could not perform scan", cFullPath, MB_ICONERROR);
		return;
	}
	
	do
	{
		char pcFullPath[MAX_PATH];
		
		// Skip dots
		
		if (finddata.cFileName[0] == '.')
			continue;
			
		strcpy(pcFullPath, cFullPath);
		
		strcat(pcFullPath, finddata.cFileName);
		
		// Add to linked list
		if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// Add to dirs list
			CPs_FilenameLLItem* pNewItem = (CPs_FilenameLLItem*)malloc(sizeof(CPs_FilenameLLItem));
			pNewItem->m_pNextItem = m_pFirstDir;
			STR_AllocSetString(&pNewItem->m_pcFilename, pcFullPath, FALSE);
			m_pFirstDir = pNewItem;
		}
		
		else
		{
			// Add to files list
			CPs_FilenameLLItem* pNewItem = (CPs_FilenameLLItem*)malloc(sizeof(CPs_FilenameLLItem));
			pNewItem->m_pNextItem = m_pFirstFile;
			STR_AllocSetString(&pNewItem->m_pcFilename, pcFullPath, FALSE);
			m_pFirstFile = pNewItem;
		}
		
	} while (FindNextFile(hFileFind, &finddata) != 0);
	
	FindClose(hFileFind);
	
	SortLList(m_pFirstDir);
	
	SortLList(m_pFirstFile);
	
	// Add files first - then directories
	for (pCursor = m_pFirstFile; pCursor; pCursor = (CPs_FilenameLLItem*)pCursor->m_pNextItem)
		CPL_AddFile(globals.m_hPlaylist, pCursor->m_pcFilename);
		
	for (pCursor = m_pFirstDir; pCursor; pCursor = (CPs_FilenameLLItem*)pCursor->m_pNextItem)
		CPL_AddDirectory_Recurse(hPlaylist, pCursor->m_pcFilename);
		
	// Cleanup
	for (pCursor = m_pFirstFile; pCursor; pCursor = pNextItem)
	{
		pNextItem = (CPs_FilenameLLItem*)pCursor->m_pNextItem;
		free(pCursor->m_pcFilename);
		free(pCursor);
	}
	
	for (pCursor = m_pFirstDir; pCursor; pCursor = pNextItem)
	{
		pNextItem = (CPs_FilenameLLItem*)pCursor->m_pNextItem;
		free(pCursor->m_pcFilename);
		free(pCursor);
	}
}

//
//
//
void CPL_AddDroppedFiles(CP_HPLAYLIST hPlaylist, HDROP hDrop)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	int iNumFiles, iFileIDX;
	char** ppFiles;
	
	CP_CHECKOBJECT(pPlaylist);
	
	// Read all the files into an array of strings
	iNumFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	ppFiles = (char**)malloc(iNumFiles * sizeof(char*));
	
	for (iFileIDX = 0; iFileIDX < iNumFiles; iFileIDX++)
	{
		const int iBufferSize = DragQueryFile(hDrop, iFileIDX, NULL, 0) + 1;
		ppFiles[iFileIDX] = (char*)malloc(iBufferSize * sizeof(char));
		DragQueryFile(hDrop, iFileIDX, ppFiles[iFileIDX], iBufferSize);
	}
	
	DragFinish(hDrop);
	
	// Sort filelist
	qsort(ppFiles, iNumFiles, sizeof(char*), exp_CompareStrings);
	
	// Add to playlist
	CLV_BeginBatch(globals.m_hPlaylistViewControl);
	
	for (iFileIDX = 0; iFileIDX < iNumFiles; iFileIDX++)
	{
		if (path_is_directory(ppFiles[iFileIDX]) == TRUE)
		{
			CPL_AddDirectory_Recurse(globals.m_hPlaylist, ppFiles[iFileIDX]);
			strcpy(options.last_used_directory, ppFiles[iFileIDX]);
		}
		
		else
			CPL_AddFile(globals.m_hPlaylist, ppFiles[iFileIDX]);
	}
	
	CLV_EndBatch(globals.m_hPlaylistViewControl);
	
	// Free string array
	
	for (iFileIDX = 0; iFileIDX < iNumFiles; iFileIDX++)
		free(ppFiles[iFileIDX]);
		
	free(ppFiles);
	
	// Shuffle playlist
	if (options.shuffle_play)
		PostThreadMessage(pPlaylist->m_dwWorkerThreadID, CPPLWT_SYNCSHUFFLE, 0, 0);
}

//
//
//
void CPL_Stack_Append(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hItem)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	int iItemNumber;
	CP_CHECKOBJECT(pPlaylist);
	
	// Ensure buffer is big enough
	
	if ((pPlaylist->m_iTrackStackSize + 1) >= pPlaylist->m_iTrackStackBufferSize)
	{
		pPlaylist->m_iTrackStackBufferSize += CPC_TRACKSTACK_BUFFER_QUANTISATION;
		pPlaylist->m_pTrackStack = realloc(pPlaylist->m_pTrackStack, pPlaylist->m_iTrackStackBufferSize * sizeof(CP_HPLAYLISTITEM));
	}
	
	// Ensure cursor is rational
	
	if (pPlaylist->m_iTrackStackCursor > pPlaylist->m_iTrackStackSize)
		pPlaylist->m_iTrackStackCursor = pPlaylist->m_iTrackStackSize;
		
	// Add item
	pPlaylist->m_pTrackStack[pPlaylist->m_iTrackStackSize] = hItem;
	pPlaylist->m_iTrackStackSize++;
	
	// Number item
	iItemNumber = (pPlaylist->m_iTrackStackSize - 1) - pPlaylist->m_iTrackStackCursor;
	
	CPLI_SetTrackStackPos(hItem, iItemNumber);
	CPL_cb_TrackStackChanged();
}

//
//
//
void CPL_Stack_Renumber(CP_HPLAYLIST hPlaylist)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	unsigned int iStackIDX;
	CP_CHECKOBJECT(pPlaylist);
	
	for (iStackIDX = 0; iStackIDX < pPlaylist->m_iTrackStackSize; iStackIDX++)
		CPLI_SetTrackStackPos(pPlaylist->m_pTrackStack[iStackIDX], iStackIDX - pPlaylist->m_iTrackStackCursor);
		
	CPL_cb_TrackStackChanged();
}

//
//
//
void CPL_Stack_Remove(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hItem)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	unsigned int iStackIDX;
	BOOL bFoundItem;
	CP_CHECKOBJECT(pPlaylist);
	
	CPLI_SetTrackStackPos(hItem, CIC_TRACKSTACK_UNSTACKED);
	
	// Search stack for item - remove it if we find it and renumber all items from there onwards
	bFoundItem = FALSE;
	
	for (iStackIDX = 0; iStackIDX < pPlaylist->m_iTrackStackSize; iStackIDX++)
	{
		if (bFoundItem == FALSE && pPlaylist->m_pTrackStack[iStackIDX] == hItem)
		{
			pPlaylist->m_iTrackStackSize--;
			
			if (iStackIDX < pPlaylist->m_iTrackStackCursor)
				pPlaylist->m_iTrackStackCursor--;
				
			bFoundItem = TRUE;
		}
		
		if (bFoundItem == TRUE && iStackIDX < pPlaylist->m_iTrackStackSize)
			pPlaylist->m_pTrackStack[iStackIDX] = pPlaylist->m_pTrackStack[iStackIDX+1];
	}
	
	CPL_Stack_Renumber(hPlaylist);
	
	if (bFoundItem == TRUE)
	{
		// If the trackstack is now empty - free the buffers
		if (pPlaylist->m_iTrackStackSize == 0)
		{
			pPlaylist->m_iTrackStackBufferSize = 0;
			free(pPlaylist->m_pTrackStack);
			pPlaylist->m_pTrackStack = NULL;
		}
		
		CPL_cb_TrackStackChanged();
	}
}

//
//
//
void CPL_Stack_SetCursor(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hItem)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	unsigned int iStackIDX;
	CP_CHECKOBJECT(pPlaylist);
	
	// Search for the item and set the cursor to this item - if it isn't in the stack then
	// add this item to the end (and move the cursor there)
	
	for (iStackIDX = 0; iStackIDX < pPlaylist->m_iTrackStackSize; iStackIDX++)
	{
		if (pPlaylist->m_pTrackStack[iStackIDX] == hItem)
		{
			pPlaylist->m_iTrackStackCursor = iStackIDX;
			break;
		}
	}
	
	CPL_Stack_Renumber(hPlaylist);
}

//
//
//
void CPL_Stack_Clear(CP_HPLAYLIST hPlaylist)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	unsigned int iStackIDX;
	CP_CHECKOBJECT(pPlaylist);
	
	// Mark all items in stack as unstacked
	
	for (iStackIDX = 0; iStackIDX < pPlaylist->m_iTrackStackSize; iStackIDX++)
		CPLI_SetTrackStackPos(pPlaylist->m_pTrackStack[iStackIDX], CIC_TRACKSTACK_UNSTACKED);
		
	// Clear the stack buffer
	pPlaylist->m_iTrackStackSize = 0;
	pPlaylist->m_iTrackStackBufferSize = 0;
	pPlaylist->m_iTrackStackCursor = 0;
	
	free(pPlaylist->m_pTrackStack);
	
	pPlaylist->m_pTrackStack = NULL;
	
	CPL_cb_TrackStackChanged();
}

//
//
//
void CPL_Stack_RestackAll(CP_HPLAYLIST hPlaylist)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	CP_HPLAYLISTITEM hCursor;
	CP_CHECKOBJECT(pPlaylist);
	
	pPlaylist->m_iTrackStackSize = 0;
	pPlaylist->m_iTrackStackCursor = 0;
	
	for (hCursor = pPlaylist->m_hFirst; hCursor; hCursor = CPLI_Next(hCursor))
		CPL_Stack_Append(hPlaylist, hCursor);
		
	if (pPlaylist->m_hCurrent)
		CPL_Stack_SetCursor(hPlaylist, pPlaylist->m_hCurrent);
}

//
//
//
CPe_ItemStackState CPL_Stack_GetItemState(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hItem)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	unsigned int iStackIDX;
	CP_CHECKOBJECT(pPlaylist);
	
	// Mark all items in stack as unstacked
	
	for (iStackIDX = 0; iStackIDX < pPlaylist->m_iTrackStackSize; iStackIDX++)
	{
		if (pPlaylist->m_pTrackStack[iStackIDX] == hItem)
		{
			if (iStackIDX < pPlaylist->m_iTrackStackCursor)
				return issPlayed;
			else if (iStackIDX == pPlaylist->m_iTrackStackCursor)
				return issStacked_Top;
			else
				return issStacked;
		}
	}
	
	return issUnstacked;
}

//
//
//
void CPL_Stack_Shuffle(CP_HPLAYLIST hPlaylist, const BOOL bForceCurrentToHead)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	unsigned int iStackIDX;
	CP_CHECKOBJECT(pPlaylist);
	
	if (pPlaylist->m_iTrackStackSize == 0)
		return;
		
	// Qsort stack
	qsort(pPlaylist->m_pTrackStack, pPlaylist->m_iTrackStackSize, sizeof(CP_HPLAYLISTITEM*), cpl_sort_Random);
	
	pPlaylist->m_iTrackStackCursor = 0;
	
	if (pPlaylist->m_hCurrent)
	{
		// If there is a playing track - ensure that it is at position 0
		if (bForceCurrentToHead == TRUE)
		{
			for (iStackIDX = 0; iStackIDX < pPlaylist->m_iTrackStackSize; iStackIDX++)
			{
				if (pPlaylist->m_pTrackStack[iStackIDX] == pPlaylist->m_hCurrent)
				{
					pPlaylist->m_pTrackStack[iStackIDX] = pPlaylist->m_pTrackStack[0];
					pPlaylist->m_pTrackStack[0] = pPlaylist->m_hCurrent;
					break;
				}
			}
		}
		
		else
		{
			// If the current song is still at the head - swap it with the last song (to prevent a double play)
			if (pPlaylist->m_pTrackStack[0] == pPlaylist->m_hCurrent)
			{
				pPlaylist->m_pTrackStack[0] = pPlaylist->m_pTrackStack[pPlaylist->m_iTrackStackSize-1];
				pPlaylist->m_pTrackStack[pPlaylist->m_iTrackStackSize-1] = pPlaylist->m_hCurrent;
			}
		}
	}
	
	CPL_Stack_Renumber(hPlaylist);
}

//
//
//
void CPL_Stack_ClipFromCurrent(CP_HPLAYLIST hPlaylist)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	unsigned int iStackIDX;
	CP_CHECKOBJECT(pPlaylist);
	
	if (pPlaylist->m_iTrackStackSize == 0 || pPlaylist->m_iTrackStackCursor >= pPlaylist->m_iTrackStackSize)
		return;
		
	// Mark all items in stack as unstacked
	for (iStackIDX = pPlaylist->m_iTrackStackCursor + 1; iStackIDX < pPlaylist->m_iTrackStackSize; iStackIDX++)
		CPLI_SetTrackStackPos(pPlaylist->m_pTrackStack[iStackIDX], CIC_TRACKSTACK_UNSTACKED);
		
	pPlaylist->m_iTrackStackSize = pPlaylist->m_iTrackStackCursor + 1;
	
	CPL_cb_TrackStackChanged();
}

//
//
//
void CPL_Stack_ClipFromItem(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hItem)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	unsigned int iStackIDX;
	BOOL bItemFound;
	unsigned int iFoundItemIDX;
	CP_CHECKOBJECT(pPlaylist);
	
	if (pPlaylist->m_iTrackStackSize == 0)
		return;
		
	// Mark items in stack after item as unstacked
	bItemFound = FALSE;
	
	iFoundItemIDX = CPC_INVALIDITEM;
	
	for (iStackIDX = pPlaylist->m_iTrackStackCursor; iStackIDX < pPlaylist->m_iTrackStackSize; iStackIDX++)
	{
		if (bItemFound == FALSE && pPlaylist->m_pTrackStack[iStackIDX] == hItem)
		{
			bItemFound = TRUE;
			iFoundItemIDX = iStackIDX;
		}
		
		else if (bItemFound)
			CPLI_SetTrackStackPos(pPlaylist->m_pTrackStack[iStackIDX], CIC_TRACKSTACK_UNSTACKED);
	}
	
	if (bItemFound)
	{
		pPlaylist->m_iTrackStackSize = iFoundItemIDX + 1;
		CPL_cb_TrackStackChanged();
	}
}

//
//
//
void CPL_Stack_PlayNext(CP_HPLAYLIST hPlaylist, CP_HPLAYLISTITEM hItem)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	unsigned int iStackIDX;
	
	CP_CHECKOBJECT(pPlaylist);
	
	CPL_Stack_Remove(hPlaylist, hItem);
	
	// Rationalise cursor
	
	if (pPlaylist->m_iTrackStackCursor >= pPlaylist->m_iTrackStackSize)
	{
		CPL_Stack_Append(hPlaylist, hItem);
		return;
	}
	
	// Simple case
	
	if (pPlaylist->m_iTrackStackBufferSize == 0)
	{
		CPL_Stack_Append(hPlaylist, hItem);
		return;
	}
	
	
	// Ensure buffer is big enough
	
	if ((pPlaylist->m_iTrackStackSize + 1) >= pPlaylist->m_iTrackStackBufferSize)
	{
		pPlaylist->m_iTrackStackBufferSize += CPC_TRACKSTACK_BUFFER_QUANTISATION;
		pPlaylist->m_pTrackStack = realloc(pPlaylist->m_pTrackStack, pPlaylist->m_iTrackStackBufferSize * sizeof(CP_HPLAYLISTITEM));
	}
	
	// Shunt all items up one
	
	for (iStackIDX = pPlaylist->m_iTrackStackSize; iStackIDX > (pPlaylist->m_iTrackStackCursor + 1); iStackIDX--)
	{
		pPlaylist->m_pTrackStack[iStackIDX] = pPlaylist->m_pTrackStack[iStackIDX-1];
		CPLI_SetTrackStackPos(pPlaylist->m_pTrackStack[iStackIDX], iStackIDX - pPlaylist->m_iTrackStackCursor);
		
	}
	
	// Add item
	pPlaylist->m_pTrackStack[pPlaylist->m_iTrackStackCursor+1] = hItem;
	CPLI_SetTrackStackPos(pPlaylist->m_pTrackStack[iStackIDX], 1);
	
	pPlaylist->m_iTrackStackSize++;
	CPL_cb_TrackStackChanged();
}

//
//
//
DWORD WINAPI CPI_PlaylistWorkerThreadEP(void* pCookie)
{
	CPs_PlaylistWorkerThreadInfo* pThreadInfo = (CPs_PlaylistWorkerThreadInfo*)pCookie;
	MSG msg;
	CPs_NotifyChunk* pPendingChunk;
	BOOL bRet;
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
	
	pPendingChunk = NULL;
	
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			// handle the error and possibly exit
			return 0;
		}
		
		if (msg.message == CPPLWT_TERMINATE)
		{
			break;
		}
		
		else if (msg.message == CPPLWT_READTAG)
		{
			MSG msgPeek;
			CP_HPLAYLISTITEM hNewFile = (CP_HPLAYLISTITEM)msg.lParam;
			
			if (pThreadInfo->m_dwCurrentBatchID == (DWORD)msg.wParam)
			{
				CPLI_ReadTag(hNewFile);
				
				// If we didn't get a track length from the tag - work it out
				
				if (CPLI_GetTrackLength(hNewFile) == 0
						&& options.work_out_track_lengths)
				{
					CPLI_CalculateLength(hNewFile);
				}
				
				// Allocate chunk
				
				if (!pPendingChunk)
				{
					pPendingChunk = (CPs_NotifyChunk*)malloc(sizeof(CPs_NotifyChunk));
					pPendingChunk->m_iNumberInChunk = 0;
				}
				
				// Add to current chunk
				pPendingChunk->m_aryItems[pPendingChunk->m_iNumberInChunk] = hNewFile;
				pPendingChunk->m_aryBatchIDs[pPendingChunk->m_iNumberInChunk] = (DWORD)msg.wParam;
				pPendingChunk->m_iNumberInChunk++;
			}
			
			else
				CPLII_DestroyItem(hNewFile);
				
			// Send the current chunk if its full or there are no more pending readtag messages
			if (pPendingChunk)
			{
				if (pPendingChunk->m_iNumberInChunk == CPC_PLAYLISTWORKER_NOTIFYCHUNKSIZE
						|| PeekMessage(&msgPeek, NULL, CPPLWT_READTAG, CPPLWT_READTAG, PM_NOREMOVE) == FALSE)
				{
					PostThreadMessage(pThreadInfo->m_dwHostThreadID, CPPLNM_TAGREAD, (WPARAM)pPendingChunk, 0L);
					pPendingChunk = NULL;
				}
			}
		}
		
		else if (msg.message == CPPLWT_SYNCSHUFFLE)
		{
			PostThreadMessage(pThreadInfo->m_dwHostThreadID, CPPLNM_SYNCSHUFFLE, 0L, 0L);
		}
		
		else if (msg.message == CPPLWT_SETACTIVE)
		{
			CP_HPLAYLISTITEM hFile = (CP_HPLAYLISTITEM)msg.lParam;
			
			// Send the current chunk if there is one
			
			if (pPendingChunk)
			{
				PostThreadMessage(pThreadInfo->m_dwHostThreadID, CPPLNM_TAGREAD, (WPARAM)pPendingChunk, 0L);
				pPendingChunk = NULL;
			}
			
			// Send the setactivate
			
			if (pThreadInfo->m_dwCurrentBatchID == (DWORD)msg.wParam)
				PostThreadMessage(pThreadInfo->m_dwHostThreadID, CPPLNM_SYNCSETACTIVE, (WPARAM)hFile, 0L);
		}
	}
	
	// Clean up any pending chunk
	
	if (pPendingChunk)
	{
		int iChunkItemIDX;
		
		for (iChunkItemIDX = 0; iChunkItemIDX < pPendingChunk->m_iNumberInChunk; iChunkItemIDX++)
			CPLII_DestroyItem(pPendingChunk->m_aryItems[iChunkItemIDX]);
			
		free(pPendingChunk);
	}
	
	
	CP_TRACE0("Playlist worker thread terminating");
	
	return 0;
}

//
//
//
void CPL_SyncLoadNextFile(CP_HPLAYLIST hPlaylist)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	CP_CHECKOBJECT(pPlaylist);
	
	pPlaylist->m_bSyncLoadNextFile = TRUE;
}

//
//
//
void CPL_SetAutoActivateInitial(CP_HPLAYLIST hPlaylist, const BOOL bAutoActivateInitial)
{
	CPs_Playlist* pPlaylist = (CPs_Playlist*)hPlaylist;
	CP_CHECKOBJECT(pPlaylist);
	
	pPlaylist->m_bAutoActivateInitial = TRUE;
}

//
//
//
