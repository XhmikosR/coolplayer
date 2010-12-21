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
#include "WindowsOS.h"
#include "CPI_InterfacePart.h"



////////////////////////////////////////////////////////////////////////////////
// Handy macro to tidy up the source (handles the "SetHandler_xxxx" functions)
#define CPC_IMPLEMENT_SETHANDLER(handlertype) \
	void IF_sethandler_##handlertype(CP_HINTERFACE hInterface, wp_IF_##handlertype pfnHandler) { \
		CPs_InterfaceWindowState* pState;              \
		pState = (CPs_InterfaceWindowState*)hInterface;           \
		CP_CHECKOBJECT(pState);                 \
		pState->m_hndlr_##handlertype = pfnHandler; }
//
////////////////////////////////////////////////////////////////////////////////




#define CPC_SIZEBORDER   0x4
////////////////////////////////////////////////////////////////////////////////
//

typedef struct _CPs_InterfaceWindowState
{
	HWND m_hWnd;
	DWORD m_dwStyle;
	SIZE m_szMinSize;
	
	// Windowpos
	POINT m_ptWindowPos;
	SIZE m_szWindowSize;
	
	// Window state
	BOOL m_bSkipRegion;
	BOOL m_bMouseLeaveEventSet;
	CPs_InterfacePart* m_pFloatActiveSubpart;
	CPs_InterfacePart* m_pActiveSubpart;
	
	// Callback handlers
	wp_IF_onCreate m_hndlr_onCreate;
	wp_IF_onDestroy m_hndlr_onDestroy;
	wp_IF_onPosChange m_hndlr_onPosChange;
	wp_IF_onDraw m_hndlr_onDraw;
	wp_IF_onKeyDown m_hndlr_onKeyDown;
	wp_IF_onDropFiles m_hndlr_onDropFiles;
	wp_IF_onFocus m_hndlr_onFocus;
	wp_IF_onAppMessage m_hndlr_onAppMessage;
	wp_IF_onCommandMessage m_hndlr_onCommandMessage;
	wp_IF_onClose m_hndlr_onClose;
	
	BOOL m_bMouseCaptured;
	wp_IF_onMouseMessage m_hndlr_onMouseMove;
	wp_IF_onMouseMessage m_hndlr_onMouseButton_LUp;
	
	// Subparts
	CPs_InterfacePart* m_pFirstSubPart;
	
} CPs_InterfaceWindowState;

//
////////////////////////////////////////////////////////////////////////////////


LRESULT CALLBACK exp_InterfaceWindowProc(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam);
void IF_PaintWindow(CPs_InterfaceWindowState* pState, CPs_DrawContext* pContext);
////////////////////////////////////////////////////////////////////////////////
//
//
//
CPC_IMPLEMENT_SETHANDLER(onCreate)
CPC_IMPLEMENT_SETHANDLER(onDestroy)
CPC_IMPLEMENT_SETHANDLER(onPosChange)
CPC_IMPLEMENT_SETHANDLER(onDraw)
CPC_IMPLEMENT_SETHANDLER(onKeyDown)
CPC_IMPLEMENT_SETHANDLER(onDropFiles)
CPC_IMPLEMENT_SETHANDLER(onFocus)
CPC_IMPLEMENT_SETHANDLER(onAppMessage)
CPC_IMPLEMENT_SETHANDLER(onCommandMessage)
CPC_IMPLEMENT_SETHANDLER(onClose)
//
//
//
HWND IF_GetHWnd(CP_HINTERFACE hInterface)
{
	CPs_InterfaceWindowState* pState;
	
	// Init
	pState = (CPs_InterfaceWindowState*)hInterface;
	CP_CHECKOBJECT(pState);
	
	return pState->m_hWnd;
}

//
//
//
void IF_ProcessInit()
{
	WNDCLASS wcPlaylist;
	wcPlaylist.style = 0L;
	wcPlaylist.lpfnWndProc = exp_InterfaceWindowProc;
	wcPlaylist.cbClsExtra = 0;
	wcPlaylist.cbWndExtra = 0;
	wcPlaylist.hInstance = GetModuleHandle(NULL);
	wcPlaylist.hIcon = NULL; // We will explicity set our icons (so that we have a nice small icon)
	wcPlaylist.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcPlaylist.hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH); // Prevent the system drawing white over our invaid rgn before we can paint
	wcPlaylist.lpszMenuName = NULL;
	wcPlaylist.lpszClassName = CLC_COOLPLAYER_INTERFACECLASSNAME;
	RegisterClass(&wcPlaylist);
}

