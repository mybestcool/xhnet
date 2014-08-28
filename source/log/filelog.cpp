
#include "xhhead.h"
#include "log/filelog.h"
#include "utility/utility.h"
#include "xhlog.h"


#include <log4cplus/consoleappender.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h>

#if defined _PLATFORM_WINDOWS_
#ifdef _DEBUG
#pragma comment( lib, "log4cplusD.lib" )
#else
#pragma comment( lib, "log4cplus.lib" )
#endif
#elif defined _PLATFORM_LINUX_
#pragma comment( lib, "log4cplus" )
#endif

using namespace log4cplus;



namespace xhnet
{
	char const logname_base[] = "base ";
	char const logname_trace[]= "trace";
	//char const logname_logic[] = "logic";
	//char const logname_user[] = "user ";


	static const char logpattern[] = "%-20D [%-5p] [%-10t] %c - %m [%l]%n";

	CFileLogger::CFileLogger()
		: m_bhasconsole( false )
	{
		static bool b_init_log4cplus = false;
		if (!b_init_log4cplus)
		{
			b_init_log4cplus = true;
			log4cplus::initialize();
		}
	}

	CFileLogger::~CFileLogger()
	{

	}

	void CFileLogger::OpenConsole(void)
	{
		SharedAppenderPtr append(new ConsoleAppender(false, true));
		append->setName(LOG4CPLUS_TEXT("globalconsole"));

		std::auto_ptr<Layout> layout(new PatternLayout(logpattern));
		append->setLayout(layout);

		Logger::getRoot().addAppender(append);
	}

	bool CFileLogger::AddLogger(log4cplus::Logger& logger, const std::string& logpath, int loglevel)
	{
		if (!logpath.empty())
		{
			SharedAppenderPtr append(new DailyRollingFileAppender(LOG4CPLUS_TEXT(logpath), DAILY, true, 100));
			append->setName(LOG4CPLUS_TEXT(GetFileAppenderName(logger)));

			std::auto_ptr<Layout> layout(new PatternLayout(logpattern));
			append->setLayout(layout);

			logger.addAppender(append);
		}

		logger.setLogLevel(loglevel);

		return true;
	}

	bool CFileLogger::AddSameFileLogger(log4cplus::Logger& sublogger, log4cplus::Logger& mainlogger, int loglevel)
	{
		if ( &sublogger == &mainlogger )
		{
			return true;
		}

		SharedAppenderPtr append = mainlogger.getAppender(LOG4CPLUS_TEXT(GetFileAppenderName(mainlogger)));
		if (append)
		{
			sublogger.addAppender(append);
		}

		sublogger.setLogLevel(loglevel);

		return true;
	}

	void CFileLogger::SetLogger(log4cplus::Logger& logger, int loglevel)
	{
		logger.setLogLevel(loglevel);
	}

	std::string CFileLogger::GetFileAppenderName(log4cplus::Logger& logger)
	{
		std::string appedname = logger.getName() + "-f-";
		appedname += T2S(__LINE__);

		return appedname;
	}
};