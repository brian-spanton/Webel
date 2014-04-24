// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.ClientHelloFrame.h"
#include "Basic.IgnoreFrame.h"
#include "Tls.Globals.h"
#include "Basic.CountStream.h"

namespace Tls
{
    using namespace Basic;

    void ClientHelloFrame::Initialize(ClientHello* clientHello, uint32 record_frame_length)
    {
        __super::Initialize();
        this->clientHello = clientHello;
        this->record_frame_length = record_frame_length;
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
            this->clientHello->heartbeat_extension_initialized = false;
            this->extensions_length = 0;

            this->counter = New<CountStream<byte> >();
            Event::AddObserver<byte>(event, this->counter);

            this->version_frame.Initialize(&this->clientHello->client_version);
            switch_to_state(event, State::version_frame_pending_state);
            break;

        case State::version_frame_pending_state:
            if (this->version_frame.Pending())
            {
                this->version_frame.Process(event, yield);
            }
            
            if (this->version_frame.Failed())
            {
                switch_to_state(event, State::version_frame_failed);
            }
            else if (this->version_frame.Succeeded())
            {
                this->random_frame.Initialize(&this->clientHello->random);
                switch_to_state(event, State::random_frame_pending_state);
            }
            break;

        case State::random_frame_pending_state:
            if (this->random_frame.Pending())
            {
                this->random_frame.Process(event, yield);
            }
            
            if (this->random_frame.Failed())
            {
                switch_to_state(event, State::random_frame_failed);
            }
            else if (this->random_frame.Succeeded())
            {
                this->session_id_frame.Initialize(&this->clientHello->session_id);
                switch_to_state(event, State::session_id_frame_pending_state);
            }
            break;

        case State::session_id_frame_pending_state:
            if (this->session_id_frame.Pending())
            {
                this->session_id_frame.Process(event, yield);
            }
            
            if (this->session_id_frame.Failed())
            {
                switch_to_state(event, State::session_id_frame_failed);
            }
            else if (this->session_id_frame.Succeeded())
            {
                this->cipher_suites_frame.Initialize(&this->clientHello->cipher_suites);
                switch_to_state(event, State::cipher_suites_frame_pending_state);
            }
            break;

        case State::cipher_suites_frame_pending_state:
            if (this->cipher_suites_frame.Pending())
            {
                this->cipher_suites_frame.Process(event, yield);
            }
            
            if (this->cipher_suites_frame.Failed())
            {
                switch_to_state(event, State::cipher_suites_frame_failed);
            }
            else if (this->cipher_suites_frame.Succeeded())
            {
                // IANA cipher suites registry
                // http:// www.iana.org/assignments/tls-parameters/tls-parameters.xml#tls-parameters-3
                this->compression_methods_frame.Initialize(&this->clientHello->compression_methods);
                switch_to_state(event, State::compression_methods_frame_pending_state);
            }
            break;

        case State::compression_methods_frame_pending_state:
            if (this->compression_methods_frame.Pending())
            {
                this->compression_methods_frame.Process(event, yield);
            }
            
            if (this->compression_methods_frame.Failed())
            {
                switch_to_state(event, State::compression_methods_frame_failed);
            }
            else if (this->compression_methods_frame.Succeeded())
            {
                uint32 received = this->counter->count;

                if (received > this->record_frame_length)
                {
                    switch_to_state(event, State::record_frame_length_error);
                }
                else if (received < this->record_frame_length)
                {
                    this->extensions_length_frame.Initialize(&this->extensions_length);
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

            if (this->extensions_length_frame.Failed())
            {
                switch_to_state(event, State::extensions_length_frame_failed);
            }
            else if (this->extensions_length_frame.Succeeded())
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
            
            if (this->extension_header_frame.Failed())
            {
                switch_to_state(event, State::extension_header_frame_failed);
            }
            else if (this->extension_header_frame.Succeeded())
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
                        this->server_name_list_frame.Initialize(&this->clientHello->server_name_list);
                        switch_to_state(event, State::server_name_list_frame_pending_state);
                        break;

                    case ExtensionType::signature_algorithms:
                        this->supported_signature_algorithms_frame.Initialize(&this->clientHello->supported_signature_algorithms);
                        switch_to_state(event, State::supported_signature_algorithms_frame_pending_state);
                        break;

                    case ExtensionType::renegotiation_info:
                        this->renegotiation_info_frame.Initialize(&this->clientHello->renegotiation_info);
                        switch_to_state(event, State::renegotiation_info_frame_pending_state);
                        break;

                    case ExtensionType::status_request:
                        this->certificate_status_request_frame.Initialize(&this->clientHello->certificate_status_request);
                        switch_to_state(event, State::certificate_status_request_frame_pending_state);
                        break;

                    case ExtensionType::elliptic_curves:
                        this->elliptic_curve_list_frame.Initialize(&this->clientHello->elliptic_curve_list);
                        switch_to_state(event, State::elliptic_curve_list_frame_pending_state);
                        break;

                    case ExtensionType::ec_point_formats:
                        this->ec_point_format_list_frame.Initialize(&this->clientHello->ec_point_format_list);
                        switch_to_state(event, State::ec_point_format_list_frame_pending_state);
                        break;

                    case ExtensionType::heartbeat_extension_type:
                        this->heartbeat_extension_frame.Initialize(&this->clientHello->heartbeat_extension);
                        switch_to_state(event, State::heartbeat_extension_frame_pending_state);
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

            if (this->server_name_list_frame.Failed())
            {
                switch_to_state(event, State::server_name_list_frame_failed);
            }
            else if (this->server_name_list_frame.Succeeded())
            {
                switch_to_state(event, State::next_extension_state);
            }
            break;

        case State::supported_signature_algorithms_frame_pending_state:
            if (this->supported_signature_algorithms_frame.Pending())
            {
                this->supported_signature_algorithms_frame.Process(event, yield);
            }
            
            if (this->supported_signature_algorithms_frame.Failed())
            {
                switch_to_state(event, State::supported_signature_algorithms_frame_failed);
            }
            else if (this->supported_signature_algorithms_frame.Succeeded())
            {
                switch_to_state(event, State::next_extension_state);
            }
            break;

        case State::renegotiation_info_frame_pending_state:
            if (this->renegotiation_info_frame.Pending())
            {
                this->renegotiation_info_frame.Process(event, yield);
            }

            if (this->renegotiation_info_frame.Failed())
            {
                switch_to_state(event, State::renegotiation_info_frame_failed);
            }
            else if (this->renegotiation_info_frame.Succeeded())
            {
                switch_to_state(event, State::next_extension_state);
            }
            break;

        case State::certificate_status_request_frame_pending_state:
            if (this->certificate_status_request_frame.Pending())
            {
                this->certificate_status_request_frame.Process(event, yield);
            }
            
            if (this->certificate_status_request_frame.Failed())
            {
                switch_to_state(event, State::certificate_status_request_frame_failed);
            }
            else if (this->certificate_status_request_frame.Succeeded())
            {
                switch_to_state(event, State::next_extension_state);
            }
            break;

        case State::elliptic_curve_list_frame_pending_state:
            if (this->elliptic_curve_list_frame.Pending())
            {
                this->elliptic_curve_list_frame.Process(event, yield);
            }
            
            if (this->elliptic_curve_list_frame.Failed())
            {
                switch_to_state(event, State::elliptic_curve_list_frame_failed);
            }
            else if (this->elliptic_curve_list_frame.Succeeded())
            {
                switch_to_state(event, State::next_extension_state);
            }
            break;

        case State::ec_point_format_list_frame_pending_state:
            if (this->ec_point_format_list_frame.Pending())
            {
                this->ec_point_format_list_frame.Process(event, yield);
            }
            
            if (this->ec_point_format_list_frame.Failed())
            {
                switch_to_state(event, State::ec_point_format_list_frame_failed);
            }
            else if (this->ec_point_format_list_frame.Succeeded())
            {
                switch_to_state(event, State::next_extension_state);
            }
            break;

        case State::heartbeat_extension_frame_pending_state:
            if (this->heartbeat_extension_frame.Pending())
            {
                this->heartbeat_extension_frame.Process(event, yield);
            }

            if (this->heartbeat_extension_frame.Failed())
            {
                switch_to_state(event, State::heartbeat_extension_frame_failed);
            }
            else if (this->heartbeat_extension_frame.Succeeded())
            {
                switch_to_state(event, State::next_extension_state);
                this->clientHello->heartbeat_extension_initialized = true;
            }
            break;

        case State::unknown_extension_frame_pending_state:
            if (this->unknown_extension_frame.Pending())
            {
                this->unknown_extension_frame.Process(event, yield);
            }

            if (this->unknown_extension_frame.Failed())
            {
                switch_to_state(event, State::unknown_extension_frame_failed);
            }
            else if (this->unknown_extension_frame.Succeeded())
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
        this->version_frame.Initialize(&this->clientHello->client_version);
        this->version_frame.SerializeTo(stream);

        this->random_frame.Initialize(&this->clientHello->random);
        this->random_frame.SerializeTo(stream);

        this->session_id_frame.Initialize(&this->clientHello->session_id);
        this->session_id_frame.SerializeTo(stream);

        this->cipher_suites_frame.Initialize(&this->clientHello->cipher_suites);
        this->cipher_suites_frame.SerializeTo(stream);

        this->compression_methods_frame.Initialize(&this->clientHello->compression_methods);
        this->compression_methods_frame.SerializeTo(stream);

        Inline<CountStream<byte> > count_stream;
        SerializeExtensionsTo(&count_stream);

        if (count_stream.count > 0)
        {
            this->extensions_length_frame.Initialize(&count_stream.count);
            SerializeExtensionsTo(stream);
        }
    }

    void ClientHelloFrame::SerializeExtensionsTo(IStream<byte>* stream)
    {
        if (this->clientHello->heartbeat_extension_initialized)
        {
            this->heartbeat_extension_frame.Initialize(&this->clientHello->heartbeat_extension);

            Inline<CountStream<byte> > count_stream;
            this->heartbeat_extension_frame.SerializeTo(&count_stream);

            this->extension_header.length = (uint16)count_stream.count;
            this->extension_header.type = ExtensionType::heartbeat_extension_type;

            this->extension_header_frame.Initialize(&this->extension_header);
            this->extension_header_frame.SerializeTo(stream);

            this->heartbeat_extension_frame.SerializeTo(stream);
        }
    }
}
