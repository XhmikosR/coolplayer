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


////////////////////////////////////////////////////////////////////////////////
//
typedef enum _CIe_WindowMode
{
	wmQuiescent,
	wmHeader_Click,
	wmHeader_ChangeWidth,
	wmHeader_ChangeOrder,
	
	wmHScrollbar_DragThumb,
	wmHScrollbar_Scroll_Left,
	wmHScrollbar_Scroll_Right,
	wmHScrollbar_Page_Left,
	wmHScrollbar_Page_Right,
	
	wmVScrollbar_DragThumb,
	wmVScrollbar_Scroll_Up,
	wmVScrollbar_Scroll_Down,
	wmVScrollbar_Page_Up,
	wmVScrollbar_Page_Down,
	
	wmList_Click,
	wmList_Drag
	
} CIe_WindowMode;
//
//
//

typedef struct _CIs_ListView_Column
{
	char* m_pColumnText;
	int m_iColumnWidth;
	DWORD m_dwFlags;
	wp_GetItemText m_pfnTextAccessor;
	wp_GetItemDrawColour m_pfnGetCustomDrawColour;
	CPe_ListColumnAlign m_enAlign;
	
} CIs_ListView_Column;

//
//
//

typedef struct _CIs_ListView_Item
{
	DWORD m_dwFlags;
	const void* m_pItemData;
	
} CIs_ListView_Item;

//
//
//

typedef struct _CIs_ListViewData
{
	HWND m_hWnd;
	BOOL m_bInBatch;
	int m_iBatchNesting;
	BOOL m_bHasFocus;
	unsigned int m_iItemHeight;
	int m_iNumItemsOnPage;
	RECT m_rClient;
	RECT m_rHeader;
	RECT m_rList;
	RECT m_rScrollbar_Horiz;
	RECT m_rScrollbar_Horiz_Thumb;
	RECT m_rScrollbar_Vert;
	RECT m_rScrollbar_Vert_Thumb;
	
	// Columns
	unsigned int m_iNumColumns;
	CIs_ListView_Column* m_pColumns;
	unsigned int* m_piColumnOrder;
	
	// Items
	CIs_ListView_Item* m_pItems;
	int m_iNumItemsInBuffer;
	int m_iNumItems;
	
	// Selection, scroll & focus
	int m_iXOrigin;
	int m_iXScrollExtent;
	BOOL m_bScrollBarVisible_Horiz;
	BOOL m_bScrollBarVisible_Vert;
	int m_iFirstVisibleItem;
	int m_iFocusItem;
	int m_iKeyboardAnchor;
	
	// State
	CIe_WindowMode m_enWindowMode;
	unsigned int m_uiAutorepeatTimer;
	BOOL m_bAutoRepeatFirst;
	BOOL m_bMouseOverScrollbutton;
	int m_iActiveHeaderCol;
	int m_iClickedItem;
	POINT m_ptMouseDown;
	POINT m_ptMouseDown_OnHitItem;
	DWORD m_dwMouseDown_Keys;
	
	// Callback handlers
	wp_DrawBackgroundRect m_hndlr_DrawBackgroundRect;
	wp_HeaderChanged m_hndlr_HeaderChanged;
	wp_ItemCallback m_hndlr_ItemSelected;
	wp_ItemCallback m_hndlr_ItemAction;
	wp_ItemCallback m_hndlr_ItemDrag;
	wp_ItemSubCallback m_hndlr_ItemRightClick;
	wp_ColHeaderClick m_hndlr_ColHeaderClick;
	wp_UnhandledKeyPress m_hndlr_UnhandledKeyPress;
	
} CIs_ListViewData;

//
////////////////////////////////////////////////////////////////////////////////


#define CPC_HEADERCOLLAPSETHRESHOLD  8
#define CPC_HEADERDRAGDISTANCE   16
#define CPC_BUFFERQUANTISATION   128
#define CPC_HEADERDRAG_HTWIDTH   8
#define CPC_HEADERDRAG_DEFAULTWIDTH  100
#define CPC_SCROLLBAR_HORIZ_LINESIZE 10
#define CPC_SCROLLBAR_HORIZ_PAGESIZE 100
#define CPC_SCROLLBAR_MOUSEWHEELAMOUNT 5
#define CPC_TIMERID_AUTOREPEAT   1
#define CPC_LISTDRAGDISTANCE   4
LRESULT CALLBACK exp_ListViewWindowProc(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam);
#define CLC_COOLPLAYER_LISTVIEW_WINDOWCLASSNAME "CoolPlayer_ListView"
////////////////////////////////////////////////////////////////////////////////
//
//
//
CP_HLISTVIEW CLV_Create(HWND hWndParent, const int iX, const int iY, const int iWidth, const int iHeight)
{
	WNDCLASS wcPlaylist;
	HWND hWndWindow;
	CIs_ListViewData* pListData;
	
	wcPlaylist.style = CS_DBLCLKS;
	wcPlaylist.lpfnWndProc = exp_ListViewWindowProc;
	wcPlaylist.cbClsExtra = 0;
	wcPlaylist.cbWndExtra = 0;
	wcPlaylist.hInstance = GetModuleHandle(NULL);
	wcPlaylist.hIcon = NULL; // We will explicity set our icons (so that we have a nice small icon)
	wcPlaylist.hCursor = NULL;
	wcPlaylist.hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH); // Prevent the system drawing white over our invaid rgn before we can paint
	wcPlaylist.lpszMenuName = NULL;
	wcPlaylist.lpszClassName = CLC_COOLPLAYER_LISTVIEW_WINDOWCLASSNAME;
	RegisterClass(&wcPlaylist);
	
	pListData = (CIs_ListViewData*)malloc(sizeof(CIs_ListViewData));
	pListData->m_bInBatch = FALSE;
	pListData->m_iBatchNesting = 0;
	pListData->m_bHasFocus = FALSE;
	pListData->m_iNumColumns = 0;
	pListData->m_pColumns = NULL;
	pListData->m_piColumnOrder = NULL;
	pListData->m_enWindowMode = wmQuiescent;
	pListData->m_uiAutorepeatTimer = 0;
	pListData->m_pItems = NULL;
	pListData->m_iNumItems = 0;
	pListData->m_iNumItemsInBuffer = 0;
	pListData->m_iFirstVisibleItem = 0;
	pListData->m_iXOrigin = 0;
	pListData->m_bScrollBarVisible_Horiz = FALSE;
	pListData->m_iFocusItem = CPC_INVALIDITEM;
	pListData->m_iKeyboardAnchor = CPC_INVALIDITEM;
	
	// Handlers
	pListData->m_hndlr_DrawBackgroundRect = NULL;
	pListData->m_hndlr_HeaderChanged = NULL;
	pListData->m_hndlr_ItemSelected = NULL;
	pListData->m_hndlr_ItemAction = NULL;
	pListData->m_hndlr_ItemDrag = NULL;
	pListData->m_hndlr_ItemRightClick = NULL;
	pListData->m_hndlr_ColHeaderClick = NULL;
	pListData->m_hndlr_UnhandledKeyPress = NULL;
	
	hWndWindow = CreateWindowEx(WS_EX_ACCEPTFILES,
								CLC_COOLPLAYER_LISTVIEW_WINDOWCLASSNAME,
								"",
								WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE,
								iX, iY, iWidth, iHeight, hWndParent,
								NULL,
								GetModuleHandle(NULL),
								pListData);
	                            
	                            
	return (CP_HLISTVIEW)pListData;
}

//
//
//
HWND CLV_GetHWND(CP_HLISTVIEW _hListData)
{
	CIs_ListViewData* pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	return pListData->m_hWnd;
}

//
//
//
void CLV_EmptyItems(CIs_ListViewData* pListData)
{
	if (pListData->m_pItems == NULL)
		return;
		
	free(pListData->m_pItems);
	
	pListData->m_pItems = NULL;
	pListData->m_iNumItems = 0;
	pListData->m_iNumItemsInBuffer = 0;
}

//
//
//
void CLV_InvalidateWindow(CIs_ListViewData* pListData)
{
	InvalidateRect(pListData->m_hWnd, NULL, FALSE);
}

//
//
//
int CLV_YOffsetToItem(CIs_ListViewData* pListData, const int iYOffset)
{
	return pListData->m_iFirstVisibleItem
		   + (int)floor((float)(iYOffset - pListData->m_rList.top) / (float)pListData->m_iItemHeight);
}

//
//
//
int CLV_GetListRect_Lines(CIs_ListViewData* pListData)
{
	return (int)floor((float)(pListData->m_rList.bottom - pListData->m_rList.top) / (float)pListData->m_iItemHeight);
}

//
//
//
void CLV_CleanupWindowData(CIs_ListViewData* pListData)
{
	// Free items
	CLV_EmptyItems(pListData);
	
	// Free columns
	
	if (pListData->m_pColumns)
	{
		unsigned int iColumnIDX;
		
		for (iColumnIDX = 0; iColumnIDX < pListData->m_iNumColumns; iColumnIDX++)
		{
			if (pListData->m_pColumns[iColumnIDX].m_pColumnText)
				free(pListData->m_pColumns[iColumnIDX].m_pColumnText);
		}
	}
	
	free(pListData);
}

//
//
//
void CLV_DrawText(CPs_DrawContext* pDC, const char* pcString, const RECT* _prTarget, const CPe_ListColumnAlign enAlign)
{
	RECT rDraw;
	UINT uiFlags;
	
	// Skip this draw if we are totally clipped
	
	if (_prTarget->right < pDC->m_rClip.left
			|| _prTarget->bottom < pDC->m_rClip.top
			|| _prTarget->left > pDC->m_rClip.right
			|| _prTarget->top > pDC->m_rClip.bottom)
	{
		return;
	}
	
	rDraw = *_prTarget;
	
	if (enAlign == lcaLeft)
		uiFlags = DT_LEFT;
	else if (enAlign == lcaCentre)
		uiFlags = DT_CENTER;
	else if (enAlign == lcaRight)
	{
		uiFlags = DT_RIGHT;
		rDraw.right -= 5;
		
		if (rDraw.right < rDraw.left)
			rDraw.right = rDraw.left;
	}
	
	else
		uiFlags = 0L;
		
	OffsetRect(&rDraw, pDC->m_ptOffset.x, pDC->m_ptOffset.y);
	DrawText(pDC->m_dcDraw, pcString, -1, &rDraw, DT_WORD_ELLIPSIS | DT_NOPREFIX | uiFlags);
}

//
//
//
void CLV_UpdateScrollBars(CIs_ListViewData* pListData)
{
	unsigned int _iColIDX;
	int iListRectWidth;
	int iListRectHeight_Lines;
	BOOL bCountedVScrollbar;
	
	// Get the total width
	pListData->m_iXScrollExtent = 0;
	
	for (_iColIDX = 0; _iColIDX < pListData->m_iNumColumns; _iColIDX++)
	{
		unsigned int iColumnIDX = pListData->m_piColumnOrder[_iColIDX];
		
		if (pListData->m_pColumns[iColumnIDX].m_dwFlags & CPLV_COLFLAG_HIDDEN)
			continue;
			
		pListData->m_iXScrollExtent += pListData->m_pColumns[iColumnIDX].m_iColumnWidth;
	}
	
	// Work out available width
	iListRectWidth = pListData->m_rClient.right - pListData->m_rClient.left;
	
	// - If we need a vertical scrollbar (at this point) - then take this into account
	iListRectHeight_Lines = CLV_GetListRect_Lines(pListData);
	
	if (iListRectHeight_Lines < pListData->m_iNumItems)
	{
		bCountedVScrollbar = TRUE;
		pListData->m_rList.right = pListData->m_rClient.right - glb_pSkin->mpl_pVScrollBar_TrackUp->m_szSize.cx;
		iListRectWidth -= glb_pSkin->mpl_pVScrollBar_TrackUp->m_szSize.cx;
	}
	
	else
		bCountedVScrollbar = FALSE;
		
	// No (horiz) scrollbar needed?
	if (pListData->m_iXScrollExtent <= iListRectWidth)
	{
		pListData->m_rList.bottom = pListData->m_rClient.bottom;
		
		if (pListData->m_bScrollBarVisible_Horiz == TRUE)
			CLV_InvalidateWindow(pListData);
			
		pListData->m_bScrollBarVisible_Horiz = FALSE;
		pListData->m_iXOrigin = 0;
		SetRect(&pListData->m_rScrollbar_Horiz, 0, 0, 0, 0);
	}
	
	else
	{
		int iTrackWidth;
		int iTrackThumbWidth;
		int iTrackThumbPos;
		int iTrackThumbWidth_Min;
		int iTrackThumbWidth_Max;
		
		pListData->m_rList.bottom = pListData->m_rClient.bottom - glb_pSkin->mpl_pHScrollBar_TrackUp->m_szSize.cy;
		
		// The presence of this scrollbar may require a vertical scrollbar - take this into account
		
		if (bCountedVScrollbar == FALSE)
		{
			iListRectHeight_Lines = CLV_GetListRect_Lines(pListData);
			
			if (iListRectHeight_Lines < pListData->m_iNumItems)
			{
				bCountedVScrollbar = TRUE;
				pListData->m_rList.right = pListData->m_rClient.right - glb_pSkin->mpl_pVScrollBar_TrackUp->m_szSize.cx;
				iListRectWidth -= glb_pSkin->mpl_pVScrollBar_TrackUp->m_szSize.cx;
			}
		}
		
		// Work out size of scroll track
		iTrackWidth = iListRectWidth - (glb_pSkin->mpl_pHScrollBar_Left->m_pImage->m_szSize.cx
										+ glb_pSkin->mpl_pHScrollBar_Right->m_pImage->m_szSize.cx);
		                                
		// Setup scrollbar
		if (pListData->m_bScrollBarVisible_Horiz == FALSE)
			CLV_InvalidateWindow(pListData);
			
		pListData->m_bScrollBarVisible_Horiz = TRUE;
		pListData->m_rScrollbar_Horiz = pListData->m_rList;
		pListData->m_rScrollbar_Horiz.top = pListData->m_rList.bottom;
		pListData->m_rScrollbar_Horiz.bottom = pListData->m_rClient.bottom;
		
		// Limit scroll to fit into window
		if ((pListData->m_iXOrigin + iListRectWidth) > pListData->m_iXScrollExtent)
			pListData->m_iXOrigin = pListData->m_iXScrollExtent - iListRectWidth;
			
		// Setup track rect
		iTrackThumbWidth = (int)((((float)iListRectWidth / (float)pListData->m_iXScrollExtent) * (float)iTrackWidth));
		
		iTrackThumbWidth_Min = glb_pSkin->mpl_rHScrollBar_Track_Tile.left
							   + (glb_pSkin->mpl_pHScrollBar_TrackUp->m_szSize.cx - glb_pSkin->mpl_rHScrollBar_Track_Tile.right);
		                       
		iTrackThumbWidth_Max = (pListData->m_rScrollbar_Horiz.right - pListData->m_rScrollbar_Horiz.left);
		
		if (iTrackThumbWidth < iTrackThumbWidth_Min)
			iTrackThumbWidth = iTrackThumbWidth_Min;
			
		if (iTrackThumbWidth > iTrackThumbWidth_Max)
			iTrackThumbWidth = iTrackThumbWidth_Max;
			
		pListData->m_rScrollbar_Horiz_Thumb = pListData->m_rScrollbar_Horiz;
		
		iTrackThumbPos = (int)((((float)pListData->m_iXOrigin / (float)(pListData->m_iXScrollExtent - iListRectWidth)) * (float)(iTrackWidth - iTrackThumbWidth)));
		
		pListData->m_rScrollbar_Horiz_Thumb.left = iTrackThumbPos + glb_pSkin->mpl_pHScrollBar_Left->m_pImage->m_szSize.cx;
		pListData->m_rScrollbar_Horiz_Thumb.right = pListData->m_rScrollbar_Horiz_Thumb.left + iTrackThumbWidth;
		
		InvalidateRect(pListData->m_hWnd, &pListData->m_rScrollbar_Horiz, FALSE);
	}
	
	// Vertical scrollbar
	iListRectHeight_Lines = CLV_GetListRect_Lines(pListData);
	
	if (pListData->m_iNumItems <= iListRectHeight_Lines)
	{
		pListData->m_rList.right = pListData->m_rClient.right;
		pListData->m_rHeader.right = pListData->m_rList.right;
		
		if (pListData->m_bScrollBarVisible_Vert == TRUE)
			CLV_InvalidateWindow(pListData);
			
		pListData->m_bScrollBarVisible_Vert = FALSE;
		pListData->m_iFirstVisibleItem = 0;
		
		SetRect(&pListData->m_rScrollbar_Vert, 0, 0, 0, 0);
	}
	
	else
	{
		int iTrackHeight;
		int iTrackThumbHeight;
		int iTrackThumbPos;
		int iTrackThumbHeight_Min;
		int iTrackThumbHeight_Max;
		const int iListRectHeight = pListData->m_rList.bottom - pListData->m_rList.top;
		
		// Work out size of scroll track
		iTrackHeight = iListRectHeight - (glb_pSkin->mpl_pVScrollBar_Up->m_iStateHeight
										  + glb_pSkin->mpl_pVScrollBar_Down->m_iStateHeight);
		                                  
		// Setup scrollbar
		pListData->m_rList.right = pListData->m_rClient.right - glb_pSkin->mpl_pVScrollBar_TrackUp->m_szSize.cx;
		pListData->m_rHeader.right = pListData->m_rList.right;
		
		if (pListData->m_bScrollBarVisible_Vert == FALSE)
			CLV_InvalidateWindow(pListData);
			
		pListData->m_bScrollBarVisible_Vert = TRUE;
		pListData->m_rScrollbar_Vert = pListData->m_rList;
		pListData->m_rScrollbar_Vert.left = pListData->m_rList.right;
		pListData->m_rScrollbar_Vert.right = pListData->m_rClient.right;
		
		// Limit scroll to fit into window
		if ((pListData->m_iFirstVisibleItem + iListRectHeight_Lines) > pListData->m_iNumItems)
			pListData->m_iFirstVisibleItem = pListData->m_iNumItems - iListRectHeight_Lines;
			
		// Setup track rect
		iTrackThumbHeight = (int)((((float)iListRectHeight_Lines / (float)pListData->m_iNumItems) * (float)iTrackHeight));
		
		iTrackThumbHeight_Min = glb_pSkin->mpl_rVScrollBar_Track_Tile.top
								+ (glb_pSkin->mpl_pVScrollBar_TrackUp->m_szSize.cy - glb_pSkin->mpl_rVScrollBar_Track_Tile.bottom);
		                        
		iTrackThumbHeight_Max = (pListData->m_rScrollbar_Vert.bottom - pListData->m_rScrollbar_Vert.top);
		
		if (iTrackThumbHeight < iTrackThumbHeight_Min)
			iTrackThumbHeight = iTrackThumbHeight_Min;
			
		if (iTrackThumbHeight > iTrackThumbHeight_Max)
			iTrackThumbHeight = iTrackThumbHeight_Max;
			
		pListData->m_rScrollbar_Vert_Thumb = pListData->m_rScrollbar_Vert;
		
		iTrackThumbPos = (int)((((float)pListData->m_iFirstVisibleItem / (float)(pListData->m_iNumItems - iListRectHeight_Lines)) * (float)(iTrackHeight - iTrackThumbHeight)));
		
		pListData->m_rScrollbar_Vert_Thumb.top = iTrackThumbPos + pListData->m_rScrollbar_Vert.top + glb_pSkin->mpl_pVScrollBar_Up->m_iStateHeight;
		pListData->m_rScrollbar_Vert_Thumb.bottom = pListData->m_rScrollbar_Vert_Thumb.top + iTrackThumbHeight;
		
		InvalidateRect(pListData->m_hWnd, &pListData->m_rScrollbar_Horiz, FALSE);
	}
}

