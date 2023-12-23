// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ISingleByteEncodingIndex.h"

namespace Basic
{
    // $$$ should be a persistent object, higher level than basic
    class SingleByteEncodingIndex : public ISingleByteEncodingIndex
    {
    public:
        typedef std::unordered_map<Codepoint, byte> CodepointMap;

        void Initialize();

        virtual Codepoint ISingleByteEncodingIndex::pointer_to_codepoint(byte pointer);
        virtual byte ISingleByteEncodingIndex::codepoint_to_pointer(Codepoint codepoint);

        Codepoint pointer_map[0x80];
        CodepointMap codepoint_map;
    };
}