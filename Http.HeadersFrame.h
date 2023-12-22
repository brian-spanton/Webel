// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Frame.h"
#include "Basic.NameValueCollection.h"
#include "Http.Globals.h"

namespace Http
{
    using namespace Basic;

    class HeadersFrame : public Frame
    {
    private:
        enum State
        {
            expecting_name_state = Start_State,
            receiving_name_state,
            expecting_colon_state,
            expecting_value_state,
            expecting_next_header_state,
            expecting_LF_after_value_state,
            receiving_value_state,
            expecting_LF_after_headers_state,
            done_state = Succeeded_State,
            expecting_name_error,
            receiving_name_error,
            expecting_colon_error,
            expecting_LF_after_value_error,
            expecting_next_header_error,
            expecting_LF_after_headers_error,
        };

        NameValueCollection* nvc;
        UnicodeStringRef name;
        UnicodeStringRef value;

        virtual EventResult IProcess::consider_event(IEvent* event);

    public:
        HeadersFrame(NameValueCollection* nvc);
    };

    template <>
    struct __declspec(novtable) serialize<NameValueCollection>
    {
        void operator()(const NameValueCollection* value, IStream<byte>* stream) const
        {
            for (NameValueCollection::const_iterator it = value->cbegin(); it != value->cend(); it++)
            {
                ascii_encode(it->first.get(), stream);
                stream->write_element(Http::globals->colon);
                stream->write_element(Http::globals->SP);
                ascii_encode(it->second.get(), stream);
                stream->write_elements(Http::globals->CRLF, _countof(Http::globals->CRLF));
            }

            stream->write_elements(Http::globals->CRLF, _countof(Http::globals->CRLF));
        }
    };
}