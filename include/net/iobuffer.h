
#pragma once

#include <string>
#include <string.h>
#include <vector>
#include <set>
#include <map>
#include <functional>

#include <xhhead.h>
#include <xhref.h>

namespace xhnet
{
	class CIOBuffer;
	typedef std::function<void(CIOBuffer* buf)> recycle_buf_cb;

	class CIOBuffer_Exception
	{
	public:
		enum 
		{
			none = 0,
			overflow,
		};

	public:
		CIOBuffer_Exception() : m_exception(none)
		{
		}

		CIOBuffer_Exception(int ep) : m_exception(ep)
		{
		}

		int		m_exception;
	};

	//区分下 read 和 write的 buffer
	// 增加set vector map的支持
	class CIOBuffer : public CPPRef
	{
	private:
		char*	m_pbase;	//缓冲区的头指针
		char*	m_pread;	//缓冲区中的读取当前指针
		char*	m_pwrite;	//缓冲区中的写入当前指针
		char*	m_pend;		//缓冲区的尾指针
		bool	m_biged;	// 
		unsigned char m_tag;

		recycle_buf_cb m_cb;
	public:
		explicit CIOBuffer(char* buff, unsigned int maxlen, bool biged=false)
			: m_pbase(buff)
			, m_pread(m_pbase)
			, m_pwrite(m_pbase)
			, m_pend(m_pbase + maxlen)
			, m_biged(biged)
		{

		}

		~CIOBuffer()
		{

		}

		unsigned int Size()
		{
			return m_pend - m_pbase;
		}

		char* GetBuffer()
		{
			return m_pbase;
		}

		void SetTag(unsigned char tag)
		{
			m_tag = tag;
		}

		unsigned char GetTag(void)
		{
			return m_tag;
		}

		// 只允许调用一次，
		// 调用多次会在对象回收的时候存在线程安全问题
		void Set_Recycle_Function(recycle_buf_cb cb)
		{
			if (!m_cb) return;
			m_cb = cb;
		}

		virtual void delete_this()
		{
			if ( m_cb )
			{
				m_cb(this);
			}
			else
			{
				CPPRef::delete_this();
			}
		}

		unsigned int LengthWrite()
		{
			return m_pwrite - m_pread;
		}

		unsigned int AvailWrite()
		{
			return m_pend - m_pwrite;
		}

		char* GetCurWrite()
		{
			return m_pwrite;
		}

		CIOBuffer& SeekWrite(int offset)
		{
			m_pwrite += offset;
			if (AvailWrite() < 0)
				throw CIOBuffer_Exception(CIOBuffer_Exception::overflow);

			return *this;
		}

		unsigned int LengthRead() 
		{
			return m_pread - m_pbase;
		}

		unsigned int AvailRead()
		{
			return m_pwrite - m_pread;
		}

		char* GetCurRead()
		{
			return m_pread;
		}

		CIOBuffer& SeekRead(int offset)
		{
			m_pread += offset;
			if (AvailRead()<0)
				throw CIOBuffer_Exception(CIOBuffer_Exception::overflow);

			return *this;
		}

	private:
		// write s
		CIOBuffer& write_order(const char* p, unsigned int len)
		{
			if (AvailWrite() < len)
				throw CIOBuffer_Exception(CIOBuffer_Exception::overflow);

			memcpy(m_pwrite, p, len);
			m_pwrite += len;

			return *this;
		}

		CIOBuffer& write_opposite(const char* p, unsigned int len)
		{
			if (AvailWrite() < len)
				throw CIOBuffer_Exception(CIOBuffer_Exception::overflow);

			unsigned int times = len / sizeof(char);
			char* pos = (char*)p + times - 1;

			do
			{
				*m_pwrite++ = *pos--;
				times--;
			} while (times);

			return *this;
		}

	public:
		CIOBuffer& Write(const char* p, unsigned int len)
		{
			return write_order( p, len );
		}

		CIOBuffer& Write_CareED(const char* p, unsigned int len)
		{
			if (XH_SYSTEM_BIGENDIAN() == m_biged)
			{
				return write_order(p, len);
			}
			else
			{
				return write_opposite(p, len);
			}
		}

