#ifndef ACHIEVEMENT_H
#define ACHIEVEMENT_H
#pragma once

#include "RA_Condition.h"
#include "RA_Defs.h"


//////////////////////////////////////////////////////////////////////////
//	Achievement
//////////////////////////////////////////////////////////////////////////
namespace ra {


enum class Achievement_DirtyFlags : std::size_t
{
	not_dirty   = 0,
	title		= 1 << 0,
	desc		= 1 << 1,
	points		= 1 << 2,
	author	    = 1 << 3,
	id		    = 1 << 4,
	badge		= 1 << 5,
	conditions  = 1 << 6,
	votes		= 1 << 7,
	description = 1 << 8,

	// this will cause a stack overflow 
	Dirty__All
};

// Shorter
using dirty_flag = Achievement_DirtyFlags;
// recommend only using this when scope resolution is needed
using df          = dirty_flag;

constexpr bool operator|(const dirty_flag a, const dirty_flag b) noexcept
{
	return etoi(a) | etoi(b);
}

constexpr bool operator&(const dirty_flag a, const dirty_flag b) noexcept
{
	return etoi(a) & etoi(b);
}

constexpr bool operator|=(dirty_flag a, const dirty_flag b) noexcept
{
	auto result{ a | b };
    return result;
}

constexpr bool operator&=(dirty_flag a, const dirty_flag b) noexcept
{
	auto result{ a & b };
	return result;
}

} // namespace ra

class Achievement
{
	
	// From the way it is implemented, an unordered_map might be more appropriate
	using Conditions = std::vector<ConditionSet>;
public:
	Achievement(AchievementSetType nType);

public:
	void Clear();
	BOOL Test();

	size_t AddCondition(size_t nConditionGroup, const Condition& pNewCond);
	BOOL RemoveCondition(size_t nConditionGroup, unsigned int nConditionID);
	void RemoveAllConditions(size_t nConditionGroup);

	void Set(const Achievement& rRHS);

	inline BOOL Active() const { return m_bActive; }
	void SetActive(BOOL bActive);

	inline BOOL Modified() const { return m_bModified; }
	void SetModified(BOOL bModified);

	inline BOOL GetPauseOnTrigger() const { return m_bPauseOnTrigger; }
	void SetPauseOnTrigger(BOOL bPause) { m_bPauseOnTrigger = bPause; }

	inline BOOL GetPauseOnReset() const { return m_bPauseOnReset; }
	void SetPauseOnReset(BOOL bPause) { m_bPauseOnReset = bPause; }

	BOOL IsCoreAchievement() const { return m_nSetType == Core; }

	void SetID(AchievementID nID);
	inline AchievementID ID() const { return m_nAchievementID; }

	inline const std::string& Title() const { return m_sTitle; }
	void SetTitle(const std::string& sTitle) { m_sTitle = sTitle; }
	inline const std::string& Description() const { return m_sDescription; }
	void SetDescription(const std::string& sDescription) { m_sDescription = sDescription; }
	inline const std::string& Author() const { return m_sAuthor; }
	void SetAuthor(const std::string& sAuthor) { m_sAuthor = sAuthor; }
	inline unsigned int Points() const { return m_nPointValue; }
	void SetPoints(unsigned int nPoints) { m_nPointValue = nPoints; }

	inline time_t CreatedDate() const { return m_nTimestampCreated; }
	void SetCreatedDate(time_t nTimeCreated) { m_nTimestampCreated = nTimeCreated; }
	inline time_t ModifiedDate() const { return m_nTimestampModified; }
	void SetModifiedDate(time_t nTimeModified) { m_nTimestampModified = nTimeModified; }

	inline unsigned short Upvotes() const { return m_nUpvotes; }
	inline unsigned short Downvotes() const { return m_nDownvotes; }

	inline const std::string& Progress() const { return m_sProgress; }
	void SetProgressIndicator(const std::string& sProgress) { m_sProgress = sProgress; }
	inline const std::string& ProgressMax() const { return m_sProgressMax; }
	void SetProgressIndicatorMax(const std::string& sProgressMax) { m_sProgressMax = sProgressMax; }
	inline const std::string& ProgressFmt() const { return m_sProgressFmt; }
	void SetProgressIndicatorFormat(const std::string& sProgressFmt) { m_sProgressFmt = sProgressFmt; }


	void AddConditionGroup();
	void RemoveConditionGroup();

	inline size_t NumConditionGroups() const { return m_vConditions.size(); }
	inline size_t NumConditions(size_t nGroup) const { return m_vConditions[nGroup].Count(); }

	inline HBITMAP BadgeImage() const { return m_hBadgeImage; }
	inline HBITMAP BadgeImageLocked() const { return m_hBadgeImageLocked; }
	inline const std::string& BadgeImageURI() const { return m_sBadgeImageURI; }

	void SetBadgeImage(const std::string& sFilename);
	void ClearBadgeImage();

	Condition& GetCondition(size_t nCondGroup, size_t i) { return m_vConditions[nCondGroup].GetAt(i); }

	std::string CreateMemString() const;

	void Reset();

	//void UpdateProgress();
	//float ProgressGetNextStep( char* sFormat, float fLastKnownProgress );
	//float ParseProgressExpression( char* pExp );

	//	Returns the new char* offset after parsing.
	char* ParseLine(char* buffer);

	//	Parse from json element
	void Parse(const Value& element);

	char* ParseMemString(char* sMem);

	//	Used for rendering updates when editing achievements. Usually always false.
	_RA dirty_flag GetDirtyFlags() const { return m_nDirtyFlags; }
	bool IsDirty() const { return (m_nDirtyFlags != _RA df::not_dirty); }
	void SetDirtyFlag(_RA dirty_flag nFlags) { m_nDirtyFlags |= nFlags; }
	void ClearDirtyFlag() { m_nDirtyFlags = _RA df::not_dirty; }

private:
	/*const*/ AchievementSetType m_nSetType;

	AchievementID m_nAchievementID{ 0_z };
	Conditions m_vConditions;

	std::string m_sTitle;
	std::string m_sDescription;
	std::string m_sAuthor;
	std::string m_sBadgeImageURI;

	unsigned int m_nPointValue;
	BOOL m_bActive{ FALSE };
	BOOL m_bModified{ FALSE };
	BOOL m_bPauseOnTrigger{ FALSE };
	BOOL m_bPauseOnReset{ FALSE };

	//	Progress:
	BOOL m_bProgressEnabled{ FALSE };	//	on/off

	std::string m_sProgress;	//	How to calculate the progress so far (syntactical)
	std::string m_sProgressMax;	//	Upper limit of the progress (syntactical? value?)
	std::string m_sProgressFmt;	//	Format of the progress to be shown (currency? step?)

	float m_fProgressLastShown;	//	The last shown progress

	_RA dirty_flag m_nDirtyFlags{ _RA df::not_dirty };	//	Use for rendering when editing.

	time_t m_nTimestampCreated;
	time_t m_nTimestampModified;

	unsigned short m_nUpvotes;
	unsigned short m_nDownvotes;

	HBITMAP m_hBadgeImage;
	HBITMAP m_hBadgeImageLocked;
};


using CAchievement = const Achievement;


#endif // !ACHIEVEMENT_H
