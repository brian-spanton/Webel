// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Globals.h"
#include "Basic.ElementSource.h"

namespace Basic
{
	template <class element_type>
	class ObservableElementSource : public IElementSource<element_type>
	{
	private:
		typedef std::shared_ptr<IStream<element_type> > StreamRef;
		typedef std::map<IStream<element_type>*, StreamRef> StreamMap; // $$ is a map best with shared_ptr, or can we use some other bag and not need the pointer key?

		IElementSource<element_type>* element_source;
		StreamMap observers;

		void Observe(const element_type* elements, uint32 count)
		{
			for (StreamMap::iterator it = this->observers.begin(); it != this->observers.end(); it++)
			{
				it->second->write_elements(elements, count);
			}
		}

	public:
		ObservableElementSource<element_type>(IElementSource<element_type>* element_source) :
			element_source(element_source)
		{
		}

		bool Exhausted()
		{
			return this->element_source->Exhausted();
		}

		void Read(uint32 requested, const element_type** out_elements, uint32* out_count)
		{
			this->element_source->Read(requested, out_elements, out_count);

			Observe(*out_elements, *out_count);
		}

		bool ReadNext(element_type* element)
		{
			bool success = this->element_source->ReadNext(element);
			if (!success)
				return false;

			Observe(element, 1);
			return true;
		}

		void AddObserver(std::shared_ptr<IStream<element_type> > stream)
		{
			this->observers.insert(StreamMap::value_type(stream.get(), stream));
		}

		void RemoveObserver(std::shared_ptr<IStream<element_type> > stream)
		{
			this->observers.erase(stream.get());
		}
	};
}