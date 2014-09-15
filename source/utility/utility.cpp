

#include <time.h>
#include <sys/timeb.h>
#include <limits>
#include <stdlib.h>
#include <random>
#include <algorithm>

#include "utility/utility.h"
#include "md5/md5.h"

#include "xhguard.h"



#if defined _PLATFORM_WINDOWS_
#include <windows.h>
#include <objbase.h>
#include <direct.h>
#include <io.h>

#include <libiconv/include/iconv.h>
#ifdef _DEBUG
#pragma comment( lib, "libiconv/lib/iconvD.lib" )
#else
#pragma comment( lib, "libiconv/lib/iconv.lib" )
#endif // _DEBUG

#elif defined _PLATFORM_LINUX_
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <dlfcn.h>

#include <iconv.h>
#pragma comment( lib, "libiconv" )

#include <uuid/uuid.h>
#pragma comment( lib, "libuuid")
#endif

namespace xhnet
{
	// 编码转换
	//
	bool Convert_String(const char* in, unsigned int inlen, char* out, unsigned int outlen, const std::string &to_encoding, const std::string &from_encoding)
	{
		iconv_t h = iconv_open(to_encoding.c_str(), from_encoding.c_str());
		if (iconv_t(-1) == h) return false;

		XH_GUARD([&]{ iconv_close(h); });

		if (size_t(-1) == iconv(h, (const char **)&in, &inlen, &out, &outlen))
		{
			return false;
		}

		return true;
	}



	// 类型转换
	//
	int	Str2Int(const std::string& data, int default)
	{
		int ret = default;
		try
		{
			S2T(data, ret);
		}
		catch (...)
		{
			;
		}
		return ret;
	}

	std::string	Int2Str(int data, const std::string& default)
	{
		std::string ret = default;
		try
		{
			ret = T2S(data);
		}
		catch (...)
		{
			;
		}
		return ret;
	}

	unsigned int Str2UInt(const std::string& data, unsigned int default)
	{
		unsigned int ret = default;
		try
		{
			S2T(data, ret);
		}
		catch (...)
		{
			;
		}
		return ret;
	}

	std::string	UInt2Str(unsigned int data, const std::string& default)
	{
		std::string ret = default;
		try
		{
			ret = T2S(data);
		}
		catch (...)
		{
			;
		}
		return ret;
	}

	std::vector<std::string> SplitString(const std::string& str, const std::string& spt)
	{
		std::vector<std::string> _list;
		std::string st = str;
		
		std::string::size_type spl = spt.length();
		std::string::size_type pos = st.find(spt);

		while (pos != std::string::npos)
		{
			_list.push_back(st.substr(0, pos));
			st.erase(0, pos + spl);
			pos = st.find(spt);
		}

		if (st.length() >= 0) _list.push_back(st);

		return _list;
	}

	std::string UpLetters(std::string& inoutstr)
	{
		std::for_each(inoutstr.begin(), inoutstr.end(), [](char& c){c = toupper((unsigned char)c); });
		return inoutstr;
	}

	std::string LowLetters(std::string& inoutstr)
	{
		std::for_each(inoutstr.begin(), inoutstr.end(), [](char& c){c = tolower((unsigned char)c); });
		return inoutstr;
	}


	//
	// 字符串hash， 使用BKDR Hash 
	unsigned int StrHash(const std::string& data)
	{
		// BKDR Hash 
		char* str = (char*)(data.c_str());

		unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
		unsigned int hash = 0;

		while (*str)
		{
			hash = hash * seed + (*str++);
		}

		return (hash & 0x7FFFFFFF);
	}



