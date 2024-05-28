// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.DigitallySignedFrame.h"

namespace Tls
{
    using namespace Basic;

    DigitallySignedFrame::DigitallySignedFrame(DigitallySigned* digitally_signed) :
        digitally_signed(digitally_signed),
        algorithm_frame(&this->digitally_signed->algorithm), // initialization is in order of declaration in class def
        signature_frame(&this->digitally_signed->signature) // initialization is in order of declaration in class def
    {
    }

    void DigitallySignedFrame::reset()
    {
        __super::reset();
    }

    ProcessResult DigitallySignedFrame::process_event(IEvent* event)
    {
        ProcessResult result;

        switch (get_state())
        {
        case State::algorithm_frame_pending_state:
            result = process_event_change_state_on_fail(&this->algorithm_frame, event, State::algorithm_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(State::signature_frame_pending_state);
            break;

        case State::signature_frame_pending_state:
            result = process_event_change_state_on_fail(&this->signature_frame, event, State::signature_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("Tls", "DigitallySignedFrame", "process_event", "unhandled state", this->get_state());
        }

        return ProcessResult::process_result_ready;
    }
}