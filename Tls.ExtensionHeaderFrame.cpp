// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.ExtensionHeaderFrame.h"

namespace Tls
{
    using namespace Basic;

    ExtensionHeaderFrame::ExtensionHeaderFrame(ExtensionHeader* extension) :
        extension(extension),
        type_frame(&this->extension->type), // initialization is in order of declaration in class def
        length_frame(&this->extension->length) // initialization is in order of declaration in class def
    {
    }

    void ExtensionHeaderFrame::reset()
    {
        __super::reset();
        type_frame.reset();
        length_frame.reset();
    }

    ProcessResult ExtensionHeaderFrame::process_event(IEvent* event)
    {
        ProcessResult result;

        switch (get_state())
        {
        case State::type_frame_pending_state:
            result = process_event_change_state_on_fail(&this->type_frame, event, State::type_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(State::length_frame_pending_state);
            break;

        case State::length_frame_pending_state:
            result = process_event_change_state_on_fail(&this->length_frame, event, State::length_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("Tls", "ExtensionHeaderFrame", "process_event", "unhandled state", this->get_state());
        }

        return ProcessResult::process_result_ready;
    }
}