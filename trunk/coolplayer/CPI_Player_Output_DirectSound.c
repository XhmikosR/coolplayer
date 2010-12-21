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
#include "CPI_Player_CoDec.h"
#include "CPI_Player_Output.h"
#include "CPI_Equaliser.h"
#define DIRECTSOUND_VERSION 0x0500  /* Version 5.0 */
#include <dsound.h>
#include <math.h>

////////////////////////////////////////////////////////////////////////////////
//
// This is an output stage that uses DirectSound.
// here - just a circular buffer of wave blocks and an event controlling the
// lot.  This should be considered the "reference" output stage.
//
////////////////////////////////////////////////////////////////////////////////

#define CPC_OUTPUTBLOCKSIZE   0x10000 // 64KB block
#define CPC_MAXFILLAMOUNT   (CPC_OUTPUTBLOCKSIZE>>4) // 1/16th of total block size
#define CPC_INVALIDCURSORPOS  0xFFFFFFFF
////////////////////////////////////////////////////////////////////////////////
//

typedef struct __CPs_OutputContext_DirectSound
{
	LPDIRECTSOUNDBUFFER lpDSB;
	LPDIRECTSOUND lpDirectSound;
	WAVEFORMATEX WaveFile;
	DWORD m_WriteCursor;
	BOOL m_TermState_Wrapped;
	DWORD m_TermState_WriteCursor;
	DWORD m_TermState_HighestPlayPos;
	DWORD m_TimerId;
	BOOL m_bStreamRunning;
	CPs_EqualiserModule* m_pEqualiser;
	BYTE* m_pShadowBuffer;
} CPs_OutputContext_DirectSound;

//
////////////////////////////////////////////////////////////////////////////////



void CPP_OMDS_Initialise(CPs_OutputModule* pModule, const CPs_FileInfo* pFileInfo, CP_HEQUALISER hEqualiser);
void CPP_OMDS_Uninitialise(CPs_OutputModule* pModule);
void CPP_OMDS_RefillBuffers(CPs_OutputModule* pModule);
void CPP_OMDS_SetPause(CPs_OutputModule* pModule, const BOOL bPause);
BOOL CPP_OMDS_IsOutputComplete(CPs_OutputModule* pModule);
void CPP_OMDS_Flush(CPs_OutputModule* pModule);
void CPP_OMDS_SetVolume(CPs_OutputModule* pModule, int iVolume);
void CPP_OMDS_GetVolume(CPs_OutputModule* pModule, int *iVolume, HANDLE waitevent);
void CPP_OMDS_EnablePlay(CPs_OutputModule* pModule, const BOOL bEnable);
void CPP_OMDS_OnEQChanged(CPs_OutputModule* pModule);
void CPP_OMDS_SetInternalVolume(CPs_OutputModule* pModule, const int iNewVolume);

////////////////////////////////////////////////////////////////////////////////
//
//
//
void CPI_Player_Output_Initialise_DirectSound(CPs_OutputModule* pModule)
{
	// This is a one off call to set up the function pointers
	pModule->Initialise = CPP_OMDS_Initialise;
	pModule->Uninitialise = CPP_OMDS_Uninitialise;
	pModule->RefillBuffers = CPP_OMDS_RefillBuffers;
	pModule->SetPause = CPP_OMDS_SetPause;
	pModule->IsOutputComplete = CPP_OMDS_IsOutputComplete;
	pModule->Flush = CPP_OMDS_Flush;
	pModule->OnEQChanged = CPP_OMDS_OnEQChanged;
	pModule->SetInternalVolume = CPP_OMDS_SetInternalVolume;
	pModule->m_pModuleCookie = NULL;
	pModule->m_pcModuleName = "DirectSound Plugout";
	pModule->m_pCoDec = NULL;
	pModule->m_pEqualiser = NULL;
}