	//
	// 目录， 传入目录编码以系统为系统编码
	std::string	GetModulePath(void)
	{
#if defined _PLATFORM_WINDOWS_

		char szFile[_MAX_PATH];
		GetModuleFileNameA(GetModuleHandle(NULL), szFile, _MAX_PATH);

		char szDrive[_MAX_PATH], szPath[_MAX_PATH];

		_splitpath_s(szFile, szDrive, sizeof(szDrive), szPath, sizeof(szPath), NULL, 0, NULL, 0);
		strncat_s(szDrive, sizeof(szDrive), szPath, sizeof(szDrive) - strlen(szDrive) - 1);

		std::string ret = szDrive;
		if (ret.find_last_of('\\') != ret.length() - 1 && ret.find_first_of('/') != ret.length() - 1)
		{
			ret.append("\\");
		}

		return ret;

#elif defined _PLATFORM_LINUX_

		char dir[1024] = { 0 };
		int count = readlink("/proc/self/exe", dir, 1024);
		if (count < 0 || count >= 1024)
			return "./";
		else
			dir[count] = 0;
		std::string ret = dir;
		std::string::size_type pos = ret.find_last_of('/');

		if (pos == std::string::npos) return ret;

		return ret.substr(0, pos + 1);

#endif
	}

	bool CreateDirectory(const std::string& path)
	{
		bool ret = false;
#if defined _PLATFORM_WINDOWS_
		if (_access(path.c_str(), 0)==0)
		{
			ret = true;
		}
		else if (0 == _mkdir(path.c_str()))
		{
			ret = true;
		}
#elif defined _PLATFORM_LINUX_
		if (access(path.c_str(), F_OK) == 0)
		{
			ret = true;
		}
		else if (0 == mkdir(path.c_str(), 0755))
		{
			ret = true;
		}
#endif
		return ret;
	}

	bool IsSamePath(const std::string& path1, const std::string& path2)
	{
#if defined _PLATFORM_WINDOWS_
		char fullpath1[_MAX_PATH] = { 0 };
		char fullpath2[_MAX_PATH] = { 0 };

		_fullpath(fullpath1, path1.c_str(), sizeof(fullpath1));
		_fullpath(fullpath2, path2.c_str(), sizeof(fullpath2));

		return strcmp(fullpath1, fullpath2) == 0;

#elif defined _PLATFORM_LINUX_
		char fullpath1[1024] = { 0 };
		char fullpath2[1024] = { 0 };

		realpath(path1.c_str(), fullpath1);
		realpath(path2.c_str(), fullpath2);

		return strcmp(fullpath1, fullpath2) == 0;
#endif
	}



	//
	// 时间，time_t使用UTC时间， tm为当地时间
	int LocalTime_UTCTime(void)
	{
		static bool binit = false;
		static int time_dis = 0;

		if (!binit)
		{
			timeb nowtime;
			ftime(&nowtime);
			//_ftime( &nowtime );

			time_dis = nowtime.timezone * 60;
			binit = true;
		}

		return time_dis;
	}

	time_t LocalTime2UTCTime(time_t localtime)
	{
		return localtime + LocalTime_UTCTime();
	}

	time_t UTCTime2LocalTime(time_t utctime)
	{
		return utctime - LocalTime_UTCTime();
	}

	unsigned long long GetNowTimeMs(void)
	{
		unsigned long long ret_time = 0;
#if defined _PLATFORM_WINDOWS_

		struct tm	tm;
		SYSTEMTIME	wtm;

		GetLocalTime(&wtm);
		tm.tm_year = wtm.wYear - 1900;
		tm.tm_mon = wtm.wMonth - 1;
		tm.tm_mday = wtm.wDay;
		tm.tm_hour = wtm.wHour;
		tm.tm_min = wtm.wMinute;
		tm.tm_sec = wtm.wSecond;
		tm.tm_isdst = -1;

		time_t	now = mktime(&tm);

		ret_time = (unsigned long long)now * 1000UL + wtm.wMilliseconds;
#elif defined _PLATFORM_LINUX_
		struct timeval now;
		gettimeofday(&now, 0);
		ret_time = now.tv_sec;
		ret_time = ret_time * 1000000;
		ret_time += now.tv_usec;

		ret_time = ret_time / 1000ULL;
#endif

		return ret_time;
	}

	time_t GetNowTime(void)
	{
		return time(NULL);
	}

	tm GetNowTM(void)
	{
		return Time2TM(GetNowTime());
	}

