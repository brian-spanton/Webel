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
            start_state = Start_State,
            BFINAL_state,
            BTYPE_state,
            block_state,
            LEN_state,
            NLEN_state,
            uncompressed_data_state,
            fixed_block_state,
            dynamic_block_state,
            HLIT_state,
            HDIST_state,
            HCLEN_state,
            HCLEN_lengths_state,
            HLIT_lengths_state,
            HDIST_lengths_state,
            compressed_data_state,
            done_state = Succeeded_State,
            BTYPE_failed,
            uncompressed_failed,
            LEN_failed,
            NLEN_failed,
            uncompressed_data_failed,
            fixed_block_failed,
            dynamic_block_failed,
            HLIT_failed,
            HDIST_failed,
            HCLEN_failed,
            HCLEN_lengths_failed,
            HLIT_lengths_failed,
            HDIST_lengths_failed,
            compressed_data_failed,
        };

        enum BlockType : byte
        {
            no_compression = 0,
            fixed = 1,
            dynamic = 2,
            reserved = 3,
        };

        std::shared_ptr<IStream<byte> > uncompressed;

        uint16 bits = 0;
        byte count = 0;

        bool BFINAL = false;
        uint16 LEN;
        uint16 NLEN;
        uint16 HLIT;
        byte HDIST;
        byte HCLEN;

        StreamFrame<byte> uncompressed_frame;
        NumberFrame<decltype(LEN)> LEN_frame;
        NumberFrame<decltype(NLEN)> NLEN_frame;
        byte HCLEN_lengths[19];
        byte HCLEN_count;

        static uint16 masks[8];
        static byte HCLEN_index[19];

        EventResult read_next(IEvent* event, byte* output, byte count);

    public:
        Deflate(std::shared_ptr<IStream<byte> > uncompressed);

        virtual EventResult IProcess::consider_event(IEvent* event);
    };
}