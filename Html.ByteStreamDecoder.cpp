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

    void ByteStreamDecoder::Initialize(Parser* parser, UnicodeString::Ref transport_charset, IStream<Codepoint>* output)
    {
        __super::Initialize();
        this->parser = parser;
        this->transport_charset = transport_charset;
        this->output = output;
        this->bom_frame.Initialize(this->bom, sizeof(this->bom));
    }

    void ByteStreamDecoder::Process(IEvent* event, bool* yield)
    {
        switch (frame_state())
        {
        case State::start_state:
            {
                this->unconsume = New<ByteString>();
                this->unconsume->reserve(1024);
                Event::AddObserver<byte>(event, this->unconsume);

                switch_to_state(State::bom_state);
            }
            break;

        case State::bom_state:
            if (this->bom_frame.Pending())
            {
                this->bom_frame.Process(event, yield);
            }
            
            if (this->bom_frame.Failed())
            {
                Event::RemoveObserver<byte>(event, this->unconsume);
                switch_to_state(State::bom_frame_failed);
            }
            else if (this->bom_frame.Succeeded())
            {
                if (memcmp(this->bom, Basic::globals->utf_16_big_endian_bom, sizeof(Basic::globals->utf_16_big_endian_bom)) == 0)
                {
                    this->encoding = Basic::globals->utf_16_big_endian_label;
                    this->confidence = Confidence_Certain;

                    this->unconsume->erase(this->unconsume->begin(), this->unconsume->begin() + _countof(Basic::globals->utf_16_big_endian_bom));

                    switch_to_state(State::sniff_done_state);
                }
                else if (memcmp(this->bom, Basic::globals->utf_16_little_endian_bom, sizeof(Basic::globals->utf_16_little_endian_bom)) == 0)
                {
                    this->encoding = Basic::globals->utf_16_little_endian_label;
                    this->confidence = Confidence_Certain;

                    this->unconsume->erase(this->unconsume->begin(), this->unconsume->begin() + _countof(Basic::globals->utf_16_little_endian_bom));

                    switch_to_state(State::sniff_done_state);
                }
                else if (memcmp(this->bom, Basic::globals->utf_8_bom, sizeof(Basic::globals->utf_8_bom)) == 0)
                {
                    this->encoding = Basic::globals->utf_8_label;
                    this->confidence = Confidence_Certain;

                    this->unconsume->erase(this->unconsume->begin(), this->unconsume->begin() + _countof(Basic::globals->utf_8_bom));

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
                if (this->transport_charset.item() == 0)
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
                Event::RemoveObserver<byte>(event, this->unconsume);

                Basic::globals->GetDecoder(this->encoding, &this->decoder);
                if (this->decoder.item() == 0)
                {
                    switch_to_state(State::get_decoder_failed);
                }
                else
                {
                    this->decoder->set_destination(this->output);

                    this->decoder->Write(this->unconsume->c_str(), this->unconsume->size());
                    switch_to_state(State::decoding_state);
                }
            }
            break;

        case State::decoding_state:
            {
                const byte* elements;
                uint32 count;

                if (!Event::Read(event, 0xffffffff, &elements, &count, yield))
                    return;

                this->decoder->Write(elements, count);
                (*yield) = true;
            }
            break;

        default:
            throw new Exception("Html::ByteStreamDecoder::Process unexpected state");
        }
    }
}