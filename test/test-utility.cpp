
#include "../include/stdhead.h"
#include "../include/xhguard.h"

#include "../3_libiconv-win32/include/iconv.h"
#pragma comment( lib, "../3_libiconv-win32/lib/iconv.lib" )

#include "memory/mpool.h"
#include "memory/opool.h"
#include "thread/nullmutex.h"

#include "utility/utility.h"

#include <direct.h>
#include <io.h>
#include <stdlib.h>
#include <random>

bool conv_string(const string& from, const string to, char* in, unsigned int inlen, char* out, unsigned int outlen)
{
	iconv_t h = iconv_open(to.c_str(), from.c_str());
	if (iconv_t(-1) == h) return false;

	XH_GUARD([&]{ iconv_close(h); });

	if (size_t(-1) == iconv(h, (const char **)&in, &inlen, &out, &outlen))
	{
		return false;
	}

	return true;
}

struct Test0
{
	int a;
	char data[64 * 1024];
};

struct Test1
{
	int a;
	unsigned short c;
	char b;
};

struct Test2 : public xhnet::CInheritOPool<Test2, std::mutex>
{
	int b;
	unsigned long long c;
};

std::string upletter(std::string& str)
{
	std::for_each(str.begin(), str.end(), [](char& c){c = toupper((unsigned char)c); });
	return str;
}

std::string lowletter(std::string& str)
{
	std::for_each(str.begin(), str.end(), [](char& c){c = tolower((unsigned char)c); });
	return str;
}

int main( int argc, char** argv )
{
	/*char gbk_1[] = "ÆÆÏþµÄ²©¿Í";
	char utf8_1[100] = { 0 };
	char gbk_2[100] = { 0 };

	size_t len0 = conv_string("GBK", "UTF-8", gbk_1, strlen(gbk_1), utf8_1, sizeof(utf8_1));
	size_t len1 = conv_string("utf-8", "gbk", utf8_1, strlen(utf8_1), gbk_2, sizeof(gbk_2));

	getchar();

	xhnet::COPool<Test0, std::mutex> mpool0;
	xhnet::COPool<Test1, std::mutex> mpool1;

	vector<Test0*> v0;
	vector<Test1*> v1;
	vector<Test2*> v2;
	for (int i = 0; i < 1000; ++i)
	{
		if ( i==17000 )
		{
			int c= 100;
		}

		Test0* t0 = mpool0.Allocate();
		Test1* t1 = mpool1.Allocate();
		Test2* t2 = new Test2();

		v0.push_back(t0);
		v1.push_back(t1);
		v2.push_back(t2);
	}

	for ( auto i:v0 )
	{
		mpool0.Free(i);
	}

	for (auto i : v1)
	{
		mpool1.Free(i);
	}

	for (auto i : v2)
	{
		delete i;
	}

	v0.clear();
	v1.clear();
	v2.clear();

	for (int i = 0; i < 1000; ++i)
	{
		Test0* t0 = mpool0.Allocate();
		Test1* t1 = mpool1.Allocate();
		Test2* t2 = new Test2();

		v0.push_back(t0);
		v1.push_back(t1);
		v2.push_back(t2);
	}

	for (auto i : v0)
	{
		mpool0.Free(i);
	}

	for (auto i : v1)
	{
		mpool1.Free(i);
	}

	for (auto i : v2)
	{
		delete (i);
	}*/

	int ret = _access("test", 0);

	int ret2 = _mkdir("test");

	char fullpath[_MAX_PATH];
	_fullpath(fullpath, "test", _MAX_PATH);

	std::random_device rd;
	std::mt19937 gen(rd());

	std::normal_distribution<> d(5, 2);

	for (int n = 0; n < 10; ++n)
	{
		std::cout << std::round(d(gen)) << "\n";
	}

	char testm[] = "0123456789";
	string md5 = xhnet::GenMd5((unsigned char*)testm, strlen(testm));
	lowletter(md5);
	upletter(md5);

	{
		time_t nowtime = xhnet::Get_0H0M0S(xhnet::GetNowTM());
		string nowtimestr = xhnet::Time2Str(nowtime);
		time_t nowtime2 = xhnet::Str2Time(nowtimestr);
	}

	{
		time_t nowtime = xhnet::GetSunday_0H0M0S(xhnet::GetNowTM());
		string nowtimestr = xhnet::Time2Str(nowtime);
		time_t nowtime2 = xhnet::Str2Time(nowtimestr);
	}

	{
		time_t nowtime = xhnet::GetFirstMDay_0H0M0S(xhnet::GetNowTM());
		string nowtimestr = xhnet::Time2Str(nowtime);
		time_t nowtime2 = xhnet::Str2Time(nowtimestr);
	}

	{
		time_t nowtime = xhnet::GetLastMDay_23H59M59S(xhnet::GetNowTM());
		string nowtimestr = xhnet::Time2Str(nowtime);
		time_t nowtime2 = xhnet::Str2Time(nowtimestr);
	}

	{
		time_t nowtime = xhnet::GetFirstYDay_0H0M0S(xhnet::GetNowTM());
		string nowtimestr = xhnet::Time2Str(nowtime);
		time_t nowtime2 = xhnet::Str2Time(nowtimestr);
	}

	{
		time_t nowtime = xhnet::GetLastYDay_23H59M59S(xhnet::GetNowTM());
		string nowtimestr = xhnet::Time2Str(nowtime);
		time_t nowtime2 = xhnet::Str2Time(nowtimestr);
	}


	clock();
	return 0;
}