//
//
//
void CLV_BeginBatch(CP_HLISTVIEW _hListData)
{
	CIs_ListViewData* pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	pListData->m_bInBatch = TRUE;
	pListData->m_iBatchNesting ++;
}

//
//
//
void CLV_EndBatch(CP_HLISTVIEW _hListData)
{
	CIs_ListViewData* pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	pListData->m_iBatchNesting--;
	
	if (pListData->m_iBatchNesting == 0)
	{
		pListData->m_bInBatch = FALSE;
		CLV_InvalidateWindow(pListData);
		CLV_UpdateScrollBars(pListData);
	}
}

//
//
//
void CLV_Scroll_Horiz(CIs_ListViewData* pListData, const int iPixels)
{
	const int iListRectWidth = pListData->m_rList.right - pListData->m_rList.left;
	int iNewXOrigin;
	
	iNewXOrigin = pListData->m_iXOrigin + iPixels;
	
	// Ensure scoll is in range
	
	if (iNewXOrigin < 0)
		iNewXOrigin = 0;
		
	if ((iNewXOrigin + iListRectWidth) > pListData->m_iXScrollExtent)
		iNewXOrigin = pListData->m_iXScrollExtent - iListRectWidth;
		
	// Update only if we have changed the origin
	if (iNewXOrigin != pListData->m_iXOrigin)
	{
		pListData->m_iXOrigin = iNewXOrigin;
		
		// Update display
		CLV_UpdateScrollBars(pListData);
		CLV_InvalidateWindow(pListData);
	}
}

//
//
//
void CLV_Scroll_Vert(CIs_ListViewData* pListData, const int iLines)
{
	const int iListRectHeight_Lines = CLV_GetListRect_Lines(pListData);
	int iNewFirstVisibleItem;
	
	iNewFirstVisibleItem = pListData->m_iFirstVisibleItem + iLines;
	
	// Ensure scoll is in range
	
	if (iNewFirstVisibleItem < 0)
		iNewFirstVisibleItem = 0;
		
	if ((iNewFirstVisibleItem + iListRectHeight_Lines) > pListData->m_iNumItems)
		iNewFirstVisibleItem = pListData->m_iNumItems - iListRectHeight_Lines;
		
	// Update only if we have changed the first visible item
	if (iNewFirstVisibleItem != pListData->m_iFirstVisibleItem)
	{
		pListData->m_iFirstVisibleItem = iNewFirstVisibleItem;
		CLV_UpdateScrollBars(pListData);
		CLV_InvalidateWindow(pListData);
	}
}

//
//
//
void CLV_UpdateWindowDims(CIs_ListViewData* pListData, const int iCX, const int iCY)
{
	// Work out window parts
	pListData->m_rClient.top = 0;
	pListData->m_rClient.left = 0;
	pListData->m_rClient.right = iCX;
	pListData->m_rClient.bottom = iCY;
	
	pListData->m_rHeader.top = 0;
	pListData->m_rHeader.left = 0;
	pListData->m_rHeader.right = iCX;
	pListData->m_rHeader.bottom = pListData->m_iItemHeight;
	
	pListData->m_rList.top = pListData->m_rHeader.bottom;
	pListData->m_rList.left = 0;
	pListData->m_rList.right = iCX;
	pListData->m_rList.bottom = iCY;
	
	CLV_UpdateScrollBars(pListData);
	
	pListData->m_iNumItemsOnPage = (int)floor((float)(pListData->m_rList.bottom - pListData->m_rList.top) / (float)pListData->m_iItemHeight);
}

//
//
//
void CLV_DrawBackgroundRect(CIs_ListViewData* pListData, CPs_DrawContext* pDC, const RECT* _prTarget)
{
	// Call the parent and get it to draw into this rect
	RECT rDraw;
	POINT ptParentOffset;
	CPs_DrawContext drawcontext;
	HRGN rgnClip;
	
	// Skip this draw if there is no handler registered
	
	if (!pListData->m_hndlr_DrawBackgroundRect)
		return;
		
	// Skip this draw if we are totally clipped
	if (_prTarget->right < pDC->m_rClip.left
			|| _prTarget->bottom < pDC->m_rClip.top
			|| _prTarget->left > pDC->m_rClip.right
			|| _prTarget->top > pDC->m_rClip.bottom)
	{
		return;
	}
	
	// Get rect and clip relative to the parent's origin
	IntersectRect(&rDraw, _prTarget, &pDC->m_rClip);
	
	// Correct our draw context to bring it into our parent's domain
	ptParentOffset.x = 0;
	ptParentOffset.y = 0;
	ClientToScreen(pListData->m_hWnd, &ptParentOffset);
	ScreenToClient(GetParent(pListData->m_hWnd), &ptParentOffset);
	
	drawcontext = *pDC;
	drawcontext.m_ptOffset.x -= ptParentOffset.x;
	drawcontext.m_ptOffset.y -= ptParentOffset.y;
	drawcontext.m_rClip = rDraw;
	OffsetRect(&drawcontext.m_rClip, ptParentOffset.x, ptParentOffset.y);
	
	// Setup a GDI clip region
	rgnClip = CreateRectRgn(rDraw.left + pDC->m_ptOffset.x, rDraw.top + pDC->m_ptOffset.y,
							rDraw.right + pDC->m_ptOffset.x, rDraw.bottom + pDC->m_ptOffset.y);
	                        
	SelectClipRgn(drawcontext.m_dcDraw, rgnClip);
	
	pListData->m_hndlr_DrawBackgroundRect(&drawcontext);
	
	SelectClipRgn(drawcontext.m_dcDraw, NULL);
}

