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
// Interface
//
// This will abstract the OS user interface.  This will improve modularity and open
// the door for non Windows ports
//
//
////////////////////////////////////////////////////////////////////////////////




#define CPC_INTERFACE_STYLE_RESIZING  0x1
////////////////////////////////////////////////////////////////////////////////
//
// One time calls (to initialise process wide structs)
void IF_ProcessInit();
void IF_ProcessDeInit();
//
// Interface basics
CP_HINTERFACE IF_Create();
void IF_OpenWindow(CP_HINTERFACE hInterface, const char* pcTitle, const RECT* pInitialSize, const DWORD dwStyle);
void IF_CloseWindow(CP_HINTERFACE hInterface);
void IF_SetVisible(CP_HINTERFACE hInterface, const BOOL bVisible);
void IF_RebuildRegion(CP_HINTERFACE hInterface);
HWND IF_GetHWnd(CP_HINTERFACE hInterface);
void IF_SetMinSize(CP_HINTERFACE hInterface, const SIZE* pMinSize);
void IF_PostAppMessage(CP_HINTERFACE hInterface, const UINT uiMessage, const WPARAM wParam, const LPARAM lParam);
// Subparts
void IF_RemoveAllSubparts(CP_HINTERFACE hInterface);
void IF_UpdateSubPartLayout(CP_HINTERFACE hInterface);
void IF_AddSubPart_CommandButton(CP_HINTERFACE hInterface,
								 const DWORD dwAlign,
								 const POINT* pptOffset,
								 CPs_Image_WithState* pStateImage,
								 wp_Verb pfnVerb);
void IF_AddSubPart_Indicator(CP_HINTERFACE hInterface,
							 const char* pcName,
							 const DWORD dwAlign,
							 const RECT* prPosition);
//
//
// Callback Handlers
// - window basics
typedef void (*wp_IF_onDestroy)(CP_HINTERFACE hInterface);
void IF_sethandler_onDestroy(CP_HINTERFACE hInterface, wp_IF_onDestroy pfnHandler);
typedef void (*wp_IF_onCreate)(CP_HINTERFACE hInterface, const RECT* pInitialPosition);
void IF_sethandler_onCreate(CP_HINTERFACE hInterface, wp_IF_onCreate pfnHandler);
typedef void (*wp_IF_onPosChange)(CP_HINTERFACE hInterface, const RECT* pNewPosition, const BOOL bSizeChanged);
void IF_sethandler_onPosChange(CP_HINTERFACE hInterface, wp_IF_onPosChange pfnHandler);
typedef void (*wp_IF_onDraw)(CP_HINTERFACE hInterface, CPs_DrawContext* pContext);
void IF_sethandler_onDraw(CP_HINTERFACE hInterface, wp_IF_onDraw pfnHandler);
typedef void (*wp_IF_onDropFiles)(CP_HINTERFACE hInterface, HDROP hDrop);
void IF_sethandler_onDropFiles(CP_HINTERFACE hInterface, wp_IF_onDropFiles pfnHandler);
typedef void (*wp_IF_onFocus)(CP_HINTERFACE hInterface, const BOOL bHasFocus);
void IF_sethandler_onFocus(CP_HINTERFACE hInterface, wp_IF_onFocus pfnHandler);
// - keyboard
typedef void (*wp_IF_onKeyDown)(CP_HINTERFACE hInterface, const unsigned int iVKeyCode, const BOOL bAlt, const BOOL bCtrl, const BOOL bShift);
void IF_sethandler_onKeyDown(CP_HINTERFACE hInterface, wp_IF_onKeyDown pfnHandler);
// - mouse
typedef void (*wp_IF_onMouseMessage)(CP_HINTERFACE hInterface, const POINTS ptMouse, const unsigned short iFlags);
void IF_SetMouseCapture(CP_HINTERFACE hInterface, wp_IF_onMouseMessage pfn_onMouseMove, wp_IF_onMouseMessage pfn_onMouseButton_LUp);
void IF_ReleaseMouseCapture(CP_HINTERFACE hInterface);
// - custom messages
typedef LRESULT(*wp_IF_onAppMessage)(CP_HINTERFACE hInterface, const UINT uiMessage, const WPARAM wParam, const LPARAM lParam);
void IF_sethandler_onAppMessage(CP_HINTERFACE hInterface, wp_IF_onAppMessage pfnHandler);
typedef void (*wp_IF_onCommandMessage)(CP_HINTERFACE hInterface, const WPARAM wParam, const LPARAM lParam);
void IF_sethandler_onCommandMessage(CP_HINTERFACE hInterface, wp_IF_onCommandMessage pfnHandler);
typedef void (*wp_IF_onClose)(CP_HINTERFACE hInterface);
void IF_sethandler_onClose(CP_HINTERFACE hInterface, wp_IF_onClose pfnHandler);
//
////////////////////////////////////////////////////////////////////////////////




