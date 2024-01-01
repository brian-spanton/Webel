#pragma once

#include "Basic.Ref.h"
#include "Basic.CommandFrame.h"
#include "Basic.IProcess.h"
#include "Basic.IBufferedStream.h"
#include "Basic.NameValueCollection.h"
#include "Http.Client.h"
#include "Html.Parser.h"
#include "Html.Types.h"
#include "Html.ElementNode.h"
#include "Basic.Lock.h"
#include "Web.Page.h"
#include "Web.Form.h"
#include "Web.Link.h"

namespace Dynamo
{
	class AmazonScrape : public Basic::Frame
	{
	private:
		enum State
		{
			anon_home_page_pending_state = Start_State,
			sign_in_page_pending_state,
			auth_landing_page_pending_state,
			your_prime_pending_state,
			instant_video_pending_state,
			movies_pending_state,
			done_state = Succeeded_State,
		};

		Basic::Lock lock;
		Http::Client::Ref client; // $$$
		Html::Parser::Ref html_parser; // $$$
		Basic::Ref<IProcess> client_completion; // $$$
		Basic::ByteString::Ref client_cookie; // $$$

		void load_doc();
		void Complete();
		void Scrape(Html::Node* node);
		bool set_body_stream();
		void Error(const char* error);

	public:
		typedef Basic::Ref<AmazonScrape, IProcess> Ref;

		Web::Page::Ref current_page; // $$$

		void Initialize(IProcess* client_completion, Basic::ByteString* client_cookie);

		virtual void Basic::IProcess::Process(Basic::IEvent* event, bool* yield);
	};
}