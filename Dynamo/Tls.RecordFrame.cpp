#include "stdafx.h"
#include "Tls.RecordFrame.h"

namespace Tls
{
	using namespace Basic;

	void RecordFrame::Initialize(Record* record)
	{
		__super::Initialize();
		this->record = record;
		this->type_frame.Initialize(&this->record->type);
		this->version_frame.Initialize(&this->record->version);
		this->length_frame.Initialize(&this->record->length);
	}

	void RecordFrame::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::type_frame_pending_state:
			if (this->type_frame.Pending())
			{
				this->type_frame.Process(event, yield);
			}
			else if (this->type_frame.Failed())
			{
				switch_to_state(State::type_frame_failed);
			}
			else
			{
				switch_to_state(State::version_frame_pending_state);
			}
			break;

		case State::version_frame_pending_state:
			if (this->version_frame.Pending())
			{
				this->version_frame.Process(event, yield);
			}
			else if (this->version_frame.Failed())
			{
				switch_to_state(State::version_frame_failed);
			}
			else
			{
				switch_to_state(State::length_frame_pending_state);
			}
			break;

		case State::length_frame_pending_state:
			if (this->length_frame.Pending())
			{
				this->length_frame.Process(event, yield);
			}
			else if (this->length_frame.Failed())
			{
				switch_to_state(State::length_frame_failed);
			}
			else
			{
				this->record->fragment = New<ByteVector>();
				this->record->fragment->resize(this->record->length);
				this->fragment_frame.Initialize(this->record->fragment->FirstElement(), this->record->fragment->size());
				switch_to_state(State::fragment_frame_pending_state);
			}
			break;

		case State::fragment_frame_pending_state:
			if (this->fragment_frame.Pending())
			{
				this->fragment_frame.Process(event, yield);
			}
			else if (this->fragment_frame.Failed())
			{
				switch_to_state(State::fragment_frame_failed);
			}
			else
			{
				switch_to_state(State::done_state);
			}
			break;

		default:
			throw new Exception("Tls::RecordFrame::Process unexpected state");
		}
	}

	void RecordFrame::SerializeTo(IStream<byte>* stream)
	{
		this->type_frame.SerializeTo(stream);
		this->version_frame.SerializeTo(stream);
		this->length_frame.SerializeTo(stream);
		this->record->fragment->SerializeTo(stream);
	}
}