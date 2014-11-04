
#pragma once

#if defined _WINDOWS_ || WIN32
#define _PLATFORM_WINDOWS_
#else
#define _PLATFORM_LINUX_
#endif


namespace xhnet
{
	// 返回true表示bigendian,false表示littleendian
	inline bool get_system_endian()
	{
		static unsigned short data = 0x1122;
		static unsigned char* pdata = (unsigned char*)&data;
		static bool bbigendian = (*pdata == 0x11);

		return bbigendian;
	}
};


#define XH_CONCATENATE_DIRECT(s1, s2)			s1##s2
#define XH_CONCATENATE(s1, s2)					XH_CONCATENATE_DIRECT(s1, s2)
#define XH_ANONYMOUS_VARIABLE(str)				XH_CONCATENATE(str, __LINE__)


#define XH_SYSTEM_BIGENDIAN()					xhnet::get_system_endian()

#define XH_SAFE_DELETE(p)						do{if(p) delete (p); (p)=0;}while(0)
#define XH_SAFE_RELEASE(p)						do{if(p) (p)->Release(); (p)=0;}while(0)



