#pragma once

#include "Basic.IRefCounted.h"

namespace Basic
{
	template <class T, class D = T>
	class Ref
	{
	protected:
		T* instance;
		IRefHolder* holder;

	public:
		Ref() :
			instance(0),
			holder(0)
		{
		}

		Ref(T* instance) :
			instance(instance),
			holder(0)
		{
			if (this->instance != 0)
				static_cast<D*>(this->instance)->AddRef(this->holder);
		}

		Ref(const Ref<T, D>& ref) :
			instance(ref.instance),
			holder(0)
		{
			if (this->instance != 0)
				static_cast<D*>(this->instance)->AddRef(this->holder);
		}

		~Ref()
		{
			if (this->instance != 0)
				static_cast<D*>(this->instance)->Release(this->holder);
		}

		void SetHolder(IRefHolder* holder)
		{
			this->holder = holder;
		}

		void operator = (T* rvalue)
		{
			if (this->instance == rvalue)
				return;

			if (rvalue != 0)
				static_cast<D*>(rvalue)->AddRef(this->holder);

			if (this->instance != 0)
				static_cast<D*>(this->instance)->Release(this->holder);

			this->instance = rvalue;
		}

		void operator = (const Ref<T, D>& rvalue)
		{
			operator = (rvalue.instance);
		}

		void Release()
		{
			if (this->instance != 0)
			{
				static_cast<D*>(this->instance)->Release(this->holder);
				this->instance = 0;
			}
		}

		T* operator -> () const
		{
			return this->instance;
		}

		operator const T* () const
		{
			return this->instance;
		}

		operator T* () const
		{
			return this->instance;
		}

		T* item() const
		{
			return this->instance;
		}

		bool operator == (const Ref<T, D>& rvalue) const
		{
			return this->instance == rvalue.instance;
		}

		bool operator != (const Ref<T, D>& rvalue) const
		{
			return this->instance != rvalue.instance;
		}
	};
}