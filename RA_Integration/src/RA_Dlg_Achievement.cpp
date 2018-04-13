#include "RA_Dlg_Achievement.h"

#include "RA_Achievement.h"
#include "RA_AchievementSet.h"
#include "RA_Core.h"
#include "RA_Defs.h"
#include "RA_Dlg_AchEditor.h"
#include "RA_Dlg_GameTitle.h"
#include "RA_GameData.h"
#include "RA_httpthread.h"
#include "RA_md5factory.h"
#include "RA_Resource.h"
#include "RA_User.h"
#include "RA_GameData.h"
#include "RA_Resource.h"
#include "RA_AchievementOverlay.h" // for the colors we could move them somewhere else
_RA Dlg_Achievements g_AchievementsDialog;

namespace ra
{
constexpr auto _size{ 6 };
using column_names = std::array<cstring, _size>;

constexpr column_names COLUMN_TITLES_CORE{
    "ID", "Title", "Points", "Author", "Achieved?", "Modified?"
};
constexpr column_names COLUMN_TITLES_UNOFFICIAL{
    "ID", "Title", "Points", "Author", "Active", "Votes"
};
constexpr column_names COLUMN_TITLES_LOCAL{
    "ID", "Title", "Points", "Author", "Active", "Votes"
};
constexpr std::array<int, _size> COLUMN_SIZE{ 45, 200, 45, 80, 65, 65 };

constexpr auto NUM_COLS = _size;

int iSelect = -1;




Dlg_Achievements::Dlg_Achievements() :
    IRA_Dialog{ IDD_RA_ACHIEVEMENTS }
{
    SetCaption(TEXT("Achievements"));

}

// this is gonna be painful
Dlg_Achievements::Dlg_Achievements(Dlg_Achievements&& b) noexcept :
m_hActivateAllAch{ b.m_hActivateAllAch },
m_hActiveCore{ b.m_hActiveCore },
m_hActiveLocal{ b.m_hActiveLocal },
m_hActiveUnofficial{ b.m_hActiveUnofficial },
m_hAddAch{ b.m_hAddAch },
m_hChkAchProcessingActive{ b.m_hChkAchProcessingActive },
m_hCloneAch{ b.m_hCloneAch },
m_hCommitAch{ b.m_hCommitAch },
m_hDelAch{ b.m_hDelAch },
m_hDownloadAch{ b.m_hDownloadAch },
m_hGameHash{ b.m_hGameHash },
m_hList{ b.m_hList },
m_hNumAch{ b.m_hNumAch },
m_hPointTotal{ b.m_hPointTotal },
m_hPromoteAch{ b.m_hPromoteAch },
m_hResetAch{ b.m_hResetAch },
m_hRevertSelected{ b.m_hRevertSelected },
m_hAchievementsDlg{ b.m_hAchievementsDlg },
cdc{ std::move(b.cdc) },
m_lbxData{ std::move(b.m_lbxData) }
{
    // The controls as well as the dialogs can't be initlized at compile time,
    // only runtime. To prevent massive override of two globals per per window,
    // we're using move semantics. If I can get my window_h to work this crap
    // will be uncesserary.
	b.OnNCDestroy(b.m_hAchievementsDlg);
}

Dlg_Achievements& Dlg_Achievements::operator=(Dlg_Achievements&& b) noexcept
{
    m_hActivateAllAch         = b.m_hActivateAllAch;
    m_hActiveCore             = b.m_hActiveCore;
    m_hActiveLocal            = b.m_hActiveLocal;
    m_hActiveUnofficial       = b.m_hActiveUnofficial;
    m_hAddAch                 = b.m_hAddAch;
    m_hChkAchProcessingActive = b.m_hChkAchProcessingActive;
    m_hCloneAch               = b.m_hCloneAch;
    m_hCommitAch              = b.m_hCommitAch;
    m_hDelAch                 = b.m_hDelAch;
    m_hDownloadAch            = b.m_hDownloadAch;
    m_hGameHash               = b.m_hGameHash;
    m_hList                   = b.m_hList;
    m_hNumAch                 = b.m_hNumAch;
    m_hPointTotal             = b.m_hPointTotal;
    m_hPromoteAch             = b.m_hPromoteAch;
    m_hResetAch               = b.m_hResetAch;
    m_hRevertSelected         = b.m_hRevertSelected;
    m_hAchievementsDlg        = b.m_hAchievementsDlg;
    cdc                       = std::move(b.cdc);
    m_lbxData                 = std::move(b.m_lbxData);

    b.OnNCDestroy(b.m_hAchievementsDlg);
    return *this;
}



void Dlg_Achievements::SetupColumns()
{
    //	Remove all columns and data.
    while (ListView_DeleteColumn(m_hList, 0) == TRUE) {}
    ListView_DeleteAllItems(m_hList);

    // yup the problem is here, you're supposed to post-increment on primitives
    // With C-Like structs you wouldn't find bugs like this, fixed the access violotion exception (out of range)
    for (int i = 0; i < NUM_COLS; i++)
    {
        auto sColTitle{ ""s };
        auto ui{ to_unsigned(i) };
        switch (g_nActiveAchievementSet)
        {
        case AchievementSetType::NumAchievementSetTypes:
            _FALLTHROUGH;
        case AchievementSetType::Core:
            sColTitle = COLUMN_TITLES_CORE.at(ui);
            break;
        case AchievementSetType::Unofficial:
            sColTitle = COLUMN_TITLES_UNOFFICIAL.at(ui);
            break;
        case AchievementSetType::Local:
            sColTitle = COLUMN_TITLES_LOCAL.at(ui);
        }

        // TODO: make this into a class
        LV_COLUMN newColumn
        {
            LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT,
            LVCFMT_LEFT | LVCFMT_FIXED_WIDTH,
            COLUMN_SIZE.at(ui),
            sColTitle.data(),
            255,
            i
        };


        if (i == (NUM_COLS - 1))
            newColumn.fmt |= LVCFMT_FILL;

        ListView_InsertColumn(m_hList, i, &newColumn);
    }

    // I don't know what's the point of clearing this but you can't empty
    // an object that's already empty
    if (!m_lbxData.empty())
        m_lbxData.clear();
    ListView_SetExtendedListViewStyle(m_hList, LVS_EX_FULLROWSELECT);
}



void Dlg_Achievements::RemoveAchievement(int nIter)
{
    ASSERT(nIter < ListView_GetItemCount(m_hList));
    ListView_DeleteItem(m_hList, nIter);
    m_lbxData.erase(m_lbxData.begin() + nIter);


    auto buffer = tfm::format(" %d", g_pActiveAchievements->NumAchievements());
    SetText(m_hNumAch, NativeStr(buffer).c_str());
    SetText(m_hPointTotal, NativeStr(std::to_string(g_pActiveAchievements->PointTotal())).c_str());

    UpdateSelectedAchievementButtons(null);

    g_AchievementEditorDialog.LoadAchievement(null, FALSE);
}

size_t Dlg_Achievements::AddAchievement(const Achievement& Ach)
{
    // I seriously don't get the difference
    AchievementDlgRow some_row{ "", "", "", "", "", "" };
    AchievementDlgRow newRow{ "", "", "", "", "", "" }; // this?

    //	Add to our local array:
    newRow.at(etoi(ach_col::ID))     = std::to_string(Ach.ID());
    newRow.at(etoi(ach_col::Title))  = Ach.Title();
    newRow.at(etoi(ach_col::Points)) = std::to_string(Ach.Points());
    newRow.at(etoi(ach_col::Author)) = Ach.Author();


    //	Add to our local array:
    switch (g_nActiveAchievementSet)
    {
    case AchievementSetType::Local:
    case AchievementSetType::Unofficial:
    case AchievementSetType::NumAchievementSetTypes:
        _FALLTHROUGH;
    case AchievementSetType::Core:
        newRow.at(etoi(ach_col::Achieved))  = !Ach.Active() ? "Yes" : "No";
        newRow.at(etoi(ach_col::Modified))  = Ach.Modified() ? "Yes" : "No";
        break;
    default:
        newRow.at(etoi(ach_col::Active)) = Ach.Active() ? "Yes" : "No";
        newRow.at(etoi(ach_col::Votes))  = "N/A";
    }

    // You don't seem to need it anymore in this function
    // why?
    // I think I get it, the capacity needs to be increased or needs a limit
    // I think I remember somewhere that max cheevs was 200
    // The "max size" would be 200 but the "max capacity" would be 216
    // That's to ensure no overflows

    //................

    // what the fuck is going on!?!?
    // I don't under standit

    // let me see if this will help ease it
    // Its not reading all of it, does it need to be forwarded
    // There must be bug with the compiler I can't think of what I'm doing wrong
    // This will slow it down somewhat

    // has to be bug really fucking weird
    m_lbxData.emplace_back(std::move(newRow));


    // somethings wrong with this
    LV_ITEM item; //  don't see any real difference though
    ZeroMemory(&item, sizeof(item));

    // LV_ITEM might require zero memory I guess...
    item.mask = LVIF_TEXT;
    item.cchTextMax = 256;
    item.iItem = static_cast<int>(m_lbxData.size());

    // god dammit
    for (item.iSubItem = 0; item.iSubItem < NUM_COLS; item.iSubItem++)
    {
        //	Cache this (stack) to ensure it lives until after ListView_*Item
        //	SD: Is this necessary?

        item.pszText = m_lbxData.back().at(to_unsigned(item.iSubItem)).data();

        if (item.iSubItem == 0)
            item.iItem = ListView_InsertItem(m_hList, &item);
        else
            ListView_SetItem(m_hList, &item);
    }
    // alright it works
    //ASSERT(item.iItem == (m_lbxData.size() - 1));
    return static_cast<size_t>(item.iItem);
}

BOOL LocalValidateAchievementsBeforeCommit(std::array<int, 1> nLbxItems)
{

    for (auto& i : nLbxItems)
    {
        const auto& Ach{ g_pActiveAchievements->GetAchievement(to_unsigned(i)) };
        if (Ach.Title().length() < 2)
        {
            auto buffer = tfm::format("Achievement title too short:\n%s\nMust be greater than 2 characters.", Ach.Title().c_str());
            show_error(buffer);
            return FALSE;
        }
        if (Ach.Title().length() > 63)
        {
            auto buffer = tfm::format("Achievement title too long:\n%s\nMust be fewer than 80 characters.", Ach.Title().c_str());
            show_error(buffer);
            return FALSE;
        }

        if (Ach.Description().length() < 2)
        {
            auto buffer = tfm::format("Achievement description too short:\n%s\nMust be greater than 2 characters.", Ach.Description().c_str());
            show_error(buffer);
            return FALSE;
        }
        if (Ach.Description().length() > 255)
        {
            auto buffer = tfm::format("Achievement description too long:\n%s\nMust be fewer than 255 characters.", Ach.Description().c_str());
            show_error(buffer);
            return FALSE;
        }
        using IllegalChars = std::array<char, 2>;
        constexpr IllegalChars sIllegalChars{ '&', ':' };
        for (auto& j : sIllegalChars)
        {
            if (Ach.Title().find(j) != std::string::npos)
            {
                auto buffer = tfm::format("Achievement title contains an illegal character: '%c'\nPlease remove and try again", i);
                show_error(buffer);
                return FALSE;
            }
            if (Ach.Title().find(j) != std::string::npos)
            {
                auto buffer = tfm::format("Achievement description contains an illegal character: '%c'\nPlease remove and try again", i);
                show_error(buffer);
                return FALSE;
            }
        } // end for




    } // end for

    return TRUE;
}



BOOL AttemptUploadAchievementBlocking(const Achievement& Ach, unsigned int nFlags, Document& doc)
{
    const std::string sMem = Ach.CreateMemString();

    //	Deal with secret:
    auto sPostCode = tfm::format("%sSECRET%dSEC%s%dRE2%d",
        RAUsers::LocalUser().Username().c_str(),
        Ach.ID(),
        sMem.c_str(),
        Ach.Points(),
        Ach.Points() * 3);

    auto sPostCodeHash = RAGenerateMD5(sPostCode);

    PostArgs args;
    args['u'] = RAUsers::LocalUser().Username();
    args['t'] = RAUsers::LocalUser().Token();
    args['a'] = std::to_string(Ach.ID());
    args['g'] = std::to_string(g_pCurrentGameData->GetGameID());
    args['n'] = Ach.Title();
    args['d'] = Ach.Description();
    args['m'] = sMem;
    args['z'] = std::to_string(Ach.Points());
    args['f'] = std::to_string(nFlags);
    args['b'] = Ach.BadgeImageURI();
    args['h'] = sPostCodeHash;

    return(RAWeb::DoBlockingRequest(RequestSubmitAchievementData, args, doc));
}

void Dlg_Achievements::OnClickAchievementSet(AchievementSetType nAchievementSet)
{
    RASetAchievementCollection(nAchievementSet);

    switch (nAchievementSet)
    {
        // never saw the point of these "num" enums
    case AchievementSetType::NumAchievementSetTypes:
        _FALLTHROUGH;
    case AchievementSetType::Core:
    {
        Hide(m_hPromoteAch);
        SetText(m_hPromoteAch, TEXT("Demote from Core"));
        SetText(m_hCommitAch, TEXT("Commit Selected"));
        SetText(m_hDownloadAch, TEXT("Refresh from Server"));

        CancelMode(m_hAddAch); // Cannot add direct to Core
        Hide(m_hActivateAllAch);

        button_check(m_hActiveCore);
        button_uncheck(m_hActiveUnofficial);
        button_uncheck(m_hActiveLocal);
    }
    break;
    case AchievementSetType::Unofficial:
    {
        IRA_Dialog::Show(m_hPromoteAch);
        SetText(m_hPromoteAch, TEXT("Promote to Core"));
        SetText(m_hCommitAch, TEXT("Commit Selected"));
        SetText(m_hDownloadAch, TEXT("Refresh from Server"));

        CancelMode(m_hAddAch); // Cannot add direct to Unofficial

        Hide(m_hActivateAllAch);

        button_uncheck(m_hActiveCore);
        button_check(m_hActiveUnofficial);
        button_uncheck(m_hActiveLocal);
    }
    break;
    case AchievementSetType::Local:
    {
        IRA_Dialog::Show(m_hPromoteAch);
        SetText(m_hPromoteAch, TEXT("Promote to Unofficial"));
        SetText(m_hCommitAch, TEXT("Save All Local"));
        SetText(m_hDownloadAch, TEXT("Refresh from Disk"));

        Enable(m_hAddAch); // Can add to Local

        IRA_Dialog::Show(m_hActivateAllAch);

        button_uncheck(m_hActiveCore);
        button_uncheck(m_hActiveUnofficial);
        button_check(m_hActiveLocal);
    }
    } // end switch

    SetText(m_hPointTotal, NativeStr(std::to_string(g_pActiveAchievements->PointTotal())).c_str());

    Button_SetCheck(m_hChkAchProcessingActive, g_pActiveAchievements->ProcessingActive());
    OnLoad_NewRom(g_pCurrentGameData->GetGameID()); // assert: calls UpdateSelectedAchievementButtons
    g_AchievementEditorDialog.OnLoad_NewRom();
}



INT_PTR Dlg_Achievements::CommitAchievements(HWND hDlg)
{
    const int nMaxUploadLimit = 1;

    size_t nNumChecked = 0;
    using one_array = std::array<int, 1>;
    one_array nIDsChecked;
    one_array nLbxItemsChecked;

    int nSel = ListView_GetNextItem(m_hList, -1, LVNI_SELECTED);
    if (nSel != -1)
    {
        Achievement& Ach = g_pActiveAchievements->GetAchievement(to_unsigned(nSel));
        nLbxItemsChecked.front() = nSel;
        nIDsChecked.front() = to_signed(Ach.ID());

        nNumChecked++;
    }

    if (nNumChecked == 0)
        return FALSE;

    if (LocalValidateAchievementsBeforeCommit(nLbxItemsChecked) == FALSE)
        return FALSE;

    std::string title;

    if (nNumChecked == 1)
    {
        title = "Upload Achievement";
    }
    //else
    //{
    //	title = std::string("Upload ") + std::to_string( nNumChecked ) + std::string(" Achievements");
    //}


    auto message = tfm::format("Uploading the selected %d achievement(s).\n"
        "Are you sure? This will update the server with your new achievements\n"
        "and players will be able to download them into their games immediately.",
        nNumChecked);

    BOOL bErrorsEncountered = FALSE;

    if (MessageBox(hDlg, NativeStr(message).c_str(), NativeStr(title).c_str(), MB_YESNO | MB_ICONWARNING) == IDYES)
    {
        for (size_t i = 0; i < nNumChecked; ++i)
        {
            Achievement& NextAch = g_pActiveAchievements->GetAchievement(to_unsigned(nLbxItemsChecked.at(i)));

            BOOL bMovedFromUserToUnofficial = (g_nActiveAchievementSet == Local);

            unsigned int nFlags = 1 << 0;	//	Active achievements! : 1
            if (g_nActiveAchievementSet == Core)
                nFlags |= 1 << 1;			//	Core: 3
            else if (g_nActiveAchievementSet == Unofficial)
                nFlags |= 1 << 2;			//	Retain at Unofficial: 5
            else if (g_nActiveAchievementSet == Local)
                nFlags |= 1 << 2;			//	Promote to Unofficial: 5

            Document response;
            if (AttemptUploadAchievementBlocking(NextAch, nFlags, response))
            {
                if (response["Success"].GetBool())
                {
                    const AchievementID nAchID = response["AchievementID"].GetUint();
                    NextAch.SetID(nAchID);

                    //	Update listbox on achievements dlg

                    //sprintf_s( LbxDataAt( nLbxItemsChecked[i], 0 ), 32, "%d", nAchID );
                    LbxDataAt(to_unsigned(nLbxItemsChecked.at(i)), ach_col::ID) = std::to_string(nAchID);

                    if (bMovedFromUserToUnofficial)
                    {
                        //	Remove the achievement from the local/user achievement set,
                        //	 add it to the unofficial set.
                        Achievement& NewAch = g_pUnofficialAchievements->AddAchievement();
                        NewAch.Set(NextAch);
                        NewAch.SetModified(FALSE);
                        g_pLocalAchievements->RemoveAchievement(to_unsigned(nLbxItemsChecked.front()));
                        RemoveAchievement(nLbxItemsChecked.front());

                        //LocalAchievements->Save();
                        //UnofficialAchievements->Save();
                    }
                    else
                    {
                        //	Updated an already existing achievement, still the same position/ID.
                        NextAch.SetModified(FALSE);

                        //	Reverse find where I am in the list:
                        size_t nIndex = g_pActiveAchievements->GetAchievementIndex(*g_AchievementEditorDialog.ActiveAchievement());
                        ASSERT(nIndex < g_pActiveAchievements->NumAchievements());
                        if (nIndex < g_pActiveAchievements->NumAchievements())
                        {
                            if (g_nActiveAchievementSet == Core)
                                OnEditData(nIndex, ach_col::Modified, "No");
                        }

                        //	Save em all - we may have changed any of them :S
                        //CoreAchievements->Save();
                        //UnofficialAchievements->Save();
                        //LocalAchievements->Save();	// Will this one have changed? Maybe
                    }
                }
                else
                {
                    auto buffer = tfm::format("Error!!\n%s", std::string(response["Error"].GetString()).c_str());
                    show_error(buffer, hDlg);
                    bErrorsEncountered = TRUE;
                }
            }
        }

        if (bErrorsEncountered)
        {
            MessageBox(hDlg, TEXT("Errors encountered!\nPlease recheck your data, or get latest."), TEXT("Errors!"), MB_OK);
        }
        else
        {
            auto buffer = tfm::format("Successfully uploaded data for %d achievements!", nNumChecked);
            show_success(buffer, hDlg);

            RECT rcBounds;
            GetClientRect(m_hList, &rcBounds);
            InvalidateRect(m_hList, &rcBounds, FALSE);
        }
    }
    return TRUE;
}

void Dlg_Achievements::UpdateSelectedAchievementButtons(const Achievement* Cheevo)
{
    if (Cheevo == null)
    {
        CancelMode(m_hResetAch);
        CancelMode(m_hRevertSelected);
        Enable(m_hCommitAch, g_nActiveAchievementSet == Local);
        CancelMode(m_hCloneAch);
        CancelMode(m_hDelAch);
        CancelMode(m_hPromoteAch);
    }
    else
    {
        Enable(m_hRevertSelected, Cheevo->Modified());
        Enable(m_hCommitAch,
            g_nActiveAchievementSet == Local ? TRUE : Cheevo->Modified());
        Enable(m_hCloneAch, TRUE);
        Enable(m_hDelAch, g_nActiveAchievementSet == Local);
        Enable(m_hPromoteAch, TRUE);

        if (g_nActiveAchievementSet != Core)
        {
            Enable(m_hResetAch);
            SetText(m_hResetAch, Cheevo->Active() ? TEXT("Deactivate Selected") :
                TEXT("Activate Selected"));
        }
        else
        {
            Enable(m_hResetAch, !Cheevo->Active());
            SetText(m_hResetAch, TEXT("Activate Selected"));
        }
    }
}

// OK! Move semantics solved all my problems!
BOOL Dlg_Achievements
::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    m_hActivateAllAch         = GetDlgItem(hwnd, IDC_RA_ACTIVATE_ALL_ACH);
    m_hActiveCore             = GetDlgItem(hwnd, IDC_RA_ACTIVE_CORE);
    m_hActiveLocal            = GetDlgItem(hwnd, IDC_RA_ACTIVE_LOCAL);
    m_hActiveUnofficial       = GetDlgItem(hwnd, IDC_RA_ACTIVE_UNOFFICIAL);
    m_hAddAch                 = GetDlgItem(hwnd, IDC_RA_ADD_ACH);
    m_hChkAchProcessingActive = GetDlgItem(hwnd, IDC_RA_CHKACHPROCESSINGACTIVE);
    m_hCloneAch               = GetDlgItem(hwnd, IDC_RA_CLONE_ACH);
    m_hCommitAch              = GetDlgItem(hwnd, IDC_RA_COMMIT_ACH);
    m_hDelAch                 = GetDlgItem(hwnd, IDC_RA_DEL_ACH);
    m_hDownloadAch            = GetDlgItem(hwnd, IDC_RA_DOWNLOAD_ACH);
    m_hGameHash               = GetDlgItem(hwnd, IDC_RA_GAMEHASH);
    m_hList                   = GetDlgItem(hwnd, IDC_RA_LISTACHIEVEMENTS);
    m_hNumAch                 = GetDlgItem(hwnd, IDC_RA_NUMACH);
    m_hPointTotal             = GetDlgItem(hwnd, IDC_RA_POINT_TOTAL);
    m_hPromoteAch             = GetDlgItem(hwnd, IDC_RA_PROMOTE_ACH);
    m_hResetAch               = GetDlgItem(hwnd, IDC_RA_RESET_ACH);
    m_hRevertSelected         = GetDlgItem(hwnd, IDC_RA_REVERTSELECTED);

