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

HWND about_create(HWND hWnd)
{
	HANDLE  hinst;

	globals.about_double_smiley_jump_pos = 20;
	globals.about_double_smiley_jump_speed = 0.1;

	hinst = GetModuleHandle(NULL);
	DialogBox(hinst, MAKEINTRESOURCE(IDD_ABOUT), NULL, (DLGPROC)about_windowproc);
	return hWnd;
}
//
//
//
void SetAboutText(HWND hWnd, const UINT uiStringResource)
{
	HRSRC hResource;
	HGLOBAL hResourceData;

	hResource = FindResource(NULL, MAKEINTRESOURCE(uiStringResource), "RAW");
	hResourceData = LoadResource(NULL, hResource);
	SetDlgItemText(hWnd, IDC_ABOUTTEXT, (LPCTSTR)LockResource(hResourceData));
}
//
//
//
LRESULT CALLBACK about_windowproc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			char cTitle[100];
			sprintf(cTitle, "About CoolPlayer %d", CP_BUILD_NUMBER);
			SetWindowText(hwndDlg, cTitle);

			// Setup up jumping head timer
			SetTimer(hwndDlg, CPC_TIMERID_BOUNCINGICON, 20, NULL);
			SetAboutText(hwndDlg, IDR_ABOUT1);

			SendDlgItemMessage(hwndDlg, IDC_ABOUTRADIO, BM_SETCHECK, TRUE, 0);

			// Float window to top (as dialog disables main window)
			SetWindowPos(hwndDlg, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);

			SetForegroundWindow(hwndDlg);
			return TRUE;
		} // end WM_INITDIALOG

		case WM_TIMER:
		{
			int iconposition;

			iconposition = (int) (cos(globals.about_double_smiley_jump_pos) * 130) + 140;
			MoveWindow(GetDlgItem(hwndDlg, IDC_ABOUTICON), 9, iconposition,
				32, 32, TRUE);
			globals.about_double_smiley_jump_pos =
				globals.about_double_smiley_jump_pos +
				globals.about_double_smiley_jump_speed;
			if (globals.about_double_smiley_jump_pos > 5.4)
				globals.about_double_smiley_jump_pos = 0.9;

			break;
		} // end WM_TIMER

		case WM_CLOSE:
		{
			EndDialog(hwndDlg, 1);
			KillTimer(hwndDlg, CPC_TIMERID_BOUNCINGICON);
			return TRUE;
		} // end WM_CLOSE

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hwndDlg, 1);
					break;

				case IDC_WEBSITE:
					ShellExecute(0, 0, "http://coolplayer.sourceforge.net", 0, 0, SW_SHOWDEFAULT);
					break;

				case IDC_KEYBOARDRADIO:
					SetAboutText(hwndDlg, IDR_KEYBOARD);
					break;

				case IDC_ABOUTRADIO:
					SetAboutText(hwndDlg, IDR_ABOUT1);
					break;

				case IDC_CHANGELOG:
					SetAboutText(hwndDlg, IDR_CHANGES);
					break;
			}
			break;
		} // end WM_COMMAND

		case WM_NOTIFY:
		{
			if (wParam == IDC_SPIN1)
			{
				NM_UPDOWN *header = (NM_UPDOWN *) lParam;
				if (header->iDelta == 1)
					globals.about_double_smiley_jump_speed = globals.about_double_smiley_jump_speed - 0.01;
				else
					globals.about_double_smiley_jump_speed = globals.about_double_smiley_jump_speed + 0.01;

				if (globals.about_double_smiley_jump_speed < 0)
					globals.about_double_smiley_jump_speed = 0;
			}
			break;
		} // end WM_NOTIFY
	}

	return FALSE;
}
//
//
//
