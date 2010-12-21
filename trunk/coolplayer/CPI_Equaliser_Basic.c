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
#include "CPI_Equaliser.h"


////////////////////////////////////////////////////////////////////////////////
//
// This is an output stage that uses the Windows Wave Mapper.  Real simple stuff
// here - just a circular buffer of wave blocks and an event controlling the
// lot.  This should be considered the "reference" output stage.
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//

typedef struct _CPs_EqualiserContext_Basic
{
	BOOL m_bEnabled;
	int m_aryLevels[10];
	BOOL m_bStreamIsCapable;
	unsigned int m_aryHistory[256];  // 256 stereo samples
	unsigned int m_aryFuture[256];  // 256 stereo samples
	unsigned int m_iCursor;
	
	int m_arySum_left[9];
	int m_arySum_right[9];
	
} CPs_EqualiserContext_Basic;

//
////////////////////////////////////////////////////////////////////////////////

#define CIC_WRAPSAMPLE(expr)   ((expr)&0xFF)   // Wraps a sample to a 256 circular buffer
#define CIC_DECODESAMPLE_LEFT(expr)  ((short)LOWORD(expr))
#define CIC_DECODESAMPLE_RIGHT(expr) ((short)HIWORD(expr))
// #define CIC_DECODESAMPLE_LEFT(expr)  (*(short*)&(expr))
// #define CIC_DECODESAMPLE_RIGHT(expr) (*((short*)&(expr) + 1))
#define CIC_FPMULTIPLY(expr1, expr2) ( ( (expr1) * (expr2) )>>8 )
#define CIC_TRUNCATESHORT(expr)    (((expr) > SHRT_MAX) ? SHRT_MAX : (((expr) < SHRT_MIN) ? SHRT_MIN : (expr)))

const int glb_iEQOffsets[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256 };
const int glb_iEQOffsets_his[] = { -1, -2, -4, -8, -16, -32, -64, -128, -256 };


void CPP_EBSC_Initialise(CPs_EqualiserModule* pModule, const int iFrequency, const BOOL b16bit);
void CPP_EBSC_Uninitialise(CPs_EqualiserModule* pModule);
void CPP_EBSC_ApplySettings(CPs_EqualiserModule* pModule, const CPs_EQSettings* pSettings, BOOL* pbEnableStateChanged);
void CPP_EBSC_ApplyEQToBlock_Inplace(CPs_EqualiserModule* pModule, void* pPCMBlock, const DWORD dwBlockSize);
////////////////////////////////////////////////////////////////////////////////
//
//
//
void CPI_Player_Equaliser_Initialise_Basic(CPs_EqualiserModule* pModule)
{
	CPs_EqualiserContext_Basic* pContext;
	
	// Create module eps
	pModule->Initialise = CPP_EBSC_Initialise;
	pModule->Uninitialise = CPP_EBSC_Uninitialise;
	pModule->ApplySettings = CPP_EBSC_ApplySettings;
	pModule->ApplyEQToBlock_Inplace = CPP_EBSC_ApplyEQToBlock_Inplace;
	
	// Create a context
	pContext = (CPs_EqualiserContext_Basic*)malloc(sizeof(CPs_EqualiserContext_Basic));
	pModule->m_pModuleCookie = pContext;
	
	// Setup defaults
	memset(pContext->m_aryLevels, 0, sizeof(pContext->m_aryLevels));
	pContext->m_bEnabled = FALSE;
}

//
//
//
void CPP_EBSC_Initialise(CPs_EqualiserModule* pModule, const int iFrequency, const BOOL b16bit)
{
	CPs_EqualiserContext_Basic* pContext = (CPs_EqualiserContext_Basic*)pModule->m_pModuleCookie;
	CP_TRACE0("Equaliser (re)initialising");
	
	// Clear circular buffers
	memset(pContext->m_aryHistory, 0, sizeof(pContext->m_aryHistory));
	memset(pContext->m_aryFuture, 0, sizeof(pContext->m_aryFuture));
	memset(pContext->m_arySum_left, 0, sizeof(pContext->m_arySum_left));
	memset(pContext->m_arySum_right, 0, sizeof(pContext->m_arySum_right));
	pContext->m_iCursor = 0;
	
	// Only certain stream formats can be eq'd
	
	if (iFrequency == 44100 && b16bit == TRUE)
		pContext->m_bStreamIsCapable = TRUE;
	else
		pContext->m_bStreamIsCapable = FALSE;
}

