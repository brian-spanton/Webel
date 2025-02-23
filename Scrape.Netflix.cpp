#include "stdafx.h"
#include "Scrape.Netflix.h"
#include "Scrape.Globals.h"
#include "Html.Globals.h"
#include "Json.Parser.h"

namespace Scrape
{
	using namespace Basic;

	Netflix::Netflix(UnicodeStringRef name, UnicodeStringRef password, UnicodeStringRef search_term, std::shared_ptr<IProcess> completion, std::shared_ptr<void> context) :
		client(std::make_shared<Web::Client>()),
		completion(completion),
		context(context),
		search_term(search_term),
        name(name),
        password(password)
	{
	}

    void Netflix::start()
    {
        // add a retry because netflix sometimes just arbitrarily(?) declines to serve
		this->client->Get(Scrape::globals->netflix_url, 1, this->shared_from_this(), ByteStringRef());
	}

	void Netflix::load_doc()
	{
		this->current_page = std::make_shared<Web::Page>(this->html_parser->tree->document, this->client);
	}

	bool Netflix::set_body_stream()
	{
		std::shared_ptr<Http::Response> response = this->client->transaction->response;
		if (response->code != 200)
		{
			std::shared_ptr<Uri> url;
			this->client->get_url(&url);

			std::shared_ptr<LogEntry> entry = std::make_shared<LogEntry>(LogLevel::Debug, "Scrape", "Netflix", "set_body_stream");
			url->write_to_stream(&entry->unicode_message, 0, 0);
			TextWriter(&entry->unicode_message).write_literal(" did not return 200");
			Basic::globals->add_entry(entry);

			return false;
		}

        std::shared_ptr<Uri> url;
        this->client->get_url(&url);

        UnicodeStringRef charset;
        this->client->get_content_type_charset(&charset);

        this->html_parser = std::make_shared<Html::Parser>(url, charset);
        this->client->set_decoded_content_stream(this->html_parser);

		return true;
	}

	ProcessResult Netflix::process_event(IEvent* event)
	{
		Hold hold(this->lock);

        try
        {
            return process_event_throw(event);
        }
        catch (State error_state)
        {
		    Basic::LogDebug("Scrape", "Netflix", "process_event", "caught error", error_state);
		    switch_to_state(error_state);
		    Complete();
			return ProcessResult::process_result_exited;
        }
    }

