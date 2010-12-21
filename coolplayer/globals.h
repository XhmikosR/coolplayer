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
// Globals

# ifndef CP_GLOBALS_H
# define CP_GLOBALS_H

typedef struct _CPs_DrawContext
{
	HDC m_dcDraw;
	POINT m_ptOffset;
	RECT m_rClip;
	
} CPs_DrawContext;


typedef void* CP_HPLAYER;
typedef void* CP_HPLAYLIST;
typedef void* CP_HPLAYLISTITEM;
typedef void* CP_COMPOSITEFILE;
typedef void* CP_HINTERFACE;
typedef void* CP_HINTERFACEPART;
typedef void* CP_HSYSICON;



#include "resource.h"
#include "skin.h"
#include "CPI_Verbs.h"
#include "CPI_Keyboard.h"
#include "CPI_Image.h"
#include "CPI_Interface.h"
#include "CPSK_Skin.h"
#include "CLV_ListView.h"
#include "String.h"

// menu id's and locations
#define SKIN_SUBMENU_INDEX		5

// Handle to player
#define CP_BUILD_NUMBER					220
#define CP_COOLPLAYER					"CoolPlayer"
#define CIC_HTTPHEADER					"http://"
#define CIC_HTTPSHEADER					"https://"
#define CIC_FTPHEADER					"ftp://"

#ifdef _DEBUG
#define CLC_COOLPLAYER_MUTEX			"COOLPLAYER-045FA840-B10D-2G3-3436-006067709674D"
#else
#define CLC_COOLPLAYER_MUTEX			"COOLPLAYER-045FA840-B10D-2G3-3436-006067709674"
#endif

#define WM_NOTIFYICON					WM_USER
#define NOTIFY_ICON_ID					4567
#define CPC_INVALIDCHAR					-1

// Registry
#define CIC_COOLPLAYER_FILETYPE						"CoolPlayer.Audio"
#define CIC_COOLPLAYER_FILEDESC						"CoolPlayer Audio file"
#define CIC_COOLPLAYER_PLAYLISTFILETYPE				"CoolPlayer.Playlist"
#define CIC_COOLPLAYER_PLAYLISTFILEDESC				"CoolPlayer Playlist"
#define CLC_COOLPLAYER_INTERFACECLASSNAME			"CoolPlayer_Interface"
#define CLC_COOLPLAYER_WINDOWCLASSNAME				"CoolPlayer"
#define CLC_COOLPLAYER_PLAYLIST_WINDOWCLASSNAME		"CoolPlayer_Playlist"

// Playlist columns
#define PLAYLIST_TRACKSTACK				0
#define PLAYLIST_TITLE					1
#define PLAYLIST_ARTIST					2
#define PLAYLIST_ALBUM					3
#define PLAYLIST_YEAR					4
#define PLAYLIST_TRACKNUM				5
#define PLAYLIST_COMMENT				6
#define PLAYLIST_GENRE					7
#define PLAYLIST_PATH					8
#define PLAYLIST_FILENAME				9
#define PLAYLIST_LENGTH					10

#define PLAYLIST_first					0
#define PLAYLIST_last					10

#define PLAYLIST_IMAGE_SELECTED			0
#define PLAYLIST_IMAGE_PLAIN			1


// Hotkeys
#define VK_MEDIA_NEXT_TRACK				0xB0
#define VK_MEDIA_PREV_TRACK				0xB1
#define VK_MEDIA_STOP					0xB2
#define VK_MEDIA_PLAY_PAUSE				0xB3
//
#define CP_HOTKEY_NEXT					0x01
#define CP_HOTKEY_PREV					0x02
#define CP_HOTKEY_PLAY0					0x03
#define CP_HOTKEY_STOP					0x04
#define CP_HOTKEY_PLAY1					0x05

#define VUVOLUME						0
#define EQ1								1
#define EQ2								2
#define EQ3								3
#define EQ4								4
#define EQ5								5
#define EQ6								6
#define EQ7								7
#define EQ8								8
#define POSITION						100

