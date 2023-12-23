// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Service.AdminProtocol.h"
#include "Basic.TextWriter.h"
#include "Web.Client.h"
#include "Http.Globals.h"
#include "Service.Globals.h"
#include "Html.Parser.h"
#include "Html.Globals.h"
#include "Web.Form.h"
#include "Basic.TextSanitizer.h"
#include "Service.Types.h"
#include "Scrape.Globals.h"

namespace Service
{
    using namespace Basic;

    AdminProtocol::AdminProtocol(std::shared_ptr<Basic::IStream<Codepoint> > transport) :
        client(std::make_shared<Web::Client>()),
        command_frame(&this->command), // initialization is in order of declaration in class def
        transport(transport)
    {
    }

    void AdminProtocol::transport_connected()
    {
        TextWriter writer(this->transport.get());
		writer.write_line("Hello admin!");
    }

    void AdminProtocol::transport_disconnected()
    {
    }

    void AdminProtocol::transport_received(const Codepoint* elements, uint32 count)
    {
    }
    
    void AdminProtocol::tbd_event() // $$$
    {
        TextWriter writer(this->transport.get());

        if (event->get_type() == Scrape::EventType::task_complete_event)
        {
			Scrape::TaskCompleteEvent* cookie_event = (Scrape::TaskCompleteEvent*)event;

            // $ pointer comparison won't work cross-process
			if (cookie_event->cookie.get() == this->amazon_cookie.get())
			{
				this->current_page = this->amazon_scrape->current_page;
				writer.write_line("Amazon completed");

                throw Yield("event consumed");
			}
            // $ this comparison won't work cross-process
			else if (cookie_event->cookie.get() == this->netflix_cookie.get())
			{
				this->current_page = this->netflix_scrape->current_page;
				writer.write_line("Netflix completed");

                throw Yield("event consumed");
			}
			else
			{
                throw FatalError("unexpected completion");
            }
        }
        else if (event->get_type() == Http::EventType::response_headers_event)
        {
            std::shared_ptr<Uri> url;
            this->client->get_url(&url);

            UnicodeStringRef charset;
            this->client->get_content_type_charset(&charset);

            this->html_parser = std::make_shared<Html::Parser>(url, charset);

            this->client->set_body_stream(this->html_parser);

            throw Yield("event consumed");
        }
        else if (event->get_type() == Http::EventType::response_complete_event)
        {
            Http::ResponseCompleteEvent* cookie_event = (Http::ResponseCompleteEvent*)event;

            if (cookie_event->cookie.get() != this->get_cookie.get())
                throw FatalError("unexpected completion");

            if (this->html_parser.get() != 0)
            {
                this->current_page = std::make_shared<Web::Page>(this->html_parser->tree->document, this->client);

                writer.write_line("Get completed");
            }
            else
            {
                writer.write_line("Get failed to produce html");
            }

            throw Yield("event consumed");
        }
    }

