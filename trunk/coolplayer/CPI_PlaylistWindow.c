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
#include "WindowsOS.h"
#include "CPI_Player.h"
#include "CPI_ID3.h"
#include "CPI_Playlist.h"
#include "CPI_PlaylistItem.h"
#include "CPI_PlaylistWindow.h"
#include "CPI_Indicators.h"


////////////////////////////////////////////////////////////////////////////////
//
typedef enum _CIe_PlaylistWindowMode
{
	wmQuiescent,
	wmListItem_Drag,
	wmCommandTarget_Click
	
} CIe_PlaylistWindowMode;
//
//
//

struct _CPs_PlaylistWindowState
{
	CIe_PlaylistWindowMode m_enWindowMode;
	CPs_CommandTarget* m_pActiveCommandTarget;
	CPs_CommandTarget* m_pFloatActiveCommandTarget;
	BOOL m_bMouseEventSet;
	
} glb_PLW_State;

//
////////////////////////////////////////////////////////////////////////////////





#define IDC_PL_LISTVIEW   0x0100
#define IDC_PL_FLOATINGEDIT  0x0101
#define IDC_PL_FLOATINGCOMBO 0x0102
#define CPPLM_CREATEINPLACE   (WM_APP+0x001)
#define CPPLM_DESTROYINPLACE  (WM_APP+0x002)
#define CPC_SIZEBORDER   0x4
//
#define IDC_CMDTS_CLEARSTACK  0x1
#define IDC_CMDTS_RESTACKALL  0x2
#define IDC_CMDTS_PLAYFROMHERE  0x3
#define IDC_CMDTS_UNSTACK   0x4
#define IDC_CMDTS_PLAYNOW   0x5
#define IDC_CMDTS_STOPAFTER   0x6
#define IDC_CMDTS_STOPAFTER_NOREP 0x7
#define IDC_CMDTS_PLAYNEXT   0x8
#define IDC_CMDTS_QUEUE    0x9
//
void LVCB_DrawBackgroundRect(CPs_DrawContext* pDC);
void LVCB_HeaderChanged(CP_HLISTVIEW _hListData);
void LVCB_ItemSelected(CP_HLISTVIEW _hListData, const int iItemIDX, const CP_HPLAYLISTITEM hItem);
void LVCB_ItemAction(CP_HLISTVIEW _hListData, const int iItemIDX, const CP_HPLAYLISTITEM hItem);
void LVCB_ItemDrag(CP_HLISTVIEW _hListData, const int iItemIDX, const CP_HPLAYLISTITEM hItem);
void LVCB_ItemRightClick(CP_HLISTVIEW _hListData, const int iItemIDX, const int iColumnIDX, const CP_HPLAYLISTITEM hItem);
void LVCB_ColHeaderClick(CP_HLISTVIEW _hListData, const int iColIDX);
void LVCB_UnhandledKeyPress(CP_HLISTVIEW _hListData, const int iVKey, const BOOL bAlt, const BOOL bCtrl, const BOOL bShift);
CPe_CustomDrawColour LVCB_GetTrackStackItemColour(const void* pvItemData);
CPe_CustomDrawColour LVCB_GetItemColour(const void* pvItemData);

void CPlaylistWindow_CB_onCreate(CP_HINTERFACE hInterface, const RECT* pInitialPosition);
void CPlaylistWindow_CB_onDraw(CP_HINTERFACE hInterface, CPs_DrawContext* pContext);
void CPlaylistWindow_CB_onKeyDown(CP_HINTERFACE hInterface, const unsigned int iVKeyCode, const BOOL bAlt, const BOOL bCtrl, const BOOL bShift);
void CPlaylistWindow_CB_onDropFiles(CP_HINTERFACE hInterface, HDROP hDrop);
void CPlaylistWindow_CB_onPosChange(CP_HINTERFACE hInterface, const RECT* pNewPosition, const BOOL bSizeChanged);
void CPlaylistWindow_CB_onFocus(CP_HINTERFACE hInterface, const BOOL bHasFocus);
void CPlaylistWindow_CB_onCommandMessage(CP_HINTERFACE hInterface, const WPARAM wParam, const LPARAM lParam);

void CPlaylistWindow_CB_onMouseMove(CP_HINTERFACE hInterface, const POINTS ptMouse, const unsigned short iFlags);
void CPlaylistWindow_CB_onMouseButton_LUp(CP_HINTERFACE hInterface, const POINTS ptMouse, const unsigned short iFlags);
LRESULT CPlaylistWindow_CB_onAppMessage(CP_HINTERFACE hInterface, const UINT uiMessage, const WPARAM wParam, const LPARAM lParam);
void CPlaylistWindow_CB_onClose(CP_HINTERFACE hInterface);
//
void CPlaylistWindow_CreateSubparts();
////////////////////////////////////////////////////////////////////////////////
//
//
//
void CPlaylistWindow_Create()
{
	// Init playlist window state
	glb_PLW_State.m_enWindowMode = wmQuiescent;
	glb_PLW_State.m_pActiveCommandTarget = NULL;
	glb_PLW_State.m_pFloatActiveCommandTarget = NULL;
	glb_PLW_State.m_bMouseEventSet = FALSE;
	
	// Create user interface window
	windows.m_hifPlaylist = IF_Create();
	IF_sethandler_onCreate(windows.m_hifPlaylist, CPlaylistWindow_CB_onCreate);
	IF_sethandler_onDraw(windows.m_hifPlaylist, CPlaylistWindow_CB_onDraw);
	IF_sethandler_onKeyDown(windows.m_hifPlaylist, CPlaylistWindow_CB_onKeyDown);
	IF_sethandler_onDropFiles(windows.m_hifPlaylist, CPlaylistWindow_CB_onDropFiles);
	IF_sethandler_onPosChange(windows.m_hifPlaylist, CPlaylistWindow_CB_onPosChange);
	IF_sethandler_onFocus(windows.m_hifPlaylist, CPlaylistWindow_CB_onFocus);
	IF_sethandler_onCommandMessage(windows.m_hifPlaylist, CPlaylistWindow_CB_onCommandMessage);
	IF_sethandler_onAppMessage(windows.m_hifPlaylist, CPlaylistWindow_CB_onAppMessage);
	IF_sethandler_onClose(windows.m_hifPlaylist, CPlaylistWindow_CB_onClose);
	
	// Add interface subparts
	CPlaylistWindow_CreateSubparts();
	
	IF_SetMinSize(windows.m_hifPlaylist, &glb_pSkin->mpl_szMinWindow);
	IF_OpenWindow(windows.m_hifPlaylist, "CoolPlayer Playlist", &options.playlist_window_pos, CPC_INTERFACE_STYLE_RESIZING);
	IF_SetVisible(windows.m_hifPlaylist, options.show_playlist);
}

//
//
//
void CPlaylistWindow_Destroy()
{
	// Cleanup windows
	IF_CloseWindow(windows.m_hifPlaylist);
	globals.m_hPlaylistViewControl = NULL;
}

//
//
//
void CPlaylistWindow_CreateSubparts()
{

	IF_RemoveAllSubparts(windows.m_hifPlaylist);
	
	{
		CPs_CommandTarget* pCT_Cursor;
		
		for (pCT_Cursor = glb_pSkin->mpl_pCommandTargets; pCT_Cursor; pCT_Cursor = (CPs_CommandTarget*)pCT_Cursor->m_pNext)
		{
			IF_AddSubPart_CommandButton(windows.m_hifPlaylist,
										pCT_Cursor->m_dwAlign,
										&pCT_Cursor->m_ptOffset,
										pCT_Cursor->m_pStateImage,
										pCT_Cursor->m_pfnVerb);
		}
	}
	
	{
		CPs_Indicator* pI_Cursor;
		
		for (pI_Cursor = glb_pSkin->mpl_pIndicators; pI_Cursor; pI_Cursor = (CPs_Indicator*)pI_Cursor->m_pNext)
		{
			IF_AddSubPart_Indicator(windows.m_hifPlaylist,
									pI_Cursor->m_pcName,
									pI_Cursor->m_dwAlign,
									&pI_Cursor->m_rAlign);
		}
	}
}

