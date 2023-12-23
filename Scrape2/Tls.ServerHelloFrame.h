// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IgnoreFrame.h"
#include "Basic.CountStream.h"
#include "Tls.Types.h"
#include "Tls.SecurityParameters.h"
#include "Tls.RandomFrame.h"
#include "Tls.ExtensionHeaderFrame.h"
#include "Tls.HeartbeatExtensionFrame.h"

namespace Tls
{
    using namespace Basic;

	class ServerHelloFrame : public StateMachine, public IElementConsumer<byte>
    {
    private:
        enum State
        {
            start_state = Start_State,
            version_frame_pending_state,
            random_frame_pending_state,
            session_id_frame_pending_state,
            cipher_suite_frame_pending_state,
            compression_method_frame_pending_state,
            extensions_length_frame_pending_state,
            extension_header_frame_pending_state,
            unknown_extension_frame_pending_state,
            heartbeat_extension_frame_pending_state,
            next_extension_state,
            done_state = Succeeded_State,
            version_frame_failed,
            random_frame_failed,
            session_id_frame_failed,
            cipher_suite_frame_failed,
            compression_method_frame_failed,
            record_frame_length_error,
            extensions_length_frame_failed,
            extension_header_frame_failed,
            extensions_length_error,
            unknown_extension_frame_failed,
            heartbeat_extension_frame_failed,
        };

        uint32 record_frame_length;
        uint16 extensions_length;
        std::shared_ptr<CountStream<byte> > counter;
        ServerHello* serverHello;
        ExtensionHeader extension_header;
        NumberFrame<uint16> version_frame;
        RandomFrame random_frame;
        VectorFrame<SessionId> session_id_frame;
        NumberFrame<CipherSuite> cipher_suite_frame;
        NumberFrame<CompressionMethod> compression_method_frame;
        NumberFrame<uint16> extensions_length_frame;
        ExtensionHeaderFrame extension_header_frame;
        IgnoreFrame<byte> unknown_extension_frame;
        HeartbeatExtensionFrame heartbeat_extension_frame;

    public:
        ServerHelloFrame(ServerHello* serverHello);

        void set_record_frame_length(uint32 record_frame_length);

		ConsumeElementsResult IElementConsumer<byte>::consume_elements(IElementSource<byte>* element_source);
	};

    template <>
    struct __declspec(novtable) serialize<ServerHello>
    {
        void operator()(const ServerHello* value, IStream<byte>* stream) const
        {
            serialize<uint16>()(&value->server_version, stream);
            serialize<Random>()(&value->random, stream);
            serialize<SessionId>()(&value->session_id, stream);
            serialize<CipherSuite>()(&value->cipher_suite, stream);
            serialize<CompressionMethod>()(&value->compression_method, stream);
        }
    };
}