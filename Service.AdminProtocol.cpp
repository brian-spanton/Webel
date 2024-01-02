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
#include "Basic.Event.h"
#include "Scrape.Globals.h"

namespace Service
{
    using namespace Basic;

    AdminProtocol::AdminProtocol(std::shared_ptr<Basic::IStream<Codepoint> > peer) :
        client(std::make_shared<Web::Client>()),
        command_frame(&this->command), // initialization is in order of declaration in class def
        peer(peer)
    {
    }

    void AdminProtocol::reset(std::shared_ptr<Basic::IStream<Codepoint> > peer)
    {
        this->peer = peer;
        this->command.clear();
        this->command_frame.reset();
    }

    ProcessResult AdminProtocol::consider_event(Basic::IEvent* event)
    {
        TextWriter writer(this->peer.get());

        if (event->get_type() == Scrape::EventType::task_complete_event)
        {
			Scrape::TaskCompleteEvent* cookie_event = (Scrape::TaskCompleteEvent*)event;

            // $ this comparison won't work cross-process
			if (cookie_event->cookie.get() == this->amazon_cookie.get())
			{
				this->current_page = this->amazon_scrape->current_page;
				writer.WriteLine("Amazon completed");

                return ProcessResult::process_result_blocked; // event consumed
			}
            // $ this comparison won't work cross-process
			else if (cookie_event->cookie.get() == this->netflix_cookie.get())
			{
				this->current_page = this->netflix_scrape->current_page;
				writer.WriteLine("Netflix completed");

                return ProcessResult::process_result_blocked; // event consumed
			}
			else
			{
                throw FatalError("unexpected completion");
            }
        }
        else if (event->get_type() == Http::EventType::response_headers_event)
        {
            this->html_parser.reset();

            std::shared_ptr<Uri> url;
            this->client->get_url(&url);

            std::shared_ptr<Web::MediaType> content_type;
            this->client->get_content_type(&content_type);

            UnicodeStringRef charset;
            this->client->get_content_type_charset(&charset);

            if (equals<UnicodeString, false>(content_type->type.get(), Http::globals->text_media_type.get()))
            {
                if (equals<UnicodeString, false>(content_type->subtype.get(), Http::globals->html_media_subtype.get()))
                {
                    this->html_parser = std::make_shared<Html::Parser>(url, charset);
                    this->html_parser->tree->document->is_iframe = this->client->transaction->request->is_iframe;
                    this->client->set_decoded_content_stream(this->html_parser);
                }
                else if (equals<UnicodeString, false>(content_type->subtype.get(), Http::globals->plain_media_subtype.get()))
                {
                    std::shared_ptr<IDecoder> decoder;
                    Basic::globals->GetDecoder(charset, &decoder);
                    if (decoder)
                    {
                        decoder->set_destination(this->peer.get());
                        this->client->set_decoded_content_stream(decoder);
                    }
                }
            }

            return ProcessResult::process_result_blocked; // event consumed
        }
        else if (event->get_type() == Http::EventType::response_complete_event)
        {
            Http::ResponseHeadersEvent* cookie_event = (Http::ResponseHeadersEvent*)event;

            if (cookie_event->cookie.get() != this->get_cookie.get())
                throw FatalError("unexpected completion");

            if (this->html_parser.get() != 0)
            {
                this->current_page = std::make_shared<Web::Page>(this->html_parser->tree->document, this->client);

                writer.WriteLine("Get completed with html");
            }
            else 
            {
                writer.WriteLine("Get completed without html");
            }

            return ProcessResult::process_result_blocked; // event consumed
        }
        else switch (get_state())
        {
        case State::start_state:
            this->command.clear();
            this->command_frame.reset();
            switch_to_state(State::command_frame_pending_state);
            break;

        case State::command_frame_pending_state:
            {
                ProcessResult result = delegate_event_throw_error_on_fail(&this->command_frame, event);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                switch_to_state(State::start_state);

                bool handled = false;

                if (this->command.size() == 1 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_stop.get()))
                {
                    writer.WriteLine("stopping");
                    Service::globals->SendStopSignal();
                    handled = true;
                }
                else if (this->command.size() == 1 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_log.get()))
                {
                    Service::globals->tail_log->write_to_stream(this->peer.get());
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
                            writer.WriteLine("Url parse error");
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
                        writer.WriteLine("Http client is busy");
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
                            auto link = this->current_page->links[index];
                            std::shared_ptr<Uri> url = link->url;
                            this->get_cookie = std::make_shared<ByteString>();
                            this->client->http_cookies = this->current_page->http_cookies;
                            this->client->Get(url, 0, this->shared_from_this(), this->get_cookie, link->is_iframe);
                        }
                        else
                        {
                            writer.WriteLine("Http client is busy");
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

                        this->current_form->form_element->write_to_human(this->peer.get(), true);
                        writer.WriteLine();

                        for (uint16 i = 0; i != this->current_form->controls.size(); i++)
                        {
                            writer.WriteFormat<0x10>("%d. ", i);
                            this->current_form->controls[i]->write_html_to_human(this->peer.get());
                            writer.WriteLine();
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
                        writer.WriteLine("Form submit failed");

                    handled = true;
                }
                else if (this->command.size() == 1 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_render_links.get()))
                {
                    if (this->current_page.get() != 0)
                    {
                        writer.WriteLine("Links <l>:");

                        for (uint16 i = 0; i != this->current_page->links.size(); i++)
                        {
                            writer.WriteFormat<0x10>("%d. ", i);
                            this->current_page->links[i]->text->write_to_stream(this->peer.get());
                            writer.write_literal(" [");

                            if (this->current_page->links[i]->url.get() != 0)
                                this->current_page->links[i]->url->write_to_stream(this->peer.get(), false, false);

                            writer.write_literal("]");
                            writer.WriteLine();
                        }
                    }
                    else
                    {
                        writer.WriteLine("No page loaded");
                    }

                    handled = true;
                }
                else if (this->command.size() == 1 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_render_forms.get()))
                {
                    if (this->current_page.get() != 0)
                    {
                        writer.WriteLine("Forms <f>:");

                        for (uint16 i = 0; i != this->current_page->forms.size(); i++)
                        {
                            writer.WriteFormat<0x10>("%d. ", i);
                            this->current_page->forms[i]->form_element->write_html_to_human(this->peer.get());
                            writer.WriteLine();
                        }
                    }
                    else
                    {
                        writer.WriteLine("No page loaded");
                    }

                    handled = true;
                }
                else if (this->command.size() >= 1 && this->command.size() <= 2 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_render_nodes.get()))
                {
                    if (this->current_page.get() != 0)
                    {
                        writer.WriteLine("Nodes:");

                        for (uint16 i = 0; i != this->current_page->leaf_nodes.size(); i++)
                        {
                            writer.WriteFormat<0x10>("%d. ", i);
                            write_to_human_with_context(this->current_page->leaf_nodes[i].get(), this->peer.get(), this->command.size() >= 2);
                            writer.WriteLine();
                        }
                    }
                    else
                    {
                        writer.WriteLine("No page loaded");
                    }

                    handled = true;
                }
                else if (this->command.size() == 2 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_search.get()))
                {
                    std::shared_ptr<Json::Array> results;
                    Scrape::globals->Search(this->command.at(1), &results);

                    results->write_to_stream(this->peer.get());
                    writer.WriteLine();

                    handled = true;
                }
				else if (this->command.size() == 1 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_amazon.get()))
				{
					if (this->amazon_scrape->current_page.get() != 0)
					{
						this->current_page = this->amazon_scrape->current_page;
						writer.WriteLine("Amazon scrape peeked");
					}
					else
					{
						writer.WriteLine("Amazon scrape has no page (yet?)");
					}

					handled = true;
				}
				else if (this->command.size() == 3 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_amazon.get()))
				{
					this->amazon_cookie = std::make_shared<ByteString>();
					this->amazon_scrape = std::make_shared<Scrape::Amazon>(this->command.at(1), this->command.at(2), this->shared_from_this(), this->amazon_cookie);
                    this->amazon_scrape->start();

					writer.WriteLine("Amazon scrape started");
				}
				else if (this->command.size() == 1 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_netflix.get()))
				{
					if (this->netflix_scrape->current_page.get() != 0)
					{
						this->current_page = this->netflix_scrape->current_page;
						writer.WriteLine("Netflix scrape peeked");
					}
					else
					{
						writer.WriteLine("Netflix scrape has no page (yet?)");
					}

					handled = true;
				}
				else if (this->command.size() == 4 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_netflix.get()))
				{
					this->netflix_cookie = std::make_shared<ByteString>();
					this->netflix_scrape = std::make_shared<Scrape::Netflix>(this->command.at(1), this->command.at(2), this->command.at(3), this->shared_from_this(), this->netflix_cookie);
                    this->netflix_scrape->start();

					writer.WriteLine("Netflix scrape started");

					handled = true;
				}

                if (!handled)
                {
                    writer.WriteLine("commands: ");

                    for (Globals::CommandList::iterator it = Service::globals->command_list.begin(); it != Service::globals->command_list.end(); it++)
                    {
                        if (it != Service::globals->command_list.begin())
                            writer.write_literal(", ");

                        this->peer->write_elements((*it)->address(), (*it)->size());
                    }

                    writer.WriteLine();
                }
            }
            break;

        default:
            throw Basic::FatalError("AdminProtocol::handle_event unexpected state");
        }

        return ProcessResult::process_result_ready;
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
                    writer.WriteFormat<0x10>("%d", i);
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