//
//
//
void CPlaylistWindow_SetVisible(const BOOL bNewVisibleState)
{
	CheckMenuItem(globals.main_menu_popup, MENU_PLAYLIST, MF_BYCOMMAND | (bNewVisibleState ? MF_CHECKED : 0));
	IF_SetVisible(windows.m_hifPlaylist, bNewVisibleState);
}

//
//
//
void CPlaylistWindow_CreateListView()
{
	RECT rClient;
	int iColumnIDX;
	
	// Create listview control
	GetClientRect(IF_GetHWnd(windows.m_hifPlaylist), &rClient);
	globals.m_hPlaylistViewControl = CLV_Create(IF_GetHWnd(windows.m_hifPlaylist),
									 glb_pSkin->mpl_rList_Border.left, glb_pSkin->mpl_rList_Border.top,
									 (rClient.right - glb_pSkin->mpl_rList_Border.right) - glb_pSkin->mpl_rList_Border.left,
									 (rClient.bottom - glb_pSkin->mpl_rList_Border.bottom) - glb_pSkin->mpl_rList_Border.top);
	                                 
	// Setup callbacks
	CLV_sethandler_DrawBackgroundRect(globals.m_hPlaylistViewControl, LVCB_DrawBackgroundRect);
	CLV_sethandler_HeaderChanged(globals.m_hPlaylistViewControl, LVCB_HeaderChanged);
	CLV_sethandler_ItemSelected(globals.m_hPlaylistViewControl, (wp_ItemCallback)LVCB_ItemSelected);
	CLV_sethandler_ItemAction(globals.m_hPlaylistViewControl, (wp_ItemCallback)LVCB_ItemAction);
	CLV_sethandler_ItemDrag(globals.m_hPlaylistViewControl, (wp_ItemCallback)LVCB_ItemDrag);
	CLV_sethandler_ItemRightClick(globals.m_hPlaylistViewControl, (wp_ItemSubCallback)LVCB_ItemRightClick);
	CLV_sethandler_ColHeaderClick(globals.m_hPlaylistViewControl, LVCB_ColHeaderClick);
	CLV_sethandler_UnhandledKeyPress(globals.m_hPlaylistViewControl, LVCB_UnhandledKeyPress);
	
	// Setup columns
	CLV_AddColumn(globals.m_hPlaylistViewControl,
				  "Stack",
				  options.playlist_column_widths[0],
				  (wp_GetItemText)CPLI_GetTrackStackPos_AsText,
				  CPLV_COLFLAG_NOHIDE);
	CLV_SetColumnCustomDrawColour(globals.m_hPlaylistViewControl, PLAYLIST_TRACKSTACK, LVCB_GetTrackStackItemColour);
	CLV_SetColumnAlign(globals.m_hPlaylistViewControl, PLAYLIST_TRACKSTACK, lcaRight);
	CLV_AddColumn(globals.m_hPlaylistViewControl,
				  "Title",
				  options.playlist_column_widths[1],
				  (wp_GetItemText)CPLI_GetTrackName,
				  options.playlist_column_visible[1] ? CPLV_COLFLAG_NONE : CPLV_COLFLAG_HIDDEN);
	CLV_AddColumn(globals.m_hPlaylistViewControl,
				  "Artist",
				  options.playlist_column_widths[2],
				  (wp_GetItemText)CPLI_GetArtist,
				  options.playlist_column_visible[2] ? CPLV_COLFLAG_NONE : CPLV_COLFLAG_HIDDEN);
	CLV_AddColumn(globals.m_hPlaylistViewControl,
				  "Album",
				  options.playlist_column_widths[3],
				  (wp_GetItemText)CPLI_GetAlbum,
				  options.playlist_column_visible[3] ? CPLV_COLFLAG_NONE : CPLV_COLFLAG_HIDDEN);
	CLV_AddColumn(globals.m_hPlaylistViewControl,
				  "Year",
				  options.playlist_column_widths[4],
				  (wp_GetItemText)CPLI_GetYear,
				  options.playlist_column_visible[4] ? CPLV_COLFLAG_NONE : CPLV_COLFLAG_HIDDEN);
	CLV_AddColumn(globals.m_hPlaylistViewControl,
				  "TrackNum",
				  options.playlist_column_widths[5],
				  (wp_GetItemText)CPLI_GetTrackNum_AsText,
				  options.playlist_column_visible[5] ? CPLV_COLFLAG_NONE : CPLV_COLFLAG_HIDDEN);
	CLV_SetColumnAlign(globals.m_hPlaylistViewControl, PLAYLIST_TRACKNUM, lcaRight);
	CLV_AddColumn(globals.m_hPlaylistViewControl,
				  "Comment",
				  options.playlist_column_widths[6],
				  (wp_GetItemText)CPLI_GetComment,
				  options.playlist_column_visible[6] ? CPLV_COLFLAG_NONE : CPLV_COLFLAG_HIDDEN);
	CLV_AddColumn(globals.m_hPlaylistViewControl,
				  "Genre",
				  options.playlist_column_widths[7],
				  (wp_GetItemText)CPLI_GetGenre,
				  options.playlist_column_visible[7] ? CPLV_COLFLAG_NONE : CPLV_COLFLAG_HIDDEN);
	CLV_AddColumn(globals.m_hPlaylistViewControl,
				  "Path",
				  options.playlist_column_widths[8],
				  (wp_GetItemText)CPLI_GetPath,
				  options.playlist_column_visible[8] ? CPLV_COLFLAG_NONE : CPLV_COLFLAG_HIDDEN);
	CLV_AddColumn(globals.m_hPlaylistViewControl,
				  "Filename",
				  options.playlist_column_widths[9],
				  (wp_GetItemText)CPLI_GetFilename,
				  options.playlist_column_visible[9] ? CPLV_COLFLAG_NONE : CPLV_COLFLAG_HIDDEN);
	CLV_AddColumn(globals.m_hPlaylistViewControl,
				  "Length",
				  options.playlist_column_widths[10],
				  (wp_GetItemText)CPLI_GetTrackLength_AsText,
				  options.playlist_column_visible[10] ? CPLV_COLFLAG_NONE : CPLV_COLFLAG_HIDDEN);
	CLV_SetColumnAlign(globals.m_hPlaylistViewControl, PLAYLIST_LENGTH, lcaRight);
	
	for (iColumnIDX = 1; iColumnIDX <= PLAYLIST_last; iColumnIDX++)
		CLV_SetColumnCustomDrawColour(globals.m_hPlaylistViewControl, iColumnIDX, LVCB_GetItemColour);
		
	CLV_SetColumnOrder(globals.m_hPlaylistViewControl, options.playlist_column_seq, PLAYLIST_last + 1);
	CPL_cb_SetWindowToReflectList();
}

//
//
//
int __cdecl exp_CompareStrings(const void *elem1, const void *elem2)
{
	const char* pcElem1 = *(const char**)elem1;
	const char* pcElem2 = *(const char**)elem2;
	return stricmp(pcElem1, pcElem2);
}