	tm Time2TM(time_t intime)
	{
#if defined _PLATFORM_WINDOWS_
		tm outtime;
		localtime_s(&outtime, &intime);
#elif defined _PLATFORM_LINUX_
		tm outtime = *localtime_r(&intime);
#endif	
		return outtime;
	}

	time_t TM2Time(tm& intime)
	{
		return mktime(&intime);
	}

	// 北京时间可以使用这个方式
	//int time_to_tm(time_t *time_input, struct tm* tm_result)
	//{
	//	static const char month_days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	//	static const bool leap_year[4] = { false, false, true, false };

	//	unsigned int leave_for_fouryear = 0;
	//	unsigned short four_year_count = 0;
	//	unsigned int temp_value = 0;

	//	tm_result->tm_sec = *time_input % 60;
	//	temp_value = *time_input / 60;// 分钟
	//	tm_result->tm_min = temp_value % 60;
	//	temp_value /= 60; // 小时

	//	temp_value += 8;// 加上时区

	//	tm_result->tm_hour = temp_value % 24;
	//	temp_value /= 24; // 天

	//	tm_result->tm_wday = (temp_value + 4) % 7;// 1970-1-1是4

	//	four_year_count = temp_value / (365 * 4 + 1);
	//	leave_for_fouryear = temp_value % (365 * 4 + 1);
	//	int leave_for_year_days = leave_for_fouryear;

	//	int day_count = 0;
	//	int i = 0;

	//	for (i = 0; i < 4; i++)
	//	{
	//		day_count = leap_year[i] ? 366 : 365;

	//		if (leave_for_year_days <= day_count)
	//		{
	//			break;
	//		}
	//		else
	//		{
	//			leave_for_year_days -= day_count;
	//		}
	//	}

	//	tm_result->tm_year = four_year_count * 4 + i + 70;
	//	tm_result->tm_yday = leave_for_year_days;// 这里不是天数，而是标记，从0开始

	//	int leave_for_month_days = leave_for_year_days;

	//	int j = 0;
	//	for (j = 0; j < 12; j++)
	//	{
	//		if (leap_year[i] && j == 1)
	//		{
	//			if (leave_for_month_days < 29)
	//			{
	//				break;
	//			}
	//			else if (leave_for_month_days == 29)
	//			{
	//				j++;
	//				leave_for_month_days = 0;
	//				break;
	//			}
	//			else
	//			{
	//				leave_for_month_days -= 29;
	//			}

	//			continue;
	//		}

	//		if (leave_for_month_days < month_days[j])
	//		{
	//			break;
	//		}
	//		else if (leave_for_month_days == month_days[j]){
	//			j++;
	//			leave_for_month_days = 0;
	//			break;
	//		}
	//		else
	//		{
	//			leave_for_month_days -= month_days[j];
	//		}
	//	}

	//	tm_result->tm_mday = leave_for_month_days + 1;
	//	tm_result->tm_mon = j;
	//	if (tm_result->tm_mon >= 12)
	//	{
	//		tm_result->tm_year++;
	//		tm_result->tm_mon -= 12;
	//	}
	//	tm_result->tm_isdst = -1;

	//	return 0;
	//}

	//int tm_to_time(struct tm* tm_input, time_t *time_result)
	//{
	//	static short monthlen[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	//	static short monthbegin[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

	//	time_t t;

	//	t = monthbegin[tm_input->tm_mon]
	//		+ tm_input->tm_mday - 1
	//		+ (!(tm_input->tm_year & 3) && tm_input->tm_mon > 1);

	//	tm_input->tm_yday = t;
	//	t += 365 * (tm_input->tm_year - 70)
	//		+ (tm_input->tm_year - 69) / 4;

	//	tm_input->tm_wday = (t + 4) % 7;

	//	t = t * 86400 + (tm_input->tm_hour - 8) * 3600 + tm_input->tm_min * 60 + tm_input->tm_sec;

	//	if (tm_input->tm_mday > monthlen[tm_input->tm_mon] + (!(tm_input->tm_year & 3) && tm_input->tm_mon == 1))
	//	{
	//		*time_result = mktime(tm_input);
	//	}
	//	else
	//	{
	//		*time_result = t;
	//	}

