#include "stdafx.h"
#include "Scrape.Amazon.h"
#include "Scrape.Globals.h"
#include "Json.Parser.h"

namespace Scrape
{
	using namespace Basic;

	Amazon::Amazon(UnicodeStringRef name, UnicodeStringRef password, std::shared_ptr<IProcess> completion, std::shared_ptr<void> context) :
		client(std::make_shared<Web::Client>()),
		completion(completion),
		context(context),
        name(name),
        password(password)
	{
	}

    void Amazon::start()
    {
		this->client->Get(Scrape::globals->amazon_url, 0, this->shared_from_this(), ByteStringRef());
    }

	void Amazon::load_doc()
	{
		this->current_page = std::make_shared<Web::Page>(this->html_parser->tree->document, this->client);
	}

	bool Amazon::set_body_stream()
	{
		std::shared_ptr<Http::Response> response = this->client->transaction->response;
		if (response->code != 200)
		{
			std::shared_ptr<Uri> url;
			this->client->get_url(&url);

			std::shared_ptr<LogEntry> entry = std::make_shared<LogEntry>(LogLevel::Debug, "Scrape", "Amazon", "set_body_stream");
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

	ProcessResult Amazon::process_event(IEvent* event)
	{
		Hold hold(this->lock);

        try
        {
            return process_event_throw(event);
        }
        catch (State error_state)
        {
		    Basic::LogDebug("Scrape", "Amazon", "process_event", "caught error", error_state);
		    switch_to_state(error_state);
		    Complete();
			return ProcessResult::process_result_exited;
        }
    }

	ProcessResult Amazon::process_event_throw(IEvent* event)
	{
		switch (this->get_state())
		{
		case State::anon_home_page_headers_state:
            {
                if (event->get_type() != Http::EventType::response_headers_event)
                    throw State::anon_home_page_headers_unexpected_event_error;

                bool success = set_body_stream();
                if (!success)
                    throw State::anon_home_page_headers_error;

                switch_to_state(State::anon_home_page_body_state);
                return ProcessResult::process_result_blocked;
            }
			break;

        case State::anon_home_page_body_state:
            {
                if (event->get_type() != Http::EventType::response_complete_event)
                    throw State::anon_home_page_body_unexpected_event_error;

				load_doc();

				std::shared_ptr<Web::Link> link;
				bool success = this->current_page->find_link(Scrape::globals->amazon_sign_in_link, &link);
				if (!success)
                    throw State::sign_in_link_error;

				this->client->Get(link->url, 0, this->shared_from_this(), ByteStringRef());

				switch_to_state(State::sign_in_page_headers_state);
                return ProcessResult::process_result_blocked;
			}
			break;

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
				bool success = this->current_page->find_form(Scrape::globals->amazon_sign_in_form, &form);
				if (!success)
                    throw State::sign_in_form_error;

				std::shared_ptr<Html::ElementNode> email_control;
				success = form->find_control(Scrape::globals->amazon_email_control, &email_control);
				if (!success)
                    throw State::email_control_error;

				form->set_control_value(email_control.get(), this->name);

				std::shared_ptr<Html::ElementNode> password_control;
				success = form->find_control(Scrape::globals->amazon_password_control, &password_control);
				if (!success)
                    throw State::password_control_error;

				form->set_control_value(password_control.get(), this->password);

				success = form->Submit(this->client.get(), this->shared_from_this(), ByteStringRef());
				if (!success)
                    throw State::sign_in_submit_error;

				switch_to_state(State::auth_landing_page_headers_state);
                return ProcessResult::process_result_blocked;
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

				load_doc();

				std::shared_ptr<Web::Link> link;
				bool success = this->current_page->find_link(Scrape::globals->amazon_prime_link, &link);
				if (!success)
                    throw State::prime_link_error;

				this->client->Get(link->url, 0, this->shared_from_this(), ByteStringRef());

				switch_to_state(State::your_prime_page_headers_state);
                return ProcessResult::process_result_blocked;
			}
			break;

		case State::your_prime_page_headers_state:
            {
                if (event->get_type() != Http::EventType::response_headers_event)
                    throw State::your_prime_page_headers_unexpected_event_error;

                bool success = set_body_stream();
                if (!success)
                    throw State::your_prime_page_headers_error;

                switch_to_state(State::your_prime_page_body_state);
                return ProcessResult::process_result_blocked;
            }
            break;

		case State::your_prime_page_body_state:
            {
                if (event->get_type() != Http::EventType::response_complete_event)
                    throw State::your_prime_page_body_unexpected_event_error;

				load_doc();

				std::shared_ptr<Web::Link> link;
				bool success = this->current_page->find_link(Scrape::globals->amazon_browse_link, &link);
				if (!success)
                    throw State::browse_link_error;

				this->client->Get(link->url, 0, this->shared_from_this(), ByteStringRef());

				switch_to_state(State::instant_video_page_headers_state);
                return ProcessResult::process_result_blocked;
			}
			break;

		case State::instant_video_page_headers_state:
            {
                if (event->get_type() != Http::EventType::response_headers_event)
                    throw State::instant_video_page_headers_unexpected_event_error;

                bool success = set_body_stream();
                if (!success)
                    throw State::instant_video_page_headers_error;

                switch_to_state(State::instant_video_page_body_state);
                return ProcessResult::process_result_blocked;
            }
            break;

		case State::instant_video_page_body_state:
            {
                if (event->get_type() != Http::EventType::response_complete_event)
                    throw State::instant_video_page_body_unexpected_event_error;

				load_doc();

				std::shared_ptr<Web::Link> link;
				bool success = this->current_page->find_link(Scrape::globals->amazon_movies_link, &link);
				if (!success)
                    throw State::movies_link_error;

				this->client->Get(link->url, 0, this->shared_from_this(), ByteStringRef());

				switch_to_state(State::movies_page_headers_state);
                return ProcessResult::process_result_blocked;
			}
			break;

		case State::movies_page_headers_state:
            {
                if (event->get_type() != Http::EventType::response_headers_event)
                    throw State::movies_page_headers_unexpected_event_error;

                bool success = set_body_stream();
                if (!success)
                    throw State::movies_page_headers_error;

                switch_to_state(State::movies_page_body_state);
                return ProcessResult::process_result_blocked;
            }
            break;

		case State::movies_page_body_state:
            {
                if (event->get_type() != Http::EventType::response_complete_event)
                    throw State::movies_page_body_unexpected_event_error;

				load_doc();

				// commence scraping
				Scrape(this->html_parser->tree->document);

				std::shared_ptr<Web::Link> link;
				bool success = this->current_page->find_link(Scrape::globals->amazon_next_page_link, &link);
				if (!success)
                {
                    switch_to_state(State::done_state);
                    Complete();
                    return ProcessResult::process_result_exited;
                }

				this->client->Get(link->url, 0, this->shared_from_this(), ByteStringRef());

				switch_to_state(State::movies_page_headers_state);
                return ProcessResult::process_result_blocked;
			}
			break;

		default:
			throw FatalError("Scrape", "Amazon", "process_event_throw", "unhandled state", this->get_state());
		}

        return ProcessResult::process_result_ready;
	}

	void Amazon::Scrape(std::shared_ptr<Html::Node> node)
	{
		ByteStringRef json_script;
		initialize_ascii(&json_script, 
			"<div.id.equals(\"btfResults\")>"
			"["
			"	<li.id.starts_with(\"result_\")>"
			"	{"
			"		<span.class.equals(\"lrg bold\")>"
			"		\"title\" : <.first_text()>,"

			"		<span.class.equals(\"med reg subt\")>"
			"		\"date\" : <.first_text()>,"

			"		<>"
			"		\"img\" : <img.src>,"

			"		\"source\" : \"Amazon Instant Video\","

			"		\"offers\" :"
			"		["
			"			<tr.children_count_equals(3)>"
			"			{"
			"				<td.class.equals(\"clabel\")>"
			"				\"label\" : <.deep_text()>,"

			"				<td.class.equals(\"cpriceR\")>"
			"				\"price\" : <.deep_text()>"
			"			}"
			"		]"
			"	}"
			"]");

		std::shared_ptr<Json::Parser> parser = std::make_shared<Json::Parser>(node, Basic::globals->utf_32_little_endian_label);

		parser->write_elements(json_script->address(), json_script->size());
		parser->write_eof();

		Scrape::globals->Store(Scrape::globals->amazon_source_name, parser->text->value);
	}

	void Amazon::Complete()
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