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
// Cooler PlaylistItem Internal
//
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
//
typedef enum _CPe_TagType
{
	ttUnread,
	ttNone,
	ttID3v1,
	ttID3v2
} CPe_TagType;
//
////////////////////////////////////////////////////////////////////////////////


#define CPC_TRACKNUMASTEXTBUFFERSIZE			16
////////////////////////////////////////////////////////////////////////////////
//

typedef struct _CPs_PlaylistItem
{
	char* m_pcPath;
	char* m_pcFilename;
	
	BOOL m_bID3Tag_SaveRequired;
	CPe_TagType m_enTagType;
	BOOL m_bDestroyOnDeactivate;
	
	char m_cTrackStackPos_AsText[16];
	int m_iTrackStackPos;
	char* m_pcArtist;
	char* m_pcAlbum;
	char* m_pcTrackName;
	char* m_pcYear;
	char* m_pcComment;
	unsigned char m_cTrackNum;
	char* m_pcTrackNum_AsText;
	unsigned char m_cGenre;
	char* m_pcTrackLength_AsText;
	unsigned int m_iTrackLength;
	
	int m_iCookie;
	
	CP_HPLAYLISTITEM m_hNext;
	CP_HPLAYLISTITEM m_hPrev;
	
} CPs_PlaylistItem;

//
#define CPLII_DECODEHANDLE(hitem)				((CPs_PlaylistItem*)(hitem))
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
CP_HPLAYLISTITEM CPLII_CreateItem(const char* pcPath);
void CPLII_DestroyItem(CP_HPLAYLISTITEM hItem);
