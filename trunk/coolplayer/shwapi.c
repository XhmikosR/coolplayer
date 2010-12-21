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


#include "stdafx.h"
#include "globals.h"

void    path_add_backslash(char *path)
{
	if (path[strlen(path) - 1] != '\\')
		strcat(path, "\\");
}

BOOL    path_is_relative(const char *path)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	
	_splitpath(path, drive, dir, fname, ext);
	
	if (!*drive)
	{
		char    buffer[MAX_PATH];
		char   *position;
		
		strcpy(buffer, dir);
		
		if (buffer[0] == '\\' && buffer[1] == '\\')
		{
			position =
				strchr(strchr
					   (strchr(strchr(buffer, '\\') + 1, '\\') +
						1, '\\') + 1, '\\') + 1;
			*position = '\0';
			
			if (GetDriveType(buffer) == DRIVE_REMOTE)
				return FALSE;
				
			return TRUE;
		}
		
		else
			return TRUE;
	}
	
	return FALSE;
}

BOOL    path_remove_filespec(LPTSTR path)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	
	_splitpath(path, drive, dir, fname, ext);
	_makepath(path, drive, dir, NULL, NULL);
	return TRUE;
}

void    path_unquote(LPTSTR path)
{
	char pathbuffer[MAX_PATH];
	int lengte;
	
	if (path[0] == '\"')
	{
		strcpy(pathbuffer, path + 1);
		lengte = strlen(pathbuffer);
		pathbuffer[lengte - 1] = '\0';
		strcpy(path, pathbuffer);
	}
}
