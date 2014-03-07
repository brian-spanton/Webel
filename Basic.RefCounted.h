// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Lock.h"
#include "Basic.Hold.h"

namespace Basic
{
	template <class T>
	class InterlockedRefCount : public T
	{
	private:
		volatile unsigned long references;

	public:
		InterlockedRefCount() :
			references(0)
		{
		}

		void AddRef(IRefHolder* holder)
		{
			InterlockedIncrement(&this->references);
		}

		void Release(IRefHolder* holder)
		{
			unsigned long result = InterlockedDecrement(&this->references);

			if (result == 0)
				delete this;
		}
	};

	template <class T>
	class SimpleRefCount : public T
	{
	private:
		unsigned long references;

	public:
		SimpleRefCount() :
			references(0)
		{
		}

		void AddRef(IRefHolder* holder)
		{
			this->references += 1;
		}

		void Release(IRefHolder* holder)
		{
			this->references -= 1;

			if (this->references == 0)
				delete this;
		}
	};

	template <class T>
	class Inline : public T
	{
	public:
		void AddRef(IRefHolder* holder)
		{
		}

		void Release(IRefHolder* holder)
		{
		}
	};

	void register_object(IRefCounted* object);
	void set_object_name(IRefCounted* object, const char* name);
	void unregister_object(IRefCounted* object);
	void set_objects_baseline();
	void check_new_objects();

	template <class T>
	class DebugRefCount : public T, public IRefCounted
	{
	private:
		typedef std::unordered_multiset<IRefHolder*> References;

		Lock lock;
		References references;

	public:
		DebugRefCount()
		{
			register_object(reinterpret_cast<IRefCounted*>(this));
		}

		virtual ~DebugRefCount()
		{
			unregister_object(reinterpret_cast<IRefCounted*>(this));
		}

		void AddRef(IRefHolder* holder)
		{
			Hold hold(this->lock);

			this->references.insert(holder);
		}

		void Release(IRefHolder* holder)
		{
			bool last_ref = false;

			{
				// this was a hard to find bug - if hold is destructed after "delete this"
				// then the lock it refers to is deleted and badness happens
				Hold hold(this->lock);

				References::_Pairii range = this->references.equal_range(holder);

				if (range.first == this->references.end())
					throw new Exception("tried to release non-existent reference");

				this->references.erase(range.first);

				last_ref = this->references.empty();
			}

			if (last_ref)
				delete this;
		}
	};

	template <class T>
	T* New()
	{
		// brian: change this to use one of the other types above for different behaviors
		return new InterlockedRefCount<T>();
	}

	template <class T>
	T* New(const char* name)
	{
		InterlockedRefCount<T>* object = new InterlockedRefCount<T>();

		// $ uncomment this to store an object name for correlation
		//set_object_name(reinterpret_cast<IRefCounted*>(object), name);

		return object;
	}
}