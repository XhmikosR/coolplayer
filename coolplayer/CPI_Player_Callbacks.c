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
#include "CPI_Player.h"
#include "CPI_Playlist.h"
#include "CPI_PlaylistItem.h"


////////////////////////////////////////////////////////////////////////////////
//
// These functions are called from the player engine but should not (in theory)
// be dependent upon it.
//
////////////////////////////////////////////////////////////////////////////////
#define CP_SYNCCOOKIE_DESTROY      301


////////////////////////////////////////////////////////////////////////////////
// Stream size and time codes
void CPI_Player_cb_OnStreamInfo(CP_HPLAYER hPlayer, const CPs_FileInfo* pInfo)
{
	globals.main_long_track_duration = pInfo->m_iFileLength_Secs;
	
	ZeroMemory(globals.main_text_bitrate, BITRATE_STRLEN);
	ZeroMemory(globals.main_text_frequency, FREQ_STRLEN);

	{
		int i;
		for (i = 0; i < 4; i++)
		{
			globals.main_text_bitrate[i] = ' ';
			globals.main_text_frequency[i] = ' ';
		}
	}

	// For those people who do like to know the bitrate (!!)
	if (pInfo->m_iBitRate_Kbs >= 1000)
		_itoa(pInfo->m_iBitRate_Kbs, globals.main_text_bitrate, 10);
	else if (pInfo->m_iBitRate_Kbs < 1000 && pInfo->m_iBitRate_Kbs > 100)
		_itoa(pInfo->m_iBitRate_Kbs, globals.main_text_bitrate+1, 10);
	else if (pInfo->m_iBitRate_Kbs < 100 && pInfo->m_iBitRate_Kbs > 10)
		_itoa(pInfo->m_iBitRate_Kbs, globals.main_text_bitrate+2, 10);
	else if (pInfo->m_iBitRate_Kbs < 10)
		_itoa(pInfo->m_iBitRate_Kbs, globals.main_text_bitrate+3, 10);
	else
		globals.main_text_bitrate[0] = '\0';
		


	if (pInfo->m_iFreq_Hz)
		_itoa(pInfo->m_iFreq_Hz / 1000, globals.main_text_frequency, 10);
	else
		globals.main_text_frequency[0] = '\0';
		
	main_draw_bitrate(windows.wnd_main);
	main_draw_frequency(windows.wnd_main);
}

//
//
//
void CPI_Player_cb_OnStreamOffset_Secs(CP_HPLAYER hPlayer, const int iTrackElapsedSeconds)
{
	globals.main_int_track_total_seconds = iTrackElapsedSeconds;
	main_draw_time(windows.wnd_main);
}

//
//
//
void CPI_Player_cb_OnStreamOffset_Range(CP_HPLAYER hPlayer, const int iTrackElapsed_Range)
{
	// Redraw progress bar
	globals.main_int_track_position = iTrackElapsed_Range;
	
	if (globals.m_bStreaming == TRUE)
		main_draw_vu_from_value(windows.wnd_main, PositionSlider, globals.m_iStreamingPortion);
	else
		main_draw_vu_from_value(windows.wnd_main, PositionSlider, globals.main_int_track_position);
}

//
//
//
////////////////////////////////////////////////////////////////////////////////
// State changes & sync
void CPI_Player_cb_OnPlayerState(CP_HPLAYER hPlayer, const CPe_PlayerState enPlayerState)
{
	switch (enPlayerState)
	{
	
		case cppsEndOfStream:
		{
			CP_HPLAYLISTITEM hCurrent = CPL_GetActiveItem(globals.m_hPlaylist);
			
			// Have we finished playing - if so do a sync close of the output device
			
			if (options.repeat_playlist == FALSE
					&& options.shuffle_play == FALSE
					&& (hCurrent == NULL || CPLI_Next(hCurrent) == NULL))
			{
				CPI_Player__BlockMessagesUntilEndOfStream(hPlayer);
				
				if (options.auto_exit_after_playing == TRUE)
					CPI_Player__SendSyncCookie(hPlayer, CP_SYNCCOOKIE_DESTROY);
			}
			
			else
			{
				// We still have more playing to do - do we pause before starting our next track
				if (options.seconds_delay_after_track > 0)
					SetTimer(windows.wnd_main, CPC_TIMERID_INTERTRACKDELAY, 2000 + (options.seconds_delay_after_track * 1000), NULL);
				else
					CPL_PlayItem(globals.m_hPlaylist, FALSE, pmNextItem);
			}
		}
		
		break;
		
		case cppsPlaying:	// FALLTHROUGH
		case cppsPaused:
			globals.m_enPlayerState = enPlayerState;
			main_draw_controls_all(windows.wnd_main);
			break;
			
		case cppsStopped:
			globals.m_enPlayerState = enPlayerState;
			
			memset(globals.main_text_bitrate, 0, sizeof(globals.main_text_bitrate));
			memset(globals.main_text_frequency, 0, sizeof(globals.main_text_frequency));
			main_draw_bitrate(windows.wnd_main);
			main_draw_frequency(windows.wnd_main);
			
			globals.main_int_track_total_seconds = 0;
			main_draw_time(windows.wnd_main);
			
			globals.main_int_track_position = 0;
			main_draw_vu_from_value(windows.wnd_main, PositionSlider, globals.main_int_track_position);
			
			main_draw_controls_all(windows.wnd_main);
			break;
			
		case cppsUndefined:
			break;
			
	}
}

//
//
//
void CPI_Player_cb_OnSyncCookie(CP_HPLAYER hPlayer, const int iCookie)
{
	if (iCookie == CP_SYNCCOOKIE_DESTROY)
		DestroyWindow(windows.wnd_main);
}

//
//
//
void CPI_Player_cb_OnVolumeChange(CP_HPLAYER hPlayer, const int iNewVolume)
{
	globals.m_iVolume = iNewVolume;
	main_draw_vu_from_value(windows.wnd_main, VolumeSlider, iNewVolume);
}

//
//
//
void CPI_Player_cb_OnStreamStateChange(CP_HPLAYER hPlayer, const BOOL bStreaming, const int iBufferUsagePercent)
{
	const int iRange = Skin.Object[PositionSlider].maxw ? Skin.Object[PositionSlider].h : Skin.Object[PositionSlider].w;
	
	// Set streaming params
	globals.m_bStreaming = bStreaming;
	
	if (iRange)
		globals.m_iStreamingPortion = (iBufferUsagePercent * iRange) / 100;
	else
		globals.m_iStreamingPortion = 0;
		
	main_draw_vu_from_value(windows.wnd_main, PositionSlider, globals.m_iStreamingPortion);
}

//
//
//
////////////////////////////////////////////////////////////////////////////////
// Output device enumeration
void CPI_Player_cb_OnEnumOutputDevice(CP_HPLAYER hPlayer, const char* pcDeviceName, const int iDeviceID)
{
	int iNewDeviceIDX;
	
	// Add this device to the configure dialog
	iNewDeviceIDX = SendDlgItemMessage(windows.dlg_options, IDC_OUTPUT, CB_ADDSTRING, 0, (LPARAM)pcDeviceName);
	SendDlgItemMessage(windows.dlg_options, IDC_OUTPUT, CB_SETITEMDATA, iNewDeviceIDX - 1, MAKELPARAM(iDeviceID, iDeviceID));
	
	// Ensure that the correct item is selected
	SendDlgItemMessage(windows.dlg_options, IDC_OUTPUT, CB_SETCURSEL, options.decoder_output_mode, 0);
}

//
//
//

