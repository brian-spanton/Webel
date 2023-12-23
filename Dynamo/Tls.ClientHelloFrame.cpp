#include "stdafx.h"
#include "Tls.ClientHelloFrame.h"
#include "Basic.IgnoreFrame.h"
#include "Tls.Globals.h"

namespace Tls
{
	using namespace Basic;

	void ClientHelloFrame::Initialize(ClientHello* clientHello, uint32 record_frame_length)
	{
		__super::Initialize();
		this->clientHello = clientHello;
		this->record_frame_length = record_frame_length;
		this->extensions_length = 0;
		this->version_frame.Initialize(&this->clientHello->client_version);
		this->random_frame.Initialize(&this->clientHello->random);
		this->session_id_frame.Initialize(&this->clientHello->session_id);
		this->cipher_suites_frame.Initialize(&this->clientHello->cipher_suites);
		this->compression_methods_frame.Initialize(&this->clientHello->compression_methods);
		this->extensions_length_frame.Initialize(&this->extensions_length);
		this->server_name_list_frame.Initialize(&this->clientHello->server_name_list);
		this->supported_signature_algorithms_frame.Initialize(&this->clientHello->supported_signature_algorithms);
		this->renegotiation_info_frame.Initialize(&this->clientHello->renegotiation_info);
		this->certificate_status_request_frame.Initialize(&this->clientHello->certificate_status_request);
		this->elliptic_curve_list_frame.Initialize(&this->clientHello->elliptic_curve_list);
		this->ec_point_format_list_frame.Initialize(&this->clientHello->ec_point_format_list);
	}

	void ClientHelloFrame::switch_to_state(IEvent* event, State state)
	{
		__super::switch_to_state(state);

		if (!Pending())
			Event::RemoveObserver<byte>(event, this->counter);
	}

	void ClientHelloFrame::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::start_state:
			this->counter = New<CountStream<byte> >();
			Event::AddObserver<byte>(event, this->counter);
			switch_to_state(event, State::version_frame_pending_state);
			break;

		case State::version_frame_pending_state:
			if (this->version_frame.Pending())
			{
				this->version_frame.Process(event, yield);
			}
			else if (this->version_frame.Failed())
			{
				switch_to_state(event, State::version_frame_failed);
			}
			else
			{
				switch_to_state(event, State::random_frame_pending_state);
			}
			break;

		case State::random_frame_pending_state:
			if (this->random_frame.Pending())
			{
				this->random_frame.Process(event, yield);
			}
			else if (this->random_frame.Failed())
			{
				switch_to_state(event, State::random_frame_failed);
			}
			else
			{
				switch_to_state(event, State::session_id_frame_pending_state);
			}
			break;

		case State::session_id_frame_pending_state:
			if (this->session_id_frame.Pending())
			{
				this->session_id_frame.Process(event, yield);
			}
			else if (this->session_id_frame.Failed())
			{
				switch_to_state(event, State::session_id_frame_failed);
			}
			else
			{
				switch_to_state(event, State::cipher_suites_frame_pending_state);
			}
			break;

		case State::cipher_suites_frame_pending_state:
			if (this->cipher_suites_frame.Pending())
			{
				this->cipher_suites_frame.Process(event, yield);
			}
			else if (this->cipher_suites_frame.Failed())
			{
				switch_to_state(event, State::cipher_suites_frame_failed);
			}
			else
			{
				// IANA cipher suites registry
				// http:// www.iana.org/assignments/tls-parameters/tls-parameters.xml#tls-parameters-3
				switch_to_state(event, State::compression_methods_frame_pending_state);
			}
			break;

		case State::compression_methods_frame_pending_state:
			if (this->compression_methods_frame.Pending())
			{
				this->compression_methods_frame.Process(event, yield);
			}
			else if (this->compression_methods_frame.Failed())
			{
				switch_to_state(event, State::compression_methods_frame_failed);
			}
			else
			{
				uint32 received = this->counter->count;

				if (received > this->record_frame_length)
				{
					switch_to_state(event, State::record_frame_length_error);
				}
				else if (received < this->record_frame_length)
				{
					switch_to_state(event, State::extensions_length_frame_pending_state);
				}
				else
				{
					switch_to_state(event, State::done_state);
				}
			}
			break;

		case State::extensions_length_frame_pending_state:
			if (this->extensions_length_frame.Pending())
			{
				this->extensions_length_frame.Process(event, yield);
			}
			else if (this->extensions_length_frame.Failed())
			{
				switch_to_state(event, State::extensions_length_frame_failed);
			}
			else
			{
				this->counter->count = 0;
				this->extension_header_frame.Initialize(&this->extension_header);
				switch_to_state(event, State::extension_header_frame_pending_state);
			}
			break;

