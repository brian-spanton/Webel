// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.ServerHelloFrame.h"
#include "Tls.Globals.h"

namespace Tls
{
    using namespace Basic;

    void ServerHelloFrame::Initialize(ServerHello* serverHello, uint32 record_frame_length)
    {
        __super::Initialize();

        this->serverHello = serverHello;
        this->record_frame_length = record_frame_length;
        this->serverHello->heartbeat_extension_initialized = false;
        this->version_frame.Initialize(&this->serverHello->server_version);
        this->random_frame.Initialize(&this->serverHello->random);
        this->session_id_frame.Initialize(&this->serverHello->session_id);
        this->cipher_suite_frame.Initialize(&this->serverHello->cipher_suite);
        this->compression_method_frame.Initialize(&this->serverHello->compression_method);
        this->extensions_length_frame.Initialize(&this->extensions_length);
    }

    void ServerHelloFrame::switch_to_state(IEvent* event, State state)
    {
        __super::switch_to_state(state);

        if (!Pending())
            Event::RemoveObserver<byte>(event, this->counter);
    }

    void ServerHelloFrame::Process(IEvent* event, bool* yield)
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

            if (this->version_frame.Failed())
            {
                switch_to_state(event, State::version_frame_failed);
            }
            else if (this->version_frame.Succeeded())
            {
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
                switch_to_state(event, State::session_id_pending_state);
            }
            break;

        case State::session_id_pending_state:
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
                switch_to_state(event, State::cipher_suite_frame_pending_state);
            }
            break;

        case State::cipher_suite_frame_pending_state:
            if (this->cipher_suite_frame.Pending())
            {
                this->cipher_suite_frame.Process(event, yield);
            }

            if (this->cipher_suite_frame.Failed())
            {
                switch_to_state(event, State::cipher_suite_frame_failed);
            }
            else if (this->cipher_suite_frame.Succeeded())
            {
                // IANA cipher suites registry
                // http:// www.iana.org/assignments/tls-parameters/tls-parameters.xml#tls-parameters-3
                switch_to_state(event, State::compression_method_frame_pending_state);
            }
            break;

        case State::compression_method_frame_pending_state:
            if (this->compression_method_frame.Pending())
            {
                this->compression_method_frame.Process(event, yield);
            }

            if (this->compression_method_frame.Failed())
            {
                switch_to_state(event, State::compression_method_frame_failed);
            }
            else if (this->compression_method_frame.Succeeded())
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
                    case ExtensionType::heartbeat_extension_type:
                        this->heartbeat_extension_frame.Initialize(&this->serverHello->heartbeat_extension);
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
                this->serverHello->heartbeat_extension_initialized = true;
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
            throw new Exception("ServerHelloFrame::Process unexpected state");
        }
    }

    void ServerHelloFrame::SerializeTo(IStream<byte>* stream)
    {
        this->version_frame.SerializeTo(stream);
        this->random_frame.SerializeTo(stream);
        this->session_id_frame.SerializeTo(stream);
        this->cipher_suite_frame.SerializeTo(stream);
        this->compression_method_frame.SerializeTo(stream);
    }
}
