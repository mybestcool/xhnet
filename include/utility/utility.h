
#pragma once

#include "../xhhead.h"

#include <string>
#include <sstream>
#include <vector>

namespace xhnet
{
	// 编码转换
	// 一般server只用多字节编码或utf8，所以只对字符串操作
	//
	std::string	Convert_String(const std::string &text, char* out, unsigned int outlen, const std::string &to_encoding, const std::string &from_encoding);
	// end 编码转换


	// 
	// 类型转换
	//
	template<class T>
	std::string T2S(const T& t)
	{
		std::ostringstream os;
		os << t;
		return os.str();
	}
	template<class T>
	T S2T(const std::string& s, T& t)
	{
		std::istringstream is(s);
		is >> t;
		return t;
	}
	int			Str2Int(const std::string& data, int default = 0);
	std::string	Int2Str(int data, const std::string& default = "");
	unsigned int Str2UInt(const std::string& data, unsigned int default = 0);
	std::string	UInt2Str(unsigned int data, const std::string& default = "");
	// end 类型转换

	//
	// 字符串操作
	//
	// 字符串分割
	std::vector<std::string> SplitString(const std::string& str, const std::string& spt);

	// 大小写转换
	std::string UpLetters(std::string& inoutstr);
	std::string LowLetters(std::string& inoutstr);
	// end 字符串操作


	//
	// 字符串hash， 使用BKDR Hash 
	unsigned int StrHash(const std::string& data);
	// end hash


	//
	// 目录， 传入目录编码以系统为系统编码
	std::string	GetModulePath(void);
	bool		CreateDirectory(const std::string& path);
	bool		IsSamePath(const std::string& path1, const std::string& path2);
	// end 目录


	//
	// 时间，
	// 以下的time_t使用UTC时间， time_t序列化的时候最好明确指定内置类型，以免各个平台或者条件下长度不一
	// 以下的tm为当地时间
	//
	// 当地时间与utc时间的 秒数差， 可正 可负
	int			LocalTime_UTCTime(void);
	// 根据本地秒数获取utc秒数
	time_t		LocalTime2UTCTime(time_t localtime);
	// 根据utc秒数获取当地秒数
	time_t		UTCTime2LocalTime(time_t utctime);
	unsigned long long GetNowTimeMs(void);
	time_t		GetNowTime(void);
	tm			GetNowTM(void);

	tm			Time2TM(time_t intime);
	time_t		TM2Time(tm& intime);

	// since 1900
	int			GetNowYear(void);
	// 0-11
	int			GetNowMonth(void);
	// 0-365
	int			GetNowYearDay(void);
	// 1-31
	int			GetNowMonthDay(void);
	// 0-6 since Sunday
	int			GetNowWeekDay(void);

	//
	void		SleepMilliseconds(unsigned int ms);
	// 获得cpu开机以来的时钟脉冲次数, 秒级别 但会重置
	unsigned long GetCPUTickCount(void);

	// 是不是都是数字
	bool		IsDigit(const std::string& str);
	// 2012-10-22 10:22:52.1111
	bool		IsTimeStr(const std::string& timestr);

	// time_t 2 std::string "2012-10-22 10:22:52.1111"
	std::string	Time2Str(time_t timesecond);
	// 
	time_t		Str2Time(const std::string& timestr);

	// 当天的0点0分0秒
	time_t		Get_0H0M0S(tm intm);

	// 都以本地时间为准
	// 本周开始时间
	time_t		GetSunday_0H0M0S(tm intm);
	// 本月开始时间
	time_t		GetFirstMDay_0H0M0S(tm intm);
	// 本月最后一天
	time_t		GetLastMDay_23H59M59S(tm intm);
	// 本年开始时间
	time_t		GetFirstYDay_0H0M0S(tm intm);
	// 本年最后时间
	time_t		GetLastYDay_23H59M59S(tm intm);

	// 是不是同一天
	bool		IsSameDay(time_t dayl, time_t dayr);
	// dayl>dayr
	bool		IsBigDay(time_t dayl, time_t dayr);
	// dayl<dayr
	bool		IsSmallDay(time_t dayl, time_t dayr);
	// end 时间相关


	//
	// 随机值
	//
	unsigned int GenRandUINT(unsigned int min = 0x0, unsigned int max = 0xffffffff);
	// end 随机值


	//
	// UUID
	// 
	std::string	GenRandUUID(void);
	// end UUID


	//
	// MD5
	//
	// 返回32位md5
	std::string GenMd5(unsigned char* data, unsigned int datalen);
	//
	void GenMd5(unsigned char md5[16], unsigned char* data, unsigned int datalen);
	// end md5


	// uint加减
	// unsigned int ++
	unsigned int UINTAdd(unsigned int src, unsigned int &add, unsigned int max = 0xffffffff);
	unsigned int UINTAddConst(unsigned int src, const unsigned int add, unsigned int max = 0xffffffff);
	// unsigned int --
	unsigned int UINTSub(unsigned int src, unsigned int &sub, unsigned int max = 0xffffffff);
	unsigned int UINTSubConst(unsigned int src, const unsigned int sub, unsigned int max = 0xffffffff);
	// end uint op


	//
	// DLL\SO 处理
	//
	// 失败返回0
	void* OpenDyLib(const std::string& path);
	void CloseDyLib(void* dy);
	void* GetDyLibFun(void* dy, const std::string& funname);
	// end DLL\SO 处理
};