//
//
//
void CLV_Handle_WM_PAINT(CIs_ListViewData* pListData)
{
	RECT rClient;
	PAINTSTRUCT ps;
	HDC dcPaint;
	HBITMAP hbmSurface, hbmSurface_Old;
	HFONT hfOld;
	CPs_DrawContext drawcontext;
	BOOL bAvoidFlicker;
	
	// Do some debug checking
	CP_ASSERT(pListData->m_iFocusItem == CPC_INVALIDITEM || (pListData->m_iFocusItem >= 0 && pListData->m_iFocusItem < pListData->m_iNumItems));
	
	// Prepare for draw
	dcPaint = BeginPaint(pListData->m_hWnd, &ps);
	rClient = pListData->m_rClient;
	GetClipBox(dcPaint, &drawcontext.m_rClip);
	
	// Null clip rgn?
	
	if (drawcontext.m_rClip.right == drawcontext.m_rClip.left
			|| drawcontext.m_rClip.top == drawcontext.m_rClip.bottom)
	{
		EndPaint(pListData->m_hWnd, &ps);
		return;
	}
	
	bAvoidFlicker = TRUE;
	
	if (bAvoidFlicker == TRUE)
	{
		hbmSurface = CreateCompatibleBitmap(dcPaint, drawcontext.m_rClip.right - drawcontext.m_rClip.left, drawcontext.m_rClip.bottom - drawcontext.m_rClip.top);
		CP_ASSERT(hbmSurface);
		drawcontext.m_dcDraw = CreateCompatibleDC(dcPaint);
		CP_ASSERT(drawcontext.m_dcDraw);
		hbmSurface_Old = (HBITMAP)SelectObject(drawcontext.m_dcDraw, hbmSurface);
		
		drawcontext.m_ptOffset.x = -drawcontext.m_rClip.left;
		drawcontext.m_ptOffset.y = -drawcontext.m_rClip.top;
	}
	
	else
	{
		hbmSurface = NULL;
		drawcontext.m_dcDraw = dcPaint;
		drawcontext.m_ptOffset.x = 0;
		drawcontext.m_ptOffset.y = 0;
		hbmSurface_Old = NULL;
	}
	
	// Draw header
	hfOld = (HFONT)SelectObject(drawcontext.m_dcDraw, glb_pSkin->mpl_hfFont);
	
	{
		int iCursorX;
		unsigned int _iColIDX;
		RECT rHeaderItem;
		
		// Draw header items
		iCursorX = -pListData->m_iXOrigin;
		SetTextColor(drawcontext.m_dcDraw, glb_pSkin->mpl_ListHeaderColour);
		SetBkMode(drawcontext.m_dcDraw, TRANSPARENT);
		
		for (_iColIDX = 0; _iColIDX < pListData->m_iNumColumns; _iColIDX++)
		{
			int iColumnIDX = pListData->m_piColumnOrder[_iColIDX];
			int iTextOffset = 0;
			int iNextCursorX;
			
			if (pListData->m_pColumns[iColumnIDX].m_dwFlags & CPLV_COLFLAG_HIDDEN)
				continue;
				
			rHeaderItem.left = iCursorX;
			rHeaderItem.right = iCursorX + pListData->m_pColumns[iColumnIDX].m_iColumnWidth;
			rHeaderItem.top = pListData->m_rHeader.top;
			rHeaderItem.bottom = pListData->m_rHeader.bottom;
			
			if (rHeaderItem.right > (pListData->m_rHeader.right - CPC_HEADERCOLLAPSETHRESHOLD))
				rHeaderItem.right = pListData->m_rHeader.right;
				
			iNextCursorX = rHeaderItem.right;
			
			// Draw this header item down - if it's the clicked or dragged header item
			if ((pListData->m_enWindowMode == wmHeader_Click || pListData->m_enWindowMode == wmHeader_ChangeOrder)
					&& iColumnIDX == pListData->m_iActiveHeaderCol)
			{
				iTextOffset = 1;
				CPIG_TiledFill(&drawcontext, &rHeaderItem, &glb_pSkin->mpl_rListHeader_SourceTile, glb_pSkin->mpl_pListHeader_Down, CIC_TILEDFILOPTIONS_NONE);
			}
			
			else
				CPIG_TiledFill(&drawcontext, &rHeaderItem, &glb_pSkin->mpl_rListHeader_SourceTile, glb_pSkin->mpl_pListHeader_Up, CIC_TILEDFILOPTIONS_NONE);
				
			rHeaderItem.left += glb_pSkin->mpl_rListHeader_SourceTile.left + iTextOffset;
			rHeaderItem.right -= glb_pSkin->mpl_pListHeader_Down->m_szSize.cx - glb_pSkin->mpl_rListHeader_SourceTile.right;
			rHeaderItem.top += iTextOffset;
			
			if (pListData->m_pColumns[iColumnIDX].m_pColumnText)
				CLV_DrawText(&drawcontext, pListData->m_pColumns[iColumnIDX].m_pColumnText, &rHeaderItem, pListData->m_pColumns[iColumnIDX].m_enAlign);
				
			iCursorX = iNextCursorX;
			
			if (iCursorX == rClient.right)
				break;
		}
		
		// Draw a dummy last column
		
		if (iCursorX < rClient.right)
		{
			rHeaderItem.left = iCursorX;
			rHeaderItem.right = pListData->m_rHeader.right;
			rHeaderItem.top = pListData->m_rHeader.top;
			rHeaderItem.bottom = pListData->m_rHeader.top + pListData->m_iItemHeight;
			CPIG_TiledFill(&drawcontext, &rHeaderItem, &glb_pSkin->mpl_rListHeader_SourceTile, glb_pSkin->mpl_pListHeader_Up, CIC_TILEDFILOPTIONS_NONE);
		}
		
	}
	
	// Draw the (horiz) scrollbar
	
	if (pListData->m_bScrollBarVisible_Horiz == TRUE)
	{
		RECT rHScrollBarTrack = pListData->m_rScrollbar_Horiz;
		rHScrollBarTrack.left += glb_pSkin->mpl_pHScrollBar_Left->m_pImage->m_szSize.cx;
		rHScrollBarTrack.right -= glb_pSkin->mpl_pHScrollBar_Right->m_pImage->m_szSize.cx;
		
		// Draw buttons
		CPIG_DrawStateImage(&drawcontext,
							pListData->m_rScrollbar_Horiz.left,
							rHScrollBarTrack.top,
							glb_pSkin->mpl_pHScrollBar_Left,
							(pListData->m_enWindowMode == wmHScrollbar_Scroll_Left && pListData->m_bMouseOverScrollbutton)
							? igsActive : igsQuiescent);
		CPIG_DrawStateImage(&drawcontext,
							rHScrollBarTrack.right,
							rHScrollBarTrack.top,
							glb_pSkin->mpl_pHScrollBar_Right,
							(pListData->m_enWindowMode == wmHScrollbar_Scroll_Right && pListData->m_bMouseOverScrollbutton)
							? igsActive : igsQuiescent);
		                    
		// Scrollbar background
		CPIG_TiledFill(&drawcontext, &rHScrollBarTrack, &glb_pSkin->mpl_rHScrollBar_Bk_Tile, glb_pSkin->mpl_pHScrollBar_Bk, CIC_TILEDFILOPTIONS_NONE);
		
		// Track
		CPIG_TiledFill(&drawcontext,
					   &pListData->m_rScrollbar_Horiz_Thumb,
					   &glb_pSkin->mpl_rHScrollBar_Track_Tile,
					   pListData->m_enWindowMode == wmHScrollbar_DragThumb ? glb_pSkin->mpl_pHScrollBar_TrackDn : glb_pSkin->mpl_pHScrollBar_TrackUp, CIC_TILEDFILOPTIONS_NONE);
	}
	
	// Draw the (vert) scrollbar
	
	if (pListData->m_bScrollBarVisible_Vert == TRUE)
	{
		RECT rHeaderPad;
		RECT rVScrollBarTrack = pListData->m_rScrollbar_Vert;
		rVScrollBarTrack.top += glb_pSkin->mpl_pHScrollBar_Left->m_iStateHeight;
		rVScrollBarTrack.bottom -= glb_pSkin->mpl_pHScrollBar_Right->m_iStateHeight;
		
		// Draw buttons
		CPIG_DrawStateImage(&drawcontext,
							rVScrollBarTrack.left, pListData->m_rScrollbar_Vert.top,
							glb_pSkin->mpl_pVScrollBar_Up,
							(pListData->m_enWindowMode == wmVScrollbar_Scroll_Up && pListData->m_bMouseOverScrollbutton)
							? igsActive : igsQuiescent);
		CPIG_DrawStateImage(&drawcontext,
							rVScrollBarTrack.left, rVScrollBarTrack.bottom,
							glb_pSkin->mpl_pVScrollBar_Down,
							(pListData->m_enWindowMode == wmVScrollbar_Scroll_Down && pListData->m_bMouseOverScrollbutton)
							? igsActive : igsQuiescent);
		                    
		// Scrollbar background
		CPIG_TiledFill(&drawcontext, &rVScrollBarTrack, &glb_pSkin->mpl_rVScrollBar_Bk_Tile, glb_pSkin->mpl_pVScrollBar_Bk, CIC_TILEDFILOPTIONS_NONE);
		
		// Track
		CPIG_TiledFill(&drawcontext,
					   &pListData->m_rScrollbar_Vert_Thumb,
					   &glb_pSkin->mpl_rVScrollBar_Track_Tile,
					   pListData->m_enWindowMode == wmVScrollbar_DragThumb ? glb_pSkin->mpl_pVScrollBar_TrackDn : glb_pSkin->mpl_pVScrollBar_TrackUp,
					   CIC_TILEDFILOPTIONS_NONE);
		               
		// Draw some background above the scrollbar (and beyond the header area)
		rHeaderPad = pListData->m_rHeader;
		rHeaderPad.left = rHeaderPad.right;
		rHeaderPad.right = pListData->m_rClient.right;
		CLV_DrawBackgroundRect(pListData, &drawcontext, &rHeaderPad);
	}
	
	// Draw a background area in the gap between the 2 scrollbars
	
	if (pListData->m_bScrollBarVisible_Horiz == TRUE && pListData->m_bScrollBarVisible_Vert == TRUE)
	{
		RECT rScrollbarGap;
		rScrollbarGap = pListData->m_rClient;
		rScrollbarGap.top = pListData->m_rScrollbar_Vert.bottom;
		rScrollbarGap.left = pListData->m_rScrollbar_Horiz.right;
		CLV_DrawBackgroundRect(pListData, &drawcontext, &rScrollbarGap);
	}
	
	// Draw the list
	// - clip to list rect
	{
		RECT rList = pListData->m_rList;
		OffsetRect(&rList, drawcontext.m_ptOffset.x, drawcontext.m_ptOffset.y);
		IntersectClipRect(drawcontext.m_dcDraw, rList.left, rList.top, rList.right, rList.bottom);
	}
	
	// - draw background
	CPIG_TiledFill(&drawcontext, &pListData->m_rList, &glb_pSkin->mpl_rListBackground_SourceTile, glb_pSkin->mpl_pListBackground, CIC_TILEDFILOPTIONS_NONE);
	
	// Draw the selection
	{
		int iItemIDX;
		int iFirstVisibleItem;
		int iLastVisibleItem;
		BOOL bInSelection;
		RECT rSelection;
		
		// Setup bounds
		iFirstVisibleItem = pListData->m_iFirstVisibleItem - 1;
		iLastVisibleItem = iFirstVisibleItem + ((pListData->m_rList.bottom - pListData->m_rList.top) / pListData->m_iItemHeight) + 1;
		
		if (iFirstVisibleItem < 0)
			iFirstVisibleItem = 0;
			
		if (iLastVisibleItem >= pListData->m_iNumItems)
			iLastVisibleItem = pListData->m_iNumItems - 1;
			
		// Part initialise selection rect
		rSelection.left = -pListData->m_iXOrigin + glb_pSkin->mpl_rListBackground_SourceTile.left;
		rSelection.right = (pListData->m_iXScrollExtent - pListData->m_iXOrigin) - (glb_pSkin->mpl_pListBackground->m_szSize.cx - glb_pSkin->mpl_rListBackground_SourceTile.right);
		
		// Draw selection (grouping adjacent selected items)
		bInSelection = FALSE;
		
		for (iItemIDX = iFirstVisibleItem; iItemIDX <= iLastVisibleItem; iItemIDX++)
		{
			// Is this item selected?
			if (pListData->m_pItems[iItemIDX].m_dwFlags & CPLV_ITEMFLAG_SELECTED)
			{
				// Open a new selection (if we are not in one already)
				if (bInSelection == FALSE)
				{
					bInSelection = TRUE;
					rSelection.top = (iItemIDX + 1 - pListData->m_iFirstVisibleItem) * pListData->m_iItemHeight;
				}
			}
			
			else
			{
				// Are we in a selection (if so complete it)
				if (bInSelection)
				{
					bInSelection = FALSE;
					rSelection.bottom = (iItemIDX + 1 - pListData->m_iFirstVisibleItem) * pListData->m_iItemHeight;
					CPIG_TiledFill(&drawcontext, &rSelection, &glb_pSkin->mpl_rSelection_Tile, glb_pSkin->mpl_pSelection, CIC_TILEDFILOPTIONS_NONE);
				}
			}
		}
		
		// Close any open groups
		
		if (bInSelection)
		{
			bInSelection = FALSE;
			rSelection.bottom = ((iLastVisibleItem + 2) - pListData->m_iFirstVisibleItem) * pListData->m_iItemHeight;
			CPIG_TiledFill(&drawcontext, &rSelection, &glb_pSkin->mpl_rSelection_Tile, glb_pSkin->mpl_pSelection, CIC_TILEDFILOPTIONS_NONE);
		}
		
		// Draw focus item
		
		if (pListData->m_iFocusItem != CPC_INVALIDITEM
				&& pListData->m_iFocusItem >= iFirstVisibleItem && pListData->m_iFocusItem <= iLastVisibleItem
				&& pListData->m_bHasFocus == TRUE)
		{
			RECT rFocus;
			rFocus = rSelection;
			rFocus.top = (pListData->m_iFocusItem + 1 - pListData->m_iFirstVisibleItem) * pListData->m_iItemHeight;
			rFocus.bottom = (pListData->m_iFocusItem + 2 - pListData->m_iFirstVisibleItem) * pListData->m_iItemHeight;
			CPIG_TiledFill(&drawcontext, &rFocus, &glb_pSkin->mpl_rFocus_Tile, glb_pSkin->mpl_pFocus, CIC_TILEDFILOPTIONS_NOCENTRE);
		}
	}
	
	// Draw the items
	{
		int iCursorX;
		unsigned int _iColIDX;
		int iFirstVisibleItem;
		int iLastVisibleItem;
		RECT rItem;
		
		iFirstVisibleItem = pListData->m_iFirstVisibleItem;
		iLastVisibleItem = iFirstVisibleItem + ((pListData->m_rList.bottom - pListData->m_rList.top) / pListData->m_iItemHeight);
		
		if (iLastVisibleItem >= pListData->m_iNumItems)
			iLastVisibleItem = pListData->m_iNumItems - 1;
			
		// Draw items column by column
		iCursorX = -pListData->m_iXOrigin;
		
		SetBkMode(drawcontext.m_dcDraw, TRANSPARENT);
		
		for (_iColIDX = 0; _iColIDX < pListData->m_iNumColumns; _iColIDX++)
		{
			int iColumnIDX = pListData->m_piColumnOrder[_iColIDX];
			int iItemIDX;
			int iNextCursorX;
			
			if (pListData->m_pColumns[iColumnIDX].m_dwFlags & CPLV_COLFLAG_HIDDEN)
				continue;
				
			rItem.left = iCursorX;
			rItem.right = iCursorX + pListData->m_pColumns[iColumnIDX].m_iColumnWidth;
			
			if (rItem.right > (pListData->m_rHeader.right - CPC_HEADERCOLLAPSETHRESHOLD))
				rItem.right = pListData->m_rHeader.right;
				
			iNextCursorX = rItem.right;
			
			// Line text up with header
			rItem.left += glb_pSkin->mpl_rListHeader_SourceTile.left;
			rItem.right -= glb_pSkin->mpl_pListHeader_Down->m_szSize.cx - glb_pSkin->mpl_rListHeader_SourceTile.right;
			
			// Draw item text (if there is any and this column is inside the clip box)
			if (pListData->m_pColumns[iColumnIDX].m_pfnTextAccessor
					&& rItem.right > drawcontext.m_rClip.left
					&& rItem.left < drawcontext.m_rClip.right)
			{
				int iCursorY = pListData->m_rList.top;
				
				for (iItemIDX = iFirstVisibleItem; iItemIDX <= iLastVisibleItem; iItemIDX++)
				{
					// Set the text colour depending on whether we are selected
					const char* pcText = pListData->m_pColumns[iColumnIDX].m_pfnTextAccessor(pListData->m_pItems[iItemIDX].m_pItemData);
					
					if (pcText)
					{
						if (pListData->m_pItems[iItemIDX].m_dwFlags & CPLV_ITEMFLAG_SELECTED)
							SetTextColor(drawcontext.m_dcDraw, glb_pSkin->mpl_ListTextColour_Selected);
						else
						{
							if (pListData->m_pColumns[iColumnIDX].m_pfnGetCustomDrawColour)
							{
								CPe_CustomDrawColour enDrawColour;
								
								enDrawColour = pListData->m_pColumns[iColumnIDX].m_pfnGetCustomDrawColour(pListData->m_pItems[iItemIDX].m_pItemData);
								
								if (enDrawColour == cdcNormal)
									SetTextColor(drawcontext.m_dcDraw, glb_pSkin->mpl_ListTextColour);
								else if (enDrawColour == cdcLowlighted)
									SetTextColor(drawcontext.m_dcDraw, RGB(0, 0, 0));
								else
									SetTextColor(drawcontext.m_dcDraw, RGB(255, 255, 255));
							}
							
							else
								SetTextColor(drawcontext.m_dcDraw, glb_pSkin->mpl_ListTextColour);
						}
						
						// Draw item's text
						rItem.top = iCursorY;
						rItem.bottom = iCursorY + pListData->m_iItemHeight;
						
						CLV_DrawText(&drawcontext, pcText, &rItem, pListData->m_pColumns[iColumnIDX].m_enAlign);
					}
					
					iCursorY += pListData->m_iItemHeight;
				}
			}
			
			iCursorX = iNextCursorX;
			
			if (iCursorX == pListData->m_rList.right)
				break;
		}
	}
	
	// Cleanup
	SelectObject(drawcontext.m_dcDraw, hfOld);
	
	if (hbmSurface)
	{
		BitBlt(dcPaint,
			   drawcontext.m_rClip.left, drawcontext.m_rClip.top,
			   drawcontext.m_rClip.right - drawcontext.m_rClip.left,
			   drawcontext.m_rClip.bottom - drawcontext.m_rClip.top,
			   drawcontext.m_dcDraw, 0, 0, SRCCOPY);
		SelectObject(drawcontext.m_dcDraw, hbmSurface_Old);
		DeleteDC(drawcontext.m_dcDraw);
		DeleteObject(hbmSurface);
	}
	
	EndPaint(pListData->m_hWnd, &ps);
}

//
//
//
void CLV_CalcItemHeight(CIs_ListViewData* pListData)
{
	HDC dcWindow;
	SIZE szText;
	HFONT hfOld;
	
	dcWindow = GetDC(pListData->m_hWnd);
	hfOld = (HFONT)SelectObject(dcWindow, glb_pSkin->mpl_hfFont);
	GetTextExtentPoint(dcWindow, "Xy", 2, &szText);
	pListData->m_iItemHeight = szText.cy;
	SelectObject(dcWindow, hfOld);
	ReleaseDC(pListData->m_hWnd, dcWindow);
}

//
//
//
int CLV_HitTest_Header_X(CIs_ListViewData* pListData, const int iTestX)
{
	// We are in the header - which one?
	int iCursorX;
	int iLastCursorX;
	unsigned int _iColIDX;
	iCursorX = -pListData->m_iXOrigin;
	iLastCursorX = 0;
	
	for (_iColIDX = 0; _iColIDX < pListData->m_iNumColumns; _iColIDX++)
	{
		unsigned int iColumnIDX = pListData->m_piColumnOrder[_iColIDX];
		
		if (pListData->m_pColumns[iColumnIDX].m_dwFlags & CPLV_COLFLAG_HIDDEN)
			continue;
			
		iLastCursorX = iCursorX;
		
		iCursorX += pListData->m_pColumns[iColumnIDX].m_iColumnWidth;
		
		if (iCursorX > (pListData->m_rHeader.right - CPC_HEADERCOLLAPSETHRESHOLD))
			iCursorX = pListData->m_rHeader.right;
			
		if (iTestX >= iLastCursorX && iTestX < iCursorX)
			return iColumnIDX;
	}
	
	return CPC_INVALIDCOLUMN;
}