//
//
//
void IF_ProcessDeInit()
{
	UnregisterClass(CLC_COOLPLAYER_INTERFACECLASSNAME, GetModuleHandle(NULL));
}

//
//
//
CP_HINTERFACE IF_Create(const char* pcTitle, const RECT* pInitialSize, const DWORD dwStyle)
{
	CPs_InterfaceWindowState* pState;
	
	pState = (CPs_InterfaceWindowState*)malloc(sizeof(*pState));
	memset(pState, 0, sizeof(*pState));
	
	return pState;
}

//
//
//
void IF_OpenWindow(CP_HINTERFACE hInterface, const char* pcTitle, const RECT* pInitialSize, const DWORD dwStyle)
{
	CPs_InterfaceWindowState* pState;
	
	// Init
	pState = (CPs_InterfaceWindowState*)hInterface;
	CP_CHECKOBJECT(pState);
	
	// Setup base state
	pState->m_dwStyle = dwStyle;
	pState->m_bSkipRegion = FALSE;
	
	// Create Windows window
	CreateWindowEx(WS_EX_ACCEPTFILES,
				   CLC_COOLPLAYER_INTERFACECLASSNAME,
				   pcTitle,
				   WS_POPUP | WS_CLIPCHILDREN | WS_SYSMENU
				   | ((dwStyle & CPC_INTERFACE_STYLE_RESIZING) ? WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME : 0),
				   pInitialSize->left, pInitialSize->top,
				   pInitialSize->right - pInitialSize->left,
				   pInitialSize->bottom - pInitialSize->top,
				   NULL,
				   NULL,
				   GetModuleHandle(NULL),
				   pState);
}

//
//
//
void IF_CleanupState(CPs_InterfaceWindowState* pState)
{
	free(pState);
}

//
//
//
void IF_CloseWindow(CP_HINTERFACE hInterface)
{
	CPs_InterfaceWindowState* pState;
	
	// Init
	pState = (CPs_InterfaceWindowState*)hInterface;
	CP_CHECKOBJECT(pState);
	
	DestroyWindow(pState->m_hWnd);
}

//
//
//
void IF_SetVisible(CP_HINTERFACE hInterface, const BOOL bVisible)
{
	CPs_InterfaceWindowState* pState;
	
	// Init
	pState = (CPs_InterfaceWindowState*)hInterface;
	CP_CHECKOBJECT(pState);
	
	if (bVisible)
		if (IsIconic(pState->m_hWnd))
			ShowWindow(pState->m_hWnd, SW_RESTORE);

	ShowWindow(pState->m_hWnd, bVisible ? SW_SHOW : SW_HIDE);

}

//
//
//
void IF_SetFloatActiveSubPart(CPs_InterfaceWindowState* pState, CPs_InterfacePart* pNewFloatActiveSubPart)
{
	// If this OS cannot give us mouseleave notification - do not do any float active stuff
	if (!pfnTrackMouseEvent)
		return;
		
	if (pState->m_pFloatActiveSubpart == pNewFloatActiveSubPart)
		return;
		
	// Set mouseleave event
	if (pState->m_bMouseLeaveEventSet == FALSE)
	{
		TRACKMOUSEEVENT tme;
		
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = pState->m_hWnd;
		pState->m_bMouseLeaveEventSet = pfnTrackMouseEvent(&tme) ? TRUE : FALSE;
	}
	
	// Reset existing FA target
	
	if (pState->m_pFloatActiveSubpart)
	{
		if (pState->m_pFloatActiveSubpart->onMouseOut)
			pState->m_pFloatActiveSubpart->onMouseOut(pState->m_pFloatActiveSubpart);
	}
	
	// Set new FA target - do not set it if another target has buttondown
	
	if (pState->m_pActiveSubpart == NULL
			|| pNewFloatActiveSubPart == NULL
			|| pState->m_pActiveSubpart == pNewFloatActiveSubPart)
	{
		pState->m_pFloatActiveSubpart = pNewFloatActiveSubPart;
		
		if (pState->m_pFloatActiveSubpart)
		{
			if (pState->m_pFloatActiveSubpart->onMouseIn)
				pState->m_pFloatActiveSubpart->onMouseIn(pState->m_pFloatActiveSubpart);
		}
	}
}

