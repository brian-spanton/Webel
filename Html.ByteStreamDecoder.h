// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Types.h"
#include "Html.Node.h"
#include "Html.Token.h"
#include "Basic.Frame.h"
#include "Basic.Types.h"
#include "Basic.MemoryRange.h"
#include "Basic.IDecoder.h"

namespace Html
{
    using namespace Basic;

    class Parser;

    class ByteStreamDecoder : public Frame
    {
    private:
        enum State
        {
            start_state = Start_State,
            bom_state,
            media_type_state,
            prescan_state,
            nested_browsing_context_state,
            previous_sniff_state,
            frequency_analysis_state,
            locale_state,
            guess_state,
            sniff_done_state,
            decoding_state,
            done_state = Succeeded_State,
            bom_frame_failed,
            get_decoder_failed,
        };

        UnicodeStringRef encoding;
        UnicodeStringRef transport_charset;
        std::shared_ptr<IStream<Codepoint> > output;
        Parser* parser;
        byte bom[3];
        ByteStringRef leftovers;
        std::shared_ptr<IDecoder> decoder;
        MemoryRange bom_frame;
        Confidence confidence;

        virtual void IProcess::consider_event(IEvent* event);

    public:
        ByteStreamDecoder(Parser* parser, UnicodeStringRef transport_charset, std::shared_ptr<IStream<Codepoint> > output);

    };
}