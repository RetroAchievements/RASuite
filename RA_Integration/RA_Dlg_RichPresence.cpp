#include "RA_Dlg_RichPresence.h"

#include "RA_Core.h"
#include "RA_Resource.h"
#include "RA_RichPresence.h"

_RA Dlg_RichPresence g_RichPresenceDialog;

namespace ra {

Dlg_RichPresence::Dlg_RichPresence() :
	IRA_Dialog{ IDD_RA_RICHPRESENCE }
{
	SetCaption(TEXT("Rich Presence Monitor"));
	hFont = CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, VARIABLE_PITCH, nullptr);

} // end constructor

void Dlg_RichPresence::StartMonitoring()
{
	if (g_RichPresenceInterpretter.Enabled())
	{
		StartTimer();
		return;
	} // end if

	StopTimer();
} // end function StartMonitoring

void Dlg_RichPresence::StartTimer()
{
	if (!m_bTimerActive)
	{
		SetTimer(hWnd, 1_z, 1000_z, nullptr);
		m_bTimerActive = true;
	} // end if
} // end function StartTimer

void Dlg_RichPresence::StopTimer()
{
	if (m_bTimerActive)
	{
		KillTimer(hWnd, 1_z);
		m_bTimerActive = false;
	} // end if
} // end function StopTimer

BOOL Dlg_RichPresence::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	m_hRichPresenceText = GetDlgItem(hwnd, IDC_RA_RICHPRESENCERESULTTEXT);
	SetFont(m_hRichPresenceText, hFont, lParam);

	RestoreWindowPosition(hwnd, lpCaption, true, true);
	return TRUE;
} // end function OnInitDialog



void Dlg_RichPresence::OnNCDestroy(HWND hwnd)
{
	IRA_Dialog::Close(m_hRichPresenceText);
	IRA_Dialog::OnNCDestroy(hwnd);
} // end function OnNCDestroy

void Dlg_RichPresence::OnTimer(HWND hwnd, UINT id)
{
	// I thought we we're doing ASCII only?
	auto sRP{ g_RichPresenceInterpretter.GetRichPresenceString() };
	SetText(m_hRichPresenceText, sRP.c_str());
} // end function OnTimer

void Dlg_RichPresence::OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	RememberWindowSize(hwnd, lpCaption);
} // end function OnSize

void Dlg_RichPresence::Move(HWND hwnd, int x, int y)
{
	RememberWindowPosition(hwnd, lpCaption);
} // end function Move

void Dlg_RichPresence::Close(HWND hwnd)
{
	StopTimer();
	IRA_Dialog::Close(hwnd); // the docs say modeless have to use destroywindow
} // end function Close

#pragma region unused
INT_PTR Dlg_RichPresence::DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return 0;
} // end function DialogProc


void Dlg_RichPresence::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
} // end function OnCommand  
#pragma endregion

} // namespace ra