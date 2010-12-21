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
#include "CPI_Player_Messages.h"


////////////////////////////////////////////////////////////////////////////////
//
// This is the Cooler play engine.  It runs in a seperate thread to the UI and
// receives it's messages through the Windows messaging system.  In this system
// CP_HPLAYER will cast into a custom struct.
//
// In case anybody was wondering, this is PlayerII because Player was the XAudio
// implementation of the play engine.
//
// This file contains the messaging and notification system.  The actual thread
// code is in CPI_Player_Engine.c
//
// The convention we use (throughout) in async message data is that the caller
// allocates and the callee frees.  The convention for sync message data is
// that the caller allocates and frees (so we can use stack variables).
//
////////////////////////////////////////////////////////////////////////////////


void CPI_Player__SetInternalVolume(CPs_PlayEngine* pPlayEngine, const int iNewVolume);
////////////////////////////////////////////////////////////////////////////////
//
//
//
CP_HPLAYER CPI_Player__Create(HWND hWndMain)
{
	CPs_PlayEngine* pNewPlayEngine;
	
	// Thread control object
	pNewPlayEngine = (CPs_PlayEngine*)malloc(sizeof(CPs_PlayEngine));
	pNewPlayEngine->m_hWndNotify = hWndMain;
	
	// Create event that will be signaled when the thread is ready for input
	pNewPlayEngine->m_hEvtThreadReady = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	// Create thread
	pNewPlayEngine->m_hThread = CreateThread(NULL, 0, CPI_Player__EngineEP, pNewPlayEngine, 0, &(pNewPlayEngine->m_dwThreadID));
	CP_ASSERT(pNewPlayEngine->m_hThread);
	
	// Wait for the engine to be ready for input
	WaitForSingleObject(pNewPlayEngine->m_hEvtThreadReady, INFINITE);
	CloseHandle(pNewPlayEngine->m_hEvtThreadReady);
	pNewPlayEngine->m_hEvtThreadReady = NULL;
	pNewPlayEngine->m_hVolumeMixer = NULL;
	
	CPI_Player__ReopenMixer((CP_HPLAYER)pNewPlayEngine);
	
	return (CP_HPLAYER)pNewPlayEngine;
}

//
//
//
void CPI_Player__Destroy(CP_HPLAYER hPlayer)
{
	CPs_PlayEngine* pPlayEngine = (CPs_PlayEngine*)hPlayer;
	CP_CHECKOBJECT(pPlayEngine);
	
	// Request thread to shutdown
	PostThreadMessage(pPlayEngine->m_dwThreadID, CPTM_STOP, 0, 0);
	PostThreadMessage(pPlayEngine->m_dwThreadID, CPTM_QUIT, 0, 0);
	
	// Close volume mixer
	
	if (pPlayEngine->m_hVolumeMixer)
		mixerClose(pPlayEngine->m_hVolumeMixer);
		
	// Wait for shutdown to actually happen
	WaitForSingleObject(pPlayEngine->m_hThread, 1000);
	
	TerminateThread(pPlayEngine->m_hThread, 0);
	
	// Cleanup
	CloseHandle(pPlayEngine->m_hThread);
	
	free(pPlayEngine);
}

