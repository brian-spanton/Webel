// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Ref.h"
#include "Basic.CommandFrame.h"
#include "Basic.IProcess.h"
#include "Basic.IBufferedStream.h"
#include "Basic.NameValueCollection.h"
#include "Web.Client.h"
#include "Html.Document.h"
#include "Html.Types.h"
#include "Html.Parser.h"
#include "Basic.Lock.h"
#include "Web.Page.h"
#include "Web.Form.h"
#include "Basic.IStream.h"

namespace Service
{
	class AdminProtocol : public Basic::Frame
	{
	private:
		enum State
		{
			start_state = Start_State,
			command_frame_pending_state,
			done_state = Succeeded_State,
			peer_write_failed,
			peer_flush_failed,
		};

		Basic::Lock lock;
		std::vector<Basic::UnicodeString::Ref> command; // REF
		Basic::Inline<Basic::CommandFrame> command_frame;
		Basic::Ref<Basic::IStream<Codepoint> > peer; // REF
		Web::Client::Ref client; // REF
		Html::Parser::Ref html_parser; // REF
		Web::Page::Ref current_page; // REF
		Web::Form::Ref current_form; // REF
		Basic::ByteString::Ref get_cookie; // REF

		void write_to_human_with_context(Html::Node* node, Basic::IStream<Codepoint>* stream, bool verbose);

	public:
		typedef Basic::Ref<AdminProtocol, IProcess> Ref;

		void Initialize(Basic::IStream<Codepoint>* peer);
		void set_peer(Basic::IStream<Codepoint>* peer);

		virtual void Basic::IProcess::Process(Basic::IEvent* event, bool* yield);
	};
}