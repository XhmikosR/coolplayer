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
#include "CPI_Playlist.h"
#include "CPI_PlaylistItem.h"
#include "CPI_PlaylistItem_Internal.h"
#include "CPI_ID3.h"
#include "ogg/ogg.h"
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"
#include "CP_RIFFStructs.h"

void CPLI_OGG_SkipOverTab(FILE* pFile);
void CPLI_SetPath(CPs_PlaylistItem* pItem, const char* pcNewPath);
void CPLI_ReadTag_ID3v1(CPs_PlaylistItem* pItem, HANDLE hFile);
void CPLI_ReadTag_ID3v2(CPs_PlaylistItem* pItem, HANDLE hFile);
void CPLI_ReadTag_OGG(CPs_PlaylistItem* pItem);
void CPLI_WriteTag_ID3v1(CPs_PlaylistItem* pItem, HANDLE hFile);
void CPLI_WriteTag_ID3v2(CPs_PlaylistItem* pItem, HANDLE hFile);
void CPLI_WriteTag_OGG(CPs_PlaylistItem* pItem, HANDLE hFile);
void CPLI_CalculateLength_OGG(CPs_PlaylistItem* pItem);
void CPLI_CalculateLength_MP3(CPs_PlaylistItem* pItem);
void CPLI_CalculateLength_WAV(CPs_PlaylistItem* pItem);
void CPLI_ShrinkFile(HANDLE hFile, const DWORD dwStartOffset, const unsigned int iNumBytes);
BOOL CPLI_GrowFile(HANDLE hFile, const DWORD dwStartOffset, const unsigned int iNumBytes);
////////////////////////////////////////////////////////////////////////////////
//
//
//
CP_HPLAYLISTITEM CPLII_CreateItem(const char* pcPath)
{
	CPs_PlaylistItem* pNewItem = (CPs_PlaylistItem*)malloc(sizeof(CPs_PlaylistItem));
	
	pNewItem->m_pcPath = NULL;
	CPLI_SetPath(pNewItem, pcPath);
	
	pNewItem->m_cTrackStackPos_AsText[0] = '\0';
	pNewItem->m_iTrackStackPos = CIC_TRACKSTACK_UNSTACKED;
	pNewItem->m_enTagType = ttUnread;
	pNewItem->m_bID3Tag_SaveRequired = FALSE;
	pNewItem->m_bDestroyOnDeactivate = FALSE;
	pNewItem->m_pcArtist = NULL;
	pNewItem->m_pcAlbum = NULL;
	pNewItem->m_pcTrackName = NULL;
	pNewItem->m_pcComment = NULL;
	pNewItem->m_pcYear = NULL;
	pNewItem->m_cGenre = CIC_INVALIDGENRE;
	pNewItem->m_cTrackNum = CIC_INVALIDTRACKNUM;
	pNewItem->m_pcTrackNum_AsText = NULL;
	pNewItem->m_iTrackLength = 0;
	pNewItem->m_pcTrackLength_AsText = NULL;
	
	pNewItem->m_iCookie = -1;
	
	pNewItem->m_hNext = NULL;
	pNewItem->m_hPrev = NULL;
	
	return pNewItem;
}

//
//
//
void CPLII_RemoveTagInfo(CPs_PlaylistItem* pItem)
{
	if (pItem->m_pcArtist)
	{
		free(pItem->m_pcArtist);
		pItem->m_pcArtist = NULL;
	}
	
	if (pItem->m_pcAlbum)
	{
		free(pItem->m_pcAlbum);
		pItem->m_pcAlbum = NULL;
	}
	
	if (pItem->m_pcTrackName)
	{
		free(pItem->m_pcTrackName);
		pItem->m_pcTrackName = NULL;
	}
	
	if (pItem->m_pcComment)
	{
		free(pItem->m_pcComment);
		pItem->m_pcComment = NULL;
	}
	
	if (pItem->m_pcYear)
	{
		free(pItem->m_pcYear);
		pItem->m_pcYear = NULL;
	}
	
	if (pItem->m_pcTrackNum_AsText)
	{
		free(pItem->m_pcTrackNum_AsText);
		pItem->m_pcTrackNum_AsText = NULL;
	}
	
	if (pItem->m_pcTrackLength_AsText)
	{
		free(pItem->m_pcTrackLength_AsText);
		pItem->m_pcTrackLength_AsText = NULL;
	}
	
	pItem->m_cGenre = CIC_INVALIDGENRE;
	
	pItem->m_iTrackLength = 0;
	pItem->m_cTrackNum = CIC_INVALIDTRACKNUM;
	pItem->m_enTagType = ttUnread;
	pItem->m_bID3Tag_SaveRequired = FALSE;
}

//
//
//
void CPLII_DestroyItem(CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	CPLII_RemoveTagInfo(pItem);
	free(pItem->m_pcPath);
	free(pItem);
}

//
//
//
const char* CPLI_GetPath(const CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	return pItem->m_pcPath;
}

//
//
//
const char* CPLI_GetFilename(const CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	return pItem->m_pcFilename;
}

//
//
//
int CPLI_GetTrackStackPos(const CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	return pItem->m_iTrackStackPos;
}

//
//
//
const char* CPLI_GetTrackStackPos_AsText(const CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	return pItem->m_cTrackStackPos_AsText;
}

//
//
//
const char* CPLI_GetArtist(const CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	return pItem->m_pcArtist;
}

//
//
//
const char* CPLI_GetAlbum(const CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	return pItem->m_pcAlbum;
}

//
//
//
const char* CPLI_GetTrackName(const CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	return pItem->m_pcTrackName;
}

//
//
//
const char* CPLI_GetComment(const CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	return pItem->m_pcComment;
}

//
//
//
const char* CPLI_GetYear(const CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	return pItem->m_pcYear;
}

//
//
//
const char* CPLI_GetGenre(const CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	if (pItem->m_cGenre == CIC_INVALIDGENRE)
		return NULL;
		
	return glb_pcGenres[pItem->m_cGenre];
}

//
//
//
const unsigned char CPLI_GetTrackNum(const CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	return pItem->m_cTrackNum;
}

//
//
//
const char* CPLI_GetTrackNum_AsText(const CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	return pItem->m_pcTrackNum_AsText;
}

//
//
//
const char* CPLI_GetTrackLength_AsText(const CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	return pItem->m_pcTrackLength_AsText;
}

//
//
//
int CPLI_GetTrackLength(const CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	return pItem->m_iTrackLength;
}

//
//
//
void CPLI_SetCookie(CP_HPLAYLISTITEM hItem, const int iCookie)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	pItem->m_iCookie = iCookie;
}

//
//
//
int CPLI_GetCookie(const CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	return pItem->m_iCookie;
}

//
//
//
CP_HPLAYLISTITEM CPLI_Next(const CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	return pItem->m_hNext;
}

//
//
//
CP_HPLAYLISTITEM CPLI_Prev(const CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	return pItem->m_hPrev;
}

//
//
//
char* DecodeID3String(const char* pcSource, const int iLength)
{
	char* cWorkString = (char*)_alloca(iLength + 1);
	char* pDestString;
	char* pcLastWhiteSpace;
	int iCharIDX;
	
	cWorkString[iLength] = '\0';
	memcpy(cWorkString, pcSource, iLength);
	
	// Remove trailing whitespace
	pcLastWhiteSpace = NULL;
	
	for (iCharIDX = 0; cWorkString[iCharIDX]; iCharIDX++)
	{
		if (cWorkString[iCharIDX] == ' ')
		{
			if (!pcLastWhiteSpace)
				pcLastWhiteSpace = cWorkString + iCharIDX;
		}
		
		else
			pcLastWhiteSpace = NULL;
	}
	
	if (pcLastWhiteSpace)
		*pcLastWhiteSpace = '\0';
		
	// Copy string
	STR_AllocSetString(&pDestString, cWorkString, FALSE);
	
	return pDestString;
}

//
//
//
void CPLI_ReadTag(CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	HANDLE hFile;
	
	CP_CHECKOBJECT(pItem);
	
	if (pItem->m_enTagType != ttUnread)
		return;
		
	// - Try to open the file
	hFile = CreateFile(pItem->m_pcPath, GENERIC_READ,
					   FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
					   OPEN_EXISTING, 0, 0);
	                   
	// Cannot open - fail silently
	if (hFile == INVALID_HANDLE_VALUE)
		return;
		
	// Try to read a V2 tag
	if (options.support_id3v2)
		CPLI_ReadTag_ID3v2(pItem, hFile);
		
	// Failed? - try a V1 tag instead
	if (pItem->m_enTagType == ttUnread)
		CPLI_ReadTag_ID3v1(pItem, hFile);
		
	CloseHandle(hFile);
	
	// Override information with any OGG tags that may be there
	if (options.prefer_native_ogg_tags
			&& stricmp(".ogg", CPLI_GetExtension(hItem)) == 0)
	{
		CPLI_ReadTag_OGG(pItem);
	}
	
	// Update interface
	CPL_cb_OnItemUpdated(hItem);
}

//
//
//
char* CPLI_ID3v2_DecodeString(const BYTE* pSourceText, const int iTagDataSize)
{
	int iStringLength;
	char* pcDestString;
	
	if (pSourceText[0] == '\0')
	{
		iStringLength = iTagDataSize - 1;
		pcDestString = malloc(iStringLength + 1);
		memcpy(pcDestString, pSourceText + 1, iStringLength);
		pcDestString[iStringLength] = 0;
	}
	
	else
	{
		CP_TRACE0("ID3v2 Unknown encoding");
		pcDestString = NULL;
	}
	
	return pcDestString;
}

