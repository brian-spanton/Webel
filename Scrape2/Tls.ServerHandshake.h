// Copyright © 2013 Brian Spanton

#pragma once

#include "Tls.HandshakeProtocol.h"
#include "Tls.SecurityParameters.h"
#include "Tls.Types.h"
#include "Tls.Globals.h"
#include "Tls.ClientHelloFrame.h"

namespace Tls
{
    using namespace Basic;

    class ServerHandshake : public HandshakeProtocol
    {
    private:
        enum State
        {
            start_state = Start_State,
            expecting_client_hello_state,
            hello_frame_pending_state,
            expecting_client_key_exchange_state,
            pre_master_secret_frame_pending,
            expecting_cipher_change_state,
            expecting_finished_state,
            finished_received_frame_pending_state,
            done_state = Succeeded_State,
            handshake_frame_1_failed,
            hello_frame_failed,
            expecting_client_hello_error,
            client_version_error,
            SelectCipherSuite_failed,
            InitializeCipherSuite_failed,
            BCryptGenRandom_1_failed,
            handshake_frame_2_failed,
            expecting_client_key_exchange_error,
            unexpected_key_exchange_algorithm_1_error,
            pre_master_secret_frame_failed,
            BCryptGenRandom_2_failed,
            handshake_frame_3_failed,
            expecting_finished_error,
            handshake_length_error,
            finished_received_frame_failed,
            finished_received_error,
            unexpected_key_exchange_algorithm_2_error,
            unexpected_heartbeat_mode_error,
			unexpected_state_1_error,
			unexpected_state_2_error,
			unexpected_state_3_error,
		};

        ClientHello clientHello;
        ClientHelloFrame client_hello_frame;
        VectorFrame<EncryptedPreMasterSecret> pre_master_secret_frame;

        virtual void PartitionKeyMaterial(ByteString* key_material);
        bool ProcessClientKeyExchange(KeyExchangeAlgorithm key_exchange_algorithm);

		virtual ConsumeElementsResult consume_elements(IElementSource<byte>* element_source);

    public:
        ServerHandshake(RecordLayer* session);

		void ITransportEventHandler<byte>::transport_connected();
		void ITransportEventHandler<byte>::transport_disconnected();

		virtual void change_cipher_spec_event();
	};
}