//
//
//
CPs_InterfacePart* IF_HitTestMouse(CPs_InterfaceWindowState* pState, const POINT* pptMouse)
{
	CPs_InterfacePart* pSubPart_Cursor;
	
	// Walk through list drawing and adding to the valid rects list
	
	for (pSubPart_Cursor = pState->m_pFirstSubPart; pSubPart_Cursor; pSubPart_Cursor = (CPs_InterfacePart*)pSubPart_Cursor->m_hNext)
	{
		if (PtInRect(&pSubPart_Cursor->m_rLocation, *pptMouse) == TRUE)
		{
			return pSubPart_Cursor;
		}
	}
	
	return NULL;
}

//
//
//
LRESULT CALLBACK exp_InterfaceWindowProc(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
	CPs_InterfaceWindowState* pState;
	
	// Get the window's data object
	
	if (uiMessage == WM_NCCREATE)
	{
		HMODULE hModApplication;
		pState = (CPs_InterfaceWindowState*)((CREATESTRUCT*)lParam)->lpCreateParams;
		pState->m_hWnd = hWnd;
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)pState);
		
		// Setup icons
		hModApplication = GetModuleHandle(NULL);
		SendMessage(hWnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)LoadIcon(hModApplication, MAKEINTRESOURCE(APP_ICON)));
		SendMessage(hWnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)LoadIcon(hModApplication, MAKEINTRESOURCE(APP_ICON)));
	}
	
	else
		pState = (CPs_InterfaceWindowState*)GetWindowLong(hWnd, GWL_USERDATA);
		
	// We get some messages before the window gets it's WM_NCCREATE!!! (just how bad is Windows eh???)
	if (!pState)
		return DefWindowProc(hWnd, uiMessage, wParam, lParam);
		
	CP_CHECKOBJECT(pState);
	
	// Message handlers
	switch (uiMessage)
	{
	
		case WM_MOUSEMOVE:
		{
			if (pState->m_bMouseCaptured)
			{
				if (pState->m_hndlr_onMouseMove)
					pState->m_hndlr_onMouseMove(pState, MAKEPOINTS(lParam), (SHORT)wParam);
			}
			
			else
			{
				CPs_InterfacePart* pHitSubPart;
				POINT ptMouse;
				ptMouse.x = (short)LOWORD(lParam);
				ptMouse.y = (short)HIWORD(lParam);
				
				pHitSubPart = IF_HitTestMouse(pState, &ptMouse);
				IF_SetFloatActiveSubPart(pState, pHitSubPart);
			}
			
			return 0;
		} // end WM_MOUSEMOVE

		case WM_LBUTTONDOWN:
		{
			CPs_InterfacePart* pHitSubPart;
			POINT ptMouse;
			ptMouse.x = (short)LOWORD(lParam);
			ptMouse.y = (short)HIWORD(lParam);
			
			pHitSubPart = IF_HitTestMouse(pState, &ptMouse);
			
			if (pHitSubPart && pHitSubPart->onMouseButton_LDown)
			{
				SetCapture(pState->m_hWnd);
				pState->m_pActiveSubpart = pHitSubPart;
				pHitSubPart->onMouseButton_LDown(pHitSubPart, MAKEPOINTS(lParam));
			}

			return 0;
		} // end WM_LBUTTONDOWN
				
		case WM_LBUTTONUP:
		{
			if (pState->m_bMouseCaptured)
			{
				if (pState->m_hndlr_onMouseButton_LUp)
					pState->m_hndlr_onMouseButton_LUp(pState, MAKEPOINTS(lParam), (SHORT)wParam);
			}
			
			else
			{
				if (pState->m_pActiveSubpart)
				{
					ReleaseCapture();
					
					if (pState->m_pActiveSubpart->onMouseButton_LUp)
						pState->m_pActiveSubpart->onMouseButton_LUp(pState->m_pActiveSubpart, MAKEPOINTS(lParam));
						
					pState->m_pActiveSubpart = NULL;
				}
			}
			
			return 0;
		} // end WM_LBUTTONUP


		case WM_NCDESTROY:
		{
			IF_CleanupState(pState);
			break;
		} // end WM_NCDESTROY
			
		case WM_CREATE:
		{
			const CREATESTRUCT* pCS = (const CREATESTRUCT*)lParam;
			
			// Update size/pos cache
			pState->m_ptWindowPos.x = pCS->x;
			pState->m_ptWindowPos.y = pCS->y;
			pState->m_szWindowSize.cx = pCS->cx;
			pState->m_szWindowSize.cy = pCS->cy;
			
			// Reposition any subitems
			IF_UpdateSubPartLayout(pState);
			
			// Callback
			
			if (pState->m_hndlr_onCreate)
			{
				RECT rInitialPos;
				
				rInitialPos.left = 0;
				rInitialPos.top = 0;
				rInitialPos.right = pCS->cx;
				rInitialPos.bottom = pCS->cy;
				pState->m_hndlr_onCreate(pState, &rInitialPos);
			}
			
			IF_RebuildRegion(pState);

			return 0;
		} // end WM_CREATE

		
		case WM_DESTROY:
		{
			if (pState->m_hndlr_onDestroy)
				pState->m_hndlr_onDestroy(pState);
				
			IF_RemoveAllSubparts(pState);
			
			break;
		} // end WM_DESTROY
		
			
		case WM_NCHITTEST:
		{
			CPs_InterfacePart* pSubPart_Cursor;
			POINT ptMouse;
			ptMouse.x = (short)LOWORD(lParam) - pState->m_ptWindowPos.x;
			ptMouse.y = (short)HIWORD(lParam) - pState->m_ptWindowPos.y;
			
			// First perform hit testing on subparts
			
			for (pSubPart_Cursor = pState->m_pFirstSubPart; pSubPart_Cursor; pSubPart_Cursor = (CPs_InterfacePart*)pSubPart_Cursor->m_hNext)
			{
				if (PtInRect(&pSubPart_Cursor->m_rLocation, ptMouse) == TRUE)
					return HTCLIENT;
			}
			
			// If the window is sizable - perform hit testing on edges
			
			if (pState->m_dwStyle & CPC_INTERFACE_STYLE_RESIZING)
			{
				// Is the mouse inside the size bands
				// - top band
				if (ptMouse.y < CPC_SIZEBORDER)
				{
					// Are we in a corner
					// - left
					if (ptMouse.x < CPC_SIZEBORDER)
						return HTTOPLEFT;
						
					// - right
					else if (ptMouse.x > (pState->m_szWindowSize.cx - CPC_SIZEBORDER))
						return HTTOPRIGHT;
					else
						return HTTOP;
				}
				
				// - bottom band
				
				else if (ptMouse.y > (pState->m_szWindowSize.cy - CPC_SIZEBORDER))
				{
					// Are we in a corner
					// - left
					if (ptMouse.x < CPC_SIZEBORDER)
						return HTBOTTOMLEFT;
						
					// - right
					else if (ptMouse.x > (pState->m_szWindowSize.cx - CPC_SIZEBORDER))
						return HTBOTTOMRIGHT;
					else
						return HTBOTTOM;
				}
				
				// - left band
				
				else if (ptMouse.x < CPC_SIZEBORDER)
					return HTLEFT;
					
				// - right band
				else if (ptMouse.x > (pState->m_szWindowSize.cx - CPC_SIZEBORDER))
					return HTRIGHT;
			}
			
			// - return the caption so that the window is dragged
			return HTCAPTION;
		} // end WM_NCHITTEST
		
		case WM_WINDOWPOSCHANGED:
		
		{
			const WINDOWPOS* pWP = (const WINDOWPOS*)lParam;
			RECT rNewPos;
			BOOL bSizeChanged;
			
			rNewPos.left = pWP->x;
			rNewPos.right = pWP->x + pWP->cx;
			rNewPos.top = pWP->y;
			rNewPos.bottom = pWP->y + pWP->cy;
			bSizeChanged = (pWP->flags & SWP_NOSIZE) ? FALSE : TRUE;
			
			pState->m_ptWindowPos.x = pWP->x;
			pState->m_ptWindowPos.y = pWP->y;
			pState->m_szWindowSize.cx = pWP->cx;
			pState->m_szWindowSize.cy = pWP->cy;
			
			if (bSizeChanged)
			{
				IF_UpdateSubPartLayout(pState);
				IF_RebuildRegion(pState);
			}
			
			// Perform callback
			
			if (pState->m_hndlr_onPosChange)
				pState->m_hndlr_onPosChange(pState, &rNewPos, bSizeChanged);

			return 0;
		} // end WM_WINDOWPOSCHANGED

		
		case WM_NCCALCSIZE:
		{
			// We do not wish to have any window area lost to captions etc (also prevent the system
			// from preserving our window contents during a resize
			return WVR_REDRAW;
		} // end WM_NCCALCSIZE
			
		case WM_NCACTIVATE:
			return TRUE;
			
		case WM_NCPAINT:
			return 0;
			
		case WM_NCMOUSEMOVE:
			IF_SetFloatActiveSubPart(pState, NULL);
			return 0;
			
		case WM_MOUSELEAVE:
			IF_SetFloatActiveSubPart(pState, NULL);
			pState->m_bMouseLeaveEventSet = FALSE;
			return 0;
			
		case WM_CLOSE:
		{
			if (pState->m_hndlr_onClose)
			{
				pState->m_hndlr_onClose(pState);
				return 0;
			}
			
			break;
		} // end WM_CLOSE
		
			
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			CPs_DrawContext drawcontext;
			
			// Prepare for draw
			drawcontext.m_dcDraw = BeginPaint(hWnd, &ps);
			drawcontext.m_ptOffset.x = 0;
			drawcontext.m_ptOffset.y = 0;
			GetClipBox(drawcontext.m_dcDraw, &drawcontext.m_rClip);
			
			IF_PaintWindow(pState, &drawcontext);
			
			// Cleanup
			EndPaint(hWnd, &ps);

			return 0;
		} // end WM_PAINT
		

		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_ACTIVE)
				SetFocus(hWnd);
				
			return 0;
		} // end WM_ACTIVATE
		
			
		case WM_SYSKEYDOWN:
			// FALLTHROUGH
		case WM_KEYDOWN:
		{
			if (pState->m_hndlr_onKeyDown)
			{
				const BOOL bAltIsDown = (GetAsyncKeyState(VK_MENU)  & 0x8000) ? TRUE : FALSE;
				const BOOL bCtrlIsDown = (GetAsyncKeyState(VK_CONTROL)  & 0x8000) ? TRUE : FALSE;
				const BOOL bShiftIsDown = (GetAsyncKeyState(VK_SHIFT)  & 0x8000) ? TRUE : FALSE;
				pState->m_hndlr_onKeyDown(pState, (unsigned int)wParam, bAltIsDown, bCtrlIsDown, bShiftIsDown);
			}
			
			return 0;
		} // end WM_KEYDOWN
		
			
		case WM_DROPFILES:
		{
			if (pState->m_hndlr_onDropFiles)
				pState->m_hndlr_onDropFiles(pState, (HDROP)wParam);
				
			return 0;
		} // end WM_DROPFILES
		
			
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* pMinMaxInfo = (MINMAXINFO*)lParam;
			RECT rWorkArea;
			
			if (pfnGetMonitorInfo)
			{
				// Multimonitors are supported by this OS
				MONITORINFO mi;
				HMONITOR hmon = pfnMonitorFromWindow(pState->m_hWnd, MONITOR_DEFAULTTOPRIMARY);
				mi.cbSize = sizeof(mi);
				pfnGetMonitorInfo(hmon, &mi);
				
				// Get the work area of this monitor - as an offset from this monitors virtual space
				rWorkArea.left = mi.rcWork.left - mi.rcMonitor.left;
				rWorkArea.right = mi.rcWork.right - mi.rcMonitor.left;
				rWorkArea.top = mi.rcWork.top - mi.rcMonitor.top;
				rWorkArea.bottom = mi.rcWork.bottom - mi.rcMonitor.top;
			}
			
			else
			{
				// Single monitor only OS
				SystemParametersInfo(SPI_GETWORKAREA, 0, &rWorkArea, 0);
			}
			
			pMinMaxInfo->ptMinTrackSize.x = pState->m_szMinSize.cx;
			
			pMinMaxInfo->ptMinTrackSize.y = pState->m_szMinSize.cy;
			pMinMaxInfo->ptMaxPosition.x = rWorkArea.left;
			pMinMaxInfo->ptMaxPosition.y = rWorkArea.top;
			pMinMaxInfo->ptMaxSize.x = rWorkArea.right - rWorkArea.left;
			pMinMaxInfo->ptMaxSize.y = rWorkArea.bottom - rWorkArea.top;
			pMinMaxInfo->ptMaxTrackSize.x = pMinMaxInfo->ptMaxSize.x;
			pMinMaxInfo->ptMaxTrackSize.y = pMinMaxInfo->ptMaxSize.y;
		
			return 0;
		} // end WM_GETMINMAXINFO
		
		
		case WM_SETFOCUS:
		{
			if (pState->m_hndlr_onFocus)
				pState->m_hndlr_onFocus(pState, TRUE);
				
			return 0;
		} // end WM_SETFOCUS
			

		case WM_KILLFOCUS:
		{
			if (pState->m_hndlr_onFocus)
				pState->m_hndlr_onFocus(pState, FALSE);
				
			return 0;
		} // end WM_KILLFOCUS
			
		case WM_COMMAND:
		{
			if (pState->m_hndlr_onCommandMessage)
				pState->m_hndlr_onCommandMessage(pState, wParam, lParam);
				
			return 0;
		} // end WM_COMMAND
	}
	
	// Route message (to windows if it isn't handled)
	
	if (uiMessage >= WM_APP && pState->m_hndlr_onAppMessage)
		return pState->m_hndlr_onAppMessage(pState, uiMessage, wParam, lParam);
		
	return DefWindowProc(hWnd, uiMessage, wParam, lParam);
}