//
//
//
void CPLI_DecodeLength(CPs_PlaylistItem* pItem, unsigned int iNewLength)
{
	int iHours, iMins, iSecs;
	
	// Free existing buffer
	
	if (pItem->m_pcTrackLength_AsText)
	{
		free(pItem->m_pcTrackLength_AsText);
		pItem->m_pcTrackLength_AsText = NULL;
	}
	
	pItem->m_iTrackLength = iNewLength;
	
	iHours = iNewLength / 3600;
	iMins = (iNewLength - (iHours * 3600)) / 60;
	iSecs = iNewLength - (iHours * 3600) - (iMins * 60);
	
	// If length has hours then format as hh:mm:ss otherwise format as mm:ss
	
	if (iHours > 0)
	{
		pItem->m_pcTrackLength_AsText = (char*)malloc(9);
		sprintf(pItem->m_pcTrackLength_AsText, "%02d:%02d:%02d", iHours, iMins, iSecs);
	}
	
	else
	{
		pItem->m_pcTrackLength_AsText = (char*)malloc(6);
		sprintf(pItem->m_pcTrackLength_AsText, "%02d:%02d", iMins, iSecs);
	}
}

//
//
//
void CPLI_ReadTag_ID3v2(CPs_PlaylistItem* pItem, HANDLE hFile)
{
	DWORD dwBytesRead;
	int iTagDataToRead;
	CIs_ID3v2Tag ID3v2;
	
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	ReadFile(hFile, &ID3v2, sizeof(ID3v2), &dwBytesRead, NULL);
	
	// Not enough file data returned - or the data returned does not look like an ID3
	
	if (dwBytesRead != sizeof(ID3v2)
			|| memcmp(ID3v2.m_cTAG, "ID3", 3) != 0
			|| (ID3v2.m_cVersion[0] != 0x03 && ID3v2.m_cVersion[0] != 0x04)) // Major version wrong
	{
		return;
	}
	
	// Work out the amount of tag left to read
	iTagDataToRead = (ID3v2.m_cSize_Encoded[0] << 21)
					 | (ID3v2.m_cSize_Encoded[1] << 14)
					 | (ID3v2.m_cSize_Encoded[2] << 7)
					 | ID3v2.m_cSize_Encoded[3];
	                 
	// Check for a big enough file now (to save endless checking)
	if (GetFileSize(hFile, NULL) < (sizeof(ID3v2) + iTagDataToRead))
		return;
		
	// Skip over extended header (if there is one)
	if (ID3v2.m_cFlags & ID3v2_FLAG_EXTENDEDHEADER)
	{
		char cExtendedHeaderSize_Encoded[4];
		int iExtendedHeaderSize;
		
		ReadFile(hFile, cExtendedHeaderSize_Encoded, sizeof(cExtendedHeaderSize_Encoded), &dwBytesRead, NULL);
		
		iExtendedHeaderSize = (cExtendedHeaderSize_Encoded[0] << 21)
							  | (cExtendedHeaderSize_Encoded[1] << 14)
							  | (cExtendedHeaderSize_Encoded[2] << 7)
							  | cExtendedHeaderSize_Encoded[3];
		                      
		SetFilePointer(hFile, iExtendedHeaderSize - sizeof(cExtendedHeaderSize_Encoded), NULL, FILE_CURRENT);
		iTagDataToRead -= iExtendedHeaderSize;
	}
	
	while (iTagDataToRead > sizeof(CIs_ID3v2Frame))
	{
		CIs_ID3v2Frame ID3v2Frame;
		BYTE* pFrameData;
		int iFrameSize;
		
		ReadFile(hFile, &ID3v2Frame, sizeof(ID3v2Frame), &dwBytesRead, NULL);
		
		// Have we encountered padding?
		
		if (ID3v2Frame.m_cFrameID[0] == '\0')
			break;
			
		if (ID3v2.m_cVersion[0] == 0x03)
		{
			iFrameSize = (ID3v2Frame.m_cSize_Encoded[0] << 24)
						 | (ID3v2Frame.m_cSize_Encoded[1] << 16)
						 | (ID3v2Frame.m_cSize_Encoded[2] << 8)
						 | ID3v2Frame.m_cSize_Encoded[3];
		}
		
		else
		{
			iFrameSize = (ID3v2Frame.m_cSize_Encoded[0] << 21)
						 | (ID3v2Frame.m_cSize_Encoded[1] << 14)
						 | (ID3v2Frame.m_cSize_Encoded[2] << 7)
						 | ID3v2Frame.m_cSize_Encoded[3];
		}
		
		// Frame size invalid?
		
		if (iFrameSize > iTagDataToRead)
			return;
			
		pFrameData = malloc(iFrameSize + 1);
		
		if (!ReadFile(hFile, pFrameData, iFrameSize, &dwBytesRead, NULL)) return;
		
		pFrameData[iFrameSize] = '\0';
		
		// Decode frames
		if (memcmp(ID3v2Frame.m_cFrameID, "TIT2", 4) == 0)
			pItem->m_pcTrackName = CPLI_ID3v2_DecodeString(pFrameData, iFrameSize);
		else if (memcmp(ID3v2Frame.m_cFrameID, "TPE1", 4) == 0)
			pItem->m_pcArtist = CPLI_ID3v2_DecodeString(pFrameData, iFrameSize);
		else if (memcmp(ID3v2Frame.m_cFrameID, "TALB", 4) == 0)
			pItem->m_pcAlbum = CPLI_ID3v2_DecodeString(pFrameData, iFrameSize);
		else if (memcmp(ID3v2Frame.m_cFrameID, "TRCK", 4) == 0)
		{
			pItem->m_pcTrackNum_AsText = CPLI_ID3v2_DecodeString(pFrameData, iFrameSize);
			
			if (pItem->m_pcTrackNum_AsText)
				pItem->m_cTrackNum = (unsigned char)atoi(pItem->m_pcTrackNum_AsText);
		}
		
		else if (memcmp(ID3v2Frame.m_cFrameID, "TYER", 4) == 0)
			pItem->m_pcYear = CPLI_ID3v2_DecodeString(pFrameData, iFrameSize);
		else if (memcmp(ID3v2Frame.m_cFrameID, "TENC", 4) == 0)
			pItem->m_pcComment = CPLI_ID3v2_DecodeString(pFrameData, iFrameSize);
		else if (memcmp(ID3v2Frame.m_cFrameID, "TCON", 4) == 0)
		{
			char* pcGenre = CPLI_ID3v2_DecodeString(pFrameData, iFrameSize);
			
			if (pcGenre)
			{
				// Search for this genre among the ID3v1 genres (don't read it if we cannot find it)
				int iGenreIDX;
				
				for (iGenreIDX = 0; iGenreIDX < CIC_NUMGENRES; iGenreIDX++)
				{
					if (stricmp(pcGenre, glb_pcGenres[iGenreIDX]) == 0)
					{
						pItem->m_cGenre = (unsigned char)iGenreIDX;
						break;
					}
				}
				
				free(pcGenre);
			}
		}
		
		else if (memcmp(ID3v2Frame.m_cFrameID, "TLEN", 4) == 0)
		{
			char* pcLength = CPLI_ID3v2_DecodeString(pFrameData, iFrameSize);
			
			if (pcLength)
			{
				CPLI_DecodeLength(pItem, atoi(pcLength) / 1000);
				free(pcLength);
			}
		}
		
#ifdef _DEBUG
		/*
		else if(ID3v2Frame.m_cFrameID[0] == 'T')
		 CP_TRACE2("Text frame %4s \"%s\"", ID3v2Frame.m_cFrameID, pFrameData+1);
		else
		 CP_TRACE1("Any old frame %4s", ID3v2Frame.m_cFrameID);
		*/
#endif
		free(pFrameData);
		
		iTagDataToRead -= iFrameSize + sizeof(ID3v2Frame);
	}
	
	pItem->m_enTagType = ttID3v2;
}

//
//
//
void CPLI_ReadTag_ID3v1(CPs_PlaylistItem* pItem, HANDLE hFile)
{
	DWORD dwBytesRead;
	CIs_ID3Tag ID3;
	
	SetFilePointer(hFile, 0 - sizeof(ID3), NULL, FILE_END);
	ReadFile(hFile, &ID3, sizeof(ID3), &dwBytesRead, NULL);
	
	// Not enough file data returned - or the data returned does not look like an ID3
	
	if (dwBytesRead != sizeof(ID3) || memcmp(ID3.m_cTAG, "TAG", 3) != 0)
		return;
		
	// Decode the fixed strings into our dynamic strings
	CPLII_RemoveTagInfo(pItem);
	
	pItem->m_pcTrackName = DecodeID3String(ID3.m_cSong, 30);
	pItem->m_pcArtist = DecodeID3String(ID3.m_cArtist, 30);
	pItem->m_pcAlbum = DecodeID3String(ID3.m_cAlbum, 30);
	pItem->m_pcYear = DecodeID3String(ID3.m_cYear, 4);
	
	// ID3v1.1 - If the 29th byte of the comment is 0 then the 30th byte is the track num
	// ** Some dodgy implementations of ID3v1.1 just slap a <32 char byte at position 30 and hope
	//    for the best - handle these too <hmph!>
	if (ID3.m_cComment[28] == '\0' || ID3.m_cComment[29] < 32)
	{
		char cTempString[33];
		
		pItem->m_pcComment = DecodeID3String(ID3.m_cComment, 28);
		pItem->m_cTrackNum = ID3.m_cComment[29];
		
		if (pItem->m_cTrackNum != CIC_INVALIDTRACKNUM)
		{
			_itoa(pItem->m_cTrackNum, cTempString, 10);
			pItem->m_pcTrackNum_AsText = (char*)malloc(CPC_TRACKNUMASTEXTBUFFERSIZE);
			strncpy(pItem->m_pcTrackNum_AsText, cTempString, CPC_TRACKNUMASTEXTBUFFERSIZE);
		}
	}
	
	else
	{
		pItem->m_pcComment = DecodeID3String(ID3.m_cComment, 30);
		pItem->m_cTrackNum = CIC_INVALIDTRACKNUM;
	}
	
	if (ID3.m_cGenre < CIC_NUMGENRES)
		pItem->m_cGenre = ID3.m_cGenre;
		
	pItem->m_enTagType = ttID3v1;
}

//
//
//
BOOL CPLI_IsTagDirty(CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	
	CP_CHECKOBJECT(pItem);
	return pItem->m_bID3Tag_SaveRequired;
}

