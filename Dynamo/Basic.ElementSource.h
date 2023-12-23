#pragma once

#include "Basic.IElementSource.h"
#include "Basic.Globals.h"

namespace Basic
{
	template <class T>
	class ElementSource : public IElementSource<T>
	{
	private:
		typedef Ref<IStream<T> > StreamRef;
		typedef std::map<IStream<T>*, StreamRef> StreamMap;

		const T* elements;
		uint32 count;
		uint32 elements_read;

		StreamMap observers;
		uint32 elements_observed;

		void Observe()
		{
			if (this->elements_observed >= this->elements_read)
				return;

			uint32 observing = this->elements_read - this->elements_observed;

			for (StreamMap::iterator it = this->observers.begin(); it != this->observers.end(); it++)
			{
				it->second->Write(this->elements + this->elements_observed, observing);
			}

			this->elements_observed += observing;
		}

	public:
		typedef Basic::Ref<ElementSource<T> > Ref;

		void Initialize(const T* elements, uint32 count)
		{
			this->elements = elements;
			this->count = count;
			this->elements_read = 0;
			this->elements_observed = 0;
		}

		bool Exhausted()
		{
			return (this->elements_read == this->count);
		}

		bool Read(uint32 count, const T** out_address, uint32* out_count, bool* yield)
		{
			if (count == 0)
				throw new Exception("Basic::ElementSource::Read count == 0");

			uint32 elements_remaining = this->count - this->elements_read;
			if (elements_remaining == 0)
			{
				(*out_address) = 0;
				(*out_count) = 0;
				(*yield) = true;
				return false;
			}
			else
			{
				const T* return_address = this->elements + this->elements_read;
				uint32 return_count = (elements_remaining < count) ? elements_remaining : count;

				this->elements_read += return_count;

				Observe();

				(*out_address) = return_address;
				(*out_count) = return_count;
				(*yield) = false;
				return true;
			}
		}

		bool ReadNext(T* element, bool* yield)
		{
			uint32 elements_remaining = this->count - this->elements_read;
			if (elements_remaining == 0)
			{
				(*yield) = true;
				return false;
			}
			else
			{
				(*element) = this->elements[this->elements_read];
				this->elements_read++;

				Observe();

				(*yield) = false;
				return true;
			}
		}

		void UndoReadNext()
		{
			if (this->elements_read == 0)
				throw new Exception("Basic::ElementSource::UndoReadNext");

			this->elements_read--;
		}

		void AddObserver(IStream<T>* stream)
		{
			this->observers.insert(StreamMap::value_type(stream, stream));
		}

		void RemoveObserver(IStream<T>* stream)
		{
			this->observers.erase(stream);
		}
	};
}