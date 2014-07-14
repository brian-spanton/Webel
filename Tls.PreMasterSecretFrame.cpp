// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.PreMasterSecretFrame.h"

namespace Tls
{
    using namespace Basic;

    PreMasterSecretFrame::PreMasterSecretFrame(PreMasterSecret* pre_master_secret) :
        pre_master_secret(pre_master_secret),
        version_frame(&this->pre_master_secret->client_version),
        random_frame((byte*)&this->pre_master_secret->random, sizeof(this->pre_master_secret->random))
    {
    }

    void PreMasterSecretFrame::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::version_frame_pending_state:
            delegate_event_change_state_on_fail(&this->version_frame, event, State::version_frame_failed);
            switch_to_state(State::random_frame_pending_state);
            break;

        case State::random_frame_pending_state:
            delegate_event_change_state_on_fail(&this->random_frame, event, State::random_frame_failed);
            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("PreMasterSecretFrame::handle_event unexpected state");
        }
    }
}