    // might make some wrappers for these liek un/check
    button_uncheck(m_hActiveCore);
    button_uncheck(m_hActiveLocal);
    button_uncheck(m_hActiveUnofficial);


    switch (g_nActiveAchievementSet)
    {
    case AchievementSetType::NumAchievementSetTypes:
        _FALLTHROUGH;
    case Core:
        button_check(m_hActiveCore);
        break;
    case Unofficial:
        button_check(m_hActiveUnofficial);
        break;
    case Local:
        button_check(m_hActiveUnofficial);
    }

    //	Continue as if a new rom had been loaded
    OnLoad_NewRom(g_pCurrentGameData->GetGameID());
    Button_SetCheck(m_hChkAchProcessingActive,
        g_pActiveAchievements->ProcessingActive());

    //	Click the core 
    OnClickAchievementSet(Core);

    RestoreWindowPosition(hwnd, lpCaption, false, true);
    return 0;
}

void Dlg_Achievements
::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    // this shit is too big to be one function, has a McAbe score of 80
    // Reduced to: 16, still kind of high but w/e we can use forwarders (maybe)
    switch (id)
    {
    case IDC_RA_ACTIVE_CORE:
        OnClickAchievementSet(Core);
        break;

    case IDC_RA_ACTIVE_UNOFFICIAL:
        OnClickAchievementSet(Unofficial);
        break;

    case IDC_RA_ACTIVE_LOCAL:
        OnClickAchievementSet(Local);
        break;

    case IDCLOSE:
        Close(hwnd); // seems pointless for modeless since it sends WM_CLOSE instead
        break;

    case IDC_RA_PROMOTE_ACH:
        OnPromoteAch(hwnd);
        break;

    case IDC_RA_DOWNLOAD_ACH:
        OnDownloadAch(hwnd);
        break;

    case IDC_RA_ADD_ACH:
        OnAddAch(hwnd);
        break;

    case IDC_RA_CLONE_ACH:
        OnCloneAch(hwnd);
        break;

    case IDC_RA_DEL_ACH:
        OnDelAch(hwnd);
        break;
    case IDC_RA_COMMIT_ACH:
        OnCommitAch(hwnd);
        break;

    case IDC_RA_REVERTSELECTED:
        OnRevertSelected(hwnd);
        break;

    case IDC_RA_CHKACHPROCESSINGACTIVE:
        g_pActiveAchievements
            ->SetPaused(Button_GetCheck(m_hChkAchProcessingActive));
        break;

    case IDC_RA_RESET_ACH:
        OnResetAch(hwnd);
        break;


    case IDC_RA_ACTIVATE_ALL_ACH:
        OnActivateAllAch(hwnd);

    }
}


