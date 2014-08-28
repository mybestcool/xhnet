
#include "log/actionstat.h"
#include "xhutility.h"

namespace xhnet
{
	COneAction::COneAction(const char* ac)
		: m_action(ac), m_entertime( GetNowTime() )
	{

	}

	COneAction::~COneAction()
	{
		CActionStat::GetInstance().AddAction(*this);
	}



	CActionStat::CActionStat()
	{
	}

	CActionStat::~CActionStat()
	{

	}

	void CActionStat::AddAction(const COneAction& action)
	{
		auto it = m_actions.find(action.m_action);
		if (it == m_actions.end())
		{
			m_actions.insert(std::make_pair(action.m_action, CActionRecord(GetNowTime() - action.m_entertime)));
		}
		else
		{
			CActionRecord& record = it->second;
			record.m_totaltime += (GetNowTime() - action.m_entertime);
			record.m_totalcnt ++;
		}
	}

	void CActionStat::TraverseActions(std::function<void(const std::string& action, const CActionRecord& record)> callf)
	{
		for (auto it = m_actions.begin(); it != m_actions.end(); ++it)
		{
			callf(it->first, it->second);
		}
	}
	
};
