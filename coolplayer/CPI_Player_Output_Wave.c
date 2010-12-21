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

////////////////////////////////////////////////////////////////////////////////
//
// This is an output stage that uses the Windows Wave Mapper.  Real simple stuff
// here - just a circular buffer of wave blocks and an event controlling the
// lot.  This should be considered the "reference" output stage.
//
////////////////////////////////////////////////////////////////////////////////


// Total buffer size must quantise at 64Kb (because that what Windows gives us anyway!)
#define CPC_NUMBEROFOUTPUTBLOCKS 16
#define CPC_OUTPUTBLOCKSIZE   0x8000 // 32k blocks (512Kb buffer total)
////////////////////////////////////////////////////////////////////////////////
//

typedef struct __CPs_OutputContext_Wave
{
	HWAVEOUT m_hWaveOut;
	WAVEHDR m_aryWaveBlocks[CPC_NUMBEROFOUTPUTBLOCKS];
	DWORD m_aryBlockSizes[CPC_NUMBEROFOUTPUTBLOCKS];
	void* m_pBlockBase;
	int m_iLastReadBlockIDX;
	
	CPs_EqualiserModule* m_pEqualiser;
	
} CPs_OutputContext_Wave;

//
////////////////////////////////////////////////////////////////////////////////



void CPP_OMWV_Initialise(CPs_OutputModule* pModule, const CPs_FileInfo* pFileInfo, CP_HEQUALISER hEqualiser);
void CPP_OMWV_Uninitialise(CPs_OutputModule* pModule);
void CPP_OMWV_RefillBuffers(CPs_OutputModule* pModule);
void CPP_OMWV_SetPause(CPs_OutputModule* pModule, const BOOL bPause);
BOOL CPP_OMWV_IsOutputComplete(CPs_OutputModule* pModule);
void CPP_OMWV_Flush(CPs_OutputModule* pModule);
void CPP_OMWV_OnEQChanged(CPs_OutputModule* pModule);
void CPP_OMWV_SetInternalVolume(CPs_OutputModule* pModule, const int iNewVolume);
////////////////////////////////////////////////////////////////////////////////
//
//
//
void CPI_Player_Output_Initialise_WaveMapper(CPs_OutputModule* pModule)
{
	// This is a one off call to set up the function pointers
	pModule->Initialise = CPP_OMWV_Initialise;
	pModule->Uninitialise = CPP_OMWV_Uninitialise;
	pModule->RefillBuffers = CPP_OMWV_RefillBuffers;
	pModule->SetPause = CPP_OMWV_SetPause;
	pModule->IsOutputComplete = CPP_OMWV_IsOutputComplete;
	pModule->Flush = CPP_OMWV_Flush;
	pModule->OnEQChanged = CPP_OMWV_OnEQChanged;
	pModule->SetInternalVolume = CPP_OMWV_SetInternalVolume;
	pModule->m_pModuleCookie = NULL;
	pModule->m_pcModuleName = "Cooler Wave mapper";
	pModule->m_pCoDec = NULL;
	pModule->m_pEqualiser = NULL;
}