//
//
//
void CPI_Player__ReopenMixer(CP_HPLAYER hPlayer)
{
	CPs_PlayEngine* pPlayEngine = (CPs_PlayEngine*)hPlayer;
	WAVEFORMATEX waveformatex;
	HWAVEOUT hWaveOut;
	CP_CHECKOBJECT(pPlayEngine);
	
	// Close volume mixer
	
	if (pPlayEngine->m_hVolumeMixer)
	{
		mixerClose(pPlayEngine->m_hVolumeMixer);
		pPlayEngine->m_hVolumeMixer = NULL;
	}
	
	if (globals.m_enMixerMode == mmMasterVolume || globals.m_enMixerMode == mmWaveVolume)
	{
		// Get the mixerID by opening a wave out stream and opening the mixer from that
		waveformatex.wFormatTag = WAVE_FORMAT_PCM;
		waveformatex.nChannels = 2;
		waveformatex.nSamplesPerSec = 44100;
		waveformatex.wBitsPerSample = 16;
		waveformatex.nBlockAlign = (waveformatex.nChannels * waveformatex.wBitsPerSample) >> 3;
		waveformatex.nAvgBytesPerSec = waveformatex.nSamplesPerSec * waveformatex.nBlockAlign;
		waveformatex.cbSize = 0;
		waveOutOpen(&hWaveOut,
					WAVE_MAPPER,
					&waveformatex,
					0,
					0, CALLBACK_NULL);
		            
		// Create a mixer control for volume (if we can)
		
		if (mixerOpen(&pPlayEngine->m_hVolumeMixer, (UINT)hWaveOut, (DWORD)pPlayEngine->m_hWndNotify, 0, CALLBACK_WINDOW | MIXER_OBJECTF_HWAVEOUT) == MMSYSERR_NOERROR)
		{
			MIXERCAPS mixercaps;
			DWORD dwLineID;
			DWORD dwNumControls;
			MIXERLINECONTROLS linecontrols;
			MIXERCONTROL mixercontrol;
			
			// Get the destination lineID for the speakers of the first wave out device
			mixerGetDevCaps((UINT)pPlayEngine->m_hVolumeMixer, &mixercaps, sizeof(mixercaps));
			
			{
				MIXERLINE mixerline;
				memset(&mixerline, 0, sizeof(mixerline));
				mixerline.cbStruct = sizeof(mixerline);
				
				if (globals.m_enMixerMode == mmMasterVolume)
				{
					mixerline.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
					mixerGetLineInfo((HMIXEROBJ)pPlayEngine->m_hVolumeMixer,
									 &mixerline,
									 MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE);
				}
				
				else
				{
				
					mixerline.Target.dwType = MIXERLINE_TARGETTYPE_WAVEOUT;
					mixerline.Target.wMid = mixercaps.wMid;
					mixerline.Target.wPid = mixercaps.wPid;
					mixerline.Target.vDriverVersion = mixercaps.vDriverVersion;
					strncpy(mixerline.Target.szPname, mixercaps.szPname, sizeof(mixerline.Target.szPname));
					
					mixerGetLineInfo((HMIXEROBJ)pPlayEngine->m_hVolumeMixer,
									 &mixerline,
									 MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_TARGETTYPE);
				}
				
				dwLineID = mixerline.dwLineID;
				
				dwNumControls = mixerline.cControls;
			}
			
			// Get the controlID of the volume fader
			memset(&linecontrols, 0, sizeof(linecontrols));
			linecontrols.cbStruct = sizeof(linecontrols);
			linecontrols.dwLineID = dwLineID;
			linecontrols.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
			linecontrols.cControls = 1;
			linecontrols.cbmxctrl = sizeof(MIXERCONTROL);
			linecontrols.pamxctrl = &mixercontrol;
			memset(&mixercontrol, 0, sizeof(mixercontrol));
			mixercontrol.cbStruct = sizeof(mixercontrol);
			
			mixerGetLineControls((HMIXEROBJ)pPlayEngine->m_hVolumeMixer,
								 &linecontrols,
								 MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);
			pPlayEngine->m_dwMixerControlID = mixercontrol.dwControlID;
			CPI_Player__SetInternalVolume(pPlayEngine, 100);
		}
		
		else
		{
			CP_TRACE0("Failed to open mixer");
			pPlayEngine->m_hVolumeMixer = NULL;
		}
		
		waveOutClose(hWaveOut);
	}
	
	else
	{
		CPI_Player__SetInternalVolume(pPlayEngine, globals.m_iVolume);
	}
}

//
//
//
void CPI_Player__OpenFile(CP_HPLAYER hPlayer, const char* pcFilename)
{
	char* pcStringCopy;
	CPs_PlayEngine* pPlayEngine = (CPs_PlayEngine*)hPlayer;
	CP_CHECKOBJECT(pPlayEngine);
	
	// Make copy of string data
	STR_AllocSetString(&pcStringCopy, pcFilename, FALSE);
	
	// Send message (callee will free string)
	PostThreadMessage(pPlayEngine->m_dwThreadID, CPTM_OPENFILE, (WPARAM)pcStringCopy, 0);
}

//
//
//
void CPI_Player__Seek(CP_HPLAYER hPlayer, const int iSeekPercent_Numerator, const int iSeekPercent_Denominator)
{
	CPs_PlayEngine* pPlayEngine = (CPs_PlayEngine*)hPlayer;
	CP_CHECKOBJECT(pPlayEngine);
	
	// Send message
	PostThreadMessage(pPlayEngine->m_dwThreadID, CPTM_SEEK, (WPARAM)iSeekPercent_Numerator, (LPARAM)iSeekPercent_Denominator);
}

//
//
//
void CPI_Player__Play(CP_HPLAYER hPlayer)
{
	CPs_PlayEngine* pPlayEngine = (CPs_PlayEngine*)hPlayer;
	CP_CHECKOBJECT(pPlayEngine);
	
	// Send message
	PostThreadMessage(pPlayEngine->m_dwThreadID, CPTM_PLAY, 0, 0);
}

//
//
//
void CPI_Player__Stop(CP_HPLAYER hPlayer)
{
	CPs_PlayEngine* pPlayEngine = (CPs_PlayEngine*)hPlayer;
	CP_CHECKOBJECT(pPlayEngine);
	
	// Send message
	PostThreadMessage(pPlayEngine->m_dwThreadID, CPTM_STOP, 0, 0);
}