//
//
//
void CPLI_WriteTag(CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	HANDLE hFile;
	
	CP_CHECKOBJECT(pItem);
	
	if (pItem->m_bID3Tag_SaveRequired == FALSE)
		return;
		
	if (stricmp(".ogg", CPLI_GetExtension(hItem)) != 0 &&
			stricmp(".mp3", CPLI_GetExtension(hItem)) != 0)
		return;
		
	// Try to open the file
	hFile = CreateFile(pItem->m_pcPath, GENERIC_READ | GENERIC_WRITE,
					   FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
					   OPEN_EXISTING, 0, 0);
	                   
	// Cannot open - fail silently
	if (hFile == INVALID_HANDLE_VALUE)
		return;
		
	pItem->m_bID3Tag_SaveRequired = FALSE;
	
	if (options.prefer_native_ogg_tags
			&& stricmp(".ogg", CPLI_GetExtension(hItem)) == 0)
	{
		CPLI_WriteTag_OGG(pItem, hFile);
	}
	
	else if (stricmp(".mp3", CPLI_GetExtension(hItem)) == 0)
	{
		CPLI_WriteTag_ID3v1(pItem, hFile);
		
		if (options.support_id3v2)
			CPLI_WriteTag_ID3v2(pItem, hFile);
	}
	
	CloseHandle(hFile);
}

//
//
//
void CPLI_WriteTag_ID3v1(CPs_PlaylistItem* pItem, HANDLE hFile)
{
	DWORD dwBytesTransferred;
	CIs_ID3Tag ID3;
	char cTagMagic[3];
	
	// Build the tag (of ID3v1.1 format)
	memset(&ID3, 32, sizeof(ID3));
	memcpy(ID3.m_cTAG, "TAG", 3);
	
	if (pItem->m_pcTrackName)
		strncpy(ID3.m_cSong, pItem->m_pcTrackName, 30);
		
	if (pItem->m_pcArtist)
		strncpy(ID3.m_cArtist, pItem->m_pcArtist, 30);
		
	if (pItem->m_pcAlbum)
		strncpy(ID3.m_cAlbum, pItem->m_pcAlbum, 30);
		
	if (pItem->m_pcYear)
		strncpy(ID3.m_cYear, pItem->m_pcYear, 4);
		
	if (pItem->m_pcComment)
	{
		if (strlen(pItem->m_pcComment) > 28)
		{
			strncpy(ID3.m_cComment, pItem->m_pcComment, 30);
		}
		
		else
		{
			strncpy(ID3.m_cComment, pItem->m_pcComment, 28);
			ID3.m_cComment[28] = '\0';
			ID3.m_cComment[29] = pItem->m_cTrackNum;
		}
	}
	
	ID3.m_cGenre = pItem->m_cGenre;
	
	// Set the file pointer to the end of the file (or the start of the tag if there is one already)
	SetFilePointer(hFile, 0 - sizeof(ID3), NULL, FILE_END);
	ReadFile(hFile, cTagMagic, sizeof(cTagMagic), &dwBytesTransferred, NULL);
	
	if (memcmp(cTagMagic, "TAG", 3) == 0)
		SetFilePointer(hFile, 0 - sizeof(ID3), NULL, FILE_END);
	else
		SetFilePointer(hFile, 0, NULL, FILE_END);
		
	WriteFile(hFile, &ID3, sizeof(ID3), &dwBytesTransferred, NULL);
}

//
//
//
void CPLI_ID3v2_WriteSyncSafeInt(char cDest[4], const int iSource)
{
	cDest[0] = (iSource >> 21) & 0x7F;
	cDest[1] = (iSource >> 14) & 0x7F;
	cDest[2] = (iSource >> 7) & 0x7F;
	cDest[3] = iSource & 0x7F;
}

//
//
//
void CPLI_ID3v2_WriteTextFrame(BYTE** ppDest, const char pcTag[4], const char* pcString)
{
	CIs_ID3v2Frame* pFrame = (CIs_ID3v2Frame*) * ppDest;
	BYTE* pFrameData;
	int iFrameDataLength;
	
	iFrameDataLength = strlen(pcString) + 1; // 1-byte for encoding
	
	memcpy(pFrame->m_cFrameID, pcTag, sizeof(pcTag));
	CPLI_ID3v2_WriteSyncSafeInt(pFrame->m_cSize_Encoded, iFrameDataLength);
	pFrame->m_cFlags = 0x0;
	
	// Write frame data
	pFrameData = ((*ppDest) + sizeof(CIs_ID3v2Frame));
	pFrameData[0] = 0x0;
	memcpy(pFrameData + 1, pcString, iFrameDataLength - 1);
	
	*ppDest += iFrameDataLength + sizeof(CIs_ID3v2Frame);
}

//
//
//
void CPLI_WriteTag_ID3v2(CPs_PlaylistItem* pItem, HANDLE hFile)
{
	unsigned int iTagDataLength;
	unsigned int iExistingTagLength;
	DWORD dwBytesTransferred;
	char atiobuffer[33];
	BYTE* pTag;
	BYTE* pTag_Cursor;
	
	// Work out the size of the data in the tag frames
	iTagDataLength = 0;
	
	if (pItem->m_pcTrackName)
		iTagDataLength += strlen(pItem->m_pcTrackName) + 1 + sizeof(CIs_ID3v2Frame); // 1-byte for encoding and a frame header
		
	if (pItem->m_pcArtist)
		iTagDataLength += strlen(pItem->m_pcArtist) + 1 + sizeof(CIs_ID3v2Frame); // 1-byte for encoding and a frame header
		
	if (pItem->m_pcAlbum)
		iTagDataLength += strlen(pItem->m_pcAlbum) + 1 + sizeof(CIs_ID3v2Frame); // 1-byte for encoding and a frame header
		
	if (pItem->m_pcYear)
		iTagDataLength += strlen(pItem->m_pcYear) + 1 + sizeof(CIs_ID3v2Frame); // 1-byte for encoding and a frame header
		
	if (pItem->m_pcComment)
		iTagDataLength += strlen(pItem->m_pcComment) + 1 + sizeof(CIs_ID3v2Frame); // 1-byte for encoding and a frame header
		
	if (pItem->m_cGenre != CIC_INVALIDGENRE)
		iTagDataLength += strlen(glb_pcGenres[pItem->m_cGenre]) + 1 + sizeof(CIs_ID3v2Frame); // 1-byte for encoding and a frame header
		
	if (pItem->m_cTrackNum != CIC_INVALIDTRACKNUM)
		iTagDataLength += strlen(_itoa(pItem->m_cTrackNum, atiobuffer, 10)) + 1 + sizeof(CIs_ID3v2Frame); // 1-byte for encoding and a frame header
		
	if (pItem->m_iTrackLength != 0)
		iTagDataLength += strlen(_itoa(pItem->m_iTrackLength * 1000, atiobuffer, 10)) + 1 + sizeof(CIs_ID3v2Frame); // 1-byte for encoding and a frame header
		
	// Add ID3v2 overhead
	iTagDataLength += sizeof(CIs_ID3v2Tag);
	
	// Quantise tag to the nearest 1K
	iTagDataLength = ((iTagDataLength >> 10) + 1) << 10;
	
	// Is there a tag big enough in the file
	{
		CIs_ID3v2Tag existingtagheader;
		
		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		ReadFile(hFile, &existingtagheader, sizeof(existingtagheader), &dwBytesTransferred, NULL);
		
		if (memcmp(existingtagheader.m_cTAG, "ID3", 3) == 0)
		{
			iExistingTagLength = (existingtagheader.m_cSize_Encoded[0] << 21)
								 | (existingtagheader.m_cSize_Encoded[1] << 14)
								 | (existingtagheader.m_cSize_Encoded[2] << 7)
								 | existingtagheader.m_cSize_Encoded[3];
			iExistingTagLength += sizeof(CIs_ID3v2Tag); // count the header
			
			if (iExistingTagLength > iTagDataLength)
				iTagDataLength = iExistingTagLength;
		}
		
		else
			iExistingTagLength = 0;
	}
	
	// Do we need to enlarge the file?
	
	if (iExistingTagLength < iTagDataLength)
	{
		if (CPLI_GrowFile(hFile, 0, iTagDataLength - iExistingTagLength) == FALSE)
			return;
	}
	
	// Build tag
	pTag = malloc(iTagDataLength);
	
	memset(pTag, 0, iTagDataLength); // ** must do this as all padding should be 0x00
	
	pTag_Cursor = pTag;
	
	// Header
	{
		CIs_ID3v2Tag* pHeader = (CIs_ID3v2Tag*)pTag_Cursor;
		int iSizeLessHeader = iTagDataLength - sizeof(CIs_ID3v2Tag);
		
		pHeader->m_cTAG[0] = 'I';
		pHeader->m_cTAG[1] = 'D';
		pHeader->m_cTAG[2] = '3';
		
		pHeader->m_cVersion[0] = 0x4;
		pHeader->m_cVersion[1] = 0x0;
		
		pHeader->m_cFlags = 0x0;
		
		CPLI_ID3v2_WriteSyncSafeInt(pHeader->m_cSize_Encoded, iSizeLessHeader);
		pTag_Cursor += sizeof(CIs_ID3v2Tag);
	}
	
	// Frames
	
	if (pItem->m_pcTrackName)
		CPLI_ID3v2_WriteTextFrame(&pTag_Cursor, "TIT2", pItem->m_pcTrackName);
		
	if (pItem->m_pcArtist)
		CPLI_ID3v2_WriteTextFrame(&pTag_Cursor, "TPE1", pItem->m_pcArtist);
		
	if (pItem->m_pcAlbum)
		CPLI_ID3v2_WriteTextFrame(&pTag_Cursor, "TALB", pItem->m_pcAlbum);
		
	if (pItem->m_pcYear)
		CPLI_ID3v2_WriteTextFrame(&pTag_Cursor, "TYER", pItem->m_pcYear);
		
	if (pItem->m_pcComment)
		CPLI_ID3v2_WriteTextFrame(&pTag_Cursor, "TENC", pItem->m_pcComment);
		
	if (pItem->m_cGenre != CIC_INVALIDGENRE)
		CPLI_ID3v2_WriteTextFrame(&pTag_Cursor, "TCON", glb_pcGenres[pItem->m_cGenre]);
		
	if (pItem->m_cTrackNum != CIC_INVALIDTRACKNUM)
		CPLI_ID3v2_WriteTextFrame(&pTag_Cursor, "TRCK", _itoa(pItem->m_cTrackNum, atiobuffer, 10));
		
	if (pItem->m_iTrackLength != 0)
		CPLI_ID3v2_WriteTextFrame(&pTag_Cursor, "TLEN", _itoa(pItem->m_iTrackLength * 1000, atiobuffer, 10));
		
	// Output tag
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	WriteFile(hFile, pTag, iTagDataLength, &dwBytesTransferred, NULL);
	CP_ASSERT(dwBytesTransferred == iTagDataLength);
}