//
//
//
int CLV_HitTest_HeaderSizer_X(CIs_ListViewData* pListData, const int iTestX)
{
	// We are in the header - are we near a drag boundry?
	int iCursorX;
	unsigned int _iColIDX;
	iCursorX = -pListData->m_iXOrigin;
	
	for (_iColIDX = 0; _iColIDX < pListData->m_iNumColumns; _iColIDX++)
	{
		unsigned int iColumnIDX = pListData->m_piColumnOrder[_iColIDX];
		int iColSizer;
		
		if (pListData->m_pColumns[iColumnIDX].m_dwFlags & CPLV_COLFLAG_HIDDEN)
			continue;
			
		iColSizer = iCursorX + pListData->m_pColumns[iColumnIDX].m_iColumnWidth;
		
		if (iColSizer > (pListData->m_rHeader.right - CPC_HEADERCOLLAPSETHRESHOLD))
			iColSizer = pListData->m_rHeader.right - CPC_HEADERCOLLAPSETHRESHOLD;
			
		if (iTestX >= (iColSizer - CPC_HEADERDRAG_HTWIDTH) && iTestX <= (iColSizer + CPC_HEADERDRAG_HTWIDTH))
		{
			// If we cannot resize - return an invalid column
			if (pListData->m_pColumns[iColumnIDX].m_dwFlags & CPLV_COLFLAG_LOCKRESIZE)
				return CPC_INVALIDCOLUMN;
				
			return iColumnIDX;
		}
		
		iCursorX = iColSizer;
	}
	
	return CPC_INVALIDCOLUMN;
}

//
//
//
void CLV_Handle_WM_MOUSEMOVE(CIs_ListViewData* pListData, const POINTS _ptCursor)
{
	POINT ptCursor;
	
	// Init
	ptCursor.x = _ptCursor.x;
	ptCursor.y = _ptCursor.y;
	
	if (pListData->m_enWindowMode == wmQuiescent)
	{
		// Set cursor according to where it is
		if (PtInRect(&pListData->m_rHeader, ptCursor) == TRUE)
		{
			if (CLV_HitTest_HeaderSizer_X(pListData, ptCursor.x) == CPC_INVALIDCOLUMN)
				SetCursor(LoadCursor(NULL, IDC_ARROW)); // In a header
			else
				SetCursor(LoadCursor(NULL, IDC_SIZEWE)); // Sizing
		}
		
		else
			SetCursor(LoadCursor(NULL, IDC_ARROW));
	}
	
	else if (pListData->m_enWindowMode == wmHeader_Click)
	{
		// If the mouse has moved beyond our drag threshold then enter into header_orderchange mode
		if (abs(ptCursor.x - pListData->m_ptMouseDown.x) > CPC_HEADERDRAGDISTANCE
				|| abs(ptCursor.y - pListData->m_ptMouseDown.y) > CPC_HEADERDRAGDISTANCE)
		{
			pListData->m_enWindowMode = wmHeader_ChangeOrder;
			InvalidateRect(pListData->m_hWnd, &pListData->m_rHeader, FALSE);
		}
	}
	
	else if (pListData->m_enWindowMode == wmHeader_ChangeWidth)
	{
		// Update the new column width
		// - get the start of this colunm
		unsigned int _iColIDX;
		int iCursorX;
		RECT rInvalid;
		DWORD dwNewFlags;
		int iNewWidth;
		
		// Find the start point of the active column
		iCursorX = -pListData->m_iXOrigin;
		
		for (_iColIDX = 0; _iColIDX < pListData->m_iNumColumns; _iColIDX++)
		{
			unsigned int iColumnIDX = pListData->m_piColumnOrder[_iColIDX];
			
			if ((int)iColumnIDX == pListData->m_iActiveHeaderCol)
				break;
				
			if (pListData->m_pColumns[iColumnIDX].m_dwFlags & CPLV_COLFLAG_HIDDEN)
				continue;
				
			iCursorX += pListData->m_pColumns[iColumnIDX].m_iColumnWidth;
		}
		
		// Invalidate only the area that needs it
		rInvalid.top = 0;
		rInvalid.bottom = pListData->m_rClient.bottom;
		rInvalid.right = pListData->m_rClient.right;
		rInvalid.left = iCursorX;
		
		// Perform resize
		dwNewFlags = pListData->m_pColumns[pListData->m_iActiveHeaderCol].m_dwFlags;
		
		iNewWidth = pListData->m_pColumns[pListData->m_iActiveHeaderCol].m_iColumnWidth;
		
		if (ptCursor.x > (iCursorX + CPC_HEADERDRAG_HTWIDTH))
		{
			dwNewFlags &= (~CPLV_COLFLAG_HIDDEN);
			iNewWidth = ptCursor.x - iCursorX;
		}
		
		else
		{
			if (dwNewFlags & CPLV_COLFLAG_NOHIDE)
			{
				iNewWidth = CPC_HEADERDRAG_HTWIDTH;
			}
			
			else
			{
				dwNewFlags |= CPLV_COLFLAG_HIDDEN;
				iNewWidth = CPC_HEADERDRAG_DEFAULTWIDTH;
			}
		}
		
		// Update dispaly only if the width (or visible state) has changed
		
		if (iNewWidth != pListData->m_pColumns[pListData->m_iActiveHeaderCol].m_iColumnWidth
				|| dwNewFlags != pListData->m_pColumns[pListData->m_iActiveHeaderCol].m_dwFlags)
		{
			pListData->m_pColumns[pListData->m_iActiveHeaderCol].m_dwFlags = dwNewFlags;
			pListData->m_pColumns[pListData->m_iActiveHeaderCol].m_iColumnWidth = iNewWidth;
			InvalidateRect(pListData->m_hWnd, &rInvalid, FALSE);
			CLV_UpdateScrollBars(pListData);
		}
		
	}
	
	else if (pListData->m_enWindowMode == wmHeader_ChangeOrder)
	{
		// Search for our drag rect - keep track of the threshold of the prev and next colunms
		unsigned int _iColIDX;
		int iXColStart_Prev = 0;
		int iXColEnd_Prev = 0;
		int iXColStart_Next = 0;
		int iXColEnd_Next = 0;
		int iPrevColIDX = CPC_INVALIDCOLUMN;
		int iNextColIDX = CPC_INVALIDCOLUMN;
		unsigned int iCurrentOrderIDX;
		int iActiveColumnWidth;
		int iCursorX;
		BOOL bFoundColumn;
		
		// Find prev and next colunms
		bFoundColumn = FALSE;
		iCursorX = -pListData->m_iXOrigin;
		iCurrentOrderIDX = 0;
		iActiveColumnWidth = 0;
		
		for (_iColIDX = 0; _iColIDX < pListData->m_iNumColumns; _iColIDX++)
		{
			int iColumnIDX = pListData->m_piColumnOrder[_iColIDX];
			
			if (pListData->m_pColumns[iColumnIDX].m_dwFlags & CPLV_COLFLAG_HIDDEN)
				continue;
				
			// Update prev and next
			if (bFoundColumn == TRUE && iNextColIDX == CPC_INVALIDCOLUMN)
			{
				iXColStart_Next = iCursorX;
				iXColEnd_Next = iCursorX + pListData->m_pColumns[iColumnIDX].m_iColumnWidth;
				iNextColIDX = iColumnIDX;
			}
			
			if (iColumnIDX == pListData->m_iActiveHeaderCol)
			{
				iCurrentOrderIDX = _iColIDX;
				iActiveColumnWidth = pListData->m_pColumns[iColumnIDX].m_iColumnWidth;
				bFoundColumn = TRUE;
			}
			
			else if (bFoundColumn == FALSE)
			{
				iPrevColIDX = iColumnIDX;
				iXColStart_Prev = iCursorX;
				iXColEnd_Prev = iCursorX + pListData->m_pColumns[iColumnIDX].m_iColumnWidth;
			}
			
			iCursorX += pListData->m_pColumns[iColumnIDX].m_iColumnWidth;
		}
		
		// If we have dragged our mouse beyond the prev threshold - swap the order of that with the current col
		// - Only swap if the mouse will remain on top of the active column
		
		if (iPrevColIDX != CPC_INVALIDCOLUMN
				&& ptCursor.x < iXColEnd_Prev
				&& ptCursor.x < (iXColStart_Prev + iActiveColumnWidth))
		{
			int iTemp;
			RECT rInvalid;
			
			iTemp = pListData->m_piColumnOrder[iCurrentOrderIDX];
			pListData->m_piColumnOrder[iCurrentOrderIDX] = pListData->m_piColumnOrder[iCurrentOrderIDX-1];
			pListData->m_piColumnOrder[iCurrentOrderIDX-1] = iTemp;
			
			// Invalidate only the area that needs it
			rInvalid.top = 0;
			rInvalid.bottom = pListData->m_rClient.bottom;
			rInvalid.left = iXColStart_Prev;
			rInvalid.right = iXColEnd_Prev + iActiveColumnWidth;
			InvalidateRect(pListData->m_hWnd, &rInvalid, FALSE);
			
			// Send notify
			
			if (pListData->m_hndlr_HeaderChanged)
				pListData->m_hndlr_HeaderChanged(pListData);
		}
		
		// If we have dragged our mouse beyond the next threshold - swap the order of that with the current col
		// - Only swap if the mouse will remain on top of the active column
		
		else if (iNextColIDX != CPC_INVALIDCOLUMN
				 && ptCursor.x > iXColStart_Next
				 && ptCursor.x > (iXColEnd_Next - iActiveColumnWidth))
		{
			int iTemp;
			RECT rInvalid;
			
			iTemp = pListData->m_piColumnOrder[iCurrentOrderIDX];
			pListData->m_piColumnOrder[iCurrentOrderIDX] = pListData->m_piColumnOrder[iCurrentOrderIDX+1];
			pListData->m_piColumnOrder[iCurrentOrderIDX+1] = iTemp;
			
			// Invalidate only the area that needs it
			rInvalid.top = 0;
			rInvalid.bottom = pListData->m_rClient.bottom;
			rInvalid.left = iXColStart_Next - iActiveColumnWidth;
			rInvalid.right = iXColEnd_Next;
			InvalidateRect(pListData->m_hWnd, &rInvalid, FALSE);
			
			// Send notify
			
			if (pListData->m_hndlr_HeaderChanged)
				pListData->m_hndlr_HeaderChanged(pListData);
		}
	}
	
	else if (pListData->m_enWindowMode == wmHScrollbar_DragThumb)
	{
		if (pListData->m_ptMouseDown.x != ptCursor.x)
		{
			const int iTrackWidth = (pListData->m_rScrollbar_Horiz.right - pListData->m_rScrollbar_Horiz.left)
									- (glb_pSkin->mpl_pHScrollBar_Left->m_pImage->m_szSize.cx
									   + glb_pSkin->mpl_pHScrollBar_Right->m_pImage->m_szSize.cx);
			const int iTrackThumbWidth = pListData->m_rScrollbar_Horiz_Thumb.right - pListData->m_rScrollbar_Horiz_Thumb.left;
			const int iListRectWidth = pListData->m_rList.right - pListData->m_rList.left;
			int iNewTrackPos;
			int iNewXOrigin;
			
			// Work out new origin based on the track drag distance
			iNewTrackPos = ptCursor.x - pListData->m_ptMouseDown_OnHitItem.x - glb_pSkin->mpl_pHScrollBar_Left->m_pImage->m_szSize.cx;
			iNewXOrigin = (int)((((float)iNewTrackPos * (float)(pListData->m_iXScrollExtent - iListRectWidth)) / (float)(iTrackWidth - iTrackThumbWidth)));
			
			if (iNewXOrigin < 0)
				iNewXOrigin = 0;
			else if (iNewXOrigin > (pListData->m_iXScrollExtent - iListRectWidth))
				iNewXOrigin = pListData->m_iXScrollExtent - iListRectWidth;
				
			// Update only if the origin has changed
			if (iNewXOrigin != pListData->m_iXOrigin)
			{
				pListData->m_iXOrigin = iNewXOrigin;
				CLV_UpdateScrollBars(pListData);
				CLV_InvalidateWindow(pListData);
			}
		}
	}
	
	else if (pListData->m_enWindowMode == wmHScrollbar_Scroll_Left)
	{
		// We will draw the button up if the mouse isn't over it
		BOOL bNewButtonState;
		
		if (PtInRect(&pListData->m_rScrollbar_Horiz, ptCursor) == TRUE
				&& ptCursor.x < (pListData->m_rScrollbar_Horiz.left + glb_pSkin->mpl_pHScrollBar_Left->m_pImage->m_szSize.cx))
		{
			bNewButtonState = TRUE;
		}
		
		else
			bNewButtonState = FALSE;
			
		// State changed?
		if (bNewButtonState != pListData->m_bMouseOverScrollbutton)
		{
			pListData->m_bMouseOverScrollbutton = bNewButtonState;
			InvalidateRect(pListData->m_hWnd, &pListData->m_rScrollbar_Horiz, FALSE);
		}
	}
	
	else if (pListData->m_enWindowMode == wmHScrollbar_Scroll_Right)
	{
		// We draw the button up if the mouse isn't over it
		BOOL bNewButtonState;
		
		if (PtInRect(&pListData->m_rScrollbar_Horiz, ptCursor) == TRUE
				&& ptCursor.x > (pListData->m_rScrollbar_Horiz.right - glb_pSkin->mpl_pHScrollBar_Right->m_pImage->m_szSize.cx))
		{
			bNewButtonState = TRUE;
		}
		
		else
			bNewButtonState = FALSE;
			
		// State changed?
		if (bNewButtonState != pListData->m_bMouseOverScrollbutton)
		{
			pListData->m_bMouseOverScrollbutton = bNewButtonState;
			InvalidateRect(pListData->m_hWnd, &pListData->m_rScrollbar_Horiz, FALSE);
		}
	}
	
	else if (pListData->m_enWindowMode == wmVScrollbar_DragThumb)
	{
		if (pListData->m_ptMouseDown.y != ptCursor.y)
		{
			const int iListRectHeight = pListData->m_rList.bottom - pListData->m_rList.top;
			const int iTrackHeight = iListRectHeight - (glb_pSkin->mpl_pVScrollBar_Up->m_iStateHeight
									 + glb_pSkin->mpl_pVScrollBar_Down->m_iStateHeight);
			const int iTrackThumbHeight = pListData->m_rScrollbar_Vert_Thumb.bottom - pListData->m_rScrollbar_Vert_Thumb.top;
			const int iListRectHeight_Lines = CLV_GetListRect_Lines(pListData);
			int iNewTrackPos;
			int iNewFirstVisibleItem;
			
			// Work out new origin based on the track drag distance
			iNewTrackPos = ptCursor.y - pListData->m_ptMouseDown_OnHitItem.y - (pListData->m_rScrollbar_Vert.top + glb_pSkin->mpl_pVScrollBar_Up->m_iStateHeight);
			iNewFirstVisibleItem = (int)((((float)iNewTrackPos * (float)(pListData->m_iNumItems - iListRectHeight_Lines)) / (float)(iTrackHeight - iTrackThumbHeight)));
			
			if (iNewFirstVisibleItem < 0)
				iNewFirstVisibleItem = 0;
				
			if ((iNewFirstVisibleItem + iListRectHeight_Lines) > pListData->m_iNumItems)
				iNewFirstVisibleItem = pListData->m_iNumItems - iListRectHeight_Lines;
				
			// Update only if the origin has changed
			if (iNewFirstVisibleItem != pListData->m_iFirstVisibleItem)
			{
				pListData->m_iFirstVisibleItem = iNewFirstVisibleItem;
				CLV_UpdateScrollBars(pListData);
				CLV_InvalidateWindow(pListData);
			}
		}
	}
	
	else if (pListData->m_enWindowMode == wmVScrollbar_Scroll_Up)
	{
		// We will draw the button up if the mouse isn't over it
		BOOL bNewButtonState;
		
		if (PtInRect(&pListData->m_rScrollbar_Vert, ptCursor) == TRUE
				&& ptCursor.y < (pListData->m_rScrollbar_Vert.top + glb_pSkin->mpl_pVScrollBar_Up->m_iStateHeight))
		{
			bNewButtonState = TRUE;
		}
		
		else
			bNewButtonState = FALSE;
			
		// State changed?
		if (bNewButtonState != pListData->m_bMouseOverScrollbutton)
		{
			pListData->m_bMouseOverScrollbutton = bNewButtonState;
			InvalidateRect(pListData->m_hWnd, &pListData->m_rScrollbar_Vert, FALSE);
		}
	}
	
	else if (pListData->m_enWindowMode == wmVScrollbar_Scroll_Down)
	{
		// We draw the button up if the mouse isn't over it
		BOOL bNewButtonState;
		
		if (PtInRect(&pListData->m_rScrollbar_Vert, ptCursor) == TRUE
				&& ptCursor.y > (pListData->m_rScrollbar_Vert.bottom - glb_pSkin->mpl_pVScrollBar_Down->m_iStateHeight))
		{
			bNewButtonState = TRUE;
		}
		
		else
			bNewButtonState = FALSE;
			
		// State changed?
		if (bNewButtonState != pListData->m_bMouseOverScrollbutton)
		{
			pListData->m_bMouseOverScrollbutton = bNewButtonState;
			InvalidateRect(pListData->m_hWnd, &pListData->m_rScrollbar_Vert, FALSE);
		}
	}
	
	else if (pListData->m_enWindowMode == wmList_Click)
	{
		// If the mouse has moved beyond our drag threshold then enter into header_orderchange mode
		if (pListData->m_iFocusItem != CPC_INVALIDITEM
				&& ((abs(ptCursor.x - pListData->m_ptMouseDown.x) > CPC_LISTDRAGDISTANCE
					 || abs(ptCursor.y - pListData->m_ptMouseDown.y) > CPC_LISTDRAGDISTANCE))
				&& (pListData->m_pItems[pListData->m_iFocusItem].m_dwFlags & CPLV_ITEMFLAG_SELECTED))
		        
		{
			pListData->m_enWindowMode = wmList_Drag;
			
			if (pListData->m_hndlr_ItemDrag && pListData->m_iFocusItem != CPC_INVALIDITEM)
				pListData->m_hndlr_ItemDrag(pListData, pListData->m_iFocusItem, pListData->m_pItems[pListData->m_iFocusItem].m_pItemData);
		}
	}
	
	else
		SetCursor(LoadCursor(NULL, IDC_ARROW));
}

