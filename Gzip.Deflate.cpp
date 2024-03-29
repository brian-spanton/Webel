// Copyright � 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.SplitStream.h"
#include "Service.Globals.h"
#include "Gzip.MemberFrame.h"
#include "Gzip.Deflate.h"

namespace Gzip
{
    uint32 Deflate::masks[] = { 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff, 0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff };
    byte Deflate::HCLEN_index[] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

    ExtraBits Deflate::clen_extra_bits_parameters[] =
    {
        { 2, 3 },
        { 3, 3 },
        { 7, 11 },
    };

    ExtraBits Deflate::lit_extra_bits_parameters[] =
    {
        { 0, 3 },
        { 0, 4 },
        { 0, 5 },
        { 0, 6 },
        { 0, 7 },
        { 0, 8 },
        { 0, 9 },
        { 0, 10 },
        { 1, 11 },
        { 1, 13 },
        { 1, 15 },
        { 1, 17 },
        { 2, 19 },
        { 2, 23 },
        { 2, 27 },
        { 2, 31 },
        { 3, 35 },
        { 3, 43 },
        { 3, 51 },
        { 3, 59 },
        { 4, 67 },
        { 4, 83 },
        { 4, 99 },
        { 4, 115 },
        { 5, 131 },
        { 5, 163 },
        { 5, 195 },
        { 5, 227 },
        { 0, 258 },
    };

    ExtraBits Deflate::dist_extra_bits_parameters[] =
    {
        { 0, 1 },
        { 0, 2 },
        { 0, 3 },
        { 0, 4 },
        { 1, 5 },
        { 1, 7 },
        { 2, 9 },
        { 2, 13 },
        { 3, 17 },
        { 3, 25 },
        { 4, 33 },
        { 4, 49 },
        { 5, 65 },
        { 5, 97 },
        { 6, 129 },
        { 6, 193 },
        { 7, 257 },
        { 7, 385 },
        { 8, 513 },
        { 8, 769 },
        { 9, 1025 },
        { 9, 1537 },
        { 10, 2049 },
        { 10, 3073 },
        { 11, 4097 },
        { 11, 6145 },
        { 12, 8193 },
        { 12, 12289 },
        { 13, 16385 },
        { 13, 24577 },
    };

    std::shared_ptr<HuffmanAlphabet<uint16> > Deflate::HLIT_fixed;
    std::shared_ptr<HuffmanAlphabet<byte> > Deflate::HDIST_fixed;
    size_t Deflate::max_look_back = 0x8000;

    Deflate::Deflate(std::shared_ptr<IStream<byte> > output_stream) :
        LEN_frame(&this->LEN),
        NLEN_frame(&this->NLEN)
    {
        if (!Deflate::HLIT_fixed)
        {
            std::vector<byte> lit_code_lengths;
            lit_code_lengths.insert(lit_code_lengths.end(), 143 - 000 + 1, 8);
            lit_code_lengths.insert(lit_code_lengths.end(), 255 - 144 + 1, 9);
            lit_code_lengths.insert(lit_code_lengths.end(), 279 - 256 + 1, 7);
            lit_code_lengths.insert(lit_code_lengths.end(), 287 - 280 + 1, 8);
            HuffmanAlphabet<uint16>::make_alphabet((uint16)lit_code_lengths.size(), lit_code_lengths, &Deflate::HLIT_fixed);

            std::vector<byte> dist_code_lengths;
            dist_code_lengths.insert(dist_code_lengths.end(), 32, 5);
            HuffmanAlphabet<byte>::make_alphabet((uint16)dist_code_lengths.size(), dist_code_lengths, &Deflate::HDIST_fixed);
        }

        auto splitter = std::make_shared<SplitStream<byte> >();

        this->look_back = std::make_shared<ByteString>();
        this->look_back->reserve(0x10000);
        splitter->outputs.push_back(this->look_back);

        if (output_stream)
            splitter->outputs.push_back(output_stream);

        this->output_stream = splitter;
    }