//
//
//
void IF_RebuildRegion(CP_HINTERFACE hInterface)
{
	CPs_InterfaceWindowState* pState;
	HBITMAP bmOld;
	HBITMAP bmSurface;
	HDC dcDraw;
	RECT rClient;
	HDC dcScreen;
	HRGN rgnWindow;
	
	// Init
	pState = (CPs_InterfaceWindowState*)hInterface;
	CP_CHECKOBJECT(pState);
	
	// There was no region last time - don't bother looking for one now
	
	if (pState->m_bSkipRegion)
		return;
		
	// Create an offscreen
	GetClientRect(pState->m_hWnd, &rClient);
	
	bmSurface = CreateBitmap(rClient.right, rClient.bottom, 1, 1, NULL);
	
	dcScreen = GetDC(pState->m_hWnd);
	dcDraw = CreateCompatibleDC(dcScreen);
	
	bmOld = (HBITMAP)SelectObject(dcDraw, bmSurface);
	
	ReleaseDC(pState->m_hWnd, dcScreen);
	
	// Draw the window
	{
		CPs_DrawContext drawcontext;
		drawcontext.m_dcDraw = dcDraw;
		drawcontext.m_ptOffset.x = 0;
		drawcontext.m_ptOffset.y = 0;
		drawcontext.m_rClip = rClient;
		IF_PaintWindow(pState, &drawcontext);
	}
	
	// Cleanup
	SelectObject(dcDraw, bmOld);
	
	DeleteDC(dcDraw);
	
	// Setup region
	rgnWindow = main_bitmap_to_region_1bit(bmSurface, glb_pSkin->m_clrTransparent);
	
	DeleteObject(bmSurface);
	
	SetWindowRgn(pState->m_hWnd, rgnWindow, TRUE);
	             
	if (!rgnWindow)
		pState->m_bSkipRegion = TRUE;
}

