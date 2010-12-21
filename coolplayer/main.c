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

// Windows Header Files:

#include "stdafx.h"
#include "globals.h"
#include "WindowsOS.h"
#include "CPI_Player.h"
#include "CPI_Playlist.h"
#include "CPI_PlaylistItem.h"
#include "DLG_Find.h"
#include "CPI_PlaylistWindow.h"
#include "RotatingIcon.h"
#include "CPI_Indicators.h"




void    main_skin_select_menu(char *name)
{
	int     teller;
	char    skinstring[MAX_PATH];
	HMENU   popje = GetSubMenu(globals.main_menu_popup, SKIN_SUBMENU_INDEX);
	int     itemcounter =
		GetMenuItemCount(GetSubMenu(globals.main_menu_popup, SKIN_SUBMENU_INDEX));
	    
	for (teller = 0; teller < itemcounter; teller++)
	{
		GetMenuString(popje, teller, skinstring, MAX_PATH, MF_BYPOSITION);
		
		if (strcmp(name, skinstring) == 0)
		{
			CheckMenuRadioItem(popje, 0, itemcounter, teller,
							   MF_BYPOSITION);
		}
	}
}


void    main_reset_window(HWND hWnd)
{
	HRGN    winregion;
	BITMAP  bm;
	GetObject(graphics.bmp_main_up, sizeof(bm), &bm);
	
	winregion =
		main_bitmap_to_region(graphics.bmp_main_up, Skin.transparentcolor);
	SetWindowPos(hWnd,  // handle to window
				 HWND_NOTOPMOST, // placement-order handle
				 0,  // horizontal position
				 0,  // vertical position
				 bm.bmWidth, // width
				 bm.bmHeight, // height
				 SWP_NOMOVE | SWP_NOZORDER // window-positioning flags
				);
	SetWindowRgn(hWnd, winregion, TRUE);
	CPI_Player__SetPositionRange(globals.m_hPlayer,
								 Skin.Object[PositionSlider].maxw ? Skin.Object[PositionSlider].h : Skin.Object[PositionSlider].w);
	RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
}

void    main_skin_add_to_menu(char *name)
{

	MENUITEMINFO menuinfo;
	int     itemcounter =
		GetMenuItemCount(GetSubMenu(globals.main_menu_popup, SKIN_SUBMENU_INDEX));
	int     teller;
	
	for (teller = 0; teller < itemcounter; teller++)
	{
		char    skinstring[MAX_PATH];
		GetMenuString(GetSubMenu(globals.main_menu_popup, SKIN_SUBMENU_INDEX), teller,
					  skinstring, MAX_PATH, MF_BYPOSITION);
		              
		if (strcmp(name, skinstring) == 0)
			return;
	}
	
	menuinfo.cbSize = sizeof(MENUITEMINFO);
	
	menuinfo.fMask = MIIM_TYPE | MIIM_ID;
	menuinfo.fType = MFT_STRING | MFT_RADIOCHECK;
	// menuinfo.fState;
	
	if (globals.main_int_skin_last_number == (5001 + options.remember_skin_count))
		globals.main_int_skin_last_number = 5001;
		
	menuinfo.wID = globals.main_int_skin_last_number++;
	
	// menuinfo.hSubMenu;
	// menuinfo.hbmpChecked;
	// menuinfo.hbmpUnchecked;
	// menuinfo.dwItemData;
	menuinfo.cch = sizeof(menuinfo.dwTypeData);
	
	menuinfo.dwTypeData = name;
	
	InsertMenuItem(globals.main_menu_popup, MENU_SKIN_DEFAULT, FALSE,
				   &menuinfo);
	               
	if (itemcounter > options.remember_skin_count)
	{
		RemoveMenu(GetSubMenu(globals.main_menu_popup, SKIN_SUBMENU_INDEX), 0,
				   MF_BYPOSITION);
	}
}

char   *str_trim(char *string)
{
	int     i = strlen(string) - 1;
	
	while (i >= 0 && string[i] == ' ')
		string[i--] = 0;
		
	while (*string == ' ')
		string++;
		
	return string;
}

DWORD   main_get_program_path(HINSTANCE hInst, LPTSTR pszBuffer,
							  DWORD dwSize)
//
//      Return the size of the path in bytes.
{
	DWORD   dwLength = GetModuleFileName(hInst, pszBuffer, dwSize);
	
	if (dwLength)
	{
		while (dwLength && pszBuffer[dwLength] != '\\')
		{
			dwLength--;
		}
		
		if (dwLength)
			pszBuffer[dwLength + 1] = '\0';
	}
	
	return dwLength;
}

//
//
//
int playlist_write()
{
	OPENFILENAME fn;
	BOOL bResult;
	char pcOutputName[MAX_PATH] = "";
	
	// Get filename to save
	fn.lStructSize = sizeof(OPENFILENAME);
	fn.hwndOwner = windows.m_hWndPlaylist;
	fn.hInstance = NULL;
	fn.lpstrFilter = "M3U Playlist Files (*.m3u)\0*.m3u\0PLS Playlist files (*.pls)\0*.pls\0";
	fn.lpstrCustomFilter = NULL;
	fn.nMaxCustFilter = 0;
	fn.nFilterIndex = 0;
	fn.lpstrFile = pcOutputName;
	fn.nMaxFile = MAX_PATH;
	fn.lpstrFileTitle = NULL;
	fn.nMaxFileTitle = 0;
	fn.lpstrInitialDir = options.last_used_directory;
	fn.lpstrTitle = NULL;
	fn.Flags = OFN_HIDEREADONLY
			   | OFN_EXPLORER
			   | OFN_OVERWRITEPROMPT
			   | OFN_PATHMUSTEXIST
			   | OFN_ENABLESIZING;
	fn.nFileOffset = 0;
	fn.nFileExtension = 0;
	fn.lpstrDefExt = "m3u";
	fn.lCustData = 0;
	fn.lpfnHook = NULL;
	fn.lpTemplateName = NULL;
	bResult = GetSaveFileName(&fn);
	
	if (bResult == FALSE)
		return FALSE;
		
	CPL_ExportPlaylist(globals.m_hPlaylist, pcOutputName);
	
	return TRUE;
}

//
//
//
void main_update_title_text()
{
	int     teller;
	HBITMAP h, h2;
	HPALETTE oldpal;
	const char* pcText;
	int     stringlen;
	int     width;
	CP_HPLAYLISTITEM hItem_Current;
	HDC SongtitleDc;
	int iG;
	RECT rect;
	
	iG = Skin.Object[SongtitleText].w;
	
	hItem_Current = CPL_GetActiveItem(globals.m_hPlaylist);
	
	if (hItem_Current)
		pcText = CPLI_GetTrackName(hItem_Current);
	else
		pcText = CP_COOLPLAYER;
		
	stringlen = strlen(pcText);
	
	width = ((stringlen + 4) * Skin.Object[SongtitleText].w);
	
	if (stringlen > Skin.Object[SongtitleText].maxw)
		globals.mail_int_title_scroll_max_position = width;
	else
		globals.mail_int_title_scroll_max_position = 0;
		
	if (width * 2 < Skin.Object[SongtitleText].w * Skin.Object[SongtitleText].maxw)
		width = Skin.Object[SongtitleText].w * Skin.Object[SongtitleText].maxw / 2;
		
	globals.main_int_title_scroll_position = 0;
	
	SongtitleDc = CreateCompatibleDC(drawables.dc_main);
	
	DeleteObject(graphics.bmp_main_title_area);
	
	graphics.bmp_main_title_area =
		CreateCompatibleBitmap(drawables.dc_main, width * 2,
							   Skin.Object[SongtitleText].h);
	                           
	h2 = SelectObject(SongtitleDc, graphics.bmp_main_title_area);
	oldpal = SelectPalette(SongtitleDc, graphics.pal_main, FALSE);
	
	RealizePalette(SongtitleDc);
	
	SetBkColor(SongtitleDc, Skin.transparentcolor);
	
	rect.top = rect.left = 0;
	rect.right = width * 2;
	rect.bottom = Skin.Object[SongtitleText].h;
	
// ExtTextOut(SongtitleDc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

	h = (HBITMAP) SelectObject(drawables.dc_memory, graphics.bmp_main_up);
	
	BitBlt(SongtitleDc, 0, 0,
		   (Skin.Object[SongtitleText].maxw +
			1) * Skin.Object[SongtitleText].w,
		   Skin.Object[SongtitleText].h, drawables.dc_memory,
		   Skin.Object[SongtitleText].x, Skin.Object[SongtitleText].y,
		   SRCCOPY);
	       
	BitBlt(SongtitleDc, Skin.Object[SongtitleText].w * stringlen, 0,
		   (Skin.Object[SongtitleText].maxw +
			1) * Skin.Object[SongtitleText].w,
		   Skin.Object[SongtitleText].h, drawables.dc_memory,
		   Skin.Object[SongtitleText].x, Skin.Object[SongtitleText].y,
		   SRCCOPY);
	       
	SelectObject(drawables.dc_memory, graphics.bmp_main_title_font);
	
	for (teller = 0; teller < stringlen; teller++)
	{
		BitBlt(SongtitleDc, (teller * Skin.Object[SongtitleText].w),
			   0, Skin.Object[SongtitleText].w,
			   Skin.Object[SongtitleText].h, drawables.dc_memory,
			   Skin.Object[SongtitleText].w * (pcText[teller] - 32), 0,
			   SRCCOPY);
		       
		if (stringlen > Skin.Object[SongtitleText].maxw)
			BitBlt(SongtitleDc, width + (teller * Skin.Object[SongtitleText].w), 0,
				   Skin.Object[SongtitleText].w,
				   Skin.Object[SongtitleText].h, drawables.dc_memory,
				   Skin.Object[SongtitleText].w * (pcText[teller] - 32), 0,
				   SRCCOPY);
	}
	
	//     TextOut(SongtitleDc,0,0,text,stringlen);
	SelectPalette(SongtitleDc, oldpal, FALSE);
	SelectObject(drawables.dc_memory, h);
	SelectObject(SongtitleDc, h2);
	
	DeleteDC(SongtitleDc);
	
	// Setup systray text
	if (globals.m_hSysIcon && hItem_Current)
	{
		// If there is a track name and artist name - set the format %artist% - %track%
		if (CPLI_GetTrackName(hItem_Current) && CPLI_GetArtist(hItem_Current))
		{
			char cBuffer[2060];
			sprintf(cBuffer, "%.1024s - %.1024s", CPLI_GetArtist(hItem_Current), CPLI_GetTrackName(hItem_Current));
			
			CPSYSICON_SetTipText(globals.m_hSysIcon, cBuffer);
		}
		
		else
			CPSYSICON_SetTipText(globals.m_hSysIcon, pcText);
	}
}

