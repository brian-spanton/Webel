// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Frame.h"
#include "Web.Client.h"
#include "Json.Parser.h"

namespace Service
{
    using namespace Basic;

    class StandardEncodings : public Frame
    {
    private:
        enum State
        {
            single_byte_encodings_state = Start_State,
            done_state = Succeeded_State,
        };

        Basic::Ref<Web::Client, IProcess> client; // REF
        Basic::Ref<Json::Parser, IStream<byte> > json_parser; // REF
        Basic::Ref<IProcess> encodings_completion; // REF
        ByteString::Ref encodings_cookie; // REF

        UnicodeString::Ref Name_encodings; // REF
        UnicodeString::Ref Name_heading; // REF
        UnicodeString::Ref heading_utf8; // REF
        UnicodeString::Ref heading_legacy; // REF
        UnicodeString::Ref Name_name; // REF
        UnicodeString::Ref Name_labels; // REF

        Uri::Ref encodings_url; // REF

    public:
        typedef Basic::Ref<StandardEncodings> Ref;

        StandardEncodings();

        void Initialize(Basic::Ref<IProcess> completion, ByteString::Ref cookie);

        virtual void IProcess::Process(IEvent* event, bool* yield);
    };
}