//
//
//
void IF_PaintWindow(CPs_InterfaceWindowState* pState, CPs_DrawContext* pContext)
{
	// Walk through list drawing and adding to the valid rects list
	CPs_InterfacePart* pSubPart_Cursor;
	
	for (pSubPart_Cursor = pState->m_pFirstSubPart; pSubPart_Cursor; pSubPart_Cursor = (CPs_InterfacePart*)pSubPart_Cursor->m_hNext)
	{
		if (pSubPart_Cursor->Draw)
		{
			pSubPart_Cursor->Draw(pSubPart_Cursor, pContext);
			ExcludeClipRect(pContext->m_dcDraw,
							pSubPart_Cursor->m_rLocation.left, pSubPart_Cursor->m_rLocation.top,
							pSubPart_Cursor->m_rLocation.right, pSubPart_Cursor->m_rLocation.bottom);
		}
	}
	
	// Perform callback
	
	if (pState->m_hndlr_onDraw)
		pState->m_hndlr_onDraw(pState, pContext);
}

//
//
//
void IF_SetMinSize(CP_HINTERFACE hInterface, const SIZE* pMinSize)
{
	CPs_InterfaceWindowState* pState;
	
	// Init
	pState = (CPs_InterfaceWindowState*)hInterface;
	CP_CHECKOBJECT(pState);
	
	pState->m_szMinSize = *pMinSize;
}