//
//
//
CPe_ReadWriteState CPLI_GetReadWriteState(const CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	HANDLE hFile;
	CP_CHECKOBJECT(pItem);
	
	// We will check this every time (and not cache the result) because the
	// file could have been played with outside of CoolPlayer
	
	// Try to open the file in RW mode
	hFile = CreateFile(pItem->m_pcPath, GENERIC_READ | GENERIC_WRITE,
					   FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
					   OPEN_EXISTING, 0, 0);
	                   
	if (hFile != INVALID_HANDLE_VALUE)
	{
		// Only cache
		CloseHandle(hFile);
		return rwsReadWrite;
	}
	
	// That didn't work - try a RO open
	hFile = CreateFile(pItem->m_pcPath, GENERIC_READ,
					   FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
					   OPEN_EXISTING, 0, 0);
	                   
	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		return rwsReadOnly;
	}
	
	return rwsBadFile;
}

//
//
//
void CPLI_SetArtist(CP_HPLAYLISTITEM hItem, const char* pcNewValue)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	STR_AllocSetString(&pItem->m_pcArtist, pcNewValue, TRUE);
	pItem->m_bID3Tag_SaveRequired = TRUE;
	CPL_cb_OnItemUpdated(hItem);
}

//
//
//
void CPLI_SetAlbum(CP_HPLAYLISTITEM hItem, const char* pcNewValue)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	STR_AllocSetString(&pItem->m_pcAlbum, pcNewValue, TRUE);
	pItem->m_bID3Tag_SaveRequired = TRUE;
	CPL_cb_OnItemUpdated(hItem);
}

//
//
//
void CPLI_SetTrackName(CP_HPLAYLISTITEM hItem, const char* pcNewValue)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	STR_AllocSetString(&pItem->m_pcTrackName, pcNewValue, TRUE);
	pItem->m_bID3Tag_SaveRequired = TRUE;
	CPL_cb_OnItemUpdated(hItem);
}

//
//
//
void CPLI_SetYear(CP_HPLAYLISTITEM hItem, const char* pcNewValue)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	STR_AllocSetString(&pItem->m_pcYear, pcNewValue, TRUE);
	pItem->m_bID3Tag_SaveRequired = TRUE;
	CPL_cb_OnItemUpdated(hItem);
}

//
//
//
void CPLI_SetGenreIDX(CP_HPLAYLISTITEM hItem, const unsigned char iNewValue)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	pItem->m_cGenre = iNewValue;
	pItem->m_bID3Tag_SaveRequired = TRUE;
	CPL_cb_OnItemUpdated(hItem);
}

//
//
//
void CPLI_SetTrackNum(CP_HPLAYLISTITEM hItem, const unsigned char iNewValue)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	char cTempString[33];
	CP_CHECKOBJECT(pItem);
	
	pItem->m_cTrackNum = iNewValue;
	
	if (pItem->m_cTrackNum != CIC_INVALIDTRACKNUM)
	{
		if (pItem->m_pcTrackNum_AsText)
			free(pItem->m_pcTrackNum_AsText);
			
		pItem->m_pcTrackNum_AsText = (char*)malloc(CPC_TRACKNUMASTEXTBUFFERSIZE);
		
		_itoa(pItem->m_cTrackNum, cTempString, 10);
		
		strncpy(pItem->m_pcTrackNum_AsText, cTempString, CPC_TRACKNUMASTEXTBUFFERSIZE);
	}
	
	else
	{
		if (pItem->m_pcTrackNum_AsText)
		{
			free(pItem->m_pcTrackNum_AsText);
			pItem->m_pcTrackNum_AsText = NULL;
		}
	}
	
	pItem->m_bID3Tag_SaveRequired = TRUE;
	
	CPL_cb_OnItemUpdated(hItem);
}

//
//
//
void CPLI_SetTrackNum_AsText(CP_HPLAYLISTITEM hItem, const char* pcNewValue)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	if (pcNewValue[0] == '\0')
		pItem->m_cTrackNum = CIC_INVALIDTRACKNUM;
	else
		pItem->m_cTrackNum = (unsigned char)atoi(pcNewValue);
		
	if (pItem->m_pcTrackNum_AsText)
		free(pItem->m_pcTrackNum_AsText);
		
	pItem->m_pcTrackNum_AsText = (char*)malloc(CPC_TRACKNUMASTEXTBUFFERSIZE);
	strncpy(pItem->m_pcTrackNum_AsText, pcNewValue, CPC_TRACKNUMASTEXTBUFFERSIZE);
	
	pItem->m_bID3Tag_SaveRequired = TRUE;
	
	CPL_cb_OnItemUpdated(hItem);
}

//
//
//
void CPLI_SetComment(CP_HPLAYLISTITEM hItem, const char* pcNewValue)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	STR_AllocSetString(&pItem->m_pcComment, pcNewValue, TRUE);
	pItem->m_bID3Tag_SaveRequired = TRUE;
	CPL_cb_OnItemUpdated(hItem);
}

//
//
//
void CPLI_SetTrackStackPos(CP_HPLAYLISTITEM hItem, const int iNewPos)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	CP_CHECKOBJECT(pItem);
	
	pItem->m_iTrackStackPos = iNewPos;
	
	if (iNewPos == 0)
	{
		pItem->m_cTrackStackPos_AsText[0] = '>';
		pItem->m_cTrackStackPos_AsText[1] = '>';
		pItem->m_cTrackStackPos_AsText[2] = '>';
		pItem->m_cTrackStackPos_AsText[3] = '\0';
	}
	
	else if (iNewPos == CIC_TRACKSTACK_UNSTACKED)
	{
		pItem->m_cTrackStackPos_AsText[0] = '\0';
	}
	
	else
	{
		_snprintf(pItem->m_cTrackStackPos_AsText, sizeof(pItem->m_cTrackStackPos_AsText), "%d", iNewPos);
	}
}

//
//
//
void CPLI_CalculateLength(CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	const char* pcExtension;
	
	CP_CHECKOBJECT(pItem);
	
	pcExtension = CPLI_GetExtension(hItem);
	
	if (stricmp(pcExtension, ".ogg") == 0)
		CPLI_CalculateLength_OGG(pItem);
	else if (stricmp(pcExtension, ".mp3") == 0
			 || stricmp(pcExtension, ".mp2") == 0)
	{
		CPLI_CalculateLength_MP3(pItem);
	}
	else if (stricmp(pcExtension, ".wav") == 0)
	{
		CPLI_CalculateLength_WAV(pItem);
	}
	
//    pItem->m_bID3Tag_SaveRequired = TRUE;
	CPL_cb_OnItemUpdated(hItem);
}

//
//
//
void CPLI_CalculateLength_OGG(CPs_PlaylistItem* pItem)
{
	FILE *hFile;
	OggVorbis_File vorbisfileinfo;
	
	hFile = fopen(pItem->m_pcPath, "rb");
	
	if (hFile == NULL)
		return;
		
	CPLI_OGG_SkipOverTab(hFile);
	
	memset(&vorbisfileinfo, 0, sizeof(vorbisfileinfo));
	
	if (ov_open(hFile, &vorbisfileinfo, NULL, 0) < 0)
	{
		fclose(hFile);
		return;
	}
	
	CPLI_DecodeLength(pItem, (int)ov_time_total(&vorbisfileinfo, -1));
	
	ov_clear(&vorbisfileinfo);
	fclose(hFile);
}

