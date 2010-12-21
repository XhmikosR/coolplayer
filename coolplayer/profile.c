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
#include "CPI_Playlist.h"
#include "CPI_PlaylistItem.h"


////////////////////////////////////////////////////////////
//
//
//
void    playlist_write_default(void)
{
	char    exepath[MAX_PATH];
	main_get_program_path(GetModuleHandle(NULL), exepath, MAX_PATH);
	strcat(exepath, "default.m3u");
	CPL_ExportPlaylist(globals.m_hPlaylist, exepath);
}

//
//
//
void    options_read(void)
{
	char    pathbuf[MAX_PATH];
	int     teller;
	int     widths[] = {   20,   200,  200,  200,  50,   50,   70,    100,  100,   100, 50};
	int     visibles[] = { TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, TRUE };
	int iColIDX;
	
	main_get_program_path(NULL, pathbuf, MAX_PATH);
	strcat(pathbuf, "coolplayer.ini");
	
	for (iColIDX = PLAYLIST_first; iColIDX <= PLAYLIST_last; iColIDX++)
	{
		char keyname[100];
		sprintf(keyname, "PlaylistCol%d", iColIDX);
		options.playlist_column_widths[iColIDX] = GetPrivateProfileInt("WindowPos", keyname, widths[iColIDX], pathbuf);
		
		sprintf(keyname, "PlaylistSeq%d", iColIDX);
		options.playlist_column_seq[iColIDX] = GetPrivateProfileInt("WindowPos", keyname, iColIDX, pathbuf);
		
		sprintf(keyname, "PlaylistVis%d", iColIDX);
		options.playlist_column_visible[iColIDX] = GetPrivateProfileInt("WindowPos", keyname, visibles[iColIDX], pathbuf) ? TRUE : FALSE;
	}
	
	options.main_window_pos.x = GetPrivateProfileInt("WindowPos", // address of section name
	
								"WindowX", // address of key name
								100, // return value if key name is not found
								pathbuf); // address of initialization filename
	                            
	if (options.main_window_pos.x < -10)
		options.main_window_pos.x = 100;
		
	options.main_window_pos.y = GetPrivateProfileInt("WindowPos", // address of section name
								"WindowY", // address of key name
								100, // return value if key name is not found
								pathbuf); // address of initialization filename
	                            
	if (options.main_window_pos.y < -10)
		options.main_window_pos.y = 100;
		
	options.playlist_window_pos.left = GetPrivateProfileInt("WindowPos", // address of section name
									   "PlaylistX", // address of key name
									   100, // return value if key name is not found
									   pathbuf); // address of initialization filename
	                                   
	if (options.playlist_window_pos.left < -10)
		options.playlist_window_pos.left = 100;
		
	options.playlist_window_pos.top = GetPrivateProfileInt("WindowPos", // address of section name
									  "PlaylistY", // address of key name
									  100, // return value if key name is not found
									  pathbuf); // address of initialization filename
	                                  
	if (options.playlist_window_pos.top < -10)
		options.playlist_window_pos.top = 100;
		
	options.playlist_window_pos.right = GetPrivateProfileInt("WindowPos", // address of section name
										"PlaylistW", // address of key name
										300, // return value if key name is not found
										pathbuf) + options.playlist_window_pos.left; // address of initialization filename
	                                    
	options.playlist_window_pos.bottom = GetPrivateProfileInt("WindowPos", // address of section name
										 "PlaylistH", // address of key name
										 400, // return value if key name is not found
										 pathbuf) + options.playlist_window_pos.top; // address of initialization filename
	                                     
	GetPrivateProfileString("LastDirectory", // points to section name
							"Directory", // points to key name
							"", // points to default string
							options.last_used_directory, // points to destination buffer
							MAX_PATH, // size of destination buffer
							pathbuf); // points to initialization filename
	                        
	options.repeat_playlist =
		GetPrivateProfileInt("Misc", "Repeat", 0, pathbuf);
	    
	options.shuffle_play =
		GetPrivateProfileInt("Misc", "Shuffle", 0, pathbuf);
	    
	options.always_on_top =
		GetPrivateProfileInt("Misc", "Ontop", 0, pathbuf);
	    
	options.auto_exit_after_playing =
		GetPrivateProfileInt("Misc", "Autoexit", 0, pathbuf);
	    
	options.remember_playlist =
		GetPrivateProfileInt("Misc", "Rememberpls", 1, pathbuf);
	    
	options.show_remaining_time =
		GetPrivateProfileInt("Misc", "Remaining", 0, pathbuf);
	    
	options.read_id3_tag =
		GetPrivateProfileInt("Misc", "ReadID3tag", 1, pathbuf);
	    
	options.support_id3v2 =
		GetPrivateProfileInt("Misc", "SuportID3v2", 1, pathbuf);
	    
	options.prefer_native_ogg_tags =
		GetPrivateProfileInt("Misc", "PreferNativeOGGtags", 1, pathbuf);
	    
	options.read_id3_tag_in_background =
		GetPrivateProfileInt("Misc", "BackgroundReadID3", 1, pathbuf);
	    
	options.work_out_track_lengths =
		GetPrivateProfileInt("Misc", "WorkOutTrackLengths", 1, pathbuf);
	    
	options.allow_multiple_instances =
		GetPrivateProfileInt("Misc", "AllowMultipleInstances", 0, pathbuf);
	    
	options.read_id3_tag_of_selected =
		GetPrivateProfileInt("Misc", "ReadSelID3tag", 1, pathbuf);
	    
	options.seconds_delay_after_track =
		GetPrivateProfileInt("Misc", "DelayTime", 0, pathbuf);
	    
	options.decoder_output_mode =
		GetPrivateProfileInt("Misc", "Outputmode", 1, pathbuf);
	    
	options.easy_move =
		GetPrivateProfileInt("Misc", "Easymove", 1, pathbuf);
	    
	options.remember_skin_count =
		GetPrivateProfileInt("Misc", "RememberSkins", 4, pathbuf);
	    
	options.allow_file_once_in_playlist =
		GetPrivateProfileInt("Misc", "Fileonce", 1, pathbuf);
	    
	options.auto_play_when_started =
		GetPrivateProfileInt("Misc", "Autoplay", 0, pathbuf);
	    
	options.show_on_taskbar =
		GetPrivateProfileInt("Misc", "TaskBar", 0, pathbuf);
	    
	options.show_playlist = GetPrivateProfileInt("Misc", "ShowPlaylist", 0, pathbuf);
	
	options.rotate_systray_icon =
		GetPrivateProfileInt("Misc", "RotateIcon", 1, pathbuf);
	    
	options.scroll_track_title =
		GetPrivateProfileInt("Misc", "Scrolltitle", 1, pathbuf);
	    
	GetPrivateProfileString("Misc", // points to section name
							"RememberLastSong", // points to key name
							"", // points to default string
							options.initial_file, // points to destination buffer
							MAX_PATH, // size of destination buffer
							pathbuf); // points to initialization filename
	                        
	if (*options.initial_file)
		options.remember_last_played_track = TRUE;
		
	options.last_selected_skin_number =
		GetPrivateProfileInt("Skin", "LastSkin", 0, pathbuf);
	    
	options.use_default_skin =
		GetPrivateProfileInt("Skin", "UseDefault", 1, pathbuf);
	    
	options.use_playlist_skin =
		GetPrivateProfileInt("Skin", "Useplaylistskin", 0, pathbuf);
	    
	{
		int     teller;
		
		for (teller = MENU_SKIN_DEFAULT + 1; teller < MENU_SKIN_DEFAULT + 1 + options.remember_skin_count;
				teller++)
		{
			char    SkinFileString[MAX_PATH];
			char    skinpath[MAX_PATH];
			sprintf(SkinFileString, "SkinFile%d", teller - MENU_SKIN_DEFAULT);
			GetPrivateProfileString("Skin", SkinFileString, "",
									skinpath, MAX_PATH, pathbuf);
			                        
			if (*skinpath != 0)
			{
				main_skin_add_to_menu(skinpath);
				
				if (options.last_selected_skin_number == teller - MENU_SKIN_DEFAULT)
				{
					strcpy(options.main_skin_file, skinpath);
				}
			}
		}
	}
	
	GetPrivateProfileString("Skin", "PlaylistSkin", "",
	
							options.playlist_skin_file, MAX_PATH, pathbuf);
	options.equalizer =
		GetPrivateProfileInt("Equalizer", "Active", 0, pathbuf);
	    
	for (teller = 1; teller <= 8; teller++)
	{
		char    keyname[100];
		sprintf(keyname, "Eq%d", teller);
		options.eq_settings[teller] =
			GetPrivateProfileInt("Equalizer", keyname, 0, pathbuf);
	}
	
	// Read quick find defaults
	{
		char pcQuickFindOption[2];
		GetPrivateProfileString("Misc", "QuickFindSearchTerm", "T", pcQuickFindOption, 2, pathbuf);
		
		if (pcQuickFindOption[0] == 'M' || pcQuickFindOption[0] == 'm')
			options.m_enQuickFindTerm = qftAlbum;
		else if (pcQuickFindOption[0] == 'A' || pcQuickFindOption[0] == 'a')
			options.m_enQuickFindTerm = qftArtist;
		else
			options.m_enQuickFindTerm = qftTitle;
	}
	
	// Read mixer mode
	{
		char cMixerMode[32];
		GetPrivateProfileString("Mixer", "Mode", "Master", cMixerMode, 32, pathbuf);
		
		if (stricmp(cMixerMode, "wave") == 0)
			globals.m_enMixerMode = mmWaveVolume;
		else if (stricmp(cMixerMode, "internal") == 0)
			globals.m_enMixerMode = mmInternal;
		else
			globals.m_enMixerMode = mmMasterVolume;
			
		globals.m_iVolume = GetPrivateProfileInt("Mixer", "InternalVolume", 60, pathbuf);
	}
}

