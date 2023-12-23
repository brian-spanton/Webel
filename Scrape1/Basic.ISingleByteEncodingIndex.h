// Copyright © 2013 Brian Spanton

#pragma once

namespace Basic
{
    __interface ISingleByteEncodingIndex
    {
        Codepoint pointer_to_codepoint(byte pointer);
        byte codepoint_to_pointer(Codepoint codepoint);
    };
}