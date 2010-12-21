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
// Known CoDecs with ids (must be sequential and zero based)
#define CP_CODEC_WINAMPPLUGIN		0x0
#define CP_CODEC_OGG				0x1
#define CP_CODEC_WAV				0x2
#define CP_CODEC_MPEG				0x3
#define CP_CODEC_first				CP_CODEC_WINAMPPLUGIN
#define CP_CODEC_last				CP_CODEC_MPEG

// Set the CoDec to use if we cannot match a file by it's extension
#define CP_CODEC_default  CP_CODEC_WINAMPPLUGIN

//
// Known Output modules (must be sequential and zero based)
//
#define CP_OUTPUT_WAVE				0x0
#define CP_OUTPUT_DIRECTSOUND		0x1
#define CP_OUTPUT_FILE				0x2
#define CP_OUTPUT_first				CP_OUTPUT_WAVE
#define CP_OUTPUT_last				CP_OUTPUT_FILE


////////////////////////////////////////////////////////////////////////////////
//
// CPI_Player
//
// Here are the functions to control the back end of CoolPlayer.  All activity
// relating to Streaming, Decoding and sound output will be accessed through
// these functions.
//
//
// Basic player creation and destruction.  Window handle is used at the moment
// to receive notification messages.
CP_HPLAYER CPI_Player__Create(HWND hWndMain);
void CPI_Player__Destroy(CP_HPLAYER hPlayer);
//
//
// Player control verbs.  Seeks are floats between 0.0 and 1.0
void CPI_Player__ReopenMixer(CP_HPLAYER hPlayer);
void CPI_Player__OpenFile(CP_HPLAYER hPlayer, const char* pcFilename);
void CPI_Player__Seek(CP_HPLAYER hPlayer, const int iSeekPercent_Numerator, const int iSeekPercent_Denominator);
void CPI_Player__Play(CP_HPLAYER hPlayer);
void CPI_Player__Stop(CP_HPLAYER hPlayer);
void CPI_Player__Pause(CP_HPLAYER hPlayer);
void CPI_Player__BlockMessagesUntilEndOfStream(CP_HPLAYER hPlayer);
void CPI_Player__OnOutputDeviceChange(CP_HPLAYER hPlayer);
//
//
// Output control verbs.
void CPI_Player__SetVolume(CP_HPLAYER hPlayer, const int iNewVolume);  // iVolume range is 0...100
int CPI_Player__GetVolume(CP_HPLAYER hPlayer); // Returns volume in 0...100
void CPI_Player__SetEQ(CP_HPLAYER hPlayer, const BOOL bEnabled, const int cBands[9]);  // cBands range is -127...+127
//
//
// Enumerate output devices (see notify messages below)
void CPI_Player__EnumOutputDevices(CP_HPLAYER hPlayer);
//
//
// Synchorise (see notify messages below)
void CPI_Player__SendSyncCookie(CP_HPLAYER hPlayer, const int iCookie);
//
//
// This setting determines the scale and precision required for the CPI_Player_cb_OnStreamOffset_Range
// notification
void CPI_Player__SetPositionRange(CP_HPLAYER hPlayer, const int iRange);
//
// Get the engine to perform file assiciations (as it knows what file types we support)
void CPI_Player__AssociateFileExtensions(CP_HPLAYER hPlayer);
////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////
//
// Message and notify routing - XAudio is currently set up to notify via
// Windows messages so CPI_Player__HandleNotifyMessages will have a crack at the main message
// queue, decode any codec notify messages and then call the handler function (prototyped
// here but implemented by the UI)
//
// Return true if message was handled.
BOOL CPI_Player__HandleNotifyMessages(CP_HPLAYER hPlayer, UINT uiMessage, WPARAM wParam, LPARAM lParam, LRESULT* plResult);
//
// These notifies are a result of normal playing
void CPI_Player_cb_OnStreamInfo(CP_HPLAYER hPlayer, const CPs_FileInfo* pInfo);
void CPI_Player_cb_OnStreamOffset_Secs(CP_HPLAYER hPlayer, const int iTrackElapsedSeconds);
void CPI_Player_cb_OnStreamOffset_Range(CP_HPLAYER hPlayer, const int iTrackElapsed_Range);
void CPI_Player_cb_OnPlayerState(CP_HPLAYER hPlayer, const CPe_PlayerState enPlayerState);
void CPI_Player_cb_OnVolumeChange(CP_HPLAYER hPlayer, const int iNewVolume); // iNewVolume range is 0...100
//
// This notify is called as a result of a previous call to CPI_Player__EnumOutputDevices.  It
// is called once per device.
void CPI_Player_cb_OnEnumOutputDevice(CP_HPLAYER hPlayer, const char* pcDeviceName, const int iDeviceID);
//
// This notify is called as a result of a previous call to CPI_Player__SendSyncCookie.  The idea
// behind this mechanism is that the sync cookie is returned when all preceding messages have been
// processed.
void CPI_Player_cb_OnSyncCookie(CP_HPLAYER hPlayer, const int iCookie);
//
// This notify is sent whenever the streaming system has a buffer size report or
// a state change
void CPI_Player_cb_OnStreamStateChange(CP_HPLAYER hPlayer, const BOOL bStreaming, const int iBufferUsagePercent);
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
//
// Equaliser stuff
//
// Forward reference

struct _CPs_EqualiserModule;

typedef struct _CPs_EqualiserModule* CP_HEQUALISER;  // This will acutally be implemented as a CPs_EqualiserModule*
//
////////////////////////////////////////////////////////////////////////////////
