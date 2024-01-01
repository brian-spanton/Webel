#include "stdafx.h"
#include "Dynamo.AmazonScrape.h"
#include "Basic.TextWriter.h"
#include "Basic.AsyncBytes.h"
#include "Basic.Ref.h"
#include "Http.Client.h"
#include "Http.Globals.h"
#include "Dynamo.Globals.h"
#include "Html.Parser.h"
#include "Html.Globals.h"
#include "Web.Form.h"
#include "Dynamo.Types.h"

namespace Dynamo
{
	using namespace Basic;

	void AmazonScrape::Initialize(IProcess* client_completion, ByteString* client_cookie)
	{
		__super::Initialize();

		switch_to_state(State::movies_pending_state);

		this->client = New<Http::Client>();
		this->client->Initialize();

		this->client_completion = client_completion;
		this->client_cookie = client_cookie;

		this->client->Get(Dynamo::globals->amazon_url, this, (ByteString*)0);
	}

	void AmazonScrape::load_doc()
	{
		Web::Page::Ref page = New<Web::Page>();
		page->Initialize(this->html_parser->tree->document, this->client);

		this->current_page = page;
	}

	bool AmazonScrape::set_body_stream()
	{
		Http::Response::Ref response = this->client->history.back().response;
		if (response->code != 200)
		{
			Uri::Ref url;
			this->client->get_url(&url);

			url->SerializeTo(Dynamo::globals->DebugStream(), 0, 0);
			Dynamo::globals->DebugWriter()->WriteLine(" did not return 200");

			return false;
		}

		this->html_parser = New<Html::Parser>();
		this->html_parser->Initialize(this->client);

		this->client->set_body_stream(this->html_parser);

		return true;
	}

	void AmazonScrape::Error(const char* error)
	{
		HandleError(error);
	}