//
//
//
void CPI_Player__Pause(CP_HPLAYER hPlayer)
{
	CPs_PlayEngine* pPlayEngine = (CPs_PlayEngine*)hPlayer;
	CP_CHECKOBJECT(pPlayEngine);
	
	// Send message
	PostThreadMessage(pPlayEngine->m_dwThreadID, CPTM_PAUSE, 0, 0);
}

//
//
//
void CPI_Player__BlockMessagesUntilEndOfStream(CP_HPLAYER hPlayer)
{
	CPs_PlayEngine* pPlayEngine = (CPs_PlayEngine*)hPlayer;
	CP_CHECKOBJECT(pPlayEngine);
	
	// Send message
	PostThreadMessage(pPlayEngine->m_dwThreadID, CPTM_BLOCKMSGUNTILENDOFSTREAM, 0, 0);
}

//
//
//
void CPI_Player__SetVolume(CP_HPLAYER hPlayer, const int iNewVolume)
{
	CPs_PlayEngine* pPlayEngine = (CPs_PlayEngine*)hPlayer;
	CP_CHECKOBJECT(pPlayEngine);
	
	// Set volume through mixer
	
	if (pPlayEngine->m_hVolumeMixer)
	{
		MIXERCONTROLDETAILS_UNSIGNED VolumeLevel;
		MIXERCONTROLDETAILS details;
		
		details.cbStruct = sizeof(details);
		details.dwControlID = pPlayEngine->m_dwMixerControlID;
		details.cChannels = 1;
		details.hwndOwner = NULL;
		details.cMultipleItems = 0;
		details.cbDetails = sizeof(VolumeLevel);
		details.paDetails = &VolumeLevel;
		VolumeLevel.dwValue = iNewVolume * 0x28F;
		
		mixerSetControlDetails((HMIXEROBJ)pPlayEngine->m_hVolumeMixer, &details, MIXER_OBJECTF_HMIXER);
	}
	
	else
		CPI_Player__SetInternalVolume(pPlayEngine, globals.m_iVolume);
}

//
//
//
int CPI_Player__GetVolume(CP_HPLAYER hPlayer)
{
	CPs_PlayEngine* pPlayEngine = (CPs_PlayEngine*)hPlayer;
	CP_CHECKOBJECT(pPlayEngine);
	
	// Get volume from mixer
	
	if (pPlayEngine->m_hVolumeMixer)
	{
		MIXERCONTROLDETAILS_UNSIGNED VolumeLevel;
		MIXERCONTROLDETAILS details;
		
		details.cbStruct = sizeof(details);
		details.dwControlID = pPlayEngine->m_dwMixerControlID;
		details.cChannels = 1;
		details.hwndOwner = NULL;
		details.cMultipleItems = 0;
		details.cbDetails = sizeof(VolumeLevel);
		details.paDetails = &VolumeLevel;
		VolumeLevel.dwValue = 0;
		
		mixerGetControlDetails((HMIXEROBJ)pPlayEngine->m_hVolumeMixer, &details, MIXER_OBJECTF_HMIXER);
		return VolumeLevel.dwValue / 0x28F;
	}
	
	else
		return globals.m_iVolume;
}

//
//
//
void CPI_Player__SetEQ(CP_HPLAYER hPlayer, const BOOL bEnabled, const int cBands[9])
{
	int iEQBandIDX;
	CPs_EQSettings* pNewEQSettings;
	CPs_PlayEngine* pPlayEngine = (CPs_PlayEngine*)hPlayer;
	CP_CHECKOBJECT(pPlayEngine);
	
	// Setup params (callee frees!)
	pNewEQSettings = (CPs_EQSettings*)malloc(sizeof(*pNewEQSettings));
	pNewEQSettings->m_bEnabled = bEnabled;
	
	// Setup bands - for some reason the UI is 1 based !!!
	
	for (iEQBandIDX = 0; iEQBandIDX < 8; iEQBandIDX++)
		pNewEQSettings->m_aryBands[iEQBandIDX] = (char)cBands[iEQBandIDX+1];
		
	PostThreadMessage(pPlayEngine->m_dwThreadID, CPTM_SETEQSETTINGS, (WPARAM)pNewEQSettings, 0);
}

//
//
//
void CPI_Player__EnumOutputDevices(CP_HPLAYER hPlayer)
{
	CPs_PlayEngine* pPlayEngine = (CPs_PlayEngine*)hPlayer;
	CP_CHECKOBJECT(pPlayEngine);
	
	// Send message
	PostThreadMessage(pPlayEngine->m_dwThreadID, CPTM_ENUMOUTPUTDEVICES, 0, 0);
}

