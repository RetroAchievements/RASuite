#include "RA_Dlg_RichPresence.h"

#include "RA_Core.h"
#include "RA_Resource.h"
#include "RA_RichPresence.h"

_RA Dlg_RichPresence g_RichPresenceDialog;

namespace ra {

Dlg_RichPresence::Dlg_RichPresence() :
    IRA_Dialog{ IDD_RA_RICHPRESENCE }
{
} // end constructor

Dlg_RichPresence::Dlg_RichPresence(Dlg_RichPresence&& b) noexcept :
    m_bTimerActive{ b.m_bTimerActive },
    m_hRichPresenceText{ b.m_hRichPresenceText },
    m_hRpWnd{ b.m_hRpWnd }
{
    b.m_bTimerActive = false;
	b.PostNcDestroy();
}
#pragma warning(suppress : 4702) // this warning is important but it's causing false alarms
Dlg_RichPresence& Dlg_RichPresence::operator=(Dlg_RichPresence&& b) noexcept
{
    m_bTimerActive      = b.m_bTimerActive;
    m_hRichPresenceText = b.m_hRichPresenceText;
    m_hRpWnd            = b.m_hRpWnd;

    b.m_bTimerActive = false;
	b.PostNcDestroy();
    return *this;

}

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
        SetTimer(m_hRpWnd, 1_z, 1000_z, nullptr);
        m_bTimerActive = true;
    } // end if
} // end function StartTimer

void Dlg_RichPresence::StopTimer()
{
    if (m_bTimerActive)
    {
        KillTimer(m_hRpWnd, 1_z);
        m_bTimerActive = false;
    } // end if
} // end function StopTimer


#pragma warning(suppress : 4702)
BOOL Dlg_RichPresence::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    m_hRichPresenceText = GetDlgItem(hwnd, IDC_RA_RICHPRESENCERESULTTEXT);

    hFont = CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, VARIABLE_PITCH, nullptr);

    SetFont(m_hRichPresenceText, hFont, TRUE);
    SetCaption(TEXT("Rich Presence Monitor"));
    RestoreWindowPosition(hwnd, lpCaption, true, true);
    return TRUE;
} // end function OnInitDialog


#pragma warning(suppress : 4702) // You're lying compiler! Tested it myself
void Dlg_RichPresence::OnNCDestroy(HWND hwnd)
{
    DestroyControl(m_hRichPresenceText);
    IRA_Dialog::OnNCDestroy(hwnd);
} // end function OnNCDestroy

void Dlg_RichPresence::OnTimer(_UNUSED HWND hwnd, _UNUSED UINT id)
{
    // I thought we we're doing ASCII only?
    auto sRP{ g_RichPresenceInterpretter.GetRichPresenceString() };
    SetText(m_hRichPresenceText, sRP.c_str());
} // end function OnTimer




void Dlg_RichPresence::Close(HWND hwnd)
{
    StopTimer();
    IRA_Dialog::Close(hwnd); // the docs say modeless have to use destroywindow
} // end function Close

#pragma region unused
INT_PTR Dlg_RichPresence::DialogProc(_UNUSED HWND hwnd, _UNUSED UINT uMsg,
    _UNUSED WPARAM wParam, _UNUSED LPARAM lParam)
{
	// If this works, this will be awesome
	return IRA_Dialog::DialogProc(hwnd, uMsg, wParam, lParam);
} // end function DialogProc


void Dlg_RichPresence::OnCommand(HWND hwnd, int id, _UNUSED HWND hwndCtl,
    _UNUSED UINT codeNotify)
{
    switch (id)
    {
    case IDOK:
        OnOK(hwnd);
    }
} // end function OnCommand  
#pragma endregion

} // namespace ra
