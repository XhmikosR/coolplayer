
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
// ID3 stuff
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
#define CIC_NUMGENRES 149
extern const char* glb_pcGenres[];
//
#pragma pack(push, 1)

typedef struct _CIs_ID3Tag
{
	char m_cTAG[3];  // Must equal "TAG"
	char m_cSong[30];
	char m_cArtist[30];
	char m_cAlbum[30];
	char m_cYear[4];
	char m_cComment[30];
	unsigned char m_cGenre;
} CIs_ID3Tag;

#pragma pack(pop)
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
// ID3v2 stuff
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
#pragma pack(push, 1)

typedef struct _CIs_ID3v2Tag
{
	char m_cTAG[3];  // Must equal ID3
	unsigned char m_cVersion[2]; // Major / Minor
	unsigned char m_cFlags;
	unsigned char m_cSize_Encoded[4];
} CIs_ID3v2Tag;

//

typedef struct _CIs_ID3v2Frame
{
	char m_cFrameID[4];
	unsigned char m_cSize_Encoded[4];
	unsigned short m_cFlags;
} CIs_ID3v2Frame;

#pragma pack(pop)
//
#define ID3v2_FLAG_UNSYNC   0x80
#define ID3v2_FLAG_EXTENDEDHEADER 0x40
#define ID3v2_FLAG_EXPERIMENTAL  0x20
#define ID3v2_FLAG_FOOTER   0x10
////////////////////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////////////////////
//
// OGG stuff
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
#pragma pack(push, 1)
//

typedef struct _CIs_OGGFrame_header
{
	char m_cCapture[4];  // Must equal OggS
	unsigned char m_cVersion;
	unsigned char m_cFlags;
	unsigned char m_cGranulePos[8];
	unsigned char m_cSerialNum[4];
	unsigned char m_cPageSEQ[4];
	unsigned char m_cPageCheckum[4];
	unsigned char m_cNumSegments;
	
} CIs_OGGFrame_header;

//

typedef struct _CIs_OGGFrame
{
	CIs_OGGFrame_header m_Header;
	unsigned char m_cSegments[0xFF];
	
} CIs_OGGFrame;

//

typedef struct _CIs_OGGComment
{
	char* m_pcVendorString;
	unsigned int m_iNumComments;
	char** m_ppUserStrings;
	
} CIs_OGGComment;

//
#pragma pack(pop)
//
////////////////////////////////////////////////////////////////////////////////