//
//
//
void    main_draw_title(HWND hWnd)
{
	// int     left = Skin.Object[SongtitleText].x;
	// int     top = Skin.Object[SongtitleText].y;
	window_bmp_transparent_blt(hWnd, graphics.bmp_main_title_area,
							   Skin.Object[SongtitleText].x,
							   Skin.Object[SongtitleText].y,
							   (Skin.Object[SongtitleText].maxw + 1) * Skin.Object[SongtitleText].w,
							   Skin.Object[SongtitleText].h,
							   0 + globals.main_int_title_scroll_position, 0);
}

void    main_draw_bitrate(HWND hWnd)
{
	int     left = Skin.Object[BitrateText].x;
	int     top = Skin.Object[BitrateText].y;
	
	if (left)
	{
		int     teller;
		
		for (teller = 0; globals.main_text_bitrate[teller]; teller++)
			window_bmp_transparent_blt(hWnd, graphics.bmp_main_title_font,
									   left + (teller * Skin.Object[SongtitleText].w),
									   top, Skin.Object[SongtitleText].w,
									   Skin.Object[SongtitleText].h,
									   Skin.Object[SongtitleText].w *
									   ((globals.main_text_bitrate[teller]) - 32), 0);
	}
}

void    main_draw_frequency(HWND hWnd)
{
	int     left = Skin.Object[FreqText].x;
	int     top = Skin.Object[FreqText].y;
	
	if (left)
	{
		int     teller;
		
		for (teller = 0; globals.main_text_frequency[teller]; teller++)
			window_bmp_transparent_blt(hWnd, graphics.bmp_main_title_font,
									   left + (teller * Skin.Object[SongtitleText].w),
									   top, Skin.Object[SongtitleText].w,
									   Skin.Object[SongtitleText].h,
									   Skin.Object[SongtitleText].w *
									   ((globals.main_text_frequency[teller]) - 32),
									   0);
	}
}

void    main_set_eq(void)
{
	CPI_Player__SetEQ(globals.m_hPlayer, options.equalizer, options.eq_settings);
}

void    main_draw_time(HWND hWnd)
{
	int     top = Skin.Object[TimeText].y;
	int     left = Skin.Object[TimeText].x;
	int     height = Skin.Object[TimeText].h;
	int     width = Skin.Object[TimeText].w;
	unsigned long hrs, minutes, seconds;
	
	unsigned long ss = globals.main_int_track_total_seconds;
	
	int     tracknr = 0;//globals.main_int_playlist_track_number;
	CP_HPLAYLISTITEM hCursor;
	
	for (hCursor = CPL_GetActiveItem(globals.m_hPlaylist); hCursor; hCursor = CPLI_Prev(hCursor))
		tracknr++;
		
	if (left)
	{
		if (options.show_remaining_time == TRUE)
			ss = (globals.main_long_track_duration - ss);
			
		seconds = ss % 60;
		ss /= 60;
		minutes = ss % 60;
		ss /= 60;
		hrs = ss;
		
		if (!tracknr && !ss)
			return;
			
		// hours
		window_bmp_transparent_blt(hWnd, graphics.bmp_main_time_font, left + width,
								   top, width, height, width * hrs, 0);
		                           
		// Separator
		window_bmp_transparent_blt(hWnd, graphics.bmp_main_time_font,
								   left + (2 * width), top, width, height, width * 10,
								   0);
		                           
		// minutes
		window_bmp_transparent_blt(hWnd, graphics.bmp_main_time_font,
								   left + (3 * width), top, width, height,
								   width * (minutes / 10), 0);
		                           
		window_bmp_transparent_blt(hWnd, graphics.bmp_main_time_font,
								   left + (4 * width), top, width, height,
								   width * (minutes % 10), 0);
		                           
		// Separator
		window_bmp_transparent_blt(hWnd, graphics.bmp_main_time_font,
								   left + (5 * width), top, width, height, width * 10,
								   0);
		                           
		// seconds
		window_bmp_transparent_blt(hWnd, graphics.bmp_main_time_font,
								   left + (6 * width), top, width, height,
								   width * (seconds / 10), 0);
		                           
		window_bmp_transparent_blt(hWnd, graphics.bmp_main_time_font,
								   left + (7 * width), top, width, height,
								   width * (seconds % 10), 0);
		                           
		if (options.show_remaining_time == TRUE)
			window_bmp_transparent_blt(hWnd, graphics.bmp_main_time_font, left, top,
									   width, height, width * 11, 0);
		else
			window_bmp_transparent_blt(hWnd, graphics.bmp_main_up, left, top, width,
									   height, left, top);
	}
}

void    main_draw_tracknr(HWND hWnd)
{
	int     top = Skin.Object[TrackText].y;
	int     left = Skin.Object[TrackText].x;
	int     width = Skin.Object[TrackText].w;
	int     height = Skin.Object[TrackText].h;
	int     nummertje;
	int     tracknr = 0;//globals.main_int_playlist_track_number;
	CP_HPLAYLISTITEM hCursor;
	
	for (hCursor = CPL_GetActiveItem(globals.m_hPlaylist); hCursor; hCursor = CPLI_Prev(hCursor))
		tracknr++;
		
	if (left)
	{
	
		if (tracknr)
		{
			nummertje = tracknr % 10;
			window_bmp_transparent_blt(hWnd, graphics.bmp_main_track_font,
									   left + (2 * width), top, width, height,
									   width * (nummertje), 0);
			nummertje = ((tracknr - nummertje) % 100);
			window_bmp_transparent_blt(hWnd, graphics.bmp_main_track_font, left + width,
									   top, width, height, width * (nummertje / 10), 0);
			nummertje = ((tracknr - nummertje) % 1000);
			window_bmp_transparent_blt(hWnd, graphics.bmp_main_track_font, left, top,
									   width, height, width * (nummertje / 100), 0);
		}
	}
}

BOOL    path_is_directory(char *filename)
{
	DWORD attribs = GetFileAttributes(filename);
	
	if (attribs & FILE_ATTRIBUTE_DIRECTORY)
		return TRUE;
	else
		return FALSE;
}

int     playlist_open_file(BOOL clearlist)
{
	OPENFILENAME fn;
	char filefilter[] =
		"All Supported files\0*.mp1;*.mp2;*.mp3;*.m3u;*.pls;*.wav;*.ogg\0"
		"MPEG audio files (*.mp1;*.mp2;*.mp3)\0*.mp1;*.mp2;*.mp3\0"
		"Vorbis files (*.ogg)\0*.ogg\0"
		"Playlist files (*.m3u;*.pls)\0*.m3u;*.pls\0"
		"WAV files (*.wav)\0*.wav\0"
		"All Files (*.*)\0*.*\0";
	BOOL    returnval;
	char    initialfilename[MAX_PATH * 200] = "";
	fn.lStructSize = sizeof(OPENFILENAME);
	fn.hwndOwner = (HWND)windows.wnd_main;
	fn.hInstance = NULL;
	fn.lpstrFilter = filefilter;
	fn.lpstrCustomFilter = NULL;
	fn.nMaxCustFilter = 0;
	fn.nFilterIndex = 0;
	fn.lpstrFile = initialfilename;
	fn.nMaxFile = MAX_PATH * 200;
	fn.lpstrFileTitle = NULL;
	fn.nMaxFileTitle = 0;
	fn.lpstrInitialDir = options.last_used_directory;
	fn.lpstrTitle = NULL;
	fn.Flags =
		OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_EXPLORER |
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ENABLESIZING;
	fn.nFileOffset = 0;
	fn.nFileExtension = 0;
	fn.lpstrDefExt = NULL;
	fn.lCustData = 0;
	fn.lpfnHook = NULL;
	fn.lpTemplateName = NULL;
	returnval = GetOpenFileName(&fn);
	
	if (returnval != FALSE)
	{
		char   *newfilename;
		char    path_buffer[_MAX_PATH];
		char    path_buffer2[_MAX_PATH];
		
		if (clearlist)
			CPL_Empty(globals.m_hPlaylist);
			
		strcpy(path_buffer, fn.lpstrFile);
		
		if (path_is_directory(fn.lpstrFile) == TRUE)
		{
			path_add_backslash(path_buffer);
		}
		
		else
		{
			path_remove_filespec(path_buffer);
		}
		
		strcpy(options.last_used_directory, path_buffer);
		
		newfilename = fn.lpstrFile + fn.nFileOffset;
		
		while (newfilename[0] != 0)
		{
			strcpy(path_buffer2, path_buffer);
			strcat(path_buffer2, newfilename);
			CPL_SyncLoadNextFile(globals.m_hPlaylist);
			CPL_AddFile(globals.m_hPlaylist, path_buffer2);
			newfilename = newfilename + strlen(newfilename) + 1;
		}
		
		return 1;
	}
	
	return 0;
}