//
//
//
void CPP_OMWV_Initialise(CPs_OutputModule* pModule, const CPs_FileInfo* pFileInfo, CP_HEQUALISER hEqualiser)
{
	MMRESULT mmErr;
	
	// This is called when some playing is required.
	// Do all allocation here so that we do not hold
	// resources while we are just sitting waiting for
	// something to happen.
	
	// Create a context
	CPs_OutputContext_Wave* pContext;
	CP_ASSERT(pModule->m_pModuleCookie == NULL);
	pContext = (CPs_OutputContext_Wave*)malloc(sizeof(CPs_OutputContext_Wave));
	pModule->m_pModuleCookie = pContext;
	CP_TRACE0("Wave out initialising");
	
	// Create sync object
	pModule->m_evtBlockFree = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	// Open a wave out mapper
	{
		WAVEFORMATEX waveformatex;
		waveformatex.wFormatTag = WAVE_FORMAT_PCM;
		waveformatex.nChannels = pFileInfo->m_bStereo ? 2 : 1;
		waveformatex.nSamplesPerSec = pFileInfo->m_iFreq_Hz;
		waveformatex.wBitsPerSample = pFileInfo->m_b16bit ? 16 : 8;
		waveformatex.nBlockAlign = (waveformatex.nChannels * waveformatex.wBitsPerSample) >> 3;
		waveformatex.nAvgBytesPerSec = waveformatex.nSamplesPerSec * waveformatex.nBlockAlign;
		waveformatex.cbSize = 0;
		mmErr = waveOutOpen(&pContext->m_hWaveOut,
							WAVE_MAPPER,
							&waveformatex,
							(DWORD)pModule->m_evtBlockFree,
							0, CALLBACK_EVENT);
		                    
		// Trap error
		
		if (mmErr != MMSYSERR_NOERROR)
		{
			CP_TRACE1("Wave Open error 0x%X", mmErr);
			pContext->m_hWaveOut = NULL;
			CloseHandle(pModule->m_evtBlockFree);
		}
		
	}
	
	// We are going to make life easer for Windows here by ensuring
	// that each of our wave blocks lies on a memory page of it's own (and that
	// it fills that page) - These blocks will be locked for DMA by the sound driver.
	pContext->m_pBlockBase = VirtualAlloc(NULL,
										  CPC_OUTPUTBLOCKSIZE * (CPC_NUMBEROFOUTPUTBLOCKS << 1),
										  MEM_COMMIT, PAGE_READWRITE);
	pContext->m_iLastReadBlockIDX = 0;
	
	// Create wave blocks
	{
		void* pBlockCursor = pContext->m_pBlockBase;
		int iWaveBlockIDX;
		
		for (iWaveBlockIDX = 0; iWaveBlockIDX < CPC_NUMBEROFOUTPUTBLOCKS; iWaveBlockIDX++)
		{
			pContext->m_aryWaveBlocks[iWaveBlockIDX].dwFlags = 0;
			pContext->m_aryWaveBlocks[iWaveBlockIDX].lpData = pBlockCursor;
			pContext->m_aryWaveBlocks[iWaveBlockIDX].dwBufferLength = CPC_OUTPUTBLOCKSIZE;
			
			// Prepare output block (lock memory page)
			waveOutPrepareHeader(pContext->m_hWaveOut, pContext->m_aryWaveBlocks + iWaveBlockIDX, sizeof(*pContext->m_aryWaveBlocks));
			pBlockCursor = (void*)((BYTE*)pBlockCursor + CPC_OUTPUTBLOCKSIZE);
		}
	}
	
	// Setup thread prioity to max - If you are having problems during
	// debugging (with a big hang!) then comment this next line out
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	pModule->m_pEqualiser = hEqualiser;
}

