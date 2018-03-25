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



	// This function will never be called since it's handled in the Message Queue
	INT_PTR Dlg_Login::DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return 0;
	}

	BOOL Dlg_Login::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
	{
		hUsername = GetDlgItem(hwnd, IDC_RA_USERNAME);
		SetText(hUsername, cusername());

		if ( username_length() > 2 )
		{
			hPassword = GetDlgItem(hwnd, IDC_RA_PASSWORD);
			_CSTD SetFocus(hPassword);
			return FALSE;	//	Must return FALSE if setting to a non-default active control.
		}
		else
		{
			return TRUE;
		}
	}

	void Dlg_Login::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
	{
		switch ( id )
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
		BOOL bValid = TRUE;
		TCHAR sUserEntry[64];
		bValid &= (GetText(hUsername, 64, sUserEntry) > 0);
		TCHAR sPassEntry[64];
		bValid &= (GetText(hPassword, 64, sPassEntry) > 0);

		if ( !bValid || lstrlen(sUserEntry) < 0 || lstrlen(sPassEntry) < 0 )
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
		if ( RAWeb::DoBlockingRequest(RequestLogin, args, doc) )
		{
			std::string sResponse;
			std::string sResponseTitle;

			if ( doc["Success"].GetBool() )
			{
				const std::string& sUser = doc["User"].GetString();
				const std::string& sToken = doc["Token"].GetString();
				const unsigned int nPoints = doc["Score"].GetUint();
				const unsigned int nUnreadMessages = doc["Messages"].GetUint();

				hSavePassword = GetDlgItem(hwnd, IDC_RA_SAVEPASSWORD);
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

			auto test{ RAUsers::LocalUser().IsLoggedIn() };
			//	If we are now logged in
			if ( RAUsers::LocalUser().IsLoggedIn() )
			{
				//	Close this dialog
				IRA_Dialog::OnOK(hwnd);
				return;
			}
			return;
		}
		else
		{
			if ( !doc.HasParseError() && doc.HasMember("Error") )
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

	void Dlg_Login::OnNCDestroy(HWND hwnd)
	{
		Close(hUsername);
		Close(hPassword);
		Close(hSavePassword);

		IRA_Dialog::OnNCDestroy(hwnd);
	}

}