// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.RandomFrame.h"

namespace Tls
{
    using namespace Basic;

    RandomFrame::RandomFrame(Random* random) :
        random(random),
        time_frame(&this->random->gmt_unix_time), // initialization is in order of declaration in class def
        bytes_frame(this->random->random_bytes, sizeof(this->random->random_bytes)) // initialization is in order of declaration in class def
    {
    }

	ConsumeElementsResult RandomFrame::consume_elements(IElementSource<byte>* element_source)
    {
        switch (this->get_state())
        {
        case State::time_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->time_frame, element_source, this, State::time_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::bytes_frame_pending_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::bytes_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->bytes_frame, element_source, this, State::bytes_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::done_state);
			return ConsumeElementsResult::succeeded;
		}

        default:
            throw FatalError("Tls::RandomFrame unexpected state");
        }
    }
}
