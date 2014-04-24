﻿// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Service.AdminProtocol.h"
#include "Basic.TextWriter.h"
#include "Basic.AsyncBytes.h"
#include "Basic.Ref.h"
#include "Web.Client.h"
#include "Http.Globals.h"
#include "Service.Globals.h"
#include "Html.Parser.h"
#include "Html.Globals.h"
#include "Web.Form.h"
#include "Basic.TextSanitizer.h"
#include "Service.Types.h"

namespace Service
{
    using namespace Basic;

    void AdminProtocol::Initialize(Basic::IStream<Codepoint>* peer)
    {
        __super::Initialize();

        set_peer(peer);

        this->client = New<Web::Client>();
        this->client->Initialize();
    }

    void AdminProtocol::set_peer(Basic::IStream<Codepoint>* peer)
    {
        this->peer = peer;

        this->command.clear();
        this->command_frame.Initialize(&this->command);
    }

    void AdminProtocol::Process(Basic::IEvent* event, bool* yield)
    {
        Hold hold(this->lock);

        TextWriter writer(peer);

        (*yield) = true;

        if (event->get_type() == Service::EventType::task_complete_event)
        {
            throw new Exception("unexpected completion");
        }
        else if (event->get_type() == Http::EventType::response_headers_event)
        {
            Uri::Ref url;
            this->client->get_url(&url);

            UnicodeString::Ref charset;
            this->client->get_content_type_charset(&charset);

            this->html_parser = New<Html::Parser>();
            this->html_parser->Initialize(url, charset);

            this->client->set_body_stream(this->html_parser);
        }
        else if (event->get_type() == Http::EventType::response_complete_event)
        {
            Service::TaskCompleteEvent* cookie_event = (Service::TaskCompleteEvent*)event;

            if (cookie_event->cookie.item() == this->get_cookie.item())
            {
                if (this->html_parser.item() != 0)
                {
                    this->current_page = New<Web::Page>();
                    this->current_page->Initialize(this->html_parser->tree->document, this->client);

                    writer.WriteLine("Get completed");
                }
                else
                {
                    writer.WriteLine("Get failed to produce html");
                }
            }
            else
            {
                throw new Exception("unexpected completion");
            }
        }
        else if (event->get_type() == Http::EventType::response_headers_event)
        {
            Uri::Ref url;
            this->client->get_url(&url);

            UnicodeString::Ref charset;
            this->client->get_content_type_charset(&charset);

            Html::Parser::Ref parser = New<Html::Parser>();
            parser->Initialize(url, charset);

            this->client->set_body_stream(parser);
        }
        else switch (frame_state())
        {
        case State::start_state:
            this->command.clear();
            this->command_frame.Initialize(&this->command);
            switch_to_state(State::command_frame_pending_state);
            (*yield) = false;
            break;

        case State::command_frame_pending_state:
            if (this->command_frame.Pending())
            {
                this->command_frame.Frame::Process(event);
            }

            if (this->command_frame.Failed())
            {
                throw new Basic::Exception("Service::AdminProtocol command_frame failed");
            }
            else if (this->command_frame.Succeeded())
            {
                (*yield) = false;

                switch_to_state(State::start_state);

                bool handled = false;

                if (this->command.size() == 1 && this->command.at(0).equals<false>(Service::globals->command_stop))
                {
                    writer.WriteLine("stopping");
                    Service::globals->SendStopSignal();
                    handled = true;
                }
                else if (this->command.size() == 1 && this->command.at(0).equals<false>(Service::globals->command_log))
                {
                    Service::globals->debugLog->WriteTo(&writer);
                    handled = true;
                }
                else if (this->command.size() == 2 && this->command.at(0).equals<false>(Service::globals->command_get))
                {
                    if (this->client->frame_state() == Web::Client::State::inactive_state)
                    {
                        Basic::Uri::Ref url = New<Basic::Uri>();
                        url->Initialize();
                        
                        bool success = url->Parse(this->command[1], (Uri*)0);
                        if (!success)
                        {
                            writer.WriteLine("Url parse error");
                        }
                        else
                        {
                            this->get_cookie = New<ByteString>();

                            if (this->current_page.item() != 0)
                                this->client->http_cookies = this->current_page->http_cookies;

                            this->client->Get(url, this, this->get_cookie);
                        }
                    }
                    else
                    {
                        writer.WriteLine("Http client is busy");
                    }

                    handled = true;
                }
                else if (this->command.size() == 2 && this->command.at(0).equals<false>(Service::globals->command_follow_link))
                {
                    bool all_digits;
                    byte index = this->command.at(1)->as_base_10<byte>(&all_digits);
                    if (all_digits && index < this->current_page->links.size())
                    {
                        if (this->client->frame_state() == Web::Client::State::inactive_state)
                        {
                            Uri::Ref url = this->current_page->links[index]->url;
                            this->get_cookie = New<ByteString>();
                            this->client->http_cookies = this->current_page->http_cookies;
                            this->client->Get(url, this, this->get_cookie);
                        }
                        else
                        {
                            writer.WriteLine("Http client is busy");
                        }

                        handled = true;
                    }
                }
                else if (this->command.size() == 2 && this->command.at(0).equals<false>(Service::globals->command_select_form))
                {
                    bool all_digits;
                    byte index = this->command.at(1)->as_base_10<byte>(&all_digits);
                    if (all_digits && index < this->current_page->forms.size())
                    {
                        this->current_form = this->current_page->forms[index];

                        this->current_form->form_element->write_to_human(this->peer, true);
                        writer.WriteLine();

                        for (uint16 i = 0; i != this->current_form->controls.size(); i++)
                        {
                            writer.WriteFormat<0x10>("%d. ", i);
                            this->current_form->controls[i]->write_html_to_human(this->peer);
                            writer.WriteLine();
                        }

                        handled = true;
                    }
                }
                else if (this->command.size() >= 2 && this->command.at(0).equals<false>(Service::globals->command_set_control_value))
                {
                    bool all_digits;
                    byte index = this->command.at(1)->as_base_10<byte>(&all_digits);
                    if (all_digits && index < this->current_form->controls.size())
                    {
                        UnicodeString::Ref value = New<UnicodeString>();

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
                else if (this->command.size() == 1 && this->command.at(0).equals<false>(Service::globals->command_submit))
                {
                    this->client->http_cookies = this->current_page->http_cookies;

                    bool success = this->current_form->Submit(this->client, this, 0);
                    if (!success)
                        writer.WriteLine("Form submit failed");

                    handled = true;
                }
                else if (this->command.size() == 1 && this->command.at(0).equals<false>(Service::globals->command_render_links))
                {
                    if (this->current_page.item() != 0)
                    {
                        writer.WriteLine("Links <l>:");

                        for (uint16 i = 0; i != this->current_page->links.size(); i++)
                        {
                            writer.WriteFormat<0x10>("%d. ", i);
                            this->current_page->links[i]->text->write_to(this->peer);
                            writer.Write(" [");

                            if (this->current_page->links[i]->url.item() != 0)
                                this->current_page->links[i]->url->SerializeTo(this->peer, false, false);

                            writer.Write("]");
                            writer.WriteLine();
                        }
                    }
                    else
                    {
                        writer.WriteLine("No page loaded");
                    }

                    handled = true;
                }
                else if (this->command.size() == 1 && this->command.at(0).equals<false>(Service::globals->command_render_forms))
                {
                    if (this->current_page.item() != 0)
                    {
                        writer.WriteLine("Forms <f>:");

                        for (uint16 i = 0; i != this->current_page->forms.size(); i++)
                        {
                            writer.WriteFormat<0x10>("%d. ", i);
                            this->current_page->forms[i]->form_element->write_html_to_human(this->peer);
                            writer.WriteLine();
                        }
                    }
                    else
                    {
                        writer.WriteLine("No page loaded");
                    }

                    handled = true;
                }
                else if (this->command.size() >= 1 && this->command.size() <= 2 && this->command.at(0).equals<false>(Service::globals->command_render_nodes))
                {
                    if (this->current_page.item() != 0)
                    {
                        writer.WriteLine("Nodes:");

                        for (uint16 i = 0; i != this->current_page->leaf_nodes.size(); i++)
                        {
                            writer.WriteFormat<0x10>("%d. ", i);
                            write_to_human_with_context(this->current_page->leaf_nodes[i], this->peer, this->command.size() >= 2);
                            writer.WriteLine();
                        }
                    }
                    else
                    {
                        writer.WriteLine("No page loaded");
                    }

                    handled = true;
                }
                else if (this->command.size() == 2 && this->command.at(0).equals<false>(Service::globals->command_search))
                {
                    Json::Array::Ref results;
                    Service::globals->Search(this->command.at(1), &results);

                    results->write_to(peer);
                    writer.WriteLine();

                    handled = true;
                }

                if (!handled)
                {
                    writer.WriteLine("commands: ");

                    for (Globals::CommandList::iterator it = Service::globals->command_list.begin(); it != Service::globals->command_list.end(); it++)
                    {
                        if (it != Service::globals->command_list.begin())
                            writer.Write(", ");

                        this->peer->Write((*it)->c_str(), (*it)->size());
                    }

                    writer.WriteLine();
                }
            }
            break;

        default:
            throw new Basic::Exception("AdminProtocol::Process unexpected state");
        }
    }
    
    void AdminProtocol::write_to_human_with_context(Html::Node* node, IStream<Codepoint>* stream, bool verbose)
    {
        if (node->parent != 0)
            write_to_human_with_context(node->parent, stream, verbose);

        TextWriter writer(stream);
        writer.Write("/");

        if (node->parent != 0)
        {
            bool found = false;

            for (uint32 i = 0; i < node->parent->children.size(); i++)
            {
                if (node->parent->children.at(i).item() == node)
                {
                    writer.WriteFormat<0x10>("%d", i);
                    found = true;
                }
            }

            if (!found)
            {
                writer.Write("?");
            }

            writer.Write(".");
        }

        Inline<TextSanitizer> clean_stream;
        clean_stream.Initialize(stream);

        node->write_to_human(&clean_stream, verbose);
    }

}