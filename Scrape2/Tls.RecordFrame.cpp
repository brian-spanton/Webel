// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.RecordFrame.h"

namespace Tls
{
    using namespace Basic;

    RecordFrame::RecordFrame(Record* record) :
        record(record),
        type_frame(&this->record->type), // initialization is in order of declaration in class def
        version_frame(&this->record->version), // initialization is in order of declaration in class def
        length_frame(&this->record->length) // initialization is in order of declaration in class def
    {
    }

    void RecordFrame::reset()
    {
        __super::reset();
        this->type_frame.reset();
        this->version_frame.reset();
        this->length_frame.reset();
    }

	ConsumeElementsResult RecordFrame::consume_elements(IElementSource<byte>* element_source)
    {
        switch (this->get_state())
        {
        case State::type_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->type_frame, element_source, this, State::type_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::version_frame_pending_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::version_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->version_frame, element_source, this, State::version_frame_failed);
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

			this->record->fragment = std::make_shared<ByteString>();
			this->record->fragment->resize(this->record->length);
			this->fragment_frame.reset(this->record->fragment->address(), this->record->fragment->size());

			this->switch_to_state(State::fragment_frame_pending_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::fragment_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->fragment_frame, element_source, this, State::fragment_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::done_state);
			return ConsumeElementsResult::succeeded;
		}

        default:
            throw FatalError("Tls::RecordFrame::handle_event unexpected state");
        }
    }
}