	void AmazonScrape::Process(Basic::IEvent* event, bool* yield)
	{
		Hold hold(this->lock);

		(*yield) = true;

		switch (frame_state())
		{
		case State::anon_home_page_pending_state:
			switch (event->get_type())
			{
			case Http::EventType::response_headers_event:
				{
					bool success = set_body_stream();
					if (!success)
					{
						Error("anon home page");
						Complete();
						switch_to_state(State::done_state);
						return;
					}
				}
				break;

			case Http::EventType::response_complete_event:
				{
					load_doc();

					Web::Link::Ref link;
					bool success = this->current_page->find_link(Dynamo::globals->amazon_sign_in_link, &link);
					if (!success)
					{
						Error("anon home page");
						Complete();
						switch_to_state(State::done_state);
						return;
					}

					this->client->Get(link->url, this, (ByteString*)0);
					switch_to_state(State::sign_in_page_pending_state);
				}
				break;
				
			default:
				throw new Exception("unexpected event type");
			}
			break;

		case State::sign_in_page_pending_state:
			switch (event->get_type())
			{
			case Http::EventType::response_headers_event:
				{
					bool success = set_body_stream();
					if (!success)
					{
						Error("sign in page");
						Complete();
						switch_to_state(State::done_state);
						return;
					}
				}
				break;

			case Http::EventType::response_complete_event:
				{
					load_doc();

					Web::Form::Ref form;
					bool success = this->current_page->find_form(Dynamo::globals->amazon_sign_in_form, &form);
					if (!success)
					{
						Error("sign in page");
						Complete();
						switch_to_state(State::done_state);
						return;
					}

					Html::ElementNode::Ref email_control;
					success = form->find_control(Dynamo::globals->amazon_email_control, &email_control);
					if (!success)
					{
						Error("sign in page");
						Complete();
						switch_to_state(State::done_state);
						return;
					}

					UnicodeString::Ref email;
					email.Initialize("jen@spanton.net");
					form->set_control_value(email_control, email);

					Html::ElementNode::Ref password_control;
					success = form->find_control(Dynamo::globals->amazon_password_control, &password_control);
					if (!success)
					{
						Error("sign in page");
						Complete();
						switch_to_state(State::done_state);
						return;
					}

					UnicodeString::Ref password;
					password.Initialize("amazon219");
					form->set_control_value(password_control, password);

					success = form->Submit(this->client, this, 0);
					if (!success)
					{
						Error("sign in page");
						Complete();
						switch_to_state(State::done_state);
						return;
					}

					switch_to_state(State::auth_landing_page_pending_state);
				}
				break;

			default:
				throw new Exception("unexpected event type");
			}
			break;

		case State::auth_landing_page_pending_state:
			switch (event->get_type())
			{
			case Http::EventType::response_headers_event:
				{
					bool success = set_body_stream();
					if (!success)
					{
						Error("auth landing page");
						Complete();
						switch_to_state(State::done_state);
						return;
					}
				}
				break;

			case Http::EventType::response_complete_event:
				{
					load_doc();

					Web::Link::Ref link;
					bool success = this->current_page->find_link(Dynamo::globals->amazon_prime_link, &link);
					if (!success)
					{
						Error("auth landing page");
						Complete();
						switch_to_state(State::done_state);
						return;
					}

					this->client->Get(link->url, this, (ByteString*)0);
					switch_to_state(State::your_prime_pending_state);
				}
				break;

			default:
				throw new Exception("unexpected event type");
			}
			break;

		case State::your_prime_pending_state:
			switch (event->get_type())
			{
			case Http::EventType::response_headers_event:
				{
					bool success = set_body_stream();
					if (!success)
					{
						Error("your prime");
						Complete();
						switch_to_state(State::done_state);
						return;
					}
				}
				break;

			case Http::EventType::response_complete_event:
				{
					load_doc();

					Web::Link::Ref link;
					bool success = this->current_page->find_link(Dynamo::globals->amazon_browse_link, &link);
					if (!success)
					{
						Error("auth home page");
						Complete();
						switch_to_state(State::done_state);
						return;
					}

					this->client->Get(link->url, this, (ByteString*)0);
					switch_to_state(State::instant_video_pending_state);
				}
				break;

			default:
				throw new Exception("unexpected event type");
			}
			break;

		case State::instant_video_pending_state:
			switch (event->get_type())
			{
			case Http::EventType::response_headers_event:
				{
					bool success = set_body_stream();
					if (!success)
					{
						Error("instant video");
						Complete();
						switch_to_state(State::done_state);
						return;
					}
				}
				break;

			case Http::EventType::response_complete_event:
				{
					load_doc();

					Web::Link::Ref link;
					bool success = this->current_page->find_link(Dynamo::globals->amazon_movies_link, &link);
					if (!success)
					{
						Error("instant video");
						Complete();
						switch_to_state(State::done_state);
						return;
					}

					this->client->Get(link->url, this, (ByteString*)0);
					switch_to_state(State::movies_pending_state);
				}
				break;

			default:
				throw new Exception("unexpected event type");
			}
			break;

		case State::movies_pending_state:
			switch (event->get_type())
			{
			case Http::EventType::response_headers_event:
				{
					bool success = set_body_stream();
					if (!success)
					{
						Error("movies");
						Complete();
						switch_to_state(State::done_state);
						return;
					}
				}
				break;

			case Http::EventType::response_complete_event:
				{
					load_doc();

					// commence scraping
					Scrape(this->html_parser->tree->document);

					Web::Link::Ref link;
					bool success = this->current_page->find_link(Dynamo::globals->amazon_next_page_link, &link);
					if (!success)
					{
						Complete();
						switch_to_state(State::done_state);
						return;
					}

					this->client->Get(link->url, this, (ByteString*)0);
				}
				break;

			default:
				throw new Exception("unexpected event type");
			}
			break;

		default:
			throw new Basic::Exception("AmazonScrape::Process unexpected state");
		}
	}

	void AmazonScrape::Scrape(Html::Node* node)
	{
		UnicodeString::Ref json_script;
		json_script.Initialize(
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

		Json::Parser::Ref parser = New<Json::Parser>();
		parser->Initialize(node, Basic::globals->utf_32_little_endian_label);

		parser->Write((byte*)json_script->c_str(), json_script->size() * sizeof(Codepoint));
		parser->WriteEOF();

		Dynamo::globals->Store(Dynamo::globals->amazon_source_name, parser->text->value);
	}

	void AmazonScrape::Complete()
	{
		Basic::Ref<IProcess> completion = this->client_completion;
		this->client_completion = 0;

		TaskCompleteEvent event;
		event.cookie = this->client_cookie;
		this->client_cookie = 0;

		if (completion.item() != 0)
			completion->Process(&event);
	}
}