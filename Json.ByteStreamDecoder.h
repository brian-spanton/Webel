// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Basic.Types.h"
#include "Basic.MemoryRange.h"
#include "Basic.Frame.h"
#include "Basic.IDecoder.h"
#include "Json.Tokenizer.h"

namespace Json
{
    using namespace Basic;

    class Parser;

    class ByteStreamDecoder : public Frame
    {
    private:
        enum State
        {
            leftovers_not_initialized_state = Start_State,
            bom_frame_pending_state,
            decoding_byte_stream,
            done_state = Succeeded_State,
            bom_frame_failed,
            could_not_guess_encoding_error,
            could_not_find_decoder_error,
        };

        UnicodeStringRef encoding;
        UnicodeStringRef charset;
        Tokenizer* output;
        byte bom[4];
        ByteStringRef leftovers;
        std::shared_ptr<IDecoder> decoder;
        MemoryRange bom_frame;

        virtual void IProcess::consider_event(IEvent* event);

    public:
        ByteStreamDecoder(UnicodeStringRef charset, Tokenizer* output);
    };
}