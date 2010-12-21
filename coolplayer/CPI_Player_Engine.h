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

#include "CPI_Player.h"
#include "CPI_Player_Messages.h"
#include "CPI_Player_CoDec.h"
#include "CPI_Player_Output.h"
#include "CPI_Equaliser.h"

////////////////////////////////////////////////////////////////////////////////
//

typedef struct __CPs_PlayerContext
{
	CPs_PlayEngine* m_pBaseEngineParams;
	CPs_CoDecModule m_CoDecs[CP_CODEC_last+1];
	CPs_OutputModule m_OutputModules[CP_OUTPUT_last+1];
	
	CPs_OutputModule* m_pCurrentOutputModule;
	BOOL m_bOutputActive;
	DWORD m_dwCurrentOutputModule;
	int m_iInternalVolume;
	
	int m_iLastSentTime_Secs;
	int m_iLastSentTime_Proportion;
	int m_iProportion_TrackLength;
	
	int m_iOpenDevice_Freq_Hz;
	BOOL m_bOpenDevice_Stereo;
	BOOL m_bOpenDevice_16bit;
	
	CPs_EqualiserModule m_Equaliser;
	
} CPs_PlayerContext;

//
////////////////////////////////////////////////////////////////////////////////
