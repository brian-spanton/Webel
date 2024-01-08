// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.ByteStreamDecoder.h"
#include "Html.Globals.h"
#include "Html.Parser.h"
#include "Html.Types.h"
#include "Basic.Globals.h"

namespace Html
{
    using namespace Basic;

    ByteStreamDecoder::ByteStreamDecoder(Parser* parser, UnicodeStringRef transport_charset, std::shared_ptr<IStream<Codepoint> > output) :
        parser(parser),
        transport_charset(transport_charset),
        output(output),
        bom_frame(this->bom, sizeof(this->bom)) // initialization is in order of declaration in class def
    {
    }

    void ByteStreamDecoder::write_element(byte b)
    {
        switch (get_state())
        {
        case State::bom_frame_pending_state:
            {
                this->bom_frame.write_element(b);

                if (this->bom_frame.in_progress())
                    return;

                if (this->bom_frame.failed())
                {
                    switch_to_state(State::bom_frame_failed);
                    return;
                }

                uint8 consumed = 0;

                if (memcmp(this->bom, Basic::globals->utf_16_big_endian_bom, sizeof(Basic::globals->utf_16_big_endian_bom)) == 0)
                {
                    this->encoding = Basic::globals->utf_16_big_endian_label;
                    this->confidence = Confidence_Certain;
                    consumed = _countof(Basic::globals->utf_16_big_endian_bom);
                }
                else if (memcmp(this->bom, Basic::globals->utf_16_little_endian_bom, sizeof(Basic::globals->utf_16_little_endian_bom)) == 0)
                {
                    this->encoding = Basic::globals->utf_16_little_endian_label;
                    this->confidence = Confidence_Certain;
                    consumed = _countof(Basic::globals->utf_16_little_endian_bom);
                }
                else if (memcmp(this->bom, Basic::globals->utf_8_bom, sizeof(Basic::globals->utf_8_bom)) == 0)
                {
                    this->encoding = Basic::globals->utf_8_label;
                    this->confidence = Confidence_Certain;
                    consumed = _countof(Basic::globals->utf_8_bom);
                }
                else if (this->transport_charset.get() != 0)
                {
                    this->encoding = this->transport_charset;
                    this->confidence = Confidence_Certain;
                }
                // $ NYI: prescan
                // $ NYI: nested_browsing_context
                // $ NYI: previous_sniff
                // $ NYI: frequency_analysis
                // $ NYI: locale
                else
                {
                    this->encoding = Basic::globals->utf_8_label;
                    this->confidence = Confidence_Tentative;
                }

                Basic::globals->GetDecoder(this->encoding, &this->decoder);
                if (this->decoder.get() == 0)
                {
                    switch_to_state(State::get_decoder_failed);
                    return;
                }

                this->decoder->set_destination(this->output.get());
                switch_to_state(State::decoding_state);

                // recursion to re-process these bytes
                this->write_elements(this->bom + consumed, _countof(this->bom) - consumed);
                return;
            }
            break;

        case State::decoding_state:
            this->decoder->write_element(b);
            break;

        default:
            throw FatalError("Html", "ByteStreamDecoder::process_event unhandled state");
        }
    }
}