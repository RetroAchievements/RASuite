#ifndef DLG_ACHIEVEMENTSREPORTER_H
#define DLG_ACHIEVEMENTSREPORTER_H
#pragma once

#include "IRA_Dialog.h"
#include "RA_Defs.h"

class Achievement;


namespace ra {
enum class ReporterColumns
{
	Checked,
	//ID,
	Title,
	Desc,
	Author,
	Achieved,

	NumReporterColumns
};



constexpr auto MAX_ACHIEVEMENTS{ 200 };
constexpr auto MAX_TEXT_LEN{ 250 };
constexpr auto num_rep_cols{ etoi(ReporterColumns::NumReporterColumns) };

using RepColArr = std::array<ReporterColumns, num_rep_cols>;

constexpr RepColArr repcol_arr
{
	ReporterColumns::Checked,
	ReporterColumns::Title,
	ReporterColumns::Desc,
	ReporterColumns::Author,
	ReporterColumns::Achieved
};


class Dlg_AchievementsReporter : public IRA_Dialog
{
	// crap I had these backwards
	// anyway std::array deallocates pointers so it's good
	// actually, it keeps on getting access violations
	using row   = std::array<std::string, num_rep_cols>;
	using table = std::array<row, MAX_ACHIEVEMENTS>;
public:
	Dlg_AchievementsReporter();
	void SetupColumns(HWND hList);
	int AddAchievementToListBox(HWND hList, const Achievement* pAch);

	INT_PTR DoModal() override;
protected:
	// Inherited via IRA_Dialog
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) override;
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) override;
	void OnNCDestroy(HWND hwnd) override;
	void OnOK(HWND hwnd) override;
	INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam,
		LPARAM lParam) override;

private:
	int ms_nNumOccupiedRows{ 0 };
	table ms_lbxData;
	HWND m_hList{ nullptr };
	HWND m_hBugReporter{ nullptr };
	HWND m_hProblem1{ nullptr };
	HWND m_hProblem2{ nullptr };
	HWND m_hComment{ nullptr };
};

} // namespace ra






#endif // !DLG_ACHIEVEMENTSREPORTER_H
