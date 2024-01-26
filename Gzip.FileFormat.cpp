// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Gzip.FileFormat.h"

namespace Gzip
{
    FileFormat::FileFormat(std::shared_ptr<IStream<byte> > first_output) :
        first_output(first_output)
    {
    }

    ProcessResult FileFormat::process_event(IEvent* event)
    {
        // RFC1952 https://www.rfc-editor.org/rfc/rfc1952

        ProcessResult result;

        switch (get_state())
        {
        case State::new_member_state:
            this->member = std::make_shared<MemberHeader>();
            this->member->output = this->first_output;
            this->members.push_back(this->member);

            this->member_frame = std::make_shared<MemberFrame>(this->member.get());

            switch_to_state(member_frame_state);
            break;

        case State::member_frame_state:
            result = process_event_change_state_on_fail(this->member_frame.get(), event, State::member_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(State::next_member_state);
            break;

        case State::next_member_state:
            if (event->get_type() == EventType::element_stream_ending_event)
            {
                switch_to_state(done_state);
                return process_result_blocked;
            }
            else
            {
                byte b;
                auto result = Event::ReadNext(event, &b);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                // $$ NYI multi-member gzip files
                switch_to_state(State::next_member_failed);
            }
            break;

        default:
            throw FatalError("Gzip", "FileFormat", "process_event", "unhandled state", this->get_state());
        }

        return ProcessResult::process_result_ready;
    }
}