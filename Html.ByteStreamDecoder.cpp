// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.ByteStreamDecoder.h"
#include "Html.Globals.h"
#include "Html.Parser.h"
#include "Html.Types.h"
#include "Basic.StreamFrame.h"

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

    void ByteStreamDecoder::consider_event(IEvent* event)
    {
        if (event->get_type() == EventType::element_stream_ending_event)
            throw Yield("event consumed");

        switch (get_state())
        {
        case State::start_state:
            {
                this->leftovers = std::make_shared<ByteString>();
                this->leftovers->reserve(1024);
                Event::AddObserver<byte>(event, this->leftovers);

                switch_to_state(State::bom_state);
            }
            break;

        case State::bom_state:
            {
                delegate_event(&this->bom_frame, event);
            
                if (this->bom_frame.failed())
                {
                    Event::RemoveObserver<byte>(event, this->leftovers);
                    switch_to_state(State::bom_frame_failed);
                    return;
                }

                if (memcmp(this->bom, Basic::globals->utf_16_big_endian_bom, sizeof(Basic::globals->utf_16_big_endian_bom)) == 0)
                {
                    this->encoding = Basic::globals->utf_16_big_endian_label;
                    this->confidence = Confidence_Certain;

                    this->leftovers->erase(this->leftovers->begin(), this->leftovers->begin() + _countof(Basic::globals->utf_16_big_endian_bom));

                    switch_to_state(State::sniff_done_state);
                }
                else if (memcmp(this->bom, Basic::globals->utf_16_little_endian_bom, sizeof(Basic::globals->utf_16_little_endian_bom)) == 0)
                {
                    this->encoding = Basic::globals->utf_16_little_endian_label;
                    this->confidence = Confidence_Certain;

                    this->leftovers->erase(this->leftovers->begin(), this->leftovers->begin() + _countof(Basic::globals->utf_16_little_endian_bom));

                    switch_to_state(State::sniff_done_state);
                }
                else if (memcmp(this->bom, Basic::globals->utf_8_bom, sizeof(Basic::globals->utf_8_bom)) == 0)
                {
                    this->encoding = Basic::globals->utf_8_label;
                    this->confidence = Confidence_Certain;

                    this->leftovers->erase(this->leftovers->begin(), this->leftovers->begin() + _countof(Basic::globals->utf_8_bom));

                    switch_to_state(State::sniff_done_state);
                }
                else
                {
                    switch_to_state(State::media_type_state);
                }
            }
            break;

        case State::media_type_state:
            {
                if (this->transport_charset.get() == 0)
                {
                    switch_to_state(State::prescan_state);
                }
                else
                {
                    this->encoding = this->transport_charset;
                    this->confidence = Confidence_Certain;
                    switch_to_state(State::sniff_done_state);
                }
            }
            break;

        case State::prescan_state:
            // $ NYI
            switch_to_state(State::nested_browsing_context_state);
            break;

        case State::nested_browsing_context_state:
            // $ NYI
            switch_to_state(State::previous_sniff_state);
            break;    

        case State::previous_sniff_state:
            // $ NYI
            switch_to_state(State::frequency_analysis_state);
            break;    

        case State::frequency_analysis_state:
            // $ NYI
            switch_to_state(State::locale_state);
            break;    

        case State::locale_state:
            // $ NYI
            switch_to_state(State::guess_state);
            break;    

        case State::guess_state:
            this->encoding = Basic::globals->utf_8_label;
            this->confidence = Confidence_Tentative;

            switch_to_state(State::sniff_done_state);
            break;

        case State::sniff_done_state:
            {
                Event::RemoveObserver<byte>(event, this->leftovers);

                Basic::globals->GetDecoder(this->encoding, &this->decoder);
                if (this->decoder.get() == 0)
                {
                    switch_to_state(State::get_decoder_failed);
                }
                else
                {
                    this->decoder->set_destination(this->output.get());

                    this->decoder->write_elements(this->leftovers->address(), this->leftovers->size());
                    switch_to_state(State::decoding_state);
                }
            }
            break;

        case State::decoding_state:
            {
                const byte* elements;
                uint32 count;

                Event::Read(event, 0xffffffff, &elements, &count);

                this->decoder->write_elements(elements, count);
            }
            break;

        default:
            throw FatalError("Html::ByteStreamDecoder::handle_event unexpected state");
        }
    }
}