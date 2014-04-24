// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Frame.h"
#include "Basic.NameValueCollection.h"
#include "Basic.ISerializable.h"

namespace Http
{
    using namespace Basic;

    class HeadersFrame : public Frame, public ISerializable
    {
    private:
        static byte colon;

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
        UnicodeString::Ref name; // REF
        UnicodeString::Ref value; // REF

    public:
        typedef Basic::Ref<HeadersFrame, IProcess> Ref;

        void Initialize(NameValueCollection* nvc);

        virtual void IProcess::Process(IEvent* event, bool* yield);

        virtual void ISerializable::SerializeTo(IStream<byte>* stream);
    };
}