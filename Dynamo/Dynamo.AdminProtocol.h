#pragma once

#include "Basic.Ref.h"
#include "Basic.CommandFrame.h"
#include "Basic.IProcess.h"
#include "Basic.IBufferedStream.h"
#include "Basic.NameValueCollection.h"
#include "Http.Client.h"
#include "Html.Document.h"
#include "Html.Types.h"
#include "Basic.Lock.h"
#include "Web.Page.h"
#include "Web.Form.h"
#include "Dynamo.AmazonScrape.h"
#include "Dynamo.NetflixSearchScrape.h"
#include "Basic.IStream.h"

namespace Dynamo
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
		std::vector<Basic::UnicodeString::Ref> command; // $$$
		Basic::Inline<Basic::CommandFrame> command_frame;
		Basic::Ref<Basic::IStream<Codepoint> > peer; // $$$
		Http::Client::Ref client; // $$$
		Html::Parser::Ref html_parser; // $$$
		Web::Page::Ref current_page; // $$$
		Web::Form::Ref current_form; // $$$
		Basic::ByteString::Ref amazon_cookie; // $$$
		Basic::ByteString::Ref netflix_cookie; // $$$
		Basic::ByteString::Ref get_cookie; // $$$
		AmazonScrape::Ref amazon_scrape; // $$$
		NetflixSearchScrape::Ref netflix_scrape; // $$$

		void write_to_human_with_context(Html::Node* node, Basic::IStream<Codepoint>* stream, bool verbose);

	public:
		typedef Basic::Ref<AdminProtocol, IProcess> Ref;

		void Initialize(Basic::IStream<Codepoint>* peer);
		void set_peer(Basic::IStream<Codepoint>* peer);

		virtual void Basic::IProcess::Process(Basic::IEvent* event, bool* yield);
	};
}