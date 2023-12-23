// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.ServerHelloFrame.h"
#include "Tls.Globals.h"
#include "Basic.ObservableElementSource.h"

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
		this->counter = std::make_shared<CountStream<byte> >();
	}

    void ServerHelloFrame::set_record_frame_length(uint32 record_frame_length)
    {
        this->record_frame_length = record_frame_length;
    }

	ConsumeElementsResult ServerHelloFrame::consume_elements(IElementSource<byte>* element_source)
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

			this->switch_to_state(State::cipher_suite_frame_pending_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::cipher_suite_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->cipher_suite_frame, element_source, this, State::cipher_suite_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			// IANA cipher suites registry
			// http:// www.iana.org/assignments/tls-parameters/tls-parameters.xml#tls-parameters-3

			this->switch_to_state(State::compression_method_frame_pending_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::compression_method_frame_pending_state:
        {
			ConsumeElementsResult result = Basic::consume_elements(&this->compression_method_frame, element_source, this, State::compression_method_frame_failed);
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
            case ExtensionType::heartbeat_extension_type:
                this->switch_to_state(State::heartbeat_extension_frame_pending_state);
				return ConsumeElementsResult::in_progress;

            default:
                this->unknown_extension_frame.reset(this->extension_header.length);
                this->switch_to_state(State::unknown_extension_frame_pending_state);
				return ConsumeElementsResult::in_progress;
			}
        }

        case State::heartbeat_extension_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->heartbeat_extension_frame, element_source, this, State::heartbeat_extension_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->serverHello->heartbeat_extension_initialized = true;

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
            throw FatalError("ServerHelloFrame::handle_event unexpected state");
        }
    }
}
