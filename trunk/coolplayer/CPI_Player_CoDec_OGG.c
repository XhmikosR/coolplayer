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
#include "CPI_Stream.h"
#include "CPI_Player_CoDec.h"
#include "CPI_ID3.h"

#include "ogg/ogg.h"
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"


////////////////////////////////////////////////////////////////////////////////
//
// This is the CoDec module - the basic idea is that the file will be opened and
// data will be sucked through the CoDec via calls to CPI_CoDec__GetPCMBlock
//
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//

typedef struct __CPs_CoDec_Ogg
{
//    HANDLE m_pInStream;
	CPs_InStream* m_pInStream;
	
	CPs_FileInfo m_FileInfo;
	HINSTANCE oggInstance;
	
	OggVorbis_File vf;
	int current_section;
} CPs_CoDec_Ogg;

//
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
//
// Module functions
void CPP_OMOGG_Uninitialise(CPs_CoDecModule* pModule);
BOOL CPP_OMOGG_OpenFile(CPs_CoDecModule* pModule, const char* pcFilename, DWORD dwCookie, HWND hWndOwner);
void CPP_OMOGG_CloseFile(CPs_CoDecModule* pModule);
void CPP_OMOGG_Seek(CPs_CoDecModule* pModule, const int iNumerator, const int iDenominator);
void CPP_OMOGG_GetFileInfo(CPs_CoDecModule* pModule, CPs_FileInfo* pInfo);
//
BOOL CPP_OMOGG_GetPCMBlock(CPs_CoDecModule* pModule, void* pBlock, DWORD* pdwBlockSize);
int CPP_OMOGG_GetCurrentPos_secs(CPs_CoDecModule* pModule);
//
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
//
//
//
void CP_InitialiseCodec_OGG(CPs_CoDecModule* pCoDec)
{
	CPs_CoDec_Ogg *pContext;
	
	// Setup functions
	pCoDec->Uninitialise = CPP_OMOGG_Uninitialise;
	pCoDec->OpenFile = CPP_OMOGG_OpenFile;
	pCoDec->CloseFile = CPP_OMOGG_CloseFile;
	pCoDec->Seek = CPP_OMOGG_Seek;
	pCoDec->GetFileInfo = CPP_OMOGG_GetFileInfo;
	
	pCoDec->GetPCMBlock = CPP_OMOGG_GetPCMBlock;
	pCoDec->GetCurrentPos_secs = CPP_OMOGG_GetCurrentPos_secs;
	
	// Setup private data
	pCoDec->m_pModuleCookie = malloc(sizeof(CPs_CoDec_Ogg));
	pContext = (CPs_CoDec_Ogg*)pCoDec->m_pModuleCookie;
//    pContext->m_pInStream = NULL;
	pContext->m_pInStream = NULL;
	
	CPFA_InitialiseFileAssociations(pCoDec);
	CPFA_AddFileAssociation(pCoDec, "OGA", 0L);
}

//
//
//
void CPP_OMOGG_Uninitialise(CPs_CoDecModule* pModule)
{
	CPs_CoDec_Ogg *pContext = (CPs_CoDec_Ogg*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);

	CP_ASSERT(pContext->m_pInStream == NULL)
	free(pContext);
	CPFA_EmptyFileAssociations(pModule);
}

CPs_InStream* gInStream = NULL;

size_t CPP_OMOGG_cbRead(void *buffer, size_t size, size_t amount, void *file)
{
	size_t result = 0;
	gInStream->Read(gInStream, buffer, size*amount, &result);
	return result;
}

int CPP_OMOGG_cbSeek(void *file, ogg_int64_t newpos, int set)
{
	if (!gInStream->IsSeekable(gInStream)) return -1;
	
	if (set == SEEK_END)
		newpos = gInStream->GetLength(gInStream);
		
	gInStream->Seek(gInStream, (UINT)newpos);
	
	return (int)newpos;
}

int CPP_OMOGG_cbClose(void *file)
{
	gInStream->Uninitialise(gInStream);
	return 1;
	
}


long CPP_OMOGG_cbTell(void *file)
{
	return gInStream->Tell(gInStream);
}

