// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IElementConsumer.h"

namespace Basic
{
    template <typename element_type>
    class IgnoreFrame : public IElementConsumer<element_type>
    {
    private:
        uint64 expected;
        uint64 received;

    public:
        IgnoreFrame() :
            received(0),
            expected(0xffffffffffffffff)
        {
        }

        void reset(uint64 expected)
        {
            this->received = 0;
            this->expected = expected;
        }

		ConsumeElementsResult IElementConsumer<element_type>::consume_elements(IElementSource<element_type>* element_source)
        {
			const element_type* elements;
            uint32 count;

            uint64 need = this->expected - this->received;
			uint32 consume = need > 0xffffffff ? 0xffffffff : (uint32)need;

            element_source->Read(consume, &elements, &count);

            this->received += count;

            if (this->received != this->expected)
                return ConsumeElementsResult::in_progress;

            return ConsumeElementsResult::succeeded;
        }
    };
}