	//	return 0;
	//}

	// since 1900
	int	GetNowYear(void)
	{
		tm nowtime = GetNowTM();
		return nowtime.tm_year;
	}

	// 0-11
	int	GetNowMonth(void)
	{
		tm nowtime = GetNowTM();
		return nowtime.tm_mon;
	}

	// 0-365
	int	GetNowYearDay(void)
	{
		tm nowtime = GetNowTM();
		return nowtime.tm_yday;
	}

	// 1-31
	int	GetNowMonthDay(void)
	{
		tm nowtime = GetNowTM();
		return nowtime.tm_mday;
	}

	// 0-6 since Sunday
	int	GetNowWeekDay(void)
	{
		tm nowtime = GetNowTM();
		return nowtime.tm_wday;
	}

	void SleepMilliseconds(unsigned int ms)
	{
#if defined _PLATFORM_WINDOWS_
		Sleep(ms);
#elif defined _PLATFORM_LINUX_
		usleep( ms*1000 );
		//struct timeval tv;
		//tv.tv_sec = 0;
		//tv.tv_usec = ms * 1000;
		//select(0, NULL, NULL, NULL, &tv);
#endif
	}

	unsigned long GetCPUTickCount(void)
	{
#if defined _PLATFORM_WINDOWS_
		return GetTickCount() / 1000UL;
#elif defined _PLATFORM_LINUX_
		struct sysinfo sinfo;

		sinfo.uptime = 0;
		::sysinfo(&sinfo);

		return sinfo.uptime;
#endif
	}



	// 是不是都是数字
	bool IsDigit(const std::string& str)
	{
		for (unsigned int i = 0; i < str.length(); ++i)
		{
			char data = str[i];
			if (data<'0' || str[i]>'9')
			{
				return false;
			}
		}

		return true;
	}

	// 2012-10-22 10:22:52.1111
	bool IsTimeStr(const std::string& timestr)
	{
		static time_t invalidtime = time_t(-1);
		static time_t begintime = Str2Time("1970-01-01 00:00:00");

		time_t curtime = Str2Time(timestr);

		return (curtime>=begintime && curtime<invalidtime);
	}

	// time_t 2 std::string "2012-10-22 10:22:52.1111"
	std::string	Time2Str(time_t timesecond)
	{
		tm timetm = Time2TM(timesecond);
		char strtime[256] = { 0 };

		strftime(strtime, sizeof(strtime), "%Y-%m-%d %H:%M:%S", &timetm);

		return strtime;
	}
	// 
	time_t Str2Time(const std::string& timestr)
	{
#if defined _PLATFORM_WINDOWS_
		// windows没有strptime，网上可以找到实现
		// 这里使用最简单的方式来实现
		const char* pDate = timestr.c_str();
		const char* pStart = pDate;

		char szYear[5], szMonth[3], szDay[3], szHour[3], szMin[3], szSec[3];

		szYear[0] = *pDate++;
		szYear[1] = *pDate++;
		szYear[2] = *pDate++;
		szYear[3] = *pDate++;
		szYear[4] = 0x0;

		++pDate;

		szMonth[0] = *pDate++;
		szMonth[1] = *pDate++;
		szMonth[2] = 0x0;

		++pDate;

		szDay[0] = *pDate++;
		szDay[1] = *pDate++;
		szDay[2] = 0x0;

		++pDate;

		szHour[0] = *pDate++;
		szHour[1] = *pDate++;
		szHour[2] = 0x0;

		++pDate;

		szMin[0] = *pDate++;
		szMin[1] = *pDate++;
		szMin[2] = 0x0;

		++pDate;

		szSec[0] = *pDate++;
		szSec[1] = *pDate++;
		szSec[2] = 0x0;

		tm timetm;

		timetm.tm_year	= atoi(szYear) - 1900;
		timetm.tm_mon	= atoi(szMonth) - 1;
		timetm.tm_mday	= atoi(szDay);
		timetm.tm_hour	= atoi(szHour);
		timetm.tm_min	= atoi(szMin);
		timetm.tm_sec	= atoi(szSec);
		timetm.tm_isdst = -1;

		return TM2Time(timetm);
#elif defined _PLATFORM_LINUX_
		tm timetm;
		strptime(timestr.c_str(), "%Y-%m-%d %H:%M:%S", &timetm);
		return TM2Time(timetm);
#endif
	}