//
//
//
void CLV_StartAutoRepeat(CIs_ListViewData* pListData)
{
	DWORD dwRepeatDelay;
	SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &dwRepeatDelay, 0L);
	dwRepeatDelay = 250 + (dwRepeatDelay * 250);
	pListData->m_bAutoRepeatFirst = TRUE;
	pListData->m_uiAutorepeatTimer = SetTimer(pListData->m_hWnd, CPC_TIMERID_AUTOREPEAT, dwRepeatDelay, NULL);
}

//
//
//
void CLV_Handle_WM_TIMER_AUTOREPEAT(CIs_ListViewData* pListData)
{
	POINT ptCursor;
	GetCursorPos(&ptCursor);
	ScreenToClient(pListData->m_hWnd, &ptCursor);
	
	// Are we autorepeating in the horizontal scrollbar?
	
	if (PtInRect(&pListData->m_rScrollbar_Horiz, ptCursor) == TRUE)
	{
		// In left button
		if (ptCursor.x < (pListData->m_rScrollbar_Horiz.left + glb_pSkin->mpl_pHScrollBar_Left->m_pImage->m_szSize.cx)
				&& pListData->m_enWindowMode == wmHScrollbar_Scroll_Left)
		{
			CLV_Scroll_Horiz(pListData, -CPC_SCROLLBAR_HORIZ_LINESIZE);
		}
		
		// In right button
		
		else if (ptCursor.x > (pListData->m_rScrollbar_Horiz.right - glb_pSkin->mpl_pHScrollBar_Right->m_pImage->m_szSize.cx)
				 && pListData->m_enWindowMode == wmHScrollbar_Scroll_Right)
		{
			CLV_Scroll_Horiz(pListData, CPC_SCROLLBAR_HORIZ_LINESIZE);
		}
		
		// In left page area
		
		else if (ptCursor.x < pListData->m_rScrollbar_Horiz_Thumb.left
				 && pListData->m_enWindowMode == wmHScrollbar_Page_Left)
		{
			CLV_Scroll_Horiz(pListData, -CPC_SCROLLBAR_HORIZ_PAGESIZE);
		}
		
		// In right page area
		
		else if (ptCursor.x > pListData->m_rScrollbar_Horiz_Thumb.right
				 && pListData->m_enWindowMode == wmHScrollbar_Page_Right)
		{
			CLV_Scroll_Horiz(pListData, CPC_SCROLLBAR_HORIZ_PAGESIZE);
		}
	}
	
	// Are we autorepeating in the vertical scrollbar?
	
	else if (PtInRect(&pListData->m_rScrollbar_Vert, ptCursor) == TRUE)
	{
		if (ptCursor.y < (pListData->m_rScrollbar_Vert.top + glb_pSkin->mpl_pVScrollBar_Up->m_iStateHeight)
				&& pListData->m_enWindowMode == wmVScrollbar_Scroll_Up)
		{
			CLV_Scroll_Vert(pListData, -1);
		}
		
		// In down button
		
		else if (ptCursor.y > (pListData->m_rScrollbar_Vert.bottom - glb_pSkin->mpl_pVScrollBar_Down->m_iStateHeight)
				 && pListData->m_enWindowMode == wmVScrollbar_Scroll_Down)
		{
			CLV_Scroll_Vert(pListData, 1);
		}
		
		// In top page area
		
		else if (ptCursor.y < pListData->m_rScrollbar_Vert_Thumb.top
				 && pListData->m_enWindowMode == wmVScrollbar_Page_Up)
		{
			CLV_Scroll_Vert(pListData, -pListData->m_iNumItemsOnPage);
		}
		
		// In bottom page area
		
		else if (ptCursor.y > pListData->m_rScrollbar_Vert_Thumb.bottom
				 && pListData->m_enWindowMode == wmVScrollbar_Page_Down)
		{
			CLV_Scroll_Vert(pListData, pListData->m_iNumItemsOnPage);
		}
	}
	
	// Do we need to update the timer (after the first autorepeat delay)
	
	if (pListData->m_bAutoRepeatFirst)
	{
		int iRepeatDelay;
		
		// Make the autorepeat the same as the keyboard autorepeat
		pListData->m_bAutoRepeatFirst = FALSE;
		SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &iRepeatDelay, 0L);
		iRepeatDelay = 400 - (iRepeatDelay * 12);
		
		if (iRepeatDelay < 0)
			iRepeatDelay = 10;
			
		pListData->m_uiAutorepeatTimer = SetTimer(pListData->m_hWnd, CPC_TIMERID_AUTOREPEAT, iRepeatDelay, NULL);
	}
}

//
//
//
void CLV_InvalidateItem(CP_HLISTVIEW _hListData, const int iItemIDX)
{
	CIs_ListViewData* pListData;
	RECT rItemRect;
	
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	if (iItemIDX == CPC_INVALIDITEM)
		return;
		
	CP_ASSERT(iItemIDX >= 0);
	CP_ASSERT(iItemIDX < pListData->m_iNumItems);
	
	rItemRect.left = pListData->m_rList.left;
	rItemRect.right = pListData->m_rList.right;
	rItemRect.top = pListData->m_rList.top + ((iItemIDX - pListData->m_iFirstVisibleItem) * pListData->m_iItemHeight);
	rItemRect.bottom = rItemRect.top + pListData->m_iItemHeight;
	
	// Handle the possiblity that the previous/next item may be selected
	rItemRect.top -= glb_pSkin->mpl_pSelection->m_szSize.cy - glb_pSkin->mpl_rSelection_Tile.bottom;
	rItemRect.bottom += glb_pSkin->mpl_rSelection_Tile.top;
	
	InvalidateRect(pListData->m_hWnd, &rItemRect, FALSE);
}

//
//
//
void CLV_ItemSelChange(CIs_ListViewData* pListData, int iItemIDX)
{
	if (iItemIDX == CPC_INVALIDITEM)
		return;
		
	CP_ASSERT(iItemIDX >= 0);
	CP_ASSERT(iItemIDX < pListData->m_iNumItems);
	
	if (pListData->m_hndlr_ItemSelected
			&& pListData->m_pItems[iItemIDX].m_dwFlags & CPLV_ITEMFLAG_SELECTED)
	{
		pListData->m_hndlr_ItemSelected(pListData, iItemIDX, pListData->m_pItems[iItemIDX].m_pItemData);
	}
}

//
//
//
void CLV_ActionFocusedItem(CIs_ListViewData* pListData)
{
	if (pListData->m_iFocusItem == CPC_INVALIDITEM)
		return;
		
	CP_ASSERT(pListData->m_iFocusItem >= 0);
	CP_ASSERT(pListData->m_iFocusItem < pListData->m_iNumItems);
	
	CLV_ClearSelection(pListData);
	
	pListData->m_pItems[pListData->m_iFocusItem].m_dwFlags |= CPLV_ITEMFLAG_SELECTED;
	
	CLV_ItemSelChange(pListData, pListData->m_iFocusItem);
	CLV_InvalidateItem(pListData, pListData->m_iFocusItem);
	
	pListData->m_iFocusItem = pListData->m_iFocusItem;
	pListData->m_iKeyboardAnchor = pListData->m_iFocusItem;
	
	if (pListData->m_hndlr_ItemAction)
		pListData->m_hndlr_ItemAction(pListData, pListData->m_iFocusItem, pListData->m_pItems[pListData->m_iFocusItem].m_pItemData);
}

//
//
//
void CLV_SelectRange(CIs_ListViewData* pListData, const int _iFirst, const int _iLast)
{
	int iStartRange;
	int iEndRange;
	int iItemIDX;
	
	if (_iFirst > _iLast)
	{
		iStartRange = _iLast;
		iEndRange = _iFirst;
	}
	
	else
	{
		iStartRange = _iFirst;
		iEndRange = _iLast;
	}
	
	CP_ASSERT(iStartRange >= 0);
	
	CP_ASSERT(iStartRange < pListData->m_iNumItems);
	CP_ASSERT(iEndRange >= 0);
	CP_ASSERT(iEndRange < pListData->m_iNumItems);
	
	for (iItemIDX = iStartRange; iItemIDX <= iEndRange; iItemIDX++)
	{
		pListData->m_pItems[iItemIDX].m_dwFlags |= CPLV_ITEMFLAG_SELECTED;
		CLV_ItemSelChange(pListData, iItemIDX);
		CLV_InvalidateItem(pListData, iItemIDX);
	}
}