		case State::extension_header_frame_pending_state:
			if (this->extension_header_frame.Pending())
			{
				this->extension_header_frame.Process(event, yield);
			}
			else if (this->extension_header_frame.Failed())
			{
				switch_to_state(event, State::extension_header_frame_failed);
			}
			else
			{
				uint32 received = this->counter->count;

				if (received > this->extensions_length)
				{
					switch_to_state(event, State::extensions_length_error);
				}
				else
				{
					switch(this->extension_header.type)
					{
					case ExtensionType::server_name:
						switch_to_state(event, State::server_name_list_frame_pending_state);
						break;

					case ExtensionType::signature_algorithms:
						switch_to_state(event, State::supported_signature_algorithms_frame_pending_state);
						break;

					case ExtensionType::renegotiation_info:
						switch_to_state(event, State::renegotiation_info_frame_pending_state);
						break;

					case ExtensionType::status_request:
						switch_to_state(event, State::certificate_status_request_frame_pending_state);
						break;

					case ExtensionType::elliptic_curves:
						switch_to_state(event, State::elliptic_curve_list_frame_pending_state);
						break;

					case ExtensionType::ec_point_formats:
						switch_to_state(event, State::ec_point_format_list_frame_pending_state);
						break;

					default:
						this->unknown_extension_frame.Initialize(this->extension_header.length);
						switch_to_state(event, State::unknown_extension_frame_pending_state);
						break;
					}
				}
			}
			break;

		case State::server_name_list_frame_pending_state:
			if (this->server_name_list_frame.Pending())
			{
				this->server_name_list_frame.Process(event, yield);
			}
			else if (this->server_name_list_frame.Failed())
			{
				switch_to_state(event, State::server_name_list_frame_failed);
			}
			else
			{
				switch_to_state(event, State::next_extension_state);
			}
			break;

		case State::supported_signature_algorithms_frame_pending_state:
			if (this->supported_signature_algorithms_frame.Pending())
			{
				this->supported_signature_algorithms_frame.Process(event, yield);
			}
			else if (this->supported_signature_algorithms_frame.Failed())
			{
				switch_to_state(event, State::supported_signature_algorithms_frame_failed);
			}
			else
			{
				switch_to_state(event, State::next_extension_state);
			}
			break;

		case State::renegotiation_info_frame_pending_state:
			if (this->renegotiation_info_frame.Pending())
			{
				this->renegotiation_info_frame.Process(event, yield);
			}
			else if (this->renegotiation_info_frame.Failed())
			{
				switch_to_state(event, State::renegotiation_info_frame_failed);
			}
			else
			{
				switch_to_state(event, State::next_extension_state);
			}
			break;

		case State::certificate_status_request_frame_pending_state:
			if (this->certificate_status_request_frame.Pending())
			{
				this->certificate_status_request_frame.Process(event, yield);
			}
			else if (this->certificate_status_request_frame.Failed())
			{
				switch_to_state(event, State::certificate_status_request_frame_failed);
			}
			else
			{
				switch_to_state(event, State::next_extension_state);
			}
			break;

		case State::elliptic_curve_list_frame_pending_state:
			if (this->elliptic_curve_list_frame.Pending())
			{
				this->elliptic_curve_list_frame.Process(event, yield);
			}
			else if (this->elliptic_curve_list_frame.Failed())
			{
				switch_to_state(event, State::elliptic_curve_list_frame_failed);
			}
			else
			{
				switch_to_state(event, State::next_extension_state);
			}
			break;

		case State::ec_point_format_list_frame_pending_state:
			if (this->ec_point_format_list_frame.Pending())
			{
				this->ec_point_format_list_frame.Process(event, yield);
			}
			else if (this->ec_point_format_list_frame.Failed())
			{
				switch_to_state(event, State::ec_point_format_list_frame_failed);
			}
			else
			{
				switch_to_state(event, State::next_extension_state);
			}
			break;

		case State::unknown_extension_frame_pending_state:
			if (this->unknown_extension_frame.Pending())
			{
				this->unknown_extension_frame.Process(event, yield);
			}
			else if (this->unknown_extension_frame.Failed())
			{
				switch_to_state(event, State::unknown_extension_frame_failed);
			}
			else
			{
				switch_to_state(event, State::next_extension_state);
			}
			break;

		case State::next_extension_state:
			{
				uint32 received = this->counter->count;

				if (received > this->extensions_length)
				{
					switch_to_state(event, State::extensions_length_error);
				}
				else if (received < this->extensions_length)
				{
					this->extension_header_frame.Initialize(&this->extension_header);
					switch_to_state(event, State::extension_header_frame_pending_state);
				}
				else
				{
					switch_to_state(event, State::done_state);
				}
			}
			break;
			
		default:
			throw new Exception("ClientHelloFrame::Process unexpected state");
		}
	}

	void ClientHelloFrame::SerializeTo(IStream<byte>* stream)
	{
		this->version_frame.SerializeTo(stream);
		this->random_frame.SerializeTo(stream);
		this->session_id_frame.SerializeTo(stream);
		this->cipher_suites_frame.SerializeTo(stream);
		this->compression_methods_frame.SerializeTo(stream);
	}
}
