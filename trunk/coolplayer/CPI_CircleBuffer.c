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
#include "CPI_CircleBuffer.h"


#define CIC_WAITTIMEOUT  3000
void CircleBufferUninitialise(CPs_CircleBuffer* pCBuffer);
void CircleBufferWrite(CPs_CircleBuffer* pCBuffer, const void* pSourceBuffer, const unsigned int iNumBytes);
BOOL CircleBufferRead(CPs_CircleBuffer* pCBuffer, void* pDestBuffer, const unsigned int iBytesToRead, unsigned int* pbBytesRead);
void CircleFlush(CPs_CircleBuffer* pCBuffer);
unsigned int CircleGetFreeSpace(CPs_CircleBuffer* pCBuffer);
unsigned int CircleGetUsedSpace(CPs_CircleBuffer* pCBuffer);
void CircleSetComplete(CPs_CircleBuffer* pCBuffer);
BOOL CircleIsComplete(CPs_CircleBuffer* pCBuffer);
////////////////////////////////////////////////////////////////////////////////
//
//
//
CPs_CircleBuffer* CP_CreateCircleBuffer(const unsigned int iBufferSize)
{
	CPs_CircleBuffer* pNewBuffer = (CPs_CircleBuffer*)malloc(sizeof(CPs_CircleBuffer));
	
	pNewBuffer->Uninitialise = CircleBufferUninitialise;
	pNewBuffer->Write = CircleBufferWrite;
	pNewBuffer->Read = CircleBufferRead;
	pNewBuffer->Flush = CircleFlush;
	pNewBuffer->GetUsedSize = CircleGetUsedSpace;
	pNewBuffer->GetFreeSize = CircleGetFreeSpace;
	pNewBuffer->SetComplete = CircleSetComplete;
	pNewBuffer->IsComplete = CircleIsComplete;
	
	pNewBuffer->m_iBufferSize = iBufferSize;
	pNewBuffer->m_pBuffer = (BYTE*)malloc(iBufferSize);
	pNewBuffer->m_iReadCursor = 0;
	pNewBuffer->m_iWriteCursor = 0;
	pNewBuffer->m_bComplete = FALSE;
	pNewBuffer->m_evtDataAvailable = CreateEvent(NULL, FALSE, FALSE, NULL);
	InitializeCriticalSection(&pNewBuffer->m_csCircleBuffer);
	
	return pNewBuffer;
}

//
//
//
void CircleBufferUninitialise(CPs_CircleBuffer* pCBuffer)
{
	CP_CHECKOBJECT(pCBuffer);
	DeleteCriticalSection(&pCBuffer->m_csCircleBuffer);
	CloseHandle(pCBuffer->m_evtDataAvailable);
	free(pCBuffer->m_pBuffer);
	free(pCBuffer);
}

//
//
//
void CircleBufferWrite(CPs_CircleBuffer* pCBuffer, const void* _pSourceBuffer, const unsigned int _iNumBytes)
{
	unsigned int iBytesToWrite = _iNumBytes;
	BYTE* pReadCursor = (BYTE*)_pSourceBuffer;
	
	CP_ASSERT(iBytesToWrite <= pCBuffer->GetFreeSize(pCBuffer));
	CP_ASSERT(pCBuffer->m_bComplete == FALSE);
	
	EnterCriticalSection(&pCBuffer->m_csCircleBuffer);
	
	// We *know* there is enough space in the buffer for this
	// entire stream - write all the data until the end of the block
	
	if (pCBuffer->m_iWriteCursor >= pCBuffer->m_iReadCursor)
	{
		// Determine how much data we can fit into the end part of the CBuffer
		unsigned int iChunkSize = pCBuffer->m_iBufferSize - pCBuffer->m_iWriteCursor;
		
		if (iChunkSize > iBytesToWrite)
			iChunkSize = iBytesToWrite;
			
		// Copy the data
		memcpy(pCBuffer->m_pBuffer + pCBuffer->m_iWriteCursor,
			   pReadCursor, iChunkSize);
		       
		pReadCursor += iChunkSize;
		
		iBytesToWrite -= iChunkSize;
		
		// Update write cursor (wrapping if needed)
		pCBuffer->m_iWriteCursor += iChunkSize;
		
		if (pCBuffer->m_iWriteCursor >= pCBuffer->m_iBufferSize)
			pCBuffer->m_iWriteCursor -= pCBuffer->m_iBufferSize;
	}
	
	// Fill the start part of the CBuffer with any data that may be left
	
	if (iBytesToWrite)
	{
		memcpy(pCBuffer->m_pBuffer + pCBuffer->m_iWriteCursor,
			   pReadCursor, iBytesToWrite);
		pCBuffer->m_iWriteCursor += iBytesToWrite;
		CP_ASSERT(pCBuffer->m_iWriteCursor < pCBuffer->m_iBufferSize);
	}
	
	SetEvent(pCBuffer->m_evtDataAvailable);
	
	LeaveCriticalSection(&pCBuffer->m_csCircleBuffer);
}

