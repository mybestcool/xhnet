
#pragma once

#include <atomic>

namespace xhnet
{
	//
	//
	// 线程安全计数
	//

	class CPPRef
	{
	public:
		virtual ~CPPRef() { }

		void Retain()
		{
			++m_ref;
		}

		void Release()
		{
			if (m_ref.fetch_sub(1) == 1)
			{
				delete_this();
			}
		}

		unsigned int GetRefCount() const
		{
			return m_ref;
		}

		virtual void delete_this()
		{
			delete this;
		}

	protected:
		CPPRef()
			: m_ref(1)
		{
		}

		std::atomic<unsigned int> m_ref;
	};


	template<class T>
	class CPPRefPtr
	{
	public:
		CPPRefPtr(T* p)
		{
			_p = p;
			if (_p)     _p->Retain();
		}

		CPPRefPtr(const CPPRefPtr& p)
		{
			_p = p._p;
			if (_p)     _p->Retain();
		}

		explicit CPPRefPtr(void) :_p(NULL){}

		~CPPRefPtr(void)
		{
			if (_p) _p->Release();
		}

		operator T*(void) const { return _p; }
		//T& operator*(void)const	{ return *_p;	}
		T* operator->(void)const{ return _p; }
		T* Get(void) const		{ return _p; }
		int GetRefCount()		{ return _p != NULL ? _p->GetRefCount() : 0; }


		template<class P>
		CPPRefPtr<T>& operator=(const CPPRefPtr<P>& p)
		{
			if (this != &p && _p != p._p)
			{
				T* pSrc = _p;
				_p = p._p;

				if (_p)     _p->Retain();
				if (pSrc)	pSrc->Release();
			}

			return *this;
		}

		template<class P>
		CPPRefPtr<T>& operator=(P* p)
		{
			if (_p != p)
			{
				T* pSrc = _p;
				_p = p;

				if (_p)     _p->Retain();
				if (pSrc)	pSrc->Release();
			}

			return *this;
		}

		//bool operator==(const CPPRefPtr<T>& p) const
		//{
		//	if (_p == p.get())
		//	{
		//		return true;
		//	}
		//	return false;
		//}

		//bool operator!=(const CPPRefPtr<T>& p) const
		//{
		//	if (_p != p.get())
		//	{
		//		return true;
		//	}
		//	return false;
		//}

	private:
		T*	_p;
	};
};