//
//
//
void CPI_Player__SendSyncCookie(CP_HPLAYER hPlayer, const int iCookie)
{
	CPs_PlayEngine* pPlayEngine = (CPs_PlayEngine*)hPlayer;
	CP_CHECKOBJECT(pPlayEngine);
	
	// Send message
	PostThreadMessage(pPlayEngine->m_dwThreadID, CPTM_SENDSYNCCOOKIE, (WPARAM)iCookie, 0);
}

//
//
//
void CPI_Player__SetPositionRange(CP_HPLAYER hPlayer, const int iRange)
{
	CPs_PlayEngine* pPlayEngine = (CPs_PlayEngine*)hPlayer;
	CP_CHECKOBJECT(pPlayEngine);
	
	// Send message
	PostThreadMessage(pPlayEngine->m_dwThreadID, CPTM_SETPROGRESSTRACKLENGTH, (WPARAM)iRange, 0);
}

//
//
//
void CPI_Player__AssociateFileExtensions(CP_HPLAYER hPlayer)
{
	CPs_PlayEngine* pPlayEngine = (CPs_PlayEngine*)hPlayer;
	CP_CHECKOBJECT(pPlayEngine);
	
	// Send message
	PostThreadMessage(pPlayEngine->m_dwThreadID, CPTM_ASSOCIATEFILEEXTENSIONS, 0, 0);
}

//
//
//
BOOL CPI_Player__HandleNotifyMessages(CP_HPLAYER hPlayer, UINT uiMessage, WPARAM wParam, LPARAM lParam, LRESULT* plResult)
{
	// Skip message if it isn't for us
	if ((uiMessage < CPNM_first || uiMessage > CPNM_last)
			&& uiMessage != MM_MIXM_CONTROL_CHANGE)
	{
		return FALSE;
	}
	
	// Decode message
	
	switch (uiMessage)
	{
	
		case CPNM_PLAYERSTATE:
		{
			CPe_PlayerState enNewState = (CPe_PlayerState)wParam;
			CPI_Player_cb_OnPlayerState(hPlayer, enNewState);
		}
		
		break;
		
		case CPNM_FILEINFO:
		{
			CPs_FileInfo* pFileInfo = (CPs_FileInfo*)wParam;
			CPI_Player_cb_OnStreamInfo(hPlayer, pFileInfo);
			free(pFileInfo);
		}
		
		break;
		
		case CPNM_FILEOFFSET_SECS:
			CPI_Player_cb_OnStreamOffset_Secs(hPlayer, (int)wParam);
			break;
			
		case CPNM_FILEOFFSET_PROP:
			CPI_Player_cb_OnStreamOffset_Range(hPlayer, (int)wParam);
			break;
			
		case CPNM_SYNCCOOKIE:
			CPI_Player_cb_OnSyncCookie(hPlayer, (int)wParam);
			break;
			
		case CPNM_FOUNDOUTPUTDEVICE:
		{
			char* pcDeviceName = (char*)wParam;
			CPI_Player_cb_OnEnumOutputDevice(hPlayer, pcDeviceName, (int)lParam);
			
			if (*pcDeviceName)
				free(pcDeviceName);
		}
		
		break;
		
		case CPNM_SETSTREAMINGSTATE:
			CPI_Player_cb_OnStreamStateChange(hPlayer, (BOOL)wParam, (int)lParam);
			break;
			
		case MM_MIXM_CONTROL_CHANGE:
		{
			const DWORD dwControlID = (DWORD)lParam;
			CPs_PlayEngine* pPlayEngine = (CPs_PlayEngine*)hPlayer;
			CP_CHECKOBJECT(pPlayEngine);
			
			if (pPlayEngine->m_dwMixerControlID == dwControlID)
				CPI_Player_cb_OnVolumeChange(hPlayer, CPI_Player__GetVolume(hPlayer));
		}
		
		break;
	}
	
	return TRUE;
}

//
//
//
void CPI_Player__OnOutputDeviceChange(CP_HPLAYER hPlayer)
{
	CPs_PlayEngine* pPlayEngine = (CPs_PlayEngine*)hPlayer;
	CP_CHECKOBJECT(pPlayEngine);
	
	// Send message
	PostThreadMessage(pPlayEngine->m_dwThreadID, CPTM_ONOUTPUTMODULECHANGE, 0, 0);
}

//
//
//
void CPI_Player__SetInternalVolume(CPs_PlayEngine* pPlayEngine, const int iNewVolume)
{
	// Send message
	PostThreadMessage(pPlayEngine->m_dwThreadID, CPTM_SETINTERNALVOLUME, (WPARAM)iNewVolume, 0);
}

//
//
//