//
//
//
BOOL CircleBufferRead(CPs_CircleBuffer* pCBuffer, void* pDestBuffer, const unsigned int _iBytesToRead, unsigned int* pbBytesRead)
{
	unsigned int iBytesToRead = _iBytesToRead;
	unsigned int iBytesRead = 0;
	DWORD dwWaitResult;
	BOOL bComplete = FALSE;
	CP_CHECKOBJECT(pCBuffer);
	
	while (iBytesToRead > 0 && bComplete == FALSE)
	{
		dwWaitResult = WaitForSingleObject(pCBuffer->m_evtDataAvailable, CIC_WAITTIMEOUT);
		
		if (dwWaitResult == WAIT_TIMEOUT)
		{
			CP_TRACE0("Circle buffer - did not fill in time!");
			*pbBytesRead = iBytesRead;
			return FALSE;
		}
		
		EnterCriticalSection(&pCBuffer->m_csCircleBuffer);
		
		// Take what we can from the CBuffer
		
		if (pCBuffer->m_iReadCursor > pCBuffer->m_iWriteCursor)
		{
			unsigned int iChunkSize = pCBuffer->m_iBufferSize - pCBuffer->m_iReadCursor;
			
			if (iChunkSize > iBytesToRead)
				iChunkSize = iBytesToRead;
				
			// Perform the read
			memcpy((BYTE*)pDestBuffer + iBytesRead,
				   pCBuffer->m_pBuffer + pCBuffer->m_iReadCursor,
				   iChunkSize);
			       
			iBytesRead += iChunkSize;
			iBytesToRead -= iChunkSize;
			
			pCBuffer->m_iReadCursor += iChunkSize;
			
			if (pCBuffer->m_iReadCursor >= pCBuffer->m_iBufferSize)
				pCBuffer->m_iReadCursor -= pCBuffer->m_iBufferSize;
		}
		
		if (iBytesToRead && pCBuffer->m_iReadCursor < pCBuffer->m_iWriteCursor)
		{
			unsigned int iChunkSize = pCBuffer->m_iWriteCursor - pCBuffer->m_iReadCursor;
			
			if (iChunkSize > iBytesToRead)
				iChunkSize = iBytesToRead;
				
			// Perform the read
			memcpy((BYTE*)pDestBuffer + iBytesRead,
				   pCBuffer->m_pBuffer + pCBuffer->m_iReadCursor,
				   iChunkSize);
			       
			iBytesRead += iChunkSize;
			iBytesToRead -= iChunkSize;
			pCBuffer->m_iReadCursor += iChunkSize;
		}
		
		// Is there any more data to read
		
		if (pCBuffer->m_iReadCursor == pCBuffer->m_iWriteCursor)
		{
			if (pCBuffer->m_bComplete)
				bComplete = TRUE;
		}
		
		else
			SetEvent(pCBuffer->m_evtDataAvailable);
			
		LeaveCriticalSection(&pCBuffer->m_csCircleBuffer);
	}
	
	*pbBytesRead = iBytesRead;
	
	return bComplete ? FALSE : TRUE;
}

//
//
//
void CircleFlush(CPs_CircleBuffer* pCBuffer)
{
	CP_CHECKOBJECT(pCBuffer);
	
	EnterCriticalSection(&pCBuffer->m_csCircleBuffer);
	pCBuffer->m_iReadCursor = 0;
	pCBuffer->m_iWriteCursor = 0;
	LeaveCriticalSection(&pCBuffer->m_csCircleBuffer);
}

//
//
//
unsigned int CircleGetFreeSpace(CPs_CircleBuffer* pCBuffer)
{
	unsigned int iNumBytesFree;
	
	CP_CHECKOBJECT(pCBuffer);
	EnterCriticalSection(&pCBuffer->m_csCircleBuffer);
	
	if (pCBuffer->m_iWriteCursor < pCBuffer->m_iReadCursor)
		iNumBytesFree = (pCBuffer->m_iReadCursor - 1) - pCBuffer->m_iWriteCursor;
	else if (pCBuffer->m_iWriteCursor == pCBuffer->m_iReadCursor)
		iNumBytesFree = pCBuffer->m_iBufferSize;
	else
		iNumBytesFree = (pCBuffer->m_iReadCursor - 1) + (pCBuffer->m_iBufferSize - pCBuffer->m_iWriteCursor);
		
	LeaveCriticalSection(&pCBuffer->m_csCircleBuffer);
	
	return iNumBytesFree;
}

//
//
//
unsigned int CircleGetUsedSpace(CPs_CircleBuffer* pCBuffer)
{
	return pCBuffer->m_iBufferSize - CircleGetFreeSpace(pCBuffer);
}

//
//
//
void CircleSetComplete(CPs_CircleBuffer* pCBuffer)
{
	CP_CHECKOBJECT(pCBuffer);
	
	EnterCriticalSection(&pCBuffer->m_csCircleBuffer);
	pCBuffer->m_bComplete = TRUE;
	SetEvent(pCBuffer->m_evtDataAvailable);
	LeaveCriticalSection(&pCBuffer->m_csCircleBuffer);
}

//
//
//
BOOL CircleIsComplete(CPs_CircleBuffer* pCBuffer)
{
	return pCBuffer->m_bComplete;
}

//
//
//

