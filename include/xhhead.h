
#pragma once

#if defined _WINDOWS_ || WIN32
#define _PLATFORM_WINDOWS_
#else
#define _PLATFORM_LINUX_
#endif


namespace xhnet
{
	inline bool is_bigendian()
	{
		static unsigned short data = 0x1122;
		static unsigned char* pdata = (unsigned char*)&data;
		static bool bbigendian = (*pdata == 0x11);

		return bbigendian;
	}


	template<class S1, class S2, bool b>
	struct ifclass
	{
		S1 m_addr;
	};

	template<class S1, class S2>
	struct ifclass < S1, S2, false >
	{
		S2 m_addr;
	};
};


#define XH_CONCATENATE_DIRECT(s1, s2)			s1##s2
#define XH_CONCATENATE(s1, s2)					XH_CONCATENATE_DIRECT(s1, s2)
#define XH_ANONYMOUS_VARIABLE(str)				XH_CONCATENATE(str, __LINE__)


#define XH_IS_BIGENDIAN()						xhnet::is_bigendian()

#define XH_SAFE_DELETE(p)						do{if(p) delete (p); (p)=0;}while(0)
#define XH_SAFE_RELEASE(p)						do{if(p) (p)->Release(); (p)=0;}while(0)



