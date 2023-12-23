// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Globals.h"
#include "Basic.IElementSource.h"
#include "Basic.IElementConsumer.h"

namespace Basic
{
    template <class element_type>
    class ElementSource : public IElementSource<element_type>
    {
    private:
        const element_type* elements;
        uint32 count;
        uint32 elements_read;

    public:
        ElementSource<element_type>(const element_type* elements, uint32 count) :
			elements(elements),
			count(count),
			elements_read(0)
		{
        }

        bool Exhausted()
        {
            return (this->elements_read == this->count);
        }

        void Read(uint32 requested, const element_type** out_elements, uint32* out_count)
        {
			if (requested == 0)
                throw FatalError("Basic::ElementSource::Read count == 0");

            uint32 elements_remaining = this->count - this->elements_read;

            if (elements_remaining == 0)
                throw FatalError("Basic::ElementSource::Read elements_remaining == 0");

            const element_type* return_elements = this->elements + this->elements_read;
			uint32 return_count = (elements_remaining < requested) ? elements_remaining : requested;

            this->elements_read += return_count;

            (*out_elements) = return_elements;
            (*out_count) = return_count;
        }

        bool ReadNext(element_type* element)
        {
            uint32 elements_remaining = this->count - this->elements_read;

            if (elements_remaining == 0)
                return false;

            (*element) = this->elements[this->elements_read];
            this->elements_read++;

            return true;
        }

		bool deliver_elements(IElementConsumer<element_type>* element_consumer)
		{
			while (true)
			{
				if (this->Exhausted())
					return true;

				ConsumeElementsResult result = element_consumer->consume_elements(this);

				switch (result)
				{
				case ConsumeElementsResult::in_progress:
					continue;

				case ConsumeElementsResult::failed:
					return false;

				case ConsumeElementsResult::succeeded:
					if (!this->Exhausted())
						throw FatalError("!element_source.Exhausted()");

					return true;
				}
			}
		}
	};
}