// Playlist defines for resizing
#define RESIZE							5
#define BOTTOMRIGHT						4
#define TOPRIGHT						2
#define BOTTOMLEFT						3
#define TOPLEFT							1

#define ASCENDING						0
#define DESCENDING						1
#define RANDOM							2

// Timer IDs
#define CPC_TIMERID_SCROLLTITLETEXT		0x033
#define CPC_TIMERID_INTERTRACKDELAY		0x038
#define CPC_TIMERID_BOUNCINGICON		0x29A
#define CPC_TIMERID_ROTATINGSMILY		0x29A

#define SC_SIZE_TOPLEFT					SC_SIZE+4
#define SC_SIZE_TOPMIDDLE				SC_SIZE+3
#define SC_SIZE_TOPRIGHT				SC_SIZE+5
#define SC_SIZE_SIDELEFT				SC_SIZE+1
#define SC_SIZE_SIDERIGHT				SC_SIZE+2
#define SC_SIZE_BOTTOMLEFT				SC_SIZE+7
#define SC_SIZE_BOTTOMMIDDLE			SC_SIZE+6
#define SC_SIZE_BOTTOMRIGHT				SC_SIZE+8


// Main graphics bitmaps

struct
{
	HBITMAP bmp_main_up;
	HBITMAP bmp_main_down;
	HBITMAP bmp_main_switch;
	HBITMAP bmp_main_time_font;
	HBITMAP bmp_main_title_font;
	HBITMAP bmp_main_track_font;
	HBITMAP bmp_main_title_area;
	HBITMAP bmp_playlist_bg;
	HPALETTE pal_main;
} graphics;

// Window pointers

struct
{
	HWND wnd_main;
	HWND    dlg_playlist;
	HWND    dlg_options;
	HWND    wnd_tooltip;
	HWND    wnd_playlist_IPEdit;
	HWND    m_hWndFindDialog;
	HWND    m_hWndMain;
	HWND    m_hWndPlaylist;
	CP_HINTERFACE m_hifMain;
	CP_HINTERFACE m_hifPlaylist;
} windows;

// Drawable areas

struct
{
	HDC     dc_memory;
	HDC     dc_main;
} drawables;


// Bitmaps for playlist

struct
{
	SIZE    UpLeft;
	SIZE    UpRight;
	SIZE    DownLeft;
	SIZE    DownRight;
	SIZE    UpMid;
	SIZE    DownMid;
	SIZE    LeftMid;
	SIZE    RightMid;
	SIZE    MinSize;
	SIZE    Colors;
	HBITMAP bm[16];
} PlayListBitmap;

// Playist item structure.

struct ListStruct
{
	int     nritem;
	unsigned char mp3track;
	char    mp3title[MAX_PATH];
	char    mp3artist[31];
	char    mp3album[31];
	char    mp3year[5];
	char    mp3comment[31];
	char    mp3genre[30];
	char    mp3path[MAX_PATH];
};

/////////////////////////////////////////////////////////////////////////////////
//
// Player state
typedef enum _CPe_PlayerState
{
	cppsUndefined,
	cppsEndOfStream,
	cppsPaused,
	cppsPlaying,
	cppsStopped
} CPe_PlayerState;
//
// File info

typedef struct _CPs_FileInfo
{
	UINT m_iFileLength_Secs;
	UINT m_iBitRate_Kbs;
	UINT m_iFreq_Hz;
	BOOL m_bStereo;
	BOOL m_b16bit;
} CPs_FileInfo;

//
// EQ settings

typedef struct _CPs_EQSettings
{
	BOOL m_bEnabled;
	char m_aryBands[8];
	
} CPs_EQSettings;

//
typedef enum _CPe_QuickFindTerm
{
	qftUndefined,
	qftTitle,
	qftAlbum,
	qftArtist
} CPe_QuickFindTerm;
//
typedef enum _CPe_MixerMode
{
	mmMasterVolume,
	mmWaveVolume,
	mmInternal
} CPe_MixerMode;
//
//
/////////////////////////////////////////////////////////////////////////////////

