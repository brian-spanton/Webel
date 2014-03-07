// Copyright © 2013 Brian Spanton

#pragma once

namespace Basic
{
	class SingleByteEncodingIndex : public ISingleByteEncodingIndex
	{
	public:
		typedef std::unordered_map<Codepoint, byte> CodepointMap;
		typedef Ref<SingleByteEncodingIndex, ISingleByteEncodingIndex> Ref;

		void Initialize();

		virtual Codepoint ISingleByteEncodingIndex::pointer_to_codepoint(byte pointer);
		virtual byte ISingleByteEncodingIndex::codepoint_to_pointer(Codepoint codepoint);

		Codepoint pointer_map[0x80];
		CodepointMap codepoint_map;
	};
}