void    options_write()
{
	char    intbuf[33];
	int     teller;
	char    pathbuf[MAX_PATH];
	int iColIDX;
	
	main_get_program_path(NULL, pathbuf, MAX_PATH);
	strcat(pathbuf, "coolplayer.ini");
	
	for (iColIDX = PLAYLIST_first; iColIDX <= PLAYLIST_last; iColIDX++)
	{
		char keyname[100];
		
		// Write the width
		sprintf(keyname, "PlaylistCol%d", iColIDX);
		WritePrivateProfileString("WindowPos", keyname, _itoa(options.playlist_column_widths[iColIDX], intbuf, 10), pathbuf);
		
		// Write the order array
		sprintf(keyname, "PlaylistSeq%d", iColIDX);
		WritePrivateProfileString("WindowPos", keyname, _itoa(options.playlist_column_seq[iColIDX], intbuf, 10), pathbuf);
		
		// Write the visiblity array
		sprintf(keyname, "PlaylistVis%d", iColIDX);
		WritePrivateProfileString("WindowPos", keyname, options.playlist_column_visible[iColIDX] ? "1" : "0", pathbuf);
	}
	
	WritePrivateProfileString("WindowPos", // pointer to section name
	
							  "WindowX", // pointer to key name
							  _itoa(options.main_window_pos.x, intbuf, 10), // pointer to string to add
							  pathbuf // pointer to initialization filename
							 );
	WritePrivateProfileString("WindowPos", // pointer to section name
							  "WindowY", // pointer to key name
							  _itoa(options.main_window_pos.y, intbuf, 10), // pointer to string to add
							  pathbuf // pointer to initialization filename
							 );
	                         
	WritePrivateProfileString("WindowPos", // pointer to section name
							  "PlaylistX", // pointer to key name
							  _itoa(options.playlist_window_pos.left,
									intbuf, 10), // pointer to string to add
							  pathbuf // pointer to initialization filename
							 );
	WritePrivateProfileString("WindowPos", // pointer to section name
							  "PlaylistY", // pointer to key name
							  _itoa(options.playlist_window_pos.top,
									intbuf, 10), // pointer to string to add
							  pathbuf // pointer to initialization filename
							 );
	WritePrivateProfileString("WindowPos", // pointer to section name
							  "PlaylistW", // pointer to key name
							  _itoa(options.playlist_window_pos.right -
									options.playlist_window_pos.left, intbuf, 10), // pointer to string to add
							  pathbuf // pointer to initialization filename
							 );
	WritePrivateProfileString("WindowPos", // pointer to section name
							  "PlaylistH", // pointer to key name
							  _itoa(options.playlist_window_pos.bottom -
									options.playlist_window_pos.top, intbuf, 10), // pointer to string to add
							  pathbuf // pointer to initialization filename
							 );
	                         
	WritePrivateProfileString("LastDirectory", // pointer to section name
							  "Directory", // pointer to key name
							  options.last_used_directory, // pointer to string to add
							  pathbuf // pointer to initialization filename
							 );
	                         
	WritePrivateProfileString("Skin", "PlaylistSkin",
							  options.playlist_skin_file, pathbuf);
	                          
	{
		int     teller;
		int     profileteller = 1;
		char    SkinFileString[MAX_PATH];
		
		for (teller = MENU_SKIN_DEFAULT + 1; teller < MENU_SKIN_DEFAULT + 1 + options.remember_skin_count;
				teller++)
		{
		
			if (GetMenuString
					(globals.main_menu_popup, teller, options.main_skin_file,
					 MAX_PATH, MF_BYCOMMAND))
			{
			
				if (GetMenuState
						(globals.main_menu_popup, teller,
						 MF_BYCOMMAND) & MF_CHECKED)
				{
					options.last_selected_skin_number = profileteller;
				}
				
				sprintf(SkinFileString, "SkinFile%d", profileteller++);
				
				WritePrivateProfileString("Skin", SkinFileString,
										  options.main_skin_file, pathbuf);
				                          
			}
			
			else
			{
				sprintf(SkinFileString, "SkinFile%d", profileteller++);
				
				WritePrivateProfileString("Skin", SkinFileString, NULL,
										  pathbuf);
			}
		}
	}
	
	WritePrivateProfileString("Skin", "LastSkin",
							  _itoa(options.last_selected_skin_number,
									intbuf, 10), pathbuf);
	WritePrivateProfileString("Skin", "UsePlaylistSkin",
							  _itoa(options.use_playlist_skin, intbuf, 10),
							  pathbuf);
	                          
	WritePrivateProfileString("Skin", "UseDefault",
							  _itoa(options.use_default_skin, intbuf, 10),
							  pathbuf);
	WritePrivateProfileString("Misc", "Repeat",
							  _itoa(options.repeat_playlist, intbuf, 10),
							  pathbuf);
	WritePrivateProfileString("Misc", "Shuffle",
							  _itoa(options.shuffle_play, intbuf, 10),
							  pathbuf);
	WritePrivateProfileString("Misc", "Easymove",
							  _itoa(options.easy_move, intbuf, 10),
							  pathbuf);
	WritePrivateProfileString("Misc", "RotateIcon",
							  _itoa(options.rotate_systray_icon, intbuf,
									10), pathbuf);
	WritePrivateProfileString("Misc", "Ontop",
							  _itoa(options.always_on_top, intbuf, 10),
							  pathbuf);
	WritePrivateProfileString("Misc", "Autoexit",
							  _itoa(options.auto_exit_after_playing,
									intbuf, 10), pathbuf);
	WritePrivateProfileString("Misc", "Rememberpls",
							  _itoa(options.remember_playlist, intbuf, 10),
							  pathbuf);
	WritePrivateProfileString("Misc", "Remaining",
							  _itoa(options.show_remaining_time, intbuf,
									10), pathbuf);
	WritePrivateProfileString("Misc", "ReadID3tag",
							  _itoa(options.read_id3_tag, intbuf, 10),
							  pathbuf);
	WritePrivateProfileString("Misc", "ReadSelID3tag",
							  _itoa(options.read_id3_tag_of_selected,
									intbuf, 10), pathbuf);
	WritePrivateProfileString("Misc", "SuportID3v2",
							  _itoa(options.support_id3v2, intbuf, 10),
							  pathbuf);
	WritePrivateProfileString("Misc", "PreferNativeOGGtags",
							  _itoa(options.prefer_native_ogg_tags, intbuf, 10),
							  pathbuf);
	WritePrivateProfileString("Misc", "BackgroundReadID3",
							  _itoa(options.read_id3_tag_in_background, intbuf, 10),
							  pathbuf);
	WritePrivateProfileString("Misc", "WorkOutTrackLengths",
							  _itoa(options.work_out_track_lengths, intbuf, 10),
							  pathbuf);
	WritePrivateProfileString("Misc", "AllowMultipleInstances",
							  _itoa(options.allow_multiple_instances, intbuf,
									10), pathbuf);
	WritePrivateProfileString("Misc", "Outputmode",
							  _itoa(options.decoder_output_mode, intbuf,
									10), pathbuf);
	WritePrivateProfileString("Misc", "Scrolltitle",
							  _itoa(options.scroll_track_title, intbuf,
									10), pathbuf);
	WritePrivateProfileString("Misc", "ShowPlaylist",
							  _itoa(options.show_playlist, intbuf, 10),
							  pathbuf);
	                          
	{
		if (options.remember_last_played_track)
			WritePrivateProfileString("Misc", "RememberLastSong", options.initial_file, pathbuf);
		else
			WritePrivateProfileString("Misc", "RememberLastSong", "", pathbuf);
	}
	
	WritePrivateProfileString("Misc", "Fileonce",
							  _itoa(options.allow_file_once_in_playlist,
									intbuf, 10), pathbuf);
	WritePrivateProfileString("Misc", "Autoplay",
							  _itoa(options.auto_play_when_started, intbuf,
									10), pathbuf);
	WritePrivateProfileString("Misc", "TaskBar",
							  _itoa(options.show_on_taskbar, intbuf, 10),
							  pathbuf);
	WritePrivateProfileString("Misc", "DelayTime",
							  _itoa(options.seconds_delay_after_track,
									intbuf, 10), pathbuf);
	WritePrivateProfileString("Misc", "RememberSkins",
							  _itoa(options.remember_skin_count, intbuf,
									10), pathbuf);
	WritePrivateProfileString("Equalizer", "Active",
							  _itoa(options.equalizer, intbuf, 10),
							  pathbuf);
	                          
	for (teller = 1; teller <= 8; teller++)
	{
		char    keyname[100];
		sprintf(keyname, "Eq%d", teller);
		WritePrivateProfileString("Equalizer", keyname,
								  _itoa(options.eq_settings[teller],
										intbuf, 10), pathbuf);
	}
	
	// Write quick find defaults
	{
		char pcQuickFindOption[2];
		pcQuickFindOption[1] = '\0';
		
		if (options.m_enQuickFindTerm == qftTitle)
			pcQuickFindOption[0] = 'T';
		else if (options.m_enQuickFindTerm == qftArtist)
			pcQuickFindOption[0] = 'A';
		else if (options.m_enQuickFindTerm == qftAlbum)
			pcQuickFindOption[0] = 'M';
			
		WritePrivateProfileString("Misc", "QuickFindSearchTerm", pcQuickFindOption, pathbuf);
	}
	
	// Write out mixer mode
	
	if (globals.m_enMixerMode == mmMasterVolume)
		WritePrivateProfileString("Mixer", "Mode", "Master", pathbuf);
	else if (globals.m_enMixerMode == mmWaveVolume)
		WritePrivateProfileString("Mixer", "Mode", "Wave", pathbuf);
	else
		WritePrivateProfileString("Mixer", "Mode", "Internal", pathbuf);
		
	WritePrivateProfileString("Mixer", "InternalVolume", _itoa(globals.m_iVolume, intbuf, 10), pathbuf);
}
