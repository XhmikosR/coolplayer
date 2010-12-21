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

int main_set_default_skin(void)
{

	HINSTANCE hInstance;
	float   positionpercentage;
	
	if (Skin.Object[PositionSlider].maxw == 1)
	{
		positionpercentage =
			(float) globals.main_int_track_position /
			(float) Skin.Object[PositionSlider].h;
	}
	
	else
	{
		positionpercentage =
			(float) globals.main_int_track_position /
			(float) Skin.Object[PositionSlider].w;
	}
	
	globals.main_int_title_scroll_position = 0;
	globals.mail_int_title_scroll_max_position = 0;

	memset(&Skin, 0, sizeof(Skin));
	
	main_skin_set_struct_value(PlaySwitch, 172, 23, 24, 16, 0, 19, 60, 24,
							   7, "");
	main_skin_set_struct_value(StopSwitch, 222, 23, 24, 16, 0, 0, 0, 0, 0,
							   "");
	main_skin_set_struct_value(PauseSwitch, 197, 23, 24, 16, 0, 197, 23,
							   25, 17, "");
	main_skin_set_struct_value(RepeatSwitch, 197, 57, 24, 16, 0, 96, 60,
							   35, 7, "");
	main_skin_set_struct_value(ShuffleSwitch, 158, 57, 38, 16, 0, 50, 60,
							   39, 7, "");
	main_skin_set_struct_value(EqSwitch, 97, 93, 17, 28, 0, 97, 93, 18, 29,
							   "");
	main_skin_set_struct_value(MinimizeButton, 230, 5, 7, 8, 0, 0, 0, 0, 0,
							   "");
	main_skin_set_struct_value(ExitButton, 239, 5, 7, 8, 0, 0, 0, 0, 0,
							   "");
	main_skin_set_struct_value(NextSkinButton, 254, 44, 9, 27, 0, 0, 0, 0,
							   0, "");
	main_skin_set_struct_value(EjectButton, 222, 40, 24, 16, 0, 0, 0, 0, 0,
							   "");
	main_skin_set_struct_value(NextButton, 197, 40, 24, 16, 0, 0, 0, 0, 0,
							   "");
	main_skin_set_struct_value(PrevButton, 172, 40, 24, 16, 0, 0, 0, 0, 0,
							   "");
	main_skin_set_struct_value(MoveArea, 0, 0, 229, 12, 0, 0, 0, 0, 0, "");
	main_skin_set_struct_value(PlaylistButton, 222, 57, 24, 16, 0, 0, 0, 0,
							   0, "");
	main_skin_set_struct_value(VolumeSlider, 84, 95, 9, 75, 1, 0, 0, 0, 0,
							   "");
	main_skin_set_struct_value(PositionSlider, 12, 78, 233, 8, 0, 0, 0, 0,
							   0, "");
	main_skin_set_struct_value(Eq1, 115, 95, 9, 75, 1, 0, 0, 0, 0, "");
	main_skin_set_struct_value(Eq2, 132, 95, 9, 75, 1, 0, 0, 0, 0, "");
	main_skin_set_struct_value(Eq3, 149, 95, 9, 75, 1, 0, 0, 0, 0, "");
	main_skin_set_struct_value(Eq4, 166, 95, 9, 75, 1, 0, 0, 0, 0, "");
	main_skin_set_struct_value(Eq5, 183, 95, 9, 75, 1, 0, 0, 0, 0, "");
	main_skin_set_struct_value(Eq6, 200, 95, 9, 75, 1, 0, 0, 0, 0, "");
	main_skin_set_struct_value(Eq7, 217, 95, 9, 75, 1, 0, 0, 0, 0, "");
	main_skin_set_struct_value(Eq8, 234, 95, 9, 75, 1, 0, 0, 0, 0, "");
	main_skin_set_struct_value(SongtitleText, 18, 21, 6, 13, 23, 0, 0, 0,
							   0, "");
	main_skin_set_struct_value(TrackText, 18, 44, 13, 14, 0, 0, 0, 0, 0,
							   "");
	main_skin_set_struct_value(TimeText, 59, 35, 13, 14, 0, 0, 0, 0, 0,
							   "");

	main_skin_set_struct_value(BitrateText, 83, 48, 0, 0, 0, 0, 0, 0, 0,
							   "");
	main_skin_set_struct_value(FreqText, 125, 48, 0, 0, 0, 0, 0, 0, 0, "");
	
	Skin.transparentcolor = 0x0000ff00;
	hInstance = GetModuleHandle(NULL);
	graphics.bmp_main_up =
		(HBITMAP) LoadImage(hInstance, MAKEINTRESOURCE(IDB_MAINUP),
							IMAGE_BITMAP, 0, 0, 0L);
	graphics.bmp_main_down =
		(HBITMAP) LoadImage(hInstance, MAKEINTRESOURCE(IDB_MAINDOWN),
							IMAGE_BITMAP, 0, 0, 0L);
	graphics.bmp_main_switch = graphics.bmp_main_down;
	graphics.bmp_main_time_font =
		(HBITMAP) LoadImage(hInstance, MAKEINTRESOURCE(IDB_MAINBIGFONT),
							IMAGE_BITMAP, 0, 0, 0L);
	graphics.bmp_main_track_font = graphics.bmp_main_time_font;
	graphics.bmp_main_title_font =
		(HBITMAP) LoadImage(hInstance, MAKEINTRESOURCE(IDB_MAINSMALLFONT),
							IMAGE_BITMAP, 0, 0, 0L);
	                        
	if (Skin.Object[PositionSlider].maxw == 1)
	{
		globals.main_int_track_position =
			(int)((float)(Skin.Object[PositionSlider].h) *
				  positionpercentage);
	}
	
	else
	{
		globals.main_int_track_position =
			(int)((float)(Skin.Object[PositionSlider].w) *
				  positionpercentage);
	}
	
	globals.main_bool_skin_next_is_default = FALSE;
	
	main_update_title_text();
	main_skin_select_menu("Default");
	
	return TRUE;
}

