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
        command_list.push_back(Service::globals->command_stop);
        command_list.push_back(Service::globals->command_log);
        command_list.push_back(Service::globals->command_get);
        command_list.push_back(Service::globals->command_follow_link);
        command_list.push_back(Service::globals->command_select_form);
        command_list.push_back(Service::globals->command_set_control_value);
        command_list.push_back(Service::globals->command_submit);
        command_list.push_back(Service::globals->command_render_links);
        command_list.push_back(Service::globals->command_render_forms);
        command_list.push_back(Service::globals->command_render_nodes);
        command_list.push_back(Service::globals->command_search);
		command_list.push_back(Scrape::globals->command_amazon);
		command_list.push_back(Scrape::globals->command_netflix);
    }

    void AdminProtocol::reset(std::shared_ptr<Basic::IStream<Codepoint> > peer)
    {
        this->peer = peer;
        this->command.clear();
        this->command_frame.reset();
    }

    ProcessResult AdminProtocol::process_event(Basic::IEvent* event)
    {
        TextWriter writer(this->peer.get());

        if (event->get_type() == Scrape::EventType::task_complete_event)
        {
			Scrape::ContextualizedEvent* contextualized_event = (Scrape::ContextualizedEvent*)event;

            // $ this comparison won't work cross-process
			if (contextualized_event->context.get() == this->amazon_context.get())
			{
				this->current_page = this->amazon_scrape->current_page;
				writer.WriteLine("Amazon completed");

                return ProcessResult::process_result_blocked;
			}
            // $ this comparison won't work cross-process
			else if (contextualized_event->context.get() == this->netflix_context.get())
			{
				this->current_page = this->netflix_scrape->current_page;
				writer.WriteLine("Netflix completed");

                return ProcessResult::process_result_blocked;
			}
			else
			{
                throw FatalError("Service", "AdminProtocol", "process_event", "unexpected completion");
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

            return ProcessResult::process_result_blocked;
        }
        else if (event->get_type() == Http::EventType::response_complete_event)
        {
            Basic::ContextualizedEvent* contextualized_event = (Basic::ContextualizedEvent*)event;

            if (contextualized_event->context.get() != this->get_context.get())
                throw FatalError("Service", "AdminProtocol", "process_event", "contextualized_event->context.get() != this->get_context.get()");

            if (this->html_parser)
            {
                this->current_page = std::make_shared<Web::Page>(this->html_parser->tree->document, this->client);

                writer.WriteLine("Get completed with html");
            }
            else
            {
                writer.WriteLine("Get completed (not html)");
            }

            return ProcessResult::process_result_blocked;
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
                ProcessResult result = process_event_throw_error_on_fail(&this->command_frame, event);
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
                            this->get_context = std::make_shared<ByteString>();
                            this->html_parser.reset();

                            if (this->current_page)
                                this->client->http_cookies = this->current_page->http_cookies;

                            this->client->Get(url, 0, this->shared_from_this(), this->get_context);
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
                            this->get_context = std::make_shared<ByteString>();
                            this->client->http_cookies = this->current_page->http_cookies;
                            this->client->Get(url, 0, this->shared_from_this(), this->get_context, link->is_iframe);
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
                    this->get_context = std::make_shared<ByteString>();
                    this->html_parser.reset();

                    this->client->http_cookies = this->current_page->http_cookies;

                    bool success = this->current_form->Submit(this->client.get(), this->shared_from_this(), this->get_context);
                    if (!success)
                        writer.WriteLine("Form submit failed");

                    handled = true;
                }
                else if (this->command.size() == 1 && equals<UnicodeString, false>(this->command.at(0).get(), Service::globals->command_render_links.get()))
                {
                    if (this->current_page)
                    {
                        writer.WriteLine("Links <l>:");

                        for (uint16 i = 0; i != this->current_page->links.size(); i++)
                        {
                            writer.WriteFormat<0x10>("%d. ", i);
                            this->current_page->links[i]->text->write_to_stream(this->peer.get());
                            writer.write_literal(" [");

                            if (this->current_page->links[i]->url)
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
                    if (this->current_page)
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
                    if (this->current_page)
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
				else if (this->command.size() == 1 && equals<UnicodeString, false>(this->command.at(0).get(), Scrape::globals->command_amazon.get()))
				{
					if (this->amazon_scrape->current_page)
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
				else if (this->command.size() == 3 && equals<UnicodeString, false>(this->command.at(0).get(), Scrape::globals->command_amazon.get()))
				{
					this->amazon_context = std::make_shared<ByteString>();
					this->amazon_scrape = std::make_shared<Scrape::Amazon>(this->command.at(1), this->command.at(2), this->shared_from_this(), this->amazon_context);
                    this->amazon_scrape->start();

					writer.WriteLine("Amazon scrape started");
				}
				else if (this->command.size() == 1 && equals<UnicodeString, false>(this->command.at(0).get(), Scrape::globals->command_netflix.get()))
				{
					if (this->netflix_scrape->current_page)
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
				else if (this->command.size() == 4 && equals<UnicodeString, false>(this->command.at(0).get(), Scrape::globals->command_netflix.get()))
				{
					this->netflix_context = std::make_shared<ByteString>();
					this->netflix_scrape = std::make_shared<Scrape::Netflix>(this->command.at(1), this->command.at(2), this->command.at(3), this->shared_from_this(), this->netflix_context);
                    this->netflix_scrape->start();

					writer.WriteLine("Netflix scrape started");

					handled = true;
				}

                if (!handled)
                {
                    writer.WriteLine("commands: ");

                    for (CommandList::iterator it = this->command_list.begin(); it != this->command_list.end(); it++)
                    {
                        if (it != this->command_list.begin())
                            writer.write_literal(", ");

                        this->peer->write_elements((*it)->address(), (*it)->size());
                    }

                    writer.WriteLine();
                }
            }
            break;

        default:
            throw Basic::FatalError("Service", "AdminProtocol", "process_event", "unhandled state", this->get_state());
        }

        return ProcessResult::process_result_ready;
    }
    
    void AdminProtocol::write_to_human_with_context(Html::Node* node, IStream<Codepoint>* stream, bool verbose)
    {
        std::shared_ptr<Html::Node> parent = node->parent.lock();
        if (parent)
            write_to_human_with_context(parent.get(), stream, verbose);

        TextWriter writer(stream);
        writer.write_literal("/");

        if (parent)
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