//
//
//
void IF_PostAppMessage(CP_HINTERFACE hInterface, const UINT uiMessage, const WPARAM wParam, const LPARAM lParam)
{
	CPs_InterfaceWindowState* pState;
	
	// Init
	pState = (CPs_InterfaceWindowState*)hInterface;
	CP_CHECKOBJECT(pState);
	CP_ASSERT(uiMessage >= WM_APP);
	
	PostMessage(pState->m_hWnd, uiMessage, wParam, lParam);
}

//
//
//
void IF_SetMouseCapture(CP_HINTERFACE hInterface, wp_IF_onMouseMessage pfn_onMouseMove, wp_IF_onMouseMessage pfn_onMouseButton_LUp)
{
	CPs_InterfaceWindowState* pState;
	
	// Init
	pState = (CPs_InterfaceWindowState*)hInterface;
	CP_CHECKOBJECT(pState);
	
	pState->m_bMouseCaptured = TRUE;
	pState->m_hndlr_onMouseMove = pfn_onMouseMove;
	pState->m_hndlr_onMouseButton_LUp = pfn_onMouseButton_LUp;
	SetCapture(pState->m_hWnd);
}

//
//
//
void IF_ReleaseMouseCapture(CP_HINTERFACE hInterface)
{
	CPs_InterfaceWindowState* pState;
	
	// Init
	pState = (CPs_InterfaceWindowState*)hInterface;
	CP_CHECKOBJECT(pState);
	
	pState->m_bMouseCaptured = FALSE;
	ReleaseCapture();
}

