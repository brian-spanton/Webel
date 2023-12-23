// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.ClientHelloFrame.h"
#include "Tls.Globals.h"
#include "Basic.CountStream.h"
#include "Tls.ServerNameFrame.h"
#include "Tls.SignatureAndHashAlgorithmFrame.h"
#include "Basic.ObservableElementSource.h"

namespace Tls
{
    using namespace Basic;

    ClientHelloFrame::ClientHelloFrame(ClientHello* clientHello) :
        record_frame_length(0),
        extensions_length(0),
        clientHello(clientHello),
        version_frame(&this->clientHello->client_version), // initialization is in order of declaration in class def
        random_frame(&this->clientHello->random), // initialization is in order of declaration in class def
        session_id_frame(&this->clientHello->session_id), // initialization is in order of declaration in class def
        cipher_suites_frame(&this->clientHello->cipher_suites), // initialization is in order of declaration in class def
        compression_methods_frame(&this->clientHello->compression_methods), // initialization is in order of declaration in class def
        extensions_length_frame(&this->extensions_length), // initialization is in order of declaration in class def
        extension_header_frame(&this->extension_header), // initialization is in order of declaration in class def
        server_name_list_frame(&this->clientHello->server_name_list), // initialization is in order of declaration in class def
        supported_signature_algorithms_frame(&this->clientHello->supported_signature_algorithms), // initialization is in order of declaration in class def
        renegotiation_info_frame(&this->clientHello->renegotiation_info), // initialization is in order of declaration in class def
        certificate_status_request_frame(&this->clientHello->certificate_status_request), // initialization is in order of declaration in class def
        elliptic_curve_list_frame(&this->clientHello->elliptic_curve_list), // initialization is in order of declaration in class def
        ec_point_format_list_frame(&this->clientHello->ec_point_format_list), // initialization is in order of declaration in class def
        heartbeat_extension_frame(&this->clientHello->heartbeat_extension) // initialization is in order of declaration in class def
    {
        this->clientHello->heartbeat_extension_initialized = false;
		this->counter = std::make_shared<CountStream<byte> >();
	}

    void ClientHelloFrame::set_record_frame_length(uint32 record_frame_length)
    {
        this->record_frame_length = record_frame_length;
    }

	ConsumeElementsResult ClientHelloFrame::consume_elements(IElementSource<byte>* element_source)
    {
		ObservableElementSource<byte> observed_element_source(element_source);

		observed_element_source.AddObserver(this->counter);
		element_source = &observed_element_source;

		switch (this->get_state())
        {
        case State::version_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->version_frame, element_source, this, State::version_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::random_frame_pending_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::random_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->random_frame, element_source, this, State::random_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::session_id_frame_pending_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::session_id_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->session_id_frame, element_source, this, State::session_id_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::cipher_suites_frame_pending_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::cipher_suites_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->cipher_suites_frame, element_source, this, State::cipher_suites_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			// IANA cipher suites registry
			// http:// www.iana.org/assignments/tls-parameters/tls-parameters.xml#tls-parameters-3

			this->switch_to_state(State::compression_methods_frame_pending_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::compression_methods_frame_pending_state:
        {
			ConsumeElementsResult result = Basic::consume_elements(&this->compression_methods_frame, element_source, this, State::compression_methods_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

            uint32 received = this->counter->count;

            if (received > this->record_frame_length)
            {
                this->switch_to_state(State::record_frame_length_error);
				return ConsumeElementsResult::failed;
			}

            if (received == this->record_frame_length)
            {
				this->switch_to_state(State::done_state);
				return ConsumeElementsResult::succeeded;
			}

			this->switch_to_state(State::extensions_length_frame_pending_state);
			return ConsumeElementsResult::in_progress;
        }

        case State::extensions_length_frame_pending_state:
        {
			ConsumeElementsResult result = Basic::consume_elements(&this->extensions_length_frame, element_source, this, State::extensions_length_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

            this->counter->count = 0;
            this->extension_header_frame.reset();

            this->switch_to_state(State::extension_header_frame_pending_state);
			return ConsumeElementsResult::in_progress;
		}
    
        case State::extension_header_frame_pending_state:
        {
			ConsumeElementsResult result = Basic::consume_elements(&this->extension_header_frame, element_source, this, State::extension_header_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

            uint32 received = this->counter->count;

            if (received > this->extensions_length)
            {
                this->switch_to_state(State::extensions_length_error);
				return ConsumeElementsResult::failed;
			}

            switch (this->extension_header.type)
            {
            case ExtensionType::server_name:
                this->switch_to_state(State::server_name_list_frame_pending_state);
				return ConsumeElementsResult::in_progress;

            case ExtensionType::signature_algorithms:
                this->switch_to_state(State::supported_signature_algorithms_frame_pending_state);
				return ConsumeElementsResult::in_progress;

            case ExtensionType::renegotiation_info:
                this->switch_to_state(State::renegotiation_info_frame_pending_state);
				return ConsumeElementsResult::in_progress;

            case ExtensionType::status_request:
                this->switch_to_state(State::certificate_status_request_frame_pending_state);
				return ConsumeElementsResult::in_progress;

            case ExtensionType::elliptic_curves:
                this->switch_to_state(State::elliptic_curve_list_frame_pending_state);
				return ConsumeElementsResult::in_progress;

            case ExtensionType::ec_point_formats:
                this->switch_to_state(State::ec_point_format_list_frame_pending_state);
				return ConsumeElementsResult::in_progress;

            case ExtensionType::heartbeat_extension_type:
                this->switch_to_state(State::heartbeat_extension_frame_pending_state);
				return ConsumeElementsResult::in_progress;

            default:
                this->unknown_extension_frame.reset(this->extension_header.length);
                this->switch_to_state(State::unknown_extension_frame_pending_state);
				return ConsumeElementsResult::in_progress;
			}
        }

		case State::server_name_list_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->server_name_list_frame, element_source, this, State::server_name_list_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::next_extension_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::supported_signature_algorithms_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->supported_signature_algorithms_frame, element_source, this, State::supported_signature_algorithms_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::next_extension_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::renegotiation_info_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->renegotiation_info_frame, element_source, this, State::renegotiation_info_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::next_extension_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::certificate_status_request_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->certificate_status_request_frame, element_source, this, State::certificate_status_request_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::next_extension_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::elliptic_curve_list_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->elliptic_curve_list_frame, element_source, this, State::elliptic_curve_list_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::next_extension_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::ec_point_format_list_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->ec_point_format_list_frame, element_source, this, State::ec_point_format_list_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::next_extension_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::heartbeat_extension_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->heartbeat_extension_frame, element_source, this, State::heartbeat_extension_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->clientHello->heartbeat_extension_initialized = true;

			this->switch_to_state(State::next_extension_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::unknown_extension_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->unknown_extension_frame, element_source, this, State::unknown_extension_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::next_extension_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::next_extension_state:
        {
            uint32 received = this->counter->count;

            if (received > this->extensions_length)
            {
                this->switch_to_state(State::extensions_length_error);
				return ConsumeElementsResult::failed;
			}

            if (received == this->extensions_length)
            {
				this->switch_to_state(State::done_state);
				return ConsumeElementsResult::succeeded;
			}

			this->extension_header_frame.reset();

			this->switch_to_state(State::extension_header_frame_pending_state);
			return ConsumeElementsResult::in_progress;
		}
            
        default:
            throw FatalError("ClientHelloFrame::handle_event unexpected state");
        }
    }
}