//
//
//
void CPlaylistWindow_DestroyIPEdit()
{
	HWND hWnd_IPEdit;
	
	if (!windows.wnd_playlist_IPEdit)
		return;
		
	hWnd_IPEdit = windows.wnd_playlist_IPEdit;
	
	UnhookWindowsHookEx(globals.m_hhkListView_Posted);
	
	globals.m_hhkListView_Posted = NULL;
	
	windows.wnd_playlist_IPEdit = NULL;
	
	DestroyWindow(hWnd_IPEdit);
	
	// Write any dirty playlist items - Check all items because the selection could have
	// changed by now
	{
		char cStatusMessage[1024];
		CP_HPLAYLISTITEM hCursor;
		
		SetCursor(LoadCursor(NULL, IDC_WAIT));
		
		for (hCursor = CPL_GetFirstItem(globals.m_hPlaylist); hCursor; hCursor = CPLI_Next(hCursor))
		{
			if (CPLI_IsTagDirty(hCursor) == FALSE)
				continue;
				
			sprintf(cStatusMessage, "Tagging \"%s\"", CPLI_GetFilename(hCursor));
			CPIC_SetIndicatorValue("status", cStatusMessage);
			UpdateWindow(IF_GetHWnd(windows.m_hifPlaylist));
			CPLI_WriteTag(hCursor);
		}
		
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		CPIC_SetIndicatorValue("status", NULL);
	}
}

//
//
//
LRESULT CALLBACK exp_ListViewHookProc_Posted(int iCode, WPARAM wParam, LPARAM lParam)
{
	if (iCode == HC_ACTION && windows.wnd_playlist_IPEdit)
	{
		MSG* pMSG = (MSG*)lParam;
		
		// If any window (apart from the IP window) has a mouse click - close the IP window
		
		if ((pMSG->message == WM_LBUTTONDOWN
				|| pMSG->message == WM_MBUTTONDOWN
				|| pMSG->message == WM_RBUTTONDOWN
				|| pMSG->message == WM_NCLBUTTONDOWN
				|| pMSG->message == WM_NCMBUTTONDOWN
				|| pMSG->message == WM_NCRBUTTONDOWN)
				&& pMSG->hwnd != windows.wnd_playlist_IPEdit)
		{
			// Get the classname to ensure that it's not our combo popup that's got the message
			char cClassname[64];
			GetClassName(pMSG->hwnd, cClassname, 64);
			
			if (strcmp("ComboLBox", cClassname))
				CPlaylistWindow_DestroyIPEdit();
		}
	}
	
	if (globals.m_hhkListView_Posted)
		return CallNextHookEx(globals.m_hhkListView_Posted, iCode, wParam, lParam);
	else
		return 0;
}

//
//
//
void CPlaylistWindow_TrackStackMenu(iItem)
{
	HWND hWndList;
	POINT ptItem;
	RECT rSubItem;
	HMENU hmMenu;
	UINT uiMenuCommand;
	int iSearchItemIDX;
	CP_HPLAYLISTITEM hClickedItem;
	CPe_ItemStackState enClickedItemState;
	BOOL bMultipleSelection;
	
	// We want to get the subitem's rect in the co-ordinate space of the dialog
	hWndList = CLV_GetHWND(globals.m_hPlaylistViewControl);
	CLV_GetItemSubRect(globals.m_hPlaylistViewControl, &rSubItem, iItem, 0);
	
	// Are there multiple items selected?
	iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, CPC_INVALIDITEM);
	
	if (iSearchItemIDX != CPC_INVALIDITEM)
		iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, iSearchItemIDX);
		
	if (iSearchItemIDX != CPC_INVALIDITEM)
		bMultipleSelection = TRUE;
	else
		bMultipleSelection = FALSE;
		
	ptItem.x = rSubItem.left;
	ptItem.y = rSubItem.bottom;
	ClientToScreen(hWndList, &ptItem);
	
	// Create menu
	hmMenu = CreatePopupMenu();
	
	hClickedItem = (CP_HPLAYLISTITEM)CLV_GetItemData(globals.m_hPlaylistViewControl, iItem);
	
	enClickedItemState = CPL_Stack_GetItemState(globals.m_hPlaylist, hClickedItem);
	
	if (enClickedItemState != issUnstacked)
	{
		AppendMenu(hmMenu, MF_STRING, IDC_CMDTS_PLAYFROMHERE, "Play from here");
		SetMenuDefaultItem(hmMenu, IDC_CMDTS_PLAYFROMHERE, FALSE);
		
		if (enClickedItemState != issStacked_Top)
		{
			AppendMenu(hmMenu, MF_STRING, IDC_CMDTS_PLAYNEXT, "Play next");
			AppendMenu(hmMenu, MF_STRING, IDC_CMDTS_UNSTACK, "Unstack");
		}
		
		// Not played yet?
		
		if (enClickedItemState == issStacked || enClickedItemState == issStacked_Top)
		{
			if (options.repeat_playlist == TRUE)
			{
				AppendMenu(hmMenu, MF_STRING, IDC_CMDTS_STOPAFTER, "Repeat after this");
				AppendMenu(hmMenu, MF_STRING, IDC_CMDTS_STOPAFTER_NOREP, "Stop after this (repeat->off)");
			}
			
			else
			{
				AppendMenu(hmMenu, MF_STRING, IDC_CMDTS_STOPAFTER, "Stop after this");
			}
		}
		
		if (enClickedItemState != issStacked_Top)
			AppendMenu(hmMenu, MF_STRING, IDC_CMDTS_QUEUE, "Queue at end");
	}
	
	else
	{
		AppendMenu(hmMenu, MF_STRING, IDC_CMDTS_PLAYNOW, "Play now");
		SetMenuDefaultItem(hmMenu, IDC_CMDTS_PLAYNOW, FALSE);
		
		AppendMenu(hmMenu, MF_STRING, IDC_CMDTS_PLAYNEXT, "Play next");
		AppendMenu(hmMenu, MF_STRING, IDC_CMDTS_QUEUE, "Queue at end");
	}
	
	AppendMenu(hmMenu, MF_SEPARATOR, 0, NULL);
	
	AppendMenu(hmMenu, MF_STRING, IDC_CMDTS_CLEARSTACK, "Clear stack");
	AppendMenu(hmMenu, MF_STRING, IDC_CMDTS_RESTACKALL, "Restack all");
	
	uiMenuCommand = TrackPopupMenuEx(hmMenu,
									 TPM_NONOTIFY
									 | TPM_RETURNCMD
									 | TPM_RIGHTBUTTON,
									 ptItem.x, ptItem.y,
									 IF_GetHWnd(windows.m_hifPlaylist),
									 NULL);
	DestroyMenu(hmMenu);
	
	if (uiMenuCommand == IDC_CMDTS_CLEARSTACK)
	{
		CPL_Stack_Clear(globals.m_hPlaylist);
	}
	
	else if (uiMenuCommand == IDC_CMDTS_RESTACKALL)
	{
		CPL_Stack_RestackAll(globals.m_hPlaylist);
	}
	
	else if (uiMenuCommand == IDC_CMDTS_PLAYFROMHERE)
	{
		CPL_Stack_SetCursor(globals.m_hPlaylist, hClickedItem);
		CPL_PlayItem(globals.m_hPlaylist, TRUE, pmCurrentItem);
	}
	
	else if (uiMenuCommand == IDC_CMDTS_UNSTACK)
	{
		iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, CPC_INVALIDITEM);
		
		while (iSearchItemIDX != CPC_INVALIDITEM)
		{
			CPL_Stack_Remove(globals.m_hPlaylist, (CP_HPLAYLISTITEM)CLV_GetItemData(globals.m_hPlaylistViewControl, iSearchItemIDX));
			iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, iSearchItemIDX);
		}
	}
	
	else if (uiMenuCommand == IDC_CMDTS_PLAYNOW)
	{
		CP_HPLAYLISTITEM hFirstItem;
		
		CPL_Stack_ClipFromCurrent(globals.m_hPlaylist);
		iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, CPC_INVALIDITEM);
		hFirstItem = NULL;
		
		while (iSearchItemIDX != CPC_INVALIDITEM)
		{
			CP_HPLAYLISTITEM hItem;
			
			hItem = (CP_HPLAYLISTITEM)CLV_GetItemData(globals.m_hPlaylistViewControl, iSearchItemIDX);
			
			if (!hFirstItem)
				hFirstItem = hItem;
				
			CPL_Stack_Append(globals.m_hPlaylist, hItem);
			
			iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, iSearchItemIDX);
		}
		
		CPL_Stack_SetCursor(globals.m_hPlaylist, hFirstItem);
		CPL_PlayItem(globals.m_hPlaylist, TRUE, pmCurrentItem);
	}
	
	else if (uiMenuCommand == IDC_CMDTS_STOPAFTER)
	{
		CPL_Stack_ClipFromItem(globals.m_hPlaylist, hClickedItem);
	}
	
	else if (uiMenuCommand == IDC_CMDTS_STOPAFTER_NOREP)
	{
		options.repeat_playlist = FALSE;
		InvalidateRect(windows.wnd_main, NULL, FALSE);
		CPL_Stack_ClipFromItem(globals.m_hPlaylist, hClickedItem);
	}
	
	else if (uiMenuCommand == IDC_CMDTS_PLAYNEXT)
	{
		iSearchItemIDX = CLV_GetPrevSelectedItem(globals.m_hPlaylistViewControl, CPC_INVALIDITEM);
		
		while (iSearchItemIDX != CPC_INVALIDITEM)
		{
			CPL_Stack_PlayNext(globals.m_hPlaylist, (CP_HPLAYLISTITEM)CLV_GetItemData(globals.m_hPlaylistViewControl, iSearchItemIDX));
			iSearchItemIDX = CLV_GetPrevSelectedItem(globals.m_hPlaylistViewControl, iSearchItemIDX);
		}
	}
	
	else if (uiMenuCommand == IDC_CMDTS_QUEUE)
	{
		iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, CPC_INVALIDITEM);
		
		while (iSearchItemIDX != CPC_INVALIDITEM)
		{
			CP_HPLAYLISTITEM hItem;
			
			hItem = (CP_HPLAYLISTITEM)CLV_GetItemData(globals.m_hPlaylistViewControl, iSearchItemIDX);
			
			CPL_Stack_Remove(globals.m_hPlaylist, hItem);
			CPL_Stack_Append(globals.m_hPlaylist, hItem);
			iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, iSearchItemIDX);
		}
	}
}

