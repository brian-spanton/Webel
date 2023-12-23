#pragma once

#include "Basic.IProcess.h"
#include "Basic.CountStream.h"
#include "Basic.IgnoreFrame.h"
#include "Tls.Types.h"
#include "Tls.RandomFrame.h"
#include "Tls.ExtensionHeaderFrame.h"
#include "Tls.ServerNameListFrame.h"
#include "Tls.CipherSuitesFrame.h"
#include "Tls.CertificateStatusRequestFrame.h"
#include "Tls.EllipticCurveListFrame.h"
#include "Tls.ECPointFormatListFrame.h"

namespace Tls
{
	using namespace Basic;

	class ClientHelloFrame : public Frame, public ISerializable
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
		};

		uint32 record_frame_length;
		uint32 extensions_length;
		CountStream<byte>::Ref counter; // $$$
		ClientHello* clientHello;
		ExtensionHeader extension_header;
		Inline<NumberFrame<uint16>> version_frame;
		Inline<RandomFrame> random_frame;
		Inline<SessionIdFrame> session_id_frame;
		Inline<CipherSuitesFrame> cipher_suites_frame;
		Inline<CompressionMethodsFrame> compression_methods_frame;
		Inline<NumberFrame<uint32, sizeof(uint16)> > extensions_length_frame;
		Inline<ExtensionHeaderFrame> extension_header_frame;
		Inline<ServerNameListFrame> server_name_list_frame;
		Inline<SignatureAndHashAlgorithmsFrame> supported_signature_algorithms_frame;
		Inline<RenegotiationInfoFrame> renegotiation_info_frame;
		Inline<CertificateStatusRequestFrame> certificate_status_request_frame;
		Inline<EllipticCurveListFrame> elliptic_curve_list_frame;
		Inline<ECPointFormatListFrame> ec_point_format_list_frame;
		Inline<IgnoreFrame> unknown_extension_frame;

		void switch_to_state(IEvent* event, State state);

	public:
		typedef Basic::Ref<ClientHelloFrame, IProcess> Ref;

		void Initialize(ClientHello* clientHello, uint32 record_frame_length);
		virtual void IProcess::Process(IEvent* event, bool* yield);
		virtual void ISerializable::SerializeTo(IStream<byte>* stream);
	};
}