//
//
//
void CPP_EBSC_Uninitialise(CPs_EqualiserModule* pModule)
{
	CPs_EqualiserContext_Basic* pContext = (CPs_EqualiserContext_Basic*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	CP_TRACE0("Equaliser shutting down");
	
	free(pContext);
	pModule->m_pModuleCookie = NULL;
}

//
//
//
void CPP_EBSC_ApplySettings(CPs_EqualiserModule* pModule, const CPs_EQSettings* pSettings, BOOL* pbEnableStateChanged)
{
	CPs_EqualiserContext_Basic* pContext = (CPs_EqualiserContext_Basic*)pModule->m_pModuleCookie;
	int iBandIDX = 0;
	
	// Setup levels
	
	for (iBandIDX = 0; iBandIDX < 8; iBandIDX++)
		pContext->m_aryLevels[9-iBandIDX] = (pSettings->m_aryBands[iBandIDX] << 1) + 256;
		
	pContext->m_aryLevels[0] = pContext->m_aryLevels[2];
	
	pContext->m_aryLevels[1] = pContext->m_aryLevels[2];
	
	// Setup enable state
	if (pContext->m_bEnabled == pSettings->m_bEnabled)
		*pbEnableStateChanged = FALSE;
	else
		*pbEnableStateChanged = TRUE;
		
	pContext->m_bEnabled = pSettings->m_bEnabled;
}

//
//
//
void CPP_EBSC_ApplyEQToBlock_Inplace(CPs_EqualiserModule* pModule, void* _pPCMBlock, const DWORD dwBlockSize)
{
	CPs_EqualiserContext_Basic* pContext = (CPs_EqualiserContext_Basic*)pModule->m_pModuleCookie;
	const int iNumSamples = dwBlockSize >> 2;
	int* pSampleBase = (int*)_pPCMBlock;
	int iSampleIDX, iPointIDX;
	unsigned int iThisSample, iThisSample_EQ, iTempSample;
	int iThisSample_left, iThisSample_right;
	int aryCoefficient_left[9], aryCoefficient_right[9];
	// int iClips = 0;
	
	// Skip EQ?
	
	if (pContext->m_bStreamIsCapable == FALSE || pContext->m_bEnabled == FALSE)
		return;
		
	// Treat samples as ints; Hiword is right channel, Loword is left channel
	for (iSampleIDX = 0; iSampleIDX < iNumSamples; iSampleIDX++)
	{
		// Scroll the future sample into this sample
		iThisSample = pContext->m_aryFuture[pContext->m_iCursor];
		pContext->m_aryFuture[pContext->m_iCursor] = pSampleBase[iSampleIDX];
		
		// Decode channels
		iThisSample_left = CIC_DECODESAMPLE_LEFT(iThisSample);
		iThisSample_right = CIC_DECODESAMPLE_RIGHT(iThisSample);
		
		// Perform processing
		
		// Add future offsets
		
		for (iPointIDX = 0; iPointIDX < 9; iPointIDX++)
		{
			iTempSample = pContext->m_aryFuture[CIC_WRAPSAMPLE(pContext->m_iCursor+glb_iEQOffsets[iPointIDX])];
			pContext->m_arySum_left[iPointIDX] += CIC_DECODESAMPLE_LEFT(iTempSample);
			pContext->m_arySum_right[iPointIDX] += CIC_DECODESAMPLE_RIGHT(iTempSample);
		}
		
		// Build scaled coefficients
		
		for (iPointIDX = 0; iPointIDX < 9; iPointIDX++)
		{
			aryCoefficient_left[iPointIDX] = pContext->m_arySum_left[iPointIDX] >> (iPointIDX + 1);
			aryCoefficient_right[iPointIDX] = pContext->m_arySum_right[iPointIDX] >> (iPointIDX + 1);
		}
		
		// Perform convolution
		iThisSample_left = CIC_FPMULTIPLY(iThisSample_left - aryCoefficient_left[0], pContext->m_aryLevels[0]);
		iThisSample_right = CIC_FPMULTIPLY(iThisSample_right - aryCoefficient_right[0], pContext->m_aryLevels[0]);
		
		for (iPointIDX = 0; iPointIDX < 8; iPointIDX++)
		{
			iThisSample_left += CIC_FPMULTIPLY(aryCoefficient_left[iPointIDX] - aryCoefficient_left[iPointIDX+1], pContext->m_aryLevels[iPointIDX+1]);
			iThisSample_right += CIC_FPMULTIPLY(aryCoefficient_right[iPointIDX] - aryCoefficient_right[iPointIDX+1], pContext->m_aryLevels[iPointIDX+1]);
		}
		
		iThisSample_left += CIC_FPMULTIPLY(aryCoefficient_left[8], pContext->m_aryLevels[9]);
		iThisSample_right += CIC_FPMULTIPLY(aryCoefficient_right[8], pContext->m_aryLevels[9]);
		
		// Update sums according to history
		
		for (iPointIDX = 0; iPointIDX < 9; iPointIDX++)
		{
			iTempSample = pContext->m_aryHistory[CIC_WRAPSAMPLE(pContext->m_iCursor+glb_iEQOffsets_his[iPointIDX])];
			pContext->m_arySum_left[iPointIDX] -= CIC_DECODESAMPLE_LEFT(iTempSample);
			pContext->m_arySum_right[iPointIDX] -= CIC_DECODESAMPLE_RIGHT(iTempSample);
		}
		
		// Encode sample
		iThisSample_EQ = MAKELONG(CIC_TRUNCATESHORT(iThisSample_left), CIC_TRUNCATESHORT(iThisSample_right));
		
		// Write this sample to the output buffer and history buffer
		pSampleBase[iSampleIDX] = iThisSample_EQ;
		
		pContext->m_aryHistory[pContext->m_iCursor] = iThisSample;
		
		// Increment circular buffers
		pContext->m_iCursor = CIC_WRAPSAMPLE(pContext->m_iCursor + 1);
	}
}

//
//
//
