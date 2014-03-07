// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.SingleByteEncodingIndex.h"

namespace Basic
{
	void SingleByteEncodingIndex::Initialize()
	{
		ZeroMemory(pointer_map, sizeof(pointer_map));
	}

	Codepoint SingleByteEncodingIndex::pointer_to_codepoint(byte pointer)
	{
		return this->pointer_map[pointer];
	}

	byte SingleByteEncodingIndex::codepoint_to_pointer(Codepoint codepoint)
	{
		CodepointMap::iterator it = this->codepoint_map.find(codepoint);
		if (it == this->codepoint_map.end())
			return 0;

		return it->second;
	}
}