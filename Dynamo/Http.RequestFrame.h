#pragma once

#include "Basic.IProcess.h"
#include "Basic.ISerializable.h"
#include "Http.Types.h"
#include "Http.BodyFrame.h"
#include "Basic.SingleByteDecoder.h"

namespace Http
{
	class RequestFrame : public Frame, public ISerializable
	{
	private:
		enum State
		{
			receiving_method_state = Start_State,
			receiving_resource_state,
			receiving_protocol_state,
			expecting_LF_after_protocol_state,
			headers_frame_pending_state,
			body_frame_pending_state,
			done_state = Succeeded_State,
			receiving_method_error,
			expecting_LF_after_protocol_error,
			headers_frame_failed,
			body_frame_failed,
		};

		Request* request;
		Inline<SingleByteDecoder> resource_decoder;
		UnicodeString::Ref resource_string; // $$$
		Inline<BodyFrame> body_frame;
		Inline<HeadersFrame> headers_frame;

		bool ParseError(byte b);

	public:
		typedef Basic::Ref<RequestFrame, IProcess> Ref;

		void Initialize(Request* request);
		void WriteRequestLineTo(IStream<byte>* stream);

		virtual void IProcess::Process(IEvent* event, bool* yield);

		virtual void ISerializable::SerializeTo(IStream<byte>* stream);
	};
}