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
#include "CPI_Player_CoDec.h"
#include "CP_RIFFStructs.h"
#include "CPI_ID3.h"

////////////////////////////////////////////////////////////////////////////////
//
// This is the CoDec module - the basic idea is that the file will be opened and
// data will be sucked through the CoDec via calls to CPI_CoDec__GetPCMBlock
//
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
//
// Module functions
void CPP_OMWAV_Uninitialise(CPs_CoDecModule* pModule);
BOOL CPP_OMWAV_OpenFile(CPs_CoDecModule* pModule, const char* pcFilename, DWORD dwCookie, HWND hWndOwner);
void CPP_OMWAV_CloseFile(CPs_CoDecModule* pModule);
void CPP_OMWAV_Seek(CPs_CoDecModule* pModule, const int iNumerator, const int iDenominator);
void CPP_OMWAV_GetFileInfo(CPs_CoDecModule* pModule, CPs_FileInfo* pInfo);
//
BOOL CPP_OMWAV_GetPCMBlock(CPs_CoDecModule* pModule, void* pBlock, DWORD* pdwBlockSize);
int CPP_OMWAV_GetCurrentPos_secs(CPs_CoDecModule* pModule);
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//

typedef struct __CPs_CoDec_Wave
{
	HANDLE m_hFile;
	unsigned int m_iStartOfWavData;
	unsigned int m_iLengthOfWavData;
	
	// Format
	CPs_FileInfo m_FileInfo;
	int m_iBytesPerSecond;
	
	// Offset
	int m_iCurrentOffset_Secs;
	int m_iCurrentOffset_Fraction_Bytes;
} CPs_CoDec_Wave;

//
////////////////////////////////////////////////////////////////////////////////


BOOL SkipToChunk(HANDLE hFile, CPs_RIFFChunk* pChunk, const char cChunkID[4]);
////////////////////////////////////////////////////////////////////////////////
//
//
//
void CP_InitialiseCodec_WAV(CPs_CoDecModule* pCoDec)
{
	CPs_CoDec_Wave *pContext;
	
	// Setup functions
	pCoDec->Uninitialise = CPP_OMWAV_Uninitialise;
	pCoDec->OpenFile = CPP_OMWAV_OpenFile;
	pCoDec->CloseFile = CPP_OMWAV_CloseFile;
	pCoDec->Seek = CPP_OMWAV_Seek;
	pCoDec->GetFileInfo = CPP_OMWAV_GetFileInfo;
	
	pCoDec->GetPCMBlock = CPP_OMWAV_GetPCMBlock;
	pCoDec->GetCurrentPos_secs = CPP_OMWAV_GetCurrentPos_secs;
	
	// Setup private data
	pCoDec->m_pModuleCookie = malloc(sizeof(CPs_CoDec_Wave));
	pContext = (CPs_CoDec_Wave*)pCoDec->m_pModuleCookie;
	pContext->m_hFile = INVALID_HANDLE_VALUE;
	
	CPFA_InitialiseFileAssociations(pCoDec);
	CPFA_AddFileAssociation(pCoDec, "WAV", 0L);
}