//
//
//
void CPlaylistWindow_RenameMenu(const int iItem, const int iSubItem)
{
	HWND hWndList;
	POINT ptItem;
	RECT rSubItem;
	HMENU hmMenu;
	UINT uiMenuCommand;
	
	// We want to get the subitem's rect in the co-ordinate space of the dialog
	hWndList = CLV_GetHWND(globals.m_hPlaylistViewControl);
	CLV_GetItemSubRect(globals.m_hPlaylistViewControl, &rSubItem, iItem, iSubItem);
	ptItem.x = rSubItem.left;
	ptItem.y = rSubItem.bottom;
	ClientToScreen(hWndList, &ptItem);
	
	// Create menu
	hmMenu = CreatePopupMenu();
	
	AppendMenu(hmMenu, MF_STRING, (DWORD)rwsArtistAlbumNumberTitle, "Rename to <artist> - <album> - <tracknum> - <title>");
	AppendMenu(hmMenu, MF_STRING, (DWORD)rwsArtistNumberTitle, "Rename to <artist> - <tracknum> - <title>");
	AppendMenu(hmMenu, MF_STRING, (DWORD)rwsAlbumNumberTitle, "Rename to <album> - <tracknum> - <title>");
	AppendMenu(hmMenu, MF_STRING, (DWORD)rwsAlbumNumber, "Rename to <album> - <tracknum>");
	AppendMenu(hmMenu, MF_STRING, (DWORD)rwsNumberTitle, "Rename to <tracknum> - <title>");
	AppendMenu(hmMenu, MF_STRING, (DWORD)rwsTitle, "Rename to <title>");
	
	uiMenuCommand = TrackPopupMenuEx(hmMenu,
									 TPM_NONOTIFY
									 | TPM_RETURNCMD
									 | TPM_RIGHTBUTTON,
									 ptItem.x, ptItem.y,
									 IF_GetHWnd(windows.m_hifPlaylist),
									 NULL);
	DestroyMenu(hmMenu);
	
	if (uiMenuCommand)
	{
		int iSearchItemIDX;
		int iNumberOfErrors;
		
		iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, CPC_INVALIDITEM);
		iNumberOfErrors = 0;
		
		while (iSearchItemIDX != CPC_INVALIDITEM)
		{
			BOOL bSucceeded;
			bSucceeded = CPLI_RenameTrack((CP_HPLAYLISTITEM)CLV_GetItemData(globals.m_hPlaylistViewControl, iSearchItemIDX),
										  (CPe_FilenameFormat)uiMenuCommand);
			iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, iSearchItemIDX);
			
			if (!bSucceeded)
				iNumberOfErrors++;
		}
		
		if (iNumberOfErrors > 0)
		{
			MessageBox(IF_GetHWnd(windows.m_hifPlaylist),
					   "Some files could not be renamed.\n\nThis could be because they are either currently playing or are read-only.",
					   "Error",
					   MB_OK | MB_ICONASTERISK);
		}
	}
}