//
//
//
void CLV_Handle_WM_LBUTTONDOWN(CIs_ListViewData* pListData, const POINTS _ptCursor, const DWORD dwKeys)
{
	POINT ptCursor;
	
	// Init
	ptCursor.x = _ptCursor.x;
	ptCursor.y = _ptCursor.y;
	pListData->m_ptMouseDown = ptCursor;
	pListData->m_dwMouseDown_Keys = dwKeys;
	
	// Hit test cursor
	// - in header?
	
	if (PtInRect(&pListData->m_rHeader, ptCursor) == TRUE)
	{
		pListData->m_iActiveHeaderCol = CLV_HitTest_HeaderSizer_X(pListData, ptCursor.x);
		
		if (pListData->m_iActiveHeaderCol != CPC_INVALIDCOLUMN)
		{
			// Resizing header
			SetCapture(pListData->m_hWnd);
			pListData->m_enWindowMode = wmHeader_ChangeWidth;
		}
		
		else
		{
			pListData->m_iActiveHeaderCol = CLV_HitTest_Header_X(pListData, ptCursor.x);
			
			if (pListData->m_iActiveHeaderCol != CPC_INVALIDCOLUMN)
			{
				// Resizing header
				SetCapture(pListData->m_hWnd);
				pListData->m_enWindowMode = wmHeader_Click;
				InvalidateRect(pListData->m_hWnd, &pListData->m_rHeader, FALSE);
			}
		}
	}
	
	// - In HScrollbar?
	
	else if (PtInRect(&pListData->m_rScrollbar_Horiz, ptCursor) == TRUE)
	{
		// In Thumb?
		if (PtInRect(&pListData->m_rScrollbar_Horiz_Thumb, ptCursor) == TRUE)
		{
			// Dragging thumb
			SetCapture(pListData->m_hWnd);
			pListData->m_enWindowMode = wmHScrollbar_DragThumb;
			pListData->m_ptMouseDown_OnHitItem.x = ptCursor.x - pListData->m_rScrollbar_Horiz_Thumb.left;
			pListData->m_ptMouseDown_OnHitItem.y = ptCursor.y - pListData->m_rScrollbar_Horiz_Thumb.top;
			InvalidateRect(pListData->m_hWnd, &pListData->m_rScrollbar_Horiz_Thumb, FALSE);
		}
		
		// In left button
		
		else if (ptCursor.x < (pListData->m_rScrollbar_Horiz.left + glb_pSkin->mpl_pHScrollBar_Left->m_pImage->m_szSize.cx))
		{
			SetCapture(pListData->m_hWnd);
			pListData->m_enWindowMode = wmHScrollbar_Scroll_Left;
			CLV_Scroll_Horiz(pListData, -CPC_SCROLLBAR_HORIZ_LINESIZE);
			InvalidateRect(pListData->m_hWnd, &pListData->m_rScrollbar_Horiz, FALSE);
			pListData->m_bMouseOverScrollbutton = TRUE;
			CLV_StartAutoRepeat(pListData);
		}
		
		// In right button
		
		else if (ptCursor.x > (pListData->m_rScrollbar_Horiz.right - glb_pSkin->mpl_pHScrollBar_Right->m_pImage->m_szSize.cx))
		{
			SetCapture(pListData->m_hWnd);
			pListData->m_enWindowMode = wmHScrollbar_Scroll_Right;
			CLV_Scroll_Horiz(pListData, CPC_SCROLLBAR_HORIZ_LINESIZE);
			InvalidateRect(pListData->m_hWnd, &pListData->m_rScrollbar_Horiz, FALSE);
			pListData->m_bMouseOverScrollbutton = TRUE;
			CLV_StartAutoRepeat(pListData);
		}
		
		// In left page area
		
		else if (ptCursor.x < pListData->m_rScrollbar_Horiz_Thumb.left)
		{
			SetCapture(pListData->m_hWnd);
			pListData->m_enWindowMode = wmHScrollbar_Page_Left;
			CLV_Scroll_Horiz(pListData, -CPC_SCROLLBAR_HORIZ_PAGESIZE);
			InvalidateRect(pListData->m_hWnd, &pListData->m_rScrollbar_Horiz, FALSE);
			CLV_StartAutoRepeat(pListData);
		}
		
		// In right page area
		
		else if (ptCursor.x > pListData->m_rScrollbar_Horiz_Thumb.right)
		{
			SetCapture(pListData->m_hWnd);
			pListData->m_enWindowMode = wmHScrollbar_Page_Right;
			CLV_Scroll_Horiz(pListData, CPC_SCROLLBAR_HORIZ_PAGESIZE);
			InvalidateRect(pListData->m_hWnd, &pListData->m_rScrollbar_Horiz, FALSE);
			CLV_StartAutoRepeat(pListData);
		}
	}
	
	// - In VScrollbar?
	
	else if (PtInRect(&pListData->m_rScrollbar_Vert, ptCursor) == TRUE)
	{
		// In Thumb?
		if (PtInRect(&pListData->m_rScrollbar_Vert_Thumb, ptCursor) == TRUE)
		{
			// Dragging thumb
			SetCapture(pListData->m_hWnd);
			pListData->m_enWindowMode = wmVScrollbar_DragThumb;
			pListData->m_ptMouseDown_OnHitItem.x = ptCursor.x - pListData->m_rScrollbar_Vert_Thumb.left;
			pListData->m_ptMouseDown_OnHitItem.y = ptCursor.y - pListData->m_rScrollbar_Vert_Thumb.top;
			InvalidateRect(pListData->m_hWnd, &pListData->m_rScrollbar_Vert_Thumb, FALSE);
		}
		
		// In up button
		
		else if (ptCursor.y < (pListData->m_rScrollbar_Vert.top + glb_pSkin->mpl_pVScrollBar_Up->m_iStateHeight))
		{
			SetCapture(pListData->m_hWnd);
			pListData->m_enWindowMode = wmVScrollbar_Scroll_Up;
			CLV_Scroll_Vert(pListData, -1);
			InvalidateRect(pListData->m_hWnd, &pListData->m_rScrollbar_Vert, FALSE);
			pListData->m_bMouseOverScrollbutton = TRUE;
			CLV_StartAutoRepeat(pListData);
		}
		
		// In down button
		
		else if (ptCursor.y > (pListData->m_rScrollbar_Vert.bottom - glb_pSkin->mpl_pVScrollBar_Down->m_iStateHeight))
		{
			SetCapture(pListData->m_hWnd);
			pListData->m_enWindowMode = wmVScrollbar_Scroll_Down;
			CLV_Scroll_Vert(pListData, 1);
			InvalidateRect(pListData->m_hWnd, &pListData->m_rScrollbar_Vert, FALSE);
			pListData->m_bMouseOverScrollbutton = TRUE;
			CLV_StartAutoRepeat(pListData);
		}
		
		// In top page area
		
		else if (ptCursor.y < pListData->m_rScrollbar_Vert_Thumb.top)
		{
			SetCapture(pListData->m_hWnd);
			pListData->m_enWindowMode = wmVScrollbar_Page_Up;
			CLV_Scroll_Vert(pListData, -pListData->m_iNumItemsOnPage);
			InvalidateRect(pListData->m_hWnd, &pListData->m_rScrollbar_Vert, FALSE);
			CLV_StartAutoRepeat(pListData);
		}
		
		// In bottom page area
		
		else if (ptCursor.y > pListData->m_rScrollbar_Vert_Thumb.bottom)
		{
			SetCapture(pListData->m_hWnd);
			pListData->m_enWindowMode = wmVScrollbar_Page_Down;
			CLV_Scroll_Vert(pListData, pListData->m_iNumItemsOnPage);
			InvalidateRect(pListData->m_hWnd, &pListData->m_rScrollbar_Vert, FALSE);
			CLV_StartAutoRepeat(pListData);
		}
	}
	
	// In list
	
	if (PtInRect(&pListData->m_rList, ptCursor) == TRUE)
	{
		pListData->m_enWindowMode = wmList_Click;
		
		// Get the item that we have clicked
		pListData->m_iClickedItem = CLV_YOffsetToItem(pListData, ptCursor.y);
		
		if (pListData->m_iClickedItem < 0 || pListData->m_iClickedItem >= pListData->m_iNumItems)
			pListData->m_iClickedItem = CPC_INVALIDITEM;
			
		// If shift is down - select a range
		if ((dwKeys & MK_SHIFT))
		{
			int iCorrectedClickedItem;
			
			// If CTRL isn't down - clear the selection
			
			if ((dwKeys & MK_CONTROL) == 0)
				CLV_ClearSelection(pListData);
				
			iCorrectedClickedItem = pListData->m_iClickedItem;
			
			if (iCorrectedClickedItem >= pListData->m_iNumItems || iCorrectedClickedItem == CPC_INVALIDITEM)
				iCorrectedClickedItem = pListData->m_iNumItems - 1;
				
			if (pListData->m_iKeyboardAnchor == CPC_INVALIDITEM)
				pListData->m_iKeyboardAnchor = pListData->m_iFocusItem;
				
			if (pListData->m_iKeyboardAnchor == CPC_INVALIDITEM)
				pListData->m_iKeyboardAnchor = iCorrectedClickedItem;
				
				
			CLV_SelectRange(pListData, pListData->m_iKeyboardAnchor, iCorrectedClickedItem);
			
			pListData->m_iFocusItem = iCorrectedClickedItem;
			
			CLV_InvalidateItem(pListData, pListData->m_iFocusItem);
		}
		
		else
		{
			CLV_InvalidateItem(pListData, pListData->m_iFocusItem);
			pListData->m_iFocusItem = pListData->m_iClickedItem;
			CLV_InvalidateItem(pListData, pListData->m_iFocusItem);
			pListData->m_iKeyboardAnchor = pListData->m_iFocusItem;
		}
		
		// ... further processing on mouse up
	}
}

//
//
//
void CLV_Handle_WM_LBUTTONDBLCLK(CIs_ListViewData* pListData, const POINTS _ptCursor)
{
	POINT ptCursor;
	
	// Init
	ptCursor.x = _ptCursor.x;
	ptCursor.y = _ptCursor.y;
	pListData->m_ptMouseDown = ptCursor;
	
	// Hit test cursor
	// - in header?
	
	if (PtInRect(&pListData->m_rHeader, ptCursor) == TRUE)
	{
		// A double click in the resize area?
		int iHitTestColIDX = CLV_HitTest_HeaderSizer_X(pListData, ptCursor.x);
		
		if (iHitTestColIDX != CPC_INVALIDCOLUMN)
		{
			int iMinWidth = CPC_HEADERDRAG_HTWIDTH << 1;
			int iItemIDX;
			HDC dcWindow;
			SIZE szText;
			HFONT hfOld;
			
			// Resize column according to it's contents
			dcWindow = GetDC(pListData->m_hWnd);
			hfOld = (HFONT)SelectObject(dcWindow, glb_pSkin->mpl_hfFont);
			
			// Ensure col is big enough for the header
			
			if (pListData->m_pColumns[iHitTestColIDX].m_pColumnText)
			{
				GetTextExtentPoint(dcWindow,
								   pListData->m_pColumns[iHitTestColIDX].m_pColumnText,
								   strlen(pListData->m_pColumns[iHitTestColIDX].m_pColumnText),
								   &szText);
				szText.cx += glb_pSkin->mpl_rListHeader_SourceTile.left
							 + (glb_pSkin->mpl_pListHeader_Up->m_szSize.cx - glb_pSkin->mpl_rListHeader_SourceTile.right);
				             
				if (szText.cx > iMinWidth)
					iMinWidth = szText.cx;
			}
			
			// Go through all the items getting their widths
			
			if (pListData->m_pColumns[iHitTestColIDX].m_pfnTextAccessor)
			{
				for (iItemIDX = 0; iItemIDX < pListData->m_iNumItems; iItemIDX++)
				{
					const char* pcText = pListData->m_pColumns[iHitTestColIDX].m_pfnTextAccessor(pListData->m_pItems[iItemIDX].m_pItemData);
					
					if (pcText)
					{
						GetTextExtentPoint(dcWindow,
										   pcText, strlen(pcText),
										   &szText);
						szText.cx += glb_pSkin->mpl_rListHeader_SourceTile.left
									 + (glb_pSkin->mpl_pListHeader_Down->m_szSize.cx - glb_pSkin->mpl_rListHeader_SourceTile.right);
						             
						if (szText.cx > iMinWidth)
							iMinWidth = szText.cx;
					}
				}
			}
			
			SelectObject(dcWindow, hfOld);
			
			ReleaseDC(pListData->m_hWnd, dcWindow);
			
			// Update column width
			pListData->m_pColumns[iHitTestColIDX].m_iColumnWidth = iMinWidth;
			CLV_InvalidateWindow(pListData);
			CLV_UpdateScrollBars(pListData);
			SetCursor(LoadCursor(NULL, IDC_ARROW)); // In a header
			
			// Send notify
			
			if (pListData->m_hndlr_HeaderChanged)
				pListData->m_hndlr_HeaderChanged(pListData);
		}
	}
	
	else if (PtInRect(&pListData->m_rList, ptCursor) == TRUE)
	{
		CLV_ActionFocusedItem(pListData);
	}
}

//
//
//
void CLV_Handle_WM_RBUTTONDOWN(CIs_ListViewData* pListData, const POINTS _ptCursor)
{
	POINT ptCursor;
	
	// Init
	ptCursor.x = _ptCursor.x;
	ptCursor.y = _ptCursor.y;
	
	// Hit test cursor
	// - in header?
	
	if (PtInRect(&pListData->m_rHeader, ptCursor) == TRUE)
	{
		HMENU hmPopup;
		POINT ptMouse_Screen;
		UINT uiMenuResult;
		unsigned int _iColIDX;
		
		// Show a popup menu of all the columns so that the visibility can be affected
		hmPopup = CreatePopupMenu();
		
		// Build menu
		
		for (_iColIDX = 0; _iColIDX < pListData->m_iNumColumns; _iColIDX++)
		{
			UINT uiMenuFlags;
			unsigned int iColumnIDX = pListData->m_piColumnOrder[_iColIDX];
			
			// Skip if there is no text on this column (or it's not hidable)
			
			if (pListData->m_pColumns[iColumnIDX].m_pColumnText == NULL
					|| pListData->m_pColumns[iColumnIDX].m_dwFlags & CPLV_COLFLAG_NOHIDE)
			{
				continue;
			}
			
			if (pListData->m_pColumns[iColumnIDX].m_dwFlags & CPLV_COLFLAG_HIDDEN)
				uiMenuFlags = 0;
			else
				uiMenuFlags = MF_CHECKED;
				
			AppendMenu(hmPopup, MF_STRING | uiMenuFlags, iColumnIDX + 1, pListData->m_pColumns[iColumnIDX].m_pColumnText);
		}
		
		// Show menu
		ptMouse_Screen = ptCursor;
		ClientToScreen(pListData->m_hWnd, &ptMouse_Screen);

		uiMenuResult = TrackPopupMenu(hmPopup,
									  TPM_NONOTIFY | TPM_RETURNCMD,
									  ptMouse_Screen.x, ptMouse_Screen.y, 0,
									  pListData->m_hWnd, NULL);
		                              
		// If there was a result - toggle the visible state of the item in question
		if (uiMenuResult != 0)
		{
			pListData->m_pColumns[uiMenuResult-1].m_dwFlags ^= CPLV_COLFLAG_HIDDEN;
			CLV_UpdateScrollBars(pListData);
			CLV_InvalidateWindow(pListData);
			
			// Send notify
			
			if (pListData->m_hndlr_HeaderChanged)
				pListData->m_hndlr_HeaderChanged(pListData);
		}
		
		DestroyMenu(hmPopup);
	}
	
	else if (PtInRect(&pListData->m_rList, ptCursor) == TRUE)
	{
		int iItemIDX;
		int iColumnIDX;
		
		// Perform hit test
		iItemIDX = CLV_YOffsetToItem(pListData, ptCursor.y);
		iColumnIDX = CLV_HitTest_Header_X(pListData, ptCursor.x);
		
		if (iItemIDX < 0 || iItemIDX >= pListData->m_iNumItems)
			iItemIDX = CPC_INVALIDITEM;
			
		if (iItemIDX != CPC_INVALIDITEM && iColumnIDX != CPC_INVALIDCOLUMN)
		{
			if (pListData->m_iFocusItem != iItemIDX)
				CLV_SetFocusItem(pListData, iItemIDX);
				
			// If the hit item isn't selected - clear selection (and select this item)
			if ((pListData->m_pItems[iItemIDX].m_dwFlags & CPLV_ITEMFLAG_SELECTED) == 0)
			{
				CLV_ClearSelection(pListData);
				pListData->m_pItems[iItemIDX].m_dwFlags |= CPLV_ITEMFLAG_SELECTED;
				CLV_InvalidateItem(pListData, iItemIDX);
				CLV_ItemSelChange(pListData, iItemIDX);
			}
			
			// Send notify
			
			if (pListData->m_hndlr_ItemRightClick)
				pListData->m_hndlr_ItemRightClick(pListData, iItemIDX, iColumnIDX, pListData->m_pItems[iItemIDX].m_pItemData);
		}
		
		else
			CLV_ClearSelection(pListData);
	}
	
}

//
//
//
void CLV_Handle_WM_LBUTTONUP(CIs_ListViewData* pListData, const POINTS _ptCursor)
{
	if (pListData->m_enWindowMode != wmQuiescent)
		ReleaseCapture();
		
	// Invalidate header if we are in click or drag mode (because the header item should now be draw "up)
	if (pListData->m_enWindowMode == wmHeader_Click || pListData->m_enWindowMode == wmHeader_ChangeOrder)
		InvalidateRect(pListData->m_hWnd, &pListData->m_rHeader, FALSE);
		
	// Invalidate thumb if we were dragging it
	else if (pListData->m_enWindowMode == wmHScrollbar_DragThumb)
		InvalidateRect(pListData->m_hWnd, &pListData->m_rScrollbar_Horiz_Thumb, FALSE);
	else if (pListData->m_enWindowMode == wmHScrollbar_Scroll_Left || pListData->m_enWindowMode == wmHScrollbar_Scroll_Right)
		InvalidateRect(pListData->m_hWnd, &pListData->m_rScrollbar_Horiz, FALSE);
	else if (pListData->m_enWindowMode == wmVScrollbar_DragThumb)
		InvalidateRect(pListData->m_hWnd, &pListData->m_rScrollbar_Vert_Thumb, FALSE);
	else if (pListData->m_enWindowMode == wmVScrollbar_Scroll_Up || pListData->m_enWindowMode == wmVScrollbar_Scroll_Down)
		InvalidateRect(pListData->m_hWnd, &pListData->m_rScrollbar_Vert, FALSE);
		
	// Clear the autorepeat timer if we need to
	if (pListData->m_uiAutorepeatTimer)
	{
		KillTimer(pListData->m_hWnd, pListData->m_uiAutorepeatTimer);
		pListData->m_uiAutorepeatTimer = 0;
	}
	
	// Send header change notify if we need to
	
	if (pListData->m_hndlr_HeaderChanged
			&& (pListData->m_enWindowMode == wmHeader_ChangeOrder || pListData->m_enWindowMode == wmHeader_ChangeWidth))
	{
		pListData->m_hndlr_HeaderChanged(pListData);
	}
	
	// Send col header click if we need to
	
	if (pListData->m_enWindowMode == wmHeader_Click && pListData->m_hndlr_ColHeaderClick)
		pListData->m_hndlr_ColHeaderClick(pListData, pListData->m_iActiveHeaderCol);
		
	// If a list item was clicked then update selection
	if (pListData->m_enWindowMode == wmList_Click
			&& pListData->m_iClickedItem != CPC_INVALIDITEM
			&& pListData->m_iClickedItem < pListData->m_iNumItems
			&& (pListData->m_dwMouseDown_Keys & MK_SHIFT) == 0)
	{
		// If CTRL isn't down - clear the selection
		if ((pListData->m_dwMouseDown_Keys & MK_CONTROL) == 0)
			CLV_ClearSelection(pListData);
			
		// Toggle the selection of the clicked item
		pListData->m_pItems[pListData->m_iClickedItem].m_dwFlags ^= CPLV_ITEMFLAG_SELECTED;
		
		CLV_ItemSelChange(pListData, pListData->m_iClickedItem);
		
		CLV_InvalidateItem(pListData, pListData->m_iClickedItem);
	}
	
	// Reset window mode
	pListData->m_enWindowMode = wmQuiescent;
}