//
//
//
void CPP_OMWAV_Uninitialise(CPs_CoDecModule* pModule)
{
	CPs_CoDec_Wave *pContext = (CPs_CoDec_Wave*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	CP_ASSERT(pContext->m_hFile == INVALID_HANDLE_VALUE)
	
	free(pContext);
	CPFA_EmptyFileAssociations(pModule);
}

//
//
//
BOOL CPP_OMWAV_OpenFile(CPs_CoDecModule* pModule, const char* pcFilename, DWORD dwCookie, HWND hWndOwner)
{
	CPs_CoDec_Wave *pContext = (CPs_CoDec_Wave*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	// If we have a stream open - close it
	
	if (pContext->m_hFile)
	{
		CloseHandle(pContext->m_hFile);
		pContext->m_hFile = INVALID_HANDLE_VALUE;
	}
	
	// Open our new stream
	CP_TRACE1("Openfile \"%s\"", pcFilename);
	
	pContext->m_hFile = CreateFile(pcFilename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
	
	if (pContext->m_hFile == INVALID_HANDLE_VALUE)
	{
		CP_TRACE0("Failed to open file");
		return FALSE; // Failure
	}
	
	// Skip over ID3v2 tag (if there is one)
	{
		CIs_ID3v2Tag tag;
		DWORD dwBytesRead;
		int iStreamStart = 0;
		
		memset(&tag, 0, sizeof(tag));
		ReadFile(pContext->m_hFile, &tag, sizeof(tag), &dwBytesRead, NULL);
		
		if (memcmp(tag.m_cTAG, "ID3", 3) == 0)
		{
			iStreamStart = sizeof(CIs_ID3v2Tag);
			iStreamStart += (tag.m_cSize_Encoded[0] << 21)
							| (tag.m_cSize_Encoded[1] << 14)
							| (tag.m_cSize_Encoded[2] << 7)
							| tag.m_cSize_Encoded[3];
		}
		
		SetFilePointer(pContext->m_hFile, iStreamStart, NULL, FILE_BEGIN);
	}
	
	// Check the header
	{
		CPs_RIFFHeader RIFFHeader;
		DWORD dwBytesRead;
		ReadFile(pContext->m_hFile, &RIFFHeader, sizeof(RIFFHeader), &dwBytesRead, NULL);
		
		if (memcmp(RIFFHeader.m_cID, "RIFF", 4) || memcmp(RIFFHeader.m_cFileType, "WAVE", 4))
		{
			CP_TRACE0("File not of RIFF WAVE type");
			CloseHandle(pContext->m_hFile);
			pContext->m_hFile = INVALID_HANDLE_VALUE;
			return FALSE; // Failure
		}
	}
	
	// Check the format of the WAV file
	{
		CPs_RIFFChunk chunk;
		PCMWAVEFORMAT* pFormat;
		DWORD dwBytesRead;
		BOOL bSuccess = SkipToChunk(pContext->m_hFile, &chunk, "fmt ");
		
		if (bSuccess == FALSE || chunk.m_dwLength < sizeof(PCMWAVEFORMAT))
		{
			CP_TRACE0("Failed to find FMT chunk");
			CloseHandle(pContext->m_hFile);
			pContext->m_hFile = INVALID_HANDLE_VALUE;
			return FALSE; // Failure
		}
		
		// Get the format data
		pFormat = (PCMWAVEFORMAT*)malloc(chunk.m_dwLength);
		
		ReadFile(pContext->m_hFile, pFormat, chunk.m_dwLength, &dwBytesRead, NULL);
		
		// We only handle PCM encoded data
		if (dwBytesRead != chunk.m_dwLength || pFormat->wf.wFormatTag != WAVE_FORMAT_PCM)
		{
			CP_TRACE0("Only PCM data supported!");
			free(pFormat);
			CloseHandle(pContext->m_hFile);
			pContext->m_hFile = INVALID_HANDLE_VALUE;
			return FALSE; // Failure
		}
		
		// Setup file info struct - this is re-read every second
		// if m_iBitRate_Kbs = 0, bitrate display is suppressed
		pContext->m_FileInfo.m_iBitRate_Kbs = 
				(pFormat->wBitsPerSample * pFormat->wf.nChannels * pFormat->wf.nSamplesPerSec) / 1000;
		

		pContext->m_FileInfo.m_iFreq_Hz = pFormat->wf.nSamplesPerSec;
		pContext->m_FileInfo.m_bStereo = pFormat->wf.nChannels == 2 ? TRUE : FALSE;
		pContext->m_FileInfo.m_b16bit = pFormat->wBitsPerSample == 16 ? TRUE : FALSE;
		pContext->m_iBytesPerSecond = pContext->m_FileInfo.m_iFreq_Hz
									  * (pContext->m_FileInfo.m_bStereo == TRUE ? 2 : 1)
									  * (pContext->m_FileInfo.m_b16bit == TRUE ? 2 : 1);
		                              
		free(pFormat);
	}
	
	// Dip into the DATA chunk
	{
		CPs_RIFFChunk chunk;
		BOOL bSuccess = SkipToChunk(pContext->m_hFile, &chunk, "data");
		
		if (bSuccess == FALSE)
		{
			CP_TRACE0("Failed to find WAVE chunk");
			CloseHandle(pContext->m_hFile);
			pContext->m_hFile = INVALID_HANDLE_VALUE;
			return FALSE; // Failure
		}
		
		// Get info about the length of this chunk
		pContext->m_iLengthOfWavData = chunk.m_dwLength;
		pContext->m_iStartOfWavData = SetFilePointer(pContext->m_hFile, 0, NULL, FILE_CURRENT);
	}
	
	pContext->m_FileInfo.m_iFileLength_Secs = pContext->m_iLengthOfWavData / pContext->m_iBytesPerSecond;
	pContext->m_iCurrentOffset_Secs = 0;
	pContext->m_iCurrentOffset_Fraction_Bytes = 0;
	return TRUE; // Success
}

//
//
//
void CPP_OMWAV_CloseFile(CPs_CoDecModule* pModule)
{
	CPs_CoDec_Wave *pContext = (CPs_CoDec_Wave*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	// If we have a stream open - close it
	
	if (pContext->m_hFile != INVALID_HANDLE_VALUE)
	{
		CP_TRACE0("Close WAV file");
		CloseHandle(pContext->m_hFile);
		pContext->m_hFile = INVALID_HANDLE_VALUE;
	}
}

//
//
//
BOOL SkipToChunk(HANDLE hFile, CPs_RIFFChunk* pChunk, const char cChunkID[4])
{
	DWORD dwBytesRead;
	
	while (ReadFile(hFile, pChunk, sizeof(*pChunk), &dwBytesRead, NULL)
			&& dwBytesRead == sizeof(*pChunk))
	{
		if (memcmp(pChunk->m_cID, cChunkID, 4) == 0)
			return TRUE;
			
		SetFilePointer(hFile, pChunk->m_dwLength, NULL, FILE_CURRENT);
	}
	
	return FALSE;
}

//
//
//
void CPP_OMWAV_Seek(CPs_CoDecModule* pModule, const int iNumerator, const int iDenominator)
{
	unsigned int iSeekPos;
	div_t progress;
	CPs_CoDec_Wave *pContext = (CPs_CoDec_Wave*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	// Real quick and dirty - but good enough (we are not a sound editing
	// suite after all!)
	iSeekPos = (int)(((float)iNumerator / (float)iDenominator) * (float)pContext->m_iLengthOfWavData);
	
	// Round seek pos to nearest sample
	iSeekPos &= ~0x3;
	
	// Setup our progress
	progress = div(iSeekPos, pContext->m_iBytesPerSecond);
	pContext->m_iCurrentOffset_Secs = progress.quot;
	pContext->m_iCurrentOffset_Fraction_Bytes = progress.rem;
	
	// Skip to sample
	SetFilePointer(pContext->m_hFile, pContext->m_iStartOfWavData + iSeekPos, NULL, FILE_BEGIN);
}

//
//
//
BOOL CPP_OMWAV_GetPCMBlock(CPs_CoDecModule* pModule, void* pBlock, DWORD* pdwBlockSize)
{
	BOOL bSuccess;
	CPs_CoDec_Wave *pContext = (CPs_CoDec_Wave*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	CP_ASSERT(pContext->m_hFile != INVALID_HANDLE_VALUE);
	
	// Read
	bSuccess = ReadFile(pContext->m_hFile, pBlock, *pdwBlockSize, pdwBlockSize, NULL);
	
	if (bSuccess == FALSE || *pdwBlockSize == 0)
		return FALSE;
		
	// Setup progress
	pContext->m_iCurrentOffset_Fraction_Bytes += *pdwBlockSize;
	
	if (pContext->m_iCurrentOffset_Fraction_Bytes > pContext->m_iBytesPerSecond)
	{
		pContext->m_iCurrentOffset_Secs++;
		pContext->m_iCurrentOffset_Fraction_Bytes -= pContext->m_iBytesPerSecond;
	}
	
	return TRUE;
}

//
//
//
void CPP_OMWAV_GetFileInfo(CPs_CoDecModule* pModule, CPs_FileInfo* pInfo)
{
	CPs_CoDec_Wave *pContext = (CPs_CoDec_Wave*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	memcpy(pInfo, &pContext->m_FileInfo, sizeof(*pInfo));
}

//
//
//
int CPP_OMWAV_GetCurrentPos_secs(CPs_CoDecModule* pModule)
{
	CPs_CoDec_Wave *pContext = (CPs_CoDec_Wave*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	return pContext->m_iCurrentOffset_Secs;
}

//
//
//
