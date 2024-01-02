#pragma once

#include "Basic.Frame.h"
#include "Web.Client.h"
#include "Html.Parser.h"
#include "Web.Page.h"

namespace Scrape
{
    using namespace Basic;

	class Netflix : public Frame, public std::enable_shared_from_this<Netflix>
	{
	private:
		enum State
		{
			sign_in_page_headers_state = Start_State,
            sign_in_page_body_state,
            auth_landing_page_headers_state,
            auth_landing_page_body_state,
            auth_home_page_headers_state,
            auth_home_page_body_state,
            search_paging_headers_state,
            search_paging_body_state,
            click_results_state,
            movie_page_headers_state,
            movie_page_body_state,
			done_state = Succeeded_State,
            sign_in_page_headers_unexpected_event_error,
            sign_in_page_headers_error,
            sign_in_page_body_unexpected_event_error,
            logon_form_error,
            email_control_error,
            password_control_error,
            logon_form_submit_error,
            auth_landing_page_headers_unexpected_event_error,
            auth_landing_page_headers_error,
            auth_landing_page_body_unexpected_event_error,
            auth_home_page_headers_unexpected_event_error,
            auth_home_page_headers_error,
            auth_home_page_body_unexpected_event_error,
            search_form_error,
            query2_control_error,
            form_submit_error,
            search_paging_headers_unexpected_event_error,
            search_paging_headers_error,
            search_paging_body_unexpected_event_error,
            movie_page_headers_unexpected_event_error,
            movie_page_headers_error,
            movie_page_body_unexpected_event_error,
		};

		Lock lock;
		std::shared_ptr<Web::Client> client;
		std::shared_ptr<Html::Parser> html_parser;
        std::weak_ptr<IProcess> completion;
        ByteStringRef completion_cookie;
		std::vector<UnicodeStringRef> movies;
		UnicodeStringRef search_term;
		uint32 current_row = 0;
		std::shared_ptr<Uri> next_page;
        UnicodeStringRef name;
        UnicodeStringRef password;

		void load_doc();
		void Complete();
		void ScrapeMovies(std::shared_ptr<Uri>* next_page, uint32* next_row);
		void ScrapeMovie(std::shared_ptr<Html::Node> node);
		bool set_body_stream();
		void Error(const char* error);
		ProcessResult consider_event_throw(IEvent* event);

	public:
		std::shared_ptr<Web::Page> current_page;

		Netflix(UnicodeStringRef name, UnicodeStringRef password, UnicodeStringRef search_term, std::shared_ptr<IProcess> completion, ByteStringRef completion_cookie);

        void start();
		virtual ProcessResult IProcess::consider_event(IEvent* event);
	};
}