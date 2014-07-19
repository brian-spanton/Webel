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

    void AdminProtocol::consider_event(Basic::IEvent* event)
    {
        Hold hold(this->lock);

        TextWriter writer(this->peer.get());

        if (event->get_type() == Service::EventType::task_complete_event)
        {
            throw FatalError("unexpected completion");
        }
        else if (event->get_type() == Http::EventType::response_headers_event)
        {
            std::shared_ptr<Uri> url;
            this->client->get_url(&url);

            UnicodeStringRef charset;
            this->client->get_content_type_charset(&charset);

            this->html_parser = std::make_shared<Html::Parser>();
            this->html_parser->Initialize(url, charset);

            this->client->set_body_stream(this->html_parser);

            throw Yield("event consumed");
        }
        else if (event->get_type() == Http::EventType::response_complete_event)
        {
            Service::TaskCompleteEvent* cookie_event = (Service::TaskCompleteEvent*)event;

            if (cookie_event->cookie.get() != this->get_cookie.get())
                throw FatalError("unexpected completion");

            if (this->html_parser.get() != 0)
            {
                this->current_page = std::make_shared<Web::Page>(this->html_parser->tree->document, this->client);

                writer.WriteLine("Get completed");
            }
            else
            {
                writer.WriteLine("Get failed to produce html");
            }

            throw Yield("event consumed");
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
                delegate_event_throw_error_on_fail(&this->command_frame, event);

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
                    Service::globals->debugLog->WriteTo(this->peer.get());
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

                            this->client->Get(url, this->shared_from_this(), this->get_cookie);
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
                            std::shared_ptr<Uri> url = this->current_page->links[index]->url;
                            this->get_cookie = std::make_shared<ByteString>();
                            this->client->http_cookies = this->current_page->http_cookies;
                            this->client->Get(url, this->shared_from_this(), this->get_cookie);
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
                    Service::globals->Search(this->command.at(1), &results);

                    results->write_to_stream(this->peer.get());
                    writer.WriteLine();

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