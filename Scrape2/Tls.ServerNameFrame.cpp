// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.ServerNameFrame.h"
#include "Tls.Globals.h"

namespace Tls
{
    using namespace Basic;

    ServerNameFrame::ServerNameFrame(ServerName* serverName) :
        serverName(serverName),
        type_frame(&this->serverName->name_type), // initialization is in order of declaration in class def
        name_frame(&this->serverName->name) // initialization is in order of declaration in class def
    {
    }

	ConsumeElementsResult ServerNameFrame::consume_elements(IElementSource<byte>* element_source)
    {
        switch (this->get_state())
        {
		case State::type_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->type_frame, element_source, this, State::type_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::name_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::name_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->name_frame, element_source, this, State::name_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::done_state);
			return ConsumeElementsResult::succeeded;
		}

        default:
            throw FatalError("Tls::ServerNameFrame::handle_event unexpected state");
        }
    }
}