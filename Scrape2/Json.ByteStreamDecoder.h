// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Types.h"
#include "Basic.MemoryRange.h"
#include "Basic.IDecoder.h"
#include "Json.Tokenizer.h"

namespace Json
{
    using namespace Basic;

    class Parser;

    class ByteStreamDecoder : public StateMachine, public UnitStream<byte>
    {
    private:
        enum State
        {
            lead_bytes_frame_pending_state = Start_State,
            decoding_byte_stream,
			could_not_guess_encoding_error = Succeeded_State + 1,
            could_not_find_decoder_error,
        };

        UnicodeStringRef encoding;
        UnicodeStringRef charset;
        Tokenizer* output;
        byte lead_bytes[4];
        std::shared_ptr<IDecoder> decoder;
        MemoryRange lead_bytes_frame;

    public:
        ByteStreamDecoder(UnicodeStringRef charset, Tokenizer* output);

        virtual void IStream<byte>::write_element(byte element);
    };
}