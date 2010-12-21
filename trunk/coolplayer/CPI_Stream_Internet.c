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
#include "CPI_CircleBuffer.h"
#include "CPI_Player_Messages.h"



#define CIC_STREAMBUFFERSIZE  0x40000
#define CIC_PREBUFFERAMOUNT   0x8000
#define CIC_READCHUNKSIZE   0x1000

////////////////////////////////////////////////////////////////////////////////
//

typedef struct _CPs_BufferFillerContext
{
	char* m_pcFlexiURL;
	CPs_CircleBuffer* m_pCircleBuffer;
	BOOL m_bTerminate;
	HWND m_hWndNotify;
	
	
} CPs_BufferFillerContext;

//
//

typedef struct _CPs_InStream_Internet
{
	CPs_CircleBuffer* m_pCircleBuffer;
	HANDLE m_hFillerThread;
	CPs_BufferFillerContext* m_pBufferFillContext;
	
} CPs_InStream_Internet;

//
//
////////////////////////////////////////////////////////////////////////////////



void CPSINET_Uninitialise(CPs_InStream* pStream);
BOOL CPSINET_Read(CPs_InStream* pStream, void* pDestBuffer, const unsigned int iBytesToRead, unsigned int* piBytesRead);
void CPSINET_Seek(CPs_InStream* pStream, const unsigned int iNewOffset);
UINT CPSINET_Tell(CPs_InStream* pStream);
UINT CPSINET_GetLength(CPs_InStream* pStream);
BOOL CPSINET_IsSeekable(CPs_InStream* pStream);
////////////////////////////////////////////////////////////////////////////////
//
//
//
unsigned int _stdcall EP_FillerThread(void* _pContext)
{
	CPs_BufferFillerContext* pContext = (CPs_BufferFillerContext*)_pContext;
	HINTERNET hInternet;
	HINTERNET hURLStream;
	DWORD dwTimeout;
	BOOL bStreamComplete = FALSE;
	INTERNET_BUFFERS internetbuffer;
	BYTE bReadBuffer[CIC_READCHUNKSIZE];
	
	CP_CHECKOBJECT(pContext);
	
	PostMessage(pContext->m_hWndNotify, CPNM_SETSTREAMINGSTATE, (WPARAM)TRUE, (LPARAM)0);
	
	// Check that we can open this file
	hInternet = InternetOpen(CP_COOLPLAYER,
							 INTERNET_OPEN_TYPE_PRECONFIG,
							 NULL, NULL, 0L);
	                         
	if (hInternet == NULL)
	{
		pContext->m_pCircleBuffer->SetComplete(pContext->m_pCircleBuffer);
		CP_TRACE0("EP_FillerThread::NoInternetOpen");
		return 0;
	}
	
	dwTimeout = 2000;
	InternetSetOption(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &dwTimeout, sizeof(dwTimeout));
	
	hURLStream = InternetOpenUrl(hInternet,
								 pContext->m_pcFlexiURL,
								 NULL,
								 0,
								 INTERNET_FLAG_NO_CACHE_WRITE
								 | INTERNET_FLAG_PRAGMA_NOCACHE,
								 0);
	                             
	if (hURLStream == NULL)
	{
		InternetCloseHandle(hInternet);
		pContext->m_pCircleBuffer->SetComplete(pContext->m_pCircleBuffer);
		CP_TRACE1("EP_FillerThread::NoOpenURL %s", pContext->m_pcFlexiURL);
		return 0;
	}
	
	// Setup the internet buffer
	internetbuffer.dwStructSize = sizeof(internetbuffer);
	internetbuffer.Next = NULL;
	internetbuffer.lpcszHeader = NULL;
	internetbuffer.lpvBuffer = bReadBuffer;
	internetbuffer.dwBufferLength = CIC_READCHUNKSIZE;
	
	// Perform reading
	while (pContext->m_bTerminate == FALSE && bStreamComplete == FALSE)
	{
		BOOL bReadResult;
		
		// Is our circle buffer full?
		
		if (pContext->m_pCircleBuffer->GetFreeSize(pContext->m_pCircleBuffer) < CIC_READCHUNKSIZE)
		{
			Sleep(20);
			continue;
		}
		
		// Read in another chunk - if we don't get any data that's ok - we would rather poll the
		// buffers than just hang on the socket (so the stream can be shutdown if needed)
		internetbuffer.dwBufferLength = CIC_READCHUNKSIZE;
		bReadResult = InternetReadFileEx(hURLStream, &internetbuffer, IRF_NO_WAIT, 0);
		
		if (bReadResult == FALSE)
			bStreamComplete = TRUE;
			
		if (internetbuffer.dwBufferLength)
		{
			pContext->m_pCircleBuffer->Write(pContext->m_pCircleBuffer,
											 internetbuffer.lpvBuffer,
											 internetbuffer.dwBufferLength);
			                                 
			PostMessage(pContext->m_hWndNotify,
						CPNM_SETSTREAMINGSTATE,
						(WPARAM)TRUE,
						(LPARAM)(pContext->m_pCircleBuffer->GetUsedSize(pContext->m_pCircleBuffer)*100) / CIC_STREAMBUFFERSIZE);
		}
		
		else
			Sleep(20);
	}
	
	InternetCloseHandle(hURLStream);
	
	InternetCloseHandle(hInternet);
	
	pContext->m_pCircleBuffer->SetComplete(pContext->m_pCircleBuffer);
	PostMessage(pContext->m_hWndNotify, CPNM_SETSTREAMINGSTATE, (WPARAM)FALSE, (LPARAM)0);
	CP_TRACE0("EP_FillerThread normal shutdown");
	return 0;
}