#pragma region OnCommand
void Dlg_Achievements::OnActivateAllAch(HWND hwnd)
{
    if (!RA_GameIsActive() || g_pActiveAchievements->NumAchievements() == 0)
        return;

    if (MessageBox(hwnd, TEXT("Activate all achievements?"), TEXT("Activate Achievements"), MB_YESNO) == IDYES)
    {
        int nSel = ListView_GetNextItem(m_hList, -1, LVNI_SELECTED);

        for (size_t nIndex = 0; nIndex < g_pActiveAchievements->NumAchievements(); ++nIndex)
        {
            Achievement& Cheevo = g_pActiveAchievements->GetAchievement(nIndex);
            if (!Cheevo.Active())
            {
                Cheevo.Reset();
                Cheevo.SetActive(true);

                if (g_nActiveAchievementSet == Core)
                    OnEditData(nIndex, ach_col::Achieved, "No");
                else
                    OnEditData(nIndex, ach_col::Active, "Yes");

                if (nIndex == to_unsigned(nSel))
                    UpdateSelectedAchievementButtons(&Cheevo);
            }
        }
    }
}

void Dlg_Achievements::OnResetAch(HWND hwnd)
{

    if (!RA_GameIsActive())
        return;

    //	this could fuck up in so, so many ways. But fuck it, just reset achieved status. lmfao!
    int nSel = ListView_GetNextItem(m_hList, -1, LVNI_SELECTED);
    if (nSel != -1)
    {
        Achievement& Cheevo = g_pActiveAchievements->GetAchievement(to_unsigned(nSel));
        if (!Cheevo.Active())
        {
            auto sMessage = "Temporarily reset 'achieved' state of this achievement?"s;
            if (g_nActiveAchievementSet != Core)
                sMessage = TEXT("Activate this achievement?");

            if (MessageBox(hwnd, TEXT(sMessage.c_str()), TEXT("Activate Achievement"), MB_YESNO) == IDYES)
            {
                Cheevo.Reset();
                Cheevo.SetActive(true);

                size_t nIndex = g_pActiveAchievements->GetAchievementIndex(Cheevo);
                ASSERT(nIndex < g_pActiveAchievements->NumAchievements());
                if (nIndex < g_pActiveAchievements->NumAchievements())
                {
                    if (g_nActiveAchievementSet == Core)
                        OnEditData(nIndex, ach_col::Achieved, "No");
                    else
                        OnEditData(nIndex, ach_col::Active, "Yes");
                }

                //	Also needs to reinject text into IDC_RA_LISTACHIEVEMENTS
            }
        }
        else if (g_nActiveAchievementSet != Core)
        {
            if (MessageBox(hwnd, TEXT("Deactivate this achievement?"), TEXT("Deactivate Achievement"), MB_YESNO) == IDYES)
            {
                Cheevo.SetActive(false);

                size_t nIndex = g_pActiveAchievements->GetAchievementIndex(Cheevo);
                ASSERT(nIndex < g_pActiveAchievements->NumAchievements());
                if (nIndex < g_pActiveAchievements->NumAchievements())
                {
                    OnEditData(nIndex, ach_col::Active, "No");
                }

                //	Also needs to reinject text into IDC_RA_LISTACHIEVEMENTS
            }
        }

        UpdateSelectedAchievementButtons(&Cheevo);
    }
}