BOOL    cmdline_parse_argument(char *token);
BOOL    main_draw_vu_all(HWND hWnd, WPARAM wParam, LPARAM lParam,
						 BOOL rememberlastval);
BOOL    path_is_directory(char *filename);
BOOL    path_is_relative(const char *path);
BOOL    path_remove_filespec(LPTSTR path);
BOOL    playlist_skin_draw_side(HDC hdc, HDC winmemdc, BITMAP * bm,
								RECT * winrect, int type);
BOOL    playlist_skin_get_ini_value(char *key, char *position,
									SIZE * rect);
BOOL    playlist_skin_set_control_bitmap(char *position);
BOOL    window_set_always_on_top(HWND hWnd, BOOL yes);
BOOL CALLBACK window_search(HWND hWnd, LPARAM lParam);
char   *str_delete_substr(char *strbuf, char *strtodel);
char   *str_trim(char *string);
DWORD   main_get_program_path(HINSTANCE hInst, LPTSTR pszBuffer,
							  DWORD dwSize);
HBITMAP systray_rotate_bmp(HBITMAP hBitmap, float radians,
						   COLORREF clrBack);
HRESULT path_create_link(LPCSTR lpszPathObj, LPSTR lpszPathLink,
						 LPSTR lpszDesc);
HRGN    main_bitmap_to_region(HBITMAP hBmp, COLORREF cTransparentColor);
HRGN    main_bitmap_to_region_1bit(HBITMAP hBmp, COLORREF cTransparentColor);
HWND    about_create(HWND hWnd);
int     cmdline_parse_files(int argc, char **argv);
int     cmdline_parse_options(int argc, char **argv, HWND hWnd);
int     main_add_tooltips(HWND hWnd, BOOL update);
int     main_play_control(WORD wParam, HWND hWnd);
int     main_set_default_skin(void);
int     main_skin_open(char *name);
int     main_skin_set_struct_value(int object, int x, int y, int w, int h,
								   int maxw, int x2, int y2, int w2,
								   int h2, char *tooltip);
int     playlist_open_file(BOOL clearlist);
int     playlist_skin_read();
int     playlist_write();
int     window_bmp_blt(HWND hWnd, HBITMAP SrcBmp, int srcx, int srcy,
					   int srcw, int srch, int dstx, int dsty);
int  window_bmp_transparent_blt(HWND hWnd, HBITMAP SrcBmp, int srcx, int srcy, int srcw, int srch, int dstx, int dsty);
int    *cmdline_get_argument(char *arg, HWND hWnd);
LRESULT CALLBACK main_windowproc(HWND hWnd, UINT message, WPARAM wParam,
								 LPARAM lParam);
LRESULT CALLBACK options_windowproc(HWND hwndDlg, UINT msg, WPARAM wParam,
									LPARAM lParam);
LRESULT CALLBACK url_windowproc(HWND hwndDlg, UINT msg, WPARAM wParam,
								LPARAM lParam);
LRESULT CALLBACK about_windowproc(HWND hwndDlg, UINT msg, WPARAM wParam,
								  LPARAM lParam);
LRESULT CALLBACK playlist_windowproc(HWND hwndDlg, UINT msg, WPARAM wParam,
									 LPARAM lParam);
void    cmdline_usage(HWND);
void    main_update_title_text();
void    main_draw_bitrate(HWND hWnd);
void    playlist_draw_control(UINT controlid, LPDRAWITEMSTRUCT ds);
void    main_draw_controls_all(HWND hWnd);
void    main_draw_frequency(HWND hWnd);
void    main_draw_time(HWND hWnd);
void    main_draw_title(HWND hWnd);
void    main_draw_tracknr(HWND hWnd);
void    main_draw_vu_from_mouse(HWND hWnd, int vunummer, int vuwaarde);
void    main_draw_vu_from_value(HWND hWnd, int vunummer, int vuwaarde);
void    main_menuproc(HWND hWnd, LPPOINT points);
void    main_reset_window(HWND hWnd);
void    main_set_eq(void);
void    main_skin_add_to_menu(char *name);
void    main_skin_check_ini_value(char *textposition,
								  Associate * associate);