//
//
//
void CLV_Handle_WM_KEYDOWN(CIs_ListViewData* pListData, const int iVKeyCode)
{
	const BOOL bAltIsDown = (GetAsyncKeyState(VK_MENU)  & 0x8000) ? TRUE : FALSE;
	const BOOL bCtrlIsDown = (GetAsyncKeyState(VK_CONTROL)  & 0x8000) ? TRUE : FALSE;
	const BOOL bShiftIsDown = (GetAsyncKeyState(VK_SHIFT)  & 0x8000) ? TRUE : FALSE;
	
	switch (iVKeyCode)
	{
	
		case VK_UP:
		
		case VK_DOWN:
		
		case VK_PRIOR:
		
		case VK_NEXT:
		
		case VK_HOME:
		
		case VK_END:
		
			if (bAltIsDown == FALSE)
			{
				if (pListData->m_iNumItems > 0)
				{
					int iNewFocusItemIDX = pListData->m_iFocusItem;
					
					if (iNewFocusItemIDX == CPC_INVALIDITEM)
						iNewFocusItemIDX = 0;
						
					if (iVKeyCode == VK_UP)
						iNewFocusItemIDX--;
					else if (iVKeyCode == VK_DOWN)
						iNewFocusItemIDX++;
					else if (iVKeyCode == VK_PRIOR)
						iNewFocusItemIDX -= pListData->m_iNumItemsOnPage;
					else if (iVKeyCode == VK_NEXT)
						iNewFocusItemIDX += pListData->m_iNumItemsOnPage;
					else if (iVKeyCode == VK_HOME)
						iNewFocusItemIDX = 0;
					else if (iVKeyCode == VK_END)
						iNewFocusItemIDX = pListData->m_iNumItems - 1;
						
					if (iNewFocusItemIDX < 0)
						iNewFocusItemIDX = 0;
					else if (iNewFocusItemIDX >= pListData->m_iNumItems)
						iNewFocusItemIDX = pListData->m_iNumItems - 1;
						
					if (iNewFocusItemIDX != pListData->m_iFocusItem)
					{
						// Update selection
						if (bCtrlIsDown == FALSE)
						{
							CLV_ClearSelection(pListData);
							pListData->m_pItems[iNewFocusItemIDX].m_dwFlags |= CPLV_ITEMFLAG_SELECTED;
							CLV_ItemSelChange(pListData, iNewFocusItemIDX);
						}
						
						if (bShiftIsDown)
						{
							if (pListData->m_iKeyboardAnchor == CPC_INVALIDITEM)
							{
								if (pListData->m_iFocusItem != CPC_INVALIDITEM)
									pListData->m_iKeyboardAnchor = pListData->m_iFocusItem;
								else
									pListData->m_iKeyboardAnchor = iNewFocusItemIDX;
							}
							
							if (pListData->m_iFocusItem != iNewFocusItemIDX)
								CLV_InvalidateItem(pListData, pListData->m_iFocusItem);
								
							pListData->m_iFocusItem = iNewFocusItemIDX;
							
							CLV_EnsureVisible(pListData, iNewFocusItemIDX);
							CLV_SelectRange(pListData, pListData->m_iKeyboardAnchor, pListData->m_iFocusItem);
						}
						
						else
						{
							CLV_InvalidateItem(pListData, pListData->m_iFocusItem);
							pListData->m_iFocusItem = iNewFocusItemIDX;
							CLV_InvalidateItem(pListData, iNewFocusItemIDX);
							CLV_EnsureVisible(pListData, iNewFocusItemIDX);
							pListData->m_iKeyboardAnchor = pListData->m_iFocusItem;
						}
					}
					
					return;
				}
			}
			
			break;
			
		case VK_SPACE:
		
			if (pListData->m_iFocusItem != CPC_INVALIDITEM)
			{
				if (bCtrlIsDown == TRUE)
					pListData->m_pItems[pListData->m_iFocusItem].m_dwFlags ^= CPLV_ITEMFLAG_SELECTED;
				else
					pListData->m_pItems[pListData->m_iFocusItem].m_dwFlags |= CPLV_ITEMFLAG_SELECTED;
					
				CLV_ItemSelChange(pListData, pListData->m_iFocusItem);
				CLV_InvalidateItem(pListData, pListData->m_iFocusItem);
				CLV_EnsureVisible(pListData, pListData->m_iFocusItem);
			}
			
			return;
			
		case 'A':
		
			if (bCtrlIsDown == TRUE && pListData->m_iNumItems > 0)
			{
				CLV_SelectRange(pListData, 0, pListData->m_iNumItems - 1);
				return;
			}
			
			break;
			
		case VK_RETURN:
			CLV_ActionFocusedItem(pListData);
			return;
	}
	
	// Default processing to parent
	
	if (pListData->m_hndlr_UnhandledKeyPress)
		pListData->m_hndlr_UnhandledKeyPress(pListData, iVKeyCode, bAltIsDown, bCtrlIsDown, bShiftIsDown);
}

//
//
//
LRESULT CALLBACK exp_ListViewWindowProc(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
	CIs_ListViewData* pListData;
	
	// Get the window's data object
	
	if (uiMessage == WM_NCCREATE)
	{
		pListData = (CIs_ListViewData*)((CREATESTRUCT*)lParam)->lpCreateParams;
		pListData->m_hWnd = hWnd;
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)pListData);
	}
	
	else
		pListData = (CIs_ListViewData*)GetWindowLong(hWnd, GWL_USERDATA);
		
	CP_CHECKOBJECT(pListData);
	
	// Message handlers
	switch (uiMessage)
	{
	
		case WM_CREATE:
		{
			CREATESTRUCT* pCS = (CREATESTRUCT*)lParam;
			CLV_CalcItemHeight(pListData);
			CLV_UpdateWindowDims(pListData, pCS->cx, pCS->cy);
		}
		
		break;
		
		case WM_NCDESTROY:
			CLV_CleanupWindowData(pListData);
			break;
			
		case WM_PAINT:
			CLV_Handle_WM_PAINT(pListData);
			return 0;
			
		case WM_NCCALCSIZE:
			// We do not wish to have any window area lost to captions etc (also prevent the system
			// from preserving our window contents during a resize
			return WVR_REDRAW;
			
		case WM_WINDOWPOSCHANGED:
		{
			const WINDOWPOS* pWP = (const WINDOWPOS*)lParam;
			CLV_UpdateWindowDims(pListData, pWP->cx, pWP->cy);
		}
		
		return 0;
		
		case WM_DROPFILES:
			return SendMessage(GetParent(pListData->m_hWnd), WM_DROPFILES, wParam, lParam);
			
		case WM_MOUSEMOVE:
			CLV_Handle_WM_MOUSEMOVE(pListData, MAKEPOINTS(lParam));
			return 0;
			
		case WM_SYSKEYDOWN:
		
		case WM_KEYDOWN:
			CLV_Handle_WM_KEYDOWN(pListData, (int)wParam);
			return 0;
			
		case WM_LBUTTONDOWN:
			SetFocus(pListData->m_hWnd);
			CLV_Handle_WM_LBUTTONDOWN(pListData, MAKEPOINTS(lParam), (DWORD)wParam);
			return 0;
			
		case WM_LBUTTONUP:
			CLV_Handle_WM_LBUTTONUP(pListData, MAKEPOINTS(lParam));
			return 0;
			
		case WM_LBUTTONDBLCLK:
			CLV_Handle_WM_LBUTTONDBLCLK(pListData, MAKEPOINTS(lParam));
			return 0;
			
		case WM_RBUTTONDOWN:
			SetFocus(pListData->m_hWnd);
			CLV_Handle_WM_RBUTTONDOWN(pListData, MAKEPOINTS(lParam));
			return 0;
			
		case WM_MOUSEWHEEL:
		
			if ((short)HIWORD(wParam) > 0)
				CLV_Scroll_Vert(pListData, -CPC_SCROLLBAR_MOUSEWHEELAMOUNT);
			else
				CLV_Scroll_Vert(pListData, CPC_SCROLLBAR_MOUSEWHEELAMOUNT);
				
			return 0;
			
		case WM_SETFOCUS:
			pListData->m_bHasFocus = TRUE;
			
			if (pListData->m_iFocusItem != CPC_INVALIDITEM)
				CLV_InvalidateItem(pListData, pListData->m_iFocusItem);
				
			return 0;
			
		case WM_KILLFOCUS:
			pListData->m_bHasFocus = FALSE;
			
			if (pListData->m_iFocusItem != CPC_INVALIDITEM)
				CLV_InvalidateItem(pListData, pListData->m_iFocusItem);
				
			return 0;
			
		case WM_TIMER:
			if (wParam == pListData->m_uiAutorepeatTimer)
				CLV_Handle_WM_TIMER_AUTOREPEAT(pListData);
				
			return 0;
	}
	
	return DefWindowProc(hWnd, uiMessage, wParam, lParam);
}

//
//
//
void CLV_AddColumn(CP_HLISTVIEW _hListData, const char* pcTitle, const int iWidth, wp_GetItemText pfnItemTextAccessor, const DWORD dwFlags)
{
	CIs_ListViewData* pListData;
	CIs_ListView_Column* pNewColumn;
	unsigned int iNewColumnIDX;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	// Allocate a new column
	iNewColumnIDX = pListData->m_iNumColumns;
	pListData->m_iNumColumns++;
	pListData->m_pColumns = (CIs_ListView_Column*)realloc(pListData->m_pColumns, pListData->m_iNumColumns * sizeof(CIs_ListView_Column));
	pListData->m_piColumnOrder = (unsigned int*)realloc(pListData->m_piColumnOrder, pListData->m_iNumColumns * sizeof(unsigned int));
	pListData->m_piColumnOrder[iNewColumnIDX] = iNewColumnIDX;
	pNewColumn = pListData->m_pColumns + iNewColumnIDX;
	
	// Setup it's data
	STR_AllocSetString(&pNewColumn->m_pColumnText, pcTitle, FALSE);
	pNewColumn->m_iColumnWidth = iWidth;
	pNewColumn->m_dwFlags = dwFlags;
	pNewColumn->m_pfnTextAccessor = pfnItemTextAccessor;
	pNewColumn->m_pfnGetCustomDrawColour = NULL;
	pNewColumn->m_enAlign = lcaLeft;
	
	// Cleanup
	CLV_InvalidateWindow(pListData);
	CLV_UpdateScrollBars(pListData);
}

//
//
//
void CLV_SetColumnCustomDrawColour(CP_HLISTVIEW _hListData, const int iColumnIDX, wp_GetItemDrawColour pfnGetCustomDrawColour)
{
	CIs_ListViewData* pListData;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	pListData->m_pColumns[iColumnIDX].m_pfnGetCustomDrawColour = pfnGetCustomDrawColour;
}

//
//
//
void CLV_SetColumnAlign(CP_HLISTVIEW _hListData, const int iColumnIDX, const CPe_ListColumnAlign enNewAlign)
{
	CIs_ListViewData* pListData;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	pListData->m_pColumns[iColumnIDX].m_enAlign = enNewAlign;
}

//
//
//
int CLV_AddItem(CP_HLISTVIEW _hListData, const void* pvItemData)
{
	CIs_ListViewData* pListData;
	int iNewItemIDX;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	// Do we need a bigger buffer?
	
	if (pListData->m_iNumItemsInBuffer == pListData->m_iNumItems)
	{
		pListData->m_iNumItemsInBuffer += CPC_BUFFERQUANTISATION;
		pListData->m_pItems = (CIs_ListView_Item*)realloc(pListData->m_pItems, pListData->m_iNumItemsInBuffer * sizeof(CIs_ListView_Item));
	}
	
	// Actually add the item
	iNewItemIDX = pListData->m_iNumItems;
	
	pListData->m_iNumItems++;
	pListData->m_pItems[iNewItemIDX].m_dwFlags = CPLV_ITEMFLAG_NONE;
	pListData->m_pItems[iNewItemIDX].m_pItemData = pvItemData;
	
	// Update the display
	if (pListData->m_bInBatch == FALSE)
	{
		CLV_UpdateScrollBars(pListData);
		CLV_InvalidateWindow(pListData);
	}
	
	return iNewItemIDX;
}

//
//
//
void CLV_RemoveAllItems(CP_HLISTVIEW _hListData)
{
	CIs_ListViewData* pListData;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	CLV_EmptyItems(pListData);
	
	if (pListData->m_bInBatch == FALSE)
	{
		CLV_UpdateScrollBars(pListData);
		CLV_InvalidateWindow(pListData);
	}
	
	pListData->m_iFocusItem = CPC_INVALIDITEM;
	pListData->m_iKeyboardAnchor = CPC_INVALIDITEM;
}

//
//
//
int CLV_GetFocusItem(CP_HLISTVIEW _hListData)
{
	CIs_ListViewData* pListData;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	return pListData->m_iFocusItem;
}

//
//
//
void CLV_SetFocusItem(CP_HLISTVIEW _hListData, int iNewItemIDX)
{
	CIs_ListViewData* pListData;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	if (pListData->m_iFocusItem != CPC_INVALIDITEM)
		CLV_InvalidateItem(pListData, pListData->m_iFocusItem);
		
	pListData->m_iFocusItem = iNewItemIDX;
	
	if (pListData->m_iFocusItem != CPC_INVALIDITEM)
		CLV_InvalidateItem(pListData, pListData->m_iFocusItem);
}

//
//
//
void CLV_sethandler_DrawBackgroundRect(CP_HLISTVIEW _hListData, wp_DrawBackgroundRect pfnHandler)
{
	CIs_ListViewData* pListData;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	pListData->m_hndlr_DrawBackgroundRect = pfnHandler;
}

//
//
//
void CLV_sethandler_HeaderChanged(CP_HLISTVIEW _hListData, wp_HeaderChanged pfnHandler)
{
	CIs_ListViewData* pListData;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	pListData->m_hndlr_HeaderChanged = pfnHandler;
}