//
//
//
void IF_RemoveAllSubparts(CP_HINTERFACE hInterface)
{
	CPs_InterfaceWindowState* pState;
	CPs_InterfacePart* pSubPart_Cursor;
	CPs_InterfacePart* pSubPart_Next;
	
	// Init
	pState = (CPs_InterfaceWindowState*)hInterface;
	CP_CHECKOBJECT(pState);
	
	// Walk through list destroying subparts
	
	for (pSubPart_Cursor = pState->m_pFirstSubPart; pSubPart_Cursor; pSubPart_Cursor = pSubPart_Next)
	{
		pSubPart_Next = (CPs_InterfacePart*)pSubPart_Cursor->m_hNext;
		IP_Destroy(pSubPart_Cursor);
	}
	
	pState->m_pFirstSubPart = NULL;
}

//
//
//
void IF_AddSubPart_CommandButton(CP_HINTERFACE hInterface,
								 const DWORD dwAlign,
								 const POINT* pptOffset,
								 CPs_Image_WithState* pStateImage,
								 wp_Verb pfnVerb)
{
	CPs_InterfaceWindowState* pState;
	CPs_InterfacePart* pSubPart_Next;
	
	// Init
	pState = (CPs_InterfaceWindowState*)hInterface;
	CP_CHECKOBJECT(pState);
	
	// Add new head to list
	pSubPart_Next = pState->m_pFirstSubPart;
	
	// Setup new part
	pState->m_pFirstSubPart = IP_Create_CommandButton(pfnVerb, pStateImage);
	pState->m_pFirstSubPart->m_hNext = pSubPart_Next;
	pState->m_pFirstSubPart->m_hOwner = hInterface;
	pState->m_pFirstSubPart->m_dwAlign = dwAlign;
	pState->m_pFirstSubPart->m_bRectAlignMode = FALSE;
	pState->m_pFirstSubPart->m_ptOffset = *pptOffset;
	pState->m_pFirstSubPart->m_szSize.cx = pStateImage->m_pImage->m_szSize.cx;
	pState->m_pFirstSubPart->m_szSize.cy = pStateImage->m_iStateHeight;
}

//
//
//
void IF_AddSubPart_Indicator(CP_HINTERFACE hInterface,
							 const char* pcName,
							 const DWORD dwAlign,
							 const RECT* prPosition)
{
	CPs_InterfaceWindowState* pState;
	CPs_InterfacePart* pSubPart_Next;
	
	// Init
	pState = (CPs_InterfaceWindowState*)hInterface;
	CP_CHECKOBJECT(pState);
	
	// Add new head to list
	pSubPart_Next = pState->m_pFirstSubPart;
	
	// Setup new part
	pState->m_pFirstSubPart = IP_Create_Indicator(pcName);
	pState->m_pFirstSubPart->m_hNext = pSubPart_Next;
	pState->m_pFirstSubPart->m_hOwner = hInterface;
	pState->m_pFirstSubPart->m_dwAlign = dwAlign;
	pState->m_pFirstSubPart->m_bRectAlignMode = TRUE;
	pState->m_pFirstSubPart->m_rPosition = *prPosition;
}