void    main_draw_vu_from_mouse(HWND hWnd, int vunummer, int vuwaarde)
{

	if (Skin.Object[vunummer].maxw == 1) // Vertical Slider
	{
		if (Skin.Object[vunummer].x2 != 0)
		{
			window_bmp_blt(hWnd, graphics.bmp_main_up,
						   Skin.Object[vunummer].x,
						   Skin.Object[vunummer].y,
						   Skin.Object[vunummer].w,
						   Skin.Object[vunummer].h +
						   Skin.Object[vunummer].h2,
						   Skin.Object[vunummer].x,
						   Skin.Object[vunummer].y);
			               
			window_bmp_blt(hWnd, graphics.bmp_main_down,
						   Skin.Object[vunummer].x,
						   vuwaarde,
						   Skin.Object[vunummer].w2,
						   Skin.Object[vunummer].h2,
						   Skin.Object[vunummer].x2,
						   Skin.Object[vunummer].y2);
			return;
		}
		
		window_bmp_blt(hWnd, graphics.bmp_main_up,
		
					   Skin.Object[vunummer].x,
					   vuwaarde,
					   Skin.Object[vunummer].w,
					   Skin.Object[vunummer].h - (vuwaarde -
												  Skin.Object[vunummer].y),
					   Skin.Object[vunummer].x, vuwaarde);
		               
		window_bmp_blt(hWnd, graphics.bmp_main_down,
					   Skin.Object[vunummer].x,
					   Skin.Object[vunummer].y,
					   Skin.Object[vunummer].w,
					   vuwaarde - Skin.Object[vunummer].y,
					   Skin.Object[vunummer].x, Skin.Object[vunummer].y);
	}
	
	else   // Horizontal Slider
	{
		if (Skin.Object[vunummer].x2 != 0)
		{
			window_bmp_blt(hWnd, graphics.bmp_main_up,
						   Skin.Object[vunummer].x,
						   Skin.Object[vunummer].y,
						   Skin.Object[vunummer].w +
						   Skin.Object[vunummer].w2,
						   Skin.Object[vunummer].h,
						   Skin.Object[vunummer].x,
						   Skin.Object[vunummer].y);
			               
			window_bmp_blt(hWnd, graphics.bmp_main_down,
						   vuwaarde,
						   Skin.Object[vunummer].y,
						   Skin.Object[vunummer].w2,
						   Skin.Object[vunummer].h2,
						   Skin.Object[vunummer].x2,
						   Skin.Object[vunummer].y2);
			               
			return;
		}
		
		window_bmp_blt(hWnd, graphics.bmp_main_up,
		
					   Skin.Object[vunummer].x,
					   Skin.Object[vunummer].y,
					   vuwaarde - Skin.Object[vunummer].x,
					   Skin.Object[vunummer].h,
					   Skin.Object[vunummer].x, Skin.Object[vunummer].y);
		               
		window_bmp_blt(hWnd, graphics.bmp_main_down,
					   vuwaarde,
					   Skin.Object[vunummer].y,
					   Skin.Object[vunummer].w - (vuwaarde -
												  Skin.Object[vunummer].x),
					   Skin.Object[vunummer].h, vuwaarde,
					   Skin.Object[vunummer].y);
	}
}

void    main_draw_vu_from_value(HWND hWnd, int vunummer, int vuwaarde)
{
	int     positionwaarde;
	
	if (Skin.Object[vunummer].maxw == 1) // Vertical slider
	{
		switch (vunummer)
		{
		
			case VolumeSlider:
				positionwaarde =
					(int)((float) Skin.Object[vunummer].y +
						  Skin.Object[vunummer].h -
						  (((float) vuwaarde / (float) 100.0f) *
						   (float) Skin.Object[vunummer].h));
				break;
				
			case PositionSlider:
				positionwaarde =
					(int)(Skin.Object[PositionSlider].y +
						  Skin.Object[PositionSlider].h) - vuwaarde;
				          
				if (vuwaarde > Skin.Object[PositionSlider].h)
					positionwaarde = Skin.Object[PositionSlider].y;
					
				break;
				
			default:  // so it's a eq
				positionwaarde =
					(int)((float) Skin.Object[vunummer].y +
						  Skin.Object[vunummer].h -
						  (
							  ((float)(vuwaarde + 128.0f) /
							   (float) 255.0f) * (float) Skin.Object[vunummer].h));
		}
		
		main_draw_vu_from_mouse(hWnd, vunummer, positionwaarde);
	}
	
	else   // Horizontal Slider
	{
		switch (vunummer)
		{
		
			case VolumeSlider:
				positionwaarde =
					(int)((float) Skin.Object[vunummer].x +
						  (((float) vuwaarde / (float) 100.0f) *
						   (float) Skin.Object[vunummer].w));
				break;
				
			case PositionSlider:
				positionwaarde =
					(int) vuwaarde + Skin.Object[PositionSlider].x;
				    
				if (vuwaarde > Skin.Object[PositionSlider].w)
					positionwaarde =
						Skin.Object[PositionSlider].w +
						Skin.Object[PositionSlider].x;
					    
				break;
				
			default:  // so it's a eq
				positionwaarde =
					(int)((float) Skin.Object[vunummer].x +
						  (
							  ((float)(vuwaarde + 128.0) /
							   (float) 255.0f) * (float) Skin.Object[vunummer].w));
		}
		
		main_draw_vu_from_mouse(hWnd, vunummer, positionwaarde);
		
	}
}

BOOL    main_draw_vu_all(HWND hWnd, WPARAM wParam, LPARAM lParam,
						 BOOL rememberlastval)
{
	POINTS  cursorpos;
	int     teller;
	int     moveit = TRUE;
	
	cursorpos = MAKEPOINTS(lParam);
	
	for (teller = VolumeSlider; teller <= Eq8; teller++)
	{
	
		int  knobx = 0, knoby = 0;
		int  addx = 0, addy = 0;
		
		if (Skin.Object[teller].maxw == 0)   // we have  a horizontal one
		{
			knobx = Skin.Object[teller].w2 / 2;
			addx = 1;
		}
		
		else   // we have  a vertical one
		{
			knoby = Skin.Object[teller].h2 / 2;
			addy = 1;
		}
		
		if (cursorpos.x >= Skin.Object[teller].x + knobx - addx
				&& cursorpos.y >= Skin.Object[teller].y + knoby - addy
				&& cursorpos.x <=
				Skin.Object[teller].x + Skin.Object[teller].w + (knobx*2) + addx
				&& cursorpos.y <=
				Skin.Object[teller].y + Skin.Object[teller].h + (knoby*2) + addy)
		{
			int waarde = 0;
			
			if (Skin.Object[teller].maxw == 0)
				cursorpos.x -= knobx;
			else
				cursorpos.y -= knoby;
				
				
			moveit = FALSE;
			
			switch (teller)
			{
			
				case Eq1:	// FALLTHROUGH
				case Eq2:
				case Eq3:
				case Eq4:
				case Eq5:
				case Eq6:
				case Eq7:
				case Eq8:
				
					if (globals.main_bool_slider_keep_focus == FALSE)
					{
						if (Skin.Object[teller].maxw == 1)
							options.eq_settings[(teller - Eq1) + 1] =
								(int)(
									((Skin.Object
									  [teller].y +
									  Skin.Object[teller].h) -
									 cursorpos.y) * (255 /
													 (float)
													 Skin.Object[teller].
													 h)) - 128;
						else
							options.eq_settings[(teller - Eq1) + 1] =
								(int)((cursorpos.x - Skin.Object[teller].x) *
									  (255 / (float) Skin.Object[teller].w)) -
								128;
							    
						if (options.eq_settings[(teller - Eq1) + 1] > 127) options.eq_settings[(teller - Eq1) + 1] = 127;
						
						if (options.eq_settings[(teller - Eq1) + 1] < -128)   options.eq_settings[(teller - Eq1) + 1] = -128;
						
						waarde = options.eq_settings[(teller - Eq1) + 1];
						
						main_set_eq();
					}
					
					break;
					
				case VolumeSlider:
				{
					globals.main_bool_slider_keep_focus = TRUE;
					
					if (Skin.Object[teller].maxw == 1) // we have  a vertical one
					{
						globals.m_iVolume = (int)(((100.0f / (float)
													Skin.Object[teller].h) *
												   ((Skin.Object[teller].y +
													 Skin.Object[teller].h) -
													cursorpos.y)));
					}
					
					else
					{
						globals.m_iVolume =
							(int)((cursorpos.x - Skin.Object[teller].x) *
								  (100 / (float) Skin.Object[teller].w));
					}
					
					if (globals.m_iVolume > 100) globals.m_iVolume = 100;
					
					if (globals.m_iVolume < 0) globals.m_iVolume = 0;
					
					//    CP_TRACE1("level=%d",globals.m_iVolume);
					CPI_Player__SetVolume(globals.m_hPlayer, globals.m_iVolume);
					
					waarde = globals.m_iVolume;
					
					break;
				}
				
				case PositionSlider:
				
					if (globals.m_bStreaming == TRUE)
					{
						waarde = globals.m_iStreamingPortion;
					}
					
					else if (globals.main_bool_slider_keep_focus == FALSE)
					{
						if (Skin.Object[teller].maxw == 0)
						{
							globals.main_int_track_position = cursorpos.x - Skin.Object[teller].x;
							CPI_Player__Seek(globals.m_hPlayer, globals.main_int_track_position, Skin.Object[teller].w);
						}
						
						else
						{
							globals.main_int_track_position = ((Skin.Object[teller].y + Skin.Object[teller].h) - cursorpos.y);
							CPI_Player__Seek(globals.m_hPlayer, globals.main_int_track_position, Skin.Object[teller].h);
						}
						
						waarde = globals.main_int_track_position;
					}
					
					break;
			}
			
			
			//  main_draw_vu_from_mouse(hWnd, teller, Skin.Object[teller].maxw ? cursorpos.y : cursorpos.x);
			main_draw_vu_from_value(hWnd, teller, waarde);
		}
	}
	
	return moveit;
}

