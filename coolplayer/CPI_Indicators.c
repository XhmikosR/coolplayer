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
#include "CPI_InterfacePart.h"

////////////////////////////////////////////////////////////////////////////////
//

typedef struct _CPs_IndicatorValue
{
	char* m_pcName;
	char* m_pcValue;
	
	void* m_pNext;
	
} CPs_IndicatorValue;

//
//

typedef struct _CPs_IndicatorBinding
{
	char* m_pcName;
	CP_HINTERFACEPART m_hPart;
	
	void* m_pNext;
	
} CPs_IndicatorBinding;

//
////////////////////////////////////////////////////////////////////////////////



CPs_IndicatorValue* glb_pIndicatorValue = NULL;
CPs_IndicatorBinding* glb_pIndicatorBinding = NULL;
////////////////////////////////////////////////////////////////////////////////
//
//
//
void CPIC_FreeIndicators()
{
	CPs_IndicatorValue* pValueCursor;
	CPs_IndicatorValue* pValueCursor_next;
	
	for (pValueCursor = glb_pIndicatorValue; pValueCursor; pValueCursor = pValueCursor_next)
	{
		pValueCursor_next = (CPs_IndicatorValue*)pValueCursor->m_pNext;
		free(pValueCursor->m_pcName);
		free(pValueCursor->m_pcValue);
		free(pValueCursor);
	}
}

//
//
//
CPs_IndicatorValue* CPIC_LookupIndicator_Value(const char* pcName)
{
	CPs_IndicatorValue* pValueCursor;
	
	for (pValueCursor = glb_pIndicatorValue; pValueCursor; pValueCursor = (CPs_IndicatorValue*)pValueCursor->m_pNext)
	{
		if (stricmp(pValueCursor->m_pcName, pcName) == 0)
			return pValueCursor;
	}
	
	return NULL;
}

//
//
//
void CPIC_SetIndicatorValue(const char* pcName, const char* pcValue)
{
	CPs_IndicatorValue* pIndicatorValue;
	
	pIndicatorValue = CPIC_LookupIndicator_Value(pcName);
	
	// No value found - create one
	
	if (!pIndicatorValue)
	{
		pIndicatorValue = (CPs_IndicatorValue*)malloc(sizeof(CPs_IndicatorValue));
		STR_AllocSetString(&pIndicatorValue->m_pcName, pcName, FALSE);
		
		pIndicatorValue->m_pcValue = NULL;
		
		// Link in
		pIndicatorValue->m_pNext = glb_pIndicatorValue;
		glb_pIndicatorValue = pIndicatorValue;
	}
	
	// Set value
	STR_AllocSetString(&pIndicatorValue->m_pcValue, pcValue, TRUE);
	
	// Invalidate any bound controls
	{
		CPs_IndicatorBinding* pBindingCursor;
		
		for (pBindingCursor = glb_pIndicatorBinding; pBindingCursor; pBindingCursor = (CPs_IndicatorBinding*)pBindingCursor->m_pNext)
		{
			if (stricmp(pBindingCursor->m_pcName, pcName) == 0)
				IP_Invalidate(pBindingCursor->m_hPart);
		}
	}
}

//
//
//
const char* CPIC_GetIndicatorValue(const char* pcName)
{
	CPs_IndicatorValue* pIndicatorValue;
	pIndicatorValue = CPIC_LookupIndicator_Value(pcName);
	
	if (pIndicatorValue)
		return pIndicatorValue->m_pcValue;
		
	return NULL;
}

//
//
//
void CPIC_BindIndicatorToControl(const char* pcName, CP_HINTERFACEPART hPart)
{
	CPs_IndicatorBinding* pIndicatorBinding;
	
	pIndicatorBinding = (CPs_IndicatorBinding*)malloc(sizeof(CPs_IndicatorBinding));
	
	STR_AllocSetString(&pIndicatorBinding->m_pcName, pcName, FALSE);
	pIndicatorBinding->m_hPart = hPart;
	
	// Link in
	pIndicatorBinding->m_pNext = glb_pIndicatorBinding;
	glb_pIndicatorBinding = pIndicatorBinding;
}

//
//
//
void CPIC_UnBindControl(CP_HINTERFACEPART hPart)
{
	CPs_IndicatorBinding* pBindingCursor;
	CPs_IndicatorBinding** ppBindingCursor_Referrer;
	
	for (pBindingCursor = glb_pIndicatorBinding, ppBindingCursor_Referrer = &glb_pIndicatorBinding;
			pBindingCursor;
			pBindingCursor = (CPs_IndicatorBinding*)pBindingCursor->m_pNext,
			ppBindingCursor_Referrer = (CPs_IndicatorBinding**) & pBindingCursor->m_pNext)
	{
		if (pBindingCursor->m_hPart == hPart)
		{
			*ppBindingCursor_Referrer = (CPs_IndicatorBinding*)pBindingCursor->m_pNext;
			free(pBindingCursor->m_pcName);
			free(pBindingCursor);
			return;
		}
	}
}

//
//
//

