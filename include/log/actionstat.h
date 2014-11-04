
#pragma once

#include <string>
#include <map>
#include <functional>

#include <xhhead.h>

namespace xhnet
{
	class COneAction
	{
	public:
		COneAction(const char* ac);
		~COneAction();

		const std::string	m_action;
		time_t				m_entertime;
	};

	class CActionRecord
	{
	public:
		time_t				m_totaltime;
		unsigned int		m_totalcnt;

		CActionRecord(time_t totaltime)
			: m_totaltime(totaltime)
			, m_totalcnt(1)
		{}

		~CActionRecord(){}
	};

	class CActionStat
	{
		std::map<std::string, CActionRecord> m_actions;
	public:
		~CActionStat();

		static CActionStat& GetInstance(void)
		{
			static CActionStat as;
			return as;
		}

		void AddAction(const COneAction& action);
		void TraverseActions(std::function<void(const std::string& action, const CActionRecord& record)> callf);

	private:
		CActionStat();

		CActionStat(const CActionStat&) = delete;
		CActionStat& operator=(const CActionStat&) = delete;
	};
};

