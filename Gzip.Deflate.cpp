// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Gzip.MemberFrame.h"
#include "Gzip.Deflate.h"

namespace Gzip
{
    uint16 Deflate::masks[] = { 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f };
    byte Deflate::HCLEN_index[] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

    Deflate::Deflate(std::shared_ptr<IStream<byte> > uncompressed) :
        uncompressed(uncompressed),
        LEN_frame(&this->LEN),
        NLEN_frame(&this->NLEN)
    {
    }

    EventResult Deflate::read_next(IEvent* event, byte* output, byte count)
    {
        byte b;
        uint16 w = 0;

        if (this->count < count)
        {
            EventResult result = Event::ReadNext(event, &b);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            w = (uint16)b << this->count;
            this->bits |= w;
            this->count += 8;
        }

        b = (byte)(this->bits & Deflate::masks[count]);
        (*output) = b;

        this->bits = this->bits >> count;
        this->count -= count;

        return EventResult::event_result_continue;
    }

    EventResult Deflate::consider_event(IEvent* event)
    {
        // RFC1951 https://www.rfc-editor.org/rfc/rfc1951

        EventResult result;
        byte b;
        uint16 w = 0;

        switch (get_state())
        {
        case State::start_state:
            if (this->BFINAL)
                switch_to_state(State::done_state);
            else
                switch_to_state(State::BFINAL_state);

            break;

        case State::BFINAL_state:
            result = read_next(event, &b, 1);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            this->BFINAL = (bool)b;
            switch_to_state(BTYPE_state);
            break;

        case State::BTYPE_state:
            {
                result = read_next(event, &b, 2);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                BlockType BTYPE = (BlockType)b;
                switch (BTYPE)
                {
                case BlockType::no_compression:
                    switch_to_state(State::block_state);
                    break;

                case BlockType::fixed:
                    switch_to_state(State::fixed_block_state);
                    break;

                case BlockType::dynamic:
                    switch_to_state(State::dynamic_block_state);
                    break;

                default:
                    switch_to_state(State::BTYPE_failed);
                    break;
                }
            }
            break;

        case State::block_state:
            this->bits = 0;
            this->count = 0;

            switch_to_state(State::LEN_state);
            break;

        case State::LEN_state:
            result = delegate_event_change_state_on_fail(&this->LEN_frame, event, State::LEN_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            this->uncompressed_frame.reset(this->uncompressed, this->LEN);

            switch_to_state(State::NLEN_state);
            break;

        case State::NLEN_state:
            {
                result = delegate_event_change_state_on_fail(&this->NLEN_frame, event, State::NLEN_failed);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                uint16 ones_complement = this->LEN ^ 0xffff;
                if (this->NLEN != ones_complement)
                {
                    switch_to_state(NLEN_failed);
                    return EventResult::event_result_yield;
                }

                switch_to_state(State::uncompressed_data_state);
            }
            break;

        case State::uncompressed_data_state:
            result = delegate_event_change_state_on_fail(&this->uncompressed_frame, event, State::uncompressed_data_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::start_state);
            break;

        case State::fixed_block_state:
            // $$$ reset fixed alphabets
            switch_to_state(State::compressed_data_state);
            break;

        case State::dynamic_block_state:
            // $$$ reset dynamic alphabets
            switch_to_state(State::HLIT_state);
            break;

        case State::HLIT_state:
            result = read_next(event, &b, 5);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            this->HLIT = (uint16)b + 257;
            switch_to_state(HDIST_state);
            break;

        case State::HDIST_state:
            result = read_next(event, &b, 5);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            this->HDIST = (byte)b + 1;
            switch_to_state(HCLEN_state);
            break;

        case State::HCLEN_state:
            result = read_next(event, &b, 4);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            this->HCLEN = (byte)b + 4;
            this->HCLEN_count = 0;
            ZeroMemory(this->HCLEN_lengths, sizeof(this->HCLEN_lengths));

            switch_to_state(HCLEN_lengths_state);
            break;

        case State::HCLEN_lengths_state:
            {
                if (this->HCLEN - this->HCLEN_count == 0)
                {
                    // $$$ construct HCLEN alphabet

                    const byte max_bits = 7;

                    byte length_count[max_bits + 1] = { 0 };

                    for (byte i = 0; i < _countof(this->HCLEN_lengths); i++)
                        length_count[this->HCLEN_lengths[i]]++;

                    length_count[0] = 0;

                    byte next_code[max_bits + 1] = { 0 };
                    byte code = 0;

                    for (byte bits = 1; bits <= max_bits; bits++)
                    {
                        code = (code + length_count[bits - 1]) << 1;
                        next_code[bits] = code;
                    }

                    byte codes[_countof(this->HCLEN_lengths)] = { 0 };

                    for (byte i = 0; i < _countof(this->HCLEN_lengths); i++)
                    {
                        byte length = this->HCLEN_lengths[i];
                        if (length == 0)
                            continue;

                        codes[i] = next_code[length];
                        next_code[length]++;
                    }

                    switch_to_state(HLIT_lengths_state);
                    break;
                }

                result = read_next(event, &b, 3);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                byte index = Deflate::HCLEN_index[this->HCLEN_count];
                this->HCLEN_lengths[index] = b;
                this->HCLEN_count++;
            }
            break;

        case State::HLIT_lengths_state:
            if (this->HLIT == 0)
            {
                switch_to_state(HDIST_lengths_state);
                break;
            }

            switch_to_state(HLIT_lengths_failed);

            // $$$ decode with HCLEN alphabet
            this->HLIT--;
            break;

        case State::HDIST_lengths_state:
            if (this->HDIST == 0)
            {
                switch_to_state(compressed_data_state);
                break;
            }

            switch_to_state(HLIT_lengths_failed);

            // $$$ decode with HCLEN alphabet
            this->HLIT--;
            break;

        case State::compressed_data_state:
            // $$$ decode with data alphabet
            switch_to_state(compressed_data_failed);
            break;

        default:
            throw FatalError("ServerHelloFrame::handle_event unexpected state");
        }

        return EventResult::event_result_continue;
    }
}