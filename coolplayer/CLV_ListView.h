
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
// Custom ListView
//
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
//
typedef enum _CPe_CustomDrawColour
{
	cdcNormal,
	cdcHighlighted,
	cdcLowlighted
} CPe_CustomDrawColour;
//
typedef enum _CPe_ListColumnAlign
{
	lcaLeft,
	lcaCentre,
	lcaRight,
} CPe_ListColumnAlign;
//
////////////////////////////////////////////////////////////////////////////////




typedef const char*(*wp_GetItemText)(const void* pvItemData);
typedef CPe_CustomDrawColour(*wp_GetItemDrawColour)(const void* pvItemData);
typedef void* CP_HLISTVIEW;
////////////////////////////////////////////////////////////////////////////////
//
CP_HLISTVIEW CLV_Create(HWND hWndParent, const int iX, const int iY, const int iWidth, const int iHeight);
HWND CLV_GetHWND(CP_HLISTVIEW _hListData);
void CLV_BeginBatch(CP_HLISTVIEW _hListData);
void CLV_EndBatch(CP_HLISTVIEW _hListData);
//
// Header state
void CLV_AddColumn(CP_HLISTVIEW _hListData, const char* pcTitle, const int iWidth, wp_GetItemText pfnItemTextAccessor, const DWORD dwFlags);
void CLV_SetColumnCustomDrawColour(CP_HLISTVIEW _hListData, const int iColumnIDX, wp_GetItemDrawColour pfnGetCustomDrawColour);
void CLV_SetColumnAlign(CP_HLISTVIEW _hListData, const int iColumnIDX, const CPe_ListColumnAlign enNewAlign);
void CLV_SetColumnOrder(CP_HLISTVIEW _hListData, const unsigned int* pOrder, const unsigned int iNumColumnsInArray);
void CLV_GetColumnOrder(CP_HLISTVIEW _hListData, unsigned int* pOrder, const unsigned int iNumColumnsInArray);
void CLV_GetColumnVisibleState(CP_HLISTVIEW _hListData, BOOL* pStates, const unsigned int iNumColumnsInArray);
void CLV_GetColumnWidths(CP_HLISTVIEW _hListData, int* pWidths, const unsigned int iNumColumnsInArray);
//
// Items
int CLV_AddItem(CP_HLISTVIEW _hListData, const void* pvItemData);
void CLV_RemoveAllItems(CP_HLISTVIEW _hListData);
void CLV_SetItem(CP_HLISTVIEW _hListData, const int iItemIDX, const void* pvItemData);
void CLV_DeleteItem(CP_HLISTVIEW _hListData, const int iItemIDX);
int CLV_GetItemCount(CP_HLISTVIEW _hListData);
void CLV_InvalidateItem(CP_HLISTVIEW _hListData, const int iItemIDX);
void CLV_InvalidateColumn(CP_HLISTVIEW _hListData, const int iColumnIDX);
void CLV_Invalidate(CP_HLISTVIEW _hListData);
void CLV_SetItemData(CP_HLISTVIEW _hListData, const int iItemIDX, const void* pvItemData);
const void* CLV_GetItemData(CP_HLISTVIEW _hListData, const int iItemIDX);
int CLV_GetNearestItem(CP_HLISTVIEW _hListData, const POINT* ptMouse);
void CLV_GetItemSubRect(CP_HLISTVIEW _hListData, RECT* pRect, const int iItemIDX, const int iColumnIDX);
//
// Focus
void CLV_SetFocusItem(CP_HLISTVIEW _hListData, int iNewItemIDX);
int CLV_GetFocusItem(CP_HLISTVIEW _hListData);
//
// Selection
void CLV_ClearSelection(CP_HLISTVIEW _hListData);
int CLV_GetNextSelectedItem(CP_HLISTVIEW _hListData, const int iSearchStart);
int CLV_GetPrevSelectedItem(CP_HLISTVIEW _hListData, const int _iSearchStart);
void CLV_SetItemSelected(CP_HLISTVIEW _hListData, const int iItemIDX, const BOOL bSelected);
BOOL CLV_IsItemSelected(CP_HLISTVIEW _hListData, const int iItemIDX);
void CLV_EnsureVisible(CP_HLISTVIEW _hListData, const int iItemIDX);
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
// Callback handlers
typedef void (*wp_DrawBackgroundRect)(CPs_DrawContext* pDC);
void CLV_sethandler_DrawBackgroundRect(CP_HLISTVIEW _hListData, wp_DrawBackgroundRect pfnHandler);
//
typedef void (*wp_HeaderChanged)(CP_HLISTVIEW _hListData);
void CLV_sethandler_HeaderChanged(CP_HLISTVIEW _hListData, wp_HeaderChanged pfnHandler);
//
typedef void (*wp_ColHeaderClick)(CP_HLISTVIEW _hListData, const int iColIDX);
void CLV_sethandler_ColHeaderClick(CP_HLISTVIEW _hListData, wp_ColHeaderClick pfnHandler);
//
typedef void (*wp_UnhandledKeyPress)(CP_HLISTVIEW _hListData, const int iVKey, const BOOL bAlt, const BOOL bCtrl, const BOOL bShift);
void CLV_sethandler_UnhandledKeyPress(CP_HLISTVIEW _hListData, wp_UnhandledKeyPress pfnHandler);
//
// Item callbacks
typedef void (*wp_ItemCallback)(CP_HLISTVIEW _hListData, const int iItemIDX, const void* pItemData);
typedef void (*wp_ItemSubCallback)(CP_HLISTVIEW _hListData, const int iItemIDX, const int iColumnIDX, const void* pItemData);
//
void CLV_sethandler_ItemSelected(CP_HLISTVIEW _hListData, wp_ItemCallback pfnHandler);
void CLV_sethandler_ItemAction(CP_HLISTVIEW _hListData, wp_ItemCallback pfnHandler);
void CLV_sethandler_ItemDrag(CP_HLISTVIEW _hListData, wp_ItemCallback pfnHandler);
void CLV_sethandler_ItemRightClick(CP_HLISTVIEW _hListData, wp_ItemSubCallback pfnHandler);
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Column flags
#define CPLV_COLFLAG_NONE			0L
#define CPLV_COLFLAG_LOCKRESIZE		1L
#define CPLV_COLFLAG_NOHIDE			2L
#define CPLV_COLFLAG_HIDDEN			4L
//
// Item flags
#define CPLV_ITEMFLAG_NONE			0L
#define CPLV_ITEMFLAG_SELECTED		1L
//
// General
#define CPC_INVALIDCOLUMN			-1
#define CPC_INVALIDITEM				-1
////////////////////////////////////////////////////////////////////////////////