//
//
//
/** // TODO: - reformat and check - move all this to separate function **/
// at the moment CPI_Player_CoDec_WAV has this same code
void CPLI_CalculateLength_WAV(CPs_PlaylistItem* pItem)
{
	
	FILE* hFile;
	DWORD bps;
	extern BOOL SkipToChunk(HANDLE hFile, CPs_RIFFChunk* pChunk, const char cChunkID[4]); // in CoDec_WAV.c
	CP_TRACE1("Openfile \"%s\"", pItem->m_pcPath);
	
	hFile = CreateFile(pItem->m_pcPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
	
	if (hFile == INVALID_HANDLE_VALUE)
	{
		CP_TRACE0("Failed to open file");
		return;
	}
	
	// Skip over ID3v2 tag (if there is one)
	{
		CIs_ID3v2Tag tag;
		DWORD dwBytesRead;
		int iStreamStart = 0;
		
		memset(&tag, 0, sizeof(tag));
		ReadFile(hFile, &tag, sizeof(tag), &dwBytesRead, NULL);
		
		if (memcmp(tag.m_cTAG, "ID3", 3) == 0)
		{
			iStreamStart = sizeof(CIs_ID3v2Tag);
			iStreamStart += (tag.m_cSize_Encoded[0] << 21)
							| (tag.m_cSize_Encoded[1] << 14)
							| (tag.m_cSize_Encoded[2] << 7)
							| tag.m_cSize_Encoded[3];
		}
		
		SetFilePointer(hFile, iStreamStart, NULL, FILE_BEGIN);
	}
	
	// Check the header
	{
		CPs_RIFFHeader RIFFHeader;
		DWORD dwBytesRead;
		ReadFile(hFile, &RIFFHeader, sizeof(RIFFHeader), &dwBytesRead, NULL);
		
		if (memcmp(RIFFHeader.m_cID, "RIFF", 4) || memcmp(RIFFHeader.m_cFileType, "WAVE", 4))
		{
			CP_TRACE0("File not of RIFF WAVE type");
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
			return;
		}
	}
	
	// Check the format of the WAV file
	{
		CPs_RIFFChunk chunk;
		PCMWAVEFORMAT* pFormat;
		DWORD dwBytesRead;
		BOOL bSuccess = SkipToChunk(hFile, &chunk, "fmt ");
		
		if (bSuccess == FALSE || chunk.m_dwLength < sizeof(PCMWAVEFORMAT))
		{
			CP_TRACE0("Failed to find FMT chunk");
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
			return;
		}
		
		// Get the format data
		pFormat = (PCMWAVEFORMAT*)malloc(chunk.m_dwLength);
		
		ReadFile(hFile, pFormat, chunk.m_dwLength, &dwBytesRead, NULL);
		
		// We only handle PCM encoded data
		if (dwBytesRead != chunk.m_dwLength || pFormat->wf.wFormatTag != WAVE_FORMAT_PCM)
		{
			CP_TRACE0("Only PCM data supported!");
			free(pFormat);
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
			return;
		}
		
		// Setup file info struct - this is re-read every second
		bps = (pFormat->wBitsPerSample * pFormat->wf.nChannels * pFormat->wf.nSamplesPerSec) / 8;
		
		free(pFormat);
	}
	
	// Dip into the DATA chunk
	{
		CPs_RIFFChunk chunk;
		int iFileLength_Secs = 0;
		BOOL bSuccess = SkipToChunk(hFile, &chunk, "data");
		int iLengthOfWavData, iStartOfWavData = 0;
		
		if (bSuccess == FALSE)
		{
			CP_TRACE0("Failed to find WAVE chunk");
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
			return;
		}
		
		// Get info about the length of this chunk
		iLengthOfWavData = chunk.m_dwLength;
		iStartOfWavData = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
		iFileLength_Secs = iLengthOfWavData / bps; //iBytesPerSecond;
		
		CPLI_DecodeLength(pItem, iFileLength_Secs);
		
		return;
	}
	
}

//
//
//
void CPLI_CalculateLength_MP3(CPs_PlaylistItem* pItem)
{
	BYTE pbBuffer[0x8000];
	unsigned int iBufferCursor;
	DWORD dwBufferSize;
	HANDLE hFile;
	BOOL bFoundFrameHeader;
	int iBitRate;
	DWORD dwFileSize;
	int iMPEG_version;
	int iLayer;
	BOOL bMono;
	unsigned int iVBRHeader;
	
	// - Try to open the file
	hFile = CreateFile(pItem->m_pcPath, GENERIC_READ,
					   FILE_SHARE_READ, 0,
					   OPEN_EXISTING, 0, 0);
	dwFileSize = GetFileSize(hFile, NULL);
	
	// Cannot open - fail silently
	
	if (hFile == INVALID_HANDLE_VALUE)
		return;
		
	// Read the first 64K of the file (that should contain the first frame header!)
	ReadFile(hFile, pbBuffer, sizeof(pbBuffer), &dwBufferSize, NULL);
	
	CloseHandle(hFile);
	
	iBufferCursor = 0;
	
	// Skip over a any ID3v2 tag
	{
		CIs_ID3v2Tag* pHeader = (CIs_ID3v2Tag*)(pbBuffer + iBufferCursor);
		
		if (memcmp(pHeader->m_cTAG, "ID3", 3) == 0)
		{
			iBufferCursor += (pHeader->m_cSize_Encoded[0] << 21)
							 | (pHeader->m_cSize_Encoded[1] << 14)
							 | (pHeader->m_cSize_Encoded[2] << 7)
							 | pHeader->m_cSize_Encoded[3];
			iBufferCursor += sizeof(CIs_ID3v2Tag); // count the header
		}
	}
	
	// Seek to the start of the first frame
	bFoundFrameHeader = FALSE;
	
	while (iBufferCursor < (dwBufferSize - 4))
	{
		if (pbBuffer[iBufferCursor] == 0xFF
				&& (pbBuffer[iBufferCursor+1] & 0xE0) == 0xE0)
		{
			bFoundFrameHeader = TRUE;
			break;
		}
		
		iBufferCursor++;
	}
	
	if (bFoundFrameHeader == FALSE)
		return;
		
	// Work out MPEG version
	if (((pbBuffer[iBufferCursor+1] >> 3) & 0x3) == 0x3)
		iMPEG_version = 1;
	else
		iMPEG_version = 2;
		
	// Work out layer
	iLayer = 0x4 - ((pbBuffer[iBufferCursor+1] >> 1) & 0x3);
	
	if (iLayer == 0)
		return;
		
	// Work out stereo
	if ((pbBuffer[iBufferCursor+3] >> 6) == 0x3)
		bMono = TRUE;
	else
		bMono = FALSE;
		
	// Work out the VBR header should be
	if (iMPEG_version == 1)
		iVBRHeader = (iBufferCursor + 4) + (bMono ? 17 : 32);
	else
		iVBRHeader = (iBufferCursor + 4) + (bMono ? 9 : 17);
		
		
	// Is this a VBR file
	if ((iBufferCursor + iVBRHeader + 12) < dwBufferSize
			&& pbBuffer[iVBRHeader] == 'X'
			&& pbBuffer[iVBRHeader+1] == 'i'
			&& pbBuffer[iVBRHeader+2] == 'n'
			&& pbBuffer[iVBRHeader+3] == 'g')
	{
		int iNumberOfFrames;
		int iFreq;
		int iDetailedVersion;
		const int aryFrequencies[3][3] =
		{
			{44100, 48000, 32000}, //MPEG 1
			{22050, 24000, 16000}, //MPEG 2
			{32000, 16000,  8000}  //MPEG 2.5
		};
		
		if (((pbBuffer[iBufferCursor+1] >> 3) & 0x3) == 0x3)
			iDetailedVersion = 1;
		else if (((pbBuffer[iBufferCursor+1] >> 3) & 0x3) == 0x2)
			iDetailedVersion = 2;
		else
			iDetailedVersion = 3;
			
		// Get the number of frames from the Xing header
		iNumberOfFrames = (pbBuffer[iVBRHeader+8] << 24)
						  | (pbBuffer[iVBRHeader+9] << 16)
						  | (pbBuffer[iVBRHeader+10] << 8)
						  | pbBuffer[iVBRHeader+11];
		                  
		if (((pbBuffer[iBufferCursor+2] >> 2) & 0x3) == 0x3)
			return;
			
		iFreq = aryFrequencies[iDetailedVersion-1][(pbBuffer[iBufferCursor+2] >> 2) & 0x3];
		
		CPLI_DecodeLength(pItem, (8 * iNumberOfFrames * 144) / iFreq);
	}
	
	// Work out the bit rate for a CBR file
	
	else
	{
		const int aryBitRates[2][3][16] =
		{
			{         //MPEG 2 & 2.5
				{0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0}, //Layer I
				{0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0}, //Layer II
				{0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0}  //Layer III
			}, {      //MPEG 1
				{0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0}, //Layer I
				{0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 0}, //Layer II
				{0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0}  //Layer III
			}
		};
		
		iBitRate = aryBitRates[2-iMPEG_version][iLayer-1][pbBuffer[iBufferCursor+2] >> 4];
		
		if (iBitRate)
			CPLI_DecodeLength(pItem, (dwFileSize*8) / (iBitRate*1000));
	}
}

//
//
//
BOOL CPLI_RenameTrack(CP_HPLAYLISTITEM hItem, const CPe_FilenameFormat enFormat)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	char cPath[MAX_PATH];
	char cNewPath[MAX_PATH];
	BOOL bMoved;
	const char* pcExtension;
	
	CP_CHECKOBJECT(pItem);
	
	strncpy(cPath, pItem->m_pcPath, MAX_PATH);
	
	// Remove the filename from the path
	{
		int iLastSlashIDX, iCharIDX;
		iLastSlashIDX = CPC_INVALIDCHAR;
		
		for (iCharIDX = 0; cPath[iCharIDX]; iCharIDX++)
		{
			if (cPath[iCharIDX] == '\\')
				iLastSlashIDX = iCharIDX;
		}
		
		if (iLastSlashIDX != CPC_INVALIDCHAR)
			cPath[iLastSlashIDX] = '\0';
	}
	
	pcExtension = CPLI_GetExtension(hItem);
	
	// Apply the name format
	{
		char cNewFilename[MAX_PATH];
		const char* pcTitle;
		const char* pcArtist;
		const char* pcAlbum;
		
		if (pItem->m_pcTrackName)
			pcTitle = pItem->m_pcTrackName;
		else
			pcTitle = "<title>";
			
		if (pItem->m_pcArtist)
			pcArtist = pItem->m_pcArtist;
		else
			pcArtist = "<title>";
			
		if (pItem->m_pcAlbum)
			pcAlbum = pItem->m_pcAlbum;
		else
			pcAlbum = "<album>";
			
			
		switch (enFormat)
		{
		
			case rwsArtistAlbumNumberTitle:
				sprintf(cNewFilename, "%s - %s - %02d - %s%s", pcArtist, pcAlbum, (int)pItem->m_cTrackNum, pcTitle, pcExtension);
				break;
				
			case rwsArtistNumberTitle:
				sprintf(cNewFilename, "%s - %02d - %s%s", pcArtist, (int)pItem->m_cTrackNum, pcTitle, pcExtension);
				break;
				
			case rwsAlbumNumberTitle:
				sprintf(cNewFilename, "%s - %02d - %s%s", pcAlbum, (int)pItem->m_cTrackNum, pcTitle, pcExtension);
				break;
				
			case rwsAlbumNumber:
				sprintf(cNewFilename, "%s - %02d%s", pcAlbum, (int)pItem->m_cTrackNum, pcExtension);
				break;
				
			case rwsNumberTitle:
				sprintf(cNewFilename, "%02d - %s%s", (int)pItem->m_cTrackNum, pcTitle, pcExtension);
				break;
				
			case rwsTitle:
				sprintf(cNewFilename, "%s%s", pcTitle, pcExtension);
				break;
				
			default:
				CP_FAIL("Unknown rename format");
		}
		
		// Replace illegal chars with _
		{
			int iCharIDX;
			
			for (iCharIDX = 0; cNewFilename[iCharIDX]; iCharIDX++)
			{
				if (cNewFilename[iCharIDX] == '\\'
						|| cNewFilename[iCharIDX] == '/'
						|| cNewFilename[iCharIDX] == ':'
						|| cNewFilename[iCharIDX] == '"')
				{
					cNewFilename[iCharIDX] = '_';
				}
			}
		}
		
		sprintf(cNewPath, "%s\\%s", cPath, cNewFilename);
	}
	
	CP_TRACE2("Rename \"%s\" to \"%s\"", pItem->m_pcPath, cNewPath);
	bMoved = MoveFile(pItem->m_pcPath, cNewPath);
	
	if (bMoved)
	{
		CPLI_SetPath(pItem, cNewPath);
		
		// Update interface
		CPL_cb_OnItemUpdated(hItem);
	}
	
	return bMoved;
}

