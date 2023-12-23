#pragma once

#include "Basic.IProcess.h"
#include "Basic.ElementSource.h"
#include "Tls.SecurityParameters.h"
#include "Tls.RecordFrame.h"
#include "Tls.ConnectionState.h"
#include "Tls.AlertProtocol.h"
#include "Tls.ServerHandshake.h"
#include "Basic.IBufferedStream.h"

namespace Tls
{
	using namespace Basic;

	class HandshakeProtocol;
	class ServerHandshake;
	class ClientHandshake;
	class RecordStream;

	class RecordLayer : public Frame, public IBufferedStream<byte>
	{
	private:
		enum State
		{
			unconnected_state = Start_State,
			receive_record_state,
			record_frame_pending_state,
			done_state = Succeeded_State,
			record_frame_failed,
			record_process_failed,
		};

		Inline<ElementSource<byte> > application_element_source;
		Basic::Ref<IProcess> application_stream; // $$$

		Inline<ElementSource<byte> > alert_element_source;
		AlertProtocol::Ref alert_protocol; // $$$

		Inline<ElementSource<byte> > handshake_element_source;
		HandshakeProtocol::Ref handshake_protocol; // $$$

		Record record;
		SessionId session_id;
		ProtocolVersion version_low;
		ProtocolVersion version_high;
		ProtocolVersion version;
		bool version_finalized;
		ByteVector::Ref response_plain; // $$$
		ContentType buffer_type;
		ContentType current_type;
		Basic::Ref<IBufferedStream<byte> > transport_peer; // $$$
		Inline<RecordFrame> record_frame;
		bool server;

		ConnectionState::Ref pending_read_state; // $$$
		ConnectionState::Ref pending_write_state; // $$$
		ConnectionState::Ref active_read_state; // $$$
		ConnectionState::Ref active_write_state; // $$$

		void ConnectApplication();
		bool FinalizeVersion(ProtocolVersion version);
		bool ProcessRecord(Record* record);
		bool SendRecord();
		void WriteChangeCipherSpec();
		void Write(ContentType type, const byte* elements, uint32 count);

		void Compress(Record* plaintext, Record* compressed);
		void Encrypt(Record* compressed, Record* encrypted);
		void EncryptStream(Record* compressed, Record* encrypted);
		void EncryptBlock(Record* compressed, Record* encrypted);

		bool Decompress(Record* compressed, Record* plaintext);
		bool Decrypt(Record* encrypted, Record* compressed);
		bool DecryptStream(Record* encrypted, Record* compressed);
		bool DecryptBlock(Record* encrypted, Record* compressed);

	public:
		friend class Tls::HandshakeProtocol;
		friend class Tls::ServerHandshake;
		friend class Tls::ClientHandshake;
		friend class Tls::RecordStream;

		typedef Basic::Ref<RecordLayer, IProcess> Ref;

		void Initialize(IBufferedStream<byte>* peer, IProcess* application_stream, bool server);

		virtual void IProcess::Process(IEvent* event, bool* yield);

		virtual void IBufferedStream<byte>::Write(const byte* elements, uint32 count);
		virtual void IBufferedStream<byte>::WriteEOF();
		virtual void IBufferedStream<byte>::Flush();
	};
}