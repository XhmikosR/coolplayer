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




////////////////////////////////////////////////////////////////////////////////
//
// Cooler equaliser module
//
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
// Equaliser functions
typedef void (*pfnEQ_Initialise)(CP_HEQUALISER pModule, const int iFrequency, const BOOL b16bit);
typedef void (*pfnEQ_Uninitialise)(CP_HEQUALISER pModule);
typedef void (*pfnEQ_ApplySettings)(CP_HEQUALISER pModule, const CPs_EQSettings* pSettings, BOOL* pbEnableStateChanged);
typedef void (*pfnEQ_ApplyEQToBlock_Inplace)(CP_HEQUALISER pModule, void* pPCMBlock, const DWORD dwBlockSize);
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
// Output module
typedef struct _CPs_EqualiserModule
{
	// Public functions
	pfnEQ_Initialise Initialise;   // Called by engine only
	pfnEQ_Uninitialise Uninitialise;  // Called by engine only
	pfnEQ_ApplySettings ApplySettings;  // Called by engine only
	pfnEQ_ApplyEQToBlock_Inplace ApplyEQToBlock_Inplace; // Called by output module
	
	// Public variables
	
	// Private variables
	void* m_pModuleCookie;  // This is a pointer to any private data the module may want to maintain
	
} CPs_EqualiserModule;

//
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
// Initialisers
void CPI_Player_Equaliser_Initialise_Basic(CPs_EqualiserModule* pModule);  // delux EQ
////////////////////////////////////////////////////////////////////////////////