int window_bmp_blt(HWND hWnd, HBITMAP SrcBmp, int srcx, int srcy, int srcw, int srch, int dstx, int dsty)
{
	if (srcw && srch)
	{
		HBITMAP h = (HBITMAP) SelectObject(drawables.dc_memory, SrcBmp);
		int     retval = BitBlt(drawables.dc_main, srcx, srcy, srcw, srch,
								drawables.dc_memory, dstx, dsty,
								SRCCOPY);
		SelectObject(drawables.dc_memory, h);
		return retval;
	}
	
	return FALSE;
}

int window_bmp_transparent_blt(HWND hWnd, HBITMAP SrcBmp, int srcx, int srcy, int srcw, int srch, int dstx, int dsty)
{
	return window_bmp_blt(hWnd,  SrcBmp,  srcx,  srcy,  srcw,  srch,  dstx, dsty);
}



void    main_draw_controls_all(HWND hWnd)
{
	int     teller;
	
	for (teller = PlaySwitch; teller <= ExitButton; teller++)
	{
		window_bmp_blt(hWnd, graphics.bmp_main_up, Skin.Object[teller].x,
					   Skin.Object[teller].y, Skin.Object[teller].w,
					   Skin.Object[teller].h, Skin.Object[teller].x,
					   Skin.Object[teller].y);
		window_bmp_blt(hWnd, graphics.bmp_main_up, Skin.Object[teller].x2,
					   Skin.Object[teller].y2, Skin.Object[teller].w2,
					   Skin.Object[teller].h2, Skin.Object[teller].x2,
					   Skin.Object[teller].y2);
		               
		switch (teller)
		{
		
			case RepeatSwitch:
			
				if (options.repeat_playlist)
					window_bmp_blt(hWnd, graphics.bmp_main_switch,
								   Skin.Object[RepeatSwitch].x2,
								   Skin.Object[RepeatSwitch].y2,
								   Skin.Object[RepeatSwitch].w2,
								   Skin.Object[RepeatSwitch].h2,
								   Skin.Object[RepeatSwitch].x2,
								   Skin.Object[RepeatSwitch].y2);
					               
				break;
				
			case ShuffleSwitch:
				if (options.shuffle_play)
					window_bmp_blt(hWnd, graphics.bmp_main_switch,
								   Skin.Object[ShuffleSwitch].x2,
								   Skin.Object[ShuffleSwitch].y2,
								   Skin.Object[ShuffleSwitch].w2,
								   Skin.Object[ShuffleSwitch].h2,
								   Skin.Object[ShuffleSwitch].x2,
								   Skin.Object[ShuffleSwitch].y2);
					               
				break;
				
			case EqSwitch:
				if (options.equalizer)
					window_bmp_blt(hWnd, graphics.bmp_main_switch,
								   Skin.Object[EqSwitch].x2,
								   Skin.Object[EqSwitch].y2,
								   Skin.Object[EqSwitch].w2,
								   Skin.Object[EqSwitch].h2,
								   Skin.Object[EqSwitch].x2,
								   Skin.Object[EqSwitch].y2);
					               
				break;
				
			case PlaySwitch:
				if (globals.m_enPlayerState == cppsPlaying)
					window_bmp_blt(hWnd, graphics.bmp_main_switch,
								   Skin.Object[teller].x2,
								   Skin.Object[teller].y2,
								   Skin.Object[teller].w2,
								   Skin.Object[teller].h2,
								   Skin.Object[teller].x2,
								   Skin.Object[teller].y2);
					               
				break;
				
			case PauseSwitch:
				if (globals.m_enPlayerState == cppsPaused)
					window_bmp_blt(hWnd, graphics.bmp_main_switch,
								   Skin.Object[teller].x2,
								   Skin.Object[teller].y2,
								   Skin.Object[teller].w2,
								   Skin.Object[teller].h2,
								   Skin.Object[teller].x2,
								   Skin.Object[teller].y2);
					               
				break;
				
			case StopSwitch:
				if (globals.m_enPlayerState == cppsStopped)
					window_bmp_blt(hWnd, graphics.bmp_main_switch,
								   Skin.Object[teller].x2,
								   Skin.Object[teller].y2,
								   Skin.Object[teller].w2,
								   Skin.Object[teller].h2,
								   Skin.Object[teller].x2,
								   Skin.Object[teller].y2);
					               
				break;
		}
	}
}

void    options_create(HWND hWnd)
{
	windows.dlg_options = DialogBox(GetModuleHandle(NULL), // handle to application instance
									MAKEINTRESOURCE(IDD_OPTIONS), // identifies dialog box template
									hWnd, // handle to owner window
									(DLGPROC) options_windowproc); // pointer to dialog box procedure
	ShowWindow(windows.dlg_options, SW_SHOWNORMAL);
}


void    url_create(HWND hWnd)
{
	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_URL), hWnd, (DLGPROC) url_windowproc);  
}

void    main_menuproc(HWND hWnd, LPPOINT points)
{
	int     retval;
	/* SetForegroundWindow and the ensuing null PostMessage is a
	   workaround for a Windows 95 bug (see MSKB article Q135788,
	   http://www.microsoft.com/kb/articles/q135/7/88.htm, I think).
	   In typical Microsoft style this bug is listed as "by design".
	   SetForegroundWindow also causes our MessageBox to pop up in front
	   of any other application's windows. */
	SetForegroundWindow(hWnd);
	/* We specifiy TPM_RETURNCMD, so TrackPopupMenu returns the menu
	   selection instead of returning immediately and our getting a
	   WM_COMMAND with the selection. You don't have to do it this way.
	 */
	
	switch (retval =
				TrackPopupMenu(globals.main_menu_popup,
							   TPM_RETURNCMD | TPM_RIGHTBUTTON, points->x,
							   points->y, 0, hWnd, NULL)) // LPRECT user can click in
		// without dismissing menu
	{
	
		case MENU_EXIT:
			CPVERB_Exit(vaDoVerb, hWnd);
			// DestroyWindow(hWnd);
			break;
			
		case MENU_PLAYLIST:
			CPVERB_TogglePlaylistWindow(vaDoVerb, hWnd);
			break;
			
		case MENU_OPENFILE:
			main_play_control(ID_LOAD, hWnd);
			break;
			
		case MENU_ADDFILE:
			CPVERB_AddFile(vaDoVerb, hWnd);
			break;
			
		case MENU_ABOUT:
			about_create(hWnd);
			break;
			
		case MENU_OPENLOC:
			url_create(hWnd);
			break;
			
		case MENU_OPTIONS:
			options_create(hWnd);
			break;
			
		case MENU_SKIN_DEFAULT:
		{
		
			options.use_default_skin = TRUE;
			globals.main_bool_skin_next_is_default = TRUE;
			main_play_control(ID_LOADSKIN, hWnd);
			break;
		}
		
		default:
		{
			if (main_play_control((WORD) retval, hWnd) != -1)
				break;
				
			if (GetMenuString
					(globals.main_menu_popup, retval, NULL, 0, MF_BYCOMMAND))
			{
				GetMenuString(globals.main_menu_popup, retval,
							  options.main_skin_file, MAX_PATH,
							  MF_BYCOMMAND);
				main_skin_select_menu(options.main_skin_file);
				options.use_default_skin = FALSE;
				globals.main_bool_skin_next_is_default = FALSE;
				main_play_control(ID_LOADSKIN, hWnd);
			}
			
			break;
		}
	}
	
	PostMessage(hWnd, 0, 0, 0); // see above
}

