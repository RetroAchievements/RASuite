#ifndef RA_DLG_ACHIEVEMENT_H
#define RA_DLG_ACHIEVEMENT_H
#pragma once

#include "RA_Achievement.h"
#include "IRA_Dialog.h"

namespace ra {
constexpr int MAX_TEXT_ITEM_SIZE = 80;


enum class Column : std::size_t
{
    ID,
    Title,
    Points,
    Author,
    Achieved,
    Active=Achieved,
    Modified,
    Votes=Modified,

    NUM_COLS
};
using ach_col = Column;

// Not sure if this should be used in a general case or just for Dlg_Achievements
class CustomDrawControl
{
public:
    LRESULT OnNMCustomdraw(LPNMHDR lpnmdr);
};


// I really don't what's wrong with this, it appears if you load achievement set without a rom first and then load a rom it works
// If you load the rom first it doesn't work
class Dlg_Achievements : public IRA_Dialog
{
    // seems to me you want an array
    // I just don't give a shit, this thing keeps on breaking it'll be a vector again
    using AchievementDlgRow = std::vector<std::string>;
    using LbxData           = std::vector<AchievementDlgRow>;

public:
	Dlg_Achievements();
	~Dlg_Achievements() noexcept = default;
	Dlg_Achievements(const Dlg_Achievements&) = delete;
	Dlg_Achievements& operator=(const Dlg_Achievements&) = delete;
	Dlg_Achievements(Dlg_Achievements&& b) noexcept;
	Dlg_Achievements& operator=(Dlg_Achievements&& b) noexcept;

public:
	BOOL Show() { return ShowWindow(m_hAchievementsDlg, SW_SHOW); }
	// Noticed achievement editor needs it
	_NODISCARD HWND GetList() noexcept { return m_hList; }
public:
	int GetSelectedAchievementIndex();
	void OnLoad_NewRom(GameID nGameID);
	void OnGet_Achievement(const Achievement& ach);
	void ReloadLBXData(int nOffset);
	void OnEditData(size_t nItem, Column nColumn, const std::string& sNewData);
	void OnEditAchievement(const Achievement& ach);
	void OnClickAchievementSet(AchievementSetType nAchievementSet);

	std::string& LbxDataAt(size_t nRow, Column nCol) {
		return(m_lbxData[nRow])[etoi(nCol)];
	}

	HWND GetWindow() const noexcept { return m_hAchievementsDlg; }
	void SetWindow() {
		m_hAchievementsDlg = Create();
	}
protected:
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) override;
	void OnCommand(HWND hwnd, int id, HWND hwndCtl,
		UINT codeNotify) override;



	



	

	

	

	
	
	LRESULT OnNotify(HWND hwnd, int idFrom, NMHDR* pnmhdr) override;
	void OnNCDestroy(HWND hwnd) override;
	INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam,
		LPARAM lParam) override;
private:
	void SetupColumns();
	void LoadAchievements();

	void RemoveAchievement(int nIter);
	size_t AddAchievement(const Achievement& Ach);

	INT_PTR CommitAchievements(HWND hDlg);

	void UpdateSelectedAchievementButtons(const Achievement* Cheevo);

    // Had to split on control event to reduce complexity
    // IDC_RA_PROMOTE_ACH
	void OnPromoteAch(HWND hwnd);
	void ProceedAchPromotion(HWND hwnd);
	void PromoteFromUnofficial(HWND hwnd);

	void OnDownloadAch(HWND hwnd);
	void OnAddAch(HWND hwnd);
	void OnCloneAch(HWND hwnd);
	void OnDelAch(HWND hwnd);
	void OnCommitAch(HWND hwnd);
	void OnRevertSelected(HWND hwnd);
	void OnActivateAllAch(HWND hwnd);
	void OnResetAch(HWND hwnd);
private:
	// For most of these don't use the destructor use OnNCDestroy
    // Don't initilize in contructor either, use OnInitDialog or as needed
    // (makes no difference)
	HWND m_hActivateAllAch{ null };
	HWND m_hActiveCore{ null };
	HWND m_hActiveLocal{ null };
	HWND m_hActiveUnofficial{ null };
	HWND m_hAddAch{ null };
	HWND m_hChkAchProcessingActive{ null };
	HWND m_hCloneAch{ null };
	HWND m_hCommitAch{ null };
	HWND m_hDelAch{ null };
	HWND m_hDownloadAch{ null };
	HWND m_hGameHash{ null };
	HWND m_hList{ null };
	HWND m_hNumAch{ null };
	HWND m_hPointTotal{ null };
	HWND m_hPromoteAch{ null };
	HWND m_hResetAch{ null };
	HWND m_hRevertSelected{ null };
    // don't nullfy this explicity it's already handled in OnNCDestroy as hwnd
	HWND m_hAchievementsDlg{ null };

    // Don't worry about destroying these two, they're non-static objects
    CustomDrawControl cdc;
	typedef std::vector< std::string > AchievementDlgRow;
	using LbxData = std::vector< AchievementDlgRow >;

    // Is this not default constructible?
	LbxData m_lbxData{ LbxData{} };


	
};
} // namespace ra

extern _RA Dlg_Achievements g_AchievementsDialog;


#endif // !RA_DLG_ACHIEVEMENT_H
