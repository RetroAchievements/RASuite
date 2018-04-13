#include "RA_Dlg_Login.h"

#include "RA_Resource.h"
#include "RA_User.h"
#include "RA_httpthread.h"
#include "RA_PopupWindows.h"
#include "RA_Defs.h"

namespace ra
{
Dlg_Login::Dlg_Login() :
	IRA_Dialog{ IDD_RA_LOGIN }
{
}



HWND Dlg_Login::GetWindow() const noexcept
{
    return GetActiveWindow();
}

// modeless shit does something weird with the handles
INT_PTR Dlg_Login::DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Some stuff works in the pure virtual some stuff doesn't
	switch (uMsg)
	{
		HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
	default:
		return IRA_Dialog::DialogProc(hwnd, uMsg, wParam, lParam);
	}
	
}

BOOL Dlg_Login::OnInitDialog(HWND hwnd, HWND hwndFocus, _UNUSED LPARAM lParam)
{
	hUsername = GetDlgItem(hwnd, IDC_RA_USERNAME);
	hPassword = GetDlgItem(hwnd, IDC_RA_PASSWORD); // seems this has to be up here or it will be null
	hSavePassword = GetDlgItem(hwnd, IDC_RA_SAVEPASSWORD); // It was being funky
	SetText(hUsername, cusername());

	if (username_length() > 2)
	{
		SetFocus(hPassword);
		return FALSE;	//	Must return FALSE if setting to a non-default active control.
	}
	else
	{
		return TRUE;
	}
}

void Dlg_Login::OnCommand(HWND hwnd, int id, _UNUSED HWND hwndCtl, 
	_UNUSED UINT codeNotify)
{
	switch (id)
	{
	case IDOK:
		OnOK(hwnd);
		break;
	case IDCANCEL:
		OnCancel(hwnd);
	}
}

void Dlg_Login::OnOK(HWND hwnd)
{
	auto bValid{ true };
	auto len{ GetTextLength(hUsername) };
	auto len2{ GetTextLength(hPassword) }; // ok something is wrong with the password handle

	// needs to be 1 because GetTextLength adds 1
	bValid &= (len > 1);
	bValid &= (len2 > 1);

	if (!bValid || len < 1 || len2 < 1)
	{
		MessageBox(nullptr, TEXT("Username/password not valid! Please check and reenter"), TEXT("Error!"), MB_OK);
		return;
	}

	tstring sUserEntry;
	sUserEntry.reserve(static_cast<size_t>(len));
	tstring sPassEntry;
	sPassEntry.reserve(static_cast<size_t>(len2));

	GetText(hUsername, len, sUserEntry.data());
	GetText(hPassword, len2, sPassEntry.data());

	// test vars
	/*auto user{ sUserEntry.data() };
	auto pass{ sPassEntry.data() };*/

	PostArgs args
	{
		{'u', Narrow(sUserEntry.data()) },
		{'p', Narrow(sPassEntry.data()) } //	Plaintext password(!)
	};

	Document doc;
	if (RAWeb::DoBlockingRequest(RequestLogin, args, doc))
	{
		std::string sResponse;
		std::string sResponseTitle;

		if (doc["Success"].GetBool())
		{
            // It works here, why not in RA_Core?
			const std::string& sUser = doc["User"].GetString();
			const std::string& sToken = doc["Token"].GetString();
			const unsigned int nPoints = doc["Score"].GetUint();
			const unsigned int nUnreadMessages = doc["Messages"].GetUint();

			
			auto bRememberLogin{ Button_GetCheck(hSavePassword) };

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
		// Figured this out, hwnd is the handle to IRA_Dialog
		// but the RA_Login spans it's own window
		MessageBox(GetActiveWindow(), NativeStr(sResponse).c_str(),
			NativeStr(sResponseTitle).c_str(), MB_OK);

		//auto test{ RAUsers::LocalUser().IsLoggedIn() };
		//	If we are now logged in
		if (RAUsers::LocalUser().IsLoggedIn())
		{
			//	Close this dialog
			IRA_Dialog::OnOK(hwnd);
			return;
		}
		return;
	}
	else
	{
		if (!doc.HasParseError() && doc.HasMember("Error"))
		{
			MessageBox(GetActiveWindow(), NativeStr(std::string("Server error: ") + std::string(doc["Error"].GetString())).c_str(), TEXT("Error!"), MB_OK);
		}
		else
		{
			MessageBox(GetActiveWindow(), TEXT("Unknown error connecting... please try again!"), TEXT("Error!"), MB_OK);
		}

		return;	//	==Handled
	}
}

#pragma warning(suppress : 4702)
void Dlg_Login::OnNCDestroy(HWND hwnd)
{
	// Ok these need destroywindow because of the bIsModal
	DestroyWindow(hUsername);
	DestroyWindow(hPassword);
	DestroyWindow(hSavePassword);

	IRA_Dialog::OnNCDestroy(hwnd);
}

}
