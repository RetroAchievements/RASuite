#ifndef DLG_ACHIEVEMENTSREPORTER_H
#define DLG_ACHIEVEMENTSREPORTER_H
#pragma once

#include "IRA_Dialog.h"
#include "RA_Defs.h"

class Achievement;


namespace ra {
enum class ReporterColumns : std::size_t
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
	~Dlg_AchievementsReporter() noexcept = default;
	Dlg_AchievementsReporter(const Dlg_AchievementsReporter&) = delete;
	Dlg_AchievementsReporter& operator=(const Dlg_AchievementsReporter&) = delete;
	Dlg_AchievementsReporter(Dlg_AchievementsReporter&&) noexcept = default;
	Dlg_AchievementsReporter& operator=(Dlg_AchievementsReporter&&) noexcept = default;

    // If you want to make it modeless you can
	HWND Create() override {
		throw std::runtime_error{ "This isn't a modeless dialog!" };
		return null;
	}

	HWND GetWindow() const noexcept override;

	void SetupColumns();
	int AddAchievementToListBox(const Achievement* pAch);

	INT_PTR DoModal() override;
protected:
	// Inherited via IRA_Dialog
	BOOL OnInitDialog(HWND hwnd, _UNUSED HWND hwndFocus, 
		_UNUSED LPARAM lParam) override;
	void OnCommand(HWND hwnd, int id, _UNUSED HWND hwndCtl, 
		_UNUSED UINT codeNotify) override;
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
