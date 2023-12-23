// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.PreMasterSecretFrame.h"

namespace Tls
{
    using namespace Basic;

    PreMasterSecretFrame::PreMasterSecretFrame(PreMasterSecret* pre_master_secret) :
        pre_master_secret(pre_master_secret),
        version_frame(&this->pre_master_secret->client_version), // initialization is in order of declaration in class def
        random_frame((byte*)&this->pre_master_secret->random, sizeof(this->pre_master_secret->random))
    {
    }

	ConsumeElementsResult PreMasterSecretFrame::consume_elements(IElementSource<byte>* element_source)
    {
        switch (this->get_state())
        {
        case State::version_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->version_frame, element_source, this, State::version_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::random_frame_pending_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::random_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->random_frame, element_source, this, State::random_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::done_state);
			return ConsumeElementsResult::succeeded;
		}

        default:
            throw FatalError("PreMasterSecretFrame::handle_event unexpected state");
        }
    }
}