	// 当天的0点0分0秒
	time_t Get_0H0M0S(tm intm)
	{
		intm.tm_hour = 0;
		intm.tm_min = 0;
		intm.tm_sec = 0;

		return TM2Time(intm);
	}

	// 本周开始时间
	time_t GetSunday_0H0M0S(tm intm)
	{
		time_t passtime = intm.tm_wday * 86400;
		passtime += intm.tm_hour * 3600;
		passtime += intm.tm_min * 60;
		passtime += intm.tm_sec;

		return TM2Time(intm)-passtime;
	}
	// 本月开始时间
	time_t GetFirstMDay_0H0M0S(tm intm)
	{
		time_t passtime = (intm.tm_mday-1) * 86400;
		passtime += intm.tm_hour * 3600;
		passtime += intm.tm_min * 60;
		passtime += intm.tm_sec;

		return TM2Time(intm) - passtime;
	}
	// 本月最后一天
	time_t GetLastMDay_23H59M59S(tm intm)
	{
		intm.tm_mday = 1;
		intm.tm_hour = 0;
		intm.tm_min = 0;
		intm.tm_sec = 0;

		if ( intm.tm_mon>=11 )
		{
			intm.tm_mon = 0;
			intm.tm_yday++;
		}
		else
		{
			intm.tm_mon++;
		}
		return TM2Time(intm)-1;
	}
	// 本年开始时间
	time_t GetFirstYDay_0H0M0S(tm intm)
	{
		time_t passtime = intm.tm_yday * 86400;
		passtime += intm.tm_hour * 3600;
		passtime += intm.tm_min * 60;
		passtime += intm.tm_sec;

		return TM2Time(intm) - passtime;
	}
	// 本年最后时间
	time_t GetLastYDay_23H59M59S(tm intm)
	{
		intm.tm_yday++;
		intm.tm_mon = 0;
		intm.tm_mday = 1;

		intm.tm_hour = 0;
		intm.tm_min = 0;
		intm.tm_sec = 0;

		return TM2Time(intm) - 1;
	}

	// 是不是同一天
	bool IsSameDay(time_t dayl, time_t dayr)
	{
		tm tml = Time2TM(dayl);
		tm tmr = Time2TM(dayr);

		return tml.tm_yday == tml.tm_yday;
	}

	// dayl>dayr
	bool IsBigDay(time_t dayl, time_t dayr)
	{
		tm tml = Time2TM(dayl);
		tm tmr = Time2TM(dayr);

		return (tml.tm_year == tmr.tm_year && tml.tm_yday > tmr.tm_yday) || (tml.tm_year > tmr.tm_year);
	}

	// dayl<dayr
	bool IsSmallDay(time_t dayl, time_t dayr)
	{
		tm tml = Time2TM(dayl);
		tm tmr = Time2TM(dayr);

		return (tml.tm_year == tmr.tm_year && tml.tm_yday < tmr.tm_yday) || (tml.tm_year < tmr.tm_year);
	}



	//
	//随机值
	unsigned int GenRandUINT(unsigned int min, unsigned int max)
	{
		static std::random_device rd;
		static std::mt19937 mt(rd());

		unsigned int value = mt();
		unsigned int outvalue = value % (max - min + 1);

		return outvalue + min;
	}



