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
#include "DLG_Find.h"
#include "CPI_Playlist.h"
#include "CPI_PlaylistItem.h"


////////////////////////////////////////////////////////////////////////////////
//
void Search_SelectItems();
//
////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////
//
//
//
BOOL CALLBACK wp_FindDialog(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	
		case WM_INITDIALOG:
		
			// Setup type initial state
		{
			UINT uiDlgItemToSet;
			
			if (options.m_enQuickFindTerm == qftTitle)
				uiDlgItemToSet = IDC_QFND_TITLES;
			else if (options.m_enQuickFindTerm == qftArtist)
				uiDlgItemToSet = IDC_QFND_ARTISTS;
			else
				uiDlgItemToSet = IDC_QFND_ALBUMS;
				
			SendDlgItemMessage(hWnd, uiDlgItemToSet, BM_SETCHECK, (WPARAM)BST_CHECKED, 0L);
		
			// Setup position
			
			if (globals.m_bQuickFindWindowPos_Valid == TRUE)
			{
				SetWindowPos(hWnd, NULL,
							 globals.m_ptQuickFindWindowPos.x, globals.m_ptQuickFindWindowPos.y,
							 0, 0,
							 SWP_NOSIZE | SWP_NOZORDER);
			}
			
			return TRUE;
		} // end WM_INITDIALOG


		case WM_CLOSE:
			DestroyWindow(hWnd);
			windows.m_hWndFindDialog = NULL;
			SetFocus(CLV_GetHWND(globals.m_hPlaylistViewControl));
			break;
			
		case WM_COMMAND:
		
			switch (LOWORD(wParam))
			{
			
				case IDOK:	// FALLTHROUGH
				case IDCANCEL:
					SendMessage(hWnd, WM_CLOSE, 0L, 0L);
					break;
					
				case IDC_FND_TEXT:
				
					if (HIWORD(wParam) == EN_CHANGE)
						Search_SelectItems();
						
					break;
					
				case IDC_QFND_TITLES:	// FALLTHROUGH
				case IDC_QFND_ARTISTS:
				case IDC_QFND_ALBUMS:
					if (HIWORD(wParam) == BN_CLICKED)
					{
						// Set the search defaults
						if (LOWORD(wParam) == IDC_QFND_TITLES)
							options.m_enQuickFindTerm = qftTitle;
						else if (LOWORD(wParam) == IDC_QFND_ARTISTS)
							options.m_enQuickFindTerm = qftArtist;
						else
							options.m_enQuickFindTerm = qftAlbum;
							
						// Research
						Search_SelectItems();
					}
					
					break;
			}
			
			break;
			
		case WM_WINDOWPOSCHANGED:
			// Store the window pos for next time
		{
			const WINDOWPOS* pWinPos = (const WINDOWPOS*)lParam;
			
			globals.m_bQuickFindWindowPos_Valid = TRUE;
			globals.m_ptQuickFindWindowPos.x = pWinPos->x;
			globals.m_ptQuickFindWindowPos.y = pWinPos->y;
		}
		
		break;
	}
	
	return FALSE;
}

//
//
//
char* CP_stristr(const char *pcString1, const char *pcString2)
{
	char *pCompareStart = (char *)pcString1;
	char *pCursor_S1, *pCursor_S2;
	char cSrc, cDst;
	
	// If there is a null source string - this is a "no match"
	
	if (!pcString1)
		return NULL;
		
	// Null length string 2 - this is a "no match"
	if (!*pcString2)
		return NULL;
		
	// Search from every start pos in the source string
	while (*pCompareStart)
	{
		pCursor_S1 = pCompareStart;
		pCursor_S2 = (char *)pcString2;
		
		// Scan both string
		
		while (*pCursor_S1 && *pCursor_S2)
		{
			cSrc = *pCursor_S1;
			cDst = *pCursor_S2;
			
			// Correct case
			
			if ((cSrc >= 'A') && (cSrc <= 'Z'))
				cSrc -= ('A' - 'a');
				
			if ((cDst >= 'A') && (cDst <= 'Z'))
				cDst -= ('A' - 'a');
				
			if (cSrc != cDst)
				break;
				
			pCursor_S1++;
			pCursor_S2++;
		}
		
		// If string 2 is exhausted - there is a match
		
		if (!*pCursor_S2)
			return(pCompareStart);
			
		// Offset source and continue
		pCompareStart++;
	}
	
	return NULL;
}

//
//
//
void Search_SelectItems()
{
	CP_HPLAYLISTITEM hCursor;
	BOOL bFirstMatch = TRUE;
	char* pcTerm;
	unsigned int iEditTextLength;
	BOOL bTitles, bAlbums, bArtists;
	
	// Get search term
	iEditTextLength = SendDlgItemMessage(windows.m_hWndFindDialog, IDC_FND_TEXT, WM_GETTEXTLENGTH, 0L, 0L);
	pcTerm = (char*)malloc(iEditTextLength + 1);
	SendDlgItemMessage(windows.m_hWndFindDialog, IDC_FND_TEXT, WM_GETTEXT, (WPARAM)(iEditTextLength + 1), (LPARAM)pcTerm);
	
	// Get search filter
	bTitles = SendDlgItemMessage(windows.m_hWndFindDialog, IDC_QFND_TITLES, BM_GETCHECK, 0L, 0L) == BST_CHECKED ? TRUE : FALSE;
	bArtists = SendDlgItemMessage(windows.m_hWndFindDialog, IDC_QFND_ARTISTS, BM_GETCHECK, 0L, 0L) == BST_CHECKED ? TRUE : FALSE;
	bAlbums = SendDlgItemMessage(windows.m_hWndFindDialog, IDC_QFND_ALBUMS, BM_GETCHECK, 0L, 0L) == BST_CHECKED ? TRUE : FALSE;
	
	// Perform select/search
	
	for (hCursor = CPL_GetFirstItem(globals.m_hPlaylist); hCursor; hCursor = CPLI_Next(hCursor))
	{
		const int iItemIDX = CPLI_GetCookie(hCursor);
		const char* pcMatchTerm;
		BOOL bSelect = FALSE;
		
		// Perform searches
		
		if (bTitles)
			pcMatchTerm = CPLI_GetTrackName(hCursor);
		else if (bArtists)
			pcMatchTerm = CPLI_GetArtist(hCursor);
		else
			pcMatchTerm = CPLI_GetAlbum(hCursor);
			
		if (CP_stristr(pcMatchTerm, pcTerm))
			bSelect = TRUE;
			
		// If this is the first match - focus (and scroll on) the item
		if (bSelect && bFirstMatch == TRUE)
		{
			bFirstMatch = FALSE;
			CLV_SetFocusItem(globals.m_hPlaylistViewControl, iItemIDX);
			CLV_EnsureVisible(globals.m_hPlaylistViewControl, iItemIDX);
		}
		
		// Update selected state
		CLV_SetItemSelected(globals.m_hPlaylistViewControl, iItemIDX, bSelect);
	}
	
	// Cleanup
	free(pcTerm);
}

//
//
//