LRESULT CALLBACK
main_windowproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Give the player a crack at the messages - it needs these so that it can call the notification callbacks
	if (globals.m_hPlayer)
	{
		LRESULT lResult;
		const BOOL bHandled = CPI_Player__HandleNotifyMessages(globals.m_hPlayer, message, wParam, lParam, &lResult);
		
		if (bHandled == TRUE)
			return lResult;
	}
	
	switch (message)
	{
			POINTS  cursorpos;
			
		case WM_TIMER:
		
			if (wParam == CPC_TIMERID_SCROLLTITLETEXT)
			{
				main_draw_title(hWnd);
				
				if (globals.mail_int_title_scroll_max_position == 0)
					globals.main_int_title_scroll_position = 0;
				else
				{
					globals.main_int_title_scroll_position++;
					
					if (globals.main_int_title_scroll_position > globals.mail_int_title_scroll_max_position)
						globals.main_int_title_scroll_position -= globals.mail_int_title_scroll_max_position;
				}
			}
			
			else if (wParam == CPC_TIMERID_INTERTRACKDELAY)
			{
				KillTimer(hWnd, CPC_TIMERID_INTERTRACKDELAY);
				CPL_PlayItem(globals.m_hPlaylist, FALSE, pmNextItem);
			}
			
			else if (wParam == CPC_TIMERID_ROTATINGSMILY)
			{
				CPSYSICON_AdvanceFrame(globals.m_hSysIcon);
			}
			
			break;
			
		case WM_MOUSEWHEEL:
		{
			short zDelta = (short) HIWORD(wParam);
			
			if (zDelta < 0)
				globals.m_iVolume -= 5;
			else
				globals.m_iVolume += 5;
				
			if (globals.m_iVolume > 100)
				globals.m_iVolume = 100;
				
			if (globals.m_iVolume < 0)
				globals.m_iVolume = 0;
				
			main_draw_vu_from_value(hWnd, VolumeSlider, globals.m_iVolume);
			
			CPI_Player__SetVolume(globals.m_hPlayer, globals.m_iVolume);
			
			return 0;
		}
		
		case WM_COPYDATA:
		{
			PCOPYDATASTRUCT copydata = (PCOPYDATASTRUCT) lParam;
			int   count = 0;
			BOOL    wegotsome;
			char *string = copydata->lpData;
			int argc = copydata->dwData;
			char *argv[255] = {0};
			
			while (count < argc)
			{
				argv[count++] = string;
				string += strlen(string) + 1;
			}
			
			cmdline_parse_options(argc, argv, hWnd);
			
			wegotsome = cmdline_parse_files(argc, argv);
			
			if (globals.playlist_bool_addsong == FALSE
					&& wegotsome == TRUE)
			{
				CPL_PlayItem(globals.m_hPlaylist, TRUE, pmCurrentItem);
			}
			
			main_draw_controls_all(hWnd);
			
			main_set_eq();
			globals.playlist_bool_addsong = FALSE;
			globals.playlist_last_add_time = GetTickCount();
			break;
		}
		
		case WM_NOTIFYICON:
		
			switch (lParam)
			{
			
				case WM_RBUTTONUP:
				{
					POINT   pt;
					GetCursorPos(&pt);
					main_menuproc(hWnd, &pt);
				}
				
				break;
				
				case WM_MBUTTONUP:
				
					if (globals.m_enPlayerState == cppsPlaying || globals.m_enPlayerState == cppsPaused)
						CPVERB_Pause(vaDoVerb, hWnd);
						
					break;
					
				case WM_LBUTTONUP:
				{
					RECT    rcClip;
					RECT    rcClient;
					int     Result;
					GetClientRect(hWnd, &rcClient);
					Result = GetClipBox(drawables.dc_main, &rcClip);
					
					if (Result == COMPLEXREGION
							&& EqualRect(&rcClip, &rcClient))
					{
						if (options.show_on_taskbar)
							ShowWindow(hWnd, SW_MINIMIZE);
						else
							ShowWindow(hWnd, SW_HIDE);
					}
					
					else
					{
						ShowWindow(hWnd, SW_RESTORE);
					}
				}
				
				BringWindowToTop(hWnd);
				
				SetForegroundWindow(hWnd);
				break;
			}
			
			break;
			
		case WM_CREATE:
		
			if (options.scroll_track_title)
				SetTimer(hWnd, CPC_TIMERID_SCROLLTITLETEXT, 50, NULL);
			else
				KillTimer(hWnd, CPC_TIMERID_SCROLLTITLETEXT);
				
			SetTimer(hWnd, CPC_TIMERID_ROTATINGSMILY, 100, NULL);
			
			break;
			
		case WM_RBUTTONDOWN:
		{
			POINT points;
			GetCursorPos(&points);
			
			main_menuproc(hWnd, &points);
			break;
		}
		
		case WM_CAPTURECHANGED:
		
			ReleaseCapture();
			break;
			
		case WM_LBUTTONDOWN:
		{
			int teller;
			BOOL moveit = TRUE;
			SetCapture(hWnd);
			cursorpos = MAKEPOINTS(lParam);
			
			for (teller = 0; teller <= ExitButton; teller++)
			{
				if (cursorpos.x >= Skin.Object[teller].x
						&& cursorpos.y >= Skin.Object[teller].y
						&& cursorpos.x <=
						Skin.Object[teller].x + Skin.Object[teller].w
						&& cursorpos.y <=
						Skin.Object[teller].y + Skin.Object[teller].h)
				{
					window_bmp_blt(hWnd, graphics.bmp_main_down,
								   Skin.Object[teller].x,
								   Skin.Object[teller].y,
								   Skin.Object[teller].w,
								   Skin.Object[teller].h,
								   Skin.Object[teller].x,
								   Skin.Object[teller].y);
					moveit = FALSE;
					return 0;
				}
			}
			
			if (cursorpos.x >= Skin.Object[TimeText].x
					&& cursorpos.y >= Skin.Object[TimeText].y
					&& cursorpos.x <=
					Skin.Object[TimeText].x + (Skin.Object[TimeText].w * 8)
					&& cursorpos.y <=
					Skin.Object[TimeText].y + Skin.Object[TimeText].h)
			{
				moveit = FALSE;
			}
			
			// id3tag editor
			/*if (cursorpos.x >= Skin.Object[SongtitleText].x
			        && cursorpos.y >= Skin.Object[SongtitleText].y
			        && cursorpos.x <=
			        Skin.Object[SongtitleText].x +
			        (Skin.Object[SongtitleText].w *
			         Skin.Object[SongtitleText].maxw)
			        && cursorpos.y <=
			        Skin.Object[SongtitleText].y +
			        Skin.Object[SongtitleText].h) {
			    moveit = FALSE;
			}*/
			// VU & volume
			
			if (main_draw_vu_all(hWnd, wParam, lParam, TRUE) == FALSE)
				moveit = FALSE;
				
			// Move Window
			if ((cursorpos.x >= Skin.Object[MoveArea].x
					&& cursorpos.y >= Skin.Object[MoveArea].y
					&& cursorpos.x <= Skin.Object[MoveArea].x + Skin.Object[MoveArea].w
					&& cursorpos.y <= Skin.Object[MoveArea].y + Skin.Object[MoveArea].h)
					|| (moveit == TRUE && options.easy_move == TRUE))
			{
				ReleaseCapture();
				SendMessage(hWnd, WM_SYSCOMMAND, SC_MOVE | HTCLIENT, 0);
			}
			
			break;
		}
		
		case WM_MOUSEMOVE:
		{
			MSG     msg;
			
			if (wParam == MK_LBUTTON)
				main_draw_vu_all(hWnd, wParam, lParam, FALSE);
				
			msg.lParam = lParam;
			msg.wParam = wParam;
			msg.message = message;
			msg.hwnd = hWnd;
			
			SendMessage(windows.wnd_tooltip, TTM_RELAYEVENT, 0,
						(LPARAM)(LPMSG) & msg);
		}
		
		break;
		
		case WM_CANCELMODE:
			ReleaseCapture();
			break;
			
		case WM_LBUTTONUP:
		{
			int     teller;
			ReleaseCapture();
			globals.main_bool_slider_keep_focus = FALSE;
			cursorpos = MAKEPOINTS(lParam);
			
			for (teller = PlaySwitch; teller <= ExitButton; teller++)
			{
				if (cursorpos.x >= Skin.Object[teller].x
						&& cursorpos.y >= Skin.Object[teller].y
						&& cursorpos.x <=
						Skin.Object[teller].x + Skin.Object[teller].w
						&& cursorpos.y <=
						Skin.Object[teller].y + Skin.Object[teller].h)
				{
					switch (teller)
					{
					
						case PlaySwitch:
							main_play_control(ID_PLAY, hWnd);
							break;
							
						case PauseSwitch:
							main_play_control(ID_PAUSE, hWnd);
							break;
							
						case StopSwitch:
							main_play_control(ID_STOP, hWnd);
							break;
							
						case RepeatSwitch:
							main_play_control(ID_REPEAT, hWnd);
							break;
							
						case ShuffleSwitch:
							main_play_control(ID_SHUFFLE, hWnd);
							break;
							
						case EqSwitch:
							main_play_control(ID_EQUALIZER, hWnd);
							break;
							
						case PlaylistButton:
							main_play_control(ID_PLAYLIST, hWnd);
							break;
							
						case NextButton:
							main_play_control(ID_NEXT, hWnd);
							break;
							
						case PrevButton:
							main_play_control(ID_PREVIOUS, hWnd);
							break;
							
						case MinimizeButton:
						
							if (options.show_on_taskbar)
								ShowWindow(hWnd, SW_MINIMIZE);
							else
								ShowWindow(hWnd, SW_HIDE);
								
							break;
							
						case NextSkinButton:
							main_play_control(ID_LOADSKIN, hWnd);
							
							break;
							
						case ExitButton:
							CPVERB_Exit(vaDoVerb, hWnd);
							// DestroyWindow(hWnd);
							break;
							
						case EjectButton:
							main_play_control(ID_LOAD, hWnd);
							
							break;
					}
				}
			}
			
			// options.show_remaining_time time
			
			if (cursorpos.x >= Skin.Object[TimeText].x
					&& cursorpos.y >= Skin.Object[TimeText].y
					&& cursorpos.x <=
					(Skin.Object[TimeText].x + (Skin.Object[TimeText].w * 8))
					&& cursorpos.y <=
					(Skin.Object[TimeText].y + Skin.Object[TimeText].h))
			{
				options.show_remaining_time = !options.show_remaining_time;
				main_draw_time(hWnd);
				break;
			}
			
			main_draw_controls_all(hWnd);
			
			break;
		}
		
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			// HDC     winhdc = BeginPaint(hWnd, &ps);
			BeginPaint(hWnd, &ps);
			
			if (graphics.bmp_main_up)
			{
				BITMAP  bm;
				int     teller, teller2 = 1;
				
				HPALETTE oldpal;
				oldpal =
					SelectPalette(drawables.dc_main, graphics.pal_main,
								  FALSE);
				RealizePalette(drawables.dc_main);
				
				GetObject(graphics.bmp_main_up, sizeof(bm), &bm);
				window_bmp_blt(hWnd, graphics.bmp_main_up, ps.rcPaint.left,
							   ps.rcPaint.top, ps.rcPaint.right,
							   ps.rcPaint.bottom, ps.rcPaint.left,
							   ps.rcPaint.top);
				               
				for (teller = Eq1; teller <= Eq8; teller++)
					main_draw_vu_from_value(hWnd, teller,
											options.eq_settings
											[teller2++]);
					                        
				main_draw_vu_from_value(hWnd, VolumeSlider, globals.m_iVolume);
				
				if (globals.m_bStreaming == TRUE)
					main_draw_vu_from_value(windows.wnd_main, PositionSlider, globals.m_iStreamingPortion);
				else
					main_draw_vu_from_value(windows.wnd_main, PositionSlider, globals.main_int_track_position);
					
				main_draw_tracknr(hWnd);
				main_draw_title(hWnd);
				main_draw_time(hWnd);
				main_draw_bitrate(hWnd);
				main_draw_frequency(hWnd);
				main_draw_controls_all(hWnd);

				SelectPalette(drawables.dc_main, oldpal, FALSE);
			}
			
			EndPaint(hWnd, &ps);
			
			return 0;
		}
		
		case WM_MOVE:
			options.main_window_pos.x = (int)(short) LOWORD(lParam);  // horizontal position
			options.main_window_pos.y = (int)(short) HIWORD(lParam);  // vertical position
			return 0;
			
		case WM_SYSKEYDOWN:
		
		case WM_KEYDOWN:
		{
			const BOOL bAltIsDown = (GetAsyncKeyState(VK_MENU)  & 0x8000) ? TRUE : FALSE;
			const BOOL bCtrlIsDown = (GetAsyncKeyState(VK_CONTROL)  & 0x8000) ? TRUE : FALSE;
			const BOOL bShiftIsDown = (GetAsyncKeyState(VK_SHIFT)  & 0x8000) ? TRUE : FALSE;
			CP_HandleKeyPress_Player(hWnd, (int)wParam, bAltIsDown, bCtrlIsDown, bShiftIsDown);
		}
		
		return 0;
		
		case WM_DESTROY:
		
			if (options.remember_playlist == TRUE)
				playlist_write_default();
				
			CPlaylistWindow_Destroy();
			
			//  CPlayerWindow_Destroy();
			options_write();
			
			CPL_DestroyPlaylist(globals.m_hPlaylist);
			CPI_Player__Destroy(globals.m_hPlayer);
			
			CPIC_FreeIndicators();
			
