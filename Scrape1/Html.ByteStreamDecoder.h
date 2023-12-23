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

    class ByteStreamDecoder : public StateMachine, public UnitStream<byte>
    {
    private:
        enum State
        {
            bom_frame_pending_state = Start_State,
            media_type_state,
            prescan_state,
            nested_browsing_context_state,
            previous_sniff_state,
            frequency_analysis_state,
            locale_state,
            guess_state,
            sniff_done_state,
            decoding_state,
            bom_frame_failed = Succeeded_State + 1,
            get_decoder_failed,
        };

        UnicodeStringRef encoding;
        UnicodeStringRef transport_charset;
        std::shared_ptr<IStream<Codepoint> > output;
        Parser* parser;
        byte bom[3];
        std::shared_ptr<IDecoder> decoder;
        MemoryRange bom_frame;
        Confidence confidence;

    public:
        ByteStreamDecoder(Parser* parser, UnicodeStringRef transport_charset, std::shared_ptr<IStream<Codepoint> > output);

        virtual void IStream<byte>::write_element(byte element);
    };
}