	//
	// UUID
	std::string	GenRandUUID(void)
	{
		unsigned char uuid[16];
		char retstr[64] = { 0 };

#if defined _PLATFORM_WINDOWS_
		GUID guid;
		::CoCreateGuid(&guid);
		memcpy(uuid, &guid, sizeof(uuid));

		_snprintf_s(retstr, sizeof(retstr) - 1
			, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X"
			, uuid[0], uuid[1], uuid[2], uuid[3]
			, uuid[4], uuid[5], uuid[6], uuid[7]
			, uuid[8], uuid[9], uuid[10], uuid[11]
			, uuid[12], uuid[13], uuid[14], uuid[15]
			);
#elif defined _PLATFORM_LINUX_
		uuid_t guid;
		uuid_generate(guid);
		memcpy(uuid, &guid, sizeof(uuid));

		snprintf(retstr, sizeof(retstr)-1
			, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X"
			, uuid[0], uuid[1], uuid[2], uuid[3]
			, uuid[4], uuid[5], uuid[6], uuid[7]
			, uuid[8], uuid[9], uuid[10], uuid[11]
			, uuid[12], uuid[13], uuid[14], uuid[15]
			);
#endif
		return retstr;
	}

	std::string GenMd5(unsigned char* data, unsigned int datalen)
	{
		unsigned char md5[16] = { 0 };
		GenMd5(md5, data, datalen);

		char retstr[64] = { 0 };
#if defined _PLATFORM_WINDOWS_
		_snprintf_s(retstr, sizeof(retstr) - 1
			, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X"
			, md5[0], md5[1], md5[2], md5[3]
			, md5[4], md5[5], md5[6], md5[7]
			, md5[8], md5[9], md5[10], md5[11]
			, md5[12], md5[13], md5[14], md5[15]
			);
#elif defined _PLATFORM_LINUX_
		snprintf(retstr, sizeof(retstr) - 1
			, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X"
			, md5[0], md5[1], md5[2], md5[3]
			, md5[4], md5[5], md5[6], md5[7]
			, md5[8], md5[9], md5[10], md5[11]
			, md5[12], md5[13], md5[14], md5[15]
			);
#endif

		return retstr;
	}
	//
	void GenMd5(unsigned char md5[16], unsigned char* data, unsigned int datalen)
	{
		MD5_CTX ctx;
		MD5Init(&ctx);
		MD5Update(&ctx, data, datalen);
		MD5Final(&ctx, md5);
	}


	// uint加减
	// unsigned int ++
	unsigned int UINTAdd(unsigned int src, unsigned int &add, unsigned int max)
	{
		if (src >= max)			{ add = 0;			return max; }
		else if (max < add)		{ add = max - src;	return max; }
		else if (src > max - add)	{ add = max - src;	return max; }

		return src + add;
	}

	unsigned int UINTAddConst(unsigned int src, const unsigned int add, unsigned int max)
	{
		unsigned int tempadd = add;

		return UINTAdd(src, tempadd, max);
	}

	// unsigned int --
	unsigned int UINTSub(unsigned int src, unsigned int &sub, unsigned int max)
	{
		if (src < sub)			{ sub = src;	return 0; }
		else if (max < sub)		{ sub = src;	return 0; }

		return src - sub;
	}

	unsigned int UINTSubConst(unsigned int src, const unsigned int sub, unsigned int max)
	{
		unsigned int tempsub = sub;

		return UINTSub(src, tempsub, max);
	}

	void* OpenDyLib(const std::string& path)
	{
#if defined _PLATFORM_WINDOWS_
		void* dy = ::LoadLibraryA(path.c_str());
#elif defined _PLATFORM_LINUX_
		void* dy = ::dlopen(dllfile, RTLD_NOW);
#endif
		return dy;
	}

	void CloseDyLib(void* dy)
	{
		if (dy == 0) return;
#if defined _PLATFORM_WINDOWS_
		::FreeLibrary((HINSTANCE)(dy));
#elif defined _PLATFORM_LINUX_
		::dlclose( dy );
#endif
	}

	void* GetDyLibFun(void* dy, const std::string& funname)
	{
		if (0 == dy) return false;
#if defined _PLATFORM_WINDOWS_
		void* pfun = ::GetProcAddress((HINSTANCE)(dy), funname.c_str());
#elif defined _PLATFORM_LINUX_
		void* pfun = ::dlsym(dy, funname);
#endif
		return pfun;
	}
};




