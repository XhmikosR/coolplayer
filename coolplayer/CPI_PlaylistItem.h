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
// Cooler PlaylistItem
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
typedef enum _CPe_ReadWriteState
{
	rwsUnknown,
	rwsReadOnly,
	rwsReadWrite,
	rwsBadFile
} CPe_ReadWriteState;
//
typedef enum _CPe_FilenameFormat
{
	rwsArtistAlbumNumberTitle = 1,
	rwsArtistNumberTitle = 2,
	rwsAlbumNumberTitle = 3,
	rwsAlbumNumber = 4,
	rwsNumberTitle = 5,
	rwsTitle = 6
} CPe_FilenameFormat;
//
//
////////////////////////////////////////////////////////////////////////////////



#define CIC_INVALIDPLAYLISTCOOKIE					0xFFFFFFFF
#define CIC_INVALIDGENRE							((unsigned char)0xFF)
#define CIC_INVALIDTRACKNUM							((unsigned char)0xFF)
#define CIC_TRACKSTACK_UNSTACKED					0xEFFFFFFF
////////////////////////////////////////////////////////////////////////////////
//
// Accessors (always use these!!)
const char* CPLI_GetPath(const CP_HPLAYLISTITEM hItem);
const char* CPLI_GetFilename(const CP_HPLAYLISTITEM hItem);
CPe_ReadWriteState CPLI_GetReadWriteState(const CP_HPLAYLISTITEM hItem);
const char* CPLI_GetExtension(const CP_HPLAYLISTITEM hItem);
//
// These may return NULL if this info isn't available
const char* CPLI_GetTrackStackPos_AsText(const CP_HPLAYLISTITEM hItem);
int CPLI_GetTrackStackPos(const CP_HPLAYLISTITEM hItem);
const char* CPLI_GetArtist(const CP_HPLAYLISTITEM hItem);
const char* CPLI_GetAlbum(const CP_HPLAYLISTITEM hItem);
const char* CPLI_GetTrackName(const CP_HPLAYLISTITEM hItem);
const char* CPLI_GetYear(const CP_HPLAYLISTITEM hItem);
const char* CPLI_GetGenre(const CP_HPLAYLISTITEM hItem);
const unsigned char CPLI_GetTrackNum(const CP_HPLAYLISTITEM hItem);
const char* CPLI_GetTrackNum_AsText(const CP_HPLAYLISTITEM hItem);
const char* CPLI_GetComment(const CP_HPLAYLISTITEM hItem);
int CPLI_GetTrackLength(const CP_HPLAYLISTITEM hItem);
const char* CPLI_GetTrackLength_AsText(const CP_HPLAYLISTITEM hItem);
//
// Update functions
void CPLI_SetTrackStackPos(CP_HPLAYLISTITEM hItem, const int iNewPos);
void CPLI_SetArtist(CP_HPLAYLISTITEM hItem, const char* pcNewValue);
void CPLI_SetAlbum(CP_HPLAYLISTITEM hItem, const char* pcNewValue);
void CPLI_SetTrackName(CP_HPLAYLISTITEM hItem, const char* pcNewValue);
void CPLI_SetYear(CP_HPLAYLISTITEM hItem, const char* pcNewValue);
void CPLI_SetGenreIDX(CP_HPLAYLISTITEM hItem, const unsigned char iNewValue);
void CPLI_SetTrackNum(CP_HPLAYLISTITEM hItem, const unsigned char iNewValue);
void CPLI_SetTrackNum_AsText(CP_HPLAYLISTITEM hItem, const char* pcNewValue);
void CPLI_SetComment(CP_HPLAYLISTITEM hItem, const char* pcNewValue);
void CPLI_CalculateLength(CP_HPLAYLISTITEM hItem);
BOOL CPLI_RenameTrack(CP_HPLAYLISTITEM hItem, const CPe_FilenameFormat enFormat);
//
// For use by the playlist window
void CPLI_SetCookie(CP_HPLAYLISTITEM hItem, const int iCookie);
int CPLI_GetCookie(const CP_HPLAYLISTITEM hItem);
//
// These may return NULL if this item is at the end or start of the playlist respectivly
CP_HPLAYLISTITEM CPLI_Next(const CP_HPLAYLISTITEM hItem);
CP_HPLAYLISTITEM CPLI_Prev(const CP_HPLAYLISTITEM hItem);
//
// ID3 tag
BOOL CPLI_IsTagDirty(CP_HPLAYLISTITEM hItem);
void CPLI_ReadTag(CP_HPLAYLISTITEM hItem);
void CPLI_WriteTag(CP_HPLAYLISTITEM hItem);
////////////////////////////////////////////////////////////////////////////////