		CIOBuffer& operator<<(char in)				{ return Write((const char*)&in, sizeof(char)); }
		CIOBuffer& operator<<(unsigned char in)		{ return Write((const char*)&in, sizeof(unsigned char)); }
		CIOBuffer& operator<<(short in)				{ return Write_CareED((const char*)&in, sizeof(short)); }
		CIOBuffer& operator<<(unsigned short in)	{ return Write_CareED((const char*)&in, sizeof(unsigned short)); }
		CIOBuffer& operator<<(int in)				{ return Write_CareED((const char*)&in, sizeof(int)); }
		CIOBuffer& operator<<(unsigned int in)		{ return Write_CareED((const char*)&in, sizeof(unsigned int)); }
		CIOBuffer& operator<<(long in)				{ return Write_CareED((const char*)&in, sizeof(long)); }
		CIOBuffer& operator<<(unsigned long in)		{ return Write_CareED((const char*)&in, sizeof(unsigned long)); }
		CIOBuffer& operator<<(long long in)			{ return Write_CareED((const char*)&in, sizeof(long long)); }
		CIOBuffer& operator<<(unsigned long long in){ return Write_CareED((const char*)&in, sizeof(unsigned long long)); }
		CIOBuffer& operator<<(float in)				{ return Write_CareED((const char*)&in, sizeof(float)); }
		CIOBuffer& operator<<(double in)			{ return Write_CareED((const char*)&in, sizeof(double)); }
		CIOBuffer& operator<<(long double in)		{ return Write_CareED((const char*)&in, sizeof(long double)); }
		CIOBuffer& operator<<(bool in)				{ return Write((const char*)&in, sizeof(bool)); }
		CIOBuffer& operator<<(const char* in)
		{
			int inlen = (int)strlen(in);

			if (inlen < 0xff)
			{
				operator<<((unsigned char)inlen);
			}
			else if (inlen < 0xffff)
			{
				operator<<((unsigned char)0xff);
				operator<<((unsigned short)inlen);
			}
			else
			{
				operator<<((unsigned char)0xff);
				operator<<((unsigned short)0xffff);
				operator<<((unsigned int)inlen);
			}
			return Write((const char*)in, inlen);
		}
		CIOBuffer& operator<<(char* in)				{ return operator<<((const char*)in); }
		CIOBuffer& operator<<(unsigned char* in)	{ return operator<<((const char*)in); }
		CIOBuffer& operator<<(const unsigned char* in)	{ return operator<<((const char*)in); }
		CIOBuffer& operator<<(const std::string& in){ return operator<<((const char*)in.c_str()); }

		CIOBuffer& writecstring(const char* _s)
		{
			return operator<<(_s);
		}

		template<class T>
		CIOBuffer& operator<<(const std::vector<T>& in)
		{
			unsigned int lth = in.size();

			this->operator<<(lth);
			for (auto it = in.begin(); it != in.end(); ++it)
			{
				this->operator<<(*it);
			}

			return *this;
		}

		template<class T>
		CIOBuffer& operator<<(const std::set<T>& in)
		{
			unsigned int lth = in.size();

			this->operator<<(lth);
			for (auto it = in.begin(); it != in.end(); ++it)
			{
				this->operator<<(*it);
			}

			return *this;
		}

		template<class K, class V>
		CIOBuffer& operator<<(const std::map<K, V>& in)
		{
			unsigned int lth = in.size();

			this->operator<<(lth);
			for (auto it = in.begin(); it != in.end(); ++it)
			{
				this->operator<<(it->first)<<(it->second);
			}

			return *this;
		}

	private:
		// read s
		CIOBuffer& read_order(char* p, unsigned int len)
		{
			if (AvailRead() < len)
				throw CIOBuffer_Exception(CIOBuffer_Exception::overflow);

			memcpy(p, m_pread, len);
			m_pread += len;

			return *this;
		}

		CIOBuffer& read_opposite(char* p, unsigned int len)
		{
			if (AvailRead() < len)
				throw CIOBuffer_Exception(CIOBuffer_Exception::overflow);

			size_t	times = len / sizeof(char);
			char*	pos = (char*)p + times - 1;

			do
			{
				*pos-- = *m_pread++;
				times--;
			} while (times);

			return *this;
		}

		CIOBuffer& read(std::string& p, size_t len) {
			if (AvailRead() < len)
				throw CIOBuffer_Exception(CIOBuffer_Exception::overflow);

			p.append(m_pread, len);
			m_pread += len;

			return *this;
		}

	public:
		CIOBuffer& Read(char* p, unsigned int len)
		{
			return read_order(p, len);
		}

		CIOBuffer& Read_CareED(char* p, size_t len)
		{
			if (XH_SYSTEM_BIGENDIAN() == m_biged)
			{
				return read_order(p, len);
			}
			else
			{
				return read_opposite(p, len);
			}
		}

