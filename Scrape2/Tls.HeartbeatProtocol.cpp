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
        heartbeat_message_frame(&this->heartbeat_message) // initialization is in order of declaration in class def
    {
    }

    void HeartbeatProtocol::SetPlaintextLength(uint16 plaintext_length)
    {
        switch (get_state())
        {
        case State::start_state:
            this->heartbeat_message_frame.set_plaintext_length(plaintext_length);
            switch_to_state(State::heartbeat_message_frame_pending_state);
            return;

        default:
            throw FatalError("Tls::HeartbeatProtocol::SetPlaintextLength unexpected state");
        }
    }

	void HeartbeatProtocol::transport_connected()
	{
	}

	void HeartbeatProtocol::transport_disconnected()
	{
	}

	void HeartbeatProtocol::transport_received(const byte* elements, uint32 count)
	{
		ElementSource<byte> element_source(elements, count);

		bool success = element_source.deliver_elements(this);
		if (!success)
		{
			this->session->DisconnectApplication();
			this->session->CloseTransport();
		}
	}

	ConsumeElementsResult HeartbeatProtocol::consume_elements(IElementSource<byte>* element_source)
    {
        switch (this->get_state())
        {
        case State::heartbeat_message_frame_pending_state:
        {
			ConsumeElementsResult result = Basic::consume_elements(&this->heartbeat_message_frame, element_source, this, State::heartbeat_message_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

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

                ByteString heartbeat_bytes;
                serialize<HeartbeatMessage>()(&this->heartbeat_message, &heartbeat_bytes);
                this->session->write_record_elements(ContentType::heartbeat_content_type, heartbeat_bytes.address(), heartbeat_bytes.size());

				return ConsumeElementsResult::in_progress;
			}

            case HeartbeatMessageType::heartbeat_response:
                switch_to_state(State::start_state);
				return ConsumeElementsResult::in_progress;

            default:
                switch_to_state(State::unexpected_type_error);
				return ConsumeElementsResult::failed;
			}
		}

        default:
            throw FatalError("Tls::HeartbeatProtocol::handle_event unexpected state");
        }
    }
}
