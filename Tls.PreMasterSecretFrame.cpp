// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.PreMasterSecretFrame.h"
#include "Tls.ResponderIDListFrame.h"

namespace Tls
{
	using namespace Basic;

	void PreMasterSecretFrame::Initialize(PreMasterSecret* pre_master_secret)
	{
		__super::Initialize();
		this->pre_master_secret = pre_master_secret;
		this->version_frame.Initialize(&this->pre_master_secret->client_version);
		this->random_frame.Initialize((byte*)&this->pre_master_secret->random, sizeof(this->pre_master_secret->random));
	}

	void PreMasterSecretFrame::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::version_frame_pending_state:
			if (this->version_frame.Pending())
			{
				this->version_frame.Process(event, yield);
			}

			if (this->version_frame.Failed())
			{
				switch_to_state(State::version_frame_failed);
			}
			else if (this->version_frame.Succeeded())
			{
				switch_to_state(State::random_frame_pending_state);
			}
			break;

		case State::random_frame_pending_state:
			if (this->random_frame.Pending())
			{
				this->random_frame.Process(event, yield);
			}

			if (this->random_frame.Failed())
			{
				switch_to_state(State::random_frame_failed);
			}
			else if (this->random_frame.Succeeded())
			{
				switch_to_state(State::done_state);
			}
			break;

		default:
			throw new Exception("PreMasterSecretFrame::Process unexpected state");
		}
	}

	void PreMasterSecretFrame::SerializeTo(IStream<byte>* stream)
	{
		this->version_frame.SerializeTo(stream);
		this->random_frame.SerializeTo(stream);
	}
}