//
//
//
void CPlaylistWindow_CreateIPEdit(const int iItem, const int iSubItem)
{
	RECT rSubItem;
	const char* pcClass;
	DWORD dwStyle;
	int iSearchItemIDX;
	UINT uiControlID;
	BOOL bClearNonSelectedItems;
	CP_HPLAYLISTITEM hClickedItem;
	HWND hWndList;
	int iNumItemsSelected;
	
	// If the clicked item is not read/write then skip this
	hClickedItem = (CP_HPLAYLISTITEM)CLV_GetItemData(globals.m_hPlaylistViewControl, iItem);
	
	if (CPLI_GetReadWriteState(hClickedItem) != rwsReadWrite)
	{
		MessageBox(windows.m_hWndPlaylist, "This file's ID3 tag cannot be updated.  This is because CoolPlayer cannot write to this file.", "Cannot update tag", MB_OK | MB_ICONSTOP);
		return;
	}
	
	// For some sub items it is not logical to multi update - action these
	
	if (iSubItem == PLAYLIST_TITLE)
		bClearNonSelectedItems = TRUE;
	else
		bClearNonSelectedItems = FALSE;
		
	// Go through the selected items and remove the selection of items that
	// cannot be written to (eg because of a read only file)
	iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, CPC_INVALIDITEM);
	
	iNumItemsSelected = 1;
	
	for (;iSearchItemIDX != -1; iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, iSearchItemIDX))
	{
		CP_HPLAYLISTITEM hItem = (CP_HPLAYLISTITEM)CLV_GetItemData(globals.m_hPlaylistViewControl, iSearchItemIDX);
		CPLI_ReadTag(hItem);
		
		// We've already checked this item
		
		if (iSearchItemIDX == iItem)
			continue;
			
		iNumItemsSelected++;
		
		if (bClearNonSelectedItems == TRUE || CPLI_GetReadWriteState(hItem) != rwsReadWrite)
			CLV_SetItemSelected(globals.m_hPlaylistViewControl, iSearchItemIDX, FALSE);
	}
	
	// If the "track number" column was clicked - and there are multiple selections - auto number them
	
	if (iSubItem == PLAYLIST_TRACKNUM && iNumItemsSelected > 1)
	{
		char cStatusMessage[1024];
		int iTrackNumber;
		
		// Autonumber
		SetCursor(LoadCursor(NULL, IDC_WAIT));
		iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, CPC_INVALIDITEM);
		iTrackNumber = 1;
		
		for (;iSearchItemIDX != CPC_INVALIDITEM; iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, iSearchItemIDX))
		{
			CP_HPLAYLISTITEM hItem = (CP_HPLAYLISTITEM)CLV_GetItemData(globals.m_hPlaylistViewControl, iSearchItemIDX);
			
			sprintf(cStatusMessage, "Tagging \"%s\"", CPLI_GetFilename(hItem));
			CP_TRACE1("status: %s", cStatusMessage);
			CPIC_SetIndicatorValue("status", cStatusMessage);
			UpdateWindow(IF_GetHWnd(windows.m_hifPlaylist));
			
			CPLI_SetTrackNum(hItem, iTrackNumber);
			CPLI_WriteTag(hItem);
			iTrackNumber++;
		}
		
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		
		CPIC_SetIndicatorValue("status", NULL);
		return;
	}
	
	// If the length was clicked - work out the lengths for all selected items
	
	if (iSubItem == PLAYLIST_LENGTH)
	{
		char cStatusMessage[1024];
		
		SetCursor(LoadCursor(NULL, IDC_WAIT));
		iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, CPC_INVALIDITEM);
		
		for (; iSearchItemIDX != CPC_INVALIDITEM; iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, iSearchItemIDX))
		{
			CP_HPLAYLISTITEM hItem = (CP_HPLAYLISTITEM)CLV_GetItemData(globals.m_hPlaylistViewControl, iSearchItemIDX);
			
			CPLI_CalculateLength(hItem);
			sprintf(cStatusMessage, "Tagging \"%s\"", CPLI_GetFilename(hItem));
			
			CPIC_SetIndicatorValue("status", cStatusMessage);
			CP_TRACE1("status: %s", cStatusMessage);
			UpdateWindow(IF_GetHWnd(windows.m_hifPlaylist));
			
			CPLI_WriteTag(hItem);
		}
		
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		
		CPIC_SetIndicatorValue("status", NULL);
		return;
	}
	
	// We want to get the subitem's rect in the co-ordinate space of the dialog
	hWndList = CLV_GetHWND(globals.m_hPlaylistViewControl);
	
	CLV_GetItemSubRect(globals.m_hPlaylistViewControl, &rSubItem, iItem, iSubItem);
	ClientToScreen(hWndList, (POINT*)&rSubItem);
	ClientToScreen(hWndList, ((POINT*)&rSubItem) + 1);
	ScreenToClient(IF_GetHWnd(windows.m_hifPlaylist), (POINT*)&rSubItem);
	ScreenToClient(IF_GetHWnd(windows.m_hifPlaylist), ((POINT*)&rSubItem) + 1);
	
	if (iSubItem == PLAYLIST_GENRE)
	{
		// int iRectHeight = rSubItem.bottom-rSubItem.top;
		// int iListHeight = iRectHeight<<3;
		
		InflateRect(&rSubItem, 2, 2);
		pcClass = "COMBOBOX";
		uiControlID = IDC_PL_FLOATINGCOMBO;
		dwStyle = CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL;
		
		rSubItem.bottom += (rSubItem.bottom - rSubItem.top) << 3;
	}
	
	else
	{
		InflateRect(&rSubItem, 2, 2);
		pcClass = "EDIT";
		dwStyle = ES_AUTOHSCROLL;
		uiControlID = IDC_PL_FLOATINGEDIT;
		
		if (iSubItem == PLAYLIST_TRACKNUM || iSubItem == PLAYLIST_YEAR)
			dwStyle |= ES_NUMBER;
	}
	
	// Setup window class and style (the Genre window will be a combo)
	globals.m_bIP_InhibitUpdates = TRUE;
	
	windows.wnd_playlist_IPEdit = CreateWindow(pcClass,
								  "",
								  WS_CHILD
								  | WS_BORDER
								  | WS_CLIPSIBLINGS
								  | dwStyle,
								  rSubItem.left, rSubItem.top,
								  rSubItem.right - rSubItem.left, rSubItem.bottom - rSubItem.top,
								  IF_GetHWnd(windows.m_hifPlaylist),
								  (HMENU)uiControlID,
								  GetModuleHandle(NULL), NULL);
	                              
	SetWindowPos(windows.wnd_playlist_IPEdit, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	ShowWindow(windows.wnd_playlist_IPEdit, SW_SHOW);
	SetFocus(windows.wnd_playlist_IPEdit);
	
	// Setup the font in the control
	SendMessage(windows.wnd_playlist_IPEdit, WM_SETFONT, (WPARAM)glb_pSkin->mpl_hfFont, MAKELPARAM(TRUE, 0));
	
	// Hook the listview window (so that we can dismiss on VScroll)
	globals.m_hhkListView_Posted = SetWindowsHookEx(WH_GETMESSAGE, exp_ListViewHookProc_Posted, NULL, GetCurrentThreadId());
	
	// Add the genre items to the list
	if (iSubItem == PLAYLIST_GENRE)
	{
		int iGenreIDX;
		
		for (iGenreIDX = 0; iGenreIDX < CIC_NUMGENRES; iGenreIDX++)
		{
			int iNewItemIDX = SendMessage(windows.wnd_playlist_IPEdit,
										  CB_ADDSTRING,
										  0L,
										  (LPARAM)glb_pcGenres[iGenreIDX]);
			SendMessage(windows.wnd_playlist_IPEdit, CB_SETITEMDATA, (WPARAM)iNewItemIDX, (LPARAM)iGenreIDX);
		}
	}
	
	// Setup the initial string
	globals.m_iInPlaceSubItem = iSubItem;
	
	{
		switch (iSubItem)
		{
		
			case PLAYLIST_TITLE:
				SendMessage(windows.wnd_playlist_IPEdit, WM_SETTEXT, 0L, (LPARAM)CPLI_GetTrackName(hClickedItem));
				
				if (!options.support_id3v2)
					SendMessage(windows.wnd_playlist_IPEdit, EM_LIMITTEXT, 30, 0);
					
				break;
				
			case PLAYLIST_ARTIST:
				SendMessage(windows.wnd_playlist_IPEdit, WM_SETTEXT, 0L, (LPARAM)CPLI_GetArtist(hClickedItem));
				
				if (!options.support_id3v2)
					SendMessage(windows.wnd_playlist_IPEdit, EM_LIMITTEXT, 30, 0);
					
				break;
				
			case PLAYLIST_ALBUM:
				SendMessage(windows.wnd_playlist_IPEdit, WM_SETTEXT, 0L, (LPARAM)CPLI_GetAlbum(hClickedItem));
				
				if (!options.support_id3v2)
					SendMessage(windows.wnd_playlist_IPEdit, EM_LIMITTEXT, 30, 0);
					
				break;
				
			case PLAYLIST_YEAR:
				SendMessage(windows.wnd_playlist_IPEdit, WM_SETTEXT, 0L, (LPARAM)CPLI_GetYear(hClickedItem));
				
				if (!options.support_id3v2)
					SendMessage(windows.wnd_playlist_IPEdit, EM_LIMITTEXT, 4, 0);
					
				break;
				
			case PLAYLIST_TRACKNUM:
			{
				char cTrackNum[33];
				unsigned char iTrackNum;
				
				iTrackNum = CPLI_GetTrackNum(hClickedItem);
				
				if (iTrackNum != CIC_INVALIDTRACKNUM && iTrackNum != 0)
					SendMessage(windows.wnd_playlist_IPEdit, WM_SETTEXT, 0L, (LPARAM)_itoa(iTrackNum, cTrackNum, 10));
			}
			
			SendMessage(windows.wnd_playlist_IPEdit, EM_LIMITTEXT, 3, 0);
			
			break;
			
			case PLAYLIST_COMMENT:
				SendMessage(windows.wnd_playlist_IPEdit, WM_SETTEXT, 0L, (LPARAM)CPLI_GetComment(hClickedItem));
				
				if (!options.support_id3v2)
					SendMessage(windows.wnd_playlist_IPEdit, EM_LIMITTEXT, 28, 0);
					
				break;
				
			case PLAYLIST_GENRE:
				SendMessage(windows.wnd_playlist_IPEdit, CB_SELECTSTRING, (WPARAM) - 1, (LPARAM)CPLI_GetGenre(hClickedItem));
				
				break;
		}
	}
	
	globals.m_bIP_InhibitUpdates = FALSE;
}

