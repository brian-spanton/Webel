// Copyright © 2013 Brian Spanton

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

	ConsumeElementsResult HandshakeFrame::consume_elements(IElementSource<byte>* element_source)
    {
        switch (this->get_state())
        {
        case State::type_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->type_frame, element_source, this, State::type_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::length_frame_pending_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::length_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->length_frame, element_source, this, State::length_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::done_state);
			return ConsumeElementsResult::succeeded;
		}

        default:
            throw FatalError("Tls::HandshakeFrame::handle_event unexpected state");
        }
    }
}
