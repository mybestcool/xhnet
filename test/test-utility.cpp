
#include "stdhead.h"
#include "xh.h"

#ifdef _PLATFORM_WINDOWS_
#ifdef _DEBUG
#pragma comment( lib, "../bins/Debug/xh-net.lib" )
#else
#pragma comment( lib, "../bins/Release/xh-net.lib" )
#endif
#endif

#include <string.h>
#include <stdlib.h>
#include <random>


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

int main( int argc, char** argv )
{
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
	}

	const string testdir = "testdir";
	xhnet::MakeDir(testdir);
	string curpath = xhnet::GetModuleDir();
	if ( xhnet::IsSameDir(".", curpath))
	{
		std::cout<<"same path"<<std::endl;
	}
	else
	{
		std::cout<<"not same path"<<std::endl;
	}


	std::random_device rd;
	std::mt19937 gen(rd());

	std::normal_distribution<> d(5, 2);

	for (int n = 0; n < 10; ++n)
	{
		std::cout << std::round(d(gen)) << "\n";
	}

	char testm[] = "0123456789";
	string md5 = xhnet::GenMd5((unsigned char*)testm, strlen(testm));
	xhnet::LowLetters(md5);
	xhnet::UpLetters(md5);

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