//
//
void CPP_OMDS_Initialise(CPs_OutputModule* pModule, const CPs_FileInfo* pFileInfo, CP_HEQUALISER hEqualiser)
{
	// This is called when some playing is required.
	// Do all allocation here so that we do not hold
	// resources while we are just sitting waiting for
	// something to happen.
	
	// Create a context
	DSBUFFERDESC dsbd;
	HRESULT hrRetVal;
	
	CPs_OutputContext_DirectSound* pContext;
	CP_ASSERT(pModule->m_pModuleCookie == NULL);
	pContext = (CPs_OutputContext_DirectSound*)malloc(sizeof(CPs_OutputContext_DirectSound));
	pModule->m_pModuleCookie = pContext;
	CP_TRACE0("DirectSound initialising");
	
	// Create sync object
	pModule->m_evtBlockFree = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	// Create the Direct Sound Object
	
	if (DirectSoundCreate(NULL, &(pContext->lpDirectSound), NULL))
	{
		CP_FAIL("Cannot create DirectSound Object");
	}
	
	if (IDirectSound_SetCooperativeLevel(pContext->lpDirectSound, windows.wnd_main, DSSCL_NORMAL))
	{
		CPP_OMDS_Uninitialise(pModule);
		CP_FAIL("Can\'t set DirectSound Cooperative level");
	}
	
	if (!pContext->lpDirectSound)
	{
		CPP_OMDS_Uninitialise(pModule);
		CP_FAIL("Unable to initialise DirectSound");
	}
	
	pContext->WaveFile.wFormatTag = WAVE_FORMAT_PCM;
	
	pContext->WaveFile.nChannels = pFileInfo->m_bStereo ? 2 : 1;
	pContext->WaveFile.nSamplesPerSec = pFileInfo->m_iFreq_Hz;
	pContext->WaveFile.wBitsPerSample = pFileInfo->m_b16bit ? 16 : 8;
	pContext->WaveFile.nBlockAlign = (pContext->WaveFile.nChannels * pContext->WaveFile.wBitsPerSample) >> 3;
	pContext->WaveFile.nAvgBytesPerSec = pContext->WaveFile.nSamplesPerSec * pContext->WaveFile.nBlockAlign;
	pContext->WaveFile.cbSize = 0;
	
	// Create sound buffer
	memset(&dsbd, 0, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME;
	dsbd.dwBufferBytes = CPC_OUTPUTBLOCKSIZE;
	dsbd.lpwfxFormat = &pContext->WaveFile;
	hrRetVal = IDirectSound_CreateSoundBuffer(pContext->lpDirectSound,
			   &dsbd,
			   &(pContext->lpDSB),
			   NULL);
	           
	if (FAILED(hrRetVal))
	{
		pContext->lpDSB = NULL;
		CP_FAIL("Cannot create soundbuffer");
	}
	
	// Empty sound buffer
	{
		BYTE *pbData = NULL;
		DWORD dwLength;
		
		// Lock DirectSound buffer
		IDirectSoundBuffer_Lock(pContext->lpDSB,
								0,
								CPC_OUTPUTBLOCKSIZE,
								(LPVOID *) &pbData,
								&dwLength,
								NULL,
								NULL,
								0);
		                        
		if (pbData)
			memset(pbData, 0, dwLength);
			
		// Unlock the buffer
		IDirectSoundBuffer_Unlock(pContext->lpDSB,
								  pbData,
								  dwLength,
								  NULL,
								  0L);
	}
	
	// Set bufferoffset to 0
	pContext->m_WriteCursor = CPC_INVALIDCURSORPOS;
	pContext->m_TermState_Wrapped = FALSE;
	pContext->m_TermState_WriteCursor = CPC_INVALIDCURSORPOS;
	pContext->m_TermState_HighestPlayPos = CPC_INVALIDCURSORPOS;
	pContext->m_TimerId = 0;
	pContext->m_bStreamRunning = FALSE;
	
	// Create shadow buffer (for live EQ)
	pContext->m_pShadowBuffer = (BYTE*)malloc(CPC_OUTPUTBLOCKSIZE);
	
	memset(pContext->m_pShadowBuffer, 0, CPC_OUTPUTBLOCKSIZE);
	
	// Setup thread prioity to max - If you are having problems during
	// debugging (with a big hang!) then comment this next line out
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	
	pModule->m_pEqualiser = hEqualiser;
}

//
//
//
void CPP_OMDS_Uninitialise(CPs_OutputModule* pModule)
{
	CPs_OutputContext_DirectSound* pContext = (CPs_OutputContext_DirectSound*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	CP_TRACE0("DirectSound shutting down");
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
	
	if (pContext->lpDSB)
	{
		// Destroy the direct sound buffer.
		IDirectSoundBuffer_Release(pContext->lpDSB);
	}
	
	if (pContext->lpDirectSound)
	{
		// Destroy the direct sound object.
		IDirectSound_Release(pContext->lpDirectSound);
		CloseHandle(pModule->m_evtBlockFree);
	}
	
	if (pContext->m_TimerId)
	{
		timeKillEvent(pContext->m_TimerId);
		pContext->m_TimerId = 0;
	}
	
	pContext->lpDirectSound = NULL;
	
	pContext->lpDSB = NULL;
	free(pContext->m_pShadowBuffer);
	free(pContext);
	pContext = NULL;
	pModule->m_pModuleCookie = NULL;
}

//
//
//
void GetPlayPosAndInvalidLength(CPs_OutputContext_DirectSound* pContext, DWORD* pdwPlayPos, DWORD* pdwLength)
{
	HRESULT hrResult;
	
	hrResult = IDirectSoundBuffer_GetCurrentPosition(pContext->lpDSB, pdwPlayPos, NULL);
	
	if (FAILED(hrResult))
		CP_TRACE0("Failed call to IDirectSoundBuffer_GetCurrentPosition");
		
	// Get amount of free data in the buffer
	if (pContext->m_WriteCursor != CPC_INVALIDCURSORPOS)
	{
		if (pContext->m_WriteCursor >= *pdwPlayPos)
			*pdwLength = (CPC_OUTPUTBLOCKSIZE - pContext->m_WriteCursor) + *pdwPlayPos;
		else
			*pdwLength = *pdwPlayPos - pContext->m_WriteCursor;
	}
	
	else
	{
		// First write
		pContext->m_WriteCursor = 0;
		*pdwLength = CPC_OUTPUTBLOCKSIZE;
	}
}

//
//
//
void CPP_OMDS_RefillBuffers(CPs_OutputModule* pModule)
{
	HRESULT hrRetVal;
	BYTE *pbData;
	DWORD dwLength = 0;
	BOOL bMoreData = TRUE;
	DWORD RealLength;
	DWORD dwAmountToFill, dwCurrentPlayCursor;
	CPs_OutputContext_DirectSound* pContext = (CPs_OutputContext_DirectSound*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	if (!pContext->lpDirectSound)
		return;
		
	GetPlayPosAndInvalidLength(pContext, &dwCurrentPlayCursor, &dwAmountToFill);
	
	// Limit the amount of filling that we do per batch
	// - This will improve seek time and skipping on heavilly loaded systems
	if (dwAmountToFill > CPC_MAXFILLAMOUNT && pContext->m_bStreamRunning == FALSE)
		dwAmountToFill = CPC_MAXFILLAMOUNT;
		
	// Try to totally fill the buffer with sound data - this is the best policy because our
	// timer (event trigger) is likely to have v.poor accrucy
	if (dwAmountToFill > 0)
	{
		// Lock DirectSound buffer
		hrRetVal = IDirectSoundBuffer_Lock(pContext->lpDSB,
										   0,
										   CPC_OUTPUTBLOCKSIZE,
										   (LPVOID *) & pbData,
										   &dwLength,
										   NULL,
										   NULL,
										   0);
		                                   
		if (FAILED(hrRetVal))
		{
			CPP_OMDS_Uninitialise(pModule);
			CP_FAIL("Cannot lock soundbuffer");
		}
		
		// Write data into shadow buffer
		
		if ((pContext->m_WriteCursor + dwAmountToFill) >= CPC_OUTPUTBLOCKSIZE)
			RealLength = CPC_OUTPUTBLOCKSIZE - pContext->m_WriteCursor;
		else
			RealLength = dwAmountToFill;
			
		if (RealLength)
		{
			bMoreData = pModule->m_pCoDec->GetPCMBlock(pModule->m_pCoDec, pContext->m_pShadowBuffer + pContext->m_WriteCursor, &RealLength);
			memcpy(pbData + pContext->m_WriteCursor, pContext->m_pShadowBuffer + pContext->m_WriteCursor, RealLength);
		}
		
		// If there is EQ then apply it
		{
			// Note that the EQ module is initialised and uninitialsed by the engine
			CPs_EqualiserModule* pEQModule = (CPs_EqualiserModule*)pModule->m_pEqualiser;
			
			if (RealLength)
				pEQModule->ApplyEQToBlock_Inplace(pEQModule, pbData + pContext->m_WriteCursor, RealLength);
		}
		
		// Move cursor
		pContext->m_WriteCursor += RealLength;
		
		if (pContext->m_WriteCursor >= CPC_OUTPUTBLOCKSIZE)
			pContext->m_WriteCursor -= CPC_OUTPUTBLOCKSIZE;
			
		// Unlock the buffer
		hrRetVal = IDirectSoundBuffer_Unlock(pContext->lpDSB,
											 pbData,
											 dwLength,
											 NULL,
											 0L);
		                                     
		if (FAILED(hrRetVal))
		{
			CPP_OMDS_Uninitialise(pModule);
			CP_FAIL("Cannot Unlock soundbuffer");
		}
	}
	
	if (bMoreData == FALSE)
	{
		CP_TRACE0("Stream exhausted");
		pModule->m_pCoDec->CloseFile(pModule->m_pCoDec);
		pModule->m_pCoDec = NULL;
	}
	
	pContext->m_TermState_Wrapped = FALSE;
	
	pContext->m_TermState_WriteCursor = CPC_INVALIDCURSORPOS;
	pContext->m_TermState_HighestPlayPos = CPC_INVALIDCURSORPOS;
	
	if (!pContext->m_bStreamRunning)
		CPP_OMDS_EnablePlay(pModule, TRUE);
}

//
//
//
void CPP_OMDS_SetPause(CPs_OutputModule* pModule, const BOOL bPause)
{
	CPs_OutputContext_DirectSound* pContext = (CPs_OutputContext_DirectSound*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	if (!pContext->lpDirectSound)
		return;
		
	CPP_OMDS_EnablePlay(pModule, !bPause);
}

//
//
//
BOOL CPP_OMDS_IsOutputComplete(CPs_OutputModule* pModule)
{
	CPs_OutputContext_DirectSound* pContext = (CPs_OutputContext_DirectSound*)pModule->m_pModuleCookie;
	DWORD dwCurrentPlayCursor, dwInvalidLength;
	CP_CHECKOBJECT(pContext);
	
	// Sound isn't open
	
	if (!pContext->lpDirectSound)
		return TRUE;
		
	// Was there ever any data in this buffer
	if (pContext->m_WriteCursor == CPC_INVALIDCURSORPOS)
		return TRUE;
		
	// Work out write cursor at terminate time
	if (pContext->m_TermState_WriteCursor == CPC_INVALIDCURSORPOS)
		pContext->m_TermState_WriteCursor = pContext->m_WriteCursor;
		
	if (pModule->m_pCoDec)
		return FALSE;
		
	GetPlayPosAndInvalidLength(pContext, &dwCurrentPlayCursor, &dwInvalidLength);
	
	// Empty the invalid area - this is because we cannot be sure that some of the
	// invalid area will no be played because our event doesn't com in on time
	if (dwInvalidLength > 0)
	{
		BYTE *pbData = NULL;
		DWORD dwLength = 0;
		
		// Lock DirectSound buffer
		IDirectSoundBuffer_Lock(pContext->lpDSB,
								pContext->m_WriteCursor,
								dwInvalidLength,
								(LPVOID *) &pbData,
								&dwLength,
								NULL,
								NULL,
								0);
		                        
		if (pbData)
			memset(pbData, 0, dwLength);
			
		// Unlock the buffer
		IDirectSoundBuffer_Unlock(pContext->lpDSB,
								  pbData,
								  dwLength,
								  NULL,
								  0L);
		                          
		// Move write cursor
		pContext->m_WriteCursor += dwInvalidLength;
		
		if (pContext->m_WriteCursor >= CPC_OUTPUTBLOCKSIZE)
			pContext->m_WriteCursor -= CPC_OUTPUTBLOCKSIZE;
			
	}
	
	// For this function to work there needs to be 2 calls - one to detect the wrap
	// and another to detect the termination - it is therefore necessary for the
	// event to be triggered on a reasonably regular interval
	
	if (pContext->m_TermState_HighestPlayPos == CPC_INVALIDCURSORPOS || pContext->m_TermState_HighestPlayPos < dwCurrentPlayCursor)
		pContext->m_TermState_HighestPlayPos = dwCurrentPlayCursor;
	else
		pContext->m_TermState_Wrapped = TRUE;
		
	if (pContext->m_TermState_Wrapped == FALSE)
		return FALSE;
		
	if (dwCurrentPlayCursor > pContext->m_TermState_WriteCursor)
		return TRUE;
		
	return FALSE;
}

//
//
//
void CPP_OMDS_Flush(CPs_OutputModule* pModule)
{
	DWORD dwCurrentPlayCursor;
	CPs_OutputContext_DirectSound* pContext = (CPs_OutputContext_DirectSound*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	if (!pContext->lpDirectSound)
		return;
		
	CPP_OMDS_EnablePlay(pModule, FALSE);
	
	IDirectSoundBuffer_GetCurrentPosition(pContext->lpDSB, &dwCurrentPlayCursor, NULL);
	
	pContext->m_WriteCursor = dwCurrentPlayCursor;
}

//
//
//
void CPP_OMDS_EnablePlay(CPs_OutputModule* pModule, const BOOL bEnable)
{
	CPs_OutputContext_DirectSound* pContext = (CPs_OutputContext_DirectSound*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	pContext->m_bStreamRunning = bEnable;
	
	// Setup event timer - according to our current requirements
	
	if (bEnable)
	{
		IDirectSoundBuffer_Play(pContext->lpDSB, 0, 0, DSBPLAY_LOOPING);
		
		// If there is a CoDec set up looping play mode
		
		if (pContext->m_TimerId == 0)
		{
			pContext->m_TimerId = timeSetEvent(20,
											   10, // 10ms Resolution
											   (LPTIMECALLBACK)pModule->m_evtBlockFree,
											   0,
											   TIME_PERIODIC | TIME_CALLBACK_EVENT_SET);
		}
	}
	
	else
	{
		IDirectSoundBuffer_Stop(pContext->lpDSB);
		timeKillEvent(pContext->m_TimerId);
		pContext->m_TimerId = 0;
	}
}

//
//
//
void CPP_OMDS_OnEQChanged(CPs_OutputModule* pModule)
{
#if 1
#ifdef _DEBUG
	CPs_OutputContext_DirectSound* pContext = (CPs_OutputContext_DirectSound*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
#endif
#else
	CPs_OutputContext_DirectSound* pContext = (CPs_OutputContext_DirectSound*)pModule->m_pModuleCookie;
	BYTE *pbData = NULL;
	DWORD dwLength = 0;
//    DWORD dwPlayPos;
//    CPs_EqualiserModule* pEQModule;
	CP_CHECKOBJECT(pContext);
	
	// Get the current play pos
	IDirectSoundBuffer_GetCurrentPosition(pContext->lpDSB, &dwPlayPos, NULL);
	
	// Lock DirectSound buffer
	IDirectSoundBuffer_Lock(pContext->lpDSB,
							0,
							CPC_OUTPUTBLOCKSIZE,
							&pbData,
							&dwLength,
							NULL,
							NULL,
							0);
	
	if (pbData)
		memcpy(pbData, pContext->m_pShadowBuffer, dwLength);
	
	// Apply EQ from play to either the end or the write cursor
	pEQModule = (CPs_EqualiserModule*)pModule->m_pEqualiser;
	
	if (pContext->m_WriteCursor > dwPlayPos)
	{
		// One block - from play pos to write cursor
		if (pContext->m_WriteCursor != CPC_INVALIDCURSORPOS)
			pEQModule->ApplyEQToBlock_Inplace(pEQModule, pbData + dwPlayPos, pContext->m_WriteCursor - dwPlayPos);
	}
	
	else
	{
		// Two blocks - from play pos to end
		// - and from start to write cursor
		pEQModule->ApplyEQToBlock_Inplace(pEQModule, pbData + dwPlayPos, CPC_OUTPUTBLOCKSIZE - dwPlayPos);
		pEQModule->ApplyEQToBlock_Inplace(pEQModule, pbData, pContext->m_WriteCursor);
	}
	
	// Unlock the buffer
	IDirectSoundBuffer_Unlock(pContext->lpDSB,
							  pbData,
							  dwLength,
							  NULL,
							  0L);
	
#endif
}

//
//
//
void CPP_OMDS_SetInternalVolume(CPs_OutputModule* pModule, const int iNewVolume)
{
	CPs_OutputContext_DirectSound* pContext = (CPs_OutputContext_DirectSound*)pModule->m_pModuleCookie;
	LONG lVolume;
	CP_CHECKOBJECT(pContext);
	
	lVolume = (int)(pow((double)(100 - iNewVolume) * 0.01, 3) * (double)DSBVOLUME_MIN);  //((100-iNewVolume) * DSBVOLUME_MIN) / 500;
	IDirectSoundBuffer_SetVolume(pContext->lpDSB, lVolume);
}

//
//
//