void Dlg_Achievements::OnRevertSelected(HWND hwnd)
{
    if (!RA_GameIsActive())
        return;

    //	Attempt to remove from list, but if it has an ID > 0, 
    //	 attempt to remove from DB
    int nSel = ListView_GetNextItem(m_hList, -1, LVNI_SELECTED);
    if (nSel != -1)
    {
        Achievement& Cheevo = g_pActiveAchievements->GetAchievement(to_unsigned(nSel));

        //	Ignore if it's not modified... no changes should be present...
        if (!Cheevo.Modified())
            return;

        if (MessageBox(hwnd,
            TEXT("Attempt to revert this achievement from file?"),
            TEXT("Revert from file?"), MB_YESNO) == IDYES)
        {
            //	Find Achievement with Ach.ID()
            unsigned int nID = Cheevo.ID();

            BOOL bFound = FALSE;

            AchievementSet TempSet(g_nActiveAchievementSet);
            if (TempSet.LoadFromFile(g_pCurrentGameData->GetGameID()))
            {
                Achievement* pAchBackup = TempSet.Find(nID);
                if (pAchBackup != null)
                {
                    Cheevo.Set(*pAchBackup);

                    Cheevo.SetActive(TRUE);
                    Cheevo.SetModified(FALSE);
                    //Cheevo.SetDirtyFlag();

                    //	Reverse find where I am in the list:
                    size_t nIndex = g_pActiveAchievements->GetAchievementIndex(Cheevo);
                    assert(nIndex < g_pActiveAchievements->NumAchievements());
                    if (nIndex < g_pActiveAchievements->NumAchievements())
                    {
                        if (g_nActiveAchievementSet == Core)
                            OnEditData(nIndex, ach_col::Achieved, "Yes");
                        else
                            OnEditData(nIndex, ach_col::Active, "No");

                        ReloadLBXData(to_signed(nIndex));
                    }

                    //	Finally, reselect to update AchEditor
                    g_AchievementEditorDialog.LoadAchievement(&Cheevo, FALSE);

                    bFound = TRUE;
                }
            }

            if (!bFound)
            {
                MessageBox(hwnd, TEXT("Couldn't find this achievement!"), TEXT("Error!"), MB_OK);
            }
            else
            {
                //MessageBox( HWnd, "Reverted", "OK!", MB_OK );
            }
        }
    }

}

