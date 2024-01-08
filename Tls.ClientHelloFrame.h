// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Basic.CountStream.h"
#include "Basic.IgnoreFrame.h"
#include "Tls.Types.h"
#include "Tls.RandomFrame.h"
#include "Tls.ExtensionHeaderFrame.h"
#include "Tls.CertificateStatusRequestFrame.h"
#include "Tls.HeartbeatExtensionFrame.h"

namespace Tls
{
    using namespace Basic;

    class ClientHelloFrame : public Frame
    {
    private:
        enum State
        {
            start_state = Start_State,
            version_frame_pending_state,
            random_frame_pending_state,
            session_id_frame_pending_state,
            cipher_suites_frame_pending_state,
            compression_methods_frame_pending_state,
            extensions_length_frame_pending_state,
            extension_header_frame_pending_state,
            server_name_list_frame_pending_state,
            supported_signature_algorithms_frame_pending_state,
            renegotiation_info_frame_pending_state,
            certificate_status_request_frame_pending_state,
            elliptic_curve_list_frame_pending_state,
            ec_point_format_list_frame_pending_state,
            unknown_extension_frame_pending_state,
            heartbeat_extension_frame_pending_state,
            next_extension_state,
            done_state = Succeeded_State,
            version_frame_failed,
            random_frame_failed,
            session_id_frame_failed,
            cipher_suites_frame_failed,
            compression_methods_frame_failed,
            record_frame_length_error,
            extensions_length_frame_failed,
            extension_header_frame_failed,
            extensions_length_error,
            server_name_list_frame_failed,
            supported_signature_algorithms_frame_failed,
            renegotiation_info_frame_failed,
            certificate_status_request_frame_failed,
            elliptic_curve_list_frame_failed,
            ec_point_format_list_frame_failed,
            unknown_extension_frame_failed,
            heartbeat_extension_frame_failed,
        };

        uint32 record_frame_length;
        uint16 extensions_length;
        std::shared_ptr<CountStream<byte> > counter;
        ClientHello* clientHello;
        ExtensionHeader extension_header;
        NumberFrame<uint16> version_frame;
        RandomFrame random_frame;
        VectorFrame<SessionId> session_id_frame;
        VectorFrame<CipherSuites> cipher_suites_frame;
        VectorFrame<CompressionMethods> compression_methods_frame;
        NumberFrame<uint16> extensions_length_frame;
        ExtensionHeaderFrame extension_header_frame;
        VectorFrame<ServerNameList> server_name_list_frame;
        VectorFrame<SignatureAndHashAlgorithms> supported_signature_algorithms_frame;
        VectorFrame<RenegotiationInfo> renegotiation_info_frame;
        CertificateStatusRequestFrame certificate_status_request_frame;
        VectorFrame<EllipticCurveList> elliptic_curve_list_frame;
        VectorFrame<ECPointFormatList> ec_point_format_list_frame;
        IgnoreFrame<byte> unknown_extension_frame;
        HeartbeatExtensionFrame heartbeat_extension_frame;

        void switch_to_state(IEvent* event, State state);
        virtual ProcessResult IProcess::process_event(IEvent* event);

    public:
        ClientHelloFrame(ClientHello* clientHello);

        void set_record_frame_length(uint32 record_frame_length);
    };
}

namespace Basic
{
    template <>
    struct __declspec(novtable) serialize<Tls::ClientHello>
    {
        void operator()(const Tls::ClientHello* value, IStream<byte>* stream) const
        {
            CountStream<byte> count_stream;
            SerializeExtensionsTo(value, &count_stream);

            serialize<Tls::ProtocolVersion>()(&value->client_version, stream);
            serialize<Tls::Random>()(&value->random, stream);
            serialize<Tls::SessionId>()(&value->session_id, stream);
            serialize<Tls::CipherSuites>()(&value->cipher_suites, stream);
            serialize<Tls::CompressionMethods>()(&value->compression_methods, stream);

            if (count_stream.count > 0xffff)
                throw FatalError("Tls", "serialize<Tls::ClientHello>::operator() { count_stream.count > 0xffff }");

            uint16 extensions_length = (uint16)count_stream.count;

            if (extensions_length > 0)
            {
                serialize<uint16>()(&extensions_length, stream);
                SerializeExtensionsTo(value, stream);
            }
        }

        void SerializeExtensionsTo(const Tls::ClientHello* value, IStream<byte>* stream) const
        {
            if (value->heartbeat_extension_initialized)
            {
                CountStream<byte> count_stream;
                serialize<Tls::HeartbeatExtension>()(&value->heartbeat_extension, &count_stream);

                Tls::ExtensionHeader extension_header;
                extension_header.length = (uint16)count_stream.count;
                extension_header.type = Tls::ExtensionType::heartbeat_extension_type;
                serialize<Tls::ExtensionHeader>()(&extension_header, stream);

                serialize<Tls::HeartbeatExtension>()(&value->heartbeat_extension, stream);
            }
        }
    };
}