	ProcessResult Netflix::process_event_throw(IEvent* event)
	{
		switch (this->get_state())
		{
		case State::sign_in_page_headers_state:
            {
                if (event->get_type() != Http::EventType::response_headers_event)
                    throw State::sign_in_page_headers_unexpected_event_error;

                bool success = set_body_stream();
                if (!success)
                    throw State::sign_in_page_headers_error;

                switch_to_state(State::sign_in_page_body_state);
                return ProcessResult::process_result_blocked;
            }
			break;

        case State::sign_in_page_body_state:
            {
                if (event->get_type() != Http::EventType::response_complete_event)
                    throw State::sign_in_page_body_unexpected_event_error;

				load_doc();

				std::shared_ptr<Web::Form> form;
				bool success = this->current_page->find_form(Scrape::globals->netflix_logon_form, &form);
				if (!success)
                    throw State::logon_form_error;

				std::shared_ptr<Html::ElementNode> email_control;
				success = form->find_control(Scrape::globals->netflix_email_control, &email_control);
				if (!success)
                    throw State::email_control_error;

				form->set_control_value(email_control.get(), this->name);

				std::shared_ptr<Html::ElementNode> password_control;
				success = form->find_control(Scrape::globals->netflix_password_control, &password_control);
                    throw State::password_control_error;

				form->set_control_value(password_control.get(), this->password);

				success = form->Submit(this->client.get(), this->shared_from_this(), ByteStringRef());
				if (!success)
                    throw State::logon_form_submit_error;

				switch_to_state(State::auth_landing_page_headers_state);
			}
			break;

		case State::auth_landing_page_headers_state:
            {
                if (event->get_type() != Http::EventType::response_headers_event)
                    throw State::auth_landing_page_headers_unexpected_event_error;

                bool success = set_body_stream();
                if (!success)
                    throw State::auth_landing_page_headers_error;

                switch_to_state(State::auth_landing_page_body_state);
                return ProcessResult::process_result_blocked;
            }
            break;

        case State::auth_landing_page_body_state:
            {
                if (event->get_type() != Http::EventType::response_complete_event)
                    throw State::auth_landing_page_body_unexpected_event_error;

				this->client->Get(Scrape::globals->netflix_url, 0, this->shared_from_this(), ByteStringRef());

				switch_to_state(State::auth_landing_page_headers_state);
                return ProcessResult::process_result_blocked;
			}
			break;

		case State::auth_home_page_headers_state:
            {
                if (event->get_type() != Http::EventType::response_headers_event)
                    throw State::auth_home_page_headers_unexpected_event_error;

                bool success = set_body_stream();
                if (!success)
                    throw State::auth_home_page_headers_error;

                switch_to_state(State::auth_home_page_body_state);
                return ProcessResult::process_result_blocked;
            }
            break;

        case State::auth_home_page_body_state:
			{
                if (event->get_type() != Http::EventType::response_complete_event)
                    throw State::auth_home_page_body_unexpected_event_error;

				load_doc();

				this->current_row = 0;
				ScrapeMovies(0, 0);

				std::shared_ptr<Web::Form> form;
				bool success = this->current_page->find_form(Scrape::globals->netflix_search_form, &form);
				if (!success)
                    throw State::search_form_error;

				std::shared_ptr<Html::ElementNode> query1_control;
				success = form->find_control(Scrape::globals->netflix_query1_control, &query1_control);
				if (success)
				{
					query1_control->remove_attribute(Html::globals->disabled_attribute_name);
					form->set_control_value(query1_control.get(), this->search_term);
				}

				std::shared_ptr<Html::ElementNode> query2_control;
				success = form->find_control(Scrape::globals->netflix_query2_control, &query2_control);
				if (!success)
                    throw State::query2_control_error;

				query2_control->remove_attribute(Html::globals->disabled_attribute_name);
				form->set_control_value(query2_control.get(), this->search_term);

				success = form->Submit(this->client.get(), this->shared_from_this(), ByteStringRef());
				if (!success)
                    throw State::form_submit_error;

				switch_to_state(State::search_paging_headers_state);
                return ProcessResult::process_result_blocked;
			}
			break;

		case State::search_paging_headers_state:
            {
                if (event->get_type() != Http::EventType::response_headers_event)
                    throw State::search_paging_headers_unexpected_event_error;

                bool success = set_body_stream();
                if (!success)
                    throw State::search_paging_headers_error;

                switch_to_state(State::search_paging_body_state);
                return ProcessResult::process_result_blocked;
            }
            break;

        case State::search_paging_body_state:
			{
                if (event->get_type() != Http::EventType::response_complete_event)
                    throw State::search_paging_body_unexpected_event_error;

				load_doc();

				uint32 next_row;

				this->next_page = 0;
				ScrapeMovies(&this->next_page, &next_row);

				if (!this->next_page)
				{
					switch_to_state(State::done_state);
					Complete();
					return ProcessResult::process_result_blocked;
				}

				this->current_row = next_row;

				switch_to_state(State::click_results_state);
			}
			break;

		case State::click_results_state:
			{
				if (this->movies.size() == 0)
				{
					switch_to_state(State::search_paging_headers_state);
					this->client->Get(this->next_page, 0, this->shared_from_this(), ByteStringRef());
					return ProcessResult::process_result_blocked;
				}

				UnicodeStringRef movie_id = this->movies.back();
				this->movies.pop_back();

				std::shared_ptr<Basic::Uri> movie_url = std::make_shared<Basic::Uri>();
				movie_url->Initialize(Scrape::globals->netflix_movie_url.get());
				movie_url->path.push_back(movie_id);

				this->client->Get(movie_url, 0, this->shared_from_this(), ByteStringRef());

				switch_to_state(State::movie_page_headers_state);
                return ProcessResult::process_result_blocked;
			}
			break;

		case State::movie_page_headers_state:
            {
                if (event->get_type() != Http::EventType::response_headers_event)
                    throw State::movie_page_headers_unexpected_event_error;

                bool success = set_body_stream();
                if (!success)
                    throw State::movie_page_headers_error;

                switch_to_state(State::movie_page_body_state);
                return ProcessResult::process_result_blocked;
            }
			break;

		case State::movie_page_body_state:
			{
                if (event->get_type() != Http::EventType::response_complete_event)
                    throw State::movie_page_body_unexpected_event_error;

			    load_doc();

			    ScrapeMovie(this->html_parser->tree->document);

			    switch_to_state(State::click_results_state);
		    }
		    break;

		default:
			throw FatalError("Scrape", "Netflix", "process_event_throw", "unhandled state", this->get_state());
		}

		return ProcessResult::process_result_ready;
	}