void Dlg_Achievements::OnCommitAch(HWND hwnd)
{
    if (!RA_GameIsActive())
    {
        return;
    }

    if (g_nActiveAchievementSet == Local)
    {
        // Local save is to disk
        if (g_pActiveAchievements->SaveToFile())
        {
            MessageBox(hwnd, TEXT("Saved OK!"), TEXT("OK"), MB_OK);
            for (unsigned int i = 0; i < g_pActiveAchievements->NumAchievements(); i++)
                g_pActiveAchievements->GetAchievement(i).SetModified(FALSE);

            InvalidateRect(hwnd, null, FALSE);
            return;
        }
        else
        {
            show_error("Error during save!"s);
            return;
        }
        return;
    }
    else
    {
        CommitAchievements(hwnd);
    }
}

void Dlg_Achievements::OnDelAch(HWND hwnd)
{

    if (!RA_GameIsActive())
    {
        no_rom_loaded();
        return;
    }

    //	Attempt to remove from list, but if it has an ID > 0, 
    //	 attempt to remove from DB
    int nSel = ListView_GetNextItem(m_hList, -1, LVNI_SELECTED);
    if (nSel != -1)
    {
        Achievement& Ach = g_pActiveAchievements->GetAchievement(to_unsigned(nSel));

        if (Ach.ID() == 0)
        {
            //	Local achievement
            if (MessageBox(hwnd, TEXT("Are you sure that you want to remove this achievement?"), TEXT("Remove Achievement"), MB_YESNO | MB_ICONWARNING) == IDYES)
            {
                g_pActiveAchievements->RemoveAchievement(to_unsigned(nSel));
                RemoveAchievement(nSel);
            }
        }
        else
        {
            auto msg{
                tfm::format("This achievement exists on %s.\n\n"
                "*Removing it will affect other gamers*\n\n"
                    "Are you absolutely sure you want to delete this??",
                    RA_HOST_URL)
            };

            //	This achievement exists on the server: must call SQL to remove!
            //	Note: this is probably going to affect other users: frown on this D:
            show_confirmation(msg);
        }
    }

}

