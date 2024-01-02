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

    ProcessResult PreMasterSecretFrame::process_event(IEvent* event)
    {
        ProcessResult result;

        switch (get_state())
        {
        case State::version_frame_pending_state:
            result = process_event_change_state_on_fail(&this->version_frame, event, State::version_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(State::random_frame_pending_state);
            break;

        case State::random_frame_pending_state:
            result = process_event_change_state_on_fail(&this->random_frame, event, State::random_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("PreMasterSecretFrame::handle_event unexpected state");
        }

        return ProcessResult::process_result_ready;
    }
}