int     main_add_tooltips(HWND hWnd, BOOL update)
{

	TOOLINFO ti;  // tool information
	
	char   *tips[] =
	{
		"Play",
		"Stop",
		"Pause",
		"Eject",
		"Repeat",
		"Shuffle",
		"Equalizer",
		"Next",
		"Previous",
		"Playlist",
		"Minimize",
		"Skinswitch",
		"Exit"
	};
	
	int     teller;
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = 0;
	ti.hwnd = hWnd;
	ti.hinst = GetModuleHandle(NULL);
	
	for (teller = PlaySwitch; teller <= ExitButton; teller++)
	{
		ti.uId = (UINT) teller;
		
		if (*Skin.Object[teller].tooltip)
			ti.lpszText = (LPSTR) Skin.Object[teller].tooltip;
		else
			ti.lpszText = (LPSTR) tips[teller];
			
		ti.rect.left = Skin.Object[teller].x;
		ti.rect.top = Skin.Object[teller].y;
		ti.rect.right = Skin.Object[teller].x + Skin.Object[teller].w;
		ti.rect.bottom = Skin.Object[teller].y + Skin.Object[teller].h;
		
		SendMessage(windows.wnd_tooltip,
					update ? TTM_NEWTOOLRECT : TTM_ADDTOOL, 0,
					(LPARAM)(LPTOOLINFO) & ti);
		            
		if (update == TRUE)
			SendMessage(windows.wnd_tooltip, TTM_UPDATETIPTEXT, 0,
						(LPARAM)(LPTOOLINFO) & ti);
			            
	}
	
	return 1;
}

int



main_skin_set_struct_value(int object, int x, int y, int w, int h, int maxw, int x2, int y2, int w2, int h2,
						   char *tooltip)
{

	Skin.Object[object].x = x;
	Skin.Object[object].y = y;
	Skin.Object[object].w = w;
	Skin.Object[object].h = h;
	Skin.Object[object].maxw = maxw;
	Skin.Object[object].x2 = x2;
	Skin.Object[object].y2 = y2;
	Skin.Object[object].w2 = w2;
	Skin.Object[object].h2 = h2;
	strcpy(Skin.Object[object].tooltip, tooltip);
	return TRUE;
}

