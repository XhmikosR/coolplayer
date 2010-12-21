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
#include "CPI_Player_Engine.h"

////////////////////////////////////////////////////////////////////////////////
//
// This is the Cooler play engine.  It runs in a seperate thread to the UI and
// receives it's messages through the Windows messaging system.  In this system
// CP_HPLAYER will cast into a custom struct.
//
// This file contains the actual engine implementation.
//
// The convention we use (throughout) in async message data is that the caller
// allocates and the callee frees.  The convention for sync message data is
// that the caller allocates and frees (so we can use stack variables).
//
////////////////////////////////////////////////////////////////////////////////


void UpdateProgress(CPs_PlayerContext* pContext);
void EmptyOutputStream(CPs_PlayerContext* pContext);
void StartPlay(CPs_CoDecModule* pCoDec, CPs_PlayerContext* pContext);
void EnumOutputDevices(CPs_PlayerContext* pContext);
CPs_CoDecModule* OpenCoDec(CPs_PlayerContext* pContext, const char* pcFilename);
void CleanupCoDecs(CPs_PlayerContext* pContext);
void SetCurrentOutputModule(CPs_PlayerContext* pContext, CPs_OutputModule* pNewOuputModule, BOOL* pbForceRefill);
void AssociateFileExtensions(CPs_PlayerContext* pContext);
////////////////////////////////////////////////////////////////////////////////
//
//
//
DWORD WINAPI CPI_Player__EngineEP(void* pCookie)
{
	BOOL bTerminateThread = FALSE;
	HRESULT hr_ComState;
	CPs_PlayerContext playercontext;
	
	playercontext.m_pBaseEngineParams = (CPs_PlayEngine*)pCookie;
	playercontext.m_bOutputActive = FALSE;
	playercontext.m_iProportion_TrackLength = 0;
	playercontext.m_iLastSentTime_Secs = -1;
	playercontext.m_iLastSentTime_Proportion = -1;
	playercontext.m_iInternalVolume = 100;
	CP_CHECKOBJECT(playercontext.m_pBaseEngineParams);
	
	CP_TRACE0("Cooler Engine Startup");
	hr_ComState = CoInitialize(NULL);
		
	// Initialise CoDecs
	CP_InitialiseCodec_MPEG(&playercontext.m_CoDecs[CP_CODEC_MPEG]);
	CP_InitialiseCodec_WAV(&playercontext.m_CoDecs[CP_CODEC_WAV]);
	CP_InitialiseCodec_OGG(&playercontext.m_CoDecs[CP_CODEC_OGG]);
	CP_InitialiseCodec_WinAmpPlugin(&playercontext.m_CoDecs[CP_CODEC_WINAMPPLUGIN]);
	
	// Initialise output module
	
	if (options.decoder_output_mode > CP_OUTPUT_last)
		options.decoder_output_mode = CP_OUTPUT_last;
		
	playercontext.m_dwCurrentOutputModule = options.decoder_output_mode;
	
	CPI_Player_Output_Initialise_WaveMapper(&playercontext.m_OutputModules[CP_OUTPUT_WAVE]);
	CPI_Player_Output_Initialise_DirectSound(&playercontext.m_OutputModules[CP_OUTPUT_DIRECTSOUND]);
	CPI_Player_Output_Initialise_File(&playercontext.m_OutputModules[CP_OUTPUT_FILE]);
	
	playercontext.m_pCurrentOutputModule = &playercontext.m_OutputModules[playercontext.m_dwCurrentOutputModule];
	
	// Initialise EQ
	CPI_Player_Equaliser_Initialise_Basic(&playercontext.m_Equaliser);
	
	{
		CPs_PlayEngine* player = (CPs_PlayEngine*)pCookie;
		player->m_pContext = &playercontext;
	}
	
	// Initialise USER32.DLL for this thread
	{
		MSG msgDummy;
		PeekMessage(&msgDummy, 0, WM_USER, WM_USER, PM_NOREMOVE);
		
		// Signal this thread ready for input
		SetEvent(playercontext.m_pBaseEngineParams->m_hEvtThreadReady);
	}
	
	do
	{
		// Process any pending messages
		BOOL bForceRefill = FALSE;
		MSG msg;
		DWORD dwWaitResult;
		
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// Decode engine message
			switch (msg.message)
			{
			
				case CPTM_QUIT:
					bTerminateThread = TRUE;
					break;
					
				case CPTM_OPENFILE:
				{
					char* pcFilename = (char*)msg.wParam;
					
					// If there is another pending openfile then ignore this one
					// This helps when this thread is non responsive (on an http connect for example)
					// and the user is hammering the hell out of the play button (as I always
					// do) - this will cause a number of open/closes to be placed into the
					// message queue which will tie up this thread for ages!!
					MSG msg2;
					
					if (PeekMessage(&msg2, NULL, CPTM_OPENFILE, CPTM_OPENFILE, PM_NOREMOVE) == FALSE)
					{
						CPs_CoDecModule* pNewCoDec;
						
						// If there is a CoDec playing then shut it down
						
						if (playercontext.m_pCurrentOutputModule->m_pCoDec)
						{
							playercontext.m_pCurrentOutputModule->m_pCoDec->CloseFile(playercontext.m_pCurrentOutputModule->m_pCoDec);
							playercontext.m_pCurrentOutputModule->m_pCoDec = NULL;
						}
						
						CP_TRACE1("Openfile \"%s\"", pcFilename);
						
						pNewCoDec = OpenCoDec(&playercontext, pcFilename);
						
						// If the open failed then request a new stream from the interface
						
						if (pNewCoDec == NULL)
						{
							PostMessage(playercontext.m_pBaseEngineParams->m_hWndNotify, CPNM_PLAYERSTATE, (WPARAM)cppsEndOfStream, 0);
						}
						
						// Check the file's format - if the sample rate, nChannels or sample size has changed
						// then clear the current output and shutdown output device (this will cause a gap
						// - but only when the format changes)
						
						else if (playercontext.m_bOutputActive == TRUE)
						{
							CPs_FileInfo FileInfo;
							pNewCoDec->GetFileInfo(pNewCoDec, &FileInfo);
							
							if (FileInfo.m_iFreq_Hz != playercontext.m_iOpenDevice_Freq_Hz
									|| FileInfo.m_bStereo != playercontext.m_bOpenDevice_Stereo
									|| FileInfo.m_b16bit != playercontext.m_bOpenDevice_16bit)
							{
								CP_TRACE0("Stream format changes - clearing stream");
								EmptyOutputStream(&playercontext);
								StartPlay(pNewCoDec, &playercontext);
								bForceRefill = TRUE;
							}
						}
						
						playercontext.m_pCurrentOutputModule->m_pCoDec = pNewCoDec;
					}
					
					#ifdef _DEBUG
					else
					{
						CP_TRACE1("Openfile of \"%s\" ignored due to other opens in the queue", pcFilename);
					}
					#endif
					
					// Cleanup
					free(pcFilename);
				}
				
				break;
				
				case CPTM_SEEK:
				
					if (playercontext.m_bOutputActive == TRUE)
					{
						// Ignore message if there is another on it's way!
						MSG msg2;
						
						if (PeekMessage(&msg2, NULL, CPTM_SEEK, CPTM_SEEK, PM_NOREMOVE) == FALSE)
						{
							if (playercontext.m_pCurrentOutputModule->m_pCoDec)
								playercontext.m_pCurrentOutputModule->m_pCoDec->Seek(playercontext.m_pCurrentOutputModule->m_pCoDec, (int)msg.wParam, (int)msg.lParam);
								
							playercontext.m_pCurrentOutputModule->Flush(playercontext.m_pCurrentOutputModule);
							
							bForceRefill = TRUE;
						}
					}
					// FALLTHROUGH - to let coolplayer know playing has resumed (bugfix from seeking when paused)  */
					
				case CPTM_PLAY:
					if (playercontext.m_pCurrentOutputModule->m_pCoDec)
					{
						// If we don't have an output stage - initialise one now
						if (playercontext.m_bOutputActive == FALSE)
						{
							StartPlay(playercontext.m_pCurrentOutputModule->m_pCoDec, &playercontext);
							bForceRefill = TRUE;
						}
						
						playercontext.m_pCurrentOutputModule->SetPause(playercontext.m_pCurrentOutputModule, FALSE);
						
						PostMessage(playercontext.m_pBaseEngineParams->m_hWndNotify, CPNM_PLAYERSTATE, (WPARAM)cppsPlaying, 0);
						playercontext.m_iLastSentTime_Secs = -1;
						playercontext.m_iLastSentTime_Proportion = -1;
						UpdateProgress(&playercontext);
					}
					
					break;
					
				case CPTM_STOP:
				
					if (playercontext.m_pCurrentOutputModule->m_pCoDec)
					{
						playercontext.m_pCurrentOutputModule->m_pCoDec->CloseFile(playercontext.m_pCurrentOutputModule->m_pCoDec);
						playercontext.m_pCurrentOutputModule->m_pCoDec = NULL;
					}
					
					if (playercontext.m_bOutputActive == TRUE)
					{
						playercontext.m_bOutputActive = FALSE;
						playercontext.m_pCurrentOutputModule->Uninitialise(playercontext.m_pCurrentOutputModule);
					}
					
					PostMessage(playercontext.m_pBaseEngineParams->m_hWndNotify, CPNM_PLAYERSTATE, (WPARAM)cppsStopped, 0);
					
					break;
					
				case CPTM_PAUSE:
					CP_TRACE0("Pause");
					
					if (playercontext.m_bOutputActive == TRUE)
						playercontext.m_pCurrentOutputModule->SetPause(playercontext.m_pCurrentOutputModule, TRUE);
						
					PostMessage(playercontext.m_pBaseEngineParams->m_hWndNotify, CPNM_PLAYERSTATE, (WPARAM)cppsPaused, 0);
					
					break;
					
				case CPTM_SETPROGRESSTRACKLENGTH:
					playercontext.m_iProportion_TrackLength = (int)msg.wParam;
					
					break;
					
				case CPTM_SENDSYNCCOOKIE:
					PostMessage(playercontext.m_pBaseEngineParams->m_hWndNotify, CPNM_SYNCCOOKIE, msg.wParam, 0);
					
					break;
					
				case CPTM_BLOCKMSGUNTILENDOFSTREAM:
					EmptyOutputStream(&playercontext);
					
					break;
					
				case CPTM_ENUMOUTPUTDEVICES:
					EnumOutputDevices(&playercontext);
					
					break;
					
				case CPTM_SETEQSETTINGS:
				{
					MSG msg2;
					CPs_EQSettings* pEQ = (CPs_EQSettings*)msg.wParam;
					
					// If there is another pending EQ message do no processing for this one (try to reduce noise)
					
					if (PeekMessage(&msg2, NULL, CPTM_SETEQSETTINGS, CPTM_OPENFILE, PM_NOREMOVE) == FALSE)
					{
						BOOL bEQEnableStateChanged;
						playercontext.m_Equaliser.ApplySettings(&playercontext.m_Equaliser, pEQ, &bEQEnableStateChanged);
						
						// Empty the buffers (this will cause a discontinuity in the music but at least
						// the EQ setting change will be immediate
						
						if (playercontext.m_bOutputActive == TRUE && playercontext.m_pCurrentOutputModule->OnEQChanged)
							playercontext.m_pCurrentOutputModule->OnEQChanged(playercontext.m_pCurrentOutputModule);
					}
					
					free(pEQ);
				}
				
				break;
				
				case CPTM_ONOUTPUTMODULECHANGE:
				{
					playercontext.m_dwCurrentOutputModule = options.decoder_output_mode;
					SetCurrentOutputModule(&playercontext, NULL, &bForceRefill);
				}
				
				break;
				
				case CPTM_ASSOCIATEFILEEXTENSIONS:
					AssociateFileExtensions(&playercontext);
					break;
					
				case CPTM_SETINTERNALVOLUME:
					playercontext.m_iInternalVolume = (int)msg.wParam;
					
					if (playercontext.m_bOutputActive == TRUE && playercontext.m_pCurrentOutputModule->SetInternalVolume)
						playercontext.m_pCurrentOutputModule->SetInternalVolume(playercontext.m_pCurrentOutputModule, playercontext.m_iInternalVolume);
						
					break;
			}
		}
		
		if (bTerminateThread)
			break;
			
		// Wait for either another message or a buffer expiry (if we have a player)
		if (playercontext.m_bOutputActive)
		{
			dwWaitResult = 0L;
			
			if (bForceRefill == FALSE)
			{
				if (playercontext.m_pCurrentOutputModule->m_evtBlockFree)
					dwWaitResult = MsgWaitForMultipleObjects(1, &playercontext.m_pCurrentOutputModule->m_evtBlockFree, FALSE, 1000, QS_POSTMESSAGE);
				else
					dwWaitResult = WAIT_OBJECT_0;
			}
			
			// If the buffer event is signaled then request a refill
			
			if (bForceRefill == TRUE || dwWaitResult == WAIT_OBJECT_0)
			{
				if (playercontext.m_pCurrentOutputModule->m_pCoDec)
				{
					playercontext.m_pCurrentOutputModule->RefillBuffers(playercontext.m_pCurrentOutputModule);
					
					if (playercontext.m_pCurrentOutputModule->m_pCoDec == NULL)
					{
						// Tell UI that we need another file to play
						PostMessage(playercontext.m_pBaseEngineParams->m_hWndNotify, CPNM_PLAYERSTATE, (WPARAM)cppsEndOfStream, 0);
					}
					
					else
						UpdateProgress(&playercontext);
				}
				
				// If output has finished everything that it was doing - close the engine
				
				else if (playercontext.m_pCurrentOutputModule->IsOutputComplete(playercontext.m_pCurrentOutputModule) == TRUE)
				{
					playercontext.m_bOutputActive = FALSE;
					playercontext.m_pCurrentOutputModule->Uninitialise(playercontext.m_pCurrentOutputModule);
					PostMessage(playercontext.m_pBaseEngineParams->m_hWndNotify, CPNM_PLAYERSTATE, (WPARAM)cppsStopped, 0);
				}
			}
		}
		
		else
		{
			WaitMessage();
		}
	}
	
	while (bTerminateThread == FALSE);
	
	// Clean up output (if it's still active)
	if (playercontext.m_pCurrentOutputModule->m_pCoDec)
	{
		playercontext.m_pCurrentOutputModule->m_pCoDec->CloseFile(playercontext.m_pCurrentOutputModule->m_pCoDec);
		playercontext.m_pCurrentOutputModule->m_pCoDec = NULL;
	}
	
	if (playercontext.m_bOutputActive == TRUE)
		playercontext.m_pCurrentOutputModule->Uninitialise(playercontext.m_pCurrentOutputModule);
		
	// Clean up modules
	playercontext.m_Equaliser.Uninitialise(&playercontext.m_Equaliser);
	
	CleanupCoDecs(&playercontext);
	
	if (hr_ComState == S_OK)
		CoUninitialize();
		
	CP_TRACE0("Cooler Engine terminating");
	
	return 0;
}

