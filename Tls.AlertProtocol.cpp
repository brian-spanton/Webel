// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.AlertProtocol.h"
#include "Tls.RecordLayer.h"
#include "Tls.ServerNameFrame.h"
#include "Tls.SignatureAndHashAlgorithmFrame.h"

namespace Tls
{
    using namespace Basic;

    AlertProtocol::AlertProtocol(RecordLayer* session) :
        session(session)
    {
    }

    ProcessResult AlertProtocol::process_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::start_state:
            this->alert_frame = std::make_shared<AlertFrame>(&this->alert);
            switch_to_state(State::alert_frame_pending_state);
            break;

        case State::alert_frame_pending_state:
            {
                ProcessResult result = process_event_change_state_on_fail(this->alert_frame.get(), event, State::alert_frame_failed);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                switch (this->alert.description)
                {
                case AlertDescription::close_notify:
                    this->session->DisconnectApplication();
                    this->session->CloseTransport();
                    switch_to_state(State::start_state);
                    break;

                default:
                    // $$ process other alerts
                    switch_to_state(State::start_state);
                    break;
                }
            }
            break;

        default:
            throw FatalError("Tls", "AlertProtocol", "process_event", "unhandled state", this->get_state());
        }

        return ProcessResult::process_result_ready;
    }
}
