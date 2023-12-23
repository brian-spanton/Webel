// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.AlertFrame.h"

namespace Tls
{
    using namespace Basic;

    AlertFrame::AlertFrame(Alert* alert) :
        alert(alert),
        level_frame(&this->alert->level), // initialization is in order of declaration in class def
        description_frame(&this->alert->description) // initialization is in order of declaration in class def
    {
    }

	ConsumeElementsResult AlertFrame::consume_elements(IElementSource<byte>* element_source)
    {
        switch (get_state())
        {
		case State::level_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->level_frame, element_source, this, State::level_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::description_frame_pending_state);
			return ConsumeElementsResult::in_progress;
		}

		case State::description_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->description_frame, element_source, this, State::description_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::done_state);
			return ConsumeElementsResult::succeeded;
		}

        default:
            throw FatalError("Tls::AlertFrame::handle_event unexpected state");
        }
    }
}