void Dlg_Achievements::OnCloneAch(HWND hwnd)
{
    if (!RA_GameIsActive())
    {
        no_rom_loaded();
        return;
    }

    int nSel = ListView_GetNextItem(m_hList, -1, LVNI_SELECTED);
    if (nSel == -1)
        return; // throw?

                //	switch to LocalAchievements
    Achievement& NewClone = g_pLocalAchievements->AddAchievement();

    //	Clone TO the user achievements
    Achievement& Ach = g_pActiveAchievements->GetAchievement(to_unsigned(nSel));

    NewClone.Set(Ach);
    NewClone.SetID(0);
    NewClone.SetAuthor(RAUsers::LocalUser().Username());

    auto buffer = tfm::format("%s (copy)", NewClone.Title());
    NewClone.SetTitle(buffer);

    OnClickAchievementSet(Local);

    ListView_SetItemState(m_hList, g_pLocalAchievements->NumAchievements() - 1_z, LVIS_FOCUSED | LVIS_SELECTED, -1);
    ListView_EnsureVisible(m_hList, g_pLocalAchievements->NumAchievements() - 1_z, FALSE);

    buffer = tfm::format(" %d", g_pActiveAchievements->NumAchievements());
    SetText(m_hNumAch, NativeStr(buffer).c_str());
    SetText(m_hPointTotal, NativeStr(std::to_string(g_pActiveAchievements->PointTotal())).c_str());

    NewClone.SetModified(TRUE);
    UpdateSelectedAchievementButtons(&NewClone);
}

void Dlg_Achievements::OnAddAch(HWND hwnd)
{
    if (!RA_GameIsActive())
    {
        MessageBox(hwnd, TEXT("ROM not loaded: please load a ROM first!"), TEXT("Error!"), MB_OK);
        return;
    }

    //	Add a new achievement with default params
    // Gonna leave a alone for now, but I think left hand side should be moved if temporary (Achievement&&)
    Achievement& Cheevo = g_pActiveAchievements->AddAchievement();
    Cheevo.SetAuthor(RAUsers::LocalUser().Username());
    Cheevo.SetBadgeImage("00000");

    //	Reverse find where I am in the list:
    unsigned int nOffset = 0;
    for (; nOffset < g_pActiveAchievements->NumAchievements(); ++nOffset)
    {
        if (&Cheevo == &g_pActiveAchievements->GetAchievement(nOffset))
            return;
    }
    ASSERT(nOffset < g_pActiveAchievements->NumAchievements());
    if (nOffset < g_pActiveAchievements->NumAchievements())
        OnEditData(nOffset, ach_col::Author, Cheevo.Author());

    auto nNewID = AddAchievement(Cheevo);
    ListView_SetItemState(m_hList, nNewID, LVIS_FOCUSED | LVIS_SELECTED, -1);
    ListView_EnsureVisible(m_hList, nNewID, FALSE);


    auto buffer = tfm::format(" %d", g_pActiveAchievements->NumAchievements());
    SetText(m_hNumAch, NativeStr(buffer).c_str());

    Cheevo.SetModified(TRUE);
    UpdateSelectedAchievementButtons(&Cheevo);
}

void Dlg_Achievements::OnDownloadAch(HWND hwnd)
{

    auto msg{ tfm::format("Are you sure that you want to download fresh achievements from %s?\n"
        "This will overwrite any changes that you have made with fresh achievements from the server.\n",
        "Refresh from Server", RA_HOST_URL) };
    if (!RA_GameIsActive())
    {
        return;
    }

    if (g_nActiveAchievementSet == Local)
    {
        if (MessageBox(hwnd,
            TEXT("Are you sure that you want to reload achievements from disk?\n")
            TEXT("This will overwrite any changes that you have made.\n"),
            TEXT("Refresh from Disk"),
            MB_YESNO | MB_ICONWARNING) == IDYES)
        {
            GameID nGameID = g_pCurrentGameData->GetGameID();
            if (nGameID != 0)
            {
                g_pLocalAchievements->Clear();
                g_pLocalAchievements->LoadFromFile(nGameID);

                //	Refresh dialog contents:
                OnLoad_NewRom(nGameID);

                //	Cause it to reload!
                OnClickAchievementSet(g_nActiveAchievementSet);
            }
        }
    }
    else if (MessageBox(hwnd, TEXT(msg.c_str()),
        TEXT("Refresh from Server"), MB_YESNO | MB_ICONWARNING) == IDYES)
    {
        GameID nGameID = g_pCurrentGameData->GetGameID();
        if (nGameID != 0)
        {
            g_pCoreAchievements->DeletePatchFile(nGameID);
            g_pUnofficialAchievements->DeletePatchFile(nGameID);

            g_pCoreAchievements->Clear();
            g_pUnofficialAchievements->Clear();
            g_pLocalAchievements->Clear();

            //	Reload the achievements file (fetch from server fresh)

            AchievementSet::FetchFromWebBlocking(nGameID);

            g_pLocalAchievements->LoadFromFile(nGameID);
            g_pUnofficialAchievements->LoadFromFile(nGameID);
            g_pCoreAchievements->LoadFromFile(nGameID);

            //	Refresh dialog contents:
            OnLoad_NewRom(nGameID);

            //	Cause it to reload!
            OnClickAchievementSet(g_nActiveAchievementSet);
        }
    }

}

void Dlg_Achievements::OnPromoteAch(HWND hwnd)
{
    //	Replace with background upload?
    if (!RA_GameIsActive())
    {
        no_rom_loaded();
        return;
    }

    ProceedAchPromotion(hwnd);

}

void Dlg_Achievements::ProceedAchPromotion(HWND hwnd)
{
    switch (g_nActiveAchievementSet)
    {
    case AchievementSetType::NumAchievementSetTypes:
    case AchievementSetType::Core:
        // I'm guessing you don't want to break if it's core but just skip it
        _FALLTHROUGH;
        // Promote from Local to Unofficial is just a commit to Unofficial
    case AchievementSetType::Local:
        CommitAchievements(hwnd);
        break;
    case AchievementSetType::Unofficial:
        PromoteFromUnofficial(hwnd);
    }
}

