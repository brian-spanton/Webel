// Copyright © 2013 Brian Spanton

#pragma once

namespace Basic
{
	template <class element_type>
	__interface IElementSource
	{
		bool Exhausted();
		void Read(uint32 count, const element_type** out_elements, uint32* out_count);
		bool ReadNext(element_type* element);
	};
}