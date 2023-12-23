#include "stdafx.h"
#include "Tls.ServerNameFrame.h"
#include "Tls.NumberFrame.h"
#include "Tls.Globals.h"

namespace Tls
{
	using namespace Basic;

	void ServerNameFrame::Initialize(ServerName* serverName)
	{
		__super::Initialize();
		this->serverName = serverName;
		this->type_frame.Initialize(&this->serverName->name_type);
		this->name_frame.Initialize(&this->serverName->name);
	}

	void ServerNameFrame::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::start_state:
			switch_to_state(State::type_state);
			break;

		case State::type_state:
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
				switch_to_state(State::name_state);
			}
			break;

		case State::name_state:
			if (this->name_frame.Pending())
			{
				this->name_frame.Process(event, yield);
			}
			else if (this->name_frame.Failed())
			{
				switch_to_state(State::name_frame_failed);
			}
			else
			{
				switch_to_state(State::done_state);
			}
			break;

		default:
			throw new Exception("Tls::ServerNameFrame::Process unexpected state");
		}
	}

	void ServerNameFrame::SerializeTo(IStream<byte>* stream)
	{
		this->type_frame.SerializeTo(stream);
		this->name_frame.SerializeTo(stream);
	}
}