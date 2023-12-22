// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Http.Globals.h"
#include "Http.HeadersFrame.h"
#include "Http.Types.h"
#include "Basic.Event.h"

namespace Http
{
    using namespace Basic;

    HeadersFrame::HeadersFrame(NameValueCollection* nvc) :
        nvc(nvc)
    {
    }

    EventResult HeadersFrame::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::expecting_name_state:
            {
                byte b;
                EventResult result = Event::ReadNext(event, &b);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                if (b == Http::globals->CR)
                {
                    switch_to_state(State::expecting_LF_after_headers_state);
                }
                else if (Http::globals->TOKEN[b])
                {
                    this->name = std::make_shared<UnicodeString>();
                    this->name->push_back(b);
                    switch_to_state(State::receiving_name_state);
                }
                else
                {
                    switch_to_state(State::expecting_name_error);
                }
            }
            break;

        case State::receiving_name_state:
            {
                byte b;
                EventResult result = Event::ReadNext(event, &b);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                if (b == Http::globals->colon)
                {
                    this->value = std::make_shared<UnicodeString>();
                    switch_to_state(State::expecting_value_state);
                }
                else if (b == Http::globals->SP || b == Http::globals->HT)
                {
                    this->value = std::make_shared<UnicodeString>();
                    switch_to_state(State::expecting_colon_state);
                }
                else if (Http::globals->TOKEN[b])
                {
                    this->name->push_back(b);
                }
                else if (b == ';')
                {
                    // a particular site seems to think this is ok, despite the RFC
                    this->name->push_back(b);
                }
                else
                {
                    switch_to_state(State::receiving_name_error);
                }
            }
            break;

        case State::expecting_colon_state:
            {
                byte b;
                EventResult result = Event::ReadNext(event, &b);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                if (b == Http::globals->colon)
                {
                    switch_to_state(State::expecting_value_state);
                }
                else if (b == Http::globals->SP || b == Http::globals->HT)
                {
                }
                else
                {
                    switch_to_state(State::expecting_colon_error);
                }
            }
            break;

        case State::expecting_value_state:
            {
                byte b;
                EventResult result = Event::ReadNext(event, &b);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                if (b == Http::globals->CR)
                {
                    switch_to_state(State::expecting_LF_after_value_state);
                }
                else if (b == Http::globals->SP || b == Http::globals->HT)
                {
                }
                else
                {
                    this->value->push_back(b);
                    switch_to_state(State::receiving_value_state);
                }
            }
            break;

        case State::receiving_value_state:
            {
                byte b;
                EventResult result = Event::ReadNext(event, &b);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                if (b == Http::globals->CR)
                {
                    switch_to_state(State::expecting_LF_after_value_state);
                }
                else
                {
                    this->value->push_back(b);
                }
            }
            break;

        case State::expecting_LF_after_value_state:
            {
                byte b;
                EventResult result = Event::ReadNext(event, &b);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                if (b == Http::globals->LF)
                {
                    switch_to_state(State::expecting_next_header_state);
                }
                else
                {
                    switch_to_state(State::expecting_LF_after_value_error);
                }
            }
            break;

        case State::expecting_next_header_state:
            {
                byte b;
                EventResult result = Event::ReadNext(event, &b);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                if (b == Http::globals->CR)
                {
                    NameValueCollection::value_type nv(this->name, this->value);
                    this->nvc->insert(nv);
                    this->name = 0;
                    this->value = 0;

                    switch_to_state(State::expecting_LF_after_headers_state);
                }
                else if (b == Http::globals->SP || b == Http::globals->HT)
                {
                    // this is called "folding", it's a continuation of the previous header line's value

                    this->value->push_back(Http::globals->SP);
                    switch_to_state(State::expecting_value_state);
                }
                else if (Http::globals->TOKEN[b])
                {
                    NameValueCollection::value_type nv(this->name, this->value);
                    this->nvc->insert(nv);
                    this->value = 0;

                    this->name = std::make_shared<UnicodeString>();
                    this->name->push_back(b);
                    switch_to_state(State::receiving_name_state);
                }
                else
                {
                    switch_to_state(State::expecting_next_header_error);
                }
            }
            break;

        case State::expecting_LF_after_headers_state:
            {
                byte b;
                EventResult result = Event::ReadNext(event, &b);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                if (b == Http::globals->LF)
                {
                    switch_to_state(State::done_state);
                }
                else
                {
                    switch_to_state(State::expecting_LF_after_headers_error);
                }
            }
            break;

        default:
            throw FatalError("HeadersFrame::handle_event unexpected state");
        }

        return EventResult::event_result_continue;
    }
}