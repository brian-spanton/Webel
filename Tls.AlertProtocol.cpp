// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.AlertProtocol.h"
#include "Tls.RecordLayer.h"

namespace Tls
{
    using namespace Basic;

    void AlertProtocol::Initialize(RecordLayer* session)
    {
        __super::Initialize();
        this->session = session;
    }

    void AlertProtocol::Process(IEvent* event, bool* yield)
    {
        switch (frame_state())
        {
        case State::start_state:
            (*yield) = false;
            this->alert_frame.Initialize(&this->alert);
            switch_to_state(State::alert_frame_pending_state);
            break;

        case State::alert_frame_pending_state:
            if (this->alert_frame.Pending())
            {
                this->alert_frame.Process(event, yield);
            }

            if (this->alert_frame.Failed())
            {
                switch_to_state(State::alert_frame_failed);
            }
            else if (this->alert_frame.Succeeded())
            {
                switch (this->alert.description)
                {
                case AlertDescription::close_notify:
                    switch_to_state(State::alert_frame_peer_close_notify_state);
                    break;

                default:
                    switch_to_state(State::start_state);
                    break;
                }
            }
            break;

        default:
            throw new Exception("Tls::AlertProtocol::Process unexpected state");
        }
    }
}
