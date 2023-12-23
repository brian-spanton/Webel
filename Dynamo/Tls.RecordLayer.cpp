#include "stdafx.h"
#include "Tls.RecordLayer.h"
#include "Tls.Globals.h"
#include "Tls.RandomFrame.h"
#include "Tls.ServerHandshake.h"
#include "Tls.ClientHandshake.h"
#include "Basic.Event.h"

namespace Tls
{
	using namespace Basic;

	void RecordLayer::Initialize(IBufferedStream<byte>* peer, IProcess* application_stream, bool server)
	{
		__super::Initialize();

		this->transport_peer = peer;
		this->application_stream = application_stream;
		this->server = server;

		this->response_plain = New<ByteVector>();
		this->response_plain->reserve(0x400);

		this->version_low = 0x0301;
		this->version_high = 0x0302;
		this->version = this->version_high;
		this->version_finalized = false;

		this->session_id.resize(32);

		NTSTATUS error = BCryptGenRandom(0, &this->session_id[0], this->session_id.size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
		if (error != 0)
			throw new Exception("BCryptGenRandom", error);

		if (server)
		{
			ServerHandshake::Ref handshake_server = New<ServerHandshake>();
			handshake_server->Initialize(this);
			this->handshake_protocol = handshake_server;
		}
		else
		{
			ClientHandshake::Ref handshake_client = New<ClientHandshake>();
			handshake_client->Initialize(this);
			this->handshake_protocol = handshake_client;
		}

		this->alert_protocol = New<AlertProtocol>();
		this->alert_protocol->Initialize(this);

		SecurityParameters::Ref security_parameters = New<SecurityParameters>();

		this->active_read_state = New<ConnectionState>();
		this->active_read_state->security_parameters = security_parameters;

		this->active_write_state = New<ConnectionState>();
		this->active_write_state->security_parameters = security_parameters;

		this->pending_read_state = New<ConnectionState>();
		this->pending_write_state = New<ConnectionState>();
	}

	void RecordLayer::Process(IEvent* event, bool* yield)
	{
		if (event->get_type() == Basic::EventType::element_stream_ending_event)
		{
			switch_to_state(State::unconnected_state);

			ElementStreamEndingEvent application_event;
			this->application_stream->Process(&application_event);
		}

		switch (frame_state())
		{
		case State::unconnected_state:
			if (event->get_type() != Basic::EventType::ready_for_write_bytes_event)
			{
				(*yield) = true;
			}
			else
			{
				this->current_type = ContentType::handshake;

				ReadyForWriteBytesEvent handshake_event;
				handshake_event.Initialize(&this->handshake_element_source);
				this->handshake_protocol->Frame::Process(&handshake_event);
			
				switch_to_state(State::receive_record_state);
			}
			break;

		case State::receive_record_state:
			this->record_frame.Initialize(&this->record);
			switch_to_state(State::record_frame_pending_state);
			break;

		case State::record_frame_pending_state:
			if (this->record_frame.Pending())
			{
				this->record_frame.Process(event, yield);
			}
			else if (this->record_frame.Failed())
			{
				switch_to_state(State::record_frame_failed);
			}
			else
			{
				bool success = ProcessRecord(&this->record);
				if (!success)
				{
					switch_to_state(State::record_process_failed);
				}
				else
				{
					switch_to_state(State::receive_record_state);
				}
			}
			break;

		default:
			throw new Exception("Tls::RecordLayer::Process unexpected state");
		}
	}

	void RecordLayer::WriteEOF()
	{
		this->transport_peer->WriteEOF();
		this->transport_peer = 0;
	}

	void RecordLayer::WriteChangeCipherSpec()
	{
		Write(ContentType::change_cipher_spec, &ChangeCipherSpec, sizeof(ChangeCipherSpec));

		// make sure to actual send this record prior to changing the write states
		Flush();

		this->active_write_state = this->pending_write_state;
		this->pending_write_state = New<ConnectionState>();
	}

	void RecordLayer::ConnectApplication()
	{
		this->current_type = ContentType::application_data;
		ReadyForWriteBytesEvent event;
		event.Initialize(&this->application_element_source);
		this->application_stream->Process(&event);
	}

	bool RecordLayer::ProcessRecord(Record* record)
	{
		// From http://www.ietf.org/rfc/rfc2246.txt section 6.2.1:
		//     The length (in bytes) of the following TLSPlaintext.fragment.
		//     The length should not exceed 2^14.
		// Brian: since it is only *should*, and I see some records in real life come back at 0x4020, let's
		// give it double.
		if (record->length > 0x8000)
			return Basic::globals->HandleError("RecordLayer::ProcessRecord record->length > 0x8000", 0);

		if (this->version_finalized && record->version != this->version)
			return Basic::globals->HandleError("RecordLayer::ProcessRecord this->version_finalized && record->version != this->version", 0);

		Record compressed;
		bool success = Decrypt(record, &compressed);
		if (!success)
			return false;

		Record plaintext;
		success = Decompress(&compressed, &plaintext);
		if (!success)
			return false;

		switch(plaintext.type)
		{
		case ContentType::change_cipher_spec:
			{
				this->active_read_state = this->pending_read_state;
				this->pending_read_state = New<ConnectionState>();

				this->current_type = ContentType::handshake;
				ChangeCipherSpecEvent event;
				this->handshake_protocol->Frame::Process(&event);
			}
			break;

		case ContentType::alert:
			{
				this->current_type = ContentType::alert;
				this->alert_element_source.Initialize(plaintext.fragment->FirstElement(), plaintext.length);
				ReadyForReadBytesEvent event;
				event.Initialize(&this->alert_element_source);
				this->alert_protocol->Frame::Process(&event);
			}
			break;

		case ContentType::handshake:
			{
				this->current_type = ContentType::handshake;
				this->handshake_element_source.Initialize(plaintext.fragment->FirstElement(), plaintext.length);
				ReadyForReadBytesEvent event;
				event.Initialize(&this->handshake_element_source);
				this->handshake_protocol->Frame::Process(&event);
			}
			break;

		case ContentType::application_data:
			{
				this->current_type = ContentType::application_data;
				this->application_element_source.Initialize(plaintext.fragment->FirstElement(), plaintext.length);
				ReadyForReadBytesEvent event;
				event.Initialize(&this->application_element_source);
				this->application_stream->Process(&event);
			}
			break;

		default:
			return Basic::globals->HandleError("Tls::RecordLayer::ProcessRecord unexpected record type", 0);
		}

		return true;
	}

	void RecordLayer::Write(const byte* elements, uint32 count)
	{
		Write(this->current_type, elements, count);
	}

	void RecordLayer::Write(ContentType type, const byte* elements, uint32 count)
	{
		if (type != this->buffer_type)
		{
			if (this->response_plain->size() > 0)
			{
				Flush();
			}

			this->buffer_type = type;
		}

		// The record layer fragments information blocks into TLSPlaintext
		// records carrying data in chunks of 2^14 bytes or less.  Client
		// message boundaries are not preserved in the record layer (i.e.,
		// multiple client messages of the same ContentType MAY be coalesced
		// into a single TLSPlaintext record, or a single message MAY be
		// fragmented across several records).

		while (true)
		{
			uint32 remaining = 0x4000 - this->response_plain->size();
			uint32 useable = (count > remaining) ? remaining : count;

			this->response_plain->insert(this->response_plain->end(), elements, elements + useable);

			elements += useable;
			count -= useable;

			if (this->response_plain->size() == 0x4000)
			{
				Flush();
			}

			if (count == 0)
				break;
		}
	}

	void RecordLayer::Flush()
	{
		// Implementations MUST NOT send zero-length fragments of HandshakeProtocol,
		// Alert, or ChangeCipherSpec content types.  Zero-length fragments of
		// Application data MAY be sent as they are potentially useful as a
		// traffic analysis countermeasure.

		if (this->buffer_type == ContentType::alert ||
			this->buffer_type == ContentType::change_cipher_spec ||
			this->buffer_type == ContentType::handshake)
		{
			if (this->response_plain->size() == 0)
				throw new Exception("RecordLayer::WriteRecord this->response_plain->size() == 0"); // $$$
		}

		Record plaintext;
		plaintext.type = this->buffer_type;
		plaintext.version = this->version;
		plaintext.fragment = this->response_plain;
		plaintext.length = this->response_plain->size();

		Record compressed;
		Compress(&plaintext, &compressed);

		Record encrypted;
		Encrypt(&compressed, &encrypted);

		Inline<RecordFrame> recordFrame;
		recordFrame.Initialize(&encrypted);

		recordFrame.SerializeTo(this->transport_peer);

		this->response_plain->resize(0);

		this->transport_peer->Flush();
	}

	void RecordLayer::Compress(Record* plaintext, Record* compressed)
	{
		compressed->type = plaintext->type;
		compressed->version = plaintext->version;

		switch(this->active_write_state->security_parameters->compression_algorithm)
		{
		case CompressionMethod::cm_null:
			compressed->fragment = plaintext->fragment;
			compressed->length = plaintext->length;
			break;

		default:
			throw new Exception("Tls::RecordLayer::Compress unexpected CompressionMethod", 0);
		}
	}

	void RecordLayer::Encrypt(Record* compressed, Record* encrypted)
	{
		switch(this->active_write_state->security_parameters->cipher_type)
		{
		case CipherType::stream:
			EncryptStream(compressed, encrypted);
			return;

		case CipherType::block:
			EncryptBlock(compressed, encrypted);
			return;

		default:
			throw new Exception("Tls::RecordLayer::Encrypt unexpected CipherType", 0);
		}
	}

	void RecordLayer::EncryptStream(Record* compressed, Record* encrypted)
	{
		encrypted->type = compressed->type;
		encrypted->version = compressed->version;

		switch(this->active_write_state->security_parameters->bulk_cipher_algorithm)
		{
		case BulkCipherAlgorithm::bca_null:
			encrypted->fragment = compressed->fragment;
			encrypted->length = compressed->length;
			break;

		default:
			throw new Exception("Tls::RecordLayer::EncryptStream unexpected BulkCipherAlgorithm", 0);
		}
	}

	void RecordLayer::EncryptBlock(Record* compressed, Record* encrypted)
	{
		encrypted->type = compressed->type;
		encrypted->version = compressed->version;

		switch(this->active_write_state->security_parameters->bulk_cipher_algorithm)
		{
		case BulkCipherAlgorithm::aes:
			{
				uint8 mac_length = this->active_write_state->security_parameters->mac_length;
				uint8 padding_length;

				std::vector<byte> payload;
				payload.reserve(compressed->fragment->size() + mac_length + sizeof(padding_length) + 0x100);

				ByteVector::Ref mask;
				uint8 record_iv_length;

				if (this->version <= 0x0301)
				{
					record_iv_length = 0;

					mask = this->active_write_state->IV;
				}
				else
				{
					record_iv_length = this->active_write_state->security_parameters->block_length;

					mask = New<ByteVector>();
					mask->resize(record_iv_length);

					NTSTATUS error = BCryptGenRandom(0, mask->FirstElement(), mask->size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
					if (error != 0)
						throw new Exception("Tls::ClientHandshake::Process BCryptGenRandom failed", error);
				}

				payload.insert(payload.end(), compressed->fragment->begin(), compressed->fragment->end());

				int mac_index = payload.size();

				payload.resize(mac_index + mac_length);

				this->active_write_state->MAC(compressed, &payload[mac_index], mac_length);

				uint16 unpadded_length = payload.size() + sizeof(padding_length);
				uint8 overflow = unpadded_length % this->active_write_state->security_parameters->block_length;
				padding_length = overflow == 0 ? 0 : this->active_write_state->security_parameters->block_length - overflow;

				payload.insert(payload.end(), padding_length + 1, padding_length);

				encrypted->fragment = New<ByteVector>();
				encrypted->fragment->reserve(record_iv_length + payload.size());

				if (record_iv_length > 0)
				{
					encrypted->fragment->insert(encrypted->fragment->end(), mask->begin(), mask->end());
				}

				encrypted->fragment->resize(record_iv_length + payload.size());

				uint32 output_length;

				NTSTATUS error = BCryptEncrypt(
					this->active_write_state->key_handle,
					&payload[0],
					payload.size(),
					0,
					mask->FirstElement(),
					mask->size(),
					encrypted->fragment->FirstElement() + record_iv_length,
					encrypted->fragment->size() - record_iv_length,
					&output_length, 
					0);
				if (error != 0)
					throw new Exception("BCryptEncrypt", error);

				if (output_length > 0xffff)
					throw new Exception("Tls::RecordLayer::EncryptBlock output_length > 0xffff", 0);

				encrypted->fragment->resize(record_iv_length + output_length);
				encrypted->length = encrypted->fragment->size();

				if (this->version <= 0x0301)
				{
					ByteVector::Ref cbc_residue = New<ByteVector>();
					cbc_residue->insert(cbc_residue->end(), encrypted->fragment->end() - this->active_write_state->security_parameters->block_length, encrypted->fragment->end());
					this->active_write_state->IV = cbc_residue;
				}

				this->active_write_state->sequence_number++;
			}
			break;

		default:
			throw new Exception("Tls::RecordLayer::EncryptBlock unexpected BulkCipherAlgorithm", 0);
		}
	}

	bool RecordLayer::Decompress(Record* compressed, Record* plaintext)
	{
		plaintext->type = compressed->type;
		plaintext->version = compressed->version;

		switch(this->active_read_state->security_parameters->compression_algorithm)
		{
		case CompressionMethod::cm_null:
			plaintext->fragment = compressed->fragment;
			plaintext->length = compressed->length;
			break;

		default:
			return Basic::globals->HandleError("Tls::RecordLayer::Decompress unexpected CompressionMethod", 0);
		}

		return true;
	}

	bool RecordLayer::Decrypt(Record* encrypted, Record* compressed)
	{
		switch(this->active_read_state->security_parameters->cipher_type)
		{
		case CipherType::stream:
			return DecryptStream(encrypted, compressed);

		case CipherType::block:
			return DecryptBlock(encrypted, compressed);

		default:
			return Basic::globals->HandleError("Tls::RecordLayer::Decrypt unexpected CipherType", 0);
		}

		return true;
	}

	bool RecordLayer::DecryptStream(Record* encrypted, Record* compressed)
	{
		compressed->type = encrypted->type;
		compressed->version = encrypted->version;

		switch(this->active_read_state->security_parameters->bulk_cipher_algorithm)
		{
		case BulkCipherAlgorithm::bca_null:
			compressed->fragment = encrypted->fragment;
			compressed->length = encrypted->length;
			break;

		default:
			return Basic::globals->HandleError("Tls::RecordLayer::DecryptStream unexpected BulkCipherAlgorithm", 0);
		}

		return true;
	}

	bool RecordLayer::DecryptBlock(Record* encrypted, Record* compressed)
	{
		compressed->type = encrypted->type;
		compressed->version = encrypted->version;

		switch(this->active_read_state->security_parameters->bulk_cipher_algorithm)
		{
		case BulkCipherAlgorithm::aes:
			{
				ByteVector::Ref mask;
				uint8 record_iv_length;

				if (this->version <= 0x0301)
				{
					record_iv_length = 0;

					mask = this->active_read_state->IV;
				}
				else
				{
					record_iv_length = this->active_read_state->security_parameters->block_length;

					mask = New<ByteVector>();
					mask->insert(mask->end(), encrypted->fragment->begin(), encrypted->fragment->begin() + record_iv_length);
				}

				ByteVector::Ref payload = New<ByteVector>();
				payload->resize(encrypted->fragment->size() - record_iv_length);

				uint32 output_length;
				NTSTATUS error = BCryptDecrypt(
					this->active_read_state->key_handle, 
					encrypted->fragment->FirstElement() + record_iv_length, 
					encrypted->fragment->size() - record_iv_length, 
					0, 
					mask->FirstElement(), 
					mask->size(), 
					payload->FirstElement(), 
					payload->size(), 
					&output_length, 
					0);
				if (error != 0)
					return Basic::globals->HandleError("BCryptDecrypt", error);

				payload->resize(output_length);

				uint8 padding_length = payload->at(payload->size() - 1);
				uint8 mac_length = this->active_read_state->security_parameters->mac_length;
				uint16 fixed_portion = mac_length + sizeof(padding_length);
				uint16 variable_portion = payload->size() - fixed_portion;

				// Implementation note: Canvel et al. [CBCTIME] have demonstrated a
				// timing attack on CBC padding based on the time required to compute
				// the MAC.  In order to defend against this attack, implementations
				// MUST ensure that record processing time is essentially the same
				// whether or not the padding is correct.  In general, the best way to
				// do this is to compute the MAC even if the padding is incorrect, and
				// only then reject the packet.  For instance, if the pad appears to be
				// incorrect, the implementation might assume a zero-length pad and then
				// compute the MAC.
				std::string canvel;

				if (padding_length > variable_portion)
				{
					canvel = "Tls::RecordLayer::DecryptBlock padding_length > variable_portion";
					padding_length = 0;
				}

				uint16 padding_index = payload->size() - padding_length - sizeof(padding_length);

				for (int i = 0; i < padding_length; i++)
				{
					if (payload->at(padding_index + i) != padding_length)
					{
						if (canvel.size() == 0)
							canvel = "Tls::RecordLayer::DecryptBlock payload->at(i) != padding_length";

						padding_length = 0;
					}
				}

				uint16 fragment_length = variable_portion - padding_length;

				std::vector<opaque> received_MAC;
				received_MAC.insert(received_MAC.end(), payload->begin() + fragment_length, payload->begin() + fragment_length + mac_length);

				std::vector<opaque> expected_MAC;
				expected_MAC.resize(mac_length);

				payload->resize(fragment_length);
				
				compressed->fragment = payload;
				compressed->length = compressed->fragment->size();

				this->active_read_state->MAC(compressed, &expected_MAC[0], expected_MAC.size());

				if (canvel.size() > 0)
				{
					// $ return bad_record_mac error per tls 1.1
					return Basic::globals->HandleError(canvel.c_str(), 0);
				}

				for (int i = 0; i < mac_length; i++)
				{
					if (received_MAC[i] != expected_MAC[i])
						return Basic::globals->HandleError("Tls::RecordLayer::DecryptBlock received_MAC[i] != expected_MAC[i]", 0);
				}

				if (this->version <= 0x0301)
				{
					ByteVector::Ref cbc_residue = New<ByteVector>();
					cbc_residue->insert(cbc_residue->end(), encrypted->fragment->end() - this->active_read_state->security_parameters->block_length, encrypted->fragment->end());
					this->active_read_state->IV = cbc_residue;
				}

				this->active_read_state->sequence_number++;
			}
			break;

		default:
			return Basic::globals->HandleError("Tls::RecordLayer::DecryptBlock unexpected BulkCipherAlgorithm", 0);
		}

		return true;
	}

	bool RecordLayer::FinalizeVersion(ProtocolVersion version)
	{
		if (this->version_finalized)
			return Basic::globals->HandleError("RecordLayer::FinalizeVersion this->version_finalized", 0);

		this->version = version;
		this->version_finalized = true;

		// $ todo for tls 1.1:
		//-  Handling of padding errors is changed to use the bad_record_mac
		//   alert rather than the decryption_failed alert to protect against
		//   CBC attacks.

		//-  IANA registries are defined for protocol parameters.

		//-  Premature closes no longer cause a session to be nonresumable.

		//-  Additional informational notes were added for various new attacks
		//   on TLS.

		// $ todo for tls 1.2:
		//-  The MD5/SHA-1 combination in the pseudorandom function (PRF) has
		//   been replaced with cipher-suite-specified PRFs.  All cipher suites
		//   in this document use P_SHA256.

		//-  The MD5/SHA-1 combination in the digitally-signed element has been
		//   replaced with a single hash.  Signed elements now include a field
		//   that explicitly specifies the hash algorithm used.

		//-  Substantial cleanup to the client's and server's ability to
		//   specify which hash and signature algorithms they will accept.
		//   Note that this also relaxes some of the constraints on signature
		//   and hash algorithms from previous versions of TLS.

		//-  Addition of support for authenticated encryption with additional
		//   data modes.

		//-  TLS Extensions definition and AES Cipher Suites were merged in
		//   from external [TLSEXT] and [TLSAES].

		//-  Tighter checking of EncryptedPreMasterSecret version numbers.

		//-  Tightened up a number of requirements.

		//-  Verify_data length now depends on the cipher suite (default is
		//   still 12).

		//-  Cleaned up description of Bleichenbacher/Klima attack defenses.

		//-  Alerts MUST now be sent in many cases.

		//-  After a certificate_request, if no certificates are available,
		//   clients now MUST send an empty certificate list.

		//-  TLS_RSA_WITH_AES_128_CBC_SHA is now the mandatory to implement
		//   cipher suite.

		//-  Added HMAC-SHA256 cipher suites.

		//-  Removed IDEA and DES cipher suites.  They are now deprecated and
		//   will be documented in a separate document.

		//-  Support for the SSLv2 backward-compatible hello is now a MAY, not
		//   a SHOULD, with sending it a SHOULD NOT.  Support will probably
		//   become a SHOULD NOT in the future.

		//-  Added limited "fall-through" to the presentation language to allow
		//   multiple case arms to have the same encoding.

		//-  Added an Implementation Pitfalls sections

		return true;
	}
}