//
//
//
void CPP_OMWV_Uninitialise(CPs_OutputModule* pModule)
{
	CPs_OutputContext_Wave* pContext = (CPs_OutputContext_Wave*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	CP_TRACE0("Wave out shutting down");
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
	
	// If there is a wave device
	
	if (pContext->m_hWaveOut)
	{
		// Stop any pending playing
		waveOutRestart(pContext->m_hWaveOut);
		waveOutReset(pContext->m_hWaveOut);
		
		// Clean up wave blocks
		{
			int iWaveBlockIDX;
			
			for (iWaveBlockIDX = 0; iWaveBlockIDX < CPC_NUMBEROFOUTPUTBLOCKS; iWaveBlockIDX++)
				waveOutUnprepareHeader(pContext->m_hWaveOut, pContext->m_aryWaveBlocks + iWaveBlockIDX, sizeof(*pContext->m_aryWaveBlocks));
		}
		
		// Clean up wave buffers
		VirtualFree(pContext->m_pBlockBase, 0, MEM_RELEASE);
		
		// Clean up
		waveOutClose(pContext->m_hWaveOut);

		CloseHandle(pModule->m_evtBlockFree);
	}
	
	free(pContext);
	
	pModule->m_pModuleCookie = NULL;
}

//
//
//
void CPP_OMWV_RefillBuffers(CPs_OutputModule* pModule)
{
	int iBlockIDX;
	
	CPs_OutputContext_Wave* pContext = (CPs_OutputContext_Wave*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	if (!pContext->m_hWaveOut)
		return;
		
	// Scan ring buffer and fill any empty blocks - any blocks that become free
	// while this loop is running will retrigger the event - the worse that can
	// happen is that we enter this loop with no blocks free
	for (iBlockIDX = 0; iBlockIDX < CPC_NUMBEROFOUTPUTBLOCKS; iBlockIDX++)
	{
		WAVEHDR* pOutputBlock = pContext->m_aryWaveBlocks + iBlockIDX;
		
		if ((pOutputBlock->dwFlags & WHDR_INQUEUE) == 0)
		{
			BOOL bMoreData;
			
			// Get block from CoDec and then just send it to the device (how easy is this!)
			pOutputBlock->dwBufferLength = CPC_OUTPUTBLOCKSIZE;
			bMoreData = pModule->m_pCoDec->GetPCMBlock(pModule->m_pCoDec, pOutputBlock->lpData, &pOutputBlock->dwBufferLength);
			
			// Store "pure" block data
			pContext->m_iLastReadBlockIDX = iBlockIDX;
			pContext->m_aryBlockSizes[iBlockIDX] = pOutputBlock->dwBufferLength;
			memcpy(((BYTE*)pOutputBlock->lpData) + (CPC_OUTPUTBLOCKSIZE * CPC_NUMBEROFOUTPUTBLOCKS),
				   pOutputBlock->lpData,
				   pOutputBlock->dwBufferLength);
			       
			// If there is EQ then apply it
			{
				// Note that the EQ module is initailised and uninitialsed by the engine
				CPs_EqualiserModule* pEQModule = (CPs_EqualiserModule*)pModule->m_pEqualiser;
				pEQModule->ApplyEQToBlock_Inplace(pEQModule, pOutputBlock->lpData, pOutputBlock->dwBufferLength);
			}
			
			if (pOutputBlock->dwBufferLength > 0)
				waveOutWrite(pContext->m_hWaveOut, pOutputBlock, sizeof(*pOutputBlock));
				
			// Nothing to send
			if (bMoreData == FALSE)
			{
				pModule->m_pCoDec->CloseFile(pModule->m_pCoDec);
				pModule->m_pCoDec = NULL;
				break;
			}
			
		}
	}
}

//
//
//
void CPP_OMWV_SetPause(CPs_OutputModule* pModule, const BOOL bPause)
{
	CPs_OutputContext_Wave* pContext = (CPs_OutputContext_Wave*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	if (!pContext->m_hWaveOut)
		return;
		
	// Toggle pause state
	if (bPause == TRUE)
		waveOutPause(pContext->m_hWaveOut);
	else
		waveOutRestart(pContext->m_hWaveOut);
}

//
//
//
BOOL CPP_OMWV_IsOutputComplete(CPs_OutputModule* pModule)
{
	int iBlockIDX;
	CPs_OutputContext_Wave* pContext = (CPs_OutputContext_Wave*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	if (!pContext->m_hWaveOut)
		return TRUE;
		
	for (iBlockIDX = 0; iBlockIDX < CPC_NUMBEROFOUTPUTBLOCKS; iBlockIDX++)
	{
		if ((pContext->m_aryWaveBlocks[iBlockIDX].dwFlags & WHDR_INQUEUE))
			return FALSE;
	}
	
	return TRUE;
}

//
//
//
void CPP_OMWV_OnEQChanged(CPs_OutputModule* pModule)
{
	CPs_OutputContext_Wave* pContext = (CPs_OutputContext_Wave*)pModule->m_pModuleCookie;
	int iBlockIDX;
	
	CP_CHECKOBJECT(pContext);
	
	if (!pContext->m_hWaveOut)
		return;
		
	// Start from the block after the last read block - copy the "pure" data back and reapply the EQ
	for (iBlockIDX = pContext->m_iLastReadBlockIDX + 1; iBlockIDX < CPC_NUMBEROFOUTPUTBLOCKS; iBlockIDX++)
	{
		if ((pContext->m_aryWaveBlocks[iBlockIDX].dwFlags & (WHDR_INQUEUE | WHDR_DONE)))
		{
			WAVEHDR* pOutputBlock = pContext->m_aryWaveBlocks + iBlockIDX;
			
			// Copy the "pure" data back
			pOutputBlock->dwBufferLength = pContext->m_aryBlockSizes[iBlockIDX];
			memcpy(pOutputBlock->lpData,
				   ((BYTE*)pOutputBlock->lpData) + (CPC_OUTPUTBLOCKSIZE * CPC_NUMBEROFOUTPUTBLOCKS),
				   pOutputBlock->dwBufferLength);
			       
			// If there is EQ then apply it
			{
				// Note that the EQ module is initailised and uninitialsed by the engine
				CPs_EqualiserModule* pEQModule = (CPs_EqualiserModule*)pModule->m_pEqualiser;
				pEQModule->ApplyEQToBlock_Inplace(pEQModule, pOutputBlock->lpData, pOutputBlock->dwBufferLength);
			}
		}
	}
	
	for (iBlockIDX = 0; iBlockIDX <= pContext->m_iLastReadBlockIDX; iBlockIDX++)
	{
		if ((pContext->m_aryWaveBlocks[iBlockIDX].dwFlags & (WHDR_INQUEUE | WHDR_DONE)))
		{
			WAVEHDR* pOutputBlock = pContext->m_aryWaveBlocks + iBlockIDX;
			
			// Copy the "pure" data back
			pOutputBlock->dwBufferLength = pContext->m_aryBlockSizes[iBlockIDX];
			memcpy(pOutputBlock->lpData,
				   ((BYTE*)pOutputBlock->lpData) + (CPC_OUTPUTBLOCKSIZE * CPC_NUMBEROFOUTPUTBLOCKS),
				   pOutputBlock->dwBufferLength);
			       
			// If there is EQ then apply it
			{
				// Note that the EQ module is initailised and uninitialsed by the engine
				CPs_EqualiserModule* pEQModule = (CPs_EqualiserModule*)pModule->m_pEqualiser;
				pEQModule->ApplyEQToBlock_Inplace(pEQModule, pOutputBlock->lpData, pOutputBlock->dwBufferLength);
			}
		}
	}
}

//
//
//
void CPP_OMWV_Flush(CPs_OutputModule* pModule)
{
	CPs_OutputContext_Wave* pContext = (CPs_OutputContext_Wave*)pModule->m_pModuleCookie;
	
	CP_CHECKOBJECT(pContext);
	
	if (!pContext->m_hWaveOut)
		return;
		
	// Stop any pending playing
	waveOutRestart(pContext->m_hWaveOut);
	
	waveOutReset(pContext->m_hWaveOut);
	
	CP_ASSERT(CPP_OMWV_IsOutputComplete(pModule));
}

//
//
//
void CPP_OMWV_SetInternalVolume(CPs_OutputModule* pModule, const int iNewVolume)
{
	CPs_OutputContext_Wave* pContext = (CPs_OutputContext_Wave*)pModule->m_pModuleCookie;
	int iNewVolume_DWORD;
	CP_CHECKOBJECT(pContext);
	
	if (!pContext->m_hWaveOut)
		return;
		
	// Clip volume to word
	iNewVolume_DWORD = iNewVolume * 656;
	
	if (iNewVolume_DWORD > 0xFFFF)
		iNewVolume_DWORD = 0xFFFF;
		
	iNewVolume_DWORD |= (iNewVolume_DWORD << 16);
	
	waveOutSetVolume(pContext->m_hWaveOut, iNewVolume_DWORD);
}

//
//
//
