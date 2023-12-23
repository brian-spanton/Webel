#pragma once

#include "Basic.IProcess.h"
#include "Tls.HandshakeProtocol.h"
#include "Tls.SecurityParameters.h"
#include "Tls.Types.h"
#include "Tls.ServerHelloFrame.h"
#include "Tls.CertificatesFrame.h"

namespace Tls
{
	using namespace Basic;

	class ClientHandshake : public HandshakeProtocol
	{
	private:
		enum State
		{
			start_state = Start_State,
			expecting_server_hello_state,
			server_hello_frame_pending_state,
			expecting_certificate_state,
			certificates_frame_pending_state,
			expecting_server_hello_done_state,
			expecting_cipher_change_state,
			expecting_finished_state,
			finished_received_frame_pending_state,
			done_state = Succeeded_State,
			WriteMessage_1_failed,
			handshake_frame_1_failed,
			expecting_server_hello_error,
			server_hello_frame_failed,
			server_version_error,
			InitializeCipherSuite_failed,
			unexpected_key_exchange_algorithm_1_error,
			handshake_frame_2_failed,
			expecting_certificate_error,
			certificates_frame_failed,
			handshake_frame_3_failed,
			expecting_server_hello_done_error,
			CertCreateCertificateContext_failed,
			CryptImportPublicKeyInfoEx2_failed,
			BCryptEncrypt_1_failed,
			BCryptEncrypt_2_failed,
			WriteMessage_2_failed,
			WriteMessage_3_failed,
			handshake_frame_4_failed,
			expecting_finished_error,
			handshake_length_1_error,
			finished_received_frame_failed,
			finished_received_error,
			handshake_length_2_error,
			BCryptGenRandom_failed,
		};

		ServerHello serverHello;
		Certificates certificates;
		Inline<ServerHelloFrame> server_hello_frame;
		Inline<CertificatesFrame> certificates_frame;

		virtual void PartitionKeyMaterial(std::vector<opaque>* key_material);

	public:
		typedef Basic::Ref<ClientHandshake, IProcess> Ref;

		void Initialize(RecordLayer* session);
		virtual void IProcess::Process(IEvent* event, bool* yield);
	};
}