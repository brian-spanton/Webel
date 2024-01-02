// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.RecordFrame.h"

namespace Tls
{
    using namespace Basic;

    RecordFrame::RecordFrame(Record* record) :
        record(record),
        type_frame(&this->record->type), // initialization is in order of declaration in class def
        version_frame(&this->record->version), // initialization is in order of declaration in class def
        length_frame(&this->record->length) // initialization is in order of declaration in class def
    {
    }

    void RecordFrame::reset()
    {
        __super::reset();
        this->type_frame.reset();
        this->version_frame.reset();
        this->length_frame.reset();
    }

    ProcessResult RecordFrame::consider_event(IEvent* event)
    {
        ProcessResult result;

        switch (get_state())
        {
        case State::type_frame_pending_state:
            result = process_event_change_state_on_fail(&this->type_frame, event, State::type_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(State::version_frame_pending_state);
            break;

        case State::version_frame_pending_state:
            result = process_event_change_state_on_fail(&this->version_frame, event, State::version_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(State::length_frame_pending_state);
            break;

        case State::length_frame_pending_state:
            {
                result = process_event_change_state_on_fail(&this->length_frame, event, State::length_frame_failed);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                this->record->fragment = std::make_shared<ByteString>();
                this->record->fragment->resize(this->record->length);
                this->fragment_frame.reset(this->record->fragment->address(), this->record->fragment->size());
                switch_to_state(State::fragment_frame_pending_state);
            }
            break;

        case State::fragment_frame_pending_state:
            result = process_event_change_state_on_fail(&this->fragment_frame, event, State::fragment_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("Tls::RecordFrame::handle_event unexpected state");
        }

        return ProcessResult::process_result_ready;
    }
}