//
//
//
BOOL CPlaylistWindow_OffsetSelectedItems(const int iOffset)
{
	int iNumItemsInList;
	int iStartItem, iTermItem, iItemInc, iItemIDX;
	int iScanStartItem, iScanTermItem;
	CP_HPLAYLISTITEM hActive;
	int iNewFocusItemIDX;
	
	if (iOffset == 0)
		return FALSE;
		
	// Determine which direction to move in list
	iNumItemsInList = CLV_GetItemCount(globals.m_hPlaylistViewControl);
	
	if (iOffset < 0)
	{
		iStartItem = 0;
		iTermItem = iNumItemsInList;
		iItemInc = 1;
		
		iScanStartItem = 0;
		iScanTermItem = -iOffset;
	}
	
	else
	{
		iStartItem = iNumItemsInList - 1;
		iTermItem = -1;
		iItemInc = -1;
		
		iScanStartItem = iNumItemsInList - 1;
		iScanTermItem = iScanStartItem - iOffset;
	}
	
	// Ensure that the selection can move intact (ie there are no selected items that could
	// be "scrolled off" the list
	
	for (iItemIDX = iScanStartItem; iItemIDX != iScanTermItem; iItemIDX += iItemInc)
	{
		if (CLV_IsItemSelected(globals.m_hPlaylistViewControl, iItemIDX))
			return FALSE;
	}
	
	iNewFocusItemIDX = CLV_GetFocusItem(globals.m_hPlaylistViewControl) + iOffset;
	
	CLV_SetFocusItem(globals.m_hPlaylistViewControl, iNewFocusItemIDX);
	CLV_EnsureVisible(globals.m_hPlaylistViewControl, iNewFocusItemIDX);
	
	
	// Go through all the items scanning the -ve offset item and swapping it
	// in if needed
	
	for (iItemIDX = iStartItem; iItemIDX != iTermItem; iItemIDX += iItemInc)
	{
		// Work out the item to probe
		const int iProbeItemIDX = iItemIDX - iOffset;
		CP_HPLAYLISTITEM hItem, hItem_Probe, hReindexCursor;
		CP_HPLAYLISTITEM hReindexStart, hReindexEnd;
		int iReindexItemIDX;
		
		// If this probe item is unselected (or out of bounds) - set the current item's selection
		// to unselected and try the next item
		
		if (iProbeItemIDX < 0 || iProbeItemIDX >= iNumItemsInList
				|| CLV_IsItemSelected(globals.m_hPlaylistViewControl, iProbeItemIDX) == FALSE)
		{
			CLV_SetItemSelected(globals.m_hPlaylistViewControl, iItemIDX, FALSE);
			continue;
		}
		
		// The probe item is selected - move that item over to this item
		// - If the item is moving down it needs to be inserted after - otherwise it
		// needs to be inserted before
		hItem = (CP_HPLAYLISTITEM)CLV_GetItemData(globals.m_hPlaylistViewControl, iItemIDX);
		
		hItem_Probe = (CP_HPLAYLISTITEM)CLV_GetItemData(globals.m_hPlaylistViewControl, iProbeItemIDX);
		
		if (iOffset > 0)
		{
			// Get start reindex item
			hReindexStart = CPLI_Next(hItem_Probe);
			
			if (hReindexStart == NULL)
				hReindexStart = CPL_GetFirstItem(globals.m_hPlaylist);
				
			CPL_InsertItemAfter(globals.m_hPlaylist, hItem, hItem_Probe);
			
			// Get end reindex item
			hReindexEnd = CPLI_Next(hItem_Probe);
			
			// Perform reindexing
			iReindexItemIDX = iProbeItemIDX;
			
			for (hReindexCursor = hReindexStart; hReindexCursor != hReindexEnd; hReindexCursor = CPLI_Next(hReindexCursor))
			{
				CPLI_SetCookie(hReindexCursor, iReindexItemIDX);
				CPL_cb_OnItemUpdated(hReindexCursor);
				iReindexItemIDX++;
			}
		}
		
		else
		{
			// Get start reindex item
			hReindexStart = CPLI_Prev(hItem_Probe);
			
			if (hReindexStart == NULL)
				hReindexStart = CPL_GetLastItem(globals.m_hPlaylist);
				
			CPL_InsertItemBefore(globals.m_hPlaylist, hItem, hItem_Probe);
			
			// Get end reindex item
			hReindexEnd = CPLI_Prev(hItem_Probe);
			
			// Perform reindexing
			iReindexItemIDX = iProbeItemIDX;
			
			for (hReindexCursor = hReindexStart; hReindexCursor != hReindexEnd; hReindexCursor = CPLI_Prev(hReindexCursor))
			{
				CPLI_SetCookie(hReindexCursor, iReindexItemIDX);
				CPL_cb_OnItemUpdated(hReindexCursor);
				iReindexItemIDX--;
			}
		}
		
		// Set the item's selection
		CLV_SetItemSelected(globals.m_hPlaylistViewControl, iItemIDX, TRUE);
	}
	
	// Set the "active" item in the list
	hActive = CPL_GetActiveItem(globals.m_hPlaylist);
	
	return TRUE;
}

//
//
//
void CPlaylistWindow_CB_onMouseMove(CP_HINTERFACE hInterface, const POINTS _ptMouse, const unsigned short iFlags)
{
	POINT ptMouse;
	int iHitItem;
	
	ptMouse.x = _ptMouse.x;
	ptMouse.y = _ptMouse.y;
	
	// Which item is this over?
	ClientToScreen(IF_GetHWnd(windows.m_hifPlaylist), &ptMouse);
	ScreenToClient(CLV_GetHWND(globals.m_hPlaylistViewControl), &ptMouse);
	iHitItem = CLV_GetNearestItem(globals.m_hPlaylistViewControl, &ptMouse);
	
	// Perform drag
	
	if (iHitItem != CPC_INVALIDITEM && iHitItem != globals.main_drag_anchor_point)
	{
		BOOL bDragResult;
		bDragResult = CPlaylistWindow_OffsetSelectedItems(iHitItem - globals.main_drag_anchor_point);
		
		if (bDragResult == TRUE)
			globals.main_drag_anchor_point = iHitItem;
	}
}

//
//
//
void CPlaylistWindow_CB_onMouseButton_LUp(CP_HINTERFACE hInterface, const POINTS ptMouse, const unsigned short iFlags)
{
	IF_ReleaseMouseCapture(windows.m_hifPlaylist);
}

