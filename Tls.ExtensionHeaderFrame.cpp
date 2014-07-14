// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.ExtensionHeaderFrame.h"

namespace Tls
{
    using namespace Basic;

    ExtensionHeaderFrame::ExtensionHeaderFrame(ExtensionHeader* extension) :
        extension(extension),
        type_frame(&this->extension->type),
        length_frame(&this->extension->length)
    {
    }

    void ExtensionHeaderFrame::reset()
    {
        __super::reset();
        type_frame.reset();
        length_frame.reset();
    }

    void ExtensionHeaderFrame::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::type_frame_pending_state:
            delegate_event_change_state_on_fail(&this->type_frame, event, State::type_frame_failed);
            switch_to_state(State::length_frame_pending_state);
            break;

        case State::length_frame_pending_state:
            delegate_event_change_state_on_fail(&this->length_frame, event, State::length_frame_failed);
            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("ExtensionHeaderFrame::handle_event unexpected state");
        }
    }
}