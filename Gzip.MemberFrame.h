// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.NumberFrame.h"
#include "Basic.IStream.h"
#include "Gzip.Types.h"
#include "Basic.NullTerminatedAsciiStringFrame.h"

namespace Gzip
{
    using namespace Basic;

    class MemberFrame : public Frame
    {
    private:
        enum State
        {
            ID1_state = Start_State,
            ID2_state,
            CM_state,
            FLG_state,
            MTIME_state,
            XFL_state,
            OS_state,
            XLEN_state,
            SI1_state,
            SI2_state,
            LEN_state,
            data_state,
            original_file_name_state,
            file_comment_state,
            CRC16_state,
            compressed_blocks_state,
            CRC32_state,
            ISIZE_state,

            done_state = Succeeded_State,

            ID1_failed,
            ID2_failed,
            CM_failed,
            FLG_failed,
            MTIME_failed,
            XFL_failed,
            OS_failed,
            XLEN_failed,
            SI1_failed,
            SI2_failed,
            LEN_failed,
            data_failed,
            original_file_name_failed,
            file_comment_failed,
            CRC16_failed,
            compressed_blocks_failed,
            CRC32_failed,
            ISIZE_failed,
        };

        MemberHeader* member;
        NumberFrame<decltype(MemberHeader::ID1)> ID1_frame;
        NumberFrame<decltype(MemberHeader::ID2)> ID2_frame;
        NumberFrame<decltype(MemberHeader::CM)> CM_frame;
        NumberFrame<decltype(MemberHeader::FLG)> FLG_frame;
        NumberFrame<decltype(MemberHeader::MTIME)> MTIME_frame;
        NumberFrame<decltype(MemberHeader::XFL)> XFL_frame;
        NumberFrame<decltype(MemberHeader::OS)> OS_frame;
        std::shared_ptr<IProcess> compressed_blocks_frame;
        NumberFrame<decltype(MemberHeader::CRC32)> CRC32_frame;
        NumberFrame<decltype(MemberHeader::ISIZE)> ISIZE_frame;
        NullTerminatedAsciiStringFrame original_file_name_frame;
        NullTerminatedAsciiStringFrame file_comment_frame;

    public:
        MemberFrame(MemberHeader* member);

        virtual ProcessResult IProcess::process_event(IEvent* event);
    };
}