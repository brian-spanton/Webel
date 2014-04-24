// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.HeartbeatProtocol.h"
#include "Tls.RecordLayer.h"

namespace Tls
{
    using namespace Basic;

    void HeartbeatProtocol::Initialize(RecordLayer* session)
    {
        __super::Initialize();
        this->session = session;
    }

    void HeartbeatProtocol::SetPlaintextLength(uint16 plaintext_length)
    {
        switch (frame_state())
        {
        case State::start_state:
            this->heartbeat_message_frame.Initialize(&this->heartbeat_message, plaintext_length);
            switch_to_state(State::heartbeat_message_frame_pending_state);
            break;

        default:
            throw new Exception("Tls::HeartbeatProtocol::SetPlaintextLength unexpected state");
        }
    }

    void HeartbeatProtocol::Process(IEvent* event, bool* yield)
    {
        switch (frame_state())
        {
        case State::heartbeat_message_frame_pending_state:
            if (this->heartbeat_message_frame.Pending())
            {
                this->heartbeat_message_frame.Process(event, yield);
            }

            if (this->heartbeat_message_frame.Failed())
            {
                switch_to_state(State::heartbeat_message_frame_failed);
            }
            else if (this->heartbeat_message_frame.Succeeded())
            {
                switch (this->heartbeat_message.type)
                {
                case HeartbeatMessageType::heartbeat_request:
                    {
                        switch_to_state(State::start_state);

                        this->heartbeat_message.type = HeartbeatMessageType::heartbeat_response;

                        this->heartbeat_message.padding.resize(16);
                        NTSTATUS error = BCryptGenRandom(0, &this->heartbeat_message.padding[0], this->heartbeat_message.padding.size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
                        if (error != 0)
                            throw new Exception("Tls::ClientHandshake::Process BCryptGenRandom failed", error);

                        this->heartbeat_message_frame.SerializeTo(this->session);
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
            throw new Exception("Tls::HeartbeatProtocol::Process unexpected state");
        }
    }
}
