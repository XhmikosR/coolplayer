
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
// Player verbs
//
// - This will contain all the verbs (commands) for the player - the verb handlers
// are also capable of returning it's skin def name and it's legacy skin def name
//
////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////
//
typedef enum _CPe_VerbAction
{
	vaDoVerb,   // pParam is the window that originates the verb
	vaQueryName
} CPe_VerbAction;
//
//

typedef struct _CPs_VerbQueryName
{
	const char* m_pcName;
	BOOL m_bNameMatched;
	
} CPs_VerbQueryName;

//
//
typedef void (*wp_Verb)(const CPe_VerbAction enAction, void* pParam);
////////////////////////////////////////////////////////////////////////////////


extern wp_Verb glb_pfnAllVerbs[];
////////////////////////////////////////////////////////////////////////////////
//
void CPVERB_TogglePlaylistWindow(const CPe_VerbAction enAction, void* pParam);
void CPVERB_ToggleRepeat(const CPe_VerbAction enAction, void* pParam);
void CPVERB_ToggleShuffle(const CPe_VerbAction enAction, void* pParam);
void CPVERB_ToggleEqualiser(const CPe_VerbAction enAction, void* pParam);
void CPVERB_ToggleFindDialog(const CPe_VerbAction enAction, void* pParam);
//
void CPVERB_PlaylistClearSelected(const CPe_VerbAction enAction, void* pParam);
void CPVERB_PlaylistClearAll(const CPe_VerbAction enAction, void* pParam);
//
void CPVERB_Play(const CPe_VerbAction enAction, void* pParam);
void CPVERB_Stop(const CPe_VerbAction enAction, void* pParam);
void CPVERB_Pause(const CPe_VerbAction enAction, void* pParam);
void CPVERB_NextTrack(const CPe_VerbAction enAction, void* pParam);
void CPVERB_PrevTrack(const CPe_VerbAction enAction, void* pParam);
void CPVERB_SkipForwards(const CPe_VerbAction enAction, void* pParam);
void CPVERB_SkipBackwards(const CPe_VerbAction enAction, void* pParam);
void CPVERB_VolumeUp(const CPe_VerbAction enAction, void* pParam);
void CPVERB_VolumeDown(const CPe_VerbAction enAction, void* pParam);
//
void CPVERB_OpenFile(const CPe_VerbAction enAction, void* pParam);
void CPVERB_AddFile(const CPe_VerbAction enAction, void* _pParam);
void CPVERB_About(const CPe_VerbAction enAction, void* pParam);
void CPVERB_Exit(const CPe_VerbAction enAction, void* pParam);
//
void CPVERB_SavePlaylist(const CPe_VerbAction enAction, void* pParam);
void CPVERB_PlaylistShuffle(const CPe_VerbAction enAction, void* pParam);
void CPVERB_PlaylistOffsetUp(const CPe_VerbAction enAction, void* pParam);
void CPVERB_PlaylistOffsetDown(const CPe_VerbAction enAction, void* pParam);
void CPVERB_AddDirectory(const CPe_VerbAction enAction, void* pParam);
void CPVERB_PlaylistMinimise(const CPe_VerbAction enAction, void* pParam);
void CPVERB_PlaylistMaximise(const CPe_VerbAction enAction, void* pParam);
////////////////////////////////////////////////////////////////////////////////
