// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.HeartbeatExtensionFrame.h"

namespace Tls
{
    using namespace Basic;

    HeartbeatExtensionFrame::HeartbeatExtensionFrame(HeartbeatExtension* heartbeat_extension) :
        heartbeat_extension(heartbeat_extension),
        mode_frame(&this->heartbeat_extension->mode) // initialization is in order of declaration in class def
    {
    }

	ConsumeElementsResult HeartbeatExtensionFrame::consume_elements(IElementSource<byte>* element_source)
    {
        switch (this->get_state())
        {
        case State::mode_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->mode_frame, element_source, this, State::mode_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::done_state);
			return ConsumeElementsResult::succeeded;
		}

        default:
            throw FatalError("HeartbeatExtensionFrame::handle_event unexpected state");
        }
    }
}