
#pragma once

#define LOG4CPLUS_BUILD_DLL
#include <log4cplus/include/log4cplus/logger.h>
#include <log4cplus/include/log4cplus/loggingmacros.h>
#include <log4cplus/include/log4cplus/appender.h>
#include <string>

namespace xhnet
{
	class CFileLogger
	{
	public:
		~CFileLogger();

		static CFileLogger& GetInstance()
		{
			static CFileLogger logger;

			return logger;
		}

		//
		void OpenConsole(void);
		// 100天以后的日志会自动删除
		bool AddLogger(log4cplus::Logger& logger, const std::string& logpath, int loglevel);
		// sublogger 与 mainlogger 的文件相同
		bool AddSameFileLogger(log4cplus::Logger& sublogger, log4cplus::Logger& mainlogger, int loglevel);
		//
		void SetLogger(log4cplus::Logger& logger, int loglevel);
	private:
		CFileLogger();

		std::string GetFileAppenderName(log4cplus::Logger& logger);

	private:
		bool m_bhasconsole;

		CFileLogger(const CFileLogger&) = delete;
		CFileLogger& operator=(const CFileLogger&) = delete;
	};

	template<char const* LogName>
	class CLogger
	{
	public:
		~CLogger(){}

		static CLogger& GetInstance(void)
		{
			static CLogger logger;

			return logger;
		}

		log4cplus::Logger	m_logger;
	private:
		CLogger()
		{
			// 确保CFileLogger比CLogger先初始化
			CFileLogger::GetInstance();
			m_logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT(LogName));
		}

		CLogger(const CLogger&) = delete;
		CLogger& operator=(const CLogger&) = delete;
	};
};

