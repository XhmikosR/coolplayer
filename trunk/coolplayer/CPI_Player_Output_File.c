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
#include "CPI_Playlist.h"
#include "CPI_PlaylistItem.h"
////////////////////////////////////////////////////////////////////////////////
//
// This is an output stage that uses the File output.
//
////////////////////////////////////////////////////////////////////////////////



// Total buffer size must quantise at 64Kb (because that what Windows gives us anyway!)
#define CPC_OUTPUTBLOCKSIZE   0x8000 // 32k blocks
////////////////////////////////////////////////////////////////////////////////
//

typedef struct __CPs_OutputContext_File
{
	FILE *m_hFile;
	CPs_EqualiserModule* m_pEqualiser;
	BOOL m_bPaused;
	
} CPs_OutputContext_File;

//
////////////////////////////////////////////////////////////////////////////////



void CPP_OMFL_Initialise(CPs_OutputModule* pModule, const CPs_FileInfo* pFileInfo, CP_HEQUALISER hEqualiser);
void CPP_OMFL_Uninitialise(CPs_OutputModule* pModule);
void CPP_OMFL_RefillBuffers(CPs_OutputModule* pModule);
void CPP_OMFL_SetPause(CPs_OutputModule* pModule, const BOOL bPause);
BOOL CPP_OMFL_IsOutputComplete(CPs_OutputModule* pModule);
void CPP_OMFL_Flush(CPs_OutputModule* pModule);
void CPP_OMFL_OnEQChanged(CPs_OutputModule* pModule);
void CPP_OMFL_SetInternalVolume(CPs_OutputModule* pModule, const int iNewVolume);
////////////////////////////////////////////////////////////////////////////////
//
//
//
void CPI_Player_Output_Initialise_File(CPs_OutputModule* pModule)
{
	// This is a one off call to set up the function pointers
	pModule->Initialise = CPP_OMFL_Initialise;
	pModule->Uninitialise = CPP_OMFL_Uninitialise;
	pModule->RefillBuffers = CPP_OMFL_RefillBuffers;
	pModule->SetPause = CPP_OMFL_SetPause;
	pModule->IsOutputComplete = CPP_OMFL_IsOutputComplete;
	pModule->Flush = CPP_OMFL_Flush;
	pModule->OnEQChanged = CPP_OMFL_OnEQChanged;
	pModule->SetInternalVolume = CPP_OMFL_SetInternalVolume;
	pModule->m_pModuleCookie = NULL;
	pModule->m_pcModuleName = "WAV File Writer";
	pModule->m_pCoDec = NULL;
	pModule->m_pEqualiser = NULL;
}

//
//
//
void CPP_OMFL_Initialise(CPs_OutputModule* pModule, const CPs_FileInfo* pFileInfo, CP_HEQUALISER hEqualiser)
{

	// This is called when some playing is required.
	// Do all allocation here so that we do not hold
	// resources while we are just sitting waiting for
	// something to happen.
	
	// Create a context
	CPs_OutputContext_File* pContext;
	CP_ASSERT(pModule->m_pModuleCookie == NULL);
	pContext = (CPs_OutputContext_File*)malloc(sizeof(CPs_OutputContext_File));
	pModule->m_pModuleCookie = pContext;
	CP_TRACE0("File out initialising");
	
	// Create sync object
	pModule->m_evtBlockFree = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	// Setup thread prioity to lowest
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
	pModule->m_pEqualiser = hEqualiser;
	pContext->m_bPaused = FALSE;
	pContext->m_hFile = NULL;
}