//
//
//
CPs_InStream* CP_CreateInStream_Internet(const char* pcFlexiURL, HWND hWndOwner)
{
	CPs_InStream* pNewStream;
	CPs_InStream_Internet* pContext;
	unsigned int iUsedSpace;
	
	// Setup stream object
	{
		pNewStream = (CPs_InStream*)malloc(sizeof(CPs_InStream));
		pContext = (CPs_InStream_Internet*)malloc(sizeof(CPs_InStream_Internet));
		
		pNewStream->Uninitialise = CPSINET_Uninitialise;
		pNewStream->Read = CPSINET_Read;
		pNewStream->Seek = CPSINET_Seek;
		pNewStream->GetLength = CPSINET_GetLength;
		pNewStream->IsSeekable = CPSINET_IsSeekable;
		pNewStream->m_pModuleCookie = pContext;
		
		pContext->m_pCircleBuffer = CP_CreateCircleBuffer(CIC_STREAMBUFFERSIZE);
	}
	
	// Create thread to fill stream
	{
		CPs_BufferFillerContext* pBufferFillContext;
		UINT uiThreadID;
		
		// Setup context
		pBufferFillContext = (CPs_BufferFillerContext*)malloc(sizeof(CPs_BufferFillerContext));
		pBufferFillContext->m_pCircleBuffer = pContext->m_pCircleBuffer;
		pBufferFillContext->m_bTerminate = FALSE;
		STR_AllocSetString(&pBufferFillContext->m_pcFlexiURL, pcFlexiURL, FALSE);
		pBufferFillContext->m_hWndNotify = hWndOwner;
		
		// Start thread
		pContext->m_hFillerThread = (HANDLE)_beginthreadex(NULL, 0, EP_FillerThread, pBufferFillContext, 0, &uiThreadID);
		pContext->m_pBufferFillContext = pBufferFillContext;
	}
	
	// Pre buffer some data
	
	do
	{
		MSG msg;
		BOOL bMessageReceived;
		
		// Stream is never going to have more data in it
		
		if (pContext->m_pCircleBuffer->IsComplete(pContext->m_pCircleBuffer))
			break;
			
		Sleep(100);
		
		iUsedSpace = pContext->m_pCircleBuffer->GetUsedSize(pContext->m_pCircleBuffer);
		
		// Stop prebuffering if there is a stop in the queue for the engine
		bMessageReceived = PeekMessage(&msg, NULL, CPTM_STOP, CPTM_STOP, PM_NOREMOVE);
		
		if (bMessageReceived)
			break;
	}
	
	while (iUsedSpace < CIC_PREBUFFERAMOUNT);
	
	return pNewStream;
}

//
//
//
void CPSINET_Uninitialise(CPs_InStream* pStream)
{
	CPs_InStream_Internet* pContext = (CPs_InStream_Internet*)pStream->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	// Clear the thread
	pContext->m_pBufferFillContext->m_bTerminate = TRUE;
	WaitForSingleObject(pContext->m_hFillerThread, INFINITE);
	CloseHandle(pContext->m_hFillerThread);
	free(pContext->m_pBufferFillContext->m_pcFlexiURL);
	free(pContext->m_pBufferFillContext);
	
	// Free this context
	pContext->m_pCircleBuffer->Uninitialise(pContext->m_pCircleBuffer);
	free(pContext);
	free(pStream);
}

//
//
//
BOOL CPSINET_Read(CPs_InStream* pStream, void* pDestBuffer, const unsigned int iBytesToRead, unsigned int* piBytesRead)
{
	CPs_InStream_Internet* pContext = (CPs_InStream_Internet*)pStream->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	return pContext->m_pCircleBuffer->Read(pContext->m_pCircleBuffer, pDestBuffer, iBytesToRead, piBytesRead);
}

//
//
//
void CPSINET_Seek(CPs_InStream* pStream, const unsigned int iNewOffset)
{
#ifdef _DEBUG
	CPs_InStream_Internet* pContext = (CPs_InStream_Internet*)pStream->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
#endif
}

//
//
//
unsigned int CPSINET_GetLength(CPs_InStream* pStream)
{
#ifdef _DEBUG
	CPs_InStream_Internet* pContext = (CPs_InStream_Internet*)pStream->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
#endif
	return 0;
}

//
//
//
BOOL CPSINET_IsSeekable(CPs_InStream* pStream)
{
	return FALSE;
}

//
//
//
UINT CPSINET_Tell(CPs_InStream* pStream)
{
	return 0;
	
}