#if _DEBUG
			globals.m_hPlaylist = NULL;
			globals.m_hPlayer = NULL;
			
#endif
			PostQuitMessage(0);
			
			break;
			
		case WM_COMMAND:
		{
			int     accelreturn = main_play_control(LOWORD(wParam), hWnd);
			main_draw_controls_all(hWnd);
			return accelreturn;
		}
		
		// Set the global window handle and handle normally
		
		case WM_NCCREATE:
			windows.wnd_main = hWnd;
			
			return DefWindowProc(hWnd, message, wParam, lParam);
			
			break;
			
		case WM_DROPFILES:
		{
			const BOOL bCtrlIsDown = (GetAsyncKeyState(VK_CONTROL)  & 0x8000) ? TRUE : FALSE;
			HDROP hDrop = (HDROP)wParam;
			
			// Replace the current list by default - append if CTRL is down
			CPL_SyncLoadNextFile(globals.m_hPlaylist);
			
			if (bCtrlIsDown == FALSE)
			{
				CPL_Empty(globals.m_hPlaylist);
				globals.m_enPlayerState = cppsStopped;
				CPL_AddDroppedFiles(globals.m_hPlaylist, hDrop);
				CPL_PlayItem(globals.m_hPlaylist, TRUE, pmCurrentItem);
			}
			
			else
				CPL_AddDroppedFiles(globals.m_hPlaylist, hDrop);
		}
		
		case WM_HOTKEY:
		
		{
			const int iIDHotKey = (int)wParam;
			
			if (iIDHotKey == CP_HOTKEY_NEXT)
				CPVERB_NextTrack(vaDoVerb, hWnd);
				
			if (iIDHotKey == CP_HOTKEY_PREV)
				CPVERB_PrevTrack(vaDoVerb, hWnd);
				
			if (iIDHotKey == CP_HOTKEY_STOP)
				CPVERB_Stop(vaDoVerb, hWnd);
				
			if (iIDHotKey == CP_HOTKEY_PLAY0 || iIDHotKey == CP_HOTKEY_PLAY1)
			{
				if (globals.m_enPlayerState == cppsPlaying)
					CPVERB_Pause(vaDoVerb, hWnd);
				else
					CPVERB_Play(vaDoVerb, hWnd);
			}
			
		}
				
		case WM_INITMENUPOPUP:
		{
			CheckMenuItem(globals.main_menu_popup, MENU_PLAYLIST, MF_BYCOMMAND | (options.show_playlist ? MF_CHECKED : 0));
			return 0;
		}
		
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	
	return (0);
}

int     main_play_control(WORD wParam, HWND hWnd)
{
	switch (wParam)
	{
	
		case ID_PLAYLIST:
			CPVERB_TogglePlaylistWindow(vaDoVerb, hWnd);
			return !options.show_playlist;
			break;
			
		case ID_REPEAT:
			CPVERB_ToggleRepeat(vaDoVerb, hWnd);
			return !options.shuffle_play;
			
		case ID_EXIT:
			CPVERB_Exit(vaDoVerb, hWnd);
			break;
			
		case ID_SHUFFLE:
			CPVERB_ToggleShuffle(vaDoVerb, hWnd);
			return !options.shuffle_play;
			
		case ID_EQUALIZER:
			CPVERB_ToggleEqualiser(vaDoVerb, hWnd);
			return !options.equalizer;
			
		case ID_STOP:
			CPVERB_Stop(vaDoVerb, hWnd);
			break;
			
		case ID_PLAY:
			CPVERB_Play(vaDoVerb, hWnd);
			break;
			
		case ID_PAUSE:
			CPVERB_Pause(vaDoVerb, hWnd);
			return 0;
			
		case ID_NEXT:
			CPVERB_NextTrack(vaDoVerb, hWnd);
			break;
			
		case ID_PREVIOUS:
			CPVERB_PrevTrack(vaDoVerb, hWnd);
			break;
			
		case ID_LOAD:
			CPVERB_OpenFile(vaDoVerb, hWnd);
			break;
			
		case ID_VOLUMEUP:
			CPVERB_VolumeUp(vaDoVerb, hWnd);
			break;
			
		case ID_VOLUMEDOWN:
			CPVERB_VolumeDown(vaDoVerb, hWnd);
			break;
			
		case ID_SEEKFORWARD:
			CPVERB_SkipForwards(vaDoVerb, hWnd);
			break;
			
		case ID_SEEKBACKWARD:
			CPVERB_SkipBackwards(vaDoVerb, hWnd);
			break;
			
		case ID_ABOUT:
			CPVERB_About(vaDoVerb, hWnd);
			break;
			
		case ID_DRAWSKINLINES:
		{
			int     teller;
			HDC     windc = GetDC(hWnd);
			HPEN    pen = CreatePen(PS_NULL, 0, 0);
			HPEN    oldpen = SelectObject(windc, pen);
			
			for (teller = PlaySwitch; teller < Lastone; teller++)
			{
				HBRUSH  brush =
					CreateSolidBrush(RGB(255 * rand(), 255 * rand(),
										 255 * rand()));
				HBRUSH  oldbrush = SelectObject(windc, brush);
				Rectangle(windc, Skin.Object[teller].x,
						  Skin.Object[teller].y,
						  Skin.Object[teller].x +
						  Skin.Object[teller].w + 2,
						  Skin.Object[teller].y + Skin.Object[teller].h +
						  2);
				SelectObject(windc, oldbrush);
				DeleteObject(brush);
			}
			
			SelectObject(windc, oldpen);
			
			DeleteObject(pen);
			ReleaseDC(hWnd, windc);
		}
		
		break;
		
		case ID_LOADSKIN:
		{
			if (globals.main_bool_skin_next_is_default == TRUE)
				options.use_default_skin = TRUE;
			else
				options.use_default_skin = FALSE;
				
			if (options.use_default_skin == FALSE)
			{
				char    skinpathje[MAX_PATH];
				strcpy(skinpathje, options.main_skin_file);
				
				if (main_skin_open(options.main_skin_file) == FALSE)
					main_set_default_skin();
				else
				{
					main_skin_add_to_menu(skinpathje);
					main_skin_select_menu(skinpathje);
				}
			}
			
			else
				main_set_default_skin();
				
			if (options.scroll_track_title)
				SetTimer(hWnd, CPC_TIMERID_SCROLLTITLETEXT, 50, NULL);
			else
				KillTimer(hWnd, CPC_TIMERID_SCROLLTITLETEXT);
				
			main_reset_window(hWnd);
			
			main_add_tooltips(hWnd, TRUE);
			
			break;
		}
		
		default:
			return -1;
	}
	
	return 0;
}

