// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Json.ByteStreamDecoder.h"
#include "Json.Globals.h"
#include "Basic.Globals.h"
#include "Basic.Event.h"

namespace Json
{
    using namespace Basic;

    ByteStreamDecoder::ByteStreamDecoder(UnicodeStringRef charset, Tokenizer* output) :
        charset(charset),
        output(output),
        lead_bytes_frame(this->lead_bytes, sizeof(this->lead_bytes)) // initialization is in order of declaration in class def
    {
    }

    void ByteStreamDecoder::write_element(byte b)
    {
        switch (this->get_state())
        {
        case State::lead_bytes_frame_pending_state:
            {
                this->lead_bytes_frame.write_element(b);

                if (this->lead_bytes_frame.in_progress())
                    return;

                if (this->lead_bytes_frame.failed())
                {
                    switch_to_state(State::bom_frame_failed);
                    return;
                }

                if (this->lead_bytes[0] == 0 &&
                    this->lead_bytes[1] == 0 &&
                    this->lead_bytes[2] == 0 &&
                    this->lead_bytes[3] != 0)
                {
                    this->encoding = Basic::globals->utf_32_big_endian_label;
                }
                else if (
                    this->lead_bytes[0] == 0 &&
                    this->lead_bytes[1] != 0 &&
                    this->lead_bytes[2] == 0 &&
                    this->lead_bytes[3] != 0)
                {
                    this->encoding = Basic::globals->utf_16_big_endian_label;
                }
                else if (
                    this->lead_bytes[0] != 0 &&
                    this->lead_bytes[1] == 0 &&
                    this->lead_bytes[2] == 0 &&
                    this->lead_bytes[3] == 0)
                {
                    this->encoding = Basic::globals->utf_32_little_endian_label;
                }
                else if (
                    this->lead_bytes[0] != 0 &&
                    this->lead_bytes[1] == 0 &&
                    this->lead_bytes[2] != 0 &&
                    this->lead_bytes[3] == 0)
                {
                    this->encoding = Basic::globals->utf_16_little_endian_label;
                }
                else if (
                    this->lead_bytes[0] != 0 &&
                    this->lead_bytes[1] != 0 &&
                    this->lead_bytes[2] != 0 &&
                    this->lead_bytes[3] != 0)
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
                    return;
                }

                this->decoder->set_destination(this->output);
                switch_to_state(State::decoding_byte_stream);

                // recursion to re-process these bytes
                this->write_elements(this->lead_bytes, _countof(this->lead_bytes));
                return;
            }
            break;

        case State::decoding_byte_stream:
            this->decoder->write_element(b);
            break;

        default:
            throw FatalError("Json", "ByteStreamDecoder::write_element unhandled state");
        }
    }
}