//
//
//
void UpdateProgress(CPs_PlayerContext* pContext)
{
	int iCurrentTime_Secs;
	
	// If the file offset (in secs) has changed resend some notifies
	
	if (pContext->m_bOutputActive == TRUE)
		iCurrentTime_Secs = pContext->m_pCurrentOutputModule->m_pCoDec->GetCurrentPos_secs(pContext->m_pCurrentOutputModule->m_pCoDec);
	else
		iCurrentTime_Secs = 0;
		
	if (iCurrentTime_Secs != pContext->m_iLastSentTime_Secs)
	{
		CPs_FileInfo* pFileInfo;
		int iFileLength_Secs;
		pContext->m_iLastSentTime_Secs = iCurrentTime_Secs;
		
		// (Re)send file info first
		pFileInfo = (CPs_FileInfo*)malloc(sizeof(*pFileInfo));
		pContext->m_pCurrentOutputModule->m_pCoDec->GetFileInfo(pContext->m_pCurrentOutputModule->m_pCoDec, pFileInfo);
		iFileLength_Secs = pFileInfo->m_iFileLength_Secs;
		PostMessage(pContext->m_pBaseEngineParams->m_hWndNotify, CPNM_FILEINFO, (WPARAM)pFileInfo, 0);
		
		// Send current progress
		PostMessage(pContext->m_pBaseEngineParams->m_hWndNotify, CPNM_FILEOFFSET_SECS, (WPARAM)iCurrentTime_Secs, 0);
		
		// Send the proportion along the track (if it has changed)
		
		if (pContext->m_iProportion_TrackLength != 0 && iFileLength_Secs != 0)
		{
			int iProportionAlongTrack = (int)(((float)iCurrentTime_Secs / (float)iFileLength_Secs)
											  * (float)pContext->m_iProportion_TrackLength);
			                                  
			if (iProportionAlongTrack != pContext->m_iLastSentTime_Proportion)
			{
				pContext->m_iLastSentTime_Proportion = iProportionAlongTrack;
				PostMessage(pContext->m_pBaseEngineParams->m_hWndNotify, CPNM_FILEOFFSET_PROP, (WPARAM)iProportionAlongTrack, 0);
			}
		}
	}
}

