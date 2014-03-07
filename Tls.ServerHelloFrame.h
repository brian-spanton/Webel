// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Basic.IgnoreFrame.h"
#include "Basic.ISerializable.h"
#include "Basic.CountStream.h"
#include "Tls.Types.h"
#include "Tls.SecurityParameters.h"
#include "Tls.RandomFrame.h"
#include "Tls.CipherSuitesFrame.h"
#include "Tls.ExtensionHeaderFrame.h"
#include "Tls.VectorFrames.h"

namespace Tls
{
	using namespace Basic;

	class ServerHelloFrame : public Frame, public ISerializable
	{
	private:
		enum State
		{
			start_state = Start_State,
			version_frame_pending_state,
			random_frame_pending_state,
			session_id_pending_state,
			cipher_suite_frame_pending_state,
			compression_method_frame_pending_state,
			extensions_length_frame_pending_state,
			extension_header_frame_pending_state,
			unknown_extension_frame_pending_state,
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
		};

		uint32 record_frame_length;
		uint32 extensions_length;
		CountStream<byte>::Ref counter; // REF
		ServerHello* serverHello;
		ExtensionHeader extension_header;
		Inline<NumberFrame<uint16> > version_frame;
		Inline<RandomFrame> random_frame;
		Inline<SessionIdFrame> session_id_frame;
		Inline<NumberFrame<CipherSuite> > cipher_suite_frame;
		Inline<NumberFrame<CompressionMethod> > compression_method_frame;
		Inline<NumberFrame<uint32, sizeof(uint16)> > extensions_length_frame;
		Inline<ExtensionHeaderFrame> extension_header_frame;
		Inline<IgnoreFrame> unknown_extension_frame;

		void switch_to_state(IEvent* event, State state);

	public:
		typedef Basic::Ref<ServerHelloFrame, IProcess> Ref;

		void Initialize(ServerHello* serverHello, uint32 record_frame_length);

		virtual void IProcess::Process(IEvent* event, bool* yield);

		virtual void ISerializable::SerializeTo(IStream<byte>* stream);
	};
}