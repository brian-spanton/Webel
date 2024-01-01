#include "stdafx.h"
#include "Dynamo.NetflixSearchScrape.h"
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

	void NetflixSearchScrape::Initialize(UnicodeString::Ref search_term, IProcess* client_completion, ByteString* client_cookie)
	{
		__super::Initialize();

		switch_to_state(State::sign_in_page_pending_state);

		this->client = New<Http::Client>();
		this->client->Initialize();

		this->client_completion = client_completion;
		this->client_cookie = client_cookie;

		this->search_term = search_term;

		this->client->Get(Dynamo::globals->netflix_url, this, (Basic::ByteString*)0);
	}

	void NetflixSearchScrape::load_doc()
	{
		Web::Page::Ref page = New<Web::Page>();
		page->Initialize(this->html_parser->tree->document, this->client);

		this->current_page = page;
	}

	bool NetflixSearchScrape::set_body_stream()
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

	void NetflixSearchScrape::Error(const char* error)
	{
		HandleError(error);
	}

	void NetflixSearchScrape::Process(Basic::IEvent* event, bool* yield)
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
					bool success = this->current_page->find_link(Dynamo::globals->netflix_sign_in_link, &link);
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
					bool success = this->current_page->find_form(Dynamo::globals->netflix_logon_form, &form);
					if (!success)
					{
						Error("sign in page");
						Complete();
						switch_to_state(State::done_state);
						return;
					}

					Html::ElementNode::Ref email_control;
					success = form->find_control(Dynamo::globals->netflix_email_control, &email_control);
					if (!success)
					{
						Error("sign in page");
						Complete();
						switch_to_state(State::done_state);
						return;
					}

					UnicodeString::Ref email;
					email.Initialize("jen_sea@hotmail.com");
					form->set_control_value(email_control, email);

					Html::ElementNode::Ref password_control;
					success = form->find_control(Dynamo::globals->netflix_password_control, &password_control);
					if (!success)
					{
						Error("sign in page");
						Complete();
						switch_to_state(State::done_state);
						return;
					}

					UnicodeString::Ref password;
					password.Initialize("netflix219");
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
					this->client->Get(Dynamo::globals->netflix_url, this, (ByteString*)0);
					switch_to_state(State::auth_home_page_pending_state);
				}
				break;

			default:
				throw new Exception("unexpected event type");
			}
			break;

		case State::auth_home_page_pending_state:
			switch (event->get_type())
			{
			case Http::EventType::response_headers_event:
				{
					bool success = set_body_stream();
					if (!success)
					{
						Error("auth home page");
						Complete();
						switch_to_state(State::done_state);
						return;
					}
				}
				break;

			case Http::EventType::response_complete_event:
				{
					load_doc();

					this->current_row = 0;
					ScrapeMovies(0, 0);

					Web::Form::Ref form;
					bool success = this->current_page->find_form(Dynamo::globals->netflix_search_form, &form);
					if (!success)
					{
						Error("auth home page");
						Complete();
						switch_to_state(State::done_state);
						return;
					}

					Html::ElementNode::Ref query1_control;
					success = form->find_control(Dynamo::globals->netflix_query1_control, &query1_control);
					if (success)
					{
						query1_control->remove_attribute(Html::globals->disabled_attribute_name);
						form->set_control_value(query1_control, this->search_term);
					}

					Html::ElementNode::Ref query2_control;
					success = form->find_control(Dynamo::globals->netflix_query2_control, &query2_control);
					if (!success)
					{
						Error("auth home page");
						Complete();
						switch_to_state(State::done_state);
						return;
					}

					query2_control->remove_attribute(Html::globals->disabled_attribute_name);
					form->set_control_value(query2_control, this->search_term);

					success = form->Submit(this->client, this, 0);
					if (!success)
					{
						Error("auth home page");
						Complete();
						switch_to_state(State::done_state);
						return;
					}

					switch_to_state(State::search_paging_state);
				}
				break;

			default:
				throw new Exception("unexpected event type");
			}
			break;

		case State::search_paging_state:
			switch (event->get_type())
			{
			case Http::EventType::response_headers_event:
				{
					bool success = set_body_stream();
					if (!success)
					{
						Error("search paging");
						Complete();
						switch_to_state(State::done_state);
						return;
					}
				}
				break;

			case Http::EventType::response_complete_event:
				{
					load_doc();

					uint32 next_row;

					this->next_page = 0;
					ScrapeMovies(&this->next_page, &next_row);

					if (this->next_page.item() == 0)
					{
						Complete();
						switch_to_state(State::done_state);
						return;
					}

					this->current_row = next_row;

					switch_to_state(State::click_results_state);
					(*yield) = false;
				}
				break;

			default:
				throw new Exception("unexpected event type");
			}
			break;

		case State::click_results_state:
			{
				if (this->movies.size() == 0)
				{
					this->client->Get(this->next_page, this, (ByteString*)0);
					switch_to_state(State::search_paging_state);
					break;
				}

				UnicodeString::Ref movie_id = this->movies.back();
				this->movies.pop_back();

				Basic::Uri::Ref movie_url = New<Basic::Uri>();
				movie_url->Initialize(Dynamo::globals->netflix_movie_url);
				movie_url->path.push_back(movie_id);

				this->client->Get(movie_url, this, (ByteString*)0);

				switch_to_state(State::pending_movie_page_state);
			}
			break;

		case State::pending_movie_page_state:
			switch (event->get_type())
			{
			case Http::EventType::response_headers_event:
				{
					bool success = set_body_stream();
					if (!success)
					{
						Error("pending_movie_page_state");
						Complete();
						switch_to_state(State::done_state);
						return;
					}
				}
				break;

			case Http::EventType::response_complete_event:
				{
					load_doc();

					ScrapeMovie(this->html_parser->tree->document);

					switch_to_state(State::click_results_state);
					(*yield) = false;
				}
				break;

			default:
				throw new Exception("unexpected event type");
			}
			break;

		default:
			throw new Basic::Exception("NetflixSearchScrape::Process unexpected state");
		}
	}

	void NetflixSearchScrape::ScrapeMovies(Basic::Uri::Ref* next_page, uint32* next_row)
	{
		Dynamo::globals->DebugWriter()->WriteFormat<0x100>("Row %d+", this->current_row);
		Dynamo::globals->DebugWriter()->WriteLine();

		for (uint32 i = 0; i < this->current_page->links.size(); i++)
		{
			Web::Link::Ref link = this->current_page->links.at(i);

			if (link->url.item() != 0)
			{
				if (link->url->query.is_null_or_empty() == false)
				{
					Basic::StringMap::Ref pairs = New<StringMap>();

					Web::Form::url_decode(link->url->query, (UnicodeString*)0, false, pairs);

					Basic::StringMap::iterator it = pairs->find(Dynamo::globals->netflix_movieid_param);
					if (it != pairs->end())
					{
						this->movies.push_back(it->second);
					}

					if (next_page != 0 && next_row != 0 && link->url->path.size() == 1 && link->url->path.front().equals<false>(Dynamo::globals->netflix_search_path))
					{
						Basic::StringMap::iterator it = pairs->find(Dynamo::globals->netflix_row_param);
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

	void NetflixSearchScrape::ScrapeMovie(Html::Node* node)
	{
		UnicodeString::Ref json_script;
		json_script.Initialize(
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

		Json::Parser::Ref parser = New<Json::Parser>();
		parser->Initialize(node, Basic::globals->utf_32_little_endian_label);

		parser->Write((byte*)json_script->c_str(), json_script->size() * sizeof(Codepoint));
		parser->WriteEOF();

		Dynamo::globals->Store(Dynamo::globals->amazon_source_name, parser->text->value);
	}

	void NetflixSearchScrape::Complete()
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