//
//
//
void CPLI_SetPath(CPs_PlaylistItem* pItem, const char* pcPath)
{
	int iCharIDX;
	char cFullPath[MAX_PATH];
	
	// uPathSize is the number of bytes in the path, including the ending NULL.  This is calculated by our allocation
	// routine, so we profit from the already spent CPU cycles to speed up our filename calculation.
	unsigned int uPathSize;
	
	if (pItem->m_pcPath)
		free(pItem->m_pcPath);
		
		
	// Store the full path to the file if this isn't a stream
	if (_strnicmp(CIC_HTTPHEADER, pcPath, 5) != 0
			&& _strnicmp(CIC_HTTPSHEADER, pcPath, 6) != 0
			&& _strnicmp(CIC_FTPHEADER, pcPath, 4) != 0)
	{
		_fullpath(cFullPath, pcPath, MAX_PATH);
		uPathSize = STR_AllocSetString(&pItem->m_pcPath, cFullPath, FALSE);
	}
	
	else
		uPathSize = STR_AllocSetString(&pItem->m_pcPath, pcPath, FALSE);
		
	if (1 >= uPathSize)
	{
		// We haven't any memory allocated, or an empty string, so there is no need to look for the filename
		pItem->m_pcFilename = pItem->m_pcPath;
		return;
	}
	
	// Get the filename (the string following the last slash).  Since we have our string size from the allocation
	// routine, we start from the end to limit the amount of work we do.
	
	for (iCharIDX = uPathSize - 2; iCharIDX >= 0; iCharIDX--)
	{
		if ((pItem->m_pcPath[iCharIDX] == '\\') || (pItem->m_pcPath[iCharIDX] == '/'))
		{
			pItem->m_pcFilename = pItem->m_pcPath + iCharIDX + 1;
			return;
		}
	}
	
	// There is no slash in the path, so the file must be local, and is the same as the path.
	pItem->m_pcFilename = pItem->m_pcPath;
}

//
//
//
const char* CPLI_GetExtension(const CP_HPLAYLISTITEM hItem)
{
	CPs_PlaylistItem* pItem = (CPs_PlaylistItem*)hItem;
	int iCharIDX;
	const char* pcLastDot;
	
	CP_CHECKOBJECT(pItem);
	
	pcLastDot = NULL;
	
	for (iCharIDX = 0; pItem->m_pcPath[iCharIDX]; iCharIDX++)
	{
		if (pItem->m_pcPath[iCharIDX] == '.')
			pcLastDot = pItem->m_pcPath + iCharIDX;
			
		// If there is a directory name with a dot in it we don't want that!
		else if (pItem->m_pcPath[iCharIDX] == '\\')
			pcLastDot = NULL;
	}
	
	// Ensure the string is valid
	
	if (!pcLastDot)
		pcLastDot = "";
		
	return pcLastDot;
}

//
//
//
void CPLI_OGG_SkipOverTab(FILE* pFile)
{
	CIs_ID3v2Tag tag;
	int iStreamStart = 0;
	
	memset(&tag, 0, sizeof(tag));
	fread(&tag, sizeof(tag), 1, pFile);
	
	if (memcmp(tag.m_cTAG, "ID3", 3) == 0)
	{
		iStreamStart = sizeof(CIs_ID3v2Tag);
		iStreamStart += (tag.m_cSize_Encoded[0] << 21)
						| (tag.m_cSize_Encoded[1] << 14)
						| (tag.m_cSize_Encoded[2] << 7)
						| tag.m_cSize_Encoded[3];
	}
	
	fseek(pFile, iStreamStart, SEEK_SET);
}

//
//
//
void CPLI_OGG_DecodeString(char** ppcString, const char* pcNewValue)
{
	int iStringLength;
	
	if (*ppcString)
		free(*ppcString);
		
	iStringLength = strlen(pcNewValue);
	
	*ppcString = malloc(iStringLength + 1);
	
	memcpy(*ppcString, pcNewValue, iStringLength + 1);
}

//
//
//
void CPLI_ReadTag_OGG(CPs_PlaylistItem* pItem)
{
	FILE *hFile;
	OggVorbis_File vorbisfileinfo;
	vorbis_comment* pComment;
	
	hFile = fopen(pItem->m_pcPath, "rb");
	
	if (hFile == NULL)
		return;
		
	CPLI_OGG_SkipOverTab(hFile);
	
	memset(&vorbisfileinfo, 0, sizeof(vorbisfileinfo));
	
	if (ov_open(hFile, &vorbisfileinfo, NULL, 0) < 0)
	{
		fclose(hFile);
		return;
	}
	
	// While we have the file open - we may as well get the length
	CPLI_DecodeLength(pItem, (int)ov_time_total(&vorbisfileinfo, -1));
	
	pComment = ov_comment(&vorbisfileinfo, -1);
	
	if (pComment)
	{
		int iCommentIDX;
		
		for (iCommentIDX = 0; iCommentIDX < pComment->comments; iCommentIDX++)
		{
			char* cTag = malloc(pComment->comment_lengths[iCommentIDX]+8);
			char* cValue = malloc(pComment->comment_lengths[iCommentIDX]+8);

			// find "=" character to parse tag and value data		
			{
				int i = 0;
				int equals_pos = 0;
				char* comment = pComment->user_comments[iCommentIDX];

				while (i < pComment->comment_lengths[iCommentIDX])
				{
					if (comment[i] == '=')
					{
						equals_pos = i;
						break;
					}

					i++;
				}

				if (equals_pos)
				{
					strncpy(cTag, comment, equals_pos+1);
					strncpy(cValue, comment+equals_pos+1, pComment->comment_lengths[iCommentIDX] - equals_pos);
				}
				else
					goto bottom_loop;
			}

			// SECURITY: rewritten due to exploit at
			// http://www.frsirt.com/english/advisories/2008/0008
			// original code used following commented line
            //if(sscanf(pComment->user_comments[iCommentIDX], " %[^= ] = %[^=]", cTag, cValue) == 2)
			{
				if (stricmp(cTag, "TITLE") == 0)
					CPLI_OGG_DecodeString(&pItem->m_pcTrackName, cValue);
				else if (stricmp(cTag, "ARTIST") == 0)
					CPLI_OGG_DecodeString(&pItem->m_pcArtist, cValue);
				else if (stricmp(cTag, "ALBUM") == 0)
					CPLI_OGG_DecodeString(&pItem->m_pcAlbum, cValue);
				else if (stricmp(cTag, "TRACKNUMBER") == 0)
				{
					CPLI_OGG_DecodeString(&pItem->m_pcTrackNum_AsText, cValue);
					pItem->m_cTrackNum = (unsigned char)atoi(pItem->m_pcTrackNum_AsText);
				}
				
				else if (stricmp(cTag, "GENRE") == 0)
				{
					// Search for this genre among the ID3v1 genres (don't read it if we cannot find it)
					int iGenreIDX;
					
					for (iGenreIDX = 0; iGenreIDX < CIC_NUMGENRES; iGenreIDX++)
					{
						if (stricmp(cValue, glb_pcGenres[iGenreIDX]) == 0)
						{
							pItem->m_cGenre = (unsigned char)iGenreIDX;
							break;
						}
					}
				}
			}

bottom_loop:
			free(cTag);
			free(cValue);

		} // end for loop
	}
	
	ov_clear(&vorbisfileinfo);
	
	fclose(hFile);
}

//
//
//
BOOL CPLI_OGG_SyncToNextFrame(HANDLE hFile)
{
	char pcBuffer[1024];
	DWORD dwBytesTransferred;
	int iCharCursor;
	int iChunkSize;
	int iPassIDX;
	
	dwBytesTransferred = 0;
	
	for (iPassIDX = 0; iPassIDX < 64; iPassIDX++)
	{
		if (iPassIDX == 0)
		{
			ReadFile(hFile, pcBuffer, 1024, &dwBytesTransferred, NULL);
			iChunkSize = dwBytesTransferred;
		}
		
		else
		{
			memcpy(pcBuffer, pcBuffer + (dwBytesTransferred - 4), 4);
			ReadFile(hFile, pcBuffer + 4, 1020, &dwBytesTransferred, NULL);
			iChunkSize = dwBytesTransferred + 4;
		}
		
		for (iCharCursor = 0; iCharCursor < (iChunkSize - 4); iCharCursor++)
		{
			if (pcBuffer[iCharCursor] == 'O'
					&& pcBuffer[iCharCursor+1] == 'g'
					&& pcBuffer[iCharCursor+2] == 'g'
					&& pcBuffer[iCharCursor+3] == 'S')
			{
				int iOffset = dwBytesTransferred - iCharCursor;
				SetFilePointer(hFile, -iOffset, NULL, FILE_CURRENT);
				return TRUE;
			}
		}
	}
	
	return FALSE;
}