void Dlg_Achievements::PromoteFromUnofficial(HWND hwnd)
{

    int nSel = ListView_GetNextItem(m_hList, -1, LVNI_SELECTED);
    if (nSel == -1)
    {
        return;
    }

    //	Promote to Core
    auto message{ "Promote this achievement to the Core Achievement set.\n\n"s
        "Please note this is a one-way operation, and will allow players\n"s
        "to officially obtain this achievement and the points for it.\n"s };
    //	Note: specify that this is a one-way operation
    if (show_confirmation(message) == IDYES)
    {
        // temp
        Achievement& selectedAch = g_pActiveAchievements->GetAchievement(to_unsigned(nSel));

        unsigned int nFlags = 1 << 0;	//	Active achievements! : 1
        if (g_nActiveAchievementSet == Unofficial)
            nFlags |= 1 << 1;			//	Official achievements: 3


        Document response;
        if (AttemptUploadAchievementBlocking(selectedAch, nFlags, response))
        {
            if (response["Success"].GetBool())
            {
                const unsigned int nID = response["AchievementID"].GetUint();

                //	Remove the achievement from the local/user achievement set,
                //	 add it to the unofficial set.
                Achievement& newAch = g_pCoreAchievements->AddAchievement();
                newAch.Set(selectedAch);
                g_pUnofficialAchievements->RemoveAchievement(to_unsigned(nSel));
                RemoveAchievement(nSel);

                newAch.SetActive(TRUE);	//	Disable it: all promoted ach must be reachieved

                                        //CoreAchievements->Save();
                                        //UnofficialAchievements->Save();
                show_success("Successfully uploaded achievement!"s);
            }
            else
            {
                MessageBox(hwnd,
                    NativeStr(std::string("Error in upload: response from server:") + std::string(response["Error"].GetString())).c_str(),
                    TEXT("Error in upload!"), MB_OK);
            }
        }
        else
        {
            MessageBox(hwnd, TEXT("Error connecting to server... are you online?"), TEXT("Error in upload!"), MB_OK);
        }
    }

}
#pragma endregion


LRESULT Dlg_Achievements
::OnNotify(HWND hwnd, int idFrom, NMHDR* pnmhdr)
{
    // First code is huge - 4294967284
    switch (pnmhdr->code)
    {
    case LVN_ITEMCHANGED:	//!?? LVN on a LPNMHDR?
    {
        iSelect = -1;
        // Need to check the cracker real quick
        // HANDLE_WM_NOTIFY

        //MessageBox( nullptr, "Item changed!", "TEST", MB_OK );
        auto pLVInfo = reinterpret_cast<LPNMLISTVIEW>(pnmhdr);
        if (pLVInfo->iItem != -1)
        {
            iSelect = pLVInfo->iItem;
            if ((pLVInfo->uNewState &= LVIS_SELECTED) != 0)
            {
                int nNewIndexSelected = pLVInfo->iItem;
                Achievement& Cheevo = g_pActiveAchievements->GetAchievement(to_unsigned(nNewIndexSelected));
                g_AchievementEditorDialog.LoadAchievement(&Cheevo, FALSE);

                UpdateSelectedAchievementButtons(&Cheevo);
            }
        }
        else
        {
            UpdateSelectedAchievementButtons(null);
        }
    }
    break;

    case NM_DBLCLK:
        if (reinterpret_cast<LPNMITEMACTIVATE>(pnmhdr)->iItem != -1)
        {
            OnCommand(g_RAMainWnd, IDM_RA_FILES_ACHIEVEMENTEDITOR, null, 0);
            g_AchievementEditorDialog.LoadAchievement(
                &g_pActiveAchievements->GetAchievement(to_unsigned(
                    reinterpret_cast<LPNMITEMACTIVATE>(pnmhdr)->iItem)), FALSE);
        }
        return 0_i;	//? TBD ##SD

    case NM_CUSTOMDRAW:
        // Ok it came here, let's see what it does
        cdc.OnNMCustomdraw(pnmhdr);
        return 1_i;
    }
    return 0_i;
}



void Dlg_Achievements::OnNCDestroy(HWND hwnd)
{
    DestroyControl(m_hActivateAllAch);
    DestroyControl(m_hActiveCore);
    DestroyControl(m_hActiveLocal);
    DestroyControl(m_hActiveUnofficial);
    DestroyControl(m_hAddAch);
    DestroyControl(m_hChkAchProcessingActive);
    DestroyControl(m_hCloneAch);
    DestroyControl(m_hCommitAch);
    DestroyControl(m_hDelAch);
    DestroyControl(m_hDownloadAch);
    DestroyControl(m_hGameHash);
    DestroyControl(m_hList);
    DestroyControl(m_hNumAch);
    DestroyControl(m_hPointTotal);
    DestroyControl(m_hPromoteAch);
    DestroyControl(m_hResetAch);
    DestroyControl(m_hRevertSelected);
    IRA_Dialog::OnNCDestroy(hwnd); // this hwnd is m_hAchievementsDlg
}

INT_PTR Dlg_Achievements
::DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// If this works, this will be awesome, should work in theory
	return IRA_Dialog::DialogProc(hwnd, uMsg, wParam, lParam);
}

void Dlg_Achievements::LoadAchievements()
{
    SetupColumns();

    // another damn access violation... do you know C?
    // The moment of truth...

    // old loops always mess things up, much better
    for (auto& i : g_pActiveAchievements->GetContainer())
        AddAchievement(i);
}

void Dlg_Achievements::OnLoad_NewRom(GameID nGameID)
{
    // WM_CANCELMODE is the opposite of WM_ENABLE
    // CancelMode -> EnableWindow(hwnd, FALSE)
    // Already handled in the message queue

    // I didn't load a new rom?

    CancelMode(m_hDownloadAch);
    CancelMode(m_hAddAch);
    CancelMode(m_hActivateAllAch);
    CancelMode(m_hPromoteAch);

    // The m_hList will never be null unless there's some bug
    LoadAchievements(); // Ok there was something wrong here


    auto buffer = tfm::format(" %d", nGameID);
    SetText(m_hGameHash, NativeStr(buffer).c_str());

    if (nGameID != 0)
    {
        Enable(m_hDownloadAch);
        Enable(m_hAddAch, g_nActiveAchievementSet == Local);
        Enable(m_hActivateAllAch);
        Enable(m_hPromoteAch);
    }

    buffer = tfm::format(" %d", g_pActiveAchievements->NumAchievements());
    SetText(m_hNumAch, NativeStr(buffer).c_str());
    SetText(m_hPointTotal,
        NativeStr(std::to_string(g_pActiveAchievements->PointTotal())).c_str());


    UpdateSelectedAchievementButtons(null);
}

