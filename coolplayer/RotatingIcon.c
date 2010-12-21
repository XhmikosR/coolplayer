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

#include "stdafx.h"
#include "globals.h"


////////////////////////////////////////////////////////////////////////////////
//

typedef struct _CPs_SysIcon
{
	HWND m_hWnd;
	HBITMAP m_bmStrip;
	HBITMAP m_bmStrip_Mask;
	
	HBITMAP m_bmIcon;
	int m_iCurrentFrame;
	
} CPs_SysIcon;

//
////////////////////////////////////////////////////////////////////////////////



void CPSYSICON_DrawCurrentFrame(CPs_SysIcon* pSysIconData);
////////////////////////////////////////////////////////////////////////////////
//
//
//
CP_HSYSICON CPSYSICON_Create(HWND hWnd)
{
	CPs_SysIcon* pSysIconData = (CPs_SysIcon*)malloc(sizeof(CPs_SysIcon));
	
	pSysIconData->m_bmStrip = LoadBitmap(GetModuleHandle(NULL),
										 MAKEINTRESOURCE(IDB_SYSICON));
	pSysIconData->m_bmStrip_Mask = LoadImage(GetModuleHandle(NULL),
								   MAKEINTRESOURCE(IDB_SYSICON_MASK), IMAGE_BITMAP, 0, 0, LR_MONOCHROME);
	pSysIconData->m_iCurrentFrame = 0;
	pSysIconData->m_hWnd = hWnd;
	pSysIconData->m_bmIcon = NULL;
	
	CPSYSICON_DrawCurrentFrame(pSysIconData);
	
	{
	
		NOTIFYICONDATA nic;
		ICONINFO iconinfo;
		
		nic.cbSize = sizeof(NOTIFYICONDATA);
		nic.hWnd = hWnd;
		nic.uID = NOTIFY_ICON_ID;
		nic.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nic.uCallbackMessage = WM_NOTIFYICON;
		
		iconinfo.fIcon = TRUE;
		iconinfo.hbmColor = pSysIconData->m_bmIcon;
		iconinfo.hbmMask = pSysIconData->m_bmStrip_Mask;
		
		nic.hIcon = CreateIconIndirect(&iconinfo);
		
		strcpy(nic.szTip, "CoolerPlayer");
		Shell_NotifyIcon(NIM_ADD, &nic);
		
		if (nic.hIcon)
			DestroyIcon(nic.hIcon);
	}
	
	return pSysIconData;
}

//
//
//
void CPSYSICON_Destroy(CP_HSYSICON hSysIconData)
{
	CPs_SysIcon* pSysIconData = (CPs_SysIcon*)hSysIconData;
	NOTIFYICONDATA nic;
	CP_CHECKOBJECT(pSysIconData);
	
	// Remove systray icon
	nic.cbSize = sizeof(NOTIFYICONDATA);
	nic.hWnd = pSysIconData->m_hWnd;
	nic.uID = NOTIFY_ICON_ID;
	nic.uFlags = NIF_ICON | NIF_MESSAGE;
	Shell_NotifyIcon(NIM_DELETE, &nic);
	
	if (pSysIconData->m_bmStrip)
		DeleteObject(pSysIconData->m_bmStrip);
		
	if (pSysIconData->m_bmStrip_Mask)
		DeleteObject(pSysIconData->m_bmStrip_Mask);
		
	if (pSysIconData->m_bmIcon)
		DeleteObject(pSysIconData->m_bmIcon);
		
	free(pSysIconData);
}

//
//
//
void CPSYSICON_DrawCurrentFrame(CPs_SysIcon* pSysIconData)
{
	HDC dcSource, dcDest;
	HDC dcScreen;
	HBITMAP bmOld_Source, bmOld_Dest;
	
	// Setup dest DC
	dcDest = CreateCompatibleDC(NULL);
	
	if (!pSysIconData->m_bmIcon)
	{
		dcScreen = GetDC(NULL);
		pSysIconData->m_bmIcon = CreateCompatibleBitmap(dcScreen, 16, 16);
		ReleaseDC(NULL, dcScreen);
	}
	
	bmOld_Dest = (HBITMAP)SelectObject(dcDest, pSysIconData->m_bmIcon);
	
	// Setup source DC
	dcSource = CreateCompatibleDC(NULL);
	bmOld_Source = (HBITMAP)SelectObject(dcSource, pSysIconData->m_bmStrip);
	
	// Blt dest onto source
	BitBlt(dcDest, 0, 0, 16, 16,
		   dcSource, pSysIconData->m_iCurrentFrame * 16, 0, SRCCOPY);
	       
	       
	// Cleanup
	SelectObject(dcDest, bmOld_Dest);
	SelectObject(dcSource, bmOld_Source);
	DeleteDC(dcDest);
	DeleteDC(dcSource);
}

//
//
//
void CPSYSICON_AdvanceFrame(CP_HSYSICON hSysIconData)
{
	CPs_SysIcon* pSysIconData = (CPs_SysIcon*)hSysIconData;
	int iNewIconFrame;
	CP_CHECKOBJECT(pSysIconData);
	
	// Work out new icon frame
	iNewIconFrame = pSysIconData->m_iCurrentFrame;
	
	if (options.rotate_systray_icon)
	{
		if (globals.m_enPlayerState == cppsPlaying)
			iNewIconFrame = (pSysIconData->m_iCurrentFrame + 1) % 10;
		else if (globals.m_enPlayerState != cppsPaused)
			iNewIconFrame = 0;
	}
	
	else
		iNewIconFrame = 0;
		
	// Not changed? - fast out
	if (pSysIconData->m_iCurrentFrame == iNewIconFrame)
		return;
		
	// Update the icon
	pSysIconData->m_iCurrentFrame = iNewIconFrame;
	
	CPSYSICON_DrawCurrentFrame(pSysIconData);
	
	{
	
		NOTIFYICONDATA nic;
		ICONINFO iconinfo;
		
		nic.cbSize = sizeof(NOTIFYICONDATA);
		nic.hWnd = pSysIconData->m_hWnd;
		nic.uID = NOTIFY_ICON_ID;
		nic.uFlags = NIF_ICON;
		
		iconinfo.fIcon = TRUE;
		iconinfo.hbmColor = pSysIconData->m_bmIcon;
		iconinfo.hbmMask = pSysIconData->m_bmStrip_Mask;
		
		nic.hIcon = CreateIconIndirect(&iconinfo);
		Shell_NotifyIcon(NIM_MODIFY, &nic);
		
		if (nic.hIcon)
			DestroyIcon(nic.hIcon);
	}
	
	
	
}

//
//
//
void CPSYSICON_SetTipText(CP_HSYSICON hSysIconData, const char* pcNewTipText)
{
	CPs_SysIcon* pSysIconData = (CPs_SysIcon*)hSysIconData;
	NOTIFYICONDATA nic;
	CP_CHECKOBJECT(pSysIconData);
	
	nic.cbSize = sizeof(NOTIFYICONDATA);
	nic.hWnd = pSysIconData->m_hWnd;
	nic.uID = NOTIFY_ICON_ID;
	nic.uFlags = NIF_TIP;
	
	strncpy(nic.szTip, pcNewTipText, sizeof(nic.szTip));
	Shell_NotifyIcon(NIM_MODIFY, &nic);
}

//
//
//

