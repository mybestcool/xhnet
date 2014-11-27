
#include "stdhead.h"
#include "xh.h"

#ifdef _PLATFORM_WINDOWS_
#ifdef _DEBUG
#pragma comment( lib, "../bins/Debug/xh-net.lib" )
#else
#pragma comment( lib, "../bins/Release/xh-net.lib" )
#endif
#endif


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

struct Test2
{
	int b;
	unsigned long long c;
};

int main( int argc, char** argv )
{
	xhnet::CMPool<xhnet::CNullMutex> mpool;
	//xhnet::CMPool<std::mutex> mpool;

	time_t begin_tm = 0;
	time_t end_tm = 0;

	begin_tm = xhnet::GetNowTime();
	for (int i = 0; i < 1000; ++i)
	{
		mpool.Free( mpool.Allocate<Test0>() );
		mpool.Free( mpool.Allocate<Test1>() );
		mpool.Free( mpool.Allocate<Test2>() );
	}

	vector<Test0*> v0;
	vector<Test1*> v1;
	vector<Test2*> v2;
	end_tm = xhnet::GetNowTime();

	cout << "cost time:" << end_tm - begin_tm << endl;

	begin_tm = xhnet::GetNowTime();
	for (int i = 0; i < 1000; ++i)
	{
		Test0* t0 = mpool.Allocate<Test0>();
		Test1* t1 = mpool.Allocate<Test1>();
		Test2* t2 = mpool.Allocate<Test2>();

		v0.push_back(t0);
		v1.push_back(t1);
		v2.push_back(t2);
	}

	for (auto i : v0)
	{
		mpool.Free(i);
	}

	for (auto i : v1)
	{
		mpool.Free(i);
	}

	for (auto i : v2)
	{
		mpool.Free(i);
	}
	end_tm = xhnet::GetNowTime();

	cout << "cost time:" << end_tm - begin_tm << endl;

	begin_tm = xhnet::GetNowTime();
	for (int i = 0; i < 1000; ++i)
	{
		Test0* t0 = mpool.Allocate<Test0>();
		Test1* t1 = mpool.Allocate<Test1>();
		Test2* t2 = mpool.Allocate<Test2>();

		v0.push_back(t0);
		v1.push_back(t1);
		v2.push_back(t2);
	}

	for (auto i : v0)
	{
		mpool.Free(i);
	}

	for (auto i : v1)
	{
		mpool.Free(i);
	}

	for (auto i : v2)
	{
		mpool.Free(i);
	}
	end_tm = xhnet::GetNowTime();

	cout << "cost time:" << end_tm - begin_tm << endl;

	getchar();

	return 0;
}