//
//
//
void IF_UpdateSubPartLayout(CP_HINTERFACE hInterface)
{
	CPs_InterfaceWindowState* pState;
	CPs_InterfacePart* pSubPart_Cursor;
	
	// Init
	pState = (CPs_InterfaceWindowState*)hInterface;
	CP_CHECKOBJECT(pState);
	
	// Walk through list setting position
	
	for (pSubPart_Cursor = pState->m_pFirstSubPart; pSubPart_Cursor; pSubPart_Cursor = (CPs_InterfacePart*)pSubPart_Cursor->m_hNext)
	{
		if (pSubPart_Cursor->m_bRectAlignMode == TRUE)
		{
			// Set position
			// - left
			if (pSubPart_Cursor->m_dwAlign & CPC_COMMANDTARGET_ALIGN_LEFT)
				pSubPart_Cursor->m_rLocation.left = pSubPart_Cursor->m_rPosition.left;
			else
			{
				CP_ASSERT(pSubPart_Cursor->m_dwAlign & CPC_COMMANDTARGET_ALIGN_RIGHT);
				pSubPart_Cursor->m_rLocation.left = pState->m_szWindowSize.cx - pSubPart_Cursor->m_rPosition.right - pSubPart_Cursor->m_rPosition.left;
			}
			
			// - right
			
			if (pSubPart_Cursor->m_dwAlign & CPC_COMMANDTARGET_ALIGN_RIGHT)
				pSubPart_Cursor->m_rLocation.right = pState->m_szWindowSize.cx - pSubPart_Cursor->m_rPosition.right;
			else
			{
				CP_ASSERT(pSubPart_Cursor->m_dwAlign & CPC_COMMANDTARGET_ALIGN_LEFT);
				pSubPart_Cursor->m_rLocation.right = pSubPart_Cursor->m_rLocation.left + pSubPart_Cursor->m_rPosition.right;
			}
			
			// - top
			
			if (pSubPart_Cursor->m_dwAlign & CPC_COMMANDTARGET_ALIGN_TOP)
				pSubPart_Cursor->m_rLocation.top = pSubPart_Cursor->m_rPosition.top;
			else
			{
				//  CP_ASSERT(pSubPart_Cursor->m_dwAlign & CPC_COMMANDTARGET_ALIGN_BOTTOM);
				pSubPart_Cursor->m_rLocation.top = pState->m_szWindowSize.cy - pSubPart_Cursor->m_rPosition.bottom - pSubPart_Cursor->m_rPosition.top;
			}
			
			// - bottom
			
			if (pSubPart_Cursor->m_dwAlign & CPC_COMMANDTARGET_ALIGN_BOTTOM)
				pSubPart_Cursor->m_rLocation.bottom = pState->m_szWindowSize.cy - pSubPart_Cursor->m_rPosition.bottom;
			else
			{
//                CP_ASSERT(pSubPart_Cursor->m_dwAlign & CPC_COMMANDTARGET_ALIGN_TOP);
				pSubPart_Cursor->m_rLocation.bottom = pSubPart_Cursor->m_rLocation.top + pSubPart_Cursor->m_rPosition.bottom;
			}
		}
		
		else
		{
			// Set position
			if (pSubPart_Cursor->m_dwAlign & CPC_COMMANDTARGET_ALIGN_LEFT)
				pSubPart_Cursor->m_rLocation.left = pSubPart_Cursor->m_ptOffset.x;
			else
			{
				CP_ASSERT(pSubPart_Cursor->m_dwAlign & CPC_COMMANDTARGET_ALIGN_RIGHT);
				pSubPart_Cursor->m_rLocation.left = pState->m_szWindowSize.cx - pSubPart_Cursor->m_szSize.cx - pSubPart_Cursor->m_ptOffset.x;
			}
			
			pSubPart_Cursor->m_rLocation.right = pSubPart_Cursor->m_rLocation.left + pSubPart_Cursor->m_szSize.cx;
			
			if (pSubPart_Cursor->m_dwAlign & CPC_COMMANDTARGET_ALIGN_TOP)
				pSubPart_Cursor->m_rLocation.top = pSubPart_Cursor->m_ptOffset.y;
			else
			{
				CP_ASSERT(pSubPart_Cursor->m_dwAlign & CPC_COMMANDTARGET_ALIGN_BOTTOM);
				pSubPart_Cursor->m_rLocation.top = pState->m_szWindowSize.cy - pSubPart_Cursor->m_szSize.cy - pSubPart_Cursor->m_ptOffset.y;
			}
			
			pSubPart_Cursor->m_rLocation.bottom = pSubPart_Cursor->m_rLocation.top + pSubPart_Cursor->m_szSize.cy;
		}
	}
}

//
//
//

