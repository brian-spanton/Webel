// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.ClientHelloFrame.h"
#include "Tls.Globals.h"
#include "Basic.CountStream.h"
#include "Tls.ServerNameFrame.h"
#include "Tls.SignatureAndHashAlgorithmFrame.h"

namespace Tls
{
    using namespace Basic;

    ClientHelloFrame::ClientHelloFrame(ClientHello* clientHello) :
        record_frame_length(0),
        extensions_length(0),
        clientHello(clientHello),
        version_frame(&this->clientHello->client_version), // order of declaration is important
        random_frame(&this->clientHello->random), // order of declaration is important
        session_id_frame(&this->clientHello->session_id), // order of declaration is important
        cipher_suites_frame(&this->clientHello->cipher_suites), // order of declaration is important
        compression_methods_frame(&this->clientHello->compression_methods), // order of declaration is important
        extensions_length_frame(&this->extensions_length), // order of declaration is important
        extension_header_frame(&this->extension_header), // order of declaration is important
        server_name_list_frame(&this->clientHello->server_name_list), // order of declaration is important
        supported_signature_algorithms_frame(&this->clientHello->supported_signature_algorithms), // order of declaration is important
        renegotiation_info_frame(&this->clientHello->renegotiation_info), // order of declaration is important
        certificate_status_request_frame(&this->clientHello->certificate_status_request), // order of declaration is important
        elliptic_curve_list_frame(&this->clientHello->elliptic_curve_list), // order of declaration is important
        ec_point_format_list_frame(&this->clientHello->ec_point_format_list), // order of declaration is important
        heartbeat_extension_frame(&this->clientHello->heartbeat_extension) // order of declaration is important
    {
        this->clientHello->heartbeat_extension_initialized = false;
    }

    void ClientHelloFrame::set_record_frame_length(uint32 record_frame_length)
    {
        this->record_frame_length = record_frame_length;
    }

    void ClientHelloFrame::switch_to_state(IEvent* event, State state)
    {
        __super::switch_to_state(state);

        if (!this->in_progress())
            Event::RemoveObserver<byte>(event, this->counter);
    }

    void ClientHelloFrame::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::start_state:
            this->counter = std::make_shared<CountStream<byte> >();
            Event::AddObserver<byte>(event, this->counter);

            switch_to_state(event, State::version_frame_pending_state);
            break;

        case State::version_frame_pending_state:
            delegate_event_change_state_on_fail(&this->version_frame, event, State::version_frame_failed);
            switch_to_state(event, State::random_frame_pending_state);
            break;

        case State::random_frame_pending_state:
            delegate_event_change_state_on_fail(&this->random_frame, event, State::random_frame_failed);
            switch_to_state(event, State::session_id_frame_pending_state);
            break;

        case State::session_id_frame_pending_state:
            delegate_event_change_state_on_fail(&this->session_id_frame, event, State::session_id_frame_failed);
            switch_to_state(event, State::cipher_suites_frame_pending_state);
            break;

        case State::cipher_suites_frame_pending_state:
            delegate_event_change_state_on_fail(&this->cipher_suites_frame, event, State::cipher_suites_frame_failed);

            // IANA cipher suites registry
            // http:// www.iana.org/assignments/tls-parameters/tls-parameters.xml#tls-parameters-3
            switch_to_state(event, State::compression_methods_frame_pending_state);
            break;

        case State::compression_methods_frame_pending_state:
            {
                delegate_event_change_state_on_fail(&this->compression_methods_frame, event, State::compression_methods_frame_failed);

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
            {
                delegate_event_change_state_on_fail(&this->extensions_length_frame, event, State::extensions_length_frame_failed);

                this->counter->count = 0;
                this->extension_header_frame.reset();
                switch_to_state(event, State::extension_header_frame_pending_state);
            }
            break;
    
        case State::extension_header_frame_pending_state:
            {
                delegate_event_change_state_on_fail(&this->extension_header_frame, event, State::extension_header_frame_failed);

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

                    case ExtensionType::heartbeat_extension_type:
                        switch_to_state(event, State::heartbeat_extension_frame_pending_state);
                        break;

                    default:
                        this->unknown_extension_frame.reset(this->extension_header.length);
                        switch_to_state(event, State::unknown_extension_frame_pending_state);
                        break;
                    }
                }
            }
            break;

        case State::server_name_list_frame_pending_state:
            delegate_event_change_state_on_fail(&this->server_name_list_frame, event, State::server_name_list_frame_failed);
            switch_to_state(event, State::next_extension_state);
            break;

        case State::supported_signature_algorithms_frame_pending_state:
            delegate_event_change_state_on_fail(&this->supported_signature_algorithms_frame, event, State::supported_signature_algorithms_frame_failed);
            switch_to_state(event, State::next_extension_state);
            break;

        case State::renegotiation_info_frame_pending_state:
            delegate_event_change_state_on_fail(&this->renegotiation_info_frame, event, State::renegotiation_info_frame_failed);
            switch_to_state(event, State::next_extension_state);
            break;

        case State::certificate_status_request_frame_pending_state:
            delegate_event_change_state_on_fail(&this->certificate_status_request_frame, event, State::certificate_status_request_frame_failed);
            switch_to_state(event, State::next_extension_state);
            break;

        case State::elliptic_curve_list_frame_pending_state:
            delegate_event_change_state_on_fail(&this->elliptic_curve_list_frame, event, State::elliptic_curve_list_frame_failed);
            switch_to_state(event, State::next_extension_state);
            break;

        case State::ec_point_format_list_frame_pending_state:
            delegate_event_change_state_on_fail(&this->ec_point_format_list_frame, event, State::ec_point_format_list_frame_failed);
            switch_to_state(event, State::next_extension_state);
            break;

        case State::heartbeat_extension_frame_pending_state:
            delegate_event_change_state_on_fail(&this->heartbeat_extension_frame, event, State::heartbeat_extension_frame_failed);
            switch_to_state(event, State::next_extension_state);
            this->clientHello->heartbeat_extension_initialized = true;
            break;

        case State::unknown_extension_frame_pending_state:
            delegate_event_change_state_on_fail(&this->unknown_extension_frame, event, State::unknown_extension_frame_failed);
            switch_to_state(event, State::next_extension_state);
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
                    this->extension_header_frame.reset();
                    switch_to_state(event, State::extension_header_frame_pending_state);
                }
                else
                {
                    switch_to_state(event, State::done_state);
                }
            }
            break;
            
        default:
            throw FatalError("ClientHelloFrame::handle_event unexpected state");
        }
    }
}
