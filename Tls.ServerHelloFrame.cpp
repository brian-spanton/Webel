// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.ServerHelloFrame.h"
#include "Tls.Globals.h"

namespace Tls
{
    using namespace Basic;

    ServerHelloFrame::ServerHelloFrame(ServerHello* serverHello) :
        record_frame_length(0),
        extensions_length(0),
        serverHello(serverHello),
        version_frame(&this->serverHello->server_version), // initialization is in order of declaration in class def
        random_frame(&this->serverHello->random), // initialization is in order of declaration in class def
        session_id_frame(&this->serverHello->session_id), // initialization is in order of declaration in class def
        cipher_suite_frame(&this->serverHello->cipher_suite), // initialization is in order of declaration in class def
        compression_method_frame(&this->serverHello->compression_method), // initialization is in order of declaration in class def
        extensions_length_frame(&this->extensions_length), // initialization is in order of declaration in class def
        extension_header_frame(&this->extension_header), // initialization is in order of declaration in class def
        heartbeat_extension_frame(&this->serverHello->heartbeat_extension) // initialization is in order of declaration in class def
    {
        this->serverHello->heartbeat_extension_initialized = false;
    }

    void ServerHelloFrame::set_record_frame_length(uint32 record_frame_length)
    {
        this->record_frame_length = record_frame_length;
    }

    void ServerHelloFrame::switch_to_state(IEvent* event, State state)
    {
        __super::switch_to_state(state);

        if (!this->in_progress())
            Event::RemoveObserver<byte>(event, this->counter);
    }

    ProcessResult ServerHelloFrame::consider_event(IEvent* event)
    {
        ProcessResult result;

        switch (get_state())
        {
        case State::start_state:
            this->counter = std::make_shared<CountStream<byte> >();
            Event::AddObserver<byte>(event, this->counter);

            switch_to_state(event, State::version_frame_pending_state);
            break;

        case State::version_frame_pending_state:
            result = process_event_change_state_on_fail(&this->version_frame, event, State::version_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(event, State::random_frame_pending_state);
            break;

        case State::random_frame_pending_state:
            result = process_event_change_state_on_fail(&this->random_frame, event, State::random_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(event, State::session_id_frame_pending_state);
            break;

        case State::session_id_frame_pending_state:
            result = process_event_change_state_on_fail(&this->session_id_frame, event, State::session_id_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(event, State::cipher_suite_frame_pending_state);
            break;

        case State::cipher_suite_frame_pending_state:
            result = process_event_change_state_on_fail(&this->cipher_suite_frame, event, State::cipher_suite_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            // IANA cipher suites registry
            // http:// www.iana.org/assignments/tls-parameters/tls-parameters.xml#tls-parameters-3
            switch_to_state(event, State::compression_method_frame_pending_state);
            break;

        case State::compression_method_frame_pending_state:
            {
                result = process_event_change_state_on_fail(&this->compression_method_frame, event, State::compression_method_frame_failed);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

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
                result = process_event_change_state_on_fail(&this->extensions_length_frame, event, State::extensions_length_frame_failed);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                this->counter->count = 0;
                this->extension_header_frame.reset();
                switch_to_state(event, State::extension_header_frame_pending_state);
            }
            break;

        case State::extension_header_frame_pending_state:
            {
                result = process_event_change_state_on_fail(&this->extension_header_frame, event, State::extension_header_frame_failed);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

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

        case State::heartbeat_extension_frame_pending_state:
            result = process_event_change_state_on_fail(&this->heartbeat_extension_frame, event, State::heartbeat_extension_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(event, State::next_extension_state);
            this->serverHello->heartbeat_extension_initialized = true;
            break;

        case State::unknown_extension_frame_pending_state:
            result = process_event_change_state_on_fail(&this->unknown_extension_frame, event, State::unknown_extension_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

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
            throw FatalError("ServerHelloFrame::handle_event unexpected state");
        }

        return ProcessResult::process_result_ready;
    }
}