//
//
//
void CLV_sethandler_ItemSelected(CP_HLISTVIEW _hListData, wp_ItemCallback pfnHandler)
{
	CIs_ListViewData* pListData;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	pListData->m_hndlr_ItemSelected = pfnHandler;
}

//
//
//
void CLV_sethandler_ItemAction(CP_HLISTVIEW _hListData, wp_ItemCallback pfnHandler)
{
	CIs_ListViewData* pListData;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	pListData->m_hndlr_ItemAction = pfnHandler;
}

//
//
//
void CLV_sethandler_ItemDrag(CP_HLISTVIEW _hListData, wp_ItemCallback pfnHandler)
{
	CIs_ListViewData* pListData;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	pListData->m_hndlr_ItemDrag = pfnHandler;
}

//
//
//
void CLV_sethandler_ItemRightClick(CP_HLISTVIEW _hListData, wp_ItemSubCallback pfnHandler)
{
	CIs_ListViewData* pListData;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	pListData->m_hndlr_ItemRightClick = pfnHandler;
}

//
//
//
void CLV_sethandler_ColHeaderClick(CP_HLISTVIEW _hListData, wp_ColHeaderClick pfnHandler)
{
	CIs_ListViewData* pListData;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	pListData->m_hndlr_ColHeaderClick = pfnHandler;
}

//
//
//
void CLV_sethandler_UnhandledKeyPress(CP_HLISTVIEW _hListData, wp_UnhandledKeyPress pfnHandler)
{
	CIs_ListViewData* pListData;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	pListData->m_hndlr_UnhandledKeyPress = pfnHandler;
}

//
//
//
void CLV_GetColumnOrder(CP_HLISTVIEW _hListData, unsigned int* pOrder, const unsigned int iNumColumnsInArray)
{
	CIs_ListViewData* pListData;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	CP_ASSERT(iNumColumnsInArray == pListData->m_iNumColumns);
	
	memcpy(pOrder, pListData->m_piColumnOrder, pListData->m_iNumColumns * sizeof(*pListData->m_piColumnOrder));
}

//
//
//
void CLV_SetColumnOrder(CP_HLISTVIEW _hListData, const unsigned int* pOrder, const unsigned int iNumColumnsInArray)
{
	CIs_ListViewData* pListData;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	CP_ASSERT(iNumColumnsInArray == pListData->m_iNumColumns);
	
	memcpy(pListData->m_piColumnOrder, pOrder, pListData->m_iNumColumns * sizeof(*pListData->m_piColumnOrder));
}

//
//
//
void CLV_GetColumnVisibleState(CP_HLISTVIEW _hListData, BOOL* pStates, const unsigned int iNumColumnsInArray)
{
	CIs_ListViewData* pListData;
	unsigned int iColIDX;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	CP_ASSERT(iNumColumnsInArray == pListData->m_iNumColumns);
	
	for (iColIDX = 0; iColIDX < pListData->m_iNumColumns; iColIDX++)
		pStates[iColIDX] = (pListData->m_pColumns[iColIDX].m_dwFlags & CPLV_COLFLAG_HIDDEN) ? FALSE : TRUE;
}

//
//
//
void CLV_GetColumnWidths(CP_HLISTVIEW _hListData, int* pWidths, const unsigned int iNumColumnsInArray)
{
	CIs_ListViewData* pListData;
	unsigned int iColIDX;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	CP_ASSERT(iNumColumnsInArray == pListData->m_iNumColumns);
	
	for (iColIDX = 0; iColIDX < pListData->m_iNumColumns; iColIDX++)
		pWidths[iColIDX] = pListData->m_pColumns[iColIDX].m_iColumnWidth;
}

//
//
//
void CLV_ClearSelection(CP_HLISTVIEW _hListData)
{
	CIs_ListViewData* pListData;
	int iItemIDX;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	for (iItemIDX = 0; iItemIDX < pListData->m_iNumItems; iItemIDX++)
	{
		if (pListData->m_bInBatch == FALSE && pListData->m_pItems[iItemIDX].m_dwFlags & CPLV_ITEMFLAG_SELECTED)
			CLV_InvalidateItem(pListData, iItemIDX);
			
		pListData->m_pItems[iItemIDX].m_dwFlags &= ~CPLV_ITEMFLAG_SELECTED;
		
		CLV_ItemSelChange(pListData, iItemIDX);
	}
}

//
//
//
int CLV_GetNextSelectedItem(CP_HLISTVIEW _hListData, const int _iSearchStart)
{
	CIs_ListViewData* pListData;
	int iItemIDX;
	int iSearchStart;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	// Decide on where to start the search
	
	if (_iSearchStart == CPC_INVALIDITEM)
		iSearchStart = 0;
	else
		iSearchStart = _iSearchStart + 1;
		
	// Search array for selected items
	for (iItemIDX = iSearchStart; iItemIDX < pListData->m_iNumItems; iItemIDX++)
	{
		if (pListData->m_pItems[iItemIDX].m_dwFlags & CPLV_ITEMFLAG_SELECTED)
			return iItemIDX;
	}
	
	return CPC_INVALIDITEM;
}

//
//
//
int CLV_GetPrevSelectedItem(CP_HLISTVIEW _hListData, const int _iSearchStart)
{
	CIs_ListViewData* pListData;
	int iItemIDX;
	int iSearchStart;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	// Decide on where to start the search
	
	if (_iSearchStart == CPC_INVALIDITEM)
		iSearchStart = pListData->m_iNumItems - 1;
	else
		iSearchStart = _iSearchStart - 1;
		
	// Search array for selected items
	for (iItemIDX = iSearchStart; iItemIDX >= 0; iItemIDX--)
	{
		if (pListData->m_pItems[iItemIDX].m_dwFlags & CPLV_ITEMFLAG_SELECTED)
			return iItemIDX;
	}
	
	return CPC_INVALIDITEM;
}

//
//
//
void CLV_SetItemSelected(CP_HLISTVIEW _hListData, const int iItemIDX, const BOOL bSelected)
{
	CIs_ListViewData* pListData;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	CP_ASSERT(iItemIDX >= 0);
	CP_ASSERT(iItemIDX < pListData->m_iNumItems);
	
	if (bSelected)
		pListData->m_pItems[iItemIDX].m_dwFlags |= CPLV_ITEMFLAG_SELECTED;
	else
		pListData->m_pItems[iItemIDX].m_dwFlags &= ~CPLV_ITEMFLAG_SELECTED;
		
	CLV_ItemSelChange(pListData, iItemIDX);
	
	if (pListData->m_bInBatch == FALSE)
		CLV_InvalidateItem(pListData, iItemIDX);
}

//
//
//
BOOL CLV_IsItemSelected(CP_HLISTVIEW _hListData, const int iItemIDX)
{
	CIs_ListViewData* pListData;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	CP_ASSERT(iItemIDX >= 0);
	CP_ASSERT(iItemIDX < pListData->m_iNumItems);
	
	if (pListData->m_pItems[iItemIDX].m_dwFlags & CPLV_ITEMFLAG_SELECTED)
		return TRUE;
		
	return FALSE;
}

//
//
//
void CLV_SetItemData(CP_HLISTVIEW _hListData, const int iItemIDX, const void* pvItemData)
{
	CIs_ListViewData* pListData;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	CP_ASSERT(iItemIDX >= 0);
	CP_ASSERT(iItemIDX < pListData->m_iNumItems);
	
	pListData->m_pItems[iItemIDX].m_pItemData = pvItemData;
	
	if (pListData->m_bInBatch == FALSE)
		CLV_InvalidateItem(pListData, iItemIDX);
}

//
//
//
const void* CLV_GetItemData(CP_HLISTVIEW _hListData, const int iItemIDX)
{
	CIs_ListViewData* pListData;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	CP_ASSERT(iItemIDX >= 0);
	CP_ASSERT(iItemIDX < pListData->m_iNumItems);
	
	return pListData->m_pItems[iItemIDX].m_pItemData;
}

//
//
//
void CLV_EnsureVisible(CP_HLISTVIEW _hListData, const int iItemIDX)
{
	CIs_ListViewData* pListData;
	
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	CP_ASSERT(iItemIDX >= 0);
	CP_ASSERT(iItemIDX < pListData->m_iNumItems);
	
	if (iItemIDX < pListData->m_iFirstVisibleItem)
	{
		pListData->m_iFirstVisibleItem = iItemIDX;
		CLV_InvalidateWindow(pListData);
		CLV_UpdateScrollBars(pListData);
	}
	
	else
	{
		int iListRectHeight_CompleteLines;
		iListRectHeight_CompleteLines = CLV_GetListRect_Lines(pListData) - 1;
		
		if (iItemIDX > (pListData->m_iFirstVisibleItem + iListRectHeight_CompleteLines))
		{
			pListData->m_iFirstVisibleItem = iItemIDX - iListRectHeight_CompleteLines;
			CLV_InvalidateWindow(pListData);
			CLV_UpdateScrollBars(pListData);
		}
	}
}

//
//
//
void CLV_SetItem(CP_HLISTVIEW _hListData, const int iItemIDX, const void* pvItemData)
{
	CIs_ListViewData* pListData;
	
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	CP_ASSERT(iItemIDX >= 0);
	CP_ASSERT(iItemIDX < pListData->m_iNumItems);
	
	pListData->m_pItems[iItemIDX].m_pItemData = pvItemData;
	
}

//
//
//
void CLV_DeleteItem(CP_HLISTVIEW _hListData, const int iItemToDelete)
{
	CIs_ListViewData* pListData;
	int iItemIDX;
	
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	CP_ASSERT(iItemToDelete >= 0);
	CP_ASSERT(iItemToDelete < pListData->m_iNumItems);
	
	// Shunt all items down one
	
	if (pListData->m_iNumItems == 1)
		CLV_RemoveAllItems(_hListData);
	else
	{
		if (iItemToDelete == pListData->m_iKeyboardAnchor)
			pListData->m_iKeyboardAnchor = CPC_INVALIDITEM;
		else if (iItemToDelete < pListData->m_iKeyboardAnchor)
			pListData->m_iKeyboardAnchor--;
			
		// Remove item (and shunt items down)
		pListData->m_iNumItems--;
		
		for (iItemIDX = iItemToDelete; iItemIDX < pListData->m_iNumItems; iItemIDX++)
			pListData->m_pItems[iItemIDX] = pListData->m_pItems[iItemIDX+1];
			
		// Ensure that the focus and anchor items are still valid
		if (pListData->m_iFocusItem >= pListData->m_iNumItems)
		{
			pListData->m_iFocusItem = pListData->m_iNumItems - 1;
			CLV_InvalidateItem(pListData, pListData->m_iFocusItem);
		}
		
		// Perform invalidation
		
		if (pListData->m_bInBatch == FALSE)
		{
			CLV_InvalidateWindow(pListData);
			CLV_UpdateScrollBars(pListData);
		}
	}
}

//
//
//
int CLV_GetItemCount(CP_HLISTVIEW _hListData)
{
	CIs_ListViewData* pListData;
	
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	return pListData->m_iNumItems;
}

//
//
//
int CLV_GetNearestItem(CP_HLISTVIEW _hListData, const POINT* ptMouse)
{
	CIs_ListViewData* pListData;
	int iHitItemIDX;
	
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	if (pListData->m_iNumItems == 0)
		return CPC_INVALIDITEM;
		
	iHitItemIDX = CLV_YOffsetToItem(pListData, ptMouse->y);
	
	if (iHitItemIDX < 0)
		iHitItemIDX = 0;
		
	if (iHitItemIDX >= pListData->m_iNumItems)
		iHitItemIDX = pListData->m_iNumItems - 1;
		
	return iHitItemIDX;
}

//
//
//
void CLV_GetItemSubRect(CP_HLISTVIEW _hListData, RECT* pRect, const int iTargetItemIDX, const int iTargetColumnIDX)
{
	CIs_ListViewData* pListData;
	unsigned int _iColIDX;
	int iCursorX;
	
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	CP_ASSERT(iTargetItemIDX >= 0);
	CP_ASSERT(iTargetItemIDX < pListData->m_iNumItems);
	CP_ASSERT(iTargetColumnIDX >= 0);
	CP_ASSERT(iTargetColumnIDX < (int)pListData->m_iNumColumns);
	
	// Work out top and bottom
	pRect->top = pListData->m_rList.top + ((iTargetItemIDX - pListData->m_iFirstVisibleItem) * pListData->m_iItemHeight);
	pRect->bottom = pRect->top + pListData->m_iItemHeight;
	
	iCursorX = -pListData->m_iXOrigin;
	
	for (_iColIDX = 0; _iColIDX < pListData->m_iNumColumns; _iColIDX++)
	{
		int iColumnIDX = pListData->m_piColumnOrder[_iColIDX];
		
		if (pListData->m_pColumns[iColumnIDX].m_dwFlags & CPLV_COLFLAG_HIDDEN)
			continue;
			
		if (iColumnIDX == iTargetColumnIDX)
		{
			pRect->left = iCursorX;
			pRect->right = pRect->left + pListData->m_pColumns[iColumnIDX].m_iColumnWidth;
			break;
		}
		
		iCursorX += pListData->m_pColumns[iColumnIDX].m_iColumnWidth;
	}
	
	// Clip to the list rect
	
	if (pRect->left < pListData->m_rList.left)
		pRect->left = pListData->m_rList.left;
		
	if (pRect->right > pListData->m_rList.right)
		pRect->right = pListData->m_rList.right;
		
	if (pRect->top < pListData->m_rList.top)
		pRect->top = pListData->m_rList.top;
		
	if (pRect->bottom > pListData->m_rList.right)
		pRect->bottom = pListData->m_rList.bottom;
}

//
//
//
void CLV_InvalidateColumn(CP_HLISTVIEW _hListData, const int iInvalidColumnIDX)
{
	CIs_ListViewData* pListData;
	RECT rInvalid;
	unsigned int _iColIDX;
	int iCursorX;
	
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	// Hidden?
	
	if (pListData->m_pColumns[iInvalidColumnIDX].m_dwFlags & CPLV_COLFLAG_HIDDEN)
		return;
		
	// Find the start point of the column
	iCursorX = -pListData->m_iXOrigin;
	
	for (_iColIDX = 0; _iColIDX < pListData->m_iNumColumns; _iColIDX++)
	{
		unsigned int iColumnIDX = pListData->m_piColumnOrder[_iColIDX];
		
		if ((int)iColumnIDX == iInvalidColumnIDX)
			break;
			
		if (pListData->m_pColumns[iColumnIDX].m_dwFlags & CPLV_COLFLAG_HIDDEN)
			continue;
			
		iCursorX += pListData->m_pColumns[iColumnIDX].m_iColumnWidth;
	}
	
	rInvalid.top = 0;
	rInvalid.bottom = pListData->m_rClient.bottom;
	rInvalid.left = iCursorX;
	rInvalid.right = rInvalid.left + pListData->m_pColumns[iInvalidColumnIDX].m_iColumnWidth;
	InvalidateRect(pListData->m_hWnd, &rInvalid, FALSE);
}

//
//
//
void CLV_Invalidate(CP_HLISTVIEW _hListData)
{
	CIs_ListViewData* pListData;
	pListData = (CIs_ListViewData*)_hListData;
	CP_CHECKOBJECT(pListData);
	
	InvalidateRect(pListData->m_hWnd, NULL, FALSE);
}

//
//
//
