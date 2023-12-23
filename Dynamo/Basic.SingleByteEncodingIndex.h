#pragma once

#include "Basic.ISingleByteEncodingIndex.h"
#include "Basic.ICompletion.h"
#include "Basic.IDecoder.h"
#include "Basic.Ref.h"
#include "Html.Types.h"
#include "Http.Types.h"
#include "Http.Client.h"
#include "Basic.DecNumberStream.h"
#include "Basic.HexNumberStream.h"

namespace Basic
{
	class SingleByteEncodingIndex : public Frame, public ISingleByteEncodingIndex
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

		typedef std::unordered_map<Codepoint, byte> CodepointMap;

		Http::Client::Ref client; // $$$
		Json::Value::Ref json_value; // $$$
		byte index;
		Codepoint codepoint;
		Inline<DecNumberStream<byte, byte> > index_stream;
		Inline<HexNumberStream<byte, Codepoint> > codepoint_stream;
		Codepoint index_map[0x80];
		CodepointMap codepoint_map;

	public:
		typedef Ref<SingleByteEncodingIndex, ISingleByteEncodingIndex> Ref;

		void Initialize(Http::Uri* index_url);

		virtual void IProcess::Process(IEvent* event, bool* yield);
		virtual Codepoint ISingleByteEncodingIndex::byte_to_codepoint(byte b);
		virtual byte ISingleByteEncodingIndex::codepoint_to_byte(Codepoint c);
	};
}