void    cmdline_usage(HWND hWndCoolPlayer)
{
	HRSRC   resource;
	HGLOBAL globaldata;
	
	resource = FindResource(NULL, // module handle
							MAKEINTRESOURCE(IDR_USAGE), // pointer to resource name
							"raw"); // pointer to resource type
	globaldata = LoadResource(NULL, // resource-module handle
							  resource);
	                          
	MessageBox(NULL, (LPCTSTR) LockResource(globaldata),
			   "CoolPlayer command line options", 0); // text to set
	           
	// only quit if no existing instance
	if (hWndCoolPlayer == NULL)
		PostQuitMessage(0);
}

int    *cmdline_get_argument(char *arg, HWND hWnd)
{

	if (_stricmp(arg, "help") == 0 || arg[0] == '?'
			|| ((arg[0] == 'h' || arg[0] == 'H') && arg[1] == '\0'))
		cmdline_usage(hWnd);
		
	if (_stricmp(arg, "add") == 0 || (GetTickCount() - globals.playlist_last_add_time) < 1000)
		globals.playlist_bool_addsong = TRUE;
	else
		globals.playlist_bool_addsong = FALSE;
		
	if (_stricmp(arg, "top") == 0)
		return &options.always_on_top;
		
	if (_stricmp(arg, "exit") == 0)
		return &options.auto_exit_after_playing;
		
	if (_stricmp(arg, "exitnow") == 0)
		PostQuitMessage(0);
		
	if (_stricmp(arg, "icon") == 0)
		return &options.rotate_systray_icon;
		
	if (_stricmp(arg, "scroll") == 0)
		return &options.scroll_track_title;
		
	if (_stricmp(arg, "time") == 0)
		return &options.show_remaining_time;
		
	if (_stricmp(arg, "tag") == 0)
		return &options.read_id3_tag;
		
	if (_stricmp(arg, "easy") == 0)
		return &options.easy_move;
		
	if (_stricmp(arg, "playlist") == 0)
		return &options.remember_playlist;
		
	if (_stricmp(arg, "played") == 0)
		return &options.remember_last_played_track;
		
	if (_stricmp(arg, "eq") == 0)
		return &options.equalizer;
		
	if (_stricmp(arg, "shuffle") == 0)
		return &options.shuffle_play;
		
	if (_stricmp(arg, "repeat") == 0)
		return &options.repeat_playlist;
		
	if (_stricmp(arg, "autoplay") == 0)
		return &options.auto_play_when_started;
		
	if (_stricmp(arg, "output") == 0)
		return &options.decoder_output_mode;
		
	if (_stricmp(arg, "skin") == 0)
		return &options.use_default_skin;
		
	if (_stricmp(arg, "showplaylist") == 0)
		return &options.show_playlist;
		
	if (_stricmp(arg, "minimized") == 0)
		return &globals.main_int_show_minimized;
		
	if (_stricmp(arg, "fileonce") == 0)
		return &options.allow_file_once_in_playlist;
		
	if (_stricmp(arg, "taskbar") == 0)
		return &options.show_on_taskbar;
		
	if (_stricmp(arg, "multipleinstances") == 0)
		return &options.allow_multiple_instances;
		
	if (_stricmp(arg, "play") == 0)
		main_play_control(ID_PLAY, hWnd);
		
	if (_stricmp(arg, "pause") == 0)
		main_play_control(ID_PAUSE, hWnd);
		
	if (_stricmp(arg, "stop") == 0)
		main_play_control(ID_STOP, hWnd);
		
	if (_stricmp(arg, "next") == 0)
		main_play_control(ID_NEXT, hWnd);
		
	if (_stricmp(arg, "prev") == 0)
		main_play_control(ID_PREVIOUS, hWnd);
		
	return NULL;
}

int     cmdline_parse_options(int argc, char **argv, HWND hWnd)
{
	//    char   *token;
	//   char    commandline[MAX_PATH];
	int i;
	int    *value;
	
	for (i = 1;i < argc;i++)
	{
		char *arg = argv[i];
		
		if (arg[0] == '-')
		{
			if ((value = cmdline_get_argument(arg + 1, hWnd)) != NULL)
				* value = FALSE;
				
			if (value == &options.use_default_skin)
			{
				i++;
				
				if (stricmp(argv[i], "default") == 0)
					*value = TRUE;
				else
					strcpy(options.main_skin_file, argv[i]);
			}
			
			if (value == &globals.main_int_show_minimized)
			{
				if (options.show_on_taskbar)
					globals.main_int_show_minimized = SW_MINIMIZE;
				else
					globals.main_int_show_minimized = SW_HIDE;
			}
			
			if (value == &options.show_playlist)
			{
				if (windows.m_hifPlaylist != NULL)
					CPlaylistWindow_SetVisible(FALSE);
			}
		}
		
		else if (arg[0] == '+')
		{
			if ((value = cmdline_get_argument(arg + 1, hWnd)) != NULL)
				* value = TRUE;
			
			if (value == &options.show_playlist)
			{
				if (windows.m_hifPlaylist != NULL)
					CPlaylistWindow_SetVisible(TRUE);
			}
		}
	}
	
	return 1;
}

//
//
//
BOOL cmdline_parse_argument(char *token)
{
	char    buffie[MAX_PATH] = "";
	char    expath[MAX_PATH];
	
	strcpy(buffie, token);
	path_unquote(buffie);
	
	if (buffie[0] == '\0')
		return FALSE;
		
	if (path_is_relative(buffie))
	{
		char    exepath[MAX_PATH];
		main_get_program_path(GetModuleHandle(NULL), exepath, MAX_PATH);
		sprintf(expath, "%s%s", exepath, buffie);
	}
	
	else
		strcpy(expath, buffie);
		
	if (_access(expath, 0) != -1)
	{
		if (globals.playlist_bool_addsong == FALSE
				&& globals.cmdline_bool_clear_playlist_first == TRUE)
		{
			CPL_Empty(globals.m_hPlaylist);
			globals.cmdline_bool_clear_playlist_first = FALSE;
		}
		
		CPL_SyncLoadNextFile(globals.m_hPlaylist);
		
		if (path_is_directory(expath) == TRUE)
			CPL_AddDirectory_Recurse(globals.m_hPlaylist, expath);
		else
			CPL_AddFile(globals.m_hPlaylist, expath);
			
		return TRUE;
	}
	
	return FALSE;
}

//
//
//
char   *str_delete_substr(char *strbuf, char *strtodel)
{
	char   *offset = NULL;
	
	while (*strbuf)
	{
		offset = strstr(strbuf, strtodel);
		
		if (offset)
			strcpy(offset, (offset + strlen(strtodel)));
		else
			break;
	}
	
	return strbuf;
}