void    main_skin_select_menu(char *name);
void    options_read(void);
void    options_write(void);
void    path_add_backslash(char *path);
void    path_unquote(LPTSTR path);
void    playlist_write_default(void);
void    options_create(HWND hWnd);
void    playlist_move_control(int ControlID, LPRECT NewPlaylistrect,
							  int type);
void    url_create(HWND hWnd);

struct
{
	BOOL    auto_exit_after_playing;
	BOOL    auto_play_when_started;
	BOOL    easy_move;
	BOOL    equalizer;
	BOOL    allow_file_once_in_playlist;
	BOOL allow_multiple_instances;
	BOOL    always_on_top;
	BOOL    read_id3_tag;
	BOOL    read_id3_tag_of_selected;
	BOOL    read_id3_tag_in_background;
	BOOL    support_id3v2;
	BOOL    prefer_native_ogg_tags;
	BOOL    work_out_track_lengths;
	BOOL    show_remaining_time;
	BOOL    remember_playlist;
	BOOL    remember_last_played_track;
	BOOL    repeat_playlist;
	BOOL    rotate_systray_icon;
	BOOL    scroll_track_title;
	BOOL    show_on_taskbar;
	BOOL    shuffle_play;
	BOOL    use_default_skin;
	BOOL    use_playlist_skin;
	char    last_used_directory[MAX_PATH];
	char initial_file[MAX_PATH];
	int     playlist_column_widths[PLAYLIST_last + 1];
	int     playlist_column_seq[PLAYLIST_last + 1];
	BOOL    playlist_column_visible[PLAYLIST_last + 1];
	int     eq_settings[9];
	int     show_playlist;
	int     decoder_output_mode;
	int     remember_skin_count;
	POINT   main_window_pos;
	RECT    playlist_window_pos;
	short   last_selected_skin_number;
	unsigned char playlist_skin_file[MAX_PATH];
	unsigned char main_skin_file[MAX_PATH];
	unsigned int seconds_delay_after_track;
	CPe_QuickFindTerm m_enQuickFindTerm;
} options;

#define BITRATE_STRLEN	40
#define FREQ_STRLEN	40

struct
{
	BOOL    playlist_bool_addsong;
	DWORD playlist_last_add_time;
	char    main_text_frequency[FREQ_STRLEN];
	char    main_text_bitrate[BITRATE_STRLEN];
	unsigned long main_long_track_duration;
	int     main_int_skin_last_number;
	CPe_PlayerState m_enPlayerState;
	int     main_drag_anchor_point;
	unsigned char main_text_last_browsed_dir[MAX_PATH];
	int     main_int_track_total_seconds;
	int     main_int_show_minimized;
	int     playlist_int_last_searched_track;
	int main_int_title_scroll_position;
	int mail_int_title_scroll_max_position;
	BOOL    cmdline_bool_clear_playlist_first;
	BOOL    main_bool_wavwrite_dir_already_known;
	BOOL    main_bool_skin_next_is_default;
	BOOL    main_bool_slider_keep_focus;
	BOOL    playlist_bool_force_skin_from_options;
	int     main_int_track_position;
	RECT    playlist_rect;
	HMENU   main_menu_popup;
	double  about_double_smiley_jump_pos;
	double  about_double_smiley_jump_speed;
	int     m_iVolume;
	CP_HPLAYER m_hPlayer;
	CP_HPLAYLIST m_hPlaylist;
	BOOL m_bStreaming;
	int  m_iStreamingPortion;
	int m_iLastPlaylistSortColoumn;
	HIMAGELIST m_hPlaylistImages;
	POINT m_ptQuickFindWindowPos;
	BOOL m_bQuickFindWindowPos_Valid;
	HHOOK m_hhkListView_Posted;
	BOOL m_bIP_InhibitUpdates;
	unsigned int m_iInPlaceSubItem;
	CP_HLISTVIEW m_hPlaylistViewControl;
	BOOL m_bOptions_ChangedSkin;
	CP_HSYSICON m_hSysIcon;
	CPe_MixerMode m_enMixerMode;
} globals;

CoolSkin Skin;

#endif