//
//
//
void CPLI_OGG_UpdateCommentString(CIs_OGGComment* pComment, const char* pcTag, const char* _pcTagValue)
{
	unsigned int iCommentIDX;
	char* pcCommentBuffer;
	int iTagLength;
	int iValueLength;
	int iCommentBufferLength;
	const char* pcTagValue;
	
	if (_pcTagValue)
		pcTagValue = _pcTagValue;
	else
		pcTagValue = "";
		
	// Build the comment
	iTagLength = strlen(pcTag);
	iValueLength = strlen(pcTagValue);
	iCommentBufferLength = iTagLength + 1 + iValueLength + 1; // 1 for = sign and 1 for terminating null
	
	pcCommentBuffer = (char*)malloc(iCommentBufferLength);
	memcpy(pcCommentBuffer, pcTag, iTagLength);
	pcCommentBuffer[iTagLength] = '=';
	memcpy(pcCommentBuffer + iTagLength + 1, pcTagValue, iValueLength);
	pcCommentBuffer[iTagLength + 1 + iValueLength] = '\0';
	
	// Search for the tag
	for (iCommentIDX = 0; iCommentIDX < pComment->m_iNumComments; iCommentIDX++)
	{
		char cTag[128];
		
		if (sscanf(pComment->m_ppUserStrings[iCommentIDX], " %[^= ] =", cTag) == 1
				&& stricmp(cTag, pcTag) == 0)
		{
			free(pComment->m_ppUserStrings[iCommentIDX]);
			pComment->m_ppUserStrings[iCommentIDX] = pcCommentBuffer;
			return;
		}
	}
	
	// No existing tag - we need to append one
	pComment->m_iNumComments++;
	pComment->m_ppUserStrings = (char**)realloc(pComment->m_ppUserStrings, pComment->m_iNumComments * sizeof(char*));
	pComment->m_ppUserStrings[pComment->m_iNumComments-1] = pcCommentBuffer;
}

//
//
//
void CPLI_WriteTag_OGG(CPs_PlaylistItem* pItem, HANDLE hFile)
{
	CIs_OGGFrame info_comment;
	DWORD dwBytesTransferred;
	int iCommentFrameLength;
	int iRequiredFrameLength;
	CIs_OGGComment comment;
	DWORD dwCommentStartOffset;
	BYTE* pCodeBook;
	unsigned int iCodeBookLength;
	
	// Remove ID3v1 tag from file
	{
		CIs_ID3Tag ID3;
		
		SetFilePointer(hFile, 0 - sizeof(ID3), NULL, FILE_END);
		ReadFile(hFile, &ID3, sizeof(ID3), &dwBytesTransferred, NULL);
		
		// Is there a tag? - remove it
		
		if (dwBytesTransferred == sizeof(ID3) && memcmp(ID3.m_cTAG, "TAG", 3) == 0)
		{
			SetFilePointer(hFile, 0 - sizeof(ID3), NULL, FILE_END);
			SetEndOfFile(hFile);
		}
	}
	
	// Remove ID3v2 tag from file
	{
		CIs_ID3v2Tag tag;
		unsigned int iStreamOffset = 0;
		
		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		ReadFile(hFile, &tag, sizeof(tag), &dwBytesTransferred, NULL);
		
		if (dwBytesTransferred == sizeof(tag) && memcmp(tag.m_cTAG, "ID3", 3) == 0)
		{
			iStreamOffset = sizeof(CIs_ID3v2Tag);
			iStreamOffset += (tag.m_cSize_Encoded[0] << 21)
							 | (tag.m_cSize_Encoded[1] << 14)
							 | (tag.m_cSize_Encoded[2] << 7)
							 | tag.m_cSize_Encoded[3];
		}
		
		if (iStreamOffset > 0)
			CPLI_ShrinkFile(hFile, 0L, iStreamOffset);
	}
	
	// Sync to first frame
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	
	if (CPLI_OGG_SyncToNextFrame(hFile) == FALSE)
		return;
		
	// Step over the first frame (which is the vorbis info)
	{
		CIs_OGGFrame info;
		int iFrameLength;
		int iSegmentIDX;
		
		ReadFile(hFile, &info.m_Header, sizeof(info.m_Header), &dwBytesTransferred, NULL);
		
		if (dwBytesTransferred != sizeof(info.m_Header))
			return;
			
		ReadFile(hFile, info.m_cSegments, sizeof(info.m_cSegments[0]) * info.m_Header.m_cNumSegments, &dwBytesTransferred, NULL);
		
		if (dwBytesTransferred != (sizeof(info.m_cSegments[0]) * info.m_Header.m_cNumSegments))
			return;
			
		// Work out how much more data in this frame - we are skipping it
		iFrameLength = 0;
		
		for (iSegmentIDX = 0; iSegmentIDX < info.m_Header.m_cNumSegments; iSegmentIDX++)
			iFrameLength += info.m_cSegments[iSegmentIDX];
			
		SetFilePointer(hFile, iFrameLength, NULL, FILE_CURRENT);
	}
	
	// Seek to next frame
	
	if (CPLI_OGG_SyncToNextFrame(hFile) == FALSE)
		return;
		
	dwCommentStartOffset = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
	
	// Get the length of the next frame (and set the file pointer to it's end)
	{
		int iSegmentIDX;
		BYTE* pCommentPacket;
		int iPacketByteIDX;
		int iSegment2PacketStart = 0;
		
		ReadFile(hFile, &info_comment.m_Header, sizeof(info_comment.m_Header), &dwBytesTransferred, NULL);
		
		if (dwBytesTransferred != sizeof(info_comment.m_Header))
			return;
			
		ReadFile(hFile, info_comment.m_cSegments, sizeof(info_comment.m_cSegments[0]) * info_comment.m_Header.m_cNumSegments, &dwBytesTransferred, NULL);
		
		if (dwBytesTransferred != (sizeof(info_comment.m_cSegments[0]) * info_comment.m_Header.m_cNumSegments))
			return;
			
		// Read in the first packet
		// - work out length first
		iCommentFrameLength = 0;
		
		for (iSegmentIDX = 0; iSegmentIDX < info_comment.m_Header.m_cNumSegments; iSegmentIDX++)
		{
			iCommentFrameLength += info_comment.m_cSegments[iSegmentIDX];
			
			if (info_comment.m_cSegments[iSegmentIDX] != 0xFF)
			{
				iSegment2PacketStart = iSegmentIDX + 1;
				break;
			}
		}
		
		// - read packet
		pCommentPacket = (BYTE*)malloc(iCommentFrameLength);
		
		ReadFile(hFile, pCommentPacket, iCommentFrameLength, &dwBytesTransferred, NULL);
		
		if ((int)dwBytesTransferred != iCommentFrameLength || iCommentFrameLength < 8)
		{
			free(pCommentPacket);
			return;
		}
		
		// Read the packet following the header (the codebook)
		iCodeBookLength = 0;
		
		for (iSegmentIDX = iSegment2PacketStart; iSegmentIDX < info_comment.m_Header.m_cNumSegments; iSegmentIDX++)
		{
			iCodeBookLength += info_comment.m_cSegments[iSegmentIDX];
			
			if (info_comment.m_cSegments[iSegmentIDX] != 0xFF)
				break;
		}
		
		pCodeBook = (BYTE*)malloc(iCodeBookLength);
		
		ReadFile(hFile, pCodeBook, iCodeBookLength, &dwBytesTransferred, NULL);
		
		if ((int)dwBytesTransferred != iCodeBookLength || iCodeBookLength < 8)
		{
			free(pCodeBook);
			free(pCommentPacket);
			return;
		}
		
		// Read the vorbis header from the comment packet (and ensure that this is a type 0x3 packet)
		
		if (pCommentPacket[0] != 3 || memcmp(pCommentPacket + 1, "vorbis", 6) != 0)
		{
			free(pCodeBook);
			free(pCommentPacket);
			return;
		}
		
		// Decode packet
		iPacketByteIDX = 7;
		
		// Decode strings
		{
			unsigned int iStringLength;
			unsigned int iCommentIDX;
			
			iStringLength = *(unsigned int*)(pCommentPacket + iPacketByteIDX);
			iPacketByteIDX += 4;
			comment.m_pcVendorString = (char*)malloc(iStringLength + 1);
			memcpy(comment.m_pcVendorString, pCommentPacket + iPacketByteIDX, iStringLength);
			iPacketByteIDX += iStringLength;
			comment.m_pcVendorString[iStringLength] = '\0';
			
			// Read in the comments
			comment.m_iNumComments = *(unsigned int*)(pCommentPacket + iPacketByteIDX);
			iPacketByteIDX += 4;
			
			if (comment.m_iNumComments)
			{
				comment.m_ppUserStrings = (char**)malloc(sizeof(char*) * comment.m_iNumComments);
				
				for (iCommentIDX = 0; iCommentIDX < comment.m_iNumComments; iCommentIDX++)
				{
					iStringLength = *(unsigned int*)(pCommentPacket + iPacketByteIDX);
					iPacketByteIDX += 4;
					comment.m_ppUserStrings[iCommentIDX] = (char*)malloc(iStringLength + 1);
					memcpy(comment.m_ppUserStrings[iCommentIDX], pCommentPacket + iPacketByteIDX, iStringLength);
					iPacketByteIDX += iStringLength;
					comment.m_ppUserStrings[iCommentIDX][iStringLength] = '\0';
				}
			}
			
			else
				comment.m_ppUserStrings = NULL;
				
		}
		
		free(pCommentPacket);
	}
	
	// Update the comment struct
	{
		CPLI_OGG_UpdateCommentString(&comment, "TITLE", pItem->m_pcTrackName);
		CPLI_OGG_UpdateCommentString(&comment, "ARTIST", pItem->m_pcArtist);
		CPLI_OGG_UpdateCommentString(&comment, "ALBUM", pItem->m_pcAlbum);
		CPLI_OGG_UpdateCommentString(&comment, "TRACKNUMBER", pItem->m_pcTrackNum_AsText);
		
		if (pItem->m_cGenre != CIC_INVALIDGENRE)
			CPLI_OGG_UpdateCommentString(&comment, "GENRE", glb_pcGenres[pItem->m_cGenre]);
		else
			CPLI_OGG_UpdateCommentString(&comment, "GENRE", "");
	}
	
	// Work out the size of the packet
	{
		unsigned int iCommentIDX;
		
		iRequiredFrameLength = 0;
		
		iRequiredFrameLength += 7;  // vorbis packet header
		iRequiredFrameLength += 4 + strlen(comment.m_pcVendorString);
		
		iRequiredFrameLength += 4;  // number of comments
		
		for (iCommentIDX = 0; iCommentIDX < comment.m_iNumComments; iCommentIDX++)
			iRequiredFrameLength += 4 + strlen(comment.m_ppUserStrings[iCommentIDX]);
			
		iRequiredFrameLength++;  // framing bit (byte??)
	}
	
	// Resize file to accomodate new data
	
	if (iRequiredFrameLength > iCommentFrameLength)
		CPLI_GrowFile(hFile, dwCommentStartOffset, iRequiredFrameLength - iCommentFrameLength);
	else if (iRequiredFrameLength < iCommentFrameLength)
		CPLI_ShrinkFile(hFile, dwCommentStartOffset, iCommentFrameLength - iRequiredFrameLength);
		
	// Pack data into block
	{
		BYTE* pFrame;
		int iFrameAndHeaderLength;
		int iChunkBytes;
		int iWriteCursor;
		int iChunkSizeRemaining;
		int iLacingBytesWritten;
		
		// Work out frame length - including the header
		iFrameAndHeaderLength = sizeof(info_comment.m_Header);
		iChunkBytes = (iRequiredFrameLength >> 8) + 1; // comment packet
		iChunkBytes += (iCodeBookLength >> 8) + 1; // codebook packet
		
		iFrameAndHeaderLength += iChunkBytes;
		iFrameAndHeaderLength += iRequiredFrameLength;  // actual frame data (comment)
		iFrameAndHeaderLength += iCodeBookLength;    // actual frame data (codebook)
		
		pFrame = (BYTE*)malloc(iFrameAndHeaderLength);
		
		// Write the header
		info_comment.m_Header.m_cNumSegments = iChunkBytes;
		iWriteCursor = 0;
		info_comment.m_Header.m_cPageCheckum[0] = 0x0;
		info_comment.m_Header.m_cPageCheckum[1] = 0x0;
		info_comment.m_Header.m_cPageCheckum[2] = 0x0;
		info_comment.m_Header.m_cPageCheckum[3] = 0x0;
		memcpy(pFrame, &info_comment.m_Header, sizeof(info_comment.m_Header));
		iWriteCursor += sizeof(info_comment.m_Header);
		
		// Write the chunk bytes for the primary packet lacing
		iLacingBytesWritten = 0;
		
		for (iChunkSizeRemaining = iRequiredFrameLength; iChunkSizeRemaining > 0;)
		{
			int iThisChunkByte;
			
			if (iChunkSizeRemaining > 0xFF)
				iThisChunkByte = 0xFF;
			else
				iThisChunkByte = iChunkSizeRemaining;
				
			pFrame[iWriteCursor++] = iThisChunkByte;
			iChunkSizeRemaining -= iThisChunkByte;
			iLacingBytesWritten++;
		}
		
		// Write out secondry packet lacing
		
		for (iChunkSizeRemaining = iCodeBookLength; iChunkSizeRemaining > 0;)
		{
			int iThisChunkByte;
			
			if (iChunkSizeRemaining > 0xFF)
				iThisChunkByte = 0xFF;
			else
				iThisChunkByte = iChunkSizeRemaining;
				
			pFrame[iWriteCursor++] = iThisChunkByte;
			iChunkSizeRemaining -= iThisChunkByte;
			iLacingBytesWritten++;
		}
		
		// Write out the primary packet data
		{
			unsigned int iStringLength;
			unsigned int iCommentIDX;
			
			// Voribs header
			pFrame[iWriteCursor++] = 0x3;
			memcpy(pFrame + iWriteCursor, "vorbis", 6);
			iWriteCursor += 6;
			
			// Vendor string
			iStringLength = strlen(comment.m_pcVendorString);
			*(unsigned int*)(pFrame + iWriteCursor) = iStringLength;
			iWriteCursor += 4;
			memcpy(pFrame + iWriteCursor, comment.m_pcVendorString, iStringLength);
			iWriteCursor += iStringLength;
			
			// Comments
			*(unsigned int*)(pFrame + iWriteCursor) = comment.m_iNumComments;
			iWriteCursor += 4;
			
			for (iCommentIDX = 0; iCommentIDX < comment.m_iNumComments; iCommentIDX++)
			{
				iStringLength = strlen(comment.m_ppUserStrings[iCommentIDX]);
				*(unsigned int*)(pFrame + iWriteCursor) = iStringLength;
				iWriteCursor += 4;
				memcpy(pFrame + iWriteCursor, comment.m_ppUserStrings[iCommentIDX], iStringLength);
				iWriteCursor += iStringLength;
			}
			
			// Stop bit
			pFrame[iWriteCursor++] = 0xFF;
		}
		
		// Write out secondry packet data
		CP_ASSERT(iWriteCursor + (int)iCodeBookLength == iFrameAndHeaderLength);
		
		memcpy(pFrame + iWriteCursor, pCodeBook, iCodeBookLength);
		
		// Build the CRC
		{
			unsigned int iCRC;
			int iByteIDX;
			extern const ogg_uint32_t crc_lookup[256]; // found in framing.c (ogg)
			
			iCRC = 0;
			
			for (iByteIDX = 0; iByteIDX < iFrameAndHeaderLength; iByteIDX++)
				iCRC = (iCRC << 8) ^ crc_lookup[((iCRC >> 24)&0xff)^pFrame[iByteIDX] ];
				
			// Paste the CRC into the stream
			memset(pFrame + 22, 0, 4);
			pFrame[22] = (BYTE)(iCRC & 0xff);
			pFrame[23] = (BYTE)((iCRC >> 8) & 0xff);
			pFrame[24] = (BYTE)((iCRC >> 16) & 0xff);
			pFrame[25] = (BYTE)((iCRC >> 24) & 0xff);
		}
		
		// Write out the comment
		SetFilePointer(hFile, dwCommentStartOffset, 0, FILE_BEGIN);
		
		WriteFile(hFile, pFrame, iFrameAndHeaderLength, &dwBytesTransferred, NULL);
		
		free(pFrame);
		
		free(pCodeBook);
	}
	
	// Clean up comment struct
	{
		unsigned int iCommentIDX;
		
		for (iCommentIDX = 0; iCommentIDX < comment.m_iNumComments; iCommentIDX++)
			free(comment.m_ppUserStrings[iCommentIDX]);
			
		free(comment.m_ppUserStrings);
		free(comment.m_pcVendorString);
	}
}