//
//
//
void EmptyOutputStream(CPs_PlayerContext* pContext)
{
	if (pContext->m_bOutputActive == FALSE)
		return;
		
	while (pContext->m_pCurrentOutputModule->IsOutputComplete(pContext->m_pCurrentOutputModule) == FALSE)
	{
		WaitForSingleObject(pContext->m_pCurrentOutputModule->m_evtBlockFree, 1000);
		
		if (pContext->m_pCurrentOutputModule->m_pCoDec)
			UpdateProgress(pContext);
	}
	
	pContext->m_bOutputActive = FALSE;
	
	pContext->m_pCurrentOutputModule->Uninitialise(pContext->m_pCurrentOutputModule);
	PostMessage(pContext->m_pBaseEngineParams->m_hWndNotify, CPNM_PLAYERSTATE, (WPARAM)cppsStopped, 0);
}

//
//
//
void EnumOutputDevices(CPs_PlayerContext* pContext)
{
	int iOutputModuleIDX;
	// Enumerate output modules
	
	for (iOutputModuleIDX = 0; iOutputModuleIDX <= CP_OUTPUT_last; iOutputModuleIDX++)
	{
		const CPs_OutputModule* pOutputModule = pContext->m_OutputModules + iOutputModuleIDX;
		char* pcDeviceName;
		
		// Buffer frees in the called
		STR_AllocSetString(&pcDeviceName, pOutputModule->m_pcModuleName, FALSE);
		PostMessage(pContext->m_pBaseEngineParams->m_hWndNotify, CPNM_FOUNDOUTPUTDEVICE, (WPARAM)pcDeviceName, (LPARAM)iOutputModuleIDX);
	}
}

