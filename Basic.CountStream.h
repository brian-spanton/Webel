// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"

namespace Basic
{
	template <class element_type>
	class CountStream : public IStream<element_type>
	{
	public:
		typedef Basic::Ref<CountStream<element_type> > Ref;

		uint32 count;

		CountStream() : 
			count(0)
		{
		}

		virtual void IStream<element_type>::Write(const element_type* elements, uint32 count)
		{
			this->count += count;
		}

		virtual void IStream<element_type>::WriteEOF()
		{
		}
	};
}