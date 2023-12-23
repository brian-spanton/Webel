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

	ConsumeElementsResult ExtensionHeaderFrame::consume_elements(IElementSource<byte>* element_source)
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
            throw FatalError("ExtensionHeaderFrame::handle_event unexpected state");
        }
    }
}