int     cmdline_parse_files(int argc, char **argv)
{
	BOOL    wegotsome = FALSE;
	int i;
	globals.cmdline_bool_clear_playlist_first = TRUE;
	
	for (i = 1;i < argc;i++)
	{
		char *arg = argv[i];
		
		if (arg[0] != '-' && arg[0] != '+')
		{
			if (cmdline_parse_argument(arg))
				wegotsome = TRUE;
		}
	}
	
	return wegotsome;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASS wc;
	HWND    hWnd;
	MSG     msg;
	HRGN    winregion;
	INITCOMMONCONTROLSEX controls;
	HMENU   hpopup;
	BOOL    bAlreadyRuning;
	HANDLE  hMutexOneInstance;
	HWND hWndCoolPlayer = NULL;
	
	// Ensure that this system is audio capable
	
	if (waveOutGetNumDevs() < 1)
	{
		MessageBox(GetDesktopWindow(), "No audio devices in this system", CP_COOLPLAYER, MB_ICONSTOP | MB_OK);
		return -1;
	}
	
	options_read();
	
	hMutexOneInstance = CreateMutex(NULL, FALSE, CLC_COOLPLAYER_MUTEX);
	bAlreadyRuning = (GetLastError() == ERROR_ALREADY_EXISTS
					  || GetLastError() == ERROR_ACCESS_DENIED);
	// The call fails with ERROR_ACCESS_DENIED if the Mutex was
	// created in a different users session because of passing
	// NULL for the SECURITY_ATTRIBUTES on Mutex creation);
	
	if (bAlreadyRuning)
	{
		// Find the other coolplayer instance
		hWndCoolPlayer = FindWindow(CLC_COOLPLAYER_WINDOWCLASSNAME, NULL);
		
		if (hWndCoolPlayer != NULL)
		{
			if (lpCmdLine[0] != '\0')
			{
				int i;
				int size = 0;
				COPYDATASTRUCT cds;
				
				for (i = 0;i < __argc;i++)
					size += strlen(__argv[i]) + 1;
					
				//    cds.cbData = strlen(lpCmdLine) + 1;
				//    cds.dwData = strlen(lpCmdLine) + 1;
				//    cds.lpData = lpCmdLine;
				
				cds.cbData = size;
				
				cds.dwData = __argc;
				
				cds.lpData = __argv[0];
				
				
				SendMessage(hWndCoolPlayer, WM_COPYDATA, (WPARAM)hWndCoolPlayer, (LPARAM)&cds);
			}
			
			else
			{
				SetForegroundWindow(hWndCoolPlayer);
				ShowWindow(hWndCoolPlayer, SW_RESTORE);
			}
		}
		
		if (!options.allow_multiple_instances)
			return FALSE;  // terminates the creation
	}
	
	/* kill this */
	controls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	
	controls.dwICC = ICC_BAR_CLASSES;
	
	InitCommonControlsEx(&controls);
	
	//    options.shuffle_play = FALSE;
	//  options.repeat_playlist = FALSE;
	//    options.equalizer = FALSE;
	globals.main_bool_slider_keep_focus = FALSE;
	globals.main_int_skin_last_number = 5001;
	windows.m_hWndFindDialog = NULL;
	globals.playlist_int_last_searched_track = 0;
	globals.m_iLastPlaylistSortColoumn = -1;
	globals.m_bQuickFindWindowPos_Valid = FALSE;
	globals.m_hPlaylist = CPL_CreatePlaylist();
	globals.m_hhkListView_Posted = NULL;
	globals.playlist_bool_addsong = FALSE;
	globals.playlist_last_add_time = 0;
	globals.m_hSysIcon = NULL;
	
	CPSK_Initialise();
	
	hpopup = LoadMenu(NULL, MAKEINTRESOURCE(IDR_MENU1));
	
	globals.main_menu_popup = GetSubMenu(hpopup, 0);
	
	globals.m_hPlaylistImages = ImageList_LoadBitmap(hInstance,
								MAKEINTRESOURCE(IDB_PLAYLIST_CURRENTTRACK),
								12,
								1,
								RGB(0xFF, 0x00, 0xFF));
	                            
	                            
	if (!options.show_on_taskbar)
		switch (nCmdShow)
		{
		
			case SW_MINIMIZE:
			
			case SW_SHOWMINIMIZED:
			
			case SW_SHOWMINNOACTIVE:
				nCmdShow = SW_HIDE;
		}
		
	globals.main_int_show_minimized = nCmdShow;
	
	cmdline_parse_options(__argc, __argv, hWndCoolPlayer);
	
	if (*options.main_skin_file && options.use_default_skin == FALSE)
	{
		char    lastskinfile[MAX_PATH];
		strcpy(lastskinfile, options.main_skin_file);
		
		if (main_skin_open(options.main_skin_file) == FALSE)
		{
			main_set_default_skin();
		}
		
		else
		{
			main_skin_add_to_menu(lastskinfile);
			main_skin_select_menu(lastskinfile);
		}
	}
	
	else
	{
		main_set_default_skin();
	}
	
	globals.main_bool_wavwrite_dir_already_known = FALSE;
	
	globals.main_int_track_position = 0;
	globals.m_enPlayerState = cppsStopped;
	
	srand((unsigned int) time(NULL));
	
	if (graphics.bmp_main_up)
	{
		HDC     hRefDC = GetDC(NULL);
		ZeroMemory(&wc, sizeof(wc));
		wc.style         = CS_OWNDC;
		//wc.style = CS_PARENTDC;
		wc.lpszClassName = CLC_COOLPLAYER_WINDOWCLASSNAME;
		wc.lpfnWndProc = (WNDPROC) main_windowproc;
		wc.hInstance = hInstance;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hIcon = NULL;
		graphics.pal_main = CreateHalftonePalette(hRefDC);
		
		ReleaseDC(NULL, hRefDC);
		
		if (RegisterClass(&wc))
		{
			BITMAP  bm;
			GetObject(graphics.bmp_main_up, sizeof(bm), &bm);
			globals.m_hPlayer = NULL;
			hWnd =
				CreateWindowEx(WS_EX_ACCEPTFILES | WS_EX_TOOLWINDOW,
							   CLC_COOLPLAYER_WINDOWCLASSNAME, CP_COOLPLAYER,
							   WS_POPUP | WS_CLIPSIBLINGS,
							   options.main_window_pos.x,
							   options.main_window_pos.y, bm.bmWidth,
							   bm.bmHeight, NULL, NULL, hInstance, NULL);
			SendMessage(hWnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(APP_ICON)));
			SendMessage(hWnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(APP_ICON)));
			
			// Register hotkey
			RegisterHotKey(hWnd, CP_HOTKEY_NEXT, 0L, VK_MEDIA_NEXT_TRACK);
			RegisterHotKey(hWnd, CP_HOTKEY_PREV, 0L, VK_MEDIA_PREV_TRACK);
			RegisterHotKey(hWnd, CP_HOTKEY_STOP, 0L, VK_MEDIA_STOP);
			RegisterHotKey(hWnd, CP_HOTKEY_PLAY0, 0L, VK_MEDIA_PLAY_PAUSE);
			RegisterHotKey(hWnd, CP_HOTKEY_PLAY1, 0L, VK_PAUSE);
			
			if (hWnd)
			{
				if (options.show_on_taskbar)
				{
					SetWindowLong(hWnd, GWL_EXSTYLE,
								  GetWindowLong(hWnd,
												GWL_EXSTYLE) &
								  ~WS_EX_TOOLWINDOW);
					SetWindowLong(hWnd, GWL_STYLE,
								  GetWindowLong(hWnd,
												GWL_STYLE) | WS_SYSMENU);
				}
				
				drawables.dc_main = GetDC(hWnd);
				
				drawables.dc_memory =
					CreateCompatibleDC(drawables.dc_main);
				main_update_title_text();
				
				// Create player instance
				globals.m_hPlayer = CPI_Player__Create(hWnd);
				globals.m_iVolume = CPI_Player__GetVolume(globals.m_hPlayer);
				
				winregion =
					main_bitmap_to_region(graphics.bmp_main_up, Skin.transparentcolor);
				    
				SetWindowRgn(hWnd, winregion, TRUE);
				ShowWindow(hWnd, globals.main_int_show_minimized);
				globals.m_hSysIcon = CPSYSICON_Create(hWnd);
				
				CPI_Player__SetPositionRange(globals.m_hPlayer,
											 Skin.Object[PositionSlider].maxw ? Skin.Object[PositionSlider].h : Skin.Object[PositionSlider].w);
				CP_InitWindowsRoutines();
				IF_ProcessInit();
				//  CPlayerWindow_Create();
				CPlaylistWindow_Create();
				
				window_set_always_on_top(hWnd, options.always_on_top);
				
				main_set_eq();
				
				globals.m_enPlayerState = cppsStopped;
				globals.m_bStreaming = FALSE;
				globals.m_iStreamingPortion = 0;
				
				{
					BOOL wegotsome;
					
					wegotsome = cmdline_parse_files(__argc, __argv);
					
					if (!wegotsome || globals.playlist_bool_addsong == TRUE)
					{
						if ((CPL_GetFirstItem(globals.m_hPlaylist)
								&& globals.playlist_bool_addsong == TRUE
								&& options.remember_playlist == TRUE)
								|| (options.remember_playlist == TRUE
									&& CPL_GetFirstItem(globals.m_hPlaylist) == NULL))
						{
							char pcPlaylistFilename[MAX_PATH];
							main_get_program_path(GetModuleHandle(NULL), pcPlaylistFilename, MAX_PATH);
							strcat(pcPlaylistFilename, "default.m3u");
							
							CPL_SyncLoadNextFile(globals.m_hPlaylist);
							CPL_SetAutoActivateInitial(globals.m_hPlaylist, TRUE);
							CPL_AddFile(globals.m_hPlaylist, pcPlaylistFilename);
							CPL_SetAutoActivateInitial(globals.m_hPlaylist, FALSE);
						}
						
						else if (*options.initial_file)
						{
							CPL_SyncLoadNextFile(globals.m_hPlaylist);
							CPL_AddSingleFile(globals.m_hPlaylist, options.initial_file, NULL);
						}
					}
					
					// Start playing
					
					if (wegotsome || options.auto_play_when_started)
						CPL_PlayItem(globals.m_hPlaylist, TRUE, pmCurrentItem);
				}
				
				windows.wnd_tooltip =
					CreateWindow(TOOLTIPS_CLASS, (LPSTR) NULL,
								 TTS_ALWAYSTIP, CW_USEDEFAULT,
								 CW_USEDEFAULT, CW_USEDEFAULT,
								 CW_USEDEFAULT, hWnd, (HMENU) NULL,
								 hInstance, NULL);
				                 
				main_add_tooltips(hWnd, FALSE);
				
				while (GetMessage(&msg, NULL, 0, 0))
				{
					if (msg.message == CPPLNM_TAGREAD)
					{
						CPL_HandleAsyncNotify(globals.m_hPlaylist, msg.wParam, msg.lParam);
						continue;
					}
					
					if (msg.message == CPPLNM_SYNCSHUFFLE)
					{
						CPL_Stack_Shuffle(globals.m_hPlaylist, TRUE);
						continue;
					}
					
					if (msg.message == CPPLNM_SYNCSETACTIVE)
					{
						CPL_SetActiveItem(globals.m_hPlaylist, (CP_HPLAYLISTITEM)msg.wParam);
						continue;
					}
					
					if (windows.m_hWndFindDialog && IsDialogMessage(windows.m_hWndFindDialog, &msg))
						continue;
						
					TranslateMessage(&msg);
					
					DispatchMessage(&msg);
				}
				
				// Clean up
				DestroyMenu(globals.main_menu_popup);
				DestroyMenu(hpopup);
				CPSYSICON_Destroy(globals.m_hSysIcon);
				DeleteDC(drawables.dc_memory);
				ReleaseDC(hWnd, drawables.dc_main);
				DeleteObject(graphics.bmp_main_title_area);
				ImageList_Destroy(globals.m_hPlaylistImages);
				CPSK_Uninitialise();
				IF_ProcessDeInit();
				
				return (msg.wParam);
			}
		}
	}
	
	return -1;
}

//
//
//