//
//
//
void StartPlay(CPs_CoDecModule* pCoDec, CPs_PlayerContext* pContext)
{
	CPs_FileInfo FileInfo;
	pCoDec->GetFileInfo(pCoDec, &FileInfo);
	pContext->m_bOutputActive = TRUE;
	pContext->m_iOpenDevice_Freq_Hz = FileInfo.m_iFreq_Hz;
	pContext->m_bOpenDevice_Stereo = FileInfo.m_bStereo;
	pContext->m_bOpenDevice_16bit = FileInfo.m_b16bit;
	
	// Get module to initialise itself
	pContext->m_Equaliser.Initialise(&pContext->m_Equaliser, FileInfo.m_iFreq_Hz, FileInfo.m_b16bit);
	pContext->m_pCurrentOutputModule->Initialise(pContext->m_pCurrentOutputModule, &FileInfo, &pContext->m_Equaliser);
	
	// If the volume isn't 100% then set the volume level
//    if(!pContext->m_iInternalVolume)
	pContext->m_pCurrentOutputModule->SetInternalVolume(pContext->m_pCurrentOutputModule, pContext->m_iInternalVolume);
}

//
//
//
CPs_CoDecModule* OpenCoDec(CPs_PlayerContext* pContext, const char* pcFilename)
{
	// const char* pcLastDot = NULL;
	int iCoDecIDX = 0;
	BOOL bOpenSucceeded;
	DWORD dwCookie = 0;
	
	// Find  the extension
	char *extension = NULL;
	char *dot = strrchr(pcFilename, '.');
	
	if (dot) extension = dot + 1;
	
	if (dot)
	{
		for (iCoDecIDX = CP_CODEC_first; iCoDecIDX <= CP_CODEC_last; iCoDecIDX++)
		{
			if (CPFA_IsAssociated(&pContext->m_CoDecs[iCoDecIDX], extension, &dwCookie) == TRUE)
			{
				bOpenSucceeded = pContext->m_CoDecs[iCoDecIDX].OpenFile(
									 &pContext->m_CoDecs[iCoDecIDX],
									 pcFilename,
									 dwCookie,
									 pContext->m_pBaseEngineParams->m_hWndNotify);
									 
				if (bOpenSucceeded == TRUE)
					return &pContext->m_CoDecs[iCoDecIDX];		
			}
		}
	}

	return NULL;
}

