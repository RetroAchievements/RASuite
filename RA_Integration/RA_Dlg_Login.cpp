#include "RA_Dlg_Login.h"

#include <stdio.h>
#include <string>

#include "RA_Resource.h"
#include "RA_User.h"
#include "RA_httpthread.h"
#include "RA_PopupWindows.h"


RA_Dlg_Login::RA_Dlg_Login() : 
	IRA_Dialog{ IDD_RA_LOGIN }
{
	// It probably doesn't like it C++ way

}



// This function will never be called since it's handled in the Message Queue
INT_PTR RA_Dlg_Login::DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

BOOL RA_Dlg_Login::OnInitDialog(HWND hDlg, HWND hDlgFocus, LPARAM lParam)
{
	auto UsernameCtl = GetDlgItem(hDlg, IDC_RA_USERNAME);
	

	SetWindowText(UsernameCtl, 
		NativeStr(RAUsers::LocalUser().Username()).c_str());

	if (RAUsers::LocalUser().Username().length() > 2)
	{
		auto PassCtl = GetDlgItem(hDlg, IDC_RA_PASSWORD);
		SetFocus(PassCtl);
		return FALSE;	//	Must return FALSE if setting to a non-default active control.
	}
	else
	{
		return TRUE;
	}
}

void RA_Dlg_Login::OnCommand(HWND hDlg, int id, HWND hDlgCtl, UINT codeNotify)
{
	switch (id)
	{
	case IDOK:
		OnOK(hDlg);
		break;
	case IDCANCEL:
		OnCancel(hDlg);
	}
}

void RA_Dlg_Login::OnOK(HWND hDlg)
{
	BOOL bValid = TRUE;
	TCHAR sUserEntry[64];
	bValid &= (GetDlgItemText(hDlg, IDC_RA_USERNAME, sUserEntry, 64) > 0);
	TCHAR sPassEntry[64];
	bValid &= (GetDlgItemText(hDlg, IDC_RA_PASSWORD, sPassEntry, 64) > 0);

	if (!bValid || lstrlen(sUserEntry) < 0 || lstrlen(sPassEntry) < 0)
	{
		MessageBox(nullptr, TEXT("Username/password not valid! Please check and reenter"), TEXT("Error!"), MB_OK);
		return;
	}

	PostArgs args
	{
		{'u', Narrow(sUserEntry) },
		{'p', Narrow(sPassEntry) } //	Plaintext password(!)
	};

	Document doc;
	if (RAWeb::DoBlockingRequest(RequestLogin, args, doc))
	{
		std::string sResponse;
		std::string sResponseTitle;

		if (doc["Success"].GetBool())
		{
			const std::string& sUser = doc["User"].GetString();
			const std::string& sToken = doc["Token"].GetString();
			const unsigned int nPoints = doc["Score"].GetUint();
			const unsigned int nUnreadMessages = doc["Messages"].GetUint();

			bool bRememberLogin = (IsDlgButtonChecked(hDlg, IDC_RA_SAVEPASSWORD) != BST_UNCHECKED);

			RAUsers::LocalUser().ProcessSuccessfulLogin(sUser, sToken, nPoints, nUnreadMessages, bRememberLogin);

			sResponse = "Logged in as " + sUser + ".";
			sResponseTitle = "Logged in Successfully!";

			//g_PopupWindows.AchievementPopups().SuppressNextDeltaUpdate();
		}
		else
		{
			sResponse = std::string("Failed!\r\n"
				"Response From Server:\r\n") +
				doc["Error"].GetString();
			sResponseTitle = "Error logging in!";
		}

		// My implementation seems to need MessageBox to be a child
		// Figured this out, hDlg is the handle to IRA_Dialog
		// but the RA_Login spans it's own window
		MessageBox(GetActiveWindow(), NativeStr(sResponse).c_str(),
			NativeStr(sResponseTitle).c_str(), MB_OK);

		auto test{ RAUsers::LocalUser().IsLoggedIn() };
		//	If we are now logged in
		if (RAUsers::LocalUser().IsLoggedIn())
		{
			//	Close this dialog
			IRA_Dialog::OnOK(hDlg);
			return;
		}
		return;
	}
	else
	{
		if (!doc.HasParseError() && doc.HasMember("Error"))
		{
			MessageBox(hDlg, NativeStr(std::string("Server error: ") + std::string(doc["Error"].GetString())).c_str(), TEXT("Error!"), MB_OK);
		}
		else
		{
			MessageBox(hDlg, TEXT("Unknown error connecting... please try again!"), TEXT("Error!"), MB_OK);
		}

		return;	//	==Handled
	}
}