//
//
//
BOOL CPP_OMOGG_OpenFile(CPs_CoDecModule* pModule, const char* pcFilename, DWORD dwCookie, HWND hWndOwner)
{
	vorbis_info *pInfo = NULL;
	CPs_CoDec_Ogg *pContext = (CPs_CoDec_Ogg*)pModule->m_pModuleCookie;
	
	CP_CHECKOBJECT(pContext);
	
	// If we have a stream open - close it
	
	if (pContext->m_pInStream != NULL)
	{
		CP_TRACE0("Already had a stream open, closing it");
		ov_clear(&pContext->vf);
		pContext->m_pInStream = NULL;
	}
	
	// Open our new stream
	CP_TRACE1("Openfile \"%s\"", pcFilename);
	
	pContext->m_pInStream = CP_CreateInStream(pcFilename, hWndOwner);
	
	if (!pContext->m_pInStream)
	{
		CP_TRACE0("CPI_CoDec__OpenFile: failed");
		return FALSE;
	}
	
	gInStream = pContext->m_pInStream;
	
	/*   pContext->m_pInStream = fopen(pcFilename, "rb");
	   if(pContext->m_pInStream == NULL)
	   {
	       CP_TRACE0("Failed to open file");
	       return FALSE; // Failure
	   }
	*/
	
	memset(&pContext->vf, 0, sizeof(pContext->vf));
	{
		ov_callbacks callbacks =
		{
			(size_t (*)(void *, size_t, size_t, void *)) CPP_OMOGG_cbRead,
			(int(*)(void *, ogg_int64_t, int))   CPP_OMOGG_cbSeek,
			(int(*)(void *))                  CPP_OMOGG_cbClose,
			(long(*)(void *))              CPP_OMOGG_cbTell
		};
		
		
		
		
		
		if (ov_open_callbacks((void *)(gInStream), &pContext->vf, NULL, 0, callbacks) < 0)
//    if(ov_open(pContext->m_pInStream, &pContext->vf, NULL, 0) < 0)
		{
			CP_TRACE0("Input does not appear to be an Ogg bitstream.");
			pContext->m_pInStream->Uninitialise(pContext->m_pInStream);
			pContext->m_pInStream = NULL;
			return FALSE;
		}
		
		if (ov_streams(&pContext->vf) != 1)
		{
			CP_TRACE1("Can\'t deal with multiple streams yet. Streams:%d", ov_streams(&pContext->vf));
			ov_clear(&pContext->vf);
			pContext->m_pInStream->Uninitialise(pContext->m_pInStream);
			pContext->m_pInStream = NULL;
			return FALSE;
		}
		
		pInfo = ov_info(&pContext->vf, -1);
		
		if (pInfo == NULL)
		{
			CP_TRACE0("Unable to get ogg info.");
			ov_clear(&pContext->vf);
			pContext->m_pInStream->Uninitialise(pContext->m_pInStream);
			pContext->m_pInStream = NULL;
			return FALSE;
		}
		
		if (pInfo->channels > 2)
		{
			CP_TRACE1("Can\'t deal with more than 2 channels yet. Channels:%d", pInfo->channels);
			ov_clear(&pContext->vf);
			pContext->m_pInStream->Uninitialise(pContext->m_pInStream);
			pContext->m_pInStream = NULL;
			return FALSE;
		}
		
		// Load m_FileInfo
		pContext->m_FileInfo.m_b16bit = TRUE; //?
		pContext->m_FileInfo.m_iBitRate_Kbs = ov_bitrate(&pContext->vf, -1) / 1000;
		pContext->m_FileInfo.m_iFileLength_Secs = (int)ov_time_total(&pContext->vf, -1);
		pContext->m_FileInfo.m_iFreq_Hz = pInfo->rate;
		pContext->m_FileInfo.m_bStereo = (pInfo->channels == 2);
	}
	
	return TRUE; // Success
}

//
//
//
void CPP_OMOGG_CloseFile(CPs_CoDecModule* pModule)
{
	CPs_CoDec_Ogg *pContext = (CPs_CoDec_Ogg*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	// If we have a stream open - close it
	
	if (pContext->m_pInStream != NULL)
	{
		ov_clear(&pContext->vf);
		pContext->m_pInStream = NULL;
	}
}

//
//
//
void CPP_OMOGG_Seek(CPs_CoDecModule* pModule, const int iNumerator, const int iDenominator)
{
	CPs_CoDec_Ogg *pContext = (CPs_CoDec_Ogg*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	// No error handling. If there is an error, there will be no seek
	ov_pcm_seek(&pContext->vf, (ogg_int64_t)(((float)iNumerator / (float)iDenominator) * (float)ov_pcm_total(&pContext->vf, -1)));
}

//
//
//
BOOL CPP_OMOGG_GetPCMBlock(CPs_CoDecModule* pModule, void* pBlock, DWORD* pdwBlockSize)
{
	int ret;
	BOOL CONTINUE = TRUE;
	DWORD bytes_read = *pdwBlockSize;
	CPs_CoDec_Ogg *pContext = (CPs_CoDec_Ogg*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	// Read PCM data from decoder. Little endian (0), 16-Bit (2), Signed (1)
	bytes_read = 0;
	
	while (bytes_read < *pdwBlockSize)
	{
		ret = ov_read(&pContext->vf, (unsigned char *)pBlock + bytes_read, *pdwBlockSize - bytes_read, 0, 2, 1, &pContext->current_section);
		
		if (ret == 0)
		{
			*pdwBlockSize = bytes_read;
			CONTINUE = FALSE;
			break;
		}
		
		else if (ret < 0)
		{
			CP_TRACE0("Hole in OGG/VORBIS datastream, ignoring...");
		}
		
		else
		{
			bytes_read += ret;
		}
	}
	
	return CONTINUE; //More data to come
}

//
//
//
void CPP_OMOGG_GetFileInfo(CPs_CoDecModule* pModule, CPs_FileInfo* pInfo)
{
	CPs_CoDec_Ogg *pContext = (CPs_CoDec_Ogg*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	memcpy(pInfo, &pContext->m_FileInfo, sizeof(*pInfo));
}

//
//
//
int CPP_OMOGG_GetCurrentPos_secs(CPs_CoDecModule* pModule)
{
	CPs_CoDec_Ogg *pContext = (CPs_CoDec_Ogg*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	return (int)ov_time_tell(&pContext->vf);;
}

//
//
//
