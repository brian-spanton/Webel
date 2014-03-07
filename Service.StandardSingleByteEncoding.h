// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Frame.h"
#include "Web.Client.h"
#include "Basic.SingleByteEncodingIndex.h"
#include "Json.Types.h"

namespace Service
{
	using namespace Basic;

	class StandardSingleByteEncoding : public Frame
	{
	private:
		enum State
		{
			line_start_state = Start_State,
			ignore_line_state,
			before_index_state,
			index_pending_state,
			before_codepoint_state,
			codepoint_pending_state,
			done_state = Succeeded_State,
			before_index_error,
			before_codepoint_error,
			line_start_error,
			index_pending_error,
			codepoint_pending_error,
		};

		Web::Client::Ref client; // REF
		Json::Value::Ref json_value; // REF
		byte pointer;
		Codepoint codepoint;
		Inline<DecNumberStream<byte, byte> > pointer_stream;
		Inline<HexNumberStream<byte, Codepoint> > codepoint_stream;
		SingleByteEncodingIndex::Ref index;

	public:
		typedef Ref<StandardSingleByteEncoding, IProcess> Ref;

		void Initialize(Http::Uri* index_url, SingleByteEncodingIndex::Ref index);

		virtual void IProcess::Process(IEvent* event, bool* yield);
	};
}