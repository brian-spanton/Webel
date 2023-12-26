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
        case ID1_state:
            result = delegate_event_change_state_on_fail(&this->ID1_frame, event, ID1_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::ID2_state);
            break;

        case ID2_state:
            result = delegate_event_change_state_on_fail(&this->ID2_frame, event, ID2_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::CM_state);
            break;

        case CM_state:
            result = delegate_event_change_state_on_fail(&this->CM_frame, event, CM_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::FLG_state);
            break;

        case FLG_state:
            result = delegate_event_change_state_on_fail(&this->FLG_frame, event, FLG_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::MTIME_state);
            break;

        case MTIME_state:
            result = delegate_event_change_state_on_fail(&this->MTIME_frame, event, MTIME_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::XFL_state);
            break;

        case XFL_state:
            result = delegate_event_change_state_on_fail(&this->XFL_frame, event, XFL_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::OS_state);
            break;

        case OS_state:
            result = delegate_event_change_state_on_fail(&this->OS_frame, event, OS_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            if (this->member->CM == CompressionMethod::cm_deflate)
            {

                this->compressed_blocks_frame = std::make_shared<Deflate>(this->member->uncompressed);
                switch_to_state(State::compressed_blocks_state);
            }
            else
            {
                switch_to_state(State::OS_failed);
            }
            break;

        case compressed_blocks_state:
            result = delegate_event_change_state_on_fail(this->compressed_blocks_frame.get(), event, OS_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::CRC32_state);
            break;

        case CRC32_state:
            result = delegate_event_change_state_on_fail(&this->CRC32_frame, event, CRC32_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::ISIZE_state);
            break;

        case ISIZE_state:
            result = delegate_event_change_state_on_fail(&this->ISIZE_frame, event, ISIZE_failed);
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