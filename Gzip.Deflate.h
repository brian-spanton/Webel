// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.NumberFrame.h"
#include "Basic.IStream.h"
#include "Basic.StreamFrame.h"
#include "Gzip.Types.h"

namespace Gzip
{
    using namespace Basic;

    class Deflate : public Frame
    {
    private:
        enum State
        {
            next_block_state = Start_State,
            BFINAL_state,
            BTYPE_state,
            LEN_state,
            NLEN_state,
            uncompressed_data_state,
            fixed_block_state,
            HLIT_state,
            HDIST_state,
            HCLEN_state,
            HCLEN_lengths_state,
            lengths_state,
            lengths_extra_bits_state,
            lengths_after_extra_bits_state,
            length_code_state,
            extra_bits_state,
            after_extra_length_bits_state,
            distance_code_state,
            after_extra_distance_bits_state,
            done_state = Succeeded_State,
            BTYPE_failed,
            uncompressed_failed,
            LEN_failed,
            NLEN_failed,
            uncompressed_data_failed,
            lengths_failed,
            length_code_failed,
            distance_code_failed,
            after_extra_distance_bits_failed,
        };

        enum BlockType : byte
        {
            no_compression = 0,
            fixed = 1,
            dynamic = 2,
            reserved = 3,
        };

        std::shared_ptr<IStream<byte> > output_stream;
        std::shared_ptr<ByteString> look_back;

        uint32 buffered_bits = 0;
        byte buffered_bits_length = 0;

        bool BFINAL = false;
        uint32 block_counter = 0;
        uint16 LEN;
        uint16 NLEN;
        uint16 HLIT;
        byte HDIST;
        byte HCLEN;

        StreamFrame<byte> uncompressed_data_frame;
        NumberFrame<decltype(LEN)> LEN_frame;
        NumberFrame<decltype(NLEN)> NLEN_frame;
        byte HCLEN_count = 0;
        ExtraBits* extra_bits_parameters;
        uint16 extra_bits_count = 0;
        State state_after_extra_bits;

        std::shared_ptr<HuffmanAlphabet<byte> > HCLEN_root;
        std::shared_ptr<HuffmanAlphabet<byte> > HCLEN_current;
        std::shared_ptr<HuffmanAlphabet<uint16> > HLIT_root;
        std::shared_ptr<HuffmanAlphabet<uint16> > HLIT_current;
        std::shared_ptr<HuffmanAlphabet<byte> > HDIST_root;
        std::shared_ptr<HuffmanAlphabet<byte> > HDIST_current;
        std::vector<byte> dynamic_code_lengths;

        uint16 symbol_with_extra_bits;
        uint16 extra_bits = 0;

        uint16 length;

        static size_t max_look_back;
        static uint32 masks[16];
        static byte HCLEN_index[19];
        static ExtraBits clen_extra_bits_parameters[18 - 16 + 1];
        static ExtraBits lit_extra_bits_parameters[285 - 257 + 1];
        static ExtraBits dist_extra_bits_parameters[30];
        static std::shared_ptr<HuffmanAlphabet<uint16> > HLIT_fixed;
        static std::shared_ptr<HuffmanAlphabet<byte> > HDIST_fixed;

        ProcessResult read_next(IEvent* event, uint16* output, byte count);
        ProcessResult read_next(IEvent* event, byte* output, byte count);

    public:
        Deflate(std::shared_ptr<IStream<byte> > output_stream);

        virtual ProcessResult IProcess::process_event(IEvent* event);
    };
}