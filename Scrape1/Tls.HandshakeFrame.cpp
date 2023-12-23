// Copyright � 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.HandshakeFrame.h"

namespace Tls
{
    using namespace Basic;

    HandshakeFrame::HandshakeFrame(Handshake* handshake) :
        handshake(handshake),
        type_frame(&this->handshake->msg_type), // initialization is in order of declaration in class def
        length_frame(&this->handshake->length) // initialization is in order of declaration in class def
    {
    }

    void HandshakeFrame::reset()
    {
        __super::reset();
        this->type_frame.reset();
        this->length_frame.reset();
    }

    void HandshakeFrame::consider_event(IEvent* event)
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
            throw FatalError("Tls::HandshakeFrame::handle_event unexpected state");
        }
    }
}
