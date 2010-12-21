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


enum Objects
{
	PlaySwitch,
	StopSwitch,
	PauseSwitch,
	EjectButton,
	RepeatSwitch,
	ShuffleSwitch,
	EqSwitch,
	NextButton,
	PrevButton,
	PlaylistButton,
	MinimizeButton,
	NextSkinButton,
	ExitButton,
	
	MoveArea,
	
	VolumeSlider,
	PositionSlider,
	Eq1,
	Eq2,
	Eq3,
	Eq4,
	Eq5,
	Eq6,
	Eq7,
	Eq8,
	
	SongtitleText,
	TrackText,
	TimeText,
	BitrateText,
	FreqText,
	ReducedSize,
	Lastone
};

typedef struct
{
	char    name[128];
	enum Objects Object;
} Associate;

typedef struct
{
	int     x, y, w, h;
	int     maxw;
	int     x2, y2, w2, h2;
	char    tooltip[100];
} Coords;

typedef struct Skin
{
	Coords  Object[Lastone];
	
	COLORREF transparentcolor;
	
	HBITMAP MainUp;
	HBITMAP MainDown;
	HBITMAP MainSwitched;
	HBITMAP TextFont;
	HBITMAP TimeFont;
	char    CoolUp[MAX_PATH];
	char    CoolDown[MAX_PATH];
	char    CoolSwitch[MAX_PATH];
	char    aTrackFont[MAX_PATH];
	char    aTimeFont[MAX_PATH];
	char    aTextFont[MAX_PATH];
} CoolSkin;