//
//
//
void CPlaylistWindow_WM_COMMAND_IDC_PL_FLOATINGEDIT(WPARAM wParam, LPARAM lParam)
{
	if (HIWORD(wParam) == EN_KILLFOCUS)
	{
		// Floating combo control looses focus - destroy it (after breaking out of kill focus context)
		IF_PostAppMessage(windows.m_hifPlaylist, CPPLM_DESTROYINPLACE, 0L, 0L);
	}
	
	else if (HIWORD(wParam) == EN_CHANGE && globals.m_bIP_InhibitUpdates == FALSE)
	{
		int iSearchItemIDX;
		char* pcEditText;
		DWORD dwTextLen;
		
		// Get the text from the control
		dwTextLen = SendMessage(windows.wnd_playlist_IPEdit, WM_GETTEXTLENGTH, 0L, 0L) + 1;
		pcEditText = (char*)malloc(dwTextLen * sizeof(char));
		SendMessage(windows.wnd_playlist_IPEdit, WM_GETTEXT, (WPARAM)dwTextLen, (LPARAM)pcEditText);
		
		// Update all of the selected items
		iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, CPC_INVALIDITEM);
		
		for (;iSearchItemIDX != -1; iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, iSearchItemIDX))
		{
			CP_HPLAYLISTITEM hPlaylistItem = (CP_HPLAYLISTITEM)CLV_GetItemData(globals.m_hPlaylistViewControl, iSearchItemIDX);
			
			switch (globals.m_iInPlaceSubItem)
			{
			
				case PLAYLIST_TITLE:
					CPLI_SetTrackName(hPlaylistItem, pcEditText);
					break;
					
				case PLAYLIST_ARTIST:
					CPLI_SetArtist(hPlaylistItem, pcEditText);
					break;
					
				case PLAYLIST_ALBUM:
					CPLI_SetAlbum(hPlaylistItem, pcEditText);
					break;
					
				case PLAYLIST_YEAR:
					CPLI_SetYear(hPlaylistItem, pcEditText);
					break;
					
				case PLAYLIST_TRACKNUM:
					CPLI_SetTrackNum_AsText(hPlaylistItem, pcEditText);
					break;
					
				case PLAYLIST_COMMENT:
					CPLI_SetComment(hPlaylistItem, pcEditText);
					break;
			}
		}
		
		free(pcEditText);
	}
}

//
//
//
void CPlaylistWindow_WM_COMMAND_IDC_PL_FLOATINGCOMBO(WPARAM wParam, LPARAM lParam)
{
	if (HIWORD(wParam) == CBN_KILLFOCUS)
	{
		// Floating combo control looses focus - destroy it (after breaking out of kill focus context)
		IF_PostAppMessage(windows.m_hifPlaylist, CPPLM_DESTROYINPLACE, 0L, 0L);
	}
	
	else if (HIWORD(wParam) == CBN_SELCHANGE && globals.m_bIP_InhibitUpdates == FALSE)
	{
		int iSearchItemIDX;
		int iSelectedItemIDX;
		unsigned char cNewGenre;
		
		iSelectedItemIDX = SendMessage(windows.wnd_playlist_IPEdit, CB_GETCURSEL, 0L, 0L);
		cNewGenre = (unsigned char)SendMessage(windows.wnd_playlist_IPEdit, CB_GETITEMDATA, (WPARAM)iSelectedItemIDX, 0L);
		
		// Update all of the selected items
		iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, CPC_INVALIDITEM);
		
		for (;iSearchItemIDX != -1; iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, iSearchItemIDX))
		{
			CP_HPLAYLISTITEM hPlaylistItem = (CP_HPLAYLISTITEM)CLV_GetItemData(globals.m_hPlaylistViewControl, iSearchItemIDX);
			CPLI_SetGenreIDX(hPlaylistItem, cNewGenre);
		}
	}
}

//
//
//
void LVCB_DrawBackgroundRect(CPs_DrawContext* pDC)
{
	RECT rClient;
	GetClientRect(IF_GetHWnd(windows.m_hifPlaylist), &rClient);
	
	// Draw the window background
	CPIG_TiledFill(pDC, &rClient, &glb_pSkin->mpl_rBackground_SourceTile, glb_pSkin->mpl_pBackground, CIC_TILEDFILOPTIONS_NONE);
}

//
//
//
void CPlaylistWindow_CB_onClose(CP_HINTERFACE hInterface)
{
	options.show_playlist = FALSE;
	CPlaylistWindow_SetVisible(FALSE);
}

//
//
//
void LVCB_HeaderChanged(CP_HLISTVIEW _hListData)
{
	CLV_GetColumnOrder(_hListData, options.playlist_column_seq, PLAYLIST_last + 1);
	CLV_GetColumnWidths(_hListData, options.playlist_column_widths, PLAYLIST_last + 1);
	CLV_GetColumnVisibleState(_hListData, options.playlist_column_visible, PLAYLIST_last + 1);
}

//
//
//
void LVCB_ItemSelected(CP_HLISTVIEW _hListData, const int iItemIDX, const CP_HPLAYLISTITEM hItem)
{
	if (options.read_id3_tag_of_selected)
		CPLI_ReadTag(hItem);
}

//
//
//
void LVCB_ItemAction(CP_HLISTVIEW _hListData, const int iItemIDX, const CP_HPLAYLISTITEM hItem)
{
	// Setup & play the active item
	if (CPL_Stack_GetItemState(globals.m_hPlaylist, hItem) == issUnstacked)
	{
		CPL_Stack_ClipFromCurrent(globals.m_hPlaylist);
		CPL_Stack_Append(globals.m_hPlaylist, hItem);
		CPL_SetActiveItem(globals.m_hPlaylist, hItem);
		CPL_PlayItem(globals.m_hPlaylist, TRUE, pmCurrentItem);
	}
	
	else
	{
		CPL_SetActiveItem(globals.m_hPlaylist, hItem);
		CPL_PlayItem(globals.m_hPlaylist, TRUE, pmCurrentItem);
	}
}

//
//
//
void LVCB_ItemDrag(CP_HLISTVIEW _hListData, const int iItemIDX, const CP_HPLAYLISTITEM hItem)
{
	globals.main_drag_anchor_point = iItemIDX;
	IF_SetMouseCapture(windows.m_hifPlaylist, CPlaylistWindow_CB_onMouseMove, CPlaylistWindow_CB_onMouseButton_LUp);
}

//
//
//
void LVCB_ColHeaderClick(CP_HLISTVIEW _hListData, const int iColIDX)
{
	// Work out assending or decending
	BOOL bDesc;
	
	if (iColIDX == globals.m_iLastPlaylistSortColoumn)
	{
		globals.m_iLastPlaylistSortColoumn = -1;
		bDesc = TRUE;
	}
	
	else
	{
		globals.m_iLastPlaylistSortColoumn = iColIDX;
		bDesc = FALSE;
	}
	
	// Perform sorting
	
	switch (iColIDX)
	{
	
		case PLAYLIST_TRACKSTACK:
			CPL_SortList(globals.m_hPlaylist, piseTrackStackPos, FALSE);
			break;
			
		case PLAYLIST_TITLE:
			CPL_SortList(globals.m_hPlaylist, piseTrackName, bDesc);
			break;
			
		case PLAYLIST_ARTIST:
			CPL_SortList(globals.m_hPlaylist, piseArtist, bDesc);
			break;
			
		case PLAYLIST_ALBUM:
			CPL_SortList(globals.m_hPlaylist, piseAlbum, bDesc);
			break;
			
		case PLAYLIST_YEAR:
			CPL_SortList(globals.m_hPlaylist, piseYear, bDesc);
			break;
			
		case PLAYLIST_TRACKNUM:
			CPL_SortList(globals.m_hPlaylist, piseTrackNum, bDesc);
			break;
			
		case PLAYLIST_COMMENT:
			CPL_SortList(globals.m_hPlaylist, piseComment, bDesc);
			break;
			
		case PLAYLIST_GENRE:
			CPL_SortList(globals.m_hPlaylist, piseGenre, bDesc);
			break;
			
		case PLAYLIST_PATH:
			CPL_SortList(globals.m_hPlaylist, pisePath, bDesc);
			break;
			
		case PLAYLIST_FILENAME:
			CPL_SortList(globals.m_hPlaylist, piseFilename, bDesc);
			break;
			
		case PLAYLIST_LENGTH:
			CPL_SortList(globals.m_hPlaylist, piseLength, bDesc);
			break;
	}
}