//
//
//
void CPP_OMFL_Uninitialise(CPs_OutputModule* pModule)
{
	CPs_OutputContext_File* pContext = (CPs_OutputContext_File*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	CP_TRACE0("Wave out shutting down");
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
	
	// If there is a File handle
	
	if (pContext->m_hFile)
	{
		// Stop any pending playing
		fclose(pContext->m_hFile);
		pContext->m_hFile = NULL;
		// Clean up
		CloseHandle(pModule->m_evtBlockFree);
	}
	
	free(pContext);
	
	pModule->m_pModuleCookie = NULL;
}

//
//
//
void CPP_OMFL_RefillBuffers(CPs_OutputModule* pModule)
{
	BOOL bMoreData;
	DWORD dwBufferLength = CPC_OUTPUTBLOCKSIZE;
	BYTE lpData[CPC_OUTPUTBLOCKSIZE];
	
	CPs_OutputContext_File* pContext = (CPs_OutputContext_File*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	
	// Open a file out
	
	if (!pContext->m_hFile)
	{
		CPs_FileInfo pFileInfo;
		CP_HPLAYLISTITEM hCurrent = CPL_GetActiveItem(globals.m_hPlaylist);
		const char *pathname = CPLI_GetPath(hCurrent);
		char newpath[MAX_PATH];
		int rec_rate;
		int rec_bits;
		UINT temp;
		WAVEFORMATEX wfs;
		char *dot;
		
		pModule->m_pCoDec->GetFileInfo(pModule->m_pCoDec, &pFileInfo);
		rec_rate = pFileInfo.m_iFreq_Hz;
		rec_bits = pFileInfo.m_b16bit == TRUE ? 16 : 8;
		
		// saving internet stream
		
		if (_strnicmp(pathname, CIC_HTTPHEADER, strlen(CIC_HTTPHEADER)) == 0)
		{
			strcpy(newpath, "Stream.wav");
			pFileInfo.m_iFileLength_Secs = 0xffffffff;
		}
		
		else
		{
			// replace the extension with .wav
			strcpy(newpath, pathname);
			dot = strrchr(newpath, '.');
			
			if (dot)
				*dot = '\0';
			
			strcat(newpath, ".wav");
		}
		
		// Trap error
		
		while (!pContext->m_hFile)
		{
			OPENFILENAME fn;
			char filefilter[] =
				"WAV files (*.wav)\0*.wav\0"
				"All Files (*.*)\0*.*\0";
			BOOL    returnval;
			fn.lStructSize = sizeof(OPENFILENAME);
			fn.hwndOwner = (HWND) GetWindowLong(windows.wnd_main, DWL_USER);
			fn.hInstance = NULL;
			fn.lpstrFilter = filefilter;
			fn.lpstrCustomFilter = NULL;
			fn.nMaxCustFilter = 0;
			fn.nFilterIndex = 0;
			fn.lpstrFile = newpath;
			fn.nMaxFile = MAX_PATH * 200;
			fn.lpstrFileTitle = NULL;
			fn.nMaxFileTitle = 0;
			fn.lpstrInitialDir = options.last_used_directory;
			fn.lpstrTitle = NULL;
			fn.Flags = OFN_ENABLESIZING |
					   OFN_HIDEREADONLY | OFN_EXPLORER;
			fn.nFileOffset = 0;
			fn.nFileExtension = 0;
			fn.lpstrDefExt = NULL;
			fn.lCustData = 0;
			fn.lpfnHook = NULL;
			fn.lpTemplateName = NULL;
			returnval = GetSaveFileName(&fn);
			
			if (!returnval)
				return;
			
			pContext->m_hFile = fopen(fn.lpstrFile, "wb");
			
			if (pContext->m_hFile)
				break;
		}
		
		// Wave header stuff
		
		// prep wave format header
		wfs.wFormatTag = WAVE_FORMAT_PCM;
		wfs.nChannels = pFileInfo.m_bStereo == TRUE ? 2 : 1;
		wfs.nSamplesPerSec = rec_rate;
		wfs.nBlockAlign = (short)(rec_bits / 8 * wfs.nChannels);
		wfs.nAvgBytesPerSec = rec_rate * wfs.nBlockAlign;
		wfs.wBitsPerSample = rec_bits;
		wfs.cbSize = 0;
		
		// RIFF header block
		fwrite("RIFF", 4, 1, pContext->m_hFile);
		temp =  sizeof(wfs) + 20 + (pFileInfo.m_iFileLength_Secs * wfs.nAvgBytesPerSec);
		fwrite(&temp, 4, 1, pContext->m_hFile);
		fwrite("WAVE", 4, 1, pContext->m_hFile);
		
		// 'fmt ' block
		fwrite("fmt ", 4, 1, pContext->m_hFile);
		temp = sizeof(wfs);
		fwrite(&temp, 4, 1, pContext->m_hFile);
		fwrite(&wfs, sizeof(wfs), 1, pContext->m_hFile);
		
		// 'data' block
		fwrite("data", 4, 1, pContext->m_hFile);
		temp = pFileInfo.m_iFileLength_Secs * wfs.nAvgBytesPerSec;
		fwrite(&temp, 4, 1, pContext->m_hFile);
	}
	
	
	
	// Scan ring buffer and fill any empty blocks - any blocks that become free
	// while this loop is running will retrigger the event - the worse that can
	// happen is that we enter this loop with no blocks free
	
	// Get block from CoDec and then just send it to the device (how easy is this!)
	bMoreData = pModule->m_pCoDec->GetPCMBlock(pModule->m_pCoDec, lpData, &dwBufferLength);
	
	
	// If there is EQ then apply it
	{
		// Note that the EQ module is initailised and uninitialsed by the engine
		CPs_EqualiserModule* pEQModule = (CPs_EqualiserModule*)pModule->m_pEqualiser;
		pEQModule->ApplyEQToBlock_Inplace(pEQModule, lpData, dwBufferLength);
	}
	
	if (dwBufferLength > 0)
		fwrite(lpData, dwBufferLength, 1, pContext->m_hFile);
		
	// Nothing to send
	if (bMoreData == FALSE)
	{
		pModule->m_pCoDec->CloseFile(pModule->m_pCoDec);
		pModule->m_pCoDec = NULL;
		
		if (pContext->m_hFile) 
			fclose(pContext->m_hFile);
		
		pContext->m_hFile = NULL;
	}
	
	if (!pContext->m_bPaused)
		SetEvent(pModule->m_evtBlockFree);
}

//
//
//
void CPP_OMFL_SetPause(CPs_OutputModule* pModule, const BOOL bPause)
{
	CPs_OutputContext_File* pContext = (CPs_OutputContext_File*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	if (!pContext->m_hFile)
		return;
		
	// Toggle pause state
	if (bPause == TRUE)
		pContext->m_bPaused = TRUE;
	else
	{
		pContext->m_bPaused = FALSE;
		SetEvent(pModule->m_evtBlockFree);
	}
}

//
//
//
BOOL CPP_OMFL_IsOutputComplete(CPs_OutputModule* pModule)
{
	//    int iBlockIDX;
	CPs_OutputContext_File* pContext = (CPs_OutputContext_File*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	if (!pContext->m_hFile)
		return TRUE;
		
	return TRUE;
}

//
//
//
void CPP_OMFL_OnEQChanged(CPs_OutputModule* pModule)
{
#ifdef _DEBUG
	CPs_OutputContext_File* pContext = (CPs_OutputContext_File*)pModule->m_pModuleCookie;
	
	CP_CHECKOBJECT(pContext);
#endif
}

//
//
//
void CPP_OMFL_Flush(CPs_OutputModule* pModule)
{
#ifdef _DEBUG
	CPs_OutputContext_File* pContext = (CPs_OutputContext_File*)pModule->m_pModuleCookie;
	
	CP_CHECKOBJECT(pContext);
	
	// Stop any pending playing
	
	CP_ASSERT(CPP_OMFL_IsOutputComplete(pModule));
#endif
}

//
//
//
void CPP_OMFL_SetInternalVolume(CPs_OutputModule* pModule, const int iNewVolume)
{
	CPs_OutputContext_File* pContext = (CPs_OutputContext_File*)pModule->m_pModuleCookie;
	int iNewVolume_DWORD;
	CP_CHECKOBJECT(pContext);
	
	if (!pContext->m_hFile)
		return;
		
	// Clip volume to word
	iNewVolume_DWORD = iNewVolume * 656;
	
	if (iNewVolume_DWORD > 0xFFFF)
		iNewVolume_DWORD = 0xFFFF;
		
	iNewVolume_DWORD |= (iNewVolume_DWORD << 16);
	
}

//
//
//