void Dlg_Achievements::OnGet_Achievement(const Achievement& ach)
{
    size_t nIndex = g_pActiveAchievements->GetAchievementIndex(ach);

    if (g_nActiveAchievementSet == Core)
        OnEditData(nIndex, ach_col::Achieved, "Yes");
    else
        OnEditData(nIndex, ach_col::Active, "No");

    UpdateSelectedAchievementButtons(&ach);
}

void Dlg_Achievements::OnEditAchievement(const Achievement& ach)
{
    size_t nIndex = g_pActiveAchievements->GetAchievementIndex(ach);
    ASSERT(nIndex < g_pActiveAchievements->NumAchievements());
    if (nIndex < g_pActiveAchievements->NumAchievements())
    {
        OnEditData(nIndex, ach_col::Points, std::to_string(ach.Points()));

        SetText(m_hPointTotal, NativeStr(std::to_string(
            g_pActiveAchievements->PointTotal())).c_str());

        if (g_nActiveAchievementSet == Core)
            OnEditData(nIndex, ach_col::Modified, "Yes");

        // Achievement stays active after edit, so this print is unnecessary.
        /*else
            OnEditData( nIndex, Dlg_Achievements::Active, "No" );*/
    }

    UpdateSelectedAchievementButtons(&ach);
}

void Dlg_Achievements::ReloadLBXData(int nOffset)
{
    //const char* g_sColTitles[]			= { "ID", "Title", "Author", "Achieved?", "Modified?" };
    //const char* g_sColTitlesUnofficial[]  = { "ID", "Title", "Author", "Active", "Votes" };
    auto unOffset{ to_unsigned(nOffset) };
    Achievement& Ach = g_pActiveAchievements->GetAchievement(unOffset);

    if (g_nActiveAchievementSet == Core)
    {
        OnEditData(unOffset, ach_col::Title, Ach.Title());
        OnEditData(unOffset, ach_col::Author, Ach.Author());
        OnEditData(unOffset, ach_col::Achieved, !Ach.Active() ? "Yes" : "No");
        OnEditData(unOffset, ach_col::Modified, Ach.Modified() ? "Yes" : "No");
    }
    else
    {
        OnEditData(unOffset, ach_col::Title, Ach.Title());
        OnEditData(unOffset, ach_col::Author, Ach.Author());
        OnEditData(unOffset, ach_col::Active, Ach.Active() ? "Yes" : "No");
        OnEditData(unOffset, ach_col::Votes, "N/A");
    }

    UpdateSelectedAchievementButtons(&Ach);
}

void Dlg_Achievements::OnEditData(size_t nItem, Column nColumn, const std::string& sNewData)
{
    if (nItem >= m_lbxData.size())
        return;

    auto current_entry{ LbxDataAt(nItem, nColumn) };
    current_entry = sNewData;

    auto col_as_int{ static_cast<int>(etoi(nColumn)) };
    if (m_hList != null)
    {
        LV_ITEM item{
            LVIF_TEXT, static_cast<int>(nItem), col_as_int, 0_z,
            0_z, current_entry.data(), 256, 0, 0_i, 0, 0, 0_z, null, null, 0
        };


        if (ListView_SetItem(m_hList, &item) == FALSE)
        {
            ASSERT(!"Failed to ListView_SetItem!");
        }
        else
        {
            RECT rcBounds;
            ListView_GetItemRect(m_hList, nItem, &rcBounds, LVIR_BOUNDS);
            InvalidateRect(m_hList, &rcBounds, FALSE);
        }
    }
}

int Dlg_Achievements::GetSelectedAchievementIndex()
{
    return ListView_GetSelectionMark(m_hList);
}
LRESULT CustomDrawControl::OnNMCustomdraw(LPNMHDR lpnmdr)
{
    // First draw stage is 1 (CDDS_PREPAINT):
    // OK now it's 3 (CDDS_PREERASE) after coloring in teh active
	// achievement group
    // Now it's 1 again
    auto lplvcd = reinterpret_cast<LPNMLVCUSTOMDRAW>(lpnmdr);
    switch (lplvcd->nmcd.dwDrawStage)
    {
    case CDDS_PREPAINT:
        //	Before the paint cycle begins
        //	request notifications for individual listview items
        return CDRF_NOTIFYITEMDRAW;

    case CDDS_ITEMPREPAINT: //Before an item is drawn
    {


        if (auto nNextItem = static_cast<int>(lplvcd->nmcd.dwItemSpec);
        to_unsigned(nNextItem) < g_pActiveAchievements->NumAchievements())
        {
            auto unNextItem{ to_unsigned(nNextItem) };
            //if (((int)lplvcd->nmcd.dwItemSpec%2)==0)
            auto bSelected = &g_pActiveAchievements->GetAchievement(unNextItem) == g_AchievementEditorDialog.ActiveAchievement();
            auto bModified = g_pActiveAchievements->GetAchievement(unNextItem).Modified();

            lplvcd->clrText = bModified ? COL_RED : COL_BLACK;
            lplvcd->clrTextBk = bSelected ? COL_GAINSBORO : COL_WHITE;
        }
        return CDRF_NEWFONT;
    }
    break;

    //Before a subitem is drawn
    // 	case CDDS_SUBITEM | CDDS_ITEMPREPAINT: 
    // 		{
    // 			if (iSelect == (int)lplvcd->nmcd.dwItemSpec)
    // 			{
    // 				if (0 == lplvcd->iSubItem)
    // 				{
    // 					//customize subitem appearance for column 0
    // 					lplvcd->clrText   = RGB(255,128,0);
    // 					lplvcd->clrTextBk = RGB(255,255,255);
    // 
    // 					//To set a custom font:
    // 					//SelectObject(lplvcd->nmcd.hdc, 
    // 					//    <your custom HFONT>);
    // 
    // 					return CDRF_NEWFONT;
    // 				}
    // 				else if (1 == lplvcd->iSubItem)
    // 				{
    // 					//customize subitem appearance for columns 1..n
    // 					//Note: setting for column i 
    // 					//carries over to columnn i+1 unless
    // 					//      it is explicitly reset
    // 					lplvcd->clrTextBk = RGB(255,0,0);
    // 					lplvcd->clrTextBk = RGB(255,255,255);
    // 
    // 					return CDRF_NEWFONT;
    // 				}
    // 			}
    // 		}
    }
    return CDRF_DODEFAULT;
}
} // namespace ra
