#pragma once

#include "Basic.Frame.h"
#include "Web.Client.h"
#include "Html.Parser.h"
#include "Web.Page.h"

namespace Scrape
{
    using namespace Basic;

	class Amazon : public Frame, public std::enable_shared_from_this<Amazon>
	{
	private:
		enum State
		{
            anon_home_page_headers_state = Start_State,
            anon_home_page_body_state,
            sign_in_page_headers_state,
            sign_in_page_body_state,
            auth_landing_page_headers_state,
            auth_landing_page_body_state,
            your_prime_page_headers_state,
            your_prime_page_body_state,
            instant_video_page_headers_state,
            instant_video_page_body_state,
            movies_page_headers_state,
            movies_page_body_state,
			done_state = Succeeded_State,
            anon_home_page_headers_unexpected_event_error,
            anon_home_page_headers_error,
            anon_home_page_body_unexpected_event_error,
            sign_in_link_error,
            sign_in_page_headers_unexpected_event_error,
            sign_in_page_headers_error,
            sign_in_page_body_unexpected_event_error,
            sign_in_form_error,
            email_control_error,
            password_control_error,
            sign_in_submit_error,
            auth_landing_page_headers_unexpected_event_error,
            auth_landing_page_headers_error,
            auth_landing_page_body_unexpected_event_error,
            prime_link_error,
            your_prime_page_headers_unexpected_event_error,
            your_prime_page_headers_error,
            your_prime_page_body_unexpected_event_error,
            browse_link_error,
            instant_video_page_headers_unexpected_event_error,
            instant_video_page_headers_error,
            instant_video_page_body_unexpected_event_error,
            movies_link_error,
            movies_page_headers_unexpected_event_error,
            movies_page_headers_error,
            movies_page_body_unexpected_event_error,
            next_page_link_error,
        };

		Lock lock;
		std::shared_ptr<Web::Client> client;
		std::shared_ptr<Html::Parser> html_parser;
        std::weak_ptr<IProcess> call_back;
        std::shared_ptr<void> context;
        UnicodeStringRef name;
        UnicodeStringRef password;

		void load_doc();
		void Complete();
		void Scrape(std::shared_ptr<Html::Node> node);
		bool set_body_stream();
		ProcessResult process_event_throw(IEvent* event);

	public:
		std::shared_ptr<Web::Page> current_page;

		Amazon(UnicodeStringRef name, UnicodeStringRef password, std::shared_ptr<IProcess> call_back, std::shared_ptr<void> context);

        void start();
		virtual ProcessResult IProcess::process_event(IEvent* event);
	};
}