//
//
//
void CPLI_ShrinkFile(HANDLE hFile, const DWORD dwStartOffset, const unsigned int iNumBytes)
{
	BYTE pBuffer[0x10000];
	DWORD dwLength;
	DWORD dwBytesTransferred;
	DWORD dwCursor;
	
	CP_TRACE1("Shrunking file by %d bytes", iNumBytes);
	
	dwLength = GetFileSize(hFile, NULL);
	CP_ASSERT((dwStartOffset + iNumBytes) < dwLength);
	dwCursor = dwStartOffset;
	
	while ((dwCursor + iNumBytes) < dwLength)
	{
		unsigned int iChunkSize;
		
		iChunkSize = 0x10000;
		
		if (iChunkSize > dwLength - (dwCursor + iNumBytes))
			iChunkSize = dwLength - (dwCursor + iNumBytes);
			
		SetFilePointer(hFile, dwCursor + iNumBytes, NULL, FILE_BEGIN);
		ReadFile(hFile, pBuffer, iChunkSize, &dwBytesTransferred, NULL);
		
		CP_ASSERT(dwBytesTransferred == iChunkSize);
		
		SetFilePointer(hFile, dwCursor, NULL, FILE_BEGIN);
		WriteFile(hFile, pBuffer, iChunkSize, &dwBytesTransferred, NULL);
		
		dwCursor += iChunkSize;
	}
	
	SetFilePointer(hFile, dwLength - iNumBytes, NULL, FILE_BEGIN);
	SetEndOfFile(hFile);
}

//
//
//
BOOL CPLI_GrowFile(HANDLE hFile, const DWORD dwStartOffset, const unsigned int iNumBytes)
{
	DWORD dwFileSize;
	unsigned int iFileCursor;
	DWORD dwBytesTransferred;
	BYTE* pbReadBlock[0x10000];
	
	dwFileSize = GetFileSize(hFile, NULL);
	CP_TRACE1("Enlarging file by %d bytes", iNumBytes);
	
	// Try to write extra data to end of file - if we fail then clip the file and return
	// (so that we don't corrupt the file in short of space situations)
	{
		BYTE* pbExtra;
		
		pbExtra = (BYTE*)malloc(iNumBytes);
		memset(pbExtra, 0, iNumBytes);
		SetFilePointer(hFile, dwFileSize + iNumBytes, NULL, FILE_BEGIN);
		WriteFile(hFile, pbExtra, iNumBytes, &dwBytesTransferred, NULL);
		
		if (dwBytesTransferred != iNumBytes)
		{
			// Failed - clip file again and abort tag write
			SetFilePointer(hFile, dwFileSize, NULL, FILE_BEGIN);
			SetEndOfFile(hFile);
			return FALSE;
		}
	}
	
	// Enlarge tag
	iFileCursor = dwFileSize;
	
	while (iFileCursor > dwStartOffset)
	{
		unsigned int iBlockSize;
		
		iBlockSize = 0x10000;
		
		if ((iFileCursor - dwStartOffset) < iBlockSize)
			iBlockSize = iFileCursor - dwStartOffset;
			
		// Read a chunk
		SetFilePointer(hFile, iFileCursor - iBlockSize, NULL, FILE_BEGIN);
		ReadFile(hFile, pbReadBlock, iBlockSize, &dwBytesTransferred, NULL);
		
		CP_ASSERT(dwBytesTransferred == iBlockSize);
		
		// Write chunk at offsetted position
		SetFilePointer(hFile, iFileCursor - iBlockSize + iNumBytes, NULL, FILE_BEGIN);
		WriteFile(hFile, pbReadBlock, iBlockSize, &dwBytesTransferred, NULL);
		
		CP_ASSERT(dwBytesTransferred == iBlockSize);
		
		iFileCursor -= iBlockSize;
	}
	
	return TRUE;
}

//
//
//
