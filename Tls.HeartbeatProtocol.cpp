// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.HeartbeatProtocol.h"
#include "Tls.RecordLayer.h"
#include "Tls.ServerNameFrame.h"
#include "Tls.SignatureAndHashAlgorithmFrame.h"

namespace Tls
{
    using namespace Basic;

    HeartbeatProtocol::HeartbeatProtocol(RecordLayer* session) :
        session(session),
        heartbeat_message_frame(&this->heartbeat_message) // order of declaration is important
    {
    }

    void HeartbeatProtocol::SetPlaintextLength(uint16 plaintext_length)
    {
        switch (get_state())
        {
        case State::start_state:
            this->heartbeat_message_frame.set_plaintext_length(plaintext_length);
            switch_to_state(State::heartbeat_message_frame_pending_state);
            break;

        default:
            throw FatalError("Tls::HeartbeatProtocol::SetPlaintextLength unexpected state");
        }
    }

    void HeartbeatProtocol::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::heartbeat_message_frame_pending_state:
            {
                delegate_event_change_state_on_fail(&this->heartbeat_message_frame, event, State::heartbeat_message_frame_failed);

                switch (this->heartbeat_message.type)
                {
                case HeartbeatMessageType::heartbeat_request:
                    {
                        switch_to_state(State::start_state);

                        this->heartbeat_message.type = HeartbeatMessageType::heartbeat_response;

                        this->heartbeat_message.padding.resize(16);
                        NTSTATUS error = BCryptGenRandom(0, this->heartbeat_message.padding.address(), this->heartbeat_message.padding.size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
                        if (error != 0)
                            throw FatalError("Tls::ClientHandshake::handle_event BCryptGenRandom failed", error);

                        serialize<HeartbeatMessage>()(&this->heartbeat_message, this->session);
                    }
                    break;

                case HeartbeatMessageType::heartbeat_response:
                    switch_to_state(State::start_state);
                    break;

                default:
                    switch_to_state(State::unexpected_type_error);
                    break;
                }
            }
            break;

        default:
            throw FatalError("Tls::HeartbeatProtocol::handle_event unexpected state");
        }
    }
}