//
//
//
void LVCB_ItemRightClick(CP_HLISTVIEW _hListData, const int iItemIDX, const int iColumnIDX, const CP_HPLAYLISTITEM hItem)
{
	IF_PostAppMessage(windows.m_hifPlaylist, CPPLM_CREATEINPLACE, (WPARAM)iItemIDX, (LPARAM)iColumnIDX);
}

//
//
//
void LVCB_UnhandledKeyPress(CP_HLISTVIEW _hListData, const int iVKey, const BOOL bAlt, const BOOL bCtrl, const BOOL bShift)
{
	CP_HandleKeyPress_Playlist(windows.m_hWndPlaylist, iVKey, bAlt, bCtrl, bShift);
}

//
//
//
void CPlaylistWindow_ClearSelectedItems()
{
	int iSearchItemIDX;
	int iFocusItem;
	
	CLV_BeginBatch(globals.m_hPlaylistViewControl);
	iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, CPC_INVALIDITEM);
	
	while (iSearchItemIDX != CPC_INVALIDITEM)
	{
		CP_HPLAYLISTITEM hItem = (CP_HPLAYLISTITEM)CLV_GetItemData(globals.m_hPlaylistViewControl, iSearchItemIDX);
		CPL_RemoveItem(globals.m_hPlaylist, hItem);
		iSearchItemIDX = CLV_GetNextSelectedItem(globals.m_hPlaylistViewControl, CPC_INVALIDITEM);
	}
	
	iFocusItem = CLV_GetFocusItem(globals.m_hPlaylistViewControl);
	
	if (iFocusItem != CPC_INVALIDITEM)
		CLV_SetItemSelected(globals.m_hPlaylistViewControl, iFocusItem, TRUE);
		
	CLV_EndBatch(globals.m_hPlaylistViewControl);
}

//
//
//
void CPlaylistWindow_CB_onCreate(CP_HINTERFACE hInterface, const RECT* pInitialPosition)
{
	CPlaylistWindow_CreateListView();
}

//
//
//
void CPlaylistWindow_CB_onDraw(CP_HINTERFACE hInterface, CPs_DrawContext* pContext)
{
	RECT rClient;
	
	// Draw the window background
	GetClientRect(IF_GetHWnd(windows.m_hifPlaylist), &rClient);
	CPIG_TiledFill(pContext, &rClient, &glb_pSkin->mpl_rBackground_SourceTile, glb_pSkin->mpl_pBackground, CIC_TILEDFILOPTIONS_NONE);
}

//
//
//
void CPlaylistWindow_CB_onKeyDown(CP_HINTERFACE hInterface, const unsigned int iVKeyCode, const BOOL bAlt, const BOOL bCtrl, const BOOL bShift)
{
	CP_HandleKeyPress_Playlist(NULL, iVKeyCode, bAlt, bCtrl, bShift);
}

//
//
//
void CPlaylistWindow_CB_onDropFiles(CP_HINTERFACE hInterface, HDROP hDrop)
{
	CPL_SyncLoadNextFile(globals.m_hPlaylist);
	CPL_AddDroppedFiles(globals.m_hPlaylist, hDrop);
}

//
//
//
void CPlaylistWindow_CB_onPosChange(CP_HINTERFACE hInterface, const RECT* pNewPosition, const BOOL bSizeChanged)
{
	options.playlist_window_pos = *pNewPosition;
	
	if (bSizeChanged)
	{
		SIZE szWindow;
		
		szWindow.cx = pNewPosition->right - pNewPosition->left;
		szWindow.cy = pNewPosition->bottom - pNewPosition->top;
		MoveWindow(CLV_GetHWND(globals.m_hPlaylistViewControl),
				   glb_pSkin->mpl_rList_Border.left,
				   glb_pSkin->mpl_rList_Border.top,
				   (szWindow.cx - glb_pSkin->mpl_rList_Border.right) - glb_pSkin->mpl_rList_Border.left,
				   (szWindow.cy - glb_pSkin->mpl_rList_Border.bottom) - glb_pSkin->mpl_rList_Border.top,
				   TRUE);
	}
}

//
//
//
void CPlaylistWindow_CB_onFocus(CP_HINTERFACE hInterface, const BOOL bHasFocus)
{
	if (bHasFocus == TRUE)
		SetFocus(CLV_GetHWND(globals.m_hPlaylistViewControl));
}

//
//
//
void CPlaylistWindow_CB_onCommandMessage(CP_HINTERFACE hInterface, const WPARAM wParam, const LPARAM lParam)
{
	if (LOWORD(wParam) == IDC_PL_FLOATINGEDIT)
		CPlaylistWindow_WM_COMMAND_IDC_PL_FLOATINGEDIT(wParam, lParam);
	else if (LOWORD(wParam) == IDC_PL_FLOATINGCOMBO)
		CPlaylistWindow_WM_COMMAND_IDC_PL_FLOATINGCOMBO(wParam, lParam);
}

//
//
//
LRESULT CPlaylistWindow_CB_onAppMessage(CP_HINTERFACE hInterface, const UINT uiMessage, const WPARAM wParam, const LPARAM lParam)
{
	if (uiMessage == CPPLM_CREATEINPLACE)
	{
		int iItem = (int)wParam;
		int iSubItem = (int)lParam;
		
		CPlaylistWindow_DestroyIPEdit();
		
		// It's a hit - create sub control (for IP controls)
		
		if (iItem != CPC_INVALIDITEM && iSubItem != CPC_INVALIDCOLUMN)
		{
			if (iSubItem != PLAYLIST_TRACKSTACK
					&& iSubItem != PLAYLIST_PATH
					&& iSubItem != PLAYLIST_FILENAME)
			{
				CPlaylistWindow_CreateIPEdit(iItem, iSubItem);
			}
			
			else if (iSubItem == PLAYLIST_TRACKSTACK)
			{
				CPlaylistWindow_TrackStackMenu(iItem);
			}
			
			else if (iSubItem == PLAYLIST_PATH || iSubItem == PLAYLIST_FILENAME)
			{
				CPlaylistWindow_RenameMenu(iItem, iSubItem);
			}
		}
	}
	
	else if (uiMessage == CPPLM_DESTROYINPLACE)
		CPlaylistWindow_DestroyIPEdit();
		
	return 0;
}

//
//
//
CPe_CustomDrawColour LVCB_GetTrackStackItemColour(const void* pvItemData)
{
	CP_HPLAYLISTITEM hItem = (CP_HPLAYLISTITEM)pvItemData;
	int iTrackStackPos;
	
	iTrackStackPos = CPLI_GetTrackStackPos(hItem);
	
	if (iTrackStackPos == CIC_TRACKSTACK_UNSTACKED)
		return cdcNormal;
	else if (iTrackStackPos == 0)
		return cdcHighlighted;
	else if (iTrackStackPos < 0)
		return cdcLowlighted;
		
	return cdcNormal;
}

//
//
//
CPe_CustomDrawColour LVCB_GetItemColour(const void* pvItemData)
{
	CP_HPLAYLISTITEM hItem = (CP_HPLAYLISTITEM)pvItemData;
	
	if (CPLI_GetTrackStackPos(hItem) == 0)
		return cdcHighlighted;
		
	return cdcNormal;
}

//
//
//