int     main_skin_open(char *name)
{
	char    pathbuf[MAX_PATH];
	char    values[32768];
	char   *textposition;
	char    buffer[4096];
	char    errorbuf[4096] = "";
	// int     teller = 0;
	int     returnval;
	HINSTANCE hInstance;
	Associate associate[] =
	{
		{ "PlaySwitch", PlaySwitch },
		{ "StopSwitch", StopSwitch },
		{ "PauseSwitch", PauseSwitch },
		{ "EjectButton", EjectButton },
		{ "RepeatSwitch", RepeatSwitch },
		{ "ShuffleSwitch", ShuffleSwitch },
		{ "EqSwitch", EqSwitch },
		{ "NextButton", NextButton },
		{ "PrevButton", PrevButton },
		{ "PlaylistButton", PlaylistButton },
		{ "MinimizeButton", MinimizeButton },
		{ "NextSkinButton", NextSkinButton },
		{ "ExitButton", ExitButton },
		{ "MoveArea", MoveArea },
		{ "VolumeSlider", VolumeSlider },
		{ "PositionSlider", PositionSlider },
		{ "Eq1", Eq1 },
		{ "Eq2", Eq2 },
		{ "Eq3", Eq3 },
		{ "Eq4", Eq4 },
		{ "Eq5", Eq5 },
		{ "Eq6", Eq6 },
		{ "Eq7", Eq7 },
		{ "Eq8", Eq8 },
		{ "SongtitleText", SongtitleText },
		{ "TrackText", TrackText },
		{ "TimeText", TimeText },
		{ "BitrateText", BitrateText },
		{ "FreqText", FreqText }
	};
	float   positionpercentage;
	
	if (Skin.Object[PositionSlider].maxw == 1)
	{
		positionpercentage =
			(float) globals.main_int_track_position /
			(float) Skin.Object[PositionSlider].h;
	}
	
	else
	{
		positionpercentage =
			(float) globals.main_int_track_position /
			(float) Skin.Object[PositionSlider].w;
	}
	
	globals.main_int_title_scroll_position = 0;
	globals.mail_int_title_scroll_max_position = 0;
	
	if (*options.main_skin_file == 0)
	{
		MessageBox(GetForegroundWindow(), "No Skin file selected!",
				   "Error", MB_ICONERROR);
		options.use_default_skin = TRUE;
		return FALSE;
	}
	
	else
		strcpy(pathbuf, options.main_skin_file);
		
	memset(&Skin, 0, sizeof(Skin));
	
	GetPrivateProfileString(NULL, NULL, NULL,
							buffer, sizeof(buffer), pathbuf);
	                        
	returnval = GetPrivateProfileSection("CoolPlayer Skin", // address of section name
										 values, // address of return buffer
										 32767, // size of return buffer
										 pathbuf // address of initialization filename
										);
	                                    
	if (returnval == 0)
	{
		char    textbuf[MAX_PATH + 50];
		sprintf(textbuf, "Not a valid CoolPlayer Skin file: %s", pathbuf);
		MessageBox(GetForegroundWindow(), textbuf, "error", MB_ICONERROR);
		options.use_default_skin = TRUE;
		return FALSE;
	}
	
	textposition = values;
	
	while (*textposition != 0)
	{
	
		main_skin_check_ini_value(textposition, associate);
		textposition = textposition + strlen(textposition) + 1;
	}
	
	path_remove_filespec(pathbuf);
	
	strcat(pathbuf, Skin.CoolUp);
	hInstance = GetModuleHandle(NULL);
	DeleteObject(graphics.bmp_main_up);
	graphics.bmp_main_up =
		(HBITMAP) LoadImage(hInstance, pathbuf, IMAGE_BITMAP, 0, 0,
							LR_LOADFROMFILE);
	                        
	if (!graphics.bmp_main_up)
	{
		strcat(errorbuf, pathbuf);
		strcat(errorbuf, "\n");
	}
	
	path_remove_filespec(pathbuf);
	
	strcat(pathbuf, Skin.CoolDown);
	DeleteObject(graphics.bmp_main_down);
	graphics.bmp_main_down =
		(HBITMAP) LoadImage(hInstance, pathbuf, IMAGE_BITMAP, 0, 0,
							LR_LOADFROMFILE);
	                        
	if (!graphics.bmp_main_down)
	{
		strcat(errorbuf, pathbuf);
		strcat(errorbuf, "\n");
	}
	
	path_remove_filespec(pathbuf);
	
	strcat(pathbuf, Skin.CoolSwitch);
	DeleteObject(graphics.bmp_main_switch);
	graphics.bmp_main_switch =
		(HBITMAP) LoadImage(hInstance, pathbuf, IMAGE_BITMAP, 0, 0,
							LR_LOADFROMFILE);
	                        
	if (!graphics.bmp_main_switch)
	{
		strcat(errorbuf, pathbuf);
		strcat(errorbuf, "\n");
	}
	
	path_remove_filespec(pathbuf);
	
	strcat(pathbuf, Skin.aTimeFont);
	DeleteObject(graphics.bmp_main_time_font);
	graphics.bmp_main_time_font =
		(HBITMAP) LoadImage(hInstance, pathbuf, IMAGE_BITMAP, 0, 0,
							LR_LOADFROMFILE);
	                        
	if (!graphics.bmp_main_time_font)
	{
		strcat(errorbuf, pathbuf);
		strcat(errorbuf, "\n");
	}
	
	path_remove_filespec(pathbuf);
	
	strcat(pathbuf, Skin.aTrackFont);
	DeleteObject(graphics.bmp_main_track_font);
	graphics.bmp_main_track_font =
		(HBITMAP) LoadImage(hInstance, pathbuf, IMAGE_BITMAP, 0, 0,
							LR_LOADFROMFILE);
	                        
	if (!graphics.bmp_main_track_font)
	{
		strcat(errorbuf, pathbuf);
		strcat(errorbuf, "\n");
	}
	
	path_remove_filespec(pathbuf);
	
	strcat(pathbuf, Skin.aTextFont);
	DeleteObject(graphics.bmp_main_title_font);
	graphics.bmp_main_title_font =
		(HBITMAP) LoadImage(hInstance, pathbuf, IMAGE_BITMAP, 0, 0,
							LR_LOADFROMFILE);
	                        
	if (!graphics.bmp_main_title_font)
	{
		strcat(errorbuf, pathbuf);
		strcat(errorbuf, "\n");
	}
	
	if (!graphics.bmp_main_up || !graphics.bmp_main_down
			|| !graphics.bmp_main_switch || !graphics.bmp_main_time_font
			|| !graphics.bmp_main_title_font || !graphics.bmp_main_track_font)
	{
		char    errorstring[5000];
		
		sprintf(errorstring, "Can\'t load bitmaps!\n%s", errorbuf);
		MessageBox(GetForegroundWindow(), errorstring, "error",
				   MB_ICONERROR);
		options.use_default_skin = TRUE;
		return FALSE;
		
	}
	
	if (Skin.Object[PositionSlider].maxw == 1)
	{
		globals.main_int_track_position =
			(int)((float)(Skin.Object[PositionSlider].h) *
				  positionpercentage);
		          
	}
	
	else
	{
		globals.main_int_track_position =
			(int)((float)(Skin.Object[PositionSlider].w) *
				  positionpercentage);
	}
	
	main_update_title_text();
	
	return 1;
}