    ProcessResult Deflate::read_next(IEvent* event, uint16* output, byte count)
    {
        uint32 dw = 0;

        if (count > 16)
            throw FatalError("Gzip", "Deflate", "read_next", "count > 16");

        while (this->buffered_bits_length < count)
        {
            byte b;
            ProcessResult result = Event::ReadNext(event, &b);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            dw = b;
            dw = (dw << this->buffered_bits_length);
            this->buffered_bits |= dw;
            this->buffered_bits_length += 8;
        }

#pragma warning (suppress: 6385)
        dw = this->buffered_bits & Deflate::masks[count];
        (*output) = (uint16)dw;

        this->buffered_bits = this->buffered_bits >> count;
        this->buffered_bits_length -= count;

        if (this->buffered_bits_length > 7)
            throw FatalError("Gzip", "Deflate", "read_next", "this->buffered_bits_length > 7");

        return ProcessResult::process_result_ready;
    }

    ProcessResult Deflate::read_next(IEvent* event, byte* output, byte count)
    {
        uint16 w;
        ProcessResult result = read_next(event, &w, count);
        if (result == ProcessResult::process_result_blocked)
            return ProcessResult::process_result_blocked;

        (*output) = (byte)w;
        return ProcessResult::process_result_ready;
    }

    ProcessResult Deflate::process_event(IEvent* event)
    {
        // RFC1951 https://www.rfc-editor.org/rfc/rfc1951

        ProcessResult result;
        byte b;

        switch (get_state())
        {
        case State::next_block_state:
            {
                this->block_counter++;

                auto size = this->look_back->size();
                if (size > Deflate::max_look_back)
                    this->look_back->erase(0, size - Deflate::max_look_back);

                if (this->BFINAL)
                    switch_to_state(State::done_state);
                else
                    switch_to_state(State::BFINAL_state);
            }
            break;

        case State::BFINAL_state:
            result = read_next(event, &b, 1);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            this->BFINAL = (bool)b;
            switch_to_state(BTYPE_state);
            break;

        case State::BTYPE_state:
            {
                result = read_next(event, &b, 2);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                BlockType BTYPE = (BlockType)b;
                switch (BTYPE)
                {
                case BlockType::no_compression:
                    this->buffered_bits = 0;
                    this->buffered_bits_length = 0;

                    switch_to_state(State::LEN_state);
                    break;

                case BlockType::fixed:
                    {
                        this->HLIT_root = Deflate::HLIT_fixed;
                        this->HLIT_current = this->HLIT_root;

                        this->HDIST_root = Deflate::HDIST_fixed;
                        this->HDIST_current = this->HDIST_root;

                        switch_to_state(State::length_code_state);
                    }
                    break;

                case BlockType::dynamic:
                    switch_to_state(State::HLIT_state);
                    break;

                default:
                    switch_to_state(State::BTYPE_failed);
                    break;
                }
            }
            break;

        case State::LEN_state:
            result = process_event_change_state_on_fail(&this->LEN_frame, event, State::LEN_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(State::NLEN_state);
            break;

        case State::NLEN_state:
            {
                result = process_event_change_state_on_fail(&this->NLEN_frame, event, State::NLEN_failed);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                uint16 ones_complement = this->LEN ^ 0xffff;
                if (this->NLEN != ones_complement)
                {
                    switch_to_state(NLEN_failed);
                    return ProcessResult::process_result_blocked;
                }

                if (this->LEN == 0)
                {
                    switch_to_state(State::next_block_state);
                }
                else
                {
                    this->uncompressed_data_frame.reset(this->output_stream, this->LEN);
                    switch_to_state(State::uncompressed_data_state);
                }
            }
            break;

        case State::uncompressed_data_state:
            result = process_event_change_state_on_fail(&this->uncompressed_data_frame, event, State::uncompressed_data_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(State::next_block_state);
            break;

        case State::HLIT_state:
            result = read_next(event, &b, 5);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            this->HLIT = (uint16)b + 257;
            switch_to_state(HDIST_state);
            break;

        case State::HDIST_state:
            result = read_next(event, &b, 5);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            this->HDIST = b + 1;
            switch_to_state(HCLEN_state);
            break;

        case State::HCLEN_state:
            result = read_next(event, &b, 4);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            this->HCLEN = b + 4;
            this->HCLEN_count = 0;
            this->dynamic_code_lengths.insert(this->dynamic_code_lengths.end(), _countof(Deflate::HCLEN_index), 0);

            switch_to_state(HCLEN_lengths_state);
            break;

        case State::HCLEN_lengths_state:
            {
                if (this->HCLEN == this->HCLEN_count)
                {
                    HuffmanAlphabet<byte>::make_alphabet((uint16)this->dynamic_code_lengths.size(), this->dynamic_code_lengths, &this->HCLEN_root);
                    this->HCLEN_current = this->HCLEN_root;
                    this->dynamic_code_lengths.clear();

                    switch_to_state(State::lengths_state);
                    break;
                }

                result = read_next(event, &b, 3);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                byte index = Deflate::HCLEN_index[this->HCLEN_count];
                this->dynamic_code_lengths[index] = b;
                this->HCLEN_count++;
            }
            break;

        case State::lengths_state:
            {
                size_t expected = this->HLIT + this->HDIST;

                if (this->dynamic_code_lengths.size() > expected)
                {
                    Basic::LogDebug("Gzip", "Deflate", "process_event", "this->dynamic_code_lengths.size() > expected (bad dynamic lengths)");
                    switch_to_state(State::lengths_failed);
                    break;
                }

                if (this->dynamic_code_lengths.size() == expected)
                {
                    HuffmanAlphabet<uint16>::make_alphabet(this->HLIT, this->dynamic_code_lengths, &this->HLIT_root);
                    this->HLIT_current = this->HLIT_root;
                    this->dynamic_code_lengths.erase(this->dynamic_code_lengths.begin(), this->dynamic_code_lengths.begin() + this->HLIT);

                    if (this->HDIST == 1)
                    {
                        if (this->dynamic_code_lengths.back() == 0)
                        {
                            // RFC1951: One distance code of zero bits means that there are no
                            // distance codes used at all (the data is all literals).
                            this->HDIST_root.reset();
                        }
                        else if (this->dynamic_code_lengths.back() == 1)
                        {
                            // RFC1951: If only one distance code is used, it is encoded using one bit, not zero bits; in
                            // this case there is a single code length of one, with one unused code.
                            // 
                            // HuffmanAlphabet<byte>::make_alphabet will fail for this special case, so construct by hand
                            this->HDIST_root = std::make_shared<HuffmanAlphabet<byte> >();
                            this->HDIST_root->children[0] = std::make_shared<HuffmanAlphabet<byte> >();
                            this->HDIST_root->children[0]->symbol = 0;
                            this->HDIST_root->children[0]->code = 0;
                            this->HDIST_root->children[0]->length = 1;
                        }
                    }
                    else
                    {
                        HuffmanAlphabet<byte>::make_alphabet(this->HDIST, this->dynamic_code_lengths, &this->HDIST_root);
                    }

                    this->HDIST_current = this->HDIST_root;
                    this->dynamic_code_lengths.clear();

                    switch_to_state(State::length_code_state);
                    break;
                }

                result = read_next(event, &b, 1);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                this->HCLEN_current = this->HCLEN_current->children[b];
                if (!this->HCLEN_current)
                {
                    Basic::LogDebug("Gzip", "Deflate", "process_event", "!this->HCLEN_current (bad code lengths (HCLEN) huffman tree)");
                    switch_to_state(State::lengths_failed);
                    break;
                }

                if (this->HCLEN_current->is_leaf())
                {
                    auto symbol = this->HCLEN_current->symbol;
                    if (symbol < 16)
                    {
                        this->dynamic_code_lengths.push_back(symbol);
                    }
                    else
                    {
                        this->symbol_with_extra_bits = symbol;
                        auto extra_bits_index = symbol - 16;
                        this->extra_bits_parameters = Deflate::clen_extra_bits_parameters + extra_bits_index;
                        this->state_after_extra_bits = State::lengths_after_extra_bits_state;
                        this->extra_bits = 0;
                        switch_to_state(State::extra_bits_state);
                    }

                    this->HCLEN_current = this->HCLEN_root;
                }
            }
            break;

        case State::extra_bits_state:
            {
                uint16 w;
                ProcessResult result = read_next(event, &w, this->extra_bits_parameters->length);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                this->extra_bits = w + this->extra_bits_parameters->base;
                switch_to_state(this->state_after_extra_bits);
            }
            break;

        case State::lengths_after_extra_bits_state:
            {
                byte b;
                auto count = (size_t)this->extra_bits;

                switch (this->symbol_with_extra_bits)
                {
                case 16:
                    b = this->dynamic_code_lengths.back();
                    this->dynamic_code_lengths.insert(this->dynamic_code_lengths.end(), count, b);
                    break;

                case 17:
                    this->dynamic_code_lengths.insert(this->dynamic_code_lengths.end(), count, 0);
                    break;

                case 18:
                    this->dynamic_code_lengths.insert(this->dynamic_code_lengths.end(), count, 0);
                    break;
                }

                switch_to_state(State::lengths_state);
            }
            break;

        case State::length_code_state:
            result = read_next(event, &b, 1);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            this->HLIT_current = this->HLIT_current->children[b];
            if (!this->HLIT_current)
            {
                Basic::LogDebug("Gzip", "Deflate", "process_event", "{ !this->HLIT_current } bad literal/length (HLIT) huffman tree");
                switch_to_state(State::length_code_failed);
                break;
            }

            if (this->HLIT_current->is_leaf())
            {
                auto symbol = this->HLIT_current->symbol;
                this->HLIT_current = this->HLIT_root;

                if (symbol < 256)
                {
                    this->output_stream->write_element((byte)symbol);
                }
                else if (symbol == 256)
                {
                    this->HCLEN_count = 0;

                    this->LEN_frame.reset();
                    this->NLEN_frame.reset();

                    this->HCLEN_root.reset();
                    this->HCLEN_current.reset();
                    this->HLIT_root.reset();
                    this->HLIT_current.reset();
                    this->HDIST_root.reset();
                    this->HDIST_current.reset();

                    switch_to_state(State::next_block_state);
                }
                else
                {
                    this->symbol_with_extra_bits = symbol;
                    auto extra_bits_index = symbol - 257;
                    this->extra_bits_parameters = Deflate::lit_extra_bits_parameters + extra_bits_index;
                    this->state_after_extra_bits = State::after_extra_length_bits_state;
                    this->extra_bits = 0;
                    switch_to_state(State::extra_bits_state);
                }
            }
            break;

        case State::after_extra_length_bits_state:
            this->length = this->extra_bits;
            switch_to_state(State::distance_code_state);
            break;

        case State::distance_code_state:
            result = read_next(event, &b, 1);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            this->HDIST_current = this->HDIST_current->children[b];
            if (!this->HDIST_current)
            {
                Basic::LogDebug("Gzip", "Deflate", "process_event", "!this->HDIST_current (bad distance (HDIST) huffman tree)");
                switch_to_state(State::distance_code_failed);
                break;
            }

            if (this->HDIST_current->is_leaf())
            {
                auto symbol = this->HDIST_current->symbol;
                this->HDIST_current = this->HDIST_root;

                this->extra_bits_parameters = Deflate::dist_extra_bits_parameters + symbol;
                this->state_after_extra_bits = State::after_extra_distance_bits_state;
                this->extra_bits = 0;

                switch_to_state(State::extra_bits_state);
            }
            break;

        case State::after_extra_distance_bits_state:
            {
                uint32 distance = this->extra_bits;
                uint32 size = this->look_back->size();

                if (distance > size)
                {
                    Basic::LogDebug("Gzip", "Deflate", "process_event", "distance > size");
                    switch_to_state(State::after_extra_distance_bits_failed);
                    break;
                }

                uint32 first = size - distance;

                for (uint32 index = first; index < first + this->length; index++)
                {
                    auto value = this->look_back->at(index);
                    this->output_stream->write_element(value);
                }

                switch_to_state(State::length_code_state);
            }
            break;

        default:
            throw FatalError("Gzip", "Deflate", "process_event", "unhandled state", this->get_state());
        }

        return ProcessResult::process_result_ready;
    }
}