// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IElementSource.h"

namespace Basic
{
	enum ConsumeElementsResult
	{
		succeeded,
		failed,
		in_progress,
	};

    template <typename element_type>
    __interface IElementConsumer
    {
        ConsumeElementsResult consume_elements(IElementSource<element_type>* element_source);
    };
}