void    main_skin_check_ini_value(char *textposition,
								  Associate * associate)
{
	char    name[128] = "";
	int     x = 0, y = 0, w = 0, h = 0, maxw = 0, x2 = 0, y2 = 0, w2 =
										 0, h2 = 0;
	char    tooltip[100] = "";
	int teller = 0;
	
	while (teller < strlen(textposition))
	{
		if (textposition[teller] == '=' || textposition[teller] == ',')
			textposition[teller] = ' ';
			
		teller++;
	}
	
	// sscanf(textposition, "%s %d %d %d %d %d %d %d %d %d %[^\0]",
	sscanf(textposition, "%s %d %d %d %d %d %d %d %d %d %s",
		   name, &x, &y, &w, &h, &maxw, &x2, &y2, &w2, &h2, tooltip);
	       
	for (teller = 0; teller < Lastone; teller++)
	{
		if (stricmp(name, associate[teller].name) == 0)
		{
			main_skin_set_struct_value(associate[teller].Object, x, y, w,
									   h, maxw, x2, y2, w2, h2, tooltip);
			return;
		}
		
		if (stricmp(name, "PlaylistSkin") == 0)
		{
			char    pathbuf[MAX_PATH];
			
			if (path_is_relative(textposition + strlen(name) + 1))
			{
				strcpy(pathbuf, options.main_skin_file);
				path_remove_filespec(pathbuf);
				strcat(pathbuf, textposition + strlen(name) + 1);
			}
			
			else
				strcpy(pathbuf, textposition + strlen(name) + 1);
				
			if (!globals.playlist_bool_force_skin_from_options)
				strcpy(options.playlist_skin_file, pathbuf);
		}
		
		if (stricmp(name, "transparentcolor") == 0)
		{
			int     colortext;
			sscanf(textposition, "%s %x", name, &colortext);
			Skin.transparentcolor = colortext;
			return;
		}
		
		if (stricmp(name, "BmpCoolUp") == 0)
		{
			strcpy(Skin.CoolUp, textposition + strlen(name) + 1);
		}
		
		if (stricmp(name, "BmpCoolDown") == 0)
		{
			strcpy(Skin.CoolDown, textposition + strlen(name) + 1);
		}
		
		if (stricmp(name, "BmpCoolSwitch") == 0)
		{
			strcpy(Skin.CoolSwitch, textposition + strlen(name) + 1);
		}
		
		if (stricmp(name, "BmpTextFont") == 0)
		{
			strcpy(Skin.aTextFont, textposition + strlen(name) + 1);
		}
		
		if (stricmp(name, "BmpTimeFont") == 0)
		{
			strcpy(Skin.aTimeFont, textposition + strlen(name) + 1);
		}
		
		if (stricmp(name, "BmpTrackFont") == 0)
		{
			strcpy(Skin.aTrackFont, textposition + strlen(name) + 1);
		}
		
		if (stricmp(name, "NextSkin") == 0)
		{
			if (stricmp(textposition + strlen(name) + 1, "default") == 0)
			{
				globals.main_bool_skin_next_is_default = TRUE;
			}
			
			else
			{
				char    drive[_MAX_DRIVE];
				char    fname[MAX_PATH];
				char    modpathbuf[MAX_PATH];
				char    ext[_MAX_EXT];
				char    dir[_MAX_DIR];
				char    skinfile2[MAX_PATH];
				strcpy(skinfile2, options.main_skin_file);
				path_remove_filespec(skinfile2);
				
				main_get_program_path(GetModuleHandle(NULL), modpathbuf,
									  MAX_PATH);
				_splitpath(textposition + strlen(name) + 1, drive, dir,
						   fname, ext);
				           
				if (strcmp(drive, "") == 0)
				{
					sprintf(options.main_skin_file, "%s%s%s", skinfile2,
							fname, ext);
				}
				
				else
					strcpy(options.main_skin_file,
						   textposition + strlen(name) + 1);
					       
				if (_access(options.main_skin_file, 0) == -1)
				{
					sprintf(options.main_skin_file, "%s%s%s%s", modpathbuf,
							dir, fname, ext);
				}
				
				globals.main_bool_skin_next_is_default = FALSE;
			}
		}
	}
}
