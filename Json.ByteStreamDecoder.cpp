// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Json.ByteStreamDecoder.h"
#include "Json.Globals.h"
#include "Basic.StreamFrame.h"
#include "Basic.Globals.h"
#include "Basic.Event.h"

namespace Json
{
    using namespace Basic;

    ByteStreamDecoder::ByteStreamDecoder(UnicodeStringRef charset, Tokenizer* output) :
        charset(charset),
        output(output),
        bom_frame(this->bom, sizeof(this->bom))
    {
    }

    void ByteStreamDecoder::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::unconsume_not_initialized_state:
            {
                this->not_consumed = std::make_shared<ByteString>();
                this->not_consumed->reserve(1024);
                Event::AddObserver<byte>(event, this->not_consumed);

                switch_to_state(State::bom_frame_pending_state);
            }
            break;

        case State::bom_frame_pending_state:
            {
                delegate_event(&this->bom_frame, event);
            
                Event::RemoveObserver<byte>(event, this->not_consumed);

                if (this->bom_frame.failed())
                {
                    switch_to_state(State::bom_frame_failed);
                    return;
                }

                if (bom[0] == 0 && bom[1] == 0 && bom[2] == 0 && bom[3] != 0)
                {
                    this->encoding = Basic::globals->utf_32_big_endian_label;
                }
                else if (bom[0] == 0 && bom[1] != 0 && bom[2] == 0 && bom[3] != 0)
                {
                    this->encoding = Basic::globals->utf_16_big_endian_label;
                }
                else if (bom[0] != 0 && bom[1] == 0 && bom[2] == 0 && bom[3] == 0)
                {
                    this->encoding = Basic::globals->utf_32_little_endian_label;
                }
                else if (bom[0] != 0 && bom[1] == 0 && bom[2] != 0 && bom[3] == 0)
                {
                    this->encoding = Basic::globals->utf_16_little_endian_label;
                }
                else if (bom[0] != 0 && bom[1] != 0 && bom[2] != 0 && bom[3] != 0)
                {
                    this->encoding = Basic::globals->utf_8_label;
                }
                else if (this->charset.get() != 0)
                {
                    this->encoding = this->charset;
                }
                else
                {
                    switch_to_state(State::could_not_guess_encoding_error);
                    return;
                }

                Basic::globals->GetDecoder(this->encoding, &this->decoder);
                if (this->decoder.get() == 0)
                {
                    switch_to_state(State::could_not_find_decoder_error);
                }
                else
                {
                    this->decoder->set_destination(this->output);

                    this->decoder->write_elements(this->not_consumed->address(), this->not_consumed->size());
                    switch_to_state(State::decoding_byte_stream);
                }
            }
            break;

        case State::decoding_byte_stream:
            {
                const byte* elements;
                uint32 count;

                Event::Read(event, 0xffffffff, &elements, &count);

                this->decoder->write_elements(elements, count);
            }
            break;

        default:
            throw FatalError("Json::ByteStreamDecoder::handle_event unexpected state");
        }
    }
}