		CIOBuffer& operator>>(char& out)				{ return Read(&out, sizeof(char)); }
		CIOBuffer& operator>>(unsigned char& out)		{ return Read((char*)&out, sizeof(unsigned char)); }
		CIOBuffer& operator>>(short& out)				{ return Read_CareED((char*)&out, sizeof(short)); }
		CIOBuffer& operator>>(unsigned short& out)		{ return Read_CareED((char*)&out, sizeof(unsigned short)); }
		CIOBuffer& operator>>(int& out)					{ return Read_CareED((char*)&out, sizeof(int)); }
		CIOBuffer& operator>>(unsigned int& out)		{ return Read_CareED((char*)&out, sizeof(unsigned int)); }
		CIOBuffer& operator>>(long& out)				{ return Read_CareED((char*)&out, sizeof(long)); }
		CIOBuffer& operator>>(unsigned long& out)		{ return Read_CareED((char*)&out, sizeof(unsigned long)); }
		CIOBuffer& operator>>(long long& out)			{ return Read_CareED((char*)&out, sizeof(long long)); }
		CIOBuffer& operator>>(unsigned long long& out)	{ return Read_CareED((char*)&out, sizeof(unsigned long long)); }
		CIOBuffer& operator>>(float& out)				{ return Read_CareED((char*)&out, sizeof(float)); }
		CIOBuffer& operator>>(double& out)				{ return Read_CareED((char*)&out, sizeof(double)); }
		CIOBuffer& operator>>(long double& out)			{ return Read_CareED((char*)&out, sizeof(long double)); }
		CIOBuffer& operator>>(bool& out)				{ return Read((char*)&out, sizeof(bool)); }
		CIOBuffer& operator>>(char* out)
		{
			unsigned char blen;
			operator>>(blen);
			if (blen < 0xff)
				return Read(out, blen);

			unsigned short wlen;
			operator>>(wlen);
			if (wlen < 0xffff)
				return Read(out, wlen);

			unsigned int dwlen;
			operator>>(dwlen);
			return Read(out, dwlen);
		}

		CIOBuffer& operator>>(unsigned char* out)					{ return operator>>((char*)out); }
		CIOBuffer& operator>>(std::string& out)
		{
			out.clear();
			unsigned char blen;
			operator>>(blen);
			if (blen < 0xff)
				return read(out, blen);

			unsigned short wlen;
			operator>>(wlen);
			if (wlen < 0xffff)
				return read(out, wlen);

			unsigned int dwlen;
			operator>>(dwlen);
			return read(out, dwlen);
		}

		CIOBuffer& readcstring(char* out, unsigned int outsize)
		{
			if (outsize <= 0) throw CIOBuffer_Exception(CIOBuffer_Exception::overflow);
			out[outsize - 1] = 0;

			unsigned char blen;
			operator>>(blen);
			if (blen < 0xff)
			{
				if ((unsigned int)blen >= outsize)
				{
					SeekRead(-(int)(sizeof(blen)));
					throw CIOBuffer_Exception(CIOBuffer_Exception::overflow);
				}
				return Read(out, blen);
			}

			unsigned short wlen;
			operator>>(wlen);
			if (wlen < 0xffff)
			{
				if ((unsigned int)wlen >= outsize)
				{
					SeekRead(-(int)(sizeof(blen) + sizeof(wlen)));
					throw CIOBuffer_Exception(CIOBuffer_Exception::overflow);
				}
				return Read(out, wlen);
			}

			unsigned int dwlen;
			operator>>(dwlen);
			if (dwlen >= outsize)
			{
				SeekRead(-(int)(sizeof(blen) + sizeof(wlen) + sizeof(dwlen)));
				throw CIOBuffer_Exception(CIOBuffer_Exception::overflow);
			}

			return Read(out, dwlen);
		}

		template<class T>
		CIOBuffer& operator>>(std::vector<T>& out)
		{
			out.clear();
			unsigned int lth = 0;
			this->operator>>(lth);
			for (unsigned int i = 0; i < lth; ++i)
			{
				T t;
				this->operator>>(t);
				out.push_back(t);
			}

			return *this;
		}

		template<class T>
		CIOBuffer& operator>>(std::set<T>& out)
		{
			out.clear();
			unsigned int lth = 0;
			this->operator>>(lth);
			for (unsigned int i = 0; i < lth; ++i)
			{
				T t;
				this->operator>>(t);
				out.insert(t);
			}

			return *this;
		}

		template<class K, class V>
		CIOBuffer& operator>>(std::map<K,V>& out)
		{
			out.clear();
			unsigned int lth = 0;
			this->operator>>(lth);
			for (unsigned int i = 0; i < lth; ++i)
			{
				K k;
				V v;
				this->operator>>(k)>>(v);
				out.insert(std::make_pair(k,v));
			}

			return *this;
		}


	public:
		CIOBuffer(const CIOBuffer&) = delete;
		CIOBuffer& operator=(const CIOBuffer&) = delete;

	};

};