    bool AdminProtocol::receiver_received(ElementSource<Codepoint>* elements)
    {
        TextWriter writer(this->transport.get());

        switch (get_state())
        {
        case State::start_state:
            this->command.clear();
            this->command_frame.reset();
            switch_to_state(State::command_frame_pending_state);
            break;

        case State::command_frame_pending_state:
            {
                this->command_frame.transport_received(elements);

                if (this->command_frame.in_progress())
                    return;
                
                if (this->command_frame.failed())
                    throw FatalError("command_frame failed");

                switch_to_state(State::start_state);

                bool handled = false;

                if (this->command.size() == 1 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_stop.get()))
                {
                    writer.write_line("stopping");
                    Service::globals->SendStopSignal();
                    handled = true;
                }
                else if (this->command.size() == 1 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_log.get()))
                {
                    Service::globals->tail_log->write_to_stream(this->transport.get());
                    handled = true;
                }
                else if (this->command.size() == 2 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_get.get()))
                {
                    if (this->client->get_state() == Web::Client::State::inactive_state)
                    {
                        std::shared_ptr<Uri> url = std::make_shared<Basic::Uri>();
                        url->Initialize();
                        
                        bool success = url->Parse(this->command[1].get(), (Uri*)0);
                        if (!success)
                        {
                            writer.write_line("Url parse error");
                        }
                        else
                        {
                            this->get_cookie = std::make_shared<ByteString>();

                            if (this->current_page.get() != 0)
                                this->client->http_cookies = this->current_page->http_cookies;

                            this->client->Get(url, 0, this->shared_from_this(), this->get_cookie);
                        }
                    }
                    else
                    {
                        writer.write_line("Http client is busy");
                    }

                    handled = true;
                }
                else if (this->command.size() == 2 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_follow_link.get()))
                {
                    bool all_digits;
                    byte index = this->command.at(1)->as_base_10<byte>(&all_digits);
                    if (all_digits && index < this->current_page->links.size())
                    {
                        if (this->client->get_state() == Web::Client::State::inactive_state)
                        {
                            std::shared_ptr<Uri> url = this->current_page->links[index]->url;
                            this->get_cookie = std::make_shared<ByteString>();
                            this->client->http_cookies = this->current_page->http_cookies;
                            this->client->Get(url, 0, this->shared_from_this(), this->get_cookie);
                        }
                        else
                        {
                            writer.write_line("Http client is busy");
                        }

                        handled = true;
                    }
                }
                else if (this->command.size() == 2 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_select_form.get()))
                {
                    bool all_digits;
                    byte index = this->command.at(1)->as_base_10<byte>(&all_digits);
                    if (all_digits && index < this->current_page->forms.size())
                    {
                        this->current_form = this->current_page->forms[index];

                        this->current_form->form_element->write_to_human(this->transport.get(), true);
                        writer.write_line();

                        for (uint16 i = 0; i != this->current_form->controls.size(); i++)
                        {
                            writer.write_format<0x10>("%d. ", i);
                            this->current_form->controls[i]->write_html_to_human(this->transport.get());
                            writer.write_line();
                        }

                        handled = true;
                    }
                }
                else if (this->command.size() >= 2 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_set_control_value.get()))
                {
                    bool all_digits;
                    byte index = this->command.at(1)->as_base_10<byte>(&all_digits);
                    if (all_digits && index < this->current_form->controls.size())
                    {
                        UnicodeStringRef value = std::make_shared<UnicodeString>();
                        value->reserve(0x40);

                        for (uint32 i = 2; i < this->command.size(); i++)
                        {
                            if (value->length() > 0)
                                value->push_back(' ');

                            value->append(this->command[i]->begin(), this->command[i]->end());
                        }

                        this->current_form->controls[index]->set_attribute(Html::globals->value_attribute_name, value);

                        handled = true;
                    }
                }
                else if (this->command.size() == 1 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_submit.get()))
                {
                    this->client->http_cookies = this->current_page->http_cookies;

                    bool success = this->current_form->Submit(this->client.get(), this->shared_from_this(), 0);
                    if (!success)
                        writer.write_line("Form submit failed");

                    handled = true;
                }
                else if (this->command.size() == 1 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_render_links.get()))
                {
                    if (this->current_page.get() != 0)
                    {
                        writer.write_line("Links <l>:");

                        for (uint16 i = 0; i != this->current_page->links.size(); i++)
                        {
                            writer.write_format<0x10>("%d. ", i);
                            this->current_page->links[i]->text->write_to_stream(this->transport.get());
                            writer.write_literal(" [");

                            if (this->current_page->links[i]->url.get() != 0)
                                this->current_page->links[i]->url->write_to_stream(this->transport.get(), false, false);

                            writer.write_literal("]");
                            writer.write_line();
                        }
                    }
                    else
                    {
                        writer.write_line("No page loaded");
                    }

                    handled = true;
                }
                else if (this->command.size() == 1 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_render_forms.get()))
                {
                    if (this->current_page.get() != 0)
                    {
                        writer.write_line("Forms <f>:");

                        for (uint16 i = 0; i != this->current_page->forms.size(); i++)
                        {
                            writer.write_format<0x10>("%d. ", i);
                            this->current_page->forms[i]->form_element->write_html_to_human(this->transport.get());
                            writer.write_line();
                        }
                    }
                    else
                    {
                        writer.write_line("No page loaded");
                    }

                    handled = true;
                }
                else if (this->command.size() >= 1 && this->command.size() <= 2 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_render_nodes.get()))
                {
                    if (this->current_page.get() != 0)
                    {
                        writer.write_line("Nodes:");

                        for (uint16 i = 0; i != this->current_page->leaf_nodes.size(); i++)
                        {
                            writer.write_format<0x10>("%d. ", i);
                            write_to_human_with_context(this->current_page->leaf_nodes[i].get(), this->transport.get(), this->command.size() >= 2);
                            writer.write_line();
                        }
                    }
                    else
                    {
                        writer.write_line("No page loaded");
                    }

                    handled = true;
                }
                else if (this->command.size() == 2 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_search.get()))
                {
                    std::shared_ptr<Json::Array> results;
                    Scrape::globals->Search(this->command.at(1), &results);

                    results->write_to_stream(this->transport.get());
                    writer.write_line();

                    handled = true;
                }
				else if (this->command.size() == 1 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_amazon.get()))
				{
					if (this->amazon_scrape->current_page.get() != 0)
					{
						this->current_page = this->amazon_scrape->current_page;
						writer.write_line("Amazon scrape peeked");
					}
					else
					{
						writer.write_line("Amazon scrape has no page (yet?)");
					}

					handled = true;
				}
				else if (this->command.size() == 3 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_amazon.get()))
				{
					this->amazon_cookie = std::make_shared<ByteString>();
					this->amazon_scrape = std::make_shared<Scrape::Amazon>(this->command.at(1), this->command.at(2), this->shared_from_this(), this->amazon_cookie);
                    this->amazon_scrape->start();

					writer.write_line("Amazon scrape started");
				}
				else if (this->command.size() == 1 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_netflix.get()))
				{
					if (this->netflix_scrape->current_page.get() != 0)
					{
						this->current_page = this->netflix_scrape->current_page;
						writer.write_line("Netflix scrape peeked");
					}
					else
					{
						writer.write_line("Netflix scrape has no page (yet?)");
					}

					handled = true;
				}
				else if (this->command.size() == 4 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_netflix.get()))
				{
					this->netflix_cookie = std::make_shared<ByteString>();
					this->netflix_scrape = std::make_shared<Scrape::Netflix>(this->command.at(1), this->command.at(2), this->command.at(3), this->shared_from_this(), this->netflix_cookie);
                    this->netflix_scrape->start();

					writer.write_line("Netflix scrape started");

					handled = true;
				}

                if (!handled)
                {
                    writer.write_line("commands: ");

                    for (Globals::CommandList::iterator it = Service::globals->command_list.begin(); it != Service::globals->command_list.end(); it++)
                    {
                        if (it != Service::globals->command_list.begin())
                            writer.write_literal(", ");

                        this->transport->write_elements((*it)->address(), (*it)->size());
                    }

                    writer.write_line();
                }
            }
            break;

        default:
            throw Basic::FatalError("AdminProtocol::handle_event unexpected state");
        }
    }
    
    void AdminProtocol::write_to_human_with_context(Html::Node* node, IStream<Codepoint>* stream, bool verbose)
    {
        std::shared_ptr<Html::Node> parent = node->parent.lock();
        if (parent.get() != 0)
            write_to_human_with_context(parent.get(), stream, verbose);

        TextWriter writer(stream);
        writer.write_literal("/");

        if (parent.get() != 0)
        {
            bool found = false;

            for (uint32 i = 0; i < parent->children.size(); i++)
            {
                if (parent->children.at(i).get() == node)
                {
                    writer.write_format<0x10>("%d", i);
                    found = true;
                }
            }

            if (!found)
            {
                writer.write_literal("?");
            }

            writer.write_literal(".");
        }

        TextSanitizer clean_stream;
        clean_stream.Initialize(stream);

        node->write_to_human(&clean_stream, verbose);
    }

}