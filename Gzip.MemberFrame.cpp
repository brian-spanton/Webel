// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Gzip.MemberFrame.h"
#include "Gzip.Deflate.h"

namespace Gzip
{
    MemberFrame::MemberFrame(MemberHeader* member) :
        member(member),
        ID1_frame(&this->member->ID1),
        ID2_frame(&this->member->ID2),
        CM_frame(&this->member->CM),
        FLG_frame(&this->member->FLG),
        MTIME_frame(&this->member->MTIME),
        XFL_frame(&this->member->XFL),
        OS_frame(&this->member->OS),
        CRC32_frame(&this->member->CRC32),
        ISIZE_frame(&this->member->ISIZE)
    {
    }

    EventResult MemberFrame::consider_event(IEvent* event)
    {
        // RFC1952 https://www.rfc-editor.org/rfc/rfc1952

        EventResult result;

        switch (get_state())
        {
        case State::ID1_state:
            result = delegate_event_change_state_on_fail(&this->ID1_frame, event, State::ID1_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            if (this->member->ID1 == 0x1f)
                switch_to_state(State::ID2_state);
            else
                switch_to_state(State::ID1_failed);

            break;

        case State::ID2_state:
            result = delegate_event_change_state_on_fail(&this->ID2_frame, event, State::ID2_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            if (this->member->ID2 == 0x8b)
                switch_to_state(State::CM_state);
            else
                switch_to_state(State::ID2_failed);

            break;

        case State::CM_state:
            result = delegate_event_change_state_on_fail(&this->CM_frame, event, State::CM_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            if (this->member->CM == CompressionMethod::cm_deflate)
            {
                this->compressed_blocks_frame = std::make_shared<Deflate>(this->member->uncompressed);
                switch_to_state(State::FLG_state);
            }
            else
            {
                switch_to_state(State::CM_failed);
            }
            break;

        case State::FLG_state:
            result = delegate_event_change_state_on_fail(&this->FLG_frame, event, State::FLG_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::MTIME_state);
            break;

        case State::MTIME_state:
            result = delegate_event_change_state_on_fail(&this->MTIME_frame, event, State::MTIME_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::XFL_state);
            break;

        case State::XFL_state:
            result = delegate_event_change_state_on_fail(&this->XFL_frame, event, State::XFL_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::OS_state);
            break;

        case State::OS_state:
            result = delegate_event_change_state_on_fail(&this->OS_frame, event, State::OS_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::XLEN_state);
            break;

        case State::XLEN_state:
            if (this->member->FLG.FEXTRA)
            {
                switch_to_state(State::XLEN_failed);
            }
            else
            {
                switch_to_state(State::original_file_name_state);
            }
            break;

        case State::original_file_name_state:
            if (this->member->FLG.FNAME)
            {
                switch_to_state(State::original_file_name_failed);
            }
            else
            {
                switch_to_state(State::file_comment_state);
            }
            break;

        case State::file_comment_state:
            if (this->member->FLG.FCOMMENT)
            {
                switch_to_state(State::file_comment_failed);
            }
            else
            {
                switch_to_state(State::CRC16_state);
            }
            break;

        case State::CRC16_state:
            if (this->member->FLG.FHCRC)
            {
                switch_to_state(State::CRC16_failed);
            }
            else
            {
                switch_to_state(State::compressed_blocks_state);
            }
            break;

        case State::compressed_blocks_state:
            result = delegate_event_change_state_on_fail(this->compressed_blocks_frame.get(), event, State::OS_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::CRC32_state);
            break;

        case State::CRC32_state:
            result = delegate_event_change_state_on_fail(&this->CRC32_frame, event, State::CRC32_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::ISIZE_state);
            break;

        case State::ISIZE_state:
            result = delegate_event_change_state_on_fail(&this->ISIZE_frame, event, State::ISIZE_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("ServerHelloFrame::handle_event unexpected state");
        }

        return EventResult::event_result_continue;
    }
}