//
//
//
void CleanupCoDecs(CPs_PlayerContext* pContext)
{
	int iCoDecIDX = 0;
	
	for (iCoDecIDX = 0; iCoDecIDX <= CP_CODEC_last; iCoDecIDX++)
		pContext->m_CoDecs[iCoDecIDX].Uninitialise(&pContext->m_CoDecs[iCoDecIDX]);
}

//
//
//
void SetCurrentOutputModule(CPs_PlayerContext* pContext, CPs_OutputModule* pNewOuputModule, BOOL* pbForceRefill)
{
	if (!pNewOuputModule)
		pNewOuputModule = &pContext->m_OutputModules[pContext->m_dwCurrentOutputModule];
		
	// If the output module has changed then close the existing one and open the new one
	if (pContext->m_pCurrentOutputModule == pNewOuputModule)
		return;
		
	// Close existing
	if (pContext->m_bOutputActive)
	{
		pContext->m_pCurrentOutputModule->Uninitialise(pContext->m_pCurrentOutputModule);
		pContext->m_bOutputActive = FALSE;
	}
	
	// Switch the CoDec over to the new output module
	pNewOuputModule->m_pCoDec = pContext->m_pCurrentOutputModule->m_pCoDec;
	
	// Set the new module as current
	pContext->m_pCurrentOutputModule = pNewOuputModule;
	
	if (pContext->m_bOutputActive == FALSE && pContext->m_pCurrentOutputModule->m_pCoDec)
	{
		StartPlay(pContext->m_pCurrentOutputModule->m_pCoDec, pContext);
		*pbForceRefill = TRUE;
	}
}

//
//
//
void AssociateFileExtensions(CPs_PlayerContext* pContext)
{
	int iCoDecIDX;
	
	for (iCoDecIDX = 0; iCoDecIDX <= CP_CODEC_last; iCoDecIDX++)
		CPFA_AssociateWithEXE(&pContext->m_CoDecs[iCoDecIDX]);
}

//
//
//