	void Netflix::ScrapeMovies(std::shared_ptr<Basic::Uri>* next_page, uint32* next_row)
	{
		char message[0x100];
		sprintf_s(message, "Row %d+", this->current_row);
		Basic::LogInfo("Scrape", "Netflix", "ScrapeMovies", message);

		for (uint32 i = 0; i < this->current_page->links.size(); i++)
		{
			std::shared_ptr<Web::Link> link = this->current_page->links.at(i);

			if (link->url)
			{
				if (is_null_or_empty(link->url->query.get()) == false)
				{
					std::shared_ptr<Basic::StringMap> pairs = std::make_shared<StringMap>();

					Web::Form::url_decode(link->url->query, UnicodeStringRef(), false, pairs.get());

					Basic::StringMap::iterator it = pairs->find(Scrape::globals->netflix_movieid_param);
					if (it != pairs->end())
					{
						this->movies.push_back(it->second);
					}

					if (next_page != 0 && next_row != 0 && link->url->path.size() == 1 && equals<UnicodeString, false>(link->url->path.front().get(), Scrape::globals->netflix_search_path.get()))
					{
						Basic::StringMap::iterator it = pairs->find(Scrape::globals->netflix_row_param);
						if (it != pairs->end())
						{
							uint32 row = it->second->as_base_10<uint32>(0);
							if (row > this->current_row)
							{
								(*next_row) = row;
								(*next_page) = link->url;
							}
						}
					}
				}
			}
		}
	}

	void Netflix::ScrapeMovie(std::shared_ptr<Html::Node> node)
	{
		ByteStringRef json_script;
		initialize_ascii(&json_script, 
			"	<div.id.equals(\"mdp-overview\")>"
			"	{"
			"		<h2.class.equals(\"title\")>"
			"		\"title\" : <.first_text()>,"

			"		<span.class.equals(\"year\")>"
			"		\"date\" : <.first_text()>,"

			"		<div.id.equals(\"mdp-boxshot\")>"
			"		\"img\" : <img.src>,"

			"		\"source\" : \"Netflix\","

			"		<div.id.equals(\"mdp-metadata-content\")>"
			"		\"offers\" :"
			"		["
			"			<a.text_equals(\"DVD\")>"
			"			{"
			"				\"label\" : \"DVD\","
			"				\"price\" : null"
			"			},"
			"			<a.text_equals(\"streaming\")>"
			"			{"
			"				\"label\" : \"streaming\","
			"				\"price\" : null"
			"			}"
			"		]"
			"	}");

		std::shared_ptr<Json::Parser> parser = std::make_shared<Json::Parser>(node, Basic::globals->utf_32_little_endian_label);

		parser->write_elements(json_script->address(), json_script->size());
		parser->write_eof();

		Scrape::globals->Store(Scrape::globals->amazon_source_name, parser->text->value);
	}

	void Netflix::Complete()
	{
		std::shared_ptr<IProcess> completion = this->completion.lock();
        if (completion)
        {
            TaskCompleteEvent event;
            event.context = this->context;
            process_event_ignore_failures(completion.get(), &event);
        }
	}
}