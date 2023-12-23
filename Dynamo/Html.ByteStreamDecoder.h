#pragma once

#include "Html.Types.h"
#include "Html.Node.h"
#include "Html.Token.h"
#include "Basic.Frame.h"
#include "Basic.Types.h"
#include "Basic.MemoryRange.h"
#include "Basic.IDecoder.h"
#include "Basic.MatchFrame.h"

namespace Html
{
	using namespace Basic;

	class Parser;

	class ByteStreamDecoder : public Frame
	{
	private:
		enum State
		{
			start_state = Start_State,
			bom_state,
			media_type_state,
			prescan_state,
			nested_browsing_context_state,
			previous_sniff_state,
			frequency_analysis_state,
			locale_state,
			guess_state,
			sniff_done_state,
			decoding_state,
			done_state = Succeeded_State,
			bom_frame_failed,
			get_decoder_failed,
		};

		UnicodeString::Ref encoding; // $$$
		UnicodeString::Ref transport_charset; // $$$
		Basic::Ref<IStream<Codepoint> > output; // $$$
		Parser* parser;
		byte bom[3];
		ByteString::Ref unconsume; // $$$
		Basic::Ref<IDecoder> decoder; // $$$
		Inline<MemoryRange> bom_frame;
		Confidence confidence;

	public:
		typedef Basic::Ref<ByteStreamDecoder, IProcess> Ref;

		void Initialize(Parser* parser, UnicodeString::Ref transport_charset, IStream<Codepoint>* output);

		virtual void IProcess